// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"context"
	"encoding/binary"
	"errors"
	"fmt"
	"math"
	"math/rand"
	"os"
	"strconv"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/featureflags"
	memdb "github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/ref"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

func stringSliceEqual(X, Y []string) bool {
	m := make(map[string]int)

	for _, y := range Y {
		m[y]++
	}

	for _, x := range X {
		if m[x] > 0 {
			m[x]--
			continue
		}
		//not present or execess
		return false
	}

	return len(m) == 0
}

// createRoutingConfig utility function to create a routing config
func createRoutingConfig(stateMgr *Statemgr, tenant, name, IP string) (*network.RoutingConfig, error) {
	rtcfg := network.RoutingConfig{
		TypeMeta: api.TypeMeta{Kind: "RoutingConfig"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    tenant,
		},
		Spec: network.RoutingConfigSpec{
			BGPConfig: &network.BGPConfig{
				RouterId: "1.1.1.1",
				ASNumber: 100,
			},
			EVPNConfig: &network.EVPNConfig{
				Shutdown: false,
			},
		},
		Status: network.RoutingConfigStatus{},
	}

	cfg := &network.BGPNeighbor{
		Shutdown:              false,
		IPAddress:             IP,
		RemoteAS:              1,
		MultiHop:              1,
		EnableAddressFamilies: []string{"IPV4", "EVPN"},
	}

	rtcfg.Spec.BGPConfig.Neighbors = append(rtcfg.Spec.BGPConfig.Neighbors, cfg)

	err := stateMgr.ctrler.RoutingConfig().Create(&rtcfg)
	return &rtcfg, err
}

// createIPAMPolicy utility function to create an IPAM Policy
func createIPAMPolicy(stateMgr *Statemgr, tenant, name, dhcpIP string) (*network.IPAMPolicy, error) {
	policy := network.IPAMPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPAMPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    tenant,
		},
		Spec: network.IPAMPolicySpec{
			DHCPRelay: &network.DHCPRelayPolicy{},
		},
		Status: network.IPAMPolicyStatus{},
	}

	server := &network.DHCPServer{
		IPAddress:     dhcpIP,
		VirtualRouter: "default",
	}

	policy.Spec.DHCPRelay.Servers = append(policy.Spec.DHCPRelay.Servers, server)

	err := stateMgr.ctrler.IPAMPolicy().Create(&policy)
	return &policy, err
}

// createNetwork utility function to create a network
func createNetwork(t *testing.T, stateMgr *Statemgr, tenant, net, subnet, gw string) error {
	// network params
	np := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      net,
			Namespace: "default",
			Tenant:    tenant,
		},
		Spec: network.NetworkSpec{
			Type:        network.NetworkType_Bridged.String(),
			IPv4Subnet:  subnet,
			IPv4Gateway: gw,
		},
		Status: network.NetworkStatus{},
	}

	// create a network
	return stateMgr.ctrler.Network().Create(&np)

}

func createSg(stateMgr *Statemgr, tenant, sgname string, selector *labels.Selector) (*security.SecurityGroup, error) {
	// sg object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: "default",
			Name:      sgname,
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: selector,
		},
	}

	// create sg
	err := stateMgr.ctrler.SecurityGroup().Create(&sg)

	return &sg, err
}

func updateSg(stateMgr *Statemgr, tenant, sgname string, selector *labels.Selector) (*security.SecurityGroup, error) {
	// sg object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: "default",
			Name:      sgname,
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: selector,
		},
	}

	// create sg
	err := stateMgr.ctrler.SecurityGroup().Update(&sg)

	return &sg, err
}

func deleteSg(stateMgr *Statemgr, tenant, sgname string, selector *labels.Selector) (*security.SecurityGroup, error) {
	// sg object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: "default",
			Name:      sgname,
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: selector,
		},
	}

	// create sg
	err := stateMgr.ctrler.SecurityGroup().Delete(&sg)

	return &sg, err
}

// createTenant utility function to create a tenant
func createTenant(t *testing.T, stateMgr *Statemgr, tenant string) error {
	// network params
	tn := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: tenant,
		},
	}

	// create a network
	stateMgr.ctrler.Tenant().Create(&tn)
	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindTenant(tenant)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Tenant not foud", "1ms", "1s")

	return nil
}

// createEndpoint utility function to create an endpoint
func createEndpoint(stateMgr *Statemgr, tenant, endpoint, net string) (*workload.Endpoint, error) {
	// network params
	ep := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      endpoint,
			Namespace: "default",
			Tenant:    tenant,
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			WorkloadName:   "testContainerName",
			Network:        net,
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}

	return &ep, stateMgr.ctrler.Endpoint().Create(&ep)
}

func createMirror(stateMgr *Statemgr, tenant, mirrorName string, collectors []monitoring.MirrorCollector, selector *labels.Selector) (*monitoring.MirrorSession, error) {
	mr := monitoring.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: "default",
			Name:      mirrorName,
		},
		Spec: monitoring.MirrorSessionSpec{
			InterfaceSelector: selector,
			Collectors:        collectors,
		},
	}

	// create sg
	err := stateMgr.ctrler.MirrorSession().Create(&mr)

	return &mr, err
}

func updateMirror(stateMgr *Statemgr, tenant, mirrorName string, collectors []monitoring.MirrorCollector, selector *labels.Selector) (*monitoring.MirrorSession, error) {
	mr := monitoring.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: "default",
			Name:      mirrorName,
		},
		Spec: monitoring.MirrorSessionSpec{
			InterfaceSelector: selector,
			Collectors:        collectors,
		},
	}

	err := stateMgr.ctrler.MirrorSession().Update(&mr)

	return &mr, err
}

func deleteMirror(stateMgr *Statemgr, tenant, mirrorName string, selector *labels.Selector) (*monitoring.MirrorSession, error) {
	mr := monitoring.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: "default",
			Name:      mirrorName,
		},
		Spec: monitoring.MirrorSessionSpec{
			InterfaceSelector: selector,
		},
	}

	// create sg
	err := stateMgr.ctrler.MirrorSession().Delete(&mr)

	return &mr, err
}

func createNetworkInterface(stateMgr *Statemgr, intfName string, dsc string, label map[string]string) (*network.NetworkInterface, error) {
	nr := network.NetworkInterface{
		TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
		ObjectMeta: api.ObjectMeta{
			Name:   intfName,
			Labels: label,
		},
		Spec:   network.NetworkInterfaceSpec{},
		Status: network.NetworkInterfaceStatus{DSC: dsc},
	}

	// create sg
	err := stateMgr.ctrler.NetworkInterface().Create(&nr)

	return &nr, err
}

func updateNetworkInterface(stateMgr *Statemgr, intfName string, dsc string, label map[string]string) (*network.NetworkInterface, error) {
	nr := network.NetworkInterface{
		TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
		ObjectMeta: api.ObjectMeta{
			Name:   intfName,
			Labels: label,
		},
		Spec:   network.NetworkInterfaceSpec{},
		Status: network.NetworkInterfaceStatus{DSC: dsc},
	}

	// create sg
	err := stateMgr.ctrler.NetworkInterface().Update(&nr)

	return &nr, err
}

func deleteNetworkInterface(stateMgr *Statemgr, intfName string, label map[string]string) (*network.NetworkInterface, error) {
	nr := network.NetworkInterface{
		TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
		ObjectMeta: api.ObjectMeta{
			Name:   intfName,
			Labels: label,
		},
		Spec: network.NetworkInterfaceSpec{},
	}

	// create sg
	err := stateMgr.ctrler.NetworkInterface().Delete(&nr)

	return &nr, err
}

// createVirtualRouter utility function to create a VirtualRouter
func createVirtualRouter(t *testing.T, stateMgr *Statemgr, tenant, virtualRouter string) (*network.VirtualRouter, error) {
	// virtual router params
	vr := network.VirtualRouter{
		TypeMeta: api.TypeMeta{Kind: "VirtualRouter"},
		ObjectMeta: api.ObjectMeta{
			Name:      virtualRouter,
			Namespace: "default",
			Tenant:    tenant,
		},
		Spec: network.VirtualRouterSpec{
			Type:             "Tenant",
			RouterMACAddress: "aaaa.bbbb.cccc",
			VxLanVNI:         10000,
			RouteImportExport: &network.RDSpec{
				AddressFamily: network.BGPAddressFamily_EVPN.String(),
				RDAuto:        false,
				ExportRTs: []*network.RouteDistinguisher{
					{
						Type:          network.RouteDistinguisher_Type0.String(),
						AssignedValue: 100,
						AdminValue:    200,
					},
				},
				ImportRTs: []*network.RouteDistinguisher{
					{
						Type:          network.RouteDistinguisher_Type0.String(),
						AssignedValue: 200,
						AdminValue:    300,
					},
					{
						Type:          network.RouteDistinguisher_Type0.String(),
						AssignedValue: 400,
						AdminValue:    500,
					},
				},
			},
		},
		Status: network.VirtualRouterStatus{},
	}

	vrs := &VirtualRouterState{}
	ctVr := &ctkit.VirtualRouter{
		VirtualRouter: vr,
		HandlerCtx:    vrs,
	}
	vrs.VirtualRouter = ctVr
	// Test convert
	nvr := convertVirtualRouter(vrs)
	log.Infof("Converted VRF is [%+v]", nvr)
	// create a virtual router
	stateMgr.ctrler.VirtualRouter().Create(&vr)
	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindVirtualRouter(tenant, "default", virtualRouter)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error finding virtualrouter %v\n", err)
		return false, nil
	}, "virtual router not foud", "1ms", "1s")

	return &vr, nil
}

// TestNetworkCreateDelete tests network create
func TestNetworkCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")
	nw, err := stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")
	Assert(t, (nw.Network.Spec.IPv4Subnet == "10.1.1.0/24"), "Network subnet did not match", nw)
	Assert(t, (nw.Network.Spec.IPv4Gateway == "10.1.1.254"), "Network gateway did not match", nw)
	Assert(t, (nw.Network.TypeMeta.Kind == "Network"), "Network type meta did not match", nw)

	verifyFail := func() {
		AssertEventually(t, func() (bool, interface{}) {
			_, err := stateMgr.FindNetwork("default", "default")
			if err != nil {
				return true, nil
			}
			return true, nil
		}, "Network not found as expected", "1ms", "1s")
	}

	// verify can not create networks with invalid params
	err = createNetwork(t, stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.255")
	verifyFail()
	err = createNetwork(t, stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.0")
	verifyFail()
	err = createNetwork(t, stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.2.254")
	verifyFail()
	err = createNetwork(t, stateMgr, "default", "invalid", "10.1.1.0/24", "10.1.1.25424")
	verifyFail()
	err = createNetwork(t, stateMgr, "default", "invalid", "10.1.1.10-20/24", "10.1.1.254")
	verifyFail()
	verifyFail()

	// verify we cant create duplicate networks
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Network update failed")
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// delete the network
	err = stateMgr.ctrler.Network().Delete(&nw.Network.Network)
	AssertOk(t, err, "Failed to delete the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err != nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Network not deleted", "1ms", "1s")

}

func TestNetworkList(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")
	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// verify the list works
	nets, err := stateMgr.ListNetworks()
	AssertOk(t, err, "Error listing networks")
	Assert(t, (len(nets) == 1), "Incorrect number of networks received", nets)
	Assert(t, (nets[0].Network.Name == "default"), "Incorrect network received", nets[0])
}

// TestEndpointCreateDelete tests network create
func TestEndpointCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")
	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint params
	epinfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			WorkloadName:   "testContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}

	// find the network
	_, err = stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	// create endpoint
	err = stateMgr.ctrler.Endpoint().Create(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", epinfo)

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not created", "1ms", "1s")

	eps, err := stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err == nil), "Error finding the endpoint", epinfo)
	Assert(t, (eps.Endpoint.TypeMeta.Kind == "Endpoint"), "Endpoint type meta did not match", eps)
	//Assert(t, (eps.Endpoint.Status.IPv4Address == "10.1.1.1/24"), "Endpoint address did not match", eps)
	//Assert(t, (eps.Endpoint.Status.IPv4Gateway == "10.1.1.254"), "Endpoint gateway did not match", eps)

	// verify you cant create duplicate endpoints
	//epinfo.Status.IPv4Address = "10.1.1.5"
	//err = stateMgr.ctrler.Endpoint().Create(&epinfo)
	//AssertOk(t, err, "updating endpoint failed")

	// verify creating new EP after ip addr have run out fails
	newEP := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "newEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			WorkloadName:   "newContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}

	err = stateMgr.ctrler.Endpoint().Create(&newEP)
	AssertOk(t, err, "Endpoint failed")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "newEndpoint")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not created", "1ms", "1s")

	//nep, err := stateMgr.FindEndpoint("default", "newEndpoint")
	//AssertOk(t, err, "Error finding the endpoint")
	//Assert(t, (nep.Endpoint.Status.IPv4Address == "10.1.1.2/24"), "Endpoint address did not match", nep)

	//hack to increase coverage
	stateMgr.GetConfigPushStatus()
	stateMgr.GetConfigPushStats()
	stateMgr.ResetConfigPushStats()
	// delete the endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&epinfo)
	Assert(t, (err == nil), "Error deleting the endpoint", epinfo)

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not deleted", "1ms", "1s")

	// verify endpoint is gone from the database
	_, err = stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err != nil), "Deleted endpoint still found in network db", "testEndpoint")

}

