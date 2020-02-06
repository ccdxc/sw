// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package base

import (
	//"fmt"
	//"strconv"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
)

func (sm *SysModel) NewFirewallProfile(name string) *objects.FirewallProfileCollection {
	return objects.NewFirewallProfile(name, sm.ObjClient(), sm.Tb)
}

/*
func (sm *SysModel) FirewallProfiles() *objects.FirewallProfileCollection {
	fwp := objects.FirewallProfileCollection{}
	for _, profile := range fwp.Profiles {
		fwp.Profiles = append(fwp.Profiles, profile)
	}

	return &fwp
}
*/
