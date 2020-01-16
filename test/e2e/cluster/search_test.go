package cluster

import (
	"errors"
	"fmt"
	"time"

	uuid "github.com/satori/go.uuid"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/search"
	testutils "github.com/pensando/sw/test/utils"

	"github.com/pensando/sw/venice/globals"
)

type queryTestCase struct {
	query         *search.SearchRequest
	resultCheck   func(resp interface{}, tc queryTestCase) error
	resp          interface{}
	expNumEntries int64
	expErr        error
}

var (
	dummyObjName = fmt.Sprintf("st-%s", uuid.NewV4().String()) // search test object
)

func auditEventResultCheck(resp interface{}, tc queryTestCase) error {
	aResp := resp.(*testutils.AuditSearchResponse)
	if tc.expNumEntries != aResp.ActualHits {
		return fmt.Errorf("Expected %d entries, actual was %d. ", tc.expNumEntries, aResp.ActualHits)
	}
	for _, ele := range aResp.Entries {
		if ele.Object.RequestObject != "" || ele.Object.ResponseObject != "" {
			return errors.New("Checking audit event's response and request should be empty string. ")
		}
	}
	return nil
}

func defaultResultCheck(resp interface{}, tc queryTestCase) error {
	dResp := resp.(*search.SearchResponse)
	if tc.expNumEntries != dResp.ActualHits {
		return fmt.Errorf("Expected %d entries, actual was %d. ", tc.expNumEntries, dResp.ActualHits)
	}
	return nil
}

var _ = Describe("search test", func() {

	BeforeEach(func() {
		validateCluster()
	})

	It("spyglass restart", func() {
		createDummyObj()
		testCases := []*queryTestCase{
			{
				query: &search.SearchRequest{
					Query: &search.SearchQuery{
						Texts: []*search.TextRequirement{
							{
								Text: []string{fmt.Sprintf(`"%s"`, dummyObjName)},
							},
						},
						Categories: []string{"Monitoring"},
					},
					Mode:      search.SearchRequest_Full.String(),
					Aggregate: true,
				},
				expNumEntries: 3, // 1 for the object, 2 for audit events
				resultCheck:   defaultResultCheck,
				resp:          &search.SearchResponse{},
			},
		}
		testQueries(testCases)

		By("Restarting spyglass...")
		_, err := ts.tu.KillContainer(globals.Spyglass)
		Expect(err).To(BeNil())

		testQueries(testCases)
		deleteDummyObj()
		testCases = []*queryTestCase{
			{
				query: &search.SearchRequest{
					Query: &search.SearchQuery{
						Texts: []*search.TextRequirement{
							{
								Text: []string{fmt.Sprintf(`"%s"`, dummyObjName)},
							},
						},
						Kinds:      []string{"AuditEvent"},
						Categories: []string{"Monitoring"},
					},
					Mode:      search.SearchRequest_Full.String(),
					Aggregate: true,
				},
				expNumEntries: 4,
				resultCheck:   auditEventResultCheck,
				resp:          &testutils.AuditSearchResponse{},
			},
		}
		testQueries(testCases)
		testCases = []*queryTestCase{
			{
				query: &search.SearchRequest{
					Query: &search.SearchQuery{
						Texts: []*search.TextRequirement{
							{
								Text: []string{fmt.Sprintf(`"%s"`, dummyObjName)},
							},
						},
						Kinds:      []string{"AlertPolicy"},
						Categories: []string{"Monitoring"},
					},
					Mode:      search.SearchRequest_Full.String(),
					Aggregate: true,
				},
				expNumEntries: 0,
				resultCheck:   defaultResultCheck,
				resp:          &search.SearchResponse{},
			},
		}
		testQueries(testCases)
	})

	AfterEach(func() {
		validateCluster()
	})
})

func createDummyObj() {
	By(fmt.Sprintf("ts=%s creating alert policy {%s}", time.Now().String(), dummyObjName))
	dummyPolicy := &monitoring.AlertPolicy{
		TypeMeta: api.TypeMeta{Kind: "AlertPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:   dummyObjName,
			Tenant: globals.DefaultTenant,
		},
		Spec: monitoring.AlertPolicySpec{
			Severity: "INFO",
		},
	}
	_, err := ts.restSvc.MonitoringV1().AlertPolicy().Create(ts.loggedInCtx, dummyPolicy)
	Expect(err).To(BeNil())
}

func deleteDummyObj() {
	By(fmt.Sprintf("ts=%s deleting alert policy {%s}", time.Now().String(), dummyObjName))
	dummyPolicyMeta := &api.ObjectMeta{
		Name:   dummyObjName,
		Tenant: globals.DefaultTenant,
	}
	_, err := ts.restSvc.MonitoringV1().AlertPolicy().Delete(ts.loggedInCtx, dummyPolicyMeta)
	Expect(err).To(BeNil())
}

func testQueries(testCases []*queryTestCase) {
	By("Executing Queries...")
	Eventually(func() bool {
		for i, tc := range testCases {
			resp := tc.resp

			err := ts.tu.Search(ts.loggedInCtx, tc.query, resp)
			if err != tc.expErr {
				By(fmt.Sprintf("tc {%d}: expected err %v, actual err was %v", i, tc.expErr, err))
				return false
			}

			if err := tc.resultCheck(resp, *tc); err != nil {
				By(fmt.Sprintf("tc {%d}, query_results: %v, err: %s", i, resp, err))
				return false
			}

		}

		return true

	}, 180, 10).Should(BeTrue(), fmt.Sprintf("ts: %s Query tests failed", time.Now().String()))
}
