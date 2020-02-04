// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"context"
	"errors"
	"fmt"
	"strings"
	"time"

	gogoproto "github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/dependencies"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	clientApi "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
)

// Nagent is an instance of network agent.
type Nagent types.NetAgent

// ErrObjectNotFound is returned when object is not found
var ErrObjectNotFound = errors.New("object not found")

// NewNetAgent creates a new network agent
func NewNetAgent(dp types.NetDatapathAPI, dbPath string, delphiClient clientApi.Client) (*Nagent, error) {
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
	na.Mode = "host-managed"
	na.DelphiClient = delphiClient
	if err := na.createDefaultTenant(); err != nil {
		log.Errorf("Failed to create default tenant. Err: %v", err)
	}

	if err := na.GetHwInterfaces(); err != nil {
		log.Errorf("Failed to program HW Interfaces. Err: %v", err)
	}

	if err := na.createDefaultVrf(); err != nil {
		log.Errorf("Failed to create default vrf. Err: %v", err)
	}

	if err := na.createDefaultUntaggedNw(); err != nil {
		log.Errorf("Failed to create default untagged network. Err: %v", err)
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

// GetWatchOptions returns the options to be used while establishing a watch from this agent.
func (na *Nagent) GetWatchOptions(cts context.Context, kind string) api.ListWatchOptions {
	var ret api.ListWatchOptions
	switch kind {
	case "Endpoint":
		str := fmt.Sprintf("spec.node-uuid=%s", na.NodeUUID)
		ret.FieldSelector = str
	}
	return ret
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
		Status: netproto.TenantStatus{
			TenantID: 1,
		},
	}
	return na.CreateTenant(&tn)
}

func (na *Nagent) createDefaultVrf() error {
	c, _ := gogoproto.TimestampProto(time.Now())
	defVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default",
			CreationTime: api.Timestamp{
				Timestamp: *c,
			},
			ModTime: api.Timestamp{
				Timestamp: *c,
			},
		},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: 65,
		},
	}
	err := na.CreateVrf(&defVrf)
	return err
}

func (na *Nagent) createDefaultUntaggedNw() error {
	c, _ := gogoproto.TimestampProto(time.Now())
	defNw := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      types.InternalUntaggedNetwork,
			CreationTime: api.Timestamp{
				Timestamp: *c,
			},
			ModTime: api.Timestamp{
				Timestamp: *c,
			},
		},
		Spec: netproto.NetworkSpec{
			VlanID: types.UntaggedCollVLAN, // Untagged
		},
	}
	err := na.CreateNetwork(&defNw)
	return err
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
	na.TenantDB = make(map[string]*netproto.Tenant)
	na.NamespaceDB = make(map[string]*netproto.Namespace)
	na.HwIfDB = make(map[string]*netproto.Interface)
	na.NetworkSecurityPolicyDB = make(map[string]*netproto.NetworkSecurityPolicy)
	na.TunnelDB = make(map[string]*netproto.Tunnel)
	na.PortDB = make(map[string]*netproto.Port)
	na.SecurityProfileDB = make(map[string]*netproto.SecurityProfile)
	na.AppDB = make(map[string]*netproto.App)
	na.VrfDB = make(map[string]*netproto.Vrf)
	na.IPAMPolicyDB = make(map[string]*netproto.IPAMPolicy)
	na.RoutingConfigDB = make(map[string]*netproto.RoutingConfig)
	na.Solver = dependencies.NewDepSolver()
	na.ArpCache = types.ArpCache{
		DoneCache: make(map[string]context.CancelFunc),
	}
	na.LateralDB = make(map[string][]string)
	dp.RegisterStateAPI(na)
}

// PurgeConfigs deletes all netagent configs. This is called during decommission workflow where the NAPLES is moved to host managed mode
func (na *Nagent) PurgeConfigs() error {
	// Perform ordered deletes of venice objects
	// Apps, SGPolicies, Endpoints,  Networks
	for _, app := range na.ListApp() {
		if err := na.DeleteApp(app.Tenant, app.Namespace, app.Name); err != nil {
			log.Errorf("Failed to delete the App. Err: %v", err)
		}
	}

	for _, sgp := range na.ListNetworkSecurityPolicy() {
		if err := na.DeleteNetworkSecurityPolicy(sgp.Tenant, sgp.Namespace, sgp.Name); err != nil {
			log.Errorf("Failed to delete the SG Policy. Err: %v", err)
		}
	}

	for _, ep := range na.ListEndpoint() {
		if strings.Contains(ep.Name, "_internal") {
			continue
		}
		if err := na.DeleteEndpoint(ep.Tenant, ep.Namespace, ep.Name); err != nil {
			log.Errorf("Failed to delete the endpoint. Err: %v", err)
		}
	}

	for _, nw := range na.ListNetwork() {
		if strings.Contains(nw.Name, "_internal") {
			continue
		}
		if err := na.DeleteNetwork(nw.Tenant, nw.Namespace, nw.Name); err != nil {
			log.Errorf("Failed to delete the network. Err: %v", err)
		}
	}

	for _, policy := range na.ListIPAMPolicy() {
		if err := na.DeleteIPAMPolicy(policy.Tenant, policy.Namespace, policy.Name); err != nil {
			log.Errorf("Failed to delete IPAM policy, err: %v", err)
		}
	}

	//for _, cfg := range na.ListRoutingConfig() {
	//	if err := na.DeleteRoutingConfig(cfg.Tenant, cfg.Namespace, cfg.Name); err != nil {
	//		log.Errorf("Failed to delete Routing config, err: %v", err)
	//	}
	//}
	return nil
}

