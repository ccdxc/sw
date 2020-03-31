package cluster

import (
	"context"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	apierrors "github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

type dscProfileTestGroup struct {
	suite           *TestSuite
	authAgentClient *netutils.HTTPClient
}

// instantiate test suite
var dscProfileTg = &dscProfileTestGroup{}

// setupTest setup test suite
func (dscTg *dscProfileTestGroup) setupTest() {
	dscTg.suite = ts
	Eventually(func() error {
		ctx, cancel := context.WithTimeout(ts.tu.MustGetLoggedInContext(context.Background()), 5*time.Second)
		defer cancel()
		var err error
		dscTg.authAgentClient, err = utils.GetNodeAuthTokenHTTPClient(ctx, ts.tu.APIGwAddr, []string{"*"})
		return err
	}, 30, 5).Should(BeNil(), "Failed to get node auth token")
}

// teardownTest cleans up test suite
func (dscTg *dscProfileTestGroup) teardownTest() {
}

func (dscTg *dscProfileTestGroup) testProfileCreateDelete() {
	dscProfile := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: "dscProfile1",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "TRANSPARENT",
			FlowPolicyMode: "BASENET",
		},
	}

	_, err := dscTg.suite.restSvc.ClusterV1().DSCProfile().Create(dscTg.suite.loggedInCtx, &dscProfile)
	apiErr := apierrors.FromError(err)
	log.Errorf("%v", apiErr)
	Expect(err).ShouldNot(HaveOccurred())

	rsg, err := dscTg.suite.restSvc.ClusterV1().DSCProfile().Get(dscTg.suite.loggedInCtx, &dscProfile.ObjectMeta)
	Expect(err).ShouldNot(HaveOccurred())

	_, err = dscTg.suite.restSvc.ClusterV1().DSCProfile().Delete(dscTg.suite.loggedInCtx, &rsg.ObjectMeta)
	apiErr = apierrors.FromError(err)
	log.Errorf("%v", apiErr)
	Expect(err).ShouldNot(HaveOccurred())

}

func (dscTg *dscProfileTestGroup) testDSCUpdateWithSameProfile() {

	insertionEnforced := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: "insertion.enforced",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "INSERTION",
			FlowPolicyMode: "ENFORCED",
		},
	}

	_, err := dscTg.suite.restSvc.ClusterV1().DSCProfile().Create(dscTg.suite.loggedInCtx, &insertionEnforced)
	apiErr := apierrors.FromError(err)
	log.Errorf("%v", apiErr)
	Expect(err).ShouldNot(HaveOccurred())

	ctx := context.Background()
	snIf := dscTg.suite.tu.APIClient.ClusterV1().DistributedServiceCard()
	snics, _ := snIf.List(ctx, &api.ListWatchOptions{})
	for _, snic := range snics {
		snic.Spec.DSCProfile = "insertion.enforced"
		_, err = snIf.Update(ctx, snic)
	}
	Eventually(func() bool {
		dstat, err := dscTg.suite.restSvc.ClusterV1().DSCProfile().Get(dscTg.suite.loggedInCtx, &insertionEnforced.ObjectMeta)
		Expect(err).ShouldNot(HaveOccurred())
		if dstat.Status.PropagationStatus.Pending != 0 {
			log.Errorf("insertion.enforced Status rsg %v", dstat.Status)
			return false
		}
		return true
	}, 60, 1).Should(BeTrue(), "Failed to Propagate the profile")

	//Verify the agent, status
	snics, err = snIf.List(ctx, &api.ListWatchOptions{})
	for _, snic := range snics {
		snic.Spec.DSCProfile = insertionFWProfileName
		_, err = snIf.Update(ctx, snic)
	}

	Eventually(func() bool {
		dstat, err := dscTg.suite.restSvc.ClusterV1().DSCProfile().Get(dscTg.suite.loggedInCtx, &insertionEnforced.ObjectMeta)
		Expect(err).ShouldNot(HaveOccurred())
		if dstat.Status.PropagationStatus.Pending != 0 {
			log.Errorf("%s Status rsg %v", insertionFWProfileName, dstat.Status)

			return false
		}
		return true
	}, 60, 1).Should(BeTrue(), "Failed to Propagate the profile")
}

var _ = Describe("profile createdelete tests", func() {
	Context("DSCProfile Creation & Deletion", func() {

		// setup
		BeforeEach(dscProfileTg.setupTest)

		// test cases
		It("DSCProfile should be created", dscProfileTg.testProfileCreateDelete)
		It("DSCProfile should be updated", dscProfileTg.testDSCUpdateWithSameProfile)

		AfterEach(dscProfileTg.teardownTest)
	})
})
