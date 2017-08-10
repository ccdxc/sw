// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/utils/log"
)

// NetAgent is the network agent instance
type NetAgent struct {
	sync.Mutex                                          // global lock for the agent
	NodeUUID         string                             // Node's UUID
	datapath         NetDatapathAPI                     // network datapath
	ctrlerif         CtrlerAPI                          // controller object
	networkDB        map[string]*netproto.Network       // Network object db
	endpointDB       map[string]*netproto.Endpoint      // Endpoint object db
	secgroupDB       map[string]*netproto.SecurityGroup // security group object db
	currentNetworkID uint32                             // poor man's id allocation FIXME:
	currentSgID      uint32                             // poor man's id allocation FIXME:
}

// NewNetAgent returns a new network agent
func NewNetAgent(dp NetDatapathAPI, nodeUUID string) (*NetAgent, error) {
	// create netagent object
	agent := NetAgent{
		NodeUUID:         nodeUUID,
		datapath:         dp,
		networkDB:        make(map[string]*netproto.Network),
		endpointDB:       make(map[string]*netproto.Endpoint),
		secgroupDB:       make(map[string]*netproto.SecurityGroup),
		currentNetworkID: 1,
		currentSgID:      1,
	}

	// register the agent with datapath
	err := dp.SetAgent(&agent)
	if err != nil {
		return nil, err
	}

	return &agent, nil
}

// RegisterCtrlerIf registers a controller object
func (ag *NetAgent) RegisterCtrlerIf(ctrlerif CtrlerAPI) error {
	// ensure two controller plugins dont register
	if ag.ctrlerif != nil {
		log.Fatalf("Multiple controllers registers to netagent.")
	}

	// add it to controller list
	ag.ctrlerif = ctrlerif

	return nil
}

// objectKey returns object key from object meta
func objectKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}
