// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package enterprise

import "github.com/pensando/sw/iota/test/venice/iotakit/model/objects"

// SGPolicies returns all SGPolicies in the model
func (sm *SysModel) SGPolicies() *objects.NetworkSecurityPolicyCollection {
	spc := objects.NetworkSecurityPolicyCollection{}
	for _, pol := range sm.sgpolicies {
		spc.Policies = append(spc.Policies, pol)
	}

	return &spc
}