func timeTrack(start time.Time, name string) time.Duration {
	elapsed := time.Since(start)
	fmt.Printf("%s took %s\n", name, elapsed)
	return elapsed
}

func TestEndpointStaleDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
			Conditions: []cluster.DSCCondition{
				cluster.DSCCondition{
					Status: cluster.ConditionStatus_TRUE.String(),
					Type:   cluster.DSCCondition_HEALTHY.String()},
			},
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	err = stateMgr.ctrler.DistributedServiceCard().Update(&snic)
	AssertOk(t, err, "Could not update the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: "testHost",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	netwrk := &network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      "network100",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: network.NetworkSpec{
			Type:   network.NetworkType_Bridged.String(),
			VlanID: 100,
		},
	}

	// create network
	err = stateMgr.ctrler.Network().Create(netwrk)
	AssertOk(t, err, "Could not create the network")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindHost("", "testHost")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
				{
					MACAddress:   "0001.0203.0406",
					MicroSegVlan: 100,
					Network:      "network100",
				},
			},
		},
	}

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// verify we can find the network for the workload
	nw, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
	AssertOk(t, err, "Could not find the network")

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")
	// verify we can find the endpoint associated with the workload
	foundEp, ok := nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, ok, "Could not find the endpoint", "testWorkload-0001.0203.0405")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	//hack to increase coverage
	stateMgr.GetConfigPushStatus()
	stateMgr.GetConfigPushStats()
	stateMgr.ResetConfigPushStats()

	//Delete a a host
	err = stateMgr.ctrler.Host().Delete(&host)
	//Assert(t, err == nil, "Error deleting the host")

	//obj, err1 := stateMgr.FindObject("Network", "default", "default", "Network-Vlan-1")
	//Assert(t, err1 == nil, "Error finding the network")
	err = stateMgr.ctrler.Network().Delete(&nw.Network.Network)
	Assert(t, err == nil, "Error deleting the network")

	// delete the workload and it will fail to trigger endpoint delete.
	err = stateMgr.ctrler.Workload().Delete(&wr)
	Assert(t, err == nil, "Error deleting the workload")

	//stateMgr.ctrler.Endpoint.Workload.Delete()
	err = stateMgr.RemoveStaleEndpoints()
	Assert(t, (err == nil), "Network still found after its deleted")

	// verify endpoint is gone from the database
	obj1, _ := stateMgr.FindEndpoint("", "testWorkload-0001.0203.0405")
	Assert(t, (obj1 == nil), "Deleted endpoint still found in network db", "testWorkload-0001.0203.0405")
}

func TestEndpointCreateFailure(t *testing.T) {
	t.Skip("Skipping as we commented out IP address allocation in workload create")
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/30", "10.1.1.2")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint params
	epinfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Status: workload.EndpointStatus{
			WorkloadName:   "testContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}

	// create endpoint
	err = stateMgr.ctrler.Endpoint().Create(&epinfo)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")
	_, err = stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err == nil), "Error finding the endpoint", epinfo)
	//Assert(t, (eps.Endpoint.Status.IPv4Address == "10.1.1.1/30"), "Endpoint address did not match", eps)

	// verify creating new EP after ip addr have run out fails
	newEP := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "newEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Status: workload.EndpointStatus{
			WorkloadName:   "newContainerName",
			Network:        "default",
			HomingHostAddr: "192.168.1.1",
			HomingHostName: "testHost",
		},
	}
	err = stateMgr.ctrler.Endpoint().Create(&newEP)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "newEndpoint")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")
	_, err = stateMgr.FindEndpoint("default", "newEndpoint")
	Assert(t, (err != nil), "Endpoint creation should have failed", newEP)

	// delete the first endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&epinfo)
	Assert(t, (err == nil), "Error deleting the endpoint", epinfo)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", epinfo.Name)
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	// verify now the creation goes thru
	err = stateMgr.ctrler.Endpoint().Create(&newEP)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", newEP.Name)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")
	AssertOk(t, err, "Second endpoint creation failed")
	_, err = stateMgr.FindEndpoint("default", newEP.Name)
	Assert(t, (err == nil), "Error finding the endpoint", newEP)
	//Assert(t, (nep.Endpoint.Status.IPv4Address == "10.1.1.1/30"), "Endpoint address did not match", nep)
}

// TestEndpointListWatch tests ep watch and list
func TestEndpointList(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint params
	epinfo := ctkit.Endpoint{
		Endpoint: workload.Endpoint{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Name:      "testEndpoint",
				Namespace: "default",
				Tenant:    "default",
			},
			Spec: workload.EndpointSpec{},
			Status: workload.EndpointStatus{
				WorkloadName:   "testContainerName",
				Network:        "default",
				HomingHostAddr: "192.168.1.1",
				HomingHostName: "testHost",
			},
		},
	}

	// create endpoint
	err = stateMgr.OnEndpointCreate(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", &epinfo)

	// verify the list works
	nets, err := stateMgr.ListNetworks()
	AssertOk(t, err, "Error listing networks")
	Assert(t, (len(nets) == 1), "Incorrect number of networks received", nets)
	for _, nt := range nets {
		endps := nt.ListEndpoints()
		Assert(t, (len(endps) == 1), "Invalid number of endpoints received", endps)
		Assert(t, (endps[0].Endpoint.Name == epinfo.Endpoint.Name), "Invalid endpoint params received", endps[0])
	}

}

func TestEndpointNodeState(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint params
	epinfo := ctkit.Endpoint{
		Endpoint: workload.Endpoint{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Name:      "testEndpoint",
				Namespace: "default",
				Tenant:    "default",
			},
			Spec: workload.EndpointSpec{},
			Status: workload.EndpointStatus{
				WorkloadName:   "testContainerName",
				Network:        "default",
				HomingHostAddr: "192.168.1.1",
				HomingHostName: "testHost",
			},
		},
	}

	// create endpoint
	err = stateMgr.OnEndpointCreate(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", &epinfo)

	//nep, err := stateMgr.mbus.FindEndpoint(&epinfo.ObjectMeta)
	//AssertOk(t, err, "Error finding endpoint in mbus")

	// trigger node state add
	//err = stateMgr.OnEndpointOperUpdate("node-1", &epinfo)
	//AssertOk(t, err, "Error adding node status")

	// trigger node state delete
	//err = stateMgr.OnEndpointOperDelete("node-1", &epinfo)
	//AssertOk(t, err, "Error deleting node status")

}

func TestSgCreateDelete(t *testing.T) {
	t.Skip("SG is not implemented")
	//// create network state manager
	//stateMgr, err := newStatemgr()
	//if err != nil {
	//	t.Fatalf("Could not create network manager. Err: %v", err)
	//	return
	//}
	//
	//// create tenant
	//err = createTenant(t, stateMgr, "default")
	//AssertOk(t, err, "Error creating the tenant")
	//
	//// create sg
	//sg, err := createSg(stateMgr, "default", "testSg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	//AssertOk(t, err, "Error creating security group")
	//
	//AssertEventually(t, func() (bool, interface{}) {
	//	_, err := stateMgr.FindSecurityGroup("default", "testSg")
	//	if err == nil {
	//		return true, nil
	//	}
	//	return false, nil
	//}, "Sg not found", "1ms", "1s")
	//
	//sg, err = updateSg(stateMgr, "default", "testSg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	//AssertOk(t, err, "Error creating security group")
	//
	//// verify we can find the sg
	//sgs, err := stateMgr.FindSecurityGroup("default", "testSg")
	//AssertOk(t, err, "Could not find the security group")
	//AssertEquals(t, sgs.SecurityGroup.Spec.WorkloadSelector.String(), sg.Spec.WorkloadSelector.String(), "Security group params did not match")
	//
	//// delete the security group
	//err = stateMgr.ctrler.SecurityGroup().Delete(&sgs.SecurityGroup.SecurityGroup)
	//AssertOk(t, err, "Error deleting security group")
	//
	//AssertEventually(t, func() (bool, interface{}) {
	//	_, err := stateMgr.FindSecurityGroup("default", "testSg")
	//	if err != nil {
	//		return true, nil
	//	}
	//	return false, nil
	//}, "Sg found", "1ms", "1s")
	//
	//// verify the sg is gone
	//_, err = stateMgr.FindSecurityGroup("default", "testSg")
	//Assert(t, (err != nil), "Security group still found after deleting")
}

func TestSgAttachEndpoint(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")
	// create sg
	_, err = createSg(stateMgr, "default", "testSg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	sg, err := stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Error finding sg")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint object
	epinfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			WorkloadName:       "testContainerName",
			Network:            "default",
			HomingHostAddr:     "192.168.1.1",
			HomingHostName:     "testHost",
			WorkloadAttributes: map[string]string{"env": "production", "app": "procurement"},
		},
	}

	// find the network
	_, err = stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	// create endpoint
	err = stateMgr.ctrler.Endpoint().Create(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", epinfo)

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "10ms", "2s")

	// verify endpoint is associated with the sg
	eps, err := stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err == nil), "Error finding the endpoint", epinfo)
	AssertEventually(t, func() (bool, interface{}) {
		if len(eps.Endpoint.Status.SecurityGroups) == 1 {
			return true, nil
		}
		return false, fmt.Sprintf("expected : %d, got : %v. Endpoint Object : %v", 1, len(eps.Endpoint.Status.SecurityGroups), eps)
	}, "Sg was not linked to endpoint", "100ms", "2s")
	Assert(t, (len(eps.Endpoint.Status.SecurityGroups) == 1), "Sg was not linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 1), "Sg was not linked to endpoint", eps)
	Assert(t, (eps.Endpoint.Status.SecurityGroups[0] == sg.SecurityGroup.Name), "Sg was not linked to endpoint", eps)
	Assert(t, (len(sg.SecurityGroup.Status.Workloads) == 1), "Endpoint is not linked to sg", sg)
	Assert(t, (len(sg.endpoints) == 1), "Endpoint is not linked to sg", sg)
	Assert(t, (sg.SecurityGroup.Status.Workloads[0] == eps.Endpoint.Name), "Endpoint is not linked to sg", sg)

	// create a new sg
	_, err = createSg(stateMgr, "default", "testSg2", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg2")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	sg2, err := stateMgr.FindSecurityGroup("default", "testSg2")
	AssertOk(t, err, "Error finding sg")

	// verify endpoint is associated with the new sg too..
	AssertEventually(t, func() (bool, interface{}) {
		if len(sg2.SecurityGroup.Status.Workloads) == 1 {
			return true, nil
		}
		return false, nil
	}, "Endpoint not associated ", "1ms", "1s")
	Assert(t, (len(eps.Endpoint.Status.SecurityGroups) == 2), "Sg was not linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 2), "Sg was not linked to endpoint", eps)
	Assert(t, (eps.Endpoint.Status.SecurityGroups[1] == sg2.SecurityGroup.Name), "Sg was not linked to endpoint", eps)
	Assert(t, (len(sg2.SecurityGroup.Status.Workloads) == 1), "Endpoint is not linked to sg", sg2)

	Assert(t, (len(sg2.endpoints) == 1), "Endpoint is not linked to sg", sg2)
	Assert(t, (sg2.SecurityGroup.Status.Workloads[0] == eps.Endpoint.Name), "Endpoint is not linked to sg", sg2)

	// delete the second sg
	err = stateMgr.ctrler.SecurityGroup().Delete(&sg2.SecurityGroup.SecurityGroup)
	AssertOk(t, err, "Error deleting security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg2")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	// verify sg is removed from endpoint
	Assert(t, (len(eps.Endpoint.Status.SecurityGroups) == 1), "Sg is still linked to endpoint", eps)
	Assert(t, (len(eps.groups) == 1), "Sg is still linked to endpoint", eps)
	Assert(t, (eps.Endpoint.Status.SecurityGroups[0] == sg.SecurityGroup.Name), "Sg is still linked to endpoint", eps)

	// delete the endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&eps.Endpoint.Endpoint)
	AssertOk(t, err, "Error deleting endpoint")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	// verify endpoint is removed from sg
	Assert(t, (len(sg.SecurityGroup.Status.Workloads) == 0), "Endpoint is still linked to sg", sg)
}

func TestSgErrorCases(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create sg with nil selector
	_, err = createSg(stateMgr, "default", "testSg", nil)
	AssertOk(t, err, "Error creating security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	_, err = stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Error finding sg")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// endpoint object with nil workload attributes
	epinfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testEndpoint",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			WorkloadName:       "testContainerName",
			Network:            "default",
			HomingHostAddr:     "192.168.1.1",
			HomingHostName:     "testHost",
			WorkloadAttributes: nil,
		},
	}

	// find the network
	_, err = stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	// create endpoint
	err = stateMgr.ctrler.Endpoint().Create(&epinfo)
	Assert(t, (err == nil), "Error creating the endpoint", epinfo)

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testEndpoint")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	// verify endpoint is created
	eps, err := stateMgr.FindEndpoint("default", "testEndpoint")
	Assert(t, (err == nil), "Error finding the endpoint", epinfo)

	// create a new sg with nil selector
	_, err = createSg(stateMgr, "default", "testSg2", nil)
	AssertOk(t, err, "Error creating security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg2")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	sg2, err := stateMgr.FindSecurityGroup("default", "testSg2")
	AssertOk(t, err, "Error finding sg")

	// delete the second sg
	err = stateMgr.ctrler.SecurityGroup().Delete(&sg2.SecurityGroup.SecurityGroup)
	AssertOk(t, err, "Error deleting security group")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("default", "testSg2")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	// delete the endpoint
	err = stateMgr.ctrler.Endpoint().Delete(&eps.Endpoint.Endpoint)
	AssertOk(t, err, "Error deleting endpoint")
}

