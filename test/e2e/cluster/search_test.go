package cluster

import (
	"errors"
	"fmt"

	"github.com/satori/go.uuid"

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
	dummyObjName = uuid.NewV4().String()
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

var _ = Describe("Search test", func() {
	It("spyglass restart", func() {
		createDummyObj()
		testCases := []*queryTestCase{
			&queryTestCase{
				query: &search.SearchRequest{
					Query: &search.SearchQuery{
						Texts: []*search.TextRequirement{
							&search.TextRequirement{
								Text: []string{fmt.Sprintf(`"%s"`, dummyObjName)},
							},
						},
						Categories: []string{"Monitoring"},
					},
					Mode: search.SearchRequest_Full.String(),
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
			&queryTestCase{
				query: &search.SearchRequest{
					Query: &search.SearchQuery{
						Texts: []*search.TextRequirement{
							&search.TextRequirement{
								Text: []string{fmt.Sprintf(`"%s"`, dummyObjName)},
							},
						},
						Kinds:      []string{"AuditEvent"},
						Categories: []string{"Monitoring"},
					},
					Mode: search.SearchRequest_Full.String(),
				},
				expNumEntries: 4,
				resultCheck:   auditEventResultCheck,
				resp:          &testutils.AuditSearchResponse{},
			},
		}
		testQueries(testCases)
		testCases = []*queryTestCase{
			&queryTestCase{
				query: &search.SearchRequest{
					Query: &search.SearchQuery{
						Texts: []*search.TextRequirement{
							&search.TextRequirement{
								Text: []string{fmt.Sprintf(`"%s"`, dummyObjName)},
							},
						},
						Kinds:      []string{"AlertPolicy"},
						Categories: []string{"Monitoring"},
					},
					Mode: search.SearchRequest_Full.String(),
				},
				expNumEntries: 0,
				resultCheck:   defaultResultCheck,
				resp:          &search.SearchResponse{},
			},
		}
		testQueries(testCases)
	})
})

func createDummyObj() {
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
				By(fmt.Sprintf("Test Case %d: Expected err %v, actual err was %d", i, tc.expErr, err))
				return false
			}

			if err := tc.resultCheck(resp, *tc); err != nil {
				By(fmt.Sprintf("Test Case %d %s", i, err))
				return false
			}

		}

		return true

	}, 90, 10).Should(BeTrue(), "Query tests failed")
}
