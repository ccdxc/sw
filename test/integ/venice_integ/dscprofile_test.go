package veniceinteg

import (
	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
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
			FlowPolicyMode: "BASENET",
		},
	}

	// create dsc
	_, err = it.restClient.ClusterV1().DSCProfile().Create(ctx, &dscProfile)
	AssertOk(c, err, "Error Creating TestDSCProfile profile")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "TestDSCProfile")
		log.Infof("Profile found")
		if obj.DSCProfile.DSCProfile.Spec.FwdMode == cluster.DSCProfileSpec_TRANSPARENT.String() &&
			obj.DSCProfile.DSCProfile.Spec.FlowPolicyMode == cluster.DSCProfileSpec_BASENET.String() {
			log.Infof("Profile matched")

			return (nerr == nil), true
		}
		return false, false
	}, "Profile not found")

	// change conn track and session timeout
	dscProfile.Spec.FlowPolicyMode = cluster.DSCProfileSpec_FLOWAWARE.String()
	_, err = it.restClient.ClusterV1().DSCProfile().Update(ctx, &dscProfile)
	AssertOk(c, err, "Error updating dscprofile")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "TestDSCProfile")
		if obj.DSCProfile.DSCProfile.Spec.FwdMode == cluster.DSCProfileSpec_TRANSPARENT.String() &&
			obj.DSCProfile.DSCProfile.Spec.FlowPolicyMode == cluster.DSCProfileSpec_FLOWAWARE.String() {
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

	nic := it.snics[0]
	// DSC Profile
	dscProfile := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "NewProfileLNS",
			Namespace: "",
			Tenant:    "",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "TRANSPARENT",
			FlowPolicyMode: "BASENET",
		},
	}
	// create app
	_, err = it.restClient.ClusterV1().DSCProfile().Create(ctx, &dscProfile)
	AssertOk(c, err, "Error Creating NewProfileLNS profile")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "NewProfileLNS")
		log.Infof("Profile found")
		if obj.DSCProfile.DSCProfile.Spec.FwdMode == cluster.DSCProfileSpec_TRANSPARENT.String() &&
			obj.DSCProfile.DSCProfile.Spec.FlowPolicyMode == cluster.DSCProfileSpec_BASENET.String() {
			log.Infof("Profile matched")

			return (nerr == nil), true
		}
		return false, false
	}, "Profile not found")

	dscObj, err := it.getDistributedServiceCard(nic.macAddr)
	AssertOk(c, err, "Error DSC object not found")
	dscObj.Spec.DSCProfile = "NewProfileLNS"
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, dscObj)
	AssertOk(c, err, "Error DistributedServicesCard update failed")

	AssertEventually(c, func() (bool, interface{}) {

		ag := nic.agent
		profile := netproto.Profile{
			TypeMeta: api.TypeMeta{Kind: "Profile"},
		}
		profiles, _ := ag.PipelineAPI.HandleProfile(agentTypes.List, profile)
		if len(profiles) != 1 {
			return false, nil
		}

		return true, nil
	}, "DSCProfile was not updated in agent", "100ms", it.pollTimeout())

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "NewProfileLNS")
		if _, ok := obj.DscList[nic.macAddr]; ok {
			return (nerr == nil), true
		}
		return false, false
	}, "DSCProfile not update")
	log.Infof("Profile Updated Successfully")

	dscProfile1 := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "insertion.enforced1",
			Namespace: "",
			Tenant:    "",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "INSERTION",
			FlowPolicyMode: "ENFORCED",
		},
	}

	_, err = it.restClient.ClusterV1().DSCProfile().Create(ctx, &dscProfile1)
	AssertOk(c, err, "Error Creating insertion.enforced profile")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "insertion.enforced1")
		log.Infof("Profile found")
		if obj.DSCProfile.DSCProfile.Spec.FwdMode == cluster.DSCProfileSpec_INSERTION.String() &&
			obj.DSCProfile.DSCProfile.Spec.FlowPolicyMode == cluster.DSCProfileSpec_ENFORCED.String() {
			log.Infof("Profile matched")

			return (nerr == nil), true
		}
		return false, false
	}, "Profile not found")

	dscObj, err = it.getDistributedServiceCard(nic.macAddr)
	AssertOk(c, err, "Error DSC object not found")

	dscObj.Spec.DSCProfile = "insertion.enforced1"
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, dscObj)
	AssertOk(c, err, "Error DistributedServicesCard update failed")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "insertion.enforced1")
		if _, ok := obj.DscList[nic.macAddr]; ok {
			return (nerr == nil), true
		}
		return false, false
	}, "DSCProfile not update")

	dscObj, err = it.getDistributedServiceCard(nic.macAddr)
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