// test concurrent add/delete/change of endpoints
func TestEndpointConcurrency(t *testing.T) {
	var concurrency = 200

	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create a network
	err = createNetwork(t, stateMgr, "default", "default", "10.1.1.0/24", "10.1.1.254")
	AssertOk(t, err, "Error creating the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "default")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// find the network
	_, err = stateMgr.FindNetwork("default", "default")
	AssertOk(t, err, "Could not find the network")

	waitCh := make(chan error, concurrency*2)

	// create endpoint
	for i := 0; i < concurrency; i++ {
		go func(idx int) {
			// endpoint object
			epinfo := workload.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Name:      fmt.Sprintf("testEndpoint-%d", idx),
					Namespace: "default",
					Tenant:    "default",
				},
				Spec: workload.EndpointSpec{},
				Status: workload.EndpointStatus{
					WorkloadName:       "testContainerName",
					Network:            "default",
					HomingHostAddr:     "192.168.1.1",
					HomingHostName:     "testHost",
					WorkloadAttributes: map[string]string{"env": "production", "app": "procurement"},
				},
			}

			// create endpoint
			eperr := stateMgr.ctrler.Endpoint().Create(&epinfo)
			waitCh <- eperr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertEventually(t, func() (bool, interface{}) {
			_, err := stateMgr.FindEndpoint("default", fmt.Sprintf("testEndpoint-%d", i))
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Endpoint not found", "1ms", "5s")

	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error creating endpoint")
	}

	// create few SGs concurrently that match on endpoints
	tsg := make([]*security.SecurityGroup, concurrency)
	for i := 0; i < concurrency; i++ {
		go func(idx int) {
			var serr error
			tsg[idx], serr = createSg(stateMgr, "default", fmt.Sprintf("testSg-%d", idx), labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
			waitCh <- serr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error creating sgs")
	}

	// delete the sgs concurrently
	for i := 0; i < concurrency; i++ {
		go func(idx int) {
			serr := stateMgr.ctrler.SecurityGroup().Delete(tsg[idx])
			fmt.Printf("Error delete sgs %v\n", serr)
			waitCh <- serr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error deleting sgs")
	}
	// delete endpoint

	for i := 0; i < concurrency; i++ {
		AssertEventually(t, func() (bool, interface{}) {
			_, err := stateMgr.FindEndpoint("default", fmt.Sprintf("testEndpoint-%d", i))
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Endpoint not found", "1ms", "1s")

	}

	for i := 0; i < concurrency; i++ {
		go func(idx int) {
			ometa := workload.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Name:      fmt.Sprintf("testEndpoint-%d", idx),
					Namespace: "default",
					Tenant:    "default",
				},
			}
			// delete the endpoint
			eperr := stateMgr.ctrler.Endpoint().Delete(&ometa)
			waitCh <- eperr
		}(i)
	}

	for i := 0; i < concurrency; i++ {
		AssertOk(t, <-waitCh, "Error deleting endpoint")
	}
}

// TestTenantCreateDelete tests tenant create and delete
func TestTenantCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "testTenant")
	AssertOk(t, err, "Error creating the tenant")

	// verify tenant got created
	tn, err := stateMgr.FindTenant("testTenant")
	AssertOk(t, err, "Error finding the tenant")
	AssertEquals(t, tn.Tenant.Name, "testTenant", "Did not match expected tenant name")

	// create security groups
	sg1, err := createSg(stateMgr, "testTenant", "testSg1", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("testTenant", "testSg1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	sg2, err := createSg(stateMgr, "testTenant", "testSg2", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindSecurityGroup("testTenant", "testSg2")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Sg not found", "1ms", "1s")

	// delete the tenant
	err = stateMgr.ctrler.Tenant().Delete(&tn.Tenant.Tenant)
	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindTenant("testTenant")
		if err != nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Tenant was deleted  foud", "1ms", "1s")

	// delete resources

	err = stateMgr.ctrler.SecurityGroup().Delete(sg1)
	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindSecurityGroup("testTenant", sg1.Name)
		if err != nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Tenant was deleted  foud", "1ms", "1s")

	err = stateMgr.ctrler.SecurityGroup().Delete(sg2)
	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindSecurityGroup("testTenant", sg2.Name)
		if err != nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Tenant was deleted  foud", "1ms", "1s")

	//Make sure Tenant  deleted.
	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindTenant("testTenant")
		if err != nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Tenant was deleted  foud", "1ms", "1s")

}

// TestNonExistentTenantDeletion will test that non existent tenants can't be deleted
func TestNonExistentTenantDeletion(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	ntn := ctkit.Tenant{
		Tenant: cluster.Tenant{
			TypeMeta: api.TypeMeta{Kind: "Tenant"},
			ObjectMeta: api.ObjectMeta{
				Name: "nonExistingTenant",
			},
		},
	}
	// delete the tenant
	err = stateMgr.OnTenantDelete(&ntn)
	Assert(t, err == ErrTenantNotFound, "Deleting tenant failed.")
}

func TestWorkloadCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0409",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
				{
					MACAddress:   "0001.0203.0406",
					MicroSegVlan: 100,
					Network:      "network100",
				},
			},
		},
	}

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "20ms", "1s")

	// verify we can find the network for the workload
	nw, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
	AssertOk(t, err, "Could not find the network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0409")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify we can find the endpoint associated with the workload
	foundEp, ok := nw.FindEndpoint("testWorkload-0001.0203.0409")
	Assert(t, ok, "Could not find the endpoint", "testWorkload-0001.0203.0409")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&wr)
	AssertOk(t, err, "Error deleting the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0409")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify endpoint is gone from the database
	_, ok = nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, (ok == false), "Deleted endpoint still found in network db", "testWorkload-0001.0203.0405")
}

func TestWorkloadUpdate(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	netwrk := &network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      "network100",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: network.NetworkSpec{
			Type:   network.NetworkType_Bridged.String(),
			VlanID: 100,
		},
	}

	// create network
	err = stateMgr.ctrler.Network().Create(netwrk)
	AssertOk(t, err, "Could not create the network")

	netwrk = &network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      "network101",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: network.NetworkSpec{
			Type:   network.NetworkType_Bridged.String(),
			VlanID: 101,
		},
	}

	// create network
	err = stateMgr.ctrler.Network().Create(netwrk)
	AssertOk(t, err, "Could not create the network")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
				{
					MACAddress:   "0001.0203.0406",
					MicroSegVlan: 100,
					Network:      "network100",
				},
			},
		},
	}

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")
	start := time.Now()

	done := false
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			if !done {
				timeTrack(start, "Network create took")
				done = true
			}
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil {
			return true, nil
		}
		_, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0406")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoints not foud", "1ms", "2s")

	// verify we can find the endpoint associated with the workload
	foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0406")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	// update workload external vlan and network
	nwr := ref.DeepCopy(wr).(workload.Workload)
	nwr.Spec.Interfaces[0].ExternalVlan = 2
	nwr.Spec.Interfaces[1].Network = "network101"
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-2")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	// verify we can find the new network for the workload
	nw, err := stateMgr.FindNetwork("default", "Network-Vlan-2")
	AssertOk(t, err, "Could not find updated network")
	nw1, err := stateMgr.FindNetwork("default", "network101")
	AssertOk(t, err, "Could not find updated network")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil {
			_, ok := nw.FindEndpoint("testWorkload-0001.0203.0405")
			return ok, nil
		}
		_, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0406")
		if err == nil {
			_, ok := nw1.FindEndpoint("testWorkload-0001.0203.0406")
			return ok, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	foundEp, ok := nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, ok, "Could not find the endpoint", "testWorkload-0001.0203.0405")
	Assert(t, (foundEp.Endpoint.Status.Network == nw.Network.Name), "endpoint network did not match")

	// change mac address of the workload
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].MACAddress = "0001.0201.0405"
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify old endpoint is deleted
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	Assert(t, (err != nil), "found endpoint with old mac address", foundEp)

	_, ok = nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, (ok == false), "old endpoint still found in network db", "testWorkload-0001.0203.0405")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify new endpoint is created
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
	AssertOk(t, err, "Could not find the new endpoint")

	// change useg vlan
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].MicroSegVlan = 101
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil && (foundEp.Endpoint.Status.MicroSegmentVlan == 101) {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// add IP address
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].IpAddresses = []string{"1.2.3.4/32"}
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil && (foundEp.Endpoint.Status.IPv4Address == "1.2.3.4/32") {
			return true, nil
		}
		fmt.Printf("IP Address %v", foundEp.Endpoint.Status.IPv4Address)
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// update IP address
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].IpAddresses = []string{"1.2.3.5/32"}
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil && (foundEp.Endpoint.Status.IPv4Address == "1.2.3.5/32") {
			return true, nil
		}
		fmt.Printf("IP Address %v", foundEp.Endpoint.Status.IPv4Address)
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// remove IP address
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces[0].IpAddresses = []string{}
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil && (foundEp.Endpoint.Status.IPv4Address == "") {
			return true, nil
		}
		fmt.Printf("IP Address %v", foundEp.Endpoint.Status.IPv4Address)
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// add new interface to workload
	newIntf := workload.WorkloadIntfSpec{
		MACAddress:   "0002.0406.0800",
		MicroSegVlan: 200,
		ExternalVlan: 1,
	}
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces = append(nwr.Spec.Interfaces, newIntf)
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0406.0800")
		if err == nil && (foundEp.Endpoint.Status.MicroSegmentVlan == 200) {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify we can find the new endpoint

	// delete third interface
	nwr = ref.DeepCopy(nwr).(workload.Workload)
	nwr.Spec.Interfaces = nwr.Spec.Interfaces[0:2]
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0406.0800")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")
	// verify endpoint is gone
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0002.0406.0800")
	Assert(t, (err != nil), "found endpoint for deleted interface", foundEp)

	// trigger a dummy and verify endpoint is not deleted
	nwr.ObjectMeta.GenerationID = "2"
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")
	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
	AssertOk(t, err, "Could not find the new endpoint")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&nwr)
	AssertOk(t, err, "Error deleting the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0201.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify endpoint is gone from the database
	_, ok = nw.FindEndpoint("testWorkload-0002.0406.0800")
	Assert(t, (ok == false), "Deleted endpoint still found in network db", "testWorkload-0001.0201.0203")
}

