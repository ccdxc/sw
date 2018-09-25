// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cluster

import (
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"

	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/labels"
)

// securityTestGroup for SG and SG policy tests
type securityTestGroup struct {
	suite *TestSuite
}

// instantiate test suite
var securityTg = &securityTestGroup{}

// setupTest setup test suite
func (stg *securityTestGroup) setupTest() {
	stg.suite = ts
}

// teardownTest cleans up test suite
func (stg *securityTestGroup) teardownTest() {
}

// testSgCreateDelete tests security group create delete
func (stg *securityTestGroup) testSgpolicyCreateDelete() {
	// sg policy params
	sg := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "policy1",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules: []*security.SGRule{
				{
					FromIPAddresses: []string{"10.0.0.0/24"},
					ToIPAddresses:   []string{"11.0.0.0/24"},
					Apps:            []string{"tcp/80"},
					Action:          "PERMIT",
				},
			},
		},
	}

	// create sg policy
	resp, err := stg.suite.restSvc.SecurityV1().SGPolicy().Create(stg.suite.loggedInCtx, &sg)
	Expect(err).ShouldNot(HaveOccurred())

	// verify we can read the policy back
	rsg, err := stg.suite.restSvc.SecurityV1().SGPolicy().Get(stg.suite.loggedInCtx, &sg.ObjectMeta)
	Expect(err).ShouldNot(HaveOccurred())
	Expect(rsg).Should(Equal(resp))

	// verify agents have the policy
	Eventually(func() bool {
		for _, rclient := range stg.suite.netagentClients {
			sgplist, err := rclient.SGPolicyList()
			if err != nil {
				By(fmt.Sprintf("ts:%s security policy list failed, err: %+v policies: %+v", time.Now().String(), err, sgplist))
				return false
			}
			if (len(sgplist) != 1) || (sgplist[0].Name != sg.Name) {
				By(fmt.Sprintf("ts:%s security policy list has invalid items, sg policies: %+v", time.Now().String(), sgplist))
				return false
			}
		}
		return true
	}, 30, 1).Should(BeTrue(), "Failed to get sg policies on netagent")

	// delete the sg policy
	_, err = stg.suite.restSvc.SecurityV1().SGPolicy().Delete(stg.suite.loggedInCtx, &sg.ObjectMeta)
	Expect(err).ShouldNot(HaveOccurred())

	// verify policy is gone from the agents
	Eventually(func() bool {
		for _, rclient := range stg.suite.netagentClients {
			sgplist, err := rclient.SGPolicyList()
			if err != nil {
				By(fmt.Sprintf("ts:%s security policy list failed, err: %+v policies: %+v", time.Now().String(), err, sgplist))
				return false
			}
			if len(sgplist) != 0 {
				By(fmt.Sprintf("ts:%s security policy list has invalid items, sg policies: %+v", time.Now().String(), sgplist))
				return false
			}
		}
		return true
	}, 30, 1).Should(BeTrue(), "Failed to get sg policies on netagent")
}

// testSecurityGroupCreateDelete tests create/delete on security group
func (stg *securityTestGroup) testSecurityGroupCreateDelete() {
	// sg params
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "group1",
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}),
		},
	}

	// create sg policy
	resp, err := stg.suite.restSvc.SecurityV1().SecurityGroup().Create(stg.suite.loggedInCtx, &sg)
	Expect(err).ShouldNot(HaveOccurred())

	// verify we can read the policy back
	rsg, err := stg.suite.restSvc.SecurityV1().SecurityGroup().Get(stg.suite.loggedInCtx, &sg.ObjectMeta)
	Expect(err).ShouldNot(HaveOccurred())
	Expect(rsg).Should(Equal(resp))

	// verify agents have the policy
	Eventually(func() bool {
		for _, rclient := range stg.suite.netagentClients {
			sglist, err := rclient.SecurityGroupList()
			if err != nil {
				By(fmt.Sprintf("ts:%s security group list failed, err: %+v sgs: %+v", time.Now().String(), err, sglist))
				return false
			}
			if (len(sglist) != 1) || (sglist[0].Name != sg.Name) {
				By(fmt.Sprintf("ts:%s security group list has invalid items, security groups: %+v", time.Now().String(), sglist))
				return false
			}
		}
		return true
	}, 30, 1).Should(BeTrue(), "Failed to get security groups on netagent")

	// delete the sg policy
	_, err = stg.suite.restSvc.SecurityV1().SecurityGroup().Delete(stg.suite.loggedInCtx, &sg.ObjectMeta)
	Expect(err).ShouldNot(HaveOccurred())

	// verify policy is gone from the agents
	Eventually(func() bool {
		for _, rclient := range stg.suite.netagentClients {
			sglist, err := rclient.SecurityGroupList()
			if err != nil {
				By(fmt.Sprintf("ts:%s security group list failed, err: %+v sgs: %+v", time.Now().String(), err, sglist))
				return false
			}
			if len(sglist) != 0 {
				By(fmt.Sprintf("ts:%s security group list has invalid items, sg groups: %+v", time.Now().String(), sglist))
				return false
			}
		}
		return true
	}, 30, 1).Should(BeTrue(), "Failed to get security groups on netagent")
}

// Security test suite
var _ = Describe("Security", func() {
	Context("Security policy tests", func() {
		// setup
		BeforeEach(securityTg.setupTest)

		// test cases
		It("Security policy create delete should succeed", securityTg.testSgpolicyCreateDelete)
		It("Security group create delete should succeed", securityTg.testSecurityGroupCreateDelete)

		// test cleanup
		AfterEach(securityTg.teardownTest)
	})
})
