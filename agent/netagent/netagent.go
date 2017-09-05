// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netagent

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/utils/emstore"
	"github.com/pensando/sw/utils/log"
)

// NetAgent is the network agent instance
type NetAgent struct {
	sync.Mutex                                          // global lock for the agent
	store            emstore.Emstore                    // embedded db
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
func NewNetAgent(dp NetDatapathAPI, dbPath, nodeUUID string) (*NetAgent, error) {
	var emdb emstore.Emstore
	var err error

	// open the embedded database
	if dbPath == "" {
		emdb, err = emstore.NewEmstore(emstore.MemStoreType, "")
	} else {
		emdb, err = emstore.NewEmstore(emstore.BoltDBType, dbPath)
	}
	if err != nil {
		log.Errorf("Error opening the embedded db. Err: %v", err)
		return nil, err
	}

	// create netagent object
	agent := NetAgent{
		store:            emdb,
		NodeUUID:         nodeUUID,
		datapath:         dp,
		networkDB:        make(map[string]*netproto.Network),
		endpointDB:       make(map[string]*netproto.Endpoint),
		secgroupDB:       make(map[string]*netproto.SecurityGroup),
		currentNetworkID: 1,
		currentSgID:      1,
	}

	// register the agent with datapath
	err = dp.SetAgent(&agent)
	if err != nil {
		// cleanup emstore and return
		emdb.Close()
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

// Stop stops the netagent
func (ag *NetAgent) Stop() error {
	return ag.store.Close()
}

// objectKey returns object key from object meta
func objectKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// GetAgentID returns UUID of the agent
func (ag *NetAgent) GetAgentID() string {
	return ag.NodeUUID
}
