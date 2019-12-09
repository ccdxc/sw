// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cluster

import (
	"context"
	"fmt"
	"net/http"
	"time"

	apierrors "github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"

	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/netutils"
)

// securityTestGroup for SG and SG policy tests
type securityTestGroup struct {
	suite           *TestSuite
	authAgentClient *netutils.HTTPClient
}

// instantiate test suite
var securityTg = &securityTestGroup{}

// setupTest setup test suite
func (stg *securityTestGroup) setupTest() {
	stg.suite = ts
	Eventually(func() error {
		ctx, cancel := context.WithTimeout(ts.tu.MustGetLoggedInContext(context.Background()), 5*time.Second)
		defer cancel()
		var err error
		stg.authAgentClient, err = utils.GetNodeAuthTokenHTTPClient(ctx, ts.tu.APIGwAddr, []string{"*"})
		return err
	}, 30, 5).Should(BeNil(), "Failed to get node auth token")
}

// teardownTest cleans up test suite
func (stg *securityTestGroup) teardownTest() {
}

// getSGPolicies() returns the list of SG Policies as read from the agent REST interface
func (stg *securityTestGroup) getSGPolicies(agent string) ([]security.NetworkSecurityPolicy, error) {
	var sgplist []security.NetworkSecurityPolicy
	status, err := stg.authAgentClient.Req("GET", "https://"+agent+":"+globals.AgentProxyPort+"/api/security/policies/", nil, &sgplist)
	if err != nil || status != http.StatusOK {
		return nil, fmt.Errorf("Error getting SG Policies list: %v", err)
	}
	return sgplist, nil
}

// getSecurityGroups() returns the list of security groups as read from the agent REST interface
func (stg *securityTestGroup) getSecurityGroups(agent string) ([]security.NetworkSecurityPolicy, error) {
	var sgplist []security.NetworkSecurityPolicy
	status, err := stg.authAgentClient.Req("GET", "https://"+agent+":"+globals.AgentProxyPort+"/api/sgs/", nil, &sgplist)
	if err != nil || status != http.StatusOK {
		return nil, fmt.Errorf("Error getting SG Policies list: %v", err)
	}
	return sgplist, nil
}

// testSgCreateDelete tests security group create delete
func (stg *securityTestGroup) testSgpolicyCreateDelete() {
	// sg policy params
	sg := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "policy1",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					FromIPAddresses: []string{"10.0.0.0/24"},
					ToIPAddresses:   []string{"11.0.0.0/24"},
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: "PERMIT",
				},
			},
		},
	}

	// create sg policy
	resp, err := stg.suite.restSvc.SecurityV1().NetworkSecurityPolicy().Create(stg.suite.loggedInCtx, &sg)
	apiErr := apierrors.FromError(err)
	log.Errorf("%v", apiErr)
	Expect(err).ShouldNot(HaveOccurred())

	// verify we can read the policy back
	rsg, err := stg.suite.restSvc.SecurityV1().NetworkSecurityPolicy().Get(stg.suite.loggedInCtx, &sg.ObjectMeta)
	Expect(err).ShouldNot(HaveOccurred())
	Expect(rsg.Spec).Should(Equal(resp.Spec))

	// verify agents have the policy
	Eventually(func() bool {
		for _, naplesIP := range ts.tu.NaplesNodeIPs {
			sgplist, err := stg.getSGPolicies(naplesIP)
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
	_, err = stg.suite.restSvc.SecurityV1().NetworkSecurityPolicy().Delete(stg.suite.loggedInCtx, &sg.ObjectMeta)
	Expect(err).ShouldNot(HaveOccurred())

	// verify policy is gone from the agents
	Eventually(func() bool {
		for _, naplesIP := range ts.tu.NaplesNodeIPs {
			sgplist, err := stg.getSGPolicies(naplesIP)
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
		for _, naplesIP := range ts.tu.NaplesNodeIPs {
			sglist, err := stg.getSecurityGroups(naplesIP)
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
	Eventually(func() error {
		_, err = stg.suite.restSvc.SecurityV1().SecurityGroup().Delete(stg.suite.loggedInCtx, &sg.ObjectMeta)
		return err
	}, 30, 1).ShouldNot(HaveOccurred())

	// verify policy is gone from the agents
	Eventually(func() bool {
		for _, naplesIP := range ts.tu.NaplesNodeIPs {
			sglist, err := stg.getSecurityGroups(naplesIP)
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
var _ = Describe("security", func() {
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
