// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import "github.com/pensando/sw/api/generated/cluster"

// CompleteRegistration is the callback function statemgr calls after init is done
func (sm *Statemgr) CompleteRegistration() {
	sm.SetModuleReactor(sm)
	sm.SetTenantReactor(sm)
	sm.SetSecurityGroupReactor(sm)
	sm.SetAppReactor(sm)
	sm.SetNetworkReactor(sm)
	sm.SetFirewallProfileReactor(sm)
	sm.SetDistributedServiceCardReactor(sm)
	sm.SetHostReactor(sm)
	sm.SetEndpointReactor(sm)
	sm.SetNetworkSecurityPolicyReactor(sm)
	sm.SetWorkloadReactor(sm)
	sm.SetDSCProfileReactor(sm)

	sm.SetEndpointStatusReactor(sm)
	sm.SetSecurityProfileStatusReactor(sm)
	sm.SetNetworkSecurityPolicyStatusReactor(sm)
	sm.SetNetworkInterfaceStatusReactor(sm)
	sm.SetAggregateStatusReactor(sm)
	sm.SetProfileStatusReactor(sm)
}

//ProcessDSCEvent to process a DSC event
func (sm *Statemgr) ProcessDSCEvent(ev EventType, dsc *cluster.DistributedServiceCard) {

	for feature, svc := range featuremgrs {
		if feature != "statemgr" {
			svc.ProcessDSCEvent(ev, dsc)
		}
	}

}

func init() {
	mgr := MustGetStatemgr()
	mgr.Register("statemgr", mgr)
}