func TestWorkloadUpdateHost(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")
	// create the smartNics
	snic1 := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard1",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}
	snic2 := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard2",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0607.0809",
		},
	}
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic1)
	AssertOk(t, err, "Could not create the smartNic")
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic2)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host1 := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: "testHost1",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}
	host2 := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: "testHost2",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0607.0809",
				},
			},
		},
	}

	// create the hosts
	err = stateMgr.ctrler.Host().Create(&host1)
	AssertOk(t, err, "Could not create the host")
	err = stateMgr.ctrler.Host().Create(&host2)
	AssertOk(t, err, "Could not create the host")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost1",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0002.0202.0202",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindHost("default", "testHost2")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "20ms", "2s")

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "20ms", "2s")

	// verify we can find the endpoint associated with the workload
	foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.HomingHostName == host1.Name), "endpoint params did not match")
	Assert(t, (foundEp.Endpoint.Status.NodeUUID == snic1.Name), "endpoint params did not match")

	// update workload hostname
	nwr := ref.DeepCopy(wr).(workload.Workload)
	nwr.Spec.HostName = "testHost2"
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
		if err == nil && (foundEp.Endpoint.Status.HomingHostName == host2.Name) &&
			(foundEp.Endpoint.Status.NodeUUID == snic2.Name) {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	foundEp, err = stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.HomingHostName == host2.Name), "endpoint host did not match")
	Assert(t, (foundEp.Endpoint.Status.NodeUUID == snic2.Name), "endpoint params did not match")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&nwr)
	AssertOk(t, err, "Error deleting the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "1s")

	// verify endpoint is gone from the database
	_, err = stateMgr.FindEndpoint("default", "testWorkload-0002.0202.0202")
	Assert(t, (err != nil), "Deleted endpoint still found in db")
}

func TestWorkloadWithDuplicateMac(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testSmartNIC",
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: "testHost",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "2s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "2s")

	// verify we can find the network for the workload
	nw, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
	AssertOk(t, err, "Could not find the network")

	// verify we can find the endpoint associated with the workload
	foundEp, ok := nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, ok, "Could not find the endpoint", "testWorkload-0001.0203.0405")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	// try to create second workload with duplicate mac address
	wr2 := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload2",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0405",
					MicroSegVlan: 101,
					ExternalVlan: 1,
				},
			},
		},
	}
	err = stateMgr.ctrler.Workload().Create(&wr2)
	AssertOk(t, err, "Could not create the second workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "2s")

	// find the second workload
	AssertEventually(t, func() (bool, interface{}) {
		fwr2, err := stateMgr.FindWorkload("default", "testWorkload2")
		if err == nil && fwr2.Workload.Status.PropagationStatus.Status == "Propagation Failed. Duplicate MAC address" {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "2s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload2-0001.0203.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "2s")

	// verify endpoint is not created
	_, ok = nw.FindEndpoint("testWorkload2-0001.0203.0405")
	Assert(t, (ok == false), "Duplicate endpoint still found in network db", "testWorkload2-0001.0203.0405")

	// delete the workload
	err = stateMgr.ctrler.Workload().Delete(&wr)
	AssertOk(t, err, "Error deleting the workload")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		_, ok = nw.FindEndpoint("testWorkload-0001.0203.0405")
		if err != nil && !ok {
			return true, nil
		}
		return false, nil
	}, "Endpoint not foud", "1ms", "2s")

	// verify endpoint is gone from the database
	_, ok = nw.FindEndpoint("testWorkload-0001.0203.0405")
	Assert(t, (ok == false), "Deleted endpoint still found in network db", "testWorkload-0001.0203.0405")
}

func TestHostCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")
	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindHost("default", "testHost")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "2s")

	// verify we can find the endpoint associated with the host
	foundHost, err := stateMgr.FindHost("default", "testHost")
	AssertOk(t, err, "Could not find the host")
	Assert(t, (len(foundHost.Host.Spec.DSCs) == 1), "host params did not match")

	// delete the host
	err = stateMgr.ctrler.Host().Delete(&host)
	AssertOk(t, err, "Error deleting the host")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindHost("default", "testHost")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "2s")

	// verify endpoint is gone from the database
	_, err = stateMgr.FindHost("default", "testHost")
	Assert(t, (err != nil), "Deleted host still found in db")
}

func TestHostUpdates(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindHost("default", "testHost")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "2s")

	// verify we can find the endpoint associated with the host
	_, err = stateMgr.FindHost("default", "testHost")
	AssertOk(t, err, "Could not find the host")

	// create a workload on the host
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "0001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "workload create failed")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint foud", "1ms", "2s")

	// verify we can not find the endpoint associated with the workload
	_, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	Assert(t, err != nil, "Endpoint got created without smartnic")

	// create the smartnic
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{
			ID: "test-snic",
		},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	AssertEventually(t, func() (bool, interface{}) {
		foundEP, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil && foundEP.Endpoint.Status.NodeUUID == snic.Name {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found or node uuid not match", "20ms", "2s")

	// change host's mac address
	nhst := ref.DeepCopy(host).(cluster.Host)
	nhst.Spec.DSCs[0].MACAddress = "0002.0406.0800"
	err = stateMgr.ctrler.Host().Update(&nhst)
	AssertOk(t, err, "Error updating the host")

	// verify the endpoint is gone
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found or node uuid not match", "1ms", "2s")

	// now associate by name
	nhst = ref.DeepCopy(nhst).(cluster.Host)
	nhst.Spec.DSCs[0].MACAddress = ""
	nhst.Spec.DSCs[0].ID = "test-snic"
	fmt.Printf("Host update kind %v\n", nhst.GetKind())
	err = stateMgr.ctrler.Host().Update(&nhst)
	AssertOk(t, err, "Error updating the host")

	// verify we can find the endpoint
	//foundEP, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	//AssertOk(t, err, "could not find the endpoint")
	//Assert(t, foundEP.Endpoint.Status.NodeUUID == snic.Name, "Endpoint nodeUUID did not match")

	AssertEventually(t, func() (bool, interface{}) {
		foundEP, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err == nil && foundEP.Endpoint.Status.NodeUUID == snic.Name {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found or node uuid not match", "1ms", "2s")

	// delete the host
	err = stateMgr.ctrler.DistributedServiceCard().Delete(&snic)
	AssertOk(t, err, "Error deleting the host")

	// verify endpoint is gone from the database
	//_, err = stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
	//Assert(t, err != nil, "endpoint still found when host isnt associated with snic")

	// verify the endpoint is gone
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-0001.0203.0405")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found or node uuid not match", "1ms", "2s")

}

func TestDSCProfileCreateUpdateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// smartNic params
	dscprof := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDSCProfile",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "TRANSPARENT",
			FlowPolicyMode: "BASE_NET",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DSCProfile().Create(&dscprof)
	AssertOk(t, err, "Could not create the smartNic")

	// verify the profile is there
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindDSCProfile("", "testDSCProfile")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Did not find DSCProfile", "1ms", "2s")

	// change update
	dscprof.Spec.FlowPolicyMode = cluster.DSCProfileSpec_FLOW_AWARE.String()
	err = stateMgr.ctrler.DSCProfile().Update(&dscprof)
	AssertOk(t, err, "Update the DSCProfile")

	// Verify Profile is updated
	AssertEventually(t, func() (bool, interface{}) {
		obj, err := stateMgr.FindDSCProfile("", "testDSCProfile")
		if err == nil {
			if obj.DSCProfile.DSCProfile.Spec.FlowPolicyMode == cluster.DSCProfileSpec_FLOW_AWARE.String() {
				return true, nil
			}
		}
		return false, nil
	}, "Didnot update the profile", "1ms", "2s")

	// List Profile
	dsclist, err := stateMgr.ListDSCProfiles()
	AssertOk(t, err, "Error listing DSC Profiles")
	Assert(t, (len(dsclist) == 1), "invalid number of dsc profiles")
	// delete the smartNic
	err = stateMgr.ctrler.DSCProfile().Delete(&dscprof)
	AssertOk(t, err, "Error deleting the dscProfile")

	// verify the endpoint is gone
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindDSCProfile("", "testDSCProfile")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Did not find DSCProfile", "1ms", "2s")
}

func TestSmartNicCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// verify the endpoint is gone
	AssertEventually(t, func() (bool, interface{}) {
		foundDistributedServiceCard, err := stateMgr.FindDistributedServiceCard("default", "testDistributedServiceCard")
		if err == nil && foundDistributedServiceCard.DistributedServiceCard.Status.PrimaryMAC == "0001.0203.0405" {
			recvr, err := stateMgr.mbus.FindReceiver(foundDistributedServiceCard.DistributedServiceCard.Status.PrimaryMAC)
			if recvr == nil || err != nil {
				return false, nil
			}
			return true, nil

		}

		return false, nil
	}, "Did not find DSC", "1ms", "2s")

	_, err = stateMgr.FindDistributedServiceCardByMacAddr("0001.0203.0405")
	AssertOk(t, err, "Could not find the smartNic")

	dscprof := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDSCProfile",
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        "TRANSPARENT",
			FlowPolicyMode: "BASE_NET",
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DSCProfile().Create(&dscprof)
	AssertOk(t, err, "Could not create the smartNic profile")

	// verify the profile is there
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindDSCProfile("", "testDSCProfile")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Did not find DSCProfile", "1ms", "2s")

	newnic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{
			DSCProfile: "testDSCProfile",
		},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: "0001.0203.0405",
		},
	}
	err = stateMgr.ctrler.DistributedServiceCard().Update(&newnic)
	AssertOk(t, err, "Error DistributedServicesCard update failed")

	// delete the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Delete(&snic)
	AssertOk(t, err, "Error deleting the smartNic")

	// verify the endpoint is gone
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindDistributedServiceCard("default", "testDistributedServiceCard")
		if err != nil {
			recvr, err := stateMgr.mbus.FindReceiver(snic.Status.PrimaryMAC)
			if recvr != nil || err == nil {
				return false, nil
			}
			return true, nil
		}
		return false, nil
	}, "Did not find DSC", "1ms", "2s")
}

func TestNetworkInterfaceConvert(t *testing.T) {
	agentNetif := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Name: "00ae.cd00.1638-uplink130",
		},
		Spec: netproto.InterfaceSpec{
			Type: "UPLINK_ETH",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "UP",
		},
	}

	cNetif := convertNetifObj("testNode", agentNetif)
	if cNetif == nil {
		t.Fatalf("Unable to convert netif object")
	}
	if cNetif.Status.DSC != "testNode" {
		t.Fatalf("smartnic not set correctly %+v", cNetif)
	}
	Assert(t, convertAgentIFToAPIProto(agentNetif.Spec.Type) == cNetif.Status.Type, " Types did not match [%v][%v]", convertAgentIFToAPIProto(agentNetif.Spec.Type), cNetif.Spec.Type)
	netifState, err := NewNetworkInterfaceState(&ctkit.NetworkInterface{NetworkInterface: *cNetif}, nil)
	AssertOk(t, err, "failed to netif state")
	nif := convertNetworkInterfaceObject(netifState)
	Assert(t, nif != nil, "convert failed")
	Assert(t, nif.Name == agentNetif.Name, "name did not match")
	Assert(t, cNetif.Status.Type == convertAgentIFToAPIProto(agentNetif.Spec.Type), "Type did not match [%v/%v]", nif.Spec.Type, convertAgentIFToAPIProto(agentNetif.Spec.Type))

	tests := []struct {
		agent string
		api   string
	}{
		{netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_NONE)], network.IFType_NONE.String()},
		{netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_HOST_PF)], network.IFType_HOST_PF.String()},
		{netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_UPLINK_ETH)], network.IFType_UPLINK_ETH.String()},
		{netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_UPLINK_MGMT)], network.IFType_UPLINK_MGMT.String()},
		{netproto.InterfaceSpec_IFType_name[int32(netproto.InterfaceSpec_LOOPBACK)], network.IFType_LOOPBACK_TEP.String()},
	}

	for _, v := range tests {
		Assert(t, convertIFTypeToAgentProto(v.api) == v.agent, "convert form api to netproto failed [%v][%v]", convertIFTypeToAgentProto(v.api), v.agent)
		Assert(t, convertAgentIFToAPIProto(v.agent) == v.api, "convert form netproto to api failed [%v][%v]", convertAgentIFToAPIProto(v.agent), v.api)
		agentNetif.Spec.Type = v.agent
		Assert(t, convertNetifObj("testnode", agentNetif) != nil, "failed to convert")
	}
}

