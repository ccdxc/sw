// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package enterprise

import (
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

// Networks returns a list of subnets
func (sm *SysModel) Networks() *objects.NetworkCollection {
	snc := objects.NetworkCollection{}
	nws, err := sm.CfgModel.ListNetwork()
	if err != nil {
		log.Errorf("Error listing networks %v", err)
		return nil
	}
	for _, sn := range nws {
		snc.AddSubnet(&objects.Network{VeniceNetwork: sn})
	}

	return &snc
}
