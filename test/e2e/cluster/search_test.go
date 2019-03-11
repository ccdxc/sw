package cluster

import (
	"fmt"

	"github.com/satori/go.uuid"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/search"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

type queryTestCase struct {
	query         *search.SearchRequest
	expNumEntries int64
	expErr        error
}

var (
	dummyObjName = uuid.NewV4().String()
)

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
			},
		}
		testQueries(testCases)
		restartSpyglass()
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
			},
		}
		testQueries(testCases)
	})
})

func restartSpyglass() {
	log.Info("Restarting spyglass...")
	getDockerContainerID := func(node, name string) string {
		return ts.tu.CommandOutput(node, fmt.Sprintf("docker ps -q -f Name=%s", name))
	}
	restartDockerContainer := func(node, id string) {
		cmd := fmt.Sprintf("docker kill %s > /dev/null", id)
		_ = ts.tu.CommandOutputIgnoreError(node, cmd)
	}

	containerID := ""
	nodeIP := ""
	for _, ip := range ts.tu.VeniceNodeIPs {
		containerID = getDockerContainerID(ip, "pen-spyglass")
		if containerID != "" {
			nodeIP = ip
			break
		}
	}
	Expect(len(containerID) > 0).To(BeTrue())
	restartDockerContainer(nodeIP, containerID)
}

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
	log.Info("Executing Queries...")
	Eventually(func() bool {
		for i, tc := range testCases {
			resp := search.SearchResponse{}
			err := ts.tu.Search(ts.loggedInCtx, tc.query, &resp)
			if err != tc.expErr {
				log.Errorf("Test Case %d: Expected err %v, actual err was %d", i, tc.expErr, err)
				return false
			}
			if tc.expNumEntries != resp.ActualHits {
				log.Errorf("Test Case %d: Expected %d entries, actual was %d", i, tc.expNumEntries, resp.ActualHits)
				return false
			}
		}
		return true

	}, 90, 10).Should(BeTrue(), "Query tests failed")
}