// ReplayConfigs replays the persisted configs from bolt DB
func (na *Nagent) ReplayConfigs() error {
	// Replay Network Object
	networks, err := na.Store.RawList("Network")
	if err == nil {
		for _, o := range networks {
			var network netproto.Network
			err := network.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to Network. Err: %v", err)
				continue
			}
			creator, ok := network.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Info("Replaying persisted Network objects")
				if err := na.CreateNetwork(&network); err != nil {
					log.Errorf("Failed to recreate Network: %v. Err: %v", network.GetKey(), err)
				}
			}
		}
	}

	// Replay Endpoint Object
	endpoints, err := na.Store.RawList("Endpoint")
	if err == nil {
		for _, o := range endpoints {
			var endpoint netproto.Endpoint
			err := endpoint.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to Endpoint. Err: %v", err)
				continue
			}
			creator, ok := endpoint.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Info("Replaying persisted Endpoint objects")
				if err := na.CreateEndpoint(&endpoint); err != nil {
					log.Errorf("Failed to recreate Endpoint: %v. Err: %v", endpoint.GetKey(), err)
				}
			}
		}
	}

	// Replay Tunnel Object
	tunnels, err := na.Store.RawList("Tunnel")
	if err == nil {
		for _, o := range tunnels {
			var tunnel netproto.Tunnel
			err := tunnel.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to Tunnel. Err: %v", err)
				continue
			}
			creator, ok := tunnel.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Info("Replaying persisted Tunnel objects")
				if err := na.CreateTunnel(&tunnel); err != nil {
					log.Errorf("Failed to recreate Tunnel: %v. Err: %v", tunnel.GetKey(), err)
				}
			}
		}
	}

	// Replay App Object
	apps, err := na.Store.RawList("App")
	if err == nil {
		for _, o := range apps {
			var app netproto.App
			err := app.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to App. Err: %v", err)
				continue
			}
			creator, ok := app.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Info("Replaying persisted App objects")
				if err := na.CreateApp(&app); err != nil {
					log.Errorf("Failed to recreate App: %v. Err: %v", app.GetKey(), err)
				}
			}
		}
	}

	// Replay NetworkSecurityPolicy Object
	policies, err := na.Store.RawList("NetworkSecurityPolicy")
	if err == nil {
		for _, o := range policies {
			var sgp netproto.NetworkSecurityPolicy
			err := sgp.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to NetworkSecurityPolicy. Err: %v", err)
				continue
			}
			creator, ok := sgp.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Info("Replaying persisted NetworkSecurityPolicy objects")
				if err := na.CreateNetworkSecurityPolicy(&sgp); err != nil {
					log.Errorf("Failed to recreate NetworkSecurityPolicy: %v. Err: %v", sgp.GetKey(), err)
				}
			}
		}
	}

	// Replay IPAMPolicy Objects
	ipamPolicies, err := na.Store.RawList("IPAMPolicy")
	if err == nil {
		for _, o := range ipamPolicies {
			var policy netproto.IPAMPolicy
			err := policy.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to IPAMPolicy. Err: %v", err)
				continue
			}
			creator, ok := policy.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Infof("Replaying persisted IPAMPolicy object: %+v", policy)
				if err := na.CreateIPAMPolicy(&policy); err != nil {
					log.Errorf("Failed to recreate IPAMPolicy: %v. Err: %v", policy.GetKey(), err)
				}
			}
		}
	}

	//// Replay RoutingConfig Objects
	//rtcfgs, err := na.Store.RawList("RoutingCofnig")
	//if err == nil {
	//	for _, o := range rtcfgs {
	//		var cfg netproto.RoutingConfig
	//		err := cfg.Unmarshal(o)
	//		if err != nil {
	//			log.Errorf("Failed to unmarshal object to RoutingConfig. Err: %v", err)
	//			continue
	//		}
	//		creator, ok := cfg.ObjectMeta.Labels["CreatedBy"]
	//		if ok && creator == "Venice" {
	//			log.Infof("Replaying persisted RoutingConfig object: %+v", cfg)
	//			if err := na.CreateRoutingConfig(&cfg); err != nil {
	//				log.Errorf("Failed to recreate RoutingConfig: %v. Err: %v", cfg.GetKey(), err)
	//			}
	//		}
	//	}
	//}
	return nil
}
