package veniceinteg

import (
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestDSCProfile tests dscprofile Create/Update/Delete operation
func (it *veniceIntegSuite) TestDSCProfileCRUD(c *C) {
	if it.config.DatapathKind == "hal" {
		c.Skip("Not tested with Hal")
	}
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// DSC Profile
	dscProfile := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "TestDSCProfile",
			Namespace: "",
			Tenant:    "",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "TRANSPARENT",
			FlowPolicyMode: "BASE_NET",
		},
	}

	// create dsc
	_, err = it.restClient.ClusterV1().DSCProfile().Create(ctx, &dscProfile)
	AssertOk(c, err, "Error Creating TestDSCProfile profile")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "TestDSCProfile")
		log.Infof("Profile found")
		if obj.DSCProfile.DSCProfile.Spec.FwdMode == cluster.DSCProfileSpec_TRANSPARENT.String() &&
			obj.DSCProfile.DSCProfile.Spec.FlowPolicyMode == cluster.DSCProfileSpec_BASE_NET.String() {
			log.Infof("Profile matched")

			return (nerr == nil), true
		}
		return false, false
	}, "Profile not found")

	// change conn track and session timeout
	dscProfile.Spec.FlowPolicyMode = cluster.DSCProfileSpec_FLOW_AWARE.String()
	_, err = it.restClient.ClusterV1().DSCProfile().Update(ctx, &dscProfile)
	AssertOk(c, err, "Error updating dscprofile")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "TestDSCProfile")
		if obj.DSCProfile.DSCProfile.Spec.FwdMode == cluster.DSCProfileSpec_TRANSPARENT.String() &&
			obj.DSCProfile.DSCProfile.Spec.FlowPolicyMode == cluster.DSCProfileSpec_FLOW_AWARE.String() {
			return (nerr == nil), nil
		}
		return false, false
	}, "DSCProfile not update")

	_, err = it.restClient.ClusterV1().DSCProfile().Delete(ctx, &dscProfile.ObjectMeta)
	AssertOk(c, err, "Error deleting dscprofile")

	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.ctrler.StateMgr.FindDSCProfile("", "TestDSCProfile")
		if nerr != nil {
			return true, true
		}
		return false, false
	}, "DSCProfile not Deleted")

}

// TestDistributedServiceCardUpdate tests relationship between DistributedServiceCard->DSCProfile
func (it *veniceIntegSuite) TestDistributedServiceCardUpdate(c *C) {
	if it.config.DatapathKind == "hal" {
		c.Skip("Not tested with Hal")
	}
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// DSC Profile
	dscProfile := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "TestDSCProfile",
			Namespace: "",
			Tenant:    "",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "TRANSPARENT",
			FlowPolicyMode: "BASE_NET",
		},
	}
	// create app
	_, err = it.restClient.ClusterV1().DSCProfile().Create(ctx, &dscProfile)
	AssertOk(c, err, "Error Creating TestDSCProfile profile")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "TestDSCProfile")
		log.Infof("Profile found")
		if obj.DSCProfile.DSCProfile.Spec.FwdMode == cluster.DSCProfileSpec_TRANSPARENT.String() &&
			obj.DSCProfile.DSCProfile.Spec.FlowPolicyMode == cluster.DSCProfileSpec_BASE_NET.String() {
			log.Infof("Profile matched")

			return (nerr == nil), true
		}
		return false, false
	}, "Profile not found")

	dscProfile1 := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "default",
			Namespace: "",
			Tenant:    "",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "INSERTION",
			FlowPolicyMode: "BASE_NET",
		},
	}

	_, err = it.restClient.ClusterV1().DSCProfile().Create(ctx, &dscProfile1)
	AssertOk(c, err, "Error Creating TestDSCProfile profile")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "default")
		log.Infof("Profile found")
		if obj.DSCProfile.DSCProfile.Spec.FwdMode == cluster.DSCProfileSpec_INSERTION.String() &&
			obj.DSCProfile.DSCProfile.Spec.FlowPolicyMode == cluster.DSCProfileSpec_BASE_NET.String() {
			log.Infof("Profile matched")

			return (nerr == nil), true
		}
		return false, false
	}, "Profile not found")

	dscObj, err := it.getDistributedServiceCard(it.getNaplesMac(0))
	AssertOk(c, err, "Error DSC object not found")

	dscObj.Spec.DSCProfile = "TestDSCProfile"

	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, dscObj)
	AssertOk(c, err, "Error DistributedServicesCard update failed")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "TestDSCProfile")
		if _, ok := obj.DscList[it.getNaplesMac(0)]; ok {
			return (nerr == nil), true
		}
		return false, false
	}, "DSCProfile not update")
	log.Infof("Profile Updated Successfully")

	dscObj, err = it.getDistributedServiceCard(it.getNaplesMac(0))
	AssertOk(c, err, "Error DSC object not found")

	dscObj.Spec.DSCProfile = "default"
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, dscObj)
	AssertOk(c, err, "Error DistributedServicesCard update failed")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "TestDSCProfile")
		if _, ok := obj.DscList[it.getNaplesMac(0)]; !ok {
			return (nerr == nil), true
		}
		return false, false
	}, "DSCProfile not update")

	dscObj, err = it.getDistributedServiceCard(it.getNaplesMac(0))
	AssertOk(c, err, "Error DSC object not found")

	log.Infof("Profile is: %v", dscObj.Spec.DSCProfile)

	_, err = it.restClient.ClusterV1().DSCProfile().Delete(ctx, &dscProfile.ObjectMeta)
	AssertOk(c, err, "Error deleting dscprofile")

	AssertEventually(c, func() (bool, interface{}) {
		_, nerr := it.ctrler.StateMgr.FindDSCProfile("", "TestDSCProfile")
		if nerr != nil {
			return true, true
		}
		return false, false
	}, "DSCProfile not Deleted")

}

// Update profile
//Error test case
//     ---> delete profile when there is still reference from DSC
//
