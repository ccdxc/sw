// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"fmt"
	"time"

	gogoproto "github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	config "github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/nic/agent/netagent/state/dependencies"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
)

// Nagent is an instance of network agent.
type Nagent types.NetAgent

// NewNetAgent creates a new network agent
func NewNetAgent(dp types.NetDatapathAPI, mode config.AgentMode, dbPath string) (*Nagent, error) {
	var na Nagent
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

	na.init(emdb, dp)

	na.Mode = mode.String()

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
		// We need to create a default tenant and default namespace at startup.
		err = na.createDefaultTenant()
		if err != nil {
			emdb.Close()
			return nil, err

		}
		// We need to create an infra namespace at startup. This will create an infra vrf in the datapath
		err = na.createInfraNamespace()
		if err != nil {
			emdb.Close()
			return nil, err

		}
	}
	err = na.GetUUID()
	err = na.GetHwInterfaces()
	if err != nil {
		return nil, err
	}

	err = dp.SetAgent(&na)
	if err != nil {
		// cleanup emstore and return
		emdb.Close()
		return nil, err
	}

	return &na, nil

}

// RegisterCtrlerIf registers a controller object
func (na *Nagent) RegisterCtrlerIf(ctrlerif types.CtrlerAPI) error {
	// ensure two controller plugins dont register
	if na.Ctrlerif != nil {
		log.Fatalf("Multiple controllers registers to netagent.")
	}

	// add it to controller list
	na.Ctrlerif = ctrlerif

	return nil
}

// Stop stops the netagent
func (na *Nagent) Stop() error {
	return na.Store.Close()
}

// GetAgentID returns UUID of the agent
func (na *Nagent) GetAgentID() string {
	return na.NodeUUID
}

func (na *Nagent) createDefaultTenant() error {
	c, _ := gogoproto.TimestampProto(time.Now())

	tn := netproto.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "default",
			CreationTime: api.Timestamp{
				Timestamp: *c,
			},
			ModTime: api.Timestamp{
				Timestamp: *c,
			},
		},
	}
	return na.CreateTenant(&tn)
}

func (na *Nagent) createInfraNamespace() error {
	c, _ := gogoproto.TimestampProto(time.Now())

	infraNS := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Name:   "infra",
			Tenant: "default",
			CreationTime: api.Timestamp{
				Timestamp: *c,
			},
			ModTime: api.Timestamp{
				Timestamp: *c,
			},
		},
		Spec: netproto.NamespaceSpec{
			NamespaceType: "INFRA",
		},
	}
	return na.CreateNamespace(&infraNS)
}

func (na *Nagent) validateMeta(kind string, oMeta api.ObjectMeta) error {
	if len(oMeta.Name) == 0 {
		return fmt.Errorf("%s name can't be empty", kind)
	}
	return nil
}

func (na *Nagent) init(emdb emstore.Emstore, dp types.NetDatapathAPI) {
	na.Store = emdb
	na.Datapath = dp
	na.NetworkDB = make(map[string]*netproto.Network)
	na.EndpointDB = make(map[string]*netproto.Endpoint)
	na.SecgroupDB = make(map[string]*netproto.SecurityGroup)
	na.TenantDB = make(map[string]*netproto.Tenant)
	na.NamespaceDB = make(map[string]*netproto.Namespace)
	na.EnicDB = make(map[string]*netproto.Interface)
	na.NatPoolDB = make(map[string]*netproto.NatPool)
	na.NatPoolLUT = make(map[string]*types.NatPoolRef)
	na.NatPolicyDB = make(map[string]*netproto.NatPolicy)
	na.NatBindingDB = make(map[string]*netproto.NatBinding)
	na.HwIfDB = make(map[string]*netproto.Interface)
	na.RouteDB = make(map[string]*netproto.Route)
	na.IPSecPolicyDB = make(map[string]*netproto.IPSecPolicy)
	na.IPSecSAEncryptDB = make(map[string]*netproto.IPSecSAEncrypt)
	na.IPSecSADecryptDB = make(map[string]*netproto.IPSecSADecrypt)
	na.IPSecPolicyLUT = make(map[string]*types.IPSecRuleRef)
	na.SGPolicyDB = make(map[string]*netproto.SGPolicy)
	na.TunnelDB = make(map[string]*netproto.Tunnel)
	na.TCPProxyPolicyDB = make(map[string]*netproto.TCPProxyPolicy)
	na.PortDB = make(map[string]*netproto.Port)
	na.SecurityProfileDB = make(map[string]*netproto.SecurityProfile)
	na.AppDB = make(map[string]*netproto.App)
	na.Solver = dependencies.NewDepSolver()
}

// GetUUID gets the naples uuid from the datapath
func (na *Nagent) GetUUID() error {

	uuid, err := na.Datapath.GetUUID()
	if err != nil {
		log.Errorf("HAL System GetUUID failed. %v", err)
		return fmt.Errorf("hal get fru uuid failed. %v", err)
	}

	na.NodeUUID = uuid
	log.Infof("Got UUID:. %v", na.NodeUUID)
	return nil
}