// Test CRUD operations on NetworkInterface object; there is no backend
// code for these objects, so we don't check on other return details
func TestNetworkInterfaceCRUD(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	netifName := "00ae.cd00.1638-uplink130"
	netif := network.NetworkInterface{
		TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
		ObjectMeta: api.ObjectMeta{
			Name: netifName,
		},
		Status: network.NetworkInterfaceStatus{
			OperStatus: "UP",
			Type:       "UPLINK_ETH",
		},
	}

	// create the netif
	err = stateMgr.ctrler.NetworkInterface().Create(&netif)
	AssertOk(t, err, "Could not create the smartNic")

	// delete the smartNic
	err = stateMgr.ctrler.NetworkInterface().Update(&netif)
	AssertOk(t, err, "Error deleting the smartNic")

	// delete the smartNic
	err = stateMgr.ctrler.NetworkInterface().Delete(&netif)
	AssertOk(t, err, "Error deleting the smartNic")

	// Statemngr empty functions
	cif := &ctkit.NetworkInterface{
		NetworkInterface: netif,
	}
	err = stateMgr.OnNetworkInterfaceCreate(cif)
	AssertOk(t, err, "expecting to pass")

	err = stateMgr.OnNetworkInterfaceUpdate(cif, &netif)
	AssertOk(t, err, "expecting to pass")

	err = stateMgr.OnNetworkInterfaceDelete(cif)
	AssertOk(t, err, "expecting to pass")

	netif.Spec.Pause = &network.PauseSpec{
		Type: network.PauseType_PRIORITY.String(),
	}

	cif.HandlerCtx = &NetworkInterfaceState{}
	nifstate, err := networkInterfaceStateFromObj(cif)
	nifstate.NetworkInterfaceState = cif
	AssertOk(t, err, "expecting to succeed creating Netif State")
	npif := convertNetworkInterfaceObject(nifstate)
	opts := stateMgr.GetInterfaceWatchOptions()
	Assert(t, opts != nil, "expecting non-nul options")

	err = stateMgr.OnInterfaceOperUpdate("node1", npif)
	AssertOk(t, err, "expecting to pass")

}

func TestRouteTableObj(t *testing.T) {
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	sma := SmRouteTable{
		sm: stateMgr,
	}
	rt := network.RouteTable{
		TypeMeta: api.TypeMeta{Kind: "RouteTable"},
		ObjectMeta: api.ObjectMeta{
			Name:      "Test",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: network.RouteTableSpec{},
	}
	// Statemngr empty functions
	crt := &ctkit.RouteTable{
		RouteTable: rt,
	}
	err = sma.OnRouteTableCreate(crt)
	AssertOk(t, err, "expecting to pass")

	err = sma.OnRouteTableUpdate(crt, &rt)
	AssertOk(t, err, "expecting to pass")

	err = sma.OnRouteTableDelete(crt)
	Assert(t, err != nil, "expecting to fail since object does not exist")

	crt.HandlerCtx = &RouteTableState{}
	rtstate, err := RouteTableFromObj(crt)
	rtstate.RouteTable = crt
	AssertOk(t, err, "expecting to succeed creating Netif State")
	_ = convertRouteTable(rtstate)
	opts := sma.GetRouteTableWatchOptions()
	Assert(t, opts != nil, "expecting non-nul options")

}

/*
func TestAgentCrudEvents(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	AssertOk(t, stateMgr.OnEndpointCreateReq("test-node", &netproto.Endpoint{}), "endpoint create")
	AssertOk(t, stateMgr.OnEndpointUpdateReq("test-node", &netproto.Endpoint{}), "endpoint update")
	AssertOk(t, stateMgr.OnEndpointDeleteReq("test-node", &netproto.Endpoint{}), "endpoint delete")

	AssertOk(t, stateMgr.OnNetworkSecurityPolicyCreateReq("test-node", &netproto.NetworkSecurityPolicy{}), "NetworkSecurityPolicy create")
	AssertOk(t, stateMgr.OnNetworkSecurityPolicyUpdateReq("test-node", &netproto.NetworkSecurityPolicy{}), "NetworkSecurityPolicy update")
	AssertOk(t, stateMgr.OnNetworkSecurityPolicyDeleteReq("test-node", &netproto.NetworkSecurityPolicy{}), "NetworkSecurityPolicy delete")

	AssertOk(t, stateMgr.OnSecurityProfileCreateReq("test-node", &netproto.SecurityProfile{}), "SecurityProfile create")
	AssertOk(t, stateMgr.OnSecurityProfileUpdateReq("test-node", &netproto.SecurityProfile{}), "SecurityProfile update")
	AssertOk(t, stateMgr.OnSecurityProfileDeleteReq("test-node", &netproto.SecurityProfile{}), "SecurityProfile delete")

	netIf := netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEnic",
		},
		Spec: netproto.InterfaceSpec{
			Type: "UPLINK_ETH",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "UP",
		},
	}
	AssertOk(t, stateMgr.OnInterfaceCreateReq("test-node", &netIf), "Interface create")
	AssertOk(t, stateMgr.OnInterfaceUpdateReq("test-node", &netIf), "Interface update")

	netIf = netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEnic",
		},
		Spec: netproto.InterfaceSpec{
			Type: "UPLINK_MGMT",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "DOWN",
		},
	}
	AssertOk(t, stateMgr.OnInterfaceCreateReq("test-node", &netIf), "Interface create")
	AssertOk(t, stateMgr.OnInterfaceUpdateReq("test-node", &netIf), "Interface update")
	stateMgr.OnInterfaceDeleteReq("test-node", &netIf)

	netIf = netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEnic",
		},
		Spec: netproto.InterfaceSpec{
			Type: "HOST_PF",
		},
		Status: netproto.InterfaceStatus{
			OperStatus: "DOWN",
		},
	}
	AssertOk(t, stateMgr.OnInterfaceCreateReq("test-node", &netIf), "Interface create")
	AssertOk(t, stateMgr.OnInterfaceUpdateReq("test-node", &netIf), "Interface update")
	stateMgr.OnInterfaceDeleteReq("test-node", &netIf)
}
*/

func TestWatchFilter(t *testing.T) {
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	options1 := api.ListWatchOptions{}
	options1.Name = "xyz"
	options1.ResourceVersion = "100"
	options1.Tenant = "test"
	options1.Namespace = "test"

	options2 := api.ListWatchOptions{}
	options2.FieldSelector = "spec.node-uuid=0000.0000.0001"

	options3 := api.ListWatchOptions{}
	options3.FieldSelector = "spec.node-uui=0000.0000.0001"

	filterFn1 := stateMgr.GetWatchFilter("Network", &options1)
	filterFn2 := stateMgr.GetWatchFilter("netproto.Endpoint", &options2)
	filterFn3 := stateMgr.GetWatchFilter("Endpoint", &options2)
	filterFn4 := stateMgr.GetWatchFilter("netproto.Endpoint", &options3)

	obj1 := netproto.Network{}
	obj1.Name = "xyz"
	obj1.ResourceVersion = "100"
	obj1.Tenant = "test"
	obj1.Namespace = "test"

	obj2 := netproto.Endpoint{}
	res := true

	for _, filt := range filterFn1 {
		if !filt(&obj1, nil) {
			res = false
			break
		}
	}

	Assert(t, res, "expecting filter to pass")

	obj2.Spec.NodeUUID = "0000.0000.0001"
	res = true
	for _, filt := range filterFn2 {
		if !filt(&obj2, nil) {
			res = false
			break
		}
	}

	Assert(t, res, "expecting filter to pass")
	obj2.Spec.NodeUUID = "0000.0000.0002"

	res = true
	for _, filt := range filterFn2 {
		if !filt(&obj2, nil) {
			res = false
			break
		}
	}

	Assert(t, res == false, "expecting filter to fail")

	res = true
	for _, filt := range filterFn3 {
		if !filt(&obj2, nil) {
			res = false
			break
		}
	}

	Assert(t, res == true, "expecting filter to pass")

	res = true
	for _, filt := range filterFn4 {
		if !filt(&obj2, nil) {
			res = false
			break
		}
	}

	Assert(t, res == true, "expecting filter to pass")

	obj3 := obj1
	obj3.Name = "newtest"
	options4 := api.ListWatchOptions{}
	options4.FieldChangeSelector = []string{"ObjectMeta.Name"}
	filterFn5 := stateMgr.GetWatchFilter("Network", &options4)

	res = true
	for _, filt := range filterFn5 {
		if !filt(&obj3, &obj1) {
			res = false
			break
		}
	}

	Assert(t, res == true, "expecting filter to pass")

	stateMgr.StopAppWatch()
	stateMgr.StartAppWatch()
	stateMgr.StopNetworkSecurityPolicyWatch()
	stateMgr.StartNetworkSecurityPolicyWatch()
}

func TestVirtualRouterCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create virtual router
	vr, err := createVirtualRouter(t, stateMgr, "default", "default")
	AssertOk(t, err, "Error creating the virtual router")

	obj, err := stateMgr.FindVirtualRouter("default", "default", "default")
	AssertOk(t, err, "Error finding virtual router")

	// update the virtual router
	version, _ := strconv.Atoi(obj.VirtualRouter.GenerationID)
	vr.GenerationID = strconv.Itoa(version + 1)
	err = stateMgr.ctrler.VirtualRouter().Update(vr)
	AssertOk(t, err, "Error updating virtual router")

	// delete the virtual router
	err = stateMgr.ctrler.VirtualRouter().Delete(vr)
	AssertOk(t, err, "Error deleting VirtualRouter")
}

func TestIPAMPolicyCreateDelete(t *testing.T) {
	name := "testPolicy"

	// enable overlayrouting feature
	errs := enableOverlayRouting()

	if len(errs) != 0 {
		t.Fatalf("TestRoutingConfigCreateDelete: enableOverlayRouting failed: %v", errs)
		return
	}

	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	obj := &IPAMState{}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create virtual router
	_, err = createVirtualRouter(t, stateMgr, "default", "default")
	AssertOk(t, err, "Error creating the virtual router")

	// create IPAMPolicy
	policy, err := createIPAMPolicy(stateMgr, "default", name, "100.1.1.1")
	AssertOk(t, err, "Error creating IPAMPolicy")

	// verify we can find the IPAMPolicy

	AssertEventually(t, func() (bool, interface{}) {
		obj, err = smgrIPAM.FindIPAMPolicy("default", "default", name)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error finding ipampolicy %v\n", err)
		return false, nil
	}, "ipampolicy not foud", "1ms", "1s")
	AssertEquals(t, obj.IPAMPolicy.Spec.DHCPRelay.Servers[0].IPAddress, policy.Spec.DHCPRelay.Servers[0].IPAddress, "IPAMPolicy params did not match")

	// update the IPAMPolicy
	//version, _ := strconv.Atoi(obj.IPAMPolicy.GenerationID)
	version := 1
	policy.GenerationID = strconv.Itoa(version + 1)
	policy.Spec.DHCPRelay.Servers[0].IPAddress = "101.1.1.1"
	err = stateMgr.ctrler.IPAMPolicy().Update(policy)
	AssertOk(t, err, "Error updating IPAMPolicy")

	// verify update went through

	AssertEventually(t, func() (bool, interface{}) {
		obj, err = smgrIPAM.FindIPAMPolicy("default", "default", name)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error finding ipampolicy %v\n", err)
		return false, nil
	}, "ipampolicy not foud", "1ms", "1s")
	AssertEquals(t, obj.IPAMPolicy.Spec.DHCPRelay.Servers[0].IPAddress, policy.Spec.DHCPRelay.Servers[0].IPAddress, "IPAMPolicy params did not match")

	// delete the IPAM policy
	err = stateMgr.ctrler.IPAMPolicy().Delete(policy)
	AssertOk(t, err, "Error deleting IPAMPolicy")

	// verify the IPAMPolicy is deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrIPAM.FindIPAMPolicy("default", "default", name)
		if err != nil {
			return true, nil
		}
		fmt.Printf("IPAMPolicy still found after deleting %v\n", err)
		return false, nil
	}, "ipampolicy still foud", "1ms", "1s")
}

func enableOverlayRouting() []error {
	fflags := []cluster.Feature{
		{FeatureKey: featureflags.OverlayRouting},
	}
	_, errs := featureflags.Validate(fflags)

	if len(errs) != 0 {
		return errs
	}

	errs = featureflags.Update(fflags)

	if len(errs) != 0 {
		return errs
	}
	return nil
}