// TestDistributedServiceCardUpdateFail tests relationship between DistributedServiceCard->DSCProfile
func (it *veniceIntegSuite) TestDistributedServiceCardXXXFail(c *C) {
	c.Skip("Need to enable this once find a way to delete and re admit the card")
	if it.config.DatapathKind == "hal" {
		c.Skip("Not tested with Hal")
	}
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// TRANSPARENT.FLOWAWARE
	transparentFlowaware := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "transparent.flowaware",
			Namespace: "",
			Tenant:    "",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "TRANSPARENT",
			FlowPolicyMode: "FLOWAWARE",
		},
	}
	_, err = it.restClient.ClusterV1().DSCProfile().Create(ctx, &transparentFlowaware)
	AssertOk(c, err, "Error Creating TestDSCProfile profile")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "transparent.flowaware")
		log.Infof("Profile found")
		if obj.DSCProfile.DSCProfile.Spec.FwdMode == cluster.DSCProfileSpec_TRANSPARENT.String() &&
			obj.DSCProfile.DSCProfile.Spec.FlowPolicyMode == cluster.DSCProfileSpec_FLOWAWARE.String() {
			log.Infof("Profile matched")

			return (nerr == nil), true
		}
		return false, false
	}, "Profile not found")

	// INSERTION.ENFORCED
	insertionEnforced := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "insertion.enforced",
			Namespace: "",
			Tenant:    "",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "INSERTION",
			FlowPolicyMode: "ENFORCED",
		},
	}

	_, err = it.restClient.ClusterV1().DSCProfile().Create(ctx, &insertionEnforced)
	AssertOk(c, err, "Error Creating TestDSCProfile profile")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "insertion.enforced")
		log.Infof("Profile found")
		if obj.DSCProfile.DSCProfile.Spec.FwdMode == cluster.DSCProfileSpec_INSERTION.String() &&
			obj.DSCProfile.DSCProfile.Spec.FlowPolicyMode == cluster.DSCProfileSpec_ENFORCED.String() {
			log.Infof("Profile matched")

			return (nerr == nil), true
		}
		return false, false
	}, "Profile not found")

	// Update DSC to TRANSPARENT.BASENET
	dscObj, err := it.getDistributedServiceCard(it.getNaplesMac(0))
	AssertOk(c, err, "Error DSC object not found")

	dscObj.Spec.DSCProfile = "transparent.basenet"

	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, dscObj)
	AssertOk(c, err, "Error DistributedServicesCard update failed")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "transparent.basenet")
		if _, ok := obj.DscList[it.getNaplesMac(0)]; ok {
			return (nerr == nil), true
		}
		return false, false
	}, "DSCProfile not update")
	log.Infof("Profile Updated Successfully")

	// TRANSPARENT.BASENET ===> TRANSPARENT.FLOWAWARE ==> EXP:ALLOW
	dscObj, err = it.getDistributedServiceCard(it.getNaplesMac(0))
	AssertOk(c, err, "Error DSC object not found")

	dscObj.Spec.DSCProfile = "transparent.flowaware"
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, dscObj)
	AssertOk(c, err, "Error DistributedServicesCard update failed")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "transparent.flowaware")
		if _, ok := obj.DscList[it.getNaplesMac(0)]; ok {
			return (nerr == nil), true
		}
		return false, false
	}, "DSCProfile not update")

	// TRANSPARENT.FLOWAWARE ===> TRANSPARENT.BASENET ==> EXP:FAIL
	dscObj, err = it.getDistributedServiceCard(it.getNaplesMac(0))
	AssertOk(c, err, "Error DSC object not found")

	dscObj.Spec.DSCProfile = "transparent.basenet"
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, dscObj)
	AssertOk(c, err, "Error DistributedServicesCard update occurred")

	// TRANSPARENT.FLOWAWARE ===> INSERTION.ENFORCED ==== > EXP: ALLOWED
	dscObj, err = it.getDistributedServiceCard(it.getNaplesMac(0))
	AssertOk(c, err, "Error DSC object not found")

	dscObj.Spec.DSCProfile = "insertion.enforced"
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, dscObj)
	AssertOk(c, err, "Error DistributedServicesCard update failed")

	AssertEventually(c, func() (bool, interface{}) {
		obj, nerr := it.ctrler.StateMgr.FindDSCProfile("", "insertion.enforced")
		if _, ok := obj.DscList[it.getNaplesMac(0)]; ok {
			return (nerr == nil), true
		}
		return false, false
	}, "DSCProfile not update")

	// INSERTION.ENFORCED ==> TRANSPARENT.FLOWAWARE  ====> EXP: FAIL
	dscObj, err = it.getDistributedServiceCard(it.getNaplesMac(0))
	AssertOk(c, err, "Error DSC object not found")

	dscObj.Spec.DSCProfile = "transparent.flowaware"

	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, dscObj)
	Assert(c, err != nil, "Error DistributedServicesCard update occurred")

	// INSERTION.ENFORCED ==> TRNASPARENT.BASENET    ====> EXP: FAIL
	dscObj, err = it.getDistributedServiceCard(it.getNaplesMac(0))
	AssertOk(c, err, "Error DSC object not found")

	dscObj.Spec.DSCProfile = "transparent.basenet"

	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, dscObj)
	Assert(c, err != nil, "Error DistributedServicesCard update occurred")

	// DELETE PROFILES
	/*_, err = it.restClient.ClusterV1().DSCProfile().Delete(ctx, &dscProfile.ObjectMeta)
	AssertOk(c, err, "Error deleting dscprofile")*/

}
