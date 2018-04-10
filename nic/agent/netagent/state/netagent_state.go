// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"fmt"

	"github.com/pensando/sw/api"
	config "github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
)

// NewNetAgent creates a new network agent
func NewNetAgent(dp NetDatapathAPI, mode config.AgentMode, dbPath, nodeUUID string) (*NetAgent, error) {
	var emdb emstore.Emstore
	var err error

	if dbPath == "" {
		emdb, err = emstore.NewEmstore(emstore.MemStoreType, "")
	} else {
		emdb, err = emstore.NewEmstore(emstore.BoltDBType, dbPath)
	}
	if err != nil {
		log.Errorf("Error opening the embedded db. Err: %v", err)
		return nil, err
	}

	nagent := NetAgent{
		store:       emdb,
		nodeUUID:    nodeUUID,
		datapath:    dp,
		networkDB:   make(map[string]*netproto.Network),
		endpointDB:  make(map[string]*netproto.Endpoint),
		secgroupDB:  make(map[string]*netproto.SecurityGroup),
		tenantDB:    make(map[string]*netproto.Tenant),
		interfaceDB: make(map[string]*netproto.Interface),
	}

	c := config.Agent{
		ObjectMeta: api.ObjectMeta{
			Name: "AgentConfig",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Agent",
		},
		Spec: config.AgentSpec{
			Mode: mode,
		},
	}

	_, err = emdb.Read(&c)

	// Blank slate. Persist config and do init stuff
	if err != nil {
		err := emdb.Write(&c)
		if err != nil {
			emdb.Close()
			return nil, err
		}
		// We need to create a default tenant at startup as HAL expects an actual tenant object to be present before any api calls
		err = nagent.createDefaultTenant()
		if err != nil {
			emdb.Close()
			return nil, err

		}
	}

	err = nagent.GetHwInterfaces()
	if err != nil {
		return nil, err
	}

	err = dp.SetAgent(&nagent)
	if err != nil {
		// cleanup emstore and return
		emdb.Close()
		return nil, err
	}

	return &nagent, nil

}

// RegisterCtrlerIf registers a controller object
func (na *NetAgent) RegisterCtrlerIf(ctrlerif CtrlerAPI) error {
	// ensure two controller plugins dont register
	if na.ctrlerif != nil {
		log.Fatalf("Multiple controllers registers to netagent.")
	}

	// add it to controller list
	na.ctrlerif = ctrlerif

	return nil
}

// Stop stops the netagent
func (na *NetAgent) Stop() error {
	return na.store.Close()
}

// objectKey returns object key from object meta
func objectKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// GetAgentID returns UUID of the agent
func (na *NetAgent) GetAgentID() string {
	return na.nodeUUID
}

func (na *NetAgent) createDefaultTenant() error {
	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name:   "default",
			Tenant: "default",
		},
	}
	return na.CreateTenant(&tn)
}