func TestRoutingConfigCreateDelete(t *testing.T) {
	name := "testCfg"

	// enable overlayrouting feature
	errs := enableOverlayRouting()

	if len(errs) != 0 {
		t.Fatalf("TestRoutingConfigCreateDelete: enableOverlayRouting failed: %v", errs)
		return
	}

	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	obj := &RoutingConfigState{}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create virtual router
	_, err = createVirtualRouter(t, stateMgr, "default", "default")
	AssertOk(t, err, "Error creating the virtual router")

	// create routingconfig
	rtcfg, err := createRoutingConfig(stateMgr, "default", name, "100.1.1.1")
	AssertOk(t, err, "Error creating RoutingConfig")

	// verify we can find the routingconfig
	AssertEventually(t, func() (bool, interface{}) {
		obj, err = smgrRoute.FindRoutingConfig("", "default", name)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error finding routingconfig %v\n", err)
		return false, nil
	}, "routingconfig not foud", "1ms", "1s")
	AssertEquals(t, obj.RoutingConfig.Spec.BGPConfig.Neighbors[0].IPAddress, rtcfg.Spec.BGPConfig.Neighbors[0].IPAddress, "routingconfig params did not match")

	// update the routingconfig
	version, _ := strconv.Atoi(obj.RoutingConfig.GenerationID)
	//version := 1
	rtcfg.GenerationID = strconv.Itoa(version + 1)
	rtcfg.Spec.BGPConfig.Neighbors[0].IPAddress = "101.1.1.1"
	err = stateMgr.ctrler.RoutingConfig().Update(rtcfg)
	AssertOk(t, err, "Error updating routingconfig")

	// verify update went through
	AssertEventually(t, func() (bool, interface{}) {
		obj, err = smgrRoute.FindRoutingConfig("", "default", name)
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error finding routingconfig %v\n", err)
		return false, nil
	}, "routingconfig not foud", "1ms", "1s")
	AssertEquals(t, obj.RoutingConfig.Spec.BGPConfig.Neighbors[0].IPAddress, rtcfg.Spec.BGPConfig.Neighbors[0].IPAddress, "routingconfig params did not match")

	// delete the routingconfig
	err = stateMgr.ctrler.RoutingConfig().Delete(rtcfg)
	AssertOk(t, err, "Error deleting routingconfig")

	// verify the routingconfig is deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrRoute.FindRoutingConfig("", "default", name)
		if err != nil {
			return true, nil
		}
		fmt.Printf("Routingconfig still found after deleting %v\n", err)
		return false, nil
	}, "routingconfig still foud", "1ms", "1s")
}

func TestModuleObject(t *testing.T) {
	// create network state manager
	smgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	oldObj := &ctkit.Module{}
	newObj := &diagapi.Module{}
	err = smgr.OnModuleCreate(oldObj)
	AssertOk(t, err, "failed to create Module")

	err = smgr.OnModuleUpdate(oldObj, newObj)
	AssertOk(t, err, "failed to update Module")

	err = smgr.OnModuleDelete(oldObj)
	AssertOk(t, err, "failed to Delete Module")
}

func getCollectors(start, end int) (collectors []monitoring.MirrorCollector) {

	for i := start; i < end; i++ {
		collectors = append(collectors, monitoring.MirrorCollector{ExportCfg: &monitoring.MirrorExportConfig{
			Destination: getCollectorName(i),
		}})
	}

	return
}

func getCollectorName(i int) string {
	return "col-" + strconv.Itoa(i)
}

func TestMirrorCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	numCollectors := 10

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	for i := 0; i < numCollectors; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		AssertOk(t, err, "Error finding collector  ")
		Assert(t, col.pushObj != nil, "push object not set")
		Assert(t, col.refCount == 1, "push object not set")
		Assert(t, col.obj != nil, "push object not set")
		Assert(t, col.obj.Spec.Destination == getCollectorName(i), "collector name not set")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	for i := 0; i < numCollectors; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		Assert(t, err != nil, "Error finding collector  ")
		Assert(t, col == nil, "Error finding collector  ")
	}

}

func TestMirrorCreateDeleteCollectorReuse(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	numCollectors := 10

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	for i := 0; i < numCollectors; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		AssertOk(t, err, "Error finding collector  ")
		Assert(t, col.pushObj != nil, "push object not set")
		Assert(t, col.refCount == 1, "push object not set")
		Assert(t, col.obj != nil, "push object not set")
		Assert(t, col.obj.Spec.Destination == getCollectorName(i), "collector name not set")
	}

	_, err = createMirror(stateMgr, "default", "testMirror1", collectors, labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	for i := 0; i < numCollectors; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		AssertOk(t, err, "Error finding collector  ")
		Assert(t, col.pushObj != nil, "push object not set")
		Assert(t, col.refCount == 2, "push object not set")
		Assert(t, col.obj != nil, "push object not set")
		Assert(t, col.obj.Spec.Destination == getCollectorName(i), "collector name not set")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	for i := 0; i < numCollectors; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		AssertOk(t, err, "Error finding collector  ")
		Assert(t, col.pushObj != nil, "push object not set")
		Assert(t, col.refCount == 1, "push object not set")
		Assert(t, col.obj != nil, "push object not set")
		Assert(t, col.obj.Spec.Destination == getCollectorName(i), "collector name not set")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror1", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	for i := 0; i < numCollectors; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		Assert(t, err != nil, "Error finding collector  ")
		Assert(t, col == nil, "Error finding collector  ")
	}

}

func TestMirrorCreateUpdateDeleteCollector(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	numCollectors := 10

	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	for i := 0; i < numCollectors; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		AssertOk(t, err, "Error finding collector  ")
		Assert(t, col.pushObj != nil, "push object not set")
		Assert(t, col.refCount == 1, "push object not set")
		Assert(t, col.obj != nil, "push object not set")
		Assert(t, col.obj.Spec.Destination == getCollectorName(i), "collector name not set")
	}

	_, err = createMirror(stateMgr, "default", "testMirror1", collectors, labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	for i := 0; i < numCollectors; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		AssertOk(t, err, "Error finding collector  ")
		Assert(t, col.pushObj != nil, "push object not set")
		Assert(t, col.refCount == 2, "push object not set")
		Assert(t, col.obj != nil, "push object not set")
		Assert(t, col.obj.Spec.Destination == getCollectorName(i), "collector name not set")
	}

	//Add new collectors for same mirror
	newCollectors := getCollectors(numCollectors, numCollectors+10)
	_, err = updateMirror(stateMgr, "default", "testMirror1", newCollectors, labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	for i := numCollectors; i < numCollectors+10; i++ {
		fmt.Printf("Checking collector %v", getCollectorName(i))
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		AssertOk(t, err, "Error finding collector  ")
		Assert(t, col.pushObj != nil, "push object not set")
		Assert(t, col.refCount == 1, "push object not set")
		Assert(t, col.obj != nil, "push object not set")
		Assert(t, col.obj.Spec.Destination == getCollectorName(i), "collector name not set")
	}

	for i := 0; i < numCollectors; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		AssertOk(t, err, "Error finding collector  ")
		Assert(t, col.pushObj != nil, "push object not set")
		Assert(t, col.refCount == 1, "push object not set")
		Assert(t, col.obj != nil, "push object not set")
		Assert(t, col.obj.Spec.Destination == getCollectorName(i), "collector name not set")
	}

	//Remove some and add new ones
	newCollectors = getCollectors(0, numCollectors+5)
	_, err = updateMirror(stateMgr, "default", "testMirror1", newCollectors, labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	for i := numCollectors; i < numCollectors+5; i++ {
		fmt.Printf("Checking collector %v", getCollectorName(i))
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		AssertOk(t, err, "Error finding collector  ")
		Assert(t, col.pushObj != nil, "push object not set")
		Assert(t, col.refCount == 1, "push object not set")
		Assert(t, col.obj != nil, "push object not set")
		Assert(t, col.obj.Spec.Destination == getCollectorName(i), "collector name not set")
	}

	for i := numCollectors + 5; i < numCollectors+10; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		Assert(t, err != nil, "Error finding collector  ")
		Assert(t, col == nil, "Error finding collector  ")
	}

	for i := 0; i < numCollectors; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		AssertOk(t, err, "Error finding collector  ")
		Assert(t, col.pushObj != nil, "push object not set")
		Assert(t, col.refCount == 2, "push object not set")
		Assert(t, col.obj != nil, "push object not set")
		Assert(t, col.obj.Spec.Destination == getCollectorName(i), "collector name not set")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror1", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror1")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	for i := 0; i < numCollectors; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		AssertOk(t, err, "Error finding collector  ")
		Assert(t, col.pushObj != nil, "push object not set")
		Assert(t, col.refCount == 1, "push object not set")
		Assert(t, col.obj != nil, "push object not set")
		Assert(t, col.obj.Spec.Destination == getCollectorName(i), "collector name not set")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	for i := 0; i < numCollectors+10; i++ {
		col, err := smgrMirrorInterface.findCollector("default", "default", getCollectorName(i))
		Assert(t, err != nil, "Error finding collector  ")
		Assert(t, col == nil, "Error finding collector  ")
	}

}

func createDSC(stateMgr *Statemgr, dscName string, mac string) (*cluster.DistributedServiceCard, error) {
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: dscName,
		},
		Spec: cluster.DistributedServiceCardSpec{},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC: mac,
		},
	}

	// create the smartNic
	err := stateMgr.ctrler.DistributedServiceCard().Create(&snic)

	return &snic, err
}

func genMACAddresses(count int) []string {
	var macAddresses []string
	macAddress := make(map[uint64]bool)
	for i := 0; i < count; i++ {
		for true {
			num := uint64(rand.Int63n(math.MaxInt64))
			if _, ok := macAddress[num]; !ok {
				b := make([]byte, 8)
				macAddress[num] = true
				binary.BigEndian.PutUint64(b, num)
				b[0] = (b[0] | 2) & 0xfe
				mac := fmt.Sprintf("%02x:%02x:%02x:%02x:%02x:%02x", b[0], b[1], b[2], b[3], b[4], b[5])
				macAddresses = append(macAddresses, mac)
				break
			}
		}
	}

	return macAddresses
}

func createsDSCs(stateMgr *Statemgr, start, end int) []*cluster.DistributedServiceCard {

	dscs := []*cluster.DistributedServiceCard{}
	macs := genMACAddresses(end - start + 1)
	for i := start; i < end; i++ {
		id := strconv.Itoa(i)
		dsc, _ := createDSC(stateMgr, "dsc"+id, macs[i])
		dscs = append(dscs, dsc)
	}

	return dscs
}

func TestNetworkInterfaceCreateDelete(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	dscs := createsDSCs(stateMgr, 0, 1)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	_, err = createNetworkInterface(stateMgr, "intf1", dscs[0].Status.PrimaryMAC, labels.Set{"env": "production", "app": "procurement"})
	AssertOk(t, err, "Error creating interface ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrNetworkInterface.FindNetworkInterface("intf1")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Interface not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.findInterfacesByLabel(labels.Set{"env": "production", "app": "procurement"})
	AssertOk(t, err, "Error creating interface ")
	Assert(t, len(intfs) == 1, "Number of interfaces don't match")

	intfs, err = smgrNetworkInterface.findInterfacesByLabel(labels.Set{"env": "production1", "app": "procurement"})
	AssertOk(t, err, "Error creating interface ")
	Assert(t, len(intfs) == 0, "Number of interfaces don't match")

	_, err = deleteNetworkInterface(stateMgr, "intf1", labels.Set{"env": "production", "app": "procurement"})
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrNetworkInterface.FindNetworkInterface("intf1")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Interface session found", "1ms", "1s")

}

func TestNetworkInterfaceCreateDeleteMultiple(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 20

	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}
	label2 := map[string]string{
		"env": "production1", "app": "procurement1",
	}
	for i := 0; i < numOfIntfs; i++ {

		intf := "intf" + strconv.Itoa(i)

		_, err = createNetworkInterface(stateMgr, intf, dscs[i].Status.PrimaryMAC, label1)
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(intf)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")

		intfs, err := smgrNetworkInterface.findInterfacesByLabel(label1)
		AssertOk(t, err, "Error creating interface ")
		Assert(t, len(intfs) == i+1, "Number of interfaces don't match")

		intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(label1))
		AssertOk(t, err, "Error creating interface ")
		Assert(t, len(intfs) == i+1, "Number of interfaces don't match")

		intfs, err = smgrNetworkInterface.findInterfacesByLabel(label2)
		AssertOk(t, err, "Error creating interface ")
		Assert(t, len(intfs) == 0, "Number of interfaces don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		intf := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, intf, label1)
		AssertOk(t, err, "Error creating mirror session ")

		intfs, err := smgrNetworkInterface.findInterfacesByLabel(label1)
		AssertOk(t, err, "Error creating interface ")
		Assert(t, len(intfs) == numOfIntfs-(i+1), "Number of interfaces don't match")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(intf)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")
	}

}

func TestNetworkInterfaceCreateDeleteMultipleLabelChange(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 20
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}
	label2 := map[string]string{
		"env": "production1", "app": "procurement1",
	}
	for i := 0; i < numOfIntfs; i++ {

		intf := "intf" + strconv.Itoa(i)

		_, err = createNetworkInterface(stateMgr, intf, dscs[i].Status.PrimaryMAC, label1)
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(intf)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")

		intfs, err := smgrNetworkInterface.findInterfacesByLabel(label1)
		AssertOk(t, err, "Error creating interface ")
		Assert(t, len(intfs) == i+1, "Number of interfaces don't match")

		intfs, err = smgrNetworkInterface.findInterfacesByLabel(label2)
		AssertOk(t, err, "Error creating interface ")
		Assert(t, len(intfs) == 0, "Number of interfaces don't match")
	}

	for i := 0; i < numOfIntfs; i++ {

		intf := "intf" + strconv.Itoa(i)

		_, err = updateNetworkInterface(stateMgr, intf, dscs[i].Status.PrimaryMAC, label2)
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(intf)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")

		intfs, err := smgrNetworkInterface.findInterfacesByLabel(label1)
		AssertOk(t, err, "Error creating interface ")
		log.Infof("Number of interfaces %v", len(intfs))
		Assert(t, len(intfs) == numOfIntfs-(i+1), "Number of interfaces don't match")

		intfs, err = smgrNetworkInterface.findInterfacesByLabel(label2)
		AssertOk(t, err, "Error creating interface ")
		Assert(t, len(intfs) == i+1, "Number of interfaces don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		intf := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, intf, label2)
		AssertOk(t, err, "Error creating mirror session ")

		intfs, err := smgrNetworkInterface.findInterfacesByLabel(label2)
		AssertOk(t, err, "Error creating interface ")
		Assert(t, len(intfs) == numOfIntfs-(i+1), "Number of interfaces don't match")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(intf)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")
	}

}

func TestMirrorCreateDeleteWithNetworkInterface(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 10
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 1
	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 1, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == 1, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}

func TestMirrorCreateUpdateLabelWithNetworkInterface(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 10
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	label2 := map[string]string{
		"env1": "production", "app2": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 10
	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == numCollectors, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == numCollectors, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	_, err = updateMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("CHECK %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}

func TestMirrorCreateUpdateCollectorsWithNetworkInterface(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 10
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 10
	newCollectorsCnt := 10
	collectors := getCollectors(0, numCollectors)
	newCollectors := getCollectors(numCollectors, numCollectors+newCollectorsCnt)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == numCollectors, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == numCollectors, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	_, err = updateMirror(stateMgr, "default", "testMirror", newCollectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	_, err = updateMirror(stateMgr, "default", "testMirror", newCollectors, nil)
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("CHECK %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}

func TestMirrorCreateUpdateLaterLabelWithNetworkInterface(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 10
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 10
	newCollectorsCnt := 10
	collectors := getCollectors(0, numCollectors)
	newCollectors := getCollectors(numCollectors, numCollectors+newCollectorsCnt)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, nil)
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	_, err = updateMirror(stateMgr, "default", "testMirror", newCollectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}

func TestMirrorCreateUpdateLabelCollectorsWithNetworkInterface(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 10
	numOfIntfs2 := 10
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	label2 := map[string]string{
		"env1": "production", "app1": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	for i := 0; i < numOfIntfs2; i++ {
		name := "intf-1-" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label2))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 10
	newCollectorsCnt := 10
	collectors := getCollectors(0, numCollectors)
	newCollectors := getCollectors(numCollectors, numCollectors+newCollectorsCnt)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == numCollectors, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == numCollectors, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	//label2 has not collectors yet
	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	_, err = updateMirror(stateMgr, "default", "testMirror", newCollectors, labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	//label1 collectors need to be removed
	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	//label2 collectors need to be addeded
	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}

func TestMirrorCreateUpdateLabelCollectorsWithNetworkInterfaceSame(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 10
	numOfIntfs2 := 10
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	label1 := map[string]string{
		"key": "value", "env": "production",
	}

	label2 := map[string]string{
		"key": "value", "app": "procurement",
	}

	//label3 is common factor
	label3 := map[string]string{
		"key": "value",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	for i := 0; i < numOfIntfs2; i++ {
		name := "intf-1-" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label2))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 10
	newCollectorsCnt := 10
	collectors := getCollectors(0, numCollectors)
	newCollectors := getCollectors(numCollectors, numCollectors+newCollectorsCnt)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == numCollectors, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == numCollectors, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	//label2 has not collectors yet
	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	_, err = updateMirror(stateMgr, "default", "testMirror", newCollectors, labels.SelectorFromSet(labels.Set(label3)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	//label1 collectors need to be removed
	//label2 collectors need to be addeded
	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	//label2 collectors need to be addeded
	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}

func TestNetworkInterfaceUpdateLabelWithMirror(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 1
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	label2 := map[string]string{
		"env1": "production", "app1": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label2))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 1
	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == 0, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	//Now change the label on interfaces

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = updateNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error updating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == numCollectors, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == numCollectors, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	//Now change the label on interfaces
	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = updateNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label2))
		AssertOk(t, err, "Error updating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == 0, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

}

func TestNetworkInterfaceUpdateLabelSwapWithMirror(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 1
	numOfIntfs2 := 1
	dscs := createsDSCs(stateMgr, 0, numOfIntfs+numOfIntfs2)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	label1 := map[string]string{
		"key": "value", "env": "production",
	}

	label2 := map[string]string{
		"key": "value", "app": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	for i := 0; i < numOfIntfs2; i++ {
		name := "intf-1-" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[numOfIntfs+i].Status.PrimaryMAC, labels.Set(label2))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 10
	newCollectorsCnt := 5
	collectors := getCollectors(0, numCollectors)
	newCollectors := getCollectors(numCollectors, numCollectors+newCollectorsCnt)
	mirrorSesssion, err := createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	mirrorSesssion1, err := createMirror(stateMgr, "default", "testMirror1", newCollectors, labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == numCollectors, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, intf.mirrorSessions[0] == mirrorSesssion.Name, "Mirror session name did not match")
		Assert(t, len(intf.rxCollectors) == numCollectors, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		stringSliceEqual(intf.NetworkInterfaceState.Status.MirroSessions, []string{"testMirror"})
		Assert(t, ok, "Collector not present")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error find interfaces")
	log.Infof("Number of interfaces %v %v", len(intfs), numOfIntfs)
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, intf.mirrorSessions[0] == mirrorSesssion1.Name, "Mirror session name did not match")
		Assert(t, len(intf.rxCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	//Now add label1 to label2

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = updateNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label2))
		AssertOk(t, err, "Error updating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs+numOfIntfs2, "Number of interfaces don't match")

	//label2 has not collectors yet
	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, intf.mirrorSessions[0] == mirrorSesssion1.Name, "Mirror session name did not match")
		Assert(t, len(intf.rxCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
		stringSliceEqual(intf.NetworkInterfaceState.Status.MirroSessions, []string{"testMirror1"})
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == 0, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs2; i++ {
		name := "intf-1-" + strconv.Itoa(i)
		_, err = updateNetworkInterface(stateMgr, name, dscs[numOfIntfs+i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error updating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs2, "Number of interfaces don't match")

	//label2 has not collectors yet
	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, intf.mirrorSessions[0] == mirrorSesssion1.Name, "Mirror session name did not match")
		Assert(t, len(intf.rxCollectors) == newCollectorsCnt, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
		stringSliceEqual(intf.NetworkInterfaceState.Status.MirroSessions, []string{"testMirror1"})
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == numCollectors, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		log.Infof("Mirror session name %v %v", intf.mirrorSessions[0], mirrorSesssion.Name)
		Assert(t, intf.mirrorSessions[0] == mirrorSesssion.Name, "Mirror session name did not match")
		Assert(t, len(intf.rxCollectors) == numCollectors, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	//Delete the mirror session1

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror1", labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

}

type watchWrapper struct {
	watcher  *memdb.Watcher
	evtsRcvd *eventsMap
	evtsExp  *eventsMap
	ctx      context.Context
	cancel   context.CancelFunc
}

type eventsMap struct {
	sync.Mutex
	evKindMap map[memdb.EventType]map[string]int
}

var watchMap map[string]*watchWrapper

func ResetWatchMap() {
	watchMap = make(map[string]*watchWrapper)
}

func StartWatch(context context.Context, w *memdb.Watcher) {

	for true {

		// verify we get a watch event
		select {
		case evt, ok := <-w.Channel:
			if ok {
				watchMap[w.Name].evtsRcvd.Update(evt, evt.Obj.GetObjectKind())
			}
		case <-context.Done():
			log.Infof("Stopped watcher")
			return
		}
	}

}

func addWatcher(watcher *memdb.Watcher) *watchWrapper {
	evtRcvd := &eventsMap{}
	evtExp := &eventsMap{}
	evtRcvd.Reset()
	ctx, cancel := context.WithCancel(context.Background())
	watchMap[watcher.Name] = &watchWrapper{ctx: ctx, cancel: cancel}
	watchMap[watcher.Name].evtsRcvd = evtRcvd
	watchMap[watcher.Name].evtsExp = evtExp
	watchMap[watcher.Name].watcher = watcher
	go StartWatch(ctx, watcher)
	return watchMap[watcher.Name]
}

func (e *eventsMap) Reset() {
	e.Lock()
	defer e.Unlock()
	e.evKindMap = make(map[memdb.EventType]map[string]int)
	e.evKindMap[memdb.CreateEvent] = make(map[string]int)
	e.evKindMap[memdb.UpdateEvent] = make(map[string]int)
	e.evKindMap[memdb.DeleteEvent] = make(map[string]int)
}

func (e *eventsMap) Update(evt memdb.Event, kind string) {
	e.Lock()
	defer e.Unlock()
	kindMap, _ := e.evKindMap[evt.EventType]
	kindMap[kind]++
}

func stopWatchers(t *testing.T, watchers []*watchWrapper) {

	// setup concurrent watches, registration should succeed now
	for _, watch := range watchers {
		watch.cancel()
	}
}

func (e *eventsMap) Equal(other *eventsMap) bool {
	e.Lock()
	defer e.Unlock()

	for evt, kindMap := range e.evKindMap {
		otherKindMap, _ := other.evKindMap[evt]
		if len(otherKindMap) != len(kindMap) {
			//Still check from
			if len(otherKindMap) < len(kindMap) {
				tmp := kindMap
				kindMap = otherKindMap
				otherKindMap = tmp

			}
			for kind, cnt := range otherKindMap {
				if val, ok := kindMap[kind]; !ok {
					if cnt != 0 {
						return false
					}
				} else if cnt != val {
					return false
				}

			}
		}
		for kind, cnt := range kindMap {
			if otherKindMap[kind] != cnt {
				return false
			}
		}
	}
	return true
}

func (e *eventsMap) Diff(exp *eventsMap) {
	e.Lock()
	defer e.Unlock()

	for evt, kindMap := range e.evKindMap {
		expKindMap, _ := exp.evKindMap[evt]
		for kind, cnt := range kindMap {
			expKindCnt, ok := expKindMap[kind]
			if !ok {
				log.Infof("Event : %v Kind : %v Exp : %v Actual : %v", evt, kind, expKindCnt, cnt)

			} else if expKindCnt != cnt {
				log.Infof("Event : %v Kind : %v Exp : %v Actual : %v", evt, kind, expKindCnt, cnt)
			}
		}
		//Check for kinds not present but where expected
		for kind, cnt := range expKindMap {
			otherKindCnt, ok := kindMap[kind]
			if !ok {
				log.Infof("Event : %v Kind : %v Exp : %v Actual : %v", evt, kind, cnt, otherKindCnt)
				log.Infof("MAP : %v ", e.evKindMap)
			}
		}
	}
}

func verifyEvObjects(t *testing.T, watch *watchWrapper,
	duration time.Duration) error {

	timedOutEvent := time.After(duration)
	for true {
		select {
		case <-timedOutEvent:
			watch.evtsRcvd.Diff(watch.evtsExp)
			log.Infof("fail Matched...")
			return errors.New("Event expectation failed")
		default:
			if watch.evtsRcvd.Equal(watch.evtsExp) {
				return nil
			}
			time.Sleep(50 * time.Millisecond)

		}
	}
	return nil
}

func TestWatcherWithMirrorCreateDelete(t *testing.T) {
	// create network state manager
	ResetWatchMap()
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 1
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	//Start watchers

	watchers := []*watchWrapper{}
	for _, dsc := range dscs {
		watcher := &memdb.Watcher{Name: dsc.Status.PrimaryMAC}
		watcher.Channel = make(chan memdb.Event, (1000))
		watchWrap := addWatcher(watcher)
		watchers = append(watchers, watchWrap)
		err = stateMgr.mbus.WatchObjects("Collector", watcher)
		AssertOk(t, err, "Error watching")
		err = stateMgr.mbus.WatchObjects("Interface", watcher)
		AssertOk(t, err, "Error watching")
	}
	defer stopWatchers(t, watchers)

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 1
	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 1, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == 1, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Collector"] = numCollectors
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Interface"] = 1
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	errs := make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		watcher.evtsExp.evKindMap[memdb.DeleteEvent]["Collector"] = numCollectors
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	errs = make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}

func TestWatcherWithMirrorCreateDeleteMultipleTimes(t *testing.T) {
	// create network state manager
	ResetWatchMap()
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 1
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	//Start watchers

	watchers := []*watchWrapper{}
	for _, dsc := range dscs {
		watcher := &memdb.Watcher{Name: dsc.Status.PrimaryMAC}
		watcher.Channel = make(chan memdb.Event, (1000))
		watchWrap := addWatcher(watcher)
		watchers = append(watchers, watchWrap)
		err = stateMgr.mbus.WatchObjects("Collector", watcher)
		AssertOk(t, err, "Error watching")
		err = stateMgr.mbus.WatchObjects("Interface", watcher)
		AssertOk(t, err, "Error watching")
	}
	defer stopWatchers(t, watchers)

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	for iter := 0; iter < 3; iter++ {
		numCollectors := 1
		collectors := getCollectors(0, numCollectors)
		for _, watcher := range watchers {
			watcher.evtsRcvd.Reset()
		}
		_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Mirror session not found", "1ms", "1s")

		intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
		AssertOk(t, err, "Error find interfaces")
		Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

		for _, intf := range intfs {
			log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
			Assert(t, len(intf.txCollectors) == 1, "Number of collectors don't match")
			_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
			Assert(t, ok, "Collector not present")
			Assert(t, len(intf.rxCollectors) == 1, "Number of collectors don't match")
			_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
			Assert(t, ok, "Collector not present")
		}

		for _, watcher := range watchers {
			watcher.evtsExp.Reset()
			watcher.evtsExp.evKindMap[memdb.CreateEvent]["Collector"] = numCollectors
			watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
		}

		errs := make(chan error, len(watchers))
		for _, watcher := range watchers {
			watcher := watcher
			go func() {
				errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
			}()

		}

		for _ = range watchers {
			err := <-errs
			AssertOk(t, err, "Error verifying objects")
		}

		for _, watcher := range watchers {
			watcher.evtsExp.Reset()
			watcher.evtsRcvd.Reset()
			watcher.evtsExp.evKindMap[memdb.DeleteEvent]["Collector"] = numCollectors
			watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
		}

		_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Mirror session found", "1ms", "1s")

		errs = make(chan error, len(watchers))
		for _, watcher := range watchers {
			watcher := watcher
			go func() {
				errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
			}()

		}

		for _ = range watchers {
			err := <-errs
			AssertOk(t, err, "Error verifying objects")
		}

		intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
		AssertOk(t, err, "Error find interfaces")
		Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

		for _, intf := range intfs {
			log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
			Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
			Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
		}
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}

func TestWatcherWithMirrorCreateFakeUpdate(t *testing.T) {
	// create network state manager
	ResetWatchMap()
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 1
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")
	//Start watchers

	watchers := []*watchWrapper{}
	for _, dsc := range dscs {
		watcher := &memdb.Watcher{Name: dsc.Status.PrimaryMAC}
		watcher.Channel = make(chan memdb.Event, (1000))
		watchWrap := addWatcher(watcher)
		watchers = append(watchers, watchWrap)
		err = stateMgr.mbus.WatchObjects("Collector", watcher)
		AssertOk(t, err, "Error watching")
		err = stateMgr.mbus.WatchObjects("Interface", watcher)
		AssertOk(t, err, "Error watching")
	}
	defer stopWatchers(t, watchers)

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	label2 := map[string]string{
		"env1": "production1", "app1": "procurement1",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 1
	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == numCollectors, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == numCollectors, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")

		stringSliceEqual(intf.NetworkInterfaceState.Status.MirroSessions, []string{"testMirror"})
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Collector"] = numCollectors
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Interface"] = 1
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	errs := make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Collector"] = 0
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Interface"] = 0
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 0
	}

	//Lets up update the mirror

	//Add new collectors for same mirror
	_, err = updateMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	errs = make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	_, err = updateMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	errs = make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		watcher.evtsExp.evKindMap[memdb.DeleteEvent]["Collector"] = numCollectors
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 0
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	errs = make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}
}

func TestWatcherWithMirrorCreateUpdateDelete(t *testing.T) {
	// create network state manager
	ResetWatchMap()
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 10
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")
	//Start watchers

	watchers := []*watchWrapper{}
	for _, dsc := range dscs {
		watcher := &memdb.Watcher{Name: dsc.Status.PrimaryMAC}
		watcher.Channel = make(chan memdb.Event, (1000))
		watchWrap := addWatcher(watcher)
		watchers = append(watchers, watchWrap)
		err = stateMgr.mbus.WatchObjects("Collector", watcher)
		AssertOk(t, err, "Error watching")
		err = stateMgr.mbus.WatchObjects("Interface", watcher)
		AssertOk(t, err, "Error watching")
	}
	defer stopWatchers(t, watchers)

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 10
	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == numCollectors, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == numCollectors, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")

		stringSliceEqual(intf.NetworkInterfaceState.Status.MirroSessions, []string{"testMirror"})
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Collector"] = numCollectors
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Interface"] = 1
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	errs := make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	newCollectorsCnt := 5
	newCollectors := getCollectors(numCollectors, numCollectors+newCollectorsCnt)
	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		watcher.evtsExp.evKindMap[memdb.DeleteEvent]["Collector"] = numCollectors
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Collector"] = newCollectorsCnt
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	//Lets up update the mirror

	//Add new collectors for same mirror
	_, err = updateMirror(stateMgr, "default", "testMirror", newCollectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	errs = make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		watcher.evtsExp.evKindMap[memdb.DeleteEvent]["Collector"] = newCollectorsCnt
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	errs = make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}

func TestWatcherWithMirrorCreateUpdateRemoveCollector(t *testing.T) {
	// create network state manager
	ResetWatchMap()
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 1
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")
	//Start watchers

	watchers := []*watchWrapper{}
	for _, dsc := range dscs {
		watcher := &memdb.Watcher{Name: dsc.Status.PrimaryMAC}
		watcher.Channel = make(chan memdb.Event, (1000))
		watchWrap := addWatcher(watcher)
		watchers = append(watchers, watchWrap)
		err = stateMgr.mbus.WatchObjects("Collector", watcher)
		AssertOk(t, err, "Error watching")
		err = stateMgr.mbus.WatchObjects("Interface", watcher)
		AssertOk(t, err, "Error watching")
	}
	defer stopWatchers(t, watchers)

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 1
	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == numCollectors, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == numCollectors, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")

		stringSliceEqual(intf.NetworkInterfaceState.Status.MirroSessions, []string{"testMirror"})
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Collector"] = numCollectors
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Interface"] = 1
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	errs := make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	newCollectorsCnt := 1
	newCollectors := getCollectors(numCollectors, numCollectors+newCollectorsCnt)
	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Collector"] = newCollectorsCnt
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	allCollectors := make([]monitoring.MirrorCollector, len(collectors))
	copy(allCollectors, collectors)
	allCollectors = append(allCollectors, newCollectors...)
	//Lets up update the mirror

	//Add new collectors for same mirror
	_, err = updateMirror(stateMgr, "default", "testMirror", allCollectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	errs = make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		watcher.evtsExp.evKindMap[memdb.DeleteEvent]["Collector"] = newCollectorsCnt
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		watcher.evtsExp.evKindMap[memdb.DeleteEvent]["Collector"] = numCollectors
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	_, err = updateMirror(stateMgr, "default", "testMirror", newCollectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	errs = make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for _, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		watcher.evtsExp.evKindMap[memdb.DeleteEvent]["Collector"] = newCollectorsCnt
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	errs = make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}
func TestWatcherWithMirrorCreateUpdateDeleteDifferentInterfaces(t *testing.T) {
	// create network state manager
	ResetWatchMap()
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 10
	numOfIntfs2 := 10
	dscs := createsDSCs(stateMgr, 0, numOfIntfs+numOfIntfs2)
	Assert(t, len(dscs) != 0, "Error creating the dscs")
	//Start watchers

	watchers := []*watchWrapper{}
	for _, dsc := range dscs {
		watcher := &memdb.Watcher{Name: dsc.Status.PrimaryMAC}
		watcher.Channel = make(chan memdb.Event, (1000))
		watchWrap := addWatcher(watcher)
		watchers = append(watchers, watchWrap)
		err = stateMgr.mbus.WatchObjects("Collector", watcher)
		AssertOk(t, err, "Error watching")
		err = stateMgr.mbus.WatchObjects("Interface", watcher)
		AssertOk(t, err, "Error watching")
	}
	defer stopWatchers(t, watchers)

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	label2 := map[string]string{
		"env1": "production", "app1": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	for i := 0; i < numOfIntfs2; i++ {
		name := "intf-1-" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i+numOfIntfs].Status.PrimaryMAC, labels.Set(label2))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 10
	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == numCollectors, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == numCollectors, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	for index, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsExp.evKindMap[memdb.CreateEvent]["Interface"] = 1
		if index < numOfIntfs {
			watcher.evtsExp.evKindMap[memdb.CreateEvent]["Collector"] = numCollectors
			watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
		}
	}

	errs := make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	//newCollectorsCnt := 5
	//newCollectors := getCollectors(numCollectors, numCollectors+newCollectorsCnt)
	for i, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		//watcher.evtsExp.evKindMap[memdb.DeleteEvent]["Collector"] = numCollectors
		if i >= numOfIntfs {
			watcher.evtsExp.evKindMap[memdb.CreateEvent]["Collector"] = numCollectors
		}
		watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
	}

	//Lets up update the mirror

	//Add new collectors for same mirror
	_, err = updateMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label2)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	errs = make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	for i, watcher := range watchers {
		watcher.evtsExp.Reset()
		watcher.evtsRcvd.Reset()
		watcher.evtsExp.evKindMap[memdb.DeleteEvent]["Collector"] = numCollectors
		if i >= numOfIntfs {
			watcher.evtsExp.evKindMap[memdb.UpdateEvent]["Interface"] = 1
		}
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	errs = make(chan error, len(watchers))
	for _, watcher := range watchers {
		watcher := watcher
		go func() {
			errs <- verifyEvObjects(t, watchMap[watcher.watcher.Name], time.Duration(1*time.Second))
		}()

	}

	for _ = range watchers {
		err := <-errs
		AssertOk(t, err, "Error verifying objects")
	}

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}

func TestM(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	// create tenant
	err = createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	numOfIntfs := 10
	dscs := createsDSCs(stateMgr, 0, numOfIntfs)
	Assert(t, len(dscs) != 0, "Error creating the dscs")

	label1 := map[string]string{
		"env": "production", "app": "procurement",
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = createNetworkInterface(stateMgr, name, dscs[i].Status.PrimaryMAC, labels.Set(label1))
		AssertOk(t, err, "Error creating interface ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Interface not found", "1ms", "1s")
	}

	numCollectors := 1
	collectors := getCollectors(0, numCollectors)
	_, err = createMirror(stateMgr, "default", "testMirror", collectors, labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session not found", "1ms", "1s")

	intfs, err := smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 1, "Number of collectors don't match")
		_, ok := smgrMirrorInterface.collectors[intf.txCollectors[0]]
		Assert(t, ok, "Collector not present")
		Assert(t, len(intf.rxCollectors) == 1, "Number of collectors don't match")
		_, ok = smgrMirrorInterface.collectors[intf.rxCollectors[0]]
		Assert(t, ok, "Collector not present")
	}

	_, err = deleteMirror(stateMgr, "default", "testMirror", labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error creating mirror session ")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := smgrMirrorInterface.FindMirrorSession("default", "testMirror")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Mirror session found", "1ms", "1s")

	intfs, err = smgrNetworkInterface.getInterfacesMatchingSelector(labels.SelectorFromSet(labels.Set(label1)))
	AssertOk(t, err, "Error find interfaces")
	Assert(t, len(intfs) == numOfIntfs, "Number of interfaces don't match")

	for _, intf := range intfs {
		log.Infof("Num %v %v", len(intf.txCollectors), len(intf.rxCollectors))
		Assert(t, len(intf.txCollectors) == 0, "Number of collectors don't match")
		Assert(t, len(intf.rxCollectors) == 0, "Number of collectors don't match")
	}

	for i := 0; i < numOfIntfs; i++ {
		name := "intf" + strconv.Itoa(i)
		_, err = deleteNetworkInterface(stateMgr, name, labels.Set{"env": "production", "app": "procurement"})
		AssertOk(t, err, "Error creating mirror session ")

		AssertEventually(t, func() (bool, interface{}) {
			_, err := smgrNetworkInterface.FindNetworkInterface(name)
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Interface session found", "1ms", "1s")

	}

}
func TestMain(m *testing.M) {
	// init tsdb client
	tsdbOpts := &tsdb.Opts{
		ClientName:              "npm-statemgr-test",
		Collector:               "test-collector",
		DBName:                  "default",
		SendInterval:            time.Duration(30) * time.Second,
		ConnectionRetryInterval: time.Second,
	}
	tsdb.Init(context.Background(), tsdbOpts)

	// call flag.Parse() here if TestMain uses flags
	os.Exit(m.Run())
}
