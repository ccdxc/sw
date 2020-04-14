package impl

import (
	"context"
	"fmt"
	"math"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/workload"
	apiintf "github.com/pensando/sw/api/interfaces"
	apisrvmocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestIPAMPolicyConfig(t *testing.T) {
	logConfig := &log.Config{
		Module:      "Network-hooks",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	l := log.SetConfig(logConfig)
	service := apisrvmocks.NewFakeService()
	meth := apisrvmocks.NewFakeMethod(true)
	msg := apisrvmocks.NewFakeMessage("test.test", "/test/path", false)
	apisrvmocks.SetFakeMethodReqType(msg, meth)
	service.AddMethod("IPAMPolicy", meth)

	// Add other fake methods
	service.AddMethod("Network", meth)
	service.AddMethod("VirtualRouter", meth)
	service.AddMethod("NetworkInterface", meth)
	service.AddMethod("IPAMPolicy", meth)
	service.AddMethod("RoutingConfig", meth)

	s := &networkHooks{
		svc:    service,
		logger: l,
	}

	policy := network.IPAMPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPAMPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testPolicy",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: network.IPAMPolicySpec{
			DHCPRelay: &network.DHCPRelayPolicy{},
		},
		Status: network.IPAMPolicyStatus{},
	}

	registerNetworkHooks(service, l)
	server := &network.DHCPServer{
		IPAddress:     "100.1.1.1",
		VirtualRouter: "default",
	}

	server1 := &network.DHCPServer{
		IPAddress:     "101.1.1.1",
		VirtualRouter: "default",
	}

	policy.Spec.DHCPRelay.Servers = append(policy.Spec.DHCPRelay.Servers, server)

	ok := s.validateIPAMPolicyConfig(policy, "", false, false)
	if ok != nil {
		t.Errorf("Failed to create a good IPAMPolicy config (%s)", ok)
	}

	policy.Spec.DHCPRelay.Servers = append(policy.Spec.DHCPRelay.Servers, server1)

	ok = s.validateIPAMPolicyConfig(policy, "", false, false)
	if ok == nil {
		t.Errorf("validation passed, expecting to fail for %v", policy.Name)
	}
	l.Infof("IPAM Policy %v : Error %v", policy.Name, ok)
}

func TestValidateHooks(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	nh := &networkHooks{
		logger: l,
	}

	nw := network.Network{
		Spec: network.NetworkSpec{
			Type:   network.NetworkType_Routed.String(),
			VlanID: 1001,
		},
	}
	errs := nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting error when there is no IP config ")

	nw.Spec.IPv4Subnet = "10.1.1.0/24"
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting error when there is no IP gateway ")

	nw.Spec.IPv4Gateway = "10.1.1.1"
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) == 0, "Expecting to succeed [%v]", errs)

	nw.Spec.IPv6Subnet = "10.1.1.1"
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "v6 not supported")
	nw.Spec.IPv6Subnet = ""

	nw.Spec.RouteImportExport = &network.RDSpec{}
	nw.Spec.RouteImportExport.AddressFamily = network.BGPAddressFamily_IPv4Unicast.String()
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "expecting to fail due to RT/RT on network")

	nw.Spec.RouteImportExport.AddressFamily = network.BGPAddressFamily_L2vpnEvpn.String()
	nw.Spec.VxlanVNI = 1000
	nw.Spec.VirtualRouter = "test"
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) == 0, "expecting to succeed [%v]", errs)

	nw.Spec.IngressSecurityPolicy = []string{"xxx"}
	nw.Spec.EgressSecurityPolicy = []string{"xxx"}
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) == 0, "Expecting to succeed [%s]", errs)

	nw.Spec.IngressSecurityPolicy = []string{"xxx", "xxx1"}
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) == 0, "Expecting to succeed")

	nw.Spec.IngressSecurityPolicy = []string{"xxx", "xxx1", "xxx2"}
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting to fail")

	nw.Spec.EgressSecurityPolicy = []string{"xxx", "xxx1", "xxx2"}
	nw.Spec.IngressSecurityPolicy = []string{"xxx", "xxx1"}
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting to fail")

	nw.Spec.RouteImportExport.AddressFamily = network.BGPAddressFamily_L2vpnEvpn.String()
	nw.Spec.VxlanVNI = 0
	nw.Spec.VirtualRouter = ""
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting to fail")

	nw.Spec.VxlanVNI = 1000
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting to fail")

	nw.Spec.VxlanVNI = 1000
	nw.Spec.VirtualRouter = "default"
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting to fail")

	nw.Spec.VxlanVNI = 0
	nw.Spec.VirtualRouter = "test"
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting to fail")

	nw.Spec.VxlanVNI = 1000
	nw.Spec.VirtualRouter = "test"
	rt := &network.RouteDistinguisher{}
	rt.Type = network.RouteDistinguisher_Type0.String()
	rt.AdminValue = math.MaxUint16 + 1
	nw.Spec.RouteImportExport.ExportRTs = []*network.RouteDistinguisher{rt}
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting to fail")

	nw.Spec.RouteImportExport.ImportRTs = []*network.RouteDistinguisher{rt}
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting to fail")

	rt.Type = network.RouteDistinguisher_Type1.String()
	rt.AssignedValue = math.MaxUint16 + 1
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting to fail")

	rt.Type = network.RouteDistinguisher_Type2.String()
	rt.AssignedValue = math.MaxUint16 + 1
	errs = nh.validateNetworkConfig(nw, "v1", false, false)
	Assert(t, len(errs) != 0, "Expecting to fail")

	vrf := network.VirtualRouter{
		Spec: network.VirtualRouterSpec{
			Type: network.VirtualRouterSpec_Infra.String(),
		},
	}

	errs = nh.validateVirtualrouterConfig(vrf, "v1", false, false)
	Assert(t, len(errs) == 0, "expecting to pass")

	vrf.Spec.VxLanVNI = 900100
	errs = nh.validateVirtualrouterConfig(vrf, "v1", false, false)
	Assert(t, len(errs) != 0, "expecting to fail due to VxLAN on infra vrf")

	vrf.Spec.VxLanVNI = 0
	vrf.Spec.RouteImportExport = &network.RDSpec{}
	errs = nh.validateVirtualrouterConfig(vrf, "v1", false, false)
	Assert(t, len(errs) != 0, "expecting to fail due to RT/RT on infra vrf")

	vrf.Spec.Type = network.VirtualRouterSpec_Tenant.String()
	errs = nh.validateVirtualrouterConfig(vrf, "v1", false, false)
	Assert(t, len(errs) == 0, "expecting to succeeed")

	vrf.Spec.VxLanVNI = 90001
	errs = nh.validateVirtualrouterConfig(vrf, "v1", false, false)
	Assert(t, len(errs) == 0, "expecting to succeeed")

	vrf.Spec.RouteImportExport.AddressFamily = network.BGPAddressFamily_IPv4Unicast.String()
	errs = nh.validateVirtualrouterConfig(vrf, "v1", false, false)
	Assert(t, len(errs) != 0, "expecting to fail due to RT/RT on vrf")

	vrf.Spec.RouteImportExport.AddressFamily = network.BGPAddressFamily_L2vpnEvpn.String()
	errs = nh.validateVirtualrouterConfig(vrf, "v1", false, false)
	Assert(t, len(errs) == 0, "expecting to succeeed")

	nwif := network.NetworkInterface{
		Spec: network.NetworkInterfaceSpec{
			Type:          network.IFType_HOST_PF.String(),
			AttachTenant:  "tenant",
			AttachNetwork: "network1",
		},
	}

	errs = nh.validateNetworkIntfConfig(nwif, "v1", false, false)
	Assert(t, len(errs) == 0, "expecting to succeed [%v]", errs)

	nwif = network.NetworkInterface{
		Spec: network.NetworkInterfaceSpec{
			Type:          network.IFType_HOST_PF.String(),
			AttachTenant:  "tenant",
			AttachNetwork: "network1",
			AdminStatus:   "down",
		},
	}
	errs = nh.validateNetworkIntfConfig(nwif, "v1", false, false)
	Assert(t, len(errs) != 0, "expecting to fail")
	nwif.Spec.AdminStatus = "up"

	nwif.Spec.AttachNetwork = ""
	errs = nh.validateNetworkIntfConfig(nwif, "v1", false, false)
	Assert(t, len(errs) != 0, "expecting to fail")

	nwif.Spec.AttachNetwork = "network1"
	nwif.Spec.AttachTenant = ""
	errs = nh.validateNetworkIntfConfig(nwif, "v1", false, false)
	Assert(t, len(errs) != 0, "expecting to fail")
	nwif.Spec.AttachTenant = "tenant"

	nwif.Spec.Type = network.IFType_UPLINK_ETH.String()

	errs = nh.validateNetworkIntfConfig(nwif, "v1", false, false)
	Assert(t, len(errs) != 0, "expecting to fail")

	rtcfg := network.RoutingConfig{
		Spec: network.RoutingConfigSpec{
			BGPConfig: &network.BGPConfig{
				ASNumber: 1000,
				Neighbors: []*network.BGPNeighbor{
					{
						DSCAutoConfig:         true,
						EnableAddressFamilies: []string{"l2vpn-evpn"},
						RemoteAS:              1000,
					},
					{
						DSCAutoConfig:         true,
						EnableAddressFamilies: []string{"ipv4-unicast"},
						RemoteAS:              1000,
					},
				},
			},
		},
	}
	errs = nh.validateRoutingConfig(rtcfg, "v1", false, false)
	Assert(t, len(errs) > 0, "Expecting errors %s", errs)
	rtcfg.Spec.BGPConfig.DSCAutoConfig = true
	errs = nh.validateRoutingConfig(rtcfg, "v1", false, false)
	Assert(t, len(errs) > 0, "Expecting errors %s", errs)
	rtcfg.Spec.BGPConfig.Neighbors[1].RemoteAS = 2000
	errs = nh.validateRoutingConfig(rtcfg, "v1", false, false)
	Assert(t, len(errs) == 0, "found errors %s", errs)
	rtcfg.Spec.BGPConfig.Neighbors = append(rtcfg.Spec.BGPConfig.Neighbors, &network.BGPNeighbor{
		DSCAutoConfig:         true,
		EnableAddressFamilies: []string{"ipv4-unicast"},
		RemoteAS:              1000,
	})
	errs = nh.validateRoutingConfig(rtcfg, "v1", false, false)
	Assert(t, len(errs) > 0, "Expecting errors %s", errs)
	rtcfg.Spec.BGPConfig.Neighbors[2].EnableAddressFamilies = []string{"l2vpn-evpn"}
	errs = nh.validateRoutingConfig(rtcfg, "v1", false, false)
	Assert(t, len(errs) > 0, "Expecting errors %s", errs)

	// holdtimer and keepalive timer tests
	cases := []struct {
		holdtime  uint32
		keepalive uint32
		ok        bool
	}{
		{holdtime: 30, keepalive: 10, ok: true},
		{holdtime: 60, keepalive: 10, ok: true},
		{holdtime: 0, keepalive: 0, ok: true},
		{holdtime: 3, keepalive: 1, ok: true},
		{holdtime: 3600, keepalive: 1200, ok: true},
		{holdtime: 180, keepalive: 60, ok: true},
		{holdtime: 2, keepalive: 1, ok: false},
		{holdtime: 30, keepalive: 0, ok: false},
		{holdtime: 0, keepalive: 30, ok: false},
		{holdtime: 30, keepalive: 15, ok: false},
	}
	rtcfg.Spec.BGPConfig.Neighbors = nil
	for _, c := range cases {
		rtcfg.Spec.BGPConfig.Holdtime, rtcfg.Spec.BGPConfig.KeepaliveInterval = c.holdtime, c.keepalive
		errs = nh.validateRoutingConfig(rtcfg, "v1", false, false)
		Assert(t, c.ok && len(errs) == 0 || !c.ok && len(errs) != 0, "case [holdtime: %v / Keepalive: %v] result [%v] got errors[%v]", c.holdtime, c.keepalive, c.ok, errs)
	}

}

func TestNetworkPrecommitHooks(t *testing.T) {
	kvs := &mocks.FakeKvStore{}
	txn := &mocks.FakeTxn{}
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()

	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	nh := &networkHooks{
		logger: l,
	}

	txn.Cmps = nil
	txn.Ops = nil

	vrf := network.VirtualRouter{
		Spec: network.VirtualRouterSpec{
			Type: network.VirtualRouterSpec_Infra.String(),
		},
	}

	_, kvw, err := nh.createDefaultVRFRouteTable(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, vrf)
	AssertOk(t, err, "expecting to succeed")
	Assert(t, kvw, "Expecting kv write to be true")
	Assert(t, len(txn.Cmps) == 0, "expecting no comparator to be added to txn")
	Assert(t, len(txn.Ops) == 1, "expecting one operation to be added to txn")

	txn.Cmps = nil
	txn.Ops = nil
	_, kvw, err = nh.deleteDefaultVRFRouteTable(ctx, kvs, txn, "/test/key", apiintf.DeleteOper, false, vrf)
	AssertOk(t, err, "expecting to succeed")
	Assert(t, kvw, "Expecting kv write to be true")
	Assert(t, len(txn.Cmps) == 0, "expecting no comparator to be added to txn")
	Assert(t, len(txn.Ops) == 1, "expecting one operation to be added to txn")

	existingrtcfg := network.RoutingConfig{
		Spec: network.RoutingConfigSpec{
			BGPConfig: &network.BGPConfig{
				ASNumber: 1000,
			},
		},
	}
	kvs.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		r := into.(*network.RoutingConfig)
		*r = existingrtcfg
		return nil
	}
	rtCfg := network.RoutingConfig{
		Spec: network.RoutingConfigSpec{
			BGPConfig: &network.BGPConfig{
				ASNumber: 1000,
			},
		},
	}
	_, _, err = nh.routingConfigPreCommit(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, rtCfg)
	AssertOk(t, err, "expecting to succeed (%v)", err)
	rtCfg.Spec.BGPConfig.ASNumber = 2000
	_, _, err = nh.routingConfigPreCommit(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, rtCfg)
	Assert(t, err != nil, "expecting to fail (%v)", err)

	intf := network.NetworkInterface{
		Spec: network.NetworkInterfaceSpec{
			Type: network.IFType_LOOPBACK_TEP.String(),
		},
	}

	curIntf := network.NetworkInterface{
		Spec: network.NetworkInterfaceSpec{
			Type: network.IFType_LOOPBACK_TEP.String(),
		},
	}
	kvs.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		return fmt.Errorf("not found")
	}
	_, kvw, err = nh.checkNetworkInterfaceMutable(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, intf)
	AssertOk(t, err, "expecting to succeed")

	_, kvw, err = nh.checkNetworkInterfaceMutable(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, intf)
	Assert(t, err != nil, "expecting to fail")

	kvs.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		r := into.(*network.NetworkInterface)
		*r = curIntf
		return nil
	}

	_, kvw, err = nh.checkNetworkInterfaceMutable(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, intf)
	AssertOk(t, err, "expecting to succeed")

	curIntf.Spec.Type = network.IFType_HOST_PF.String()
	_, kvw, err = nh.checkNetworkInterfaceMutable(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, intf)
	Assert(t, err != nil, "expecting to fail")
	vrf = network.VirtualRouter{
		Spec: network.VirtualRouterSpec{
			Type: network.VirtualRouterSpec_Tenant.String(),
		},
	}

	curvrf := network.VirtualRouter{
		Spec: network.VirtualRouterSpec{
			Type: network.VirtualRouterSpec_Tenant.String(),
		},
	}
	kvs.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		return fmt.Errorf("not found")
	}
	_, kvw, err = nh.checkVirtualRouterMutableUpdate(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, vrf)
	Assert(t, err != nil, "precommit check should fail")

	kvs.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		r := into.(*network.VirtualRouter)
		*r = curvrf
		return nil
	}
	_, kvw, err = nh.checkVirtualRouterMutableUpdate(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, vrf)
	AssertOk(t, err, "precommit check failed (%s)", err)
	Assert(t, kvw, "kvwrite should be set")

	curvrf.Spec.VxLanVNI = 1000
	_, kvw, err = nh.checkVirtualRouterMutableUpdate(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, vrf)
	Assert(t, err != nil, "precommit check should fail")
	curvrf.Spec.VxLanVNI = 0

	curvrf.Spec.Type = network.VirtualRouterSpec_Infra.String()
	_, kvw, err = nh.checkVirtualRouterMutableUpdate(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, vrf)
	Assert(t, err != nil, "precommit check should fail")

	nw := network.Network{
		Spec: network.NetworkSpec{
			Type:          network.NetworkType_Routed.String(),
			VlanID:        1001,
			VirtualRouter: "test",
		},
	}
	curnw := network.Network{
		Spec: network.NetworkSpec{
			Type:          network.NetworkType_Routed.String(),
			VlanID:        1001,
			VirtualRouter: "test",
		},
	}
	kvs.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		return fmt.Errorf("not found")
	}
	_, kvw, err = nh.checkNetworkMutableFields(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, nw)
	Assert(t, err != nil, "precommit check should fail")

	kvs.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		r := into.(*network.Network)
		*r = curnw
		return nil
	}

	_, kvw, err = nh.checkNetworkMutableFields(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, nw)
	AssertOk(t, err, "precommit check failed (%s)", err)
	Assert(t, kvw, "kvwrite should be set")

	nw.Spec.VxlanVNI = 1000
	_, kvw, err = nh.checkNetworkMutableFields(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, nw)
	Assert(t, err != nil, "precommit check should fail")
	nw.Spec.VxlanVNI = 0

	nw.Spec.VlanID = 2
	_, kvw, err = nh.checkNetworkMutableFields(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, nw)
	Assert(t, err != nil, "precommit check should fail")

	nw.Spec.VlanID = 1001
	nw.Spec.Type = network.NetworkType_Bridged.String()
	_, kvw, err = nh.checkNetworkMutableFields(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, nw)
	Assert(t, err != nil, "precommit check should fail")

	nw.Spec.Type = network.NetworkType_Routed.String()
	nw.Spec.VirtualRouter = "test1"
	nw.Spec.Type = network.NetworkType_Bridged.String()
	_, kvw, err = nh.checkNetworkMutableFields(ctx, kvs, txn, "/test/key", apiintf.UpdateOper, false, nw)
	Assert(t, err != nil, "precommit check should fail")

}

func TestNetworkCreate(t *testing.T) {
	logConfig := &log.Config{
		Module:      "Network-hooks",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	l := log.SetConfig(logConfig)
	hooks := &networkHooks{
		logger: l,
	}

	kvs := &mocks.FakeKvStore{}
	txn := &mocks.FakeTxn{}
	nw := &network.Network{
		ObjectMeta: api.ObjectMeta{
			Name:   "nw",
			Tenant: "default",
		},
		Spec: network.NetworkSpec{
			Type:          network.NetworkType_Bridged.String(),
			VlanID:        1001,
			VirtualRouter: "test",
		},
	}
	newnw := &network.Network{
		ObjectMeta: api.ObjectMeta{
			Name:   "newnw",
			Tenant: "default",
		},
		Spec: network.NetworkSpec{
			Type:          network.NetworkType_Routed.String(),
			VlanID:        1001,
			VirtualRouter: "test",
		},
	}
	ctx := context.TODO()
	kvs.Listfn = func(ctx context.Context, prefix string, into runtime.Object) error {
		nwList, ok := into.(*network.NetworkList)
		if !ok {
			return fmt.Errorf("Incorrect output list type")
		}
		nwList.Items = append(nwList.Items, nw)
		return nil
	}

	// Create network
	err := kvs.Create(ctx, nw.MakeKey(string(apiclient.GroupNetwork)), nw)
	AssertOk(t, err, "network create operation failed")

	// Create another network - same vlan
	_, _, err = hooks.checkNetworkCreateConfig(ctx, kvs, txn, newnw.MakeKey(string(apiclient.GroupNetwork)),
		apiintf.CreateOper, false, *newnw)
	Assert(t, err != nil, "network create operation should have failed")
	// Create another network - different vlan
	newnw.Spec.VlanID = 2
	_, _, err = hooks.checkNetworkCreateConfig(ctx, kvs, txn, newnw.MakeKey(string(apiclient.GroupNetwork)),
		apiintf.CreateOper, false, *newnw)
	AssertOk(t, err, "network create operation failed")
}

func TestNetworkOrchestratorRemoval(t *testing.T) {
	logConfig := &log.Config{
		Module:      "Network-hooks",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	l := log.SetConfig(logConfig)
	hooks := &networkHooks{
		logger: l,
	}

	schema := runtime.GetDefaultScheme()
	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(schema)}
	kv, err := store.New(config)
	AssertOk(t, err, "Error instantiating KVStore")

	nw := &network.Network{
		ObjectMeta: api.ObjectMeta{
			Name:   "nw",
			Tenant: "default",
		},
		Spec: network.NetworkSpec{
			Type:   network.NetworkType_Bridged.String(),
			VlanID: 1001,
			Orchestrators: []*network.OrchestratorInfo{
				&network.OrchestratorInfo{
					Name:      "o1",
					Namespace: "namespace1",
				},
				&network.OrchestratorInfo{
					Name:      "o1",
					Namespace: "namespace2",
				},
				&network.OrchestratorInfo{
					Name:      "o2",
					Namespace: "namesapce1",
				},
			},
		},
	}

	ctx := context.TODO()
	// Create orch
	o1 := makeOrchObj("o1", "10.1.1.1")
	key := o1.MakeKey(string(apiclient.GroupOrchestration))
	err = kv.Create(ctx, key, o1)
	defer kv.Delete(ctx, key, nil)
	AssertOk(t, err, "kv operation failed")

	o2 := makeOrchObj("o2", "10.1.1.11")
	key = o2.MakeKey(string(apiclient.GroupOrchestration))
	err = kv.Create(ctx, key, o2)
	defer kv.Delete(ctx, key, nil)
	AssertOk(t, err, "kv operation failed")

	// Create network
	err = kv.Create(ctx, nw.MakeKey(string(apiclient.GroupNetwork)), nw)
	AssertOk(t, err, "kv operation failed")

	// Create workloads
	h1 := makeHostObj("h1", "aaaa.bbbb.cccc", "")
	key = h1.MakeKey(string(apiclient.GroupCluster))
	err = kv.Create(ctx, key, h1)
	defer kv.Delete(ctx, key, nil)
	AssertOk(t, err, "kv operation failed")

	// Even numbers belong to o1
	for i := 0; i < 5; i++ {
		wl := makeWorkloadObj(fmt.Sprintf("w%d", i), "h1", []workload.WorkloadIntfSpec{
			workload.WorkloadIntfSpec{
				MACAddress: "aaaa.bbbb.cccc",
				Network:    "nw",
			},
			workload.WorkloadIntfSpec{
				MACAddress: "aaaa.bbbb.dddd",
			},
		})
		key := wl.MakeKey(string(apiclient.GroupWorkload))
		if i%2 == 0 {
			utils.AddOrchNameLabel(wl.Labels, "o1")
			utils.AddOrchNamespaceLabel(wl.Labels, "namespace1")
		}
		err = kv.Create(ctx, key, wl)
		AssertOk(t, err, "kv operation failed")

		defer kv.Delete(ctx, key, nil)
	}

	{
		// validate remove orch config o1 - namespace1 fails
		nw = &network.Network{
			ObjectMeta: api.ObjectMeta{
				Name:   "nw",
				Tenant: globals.DefaultTenant,
			},
			Spec: network.NetworkSpec{
				Type: network.NetworkType_Bridged.String(),
				Orchestrators: []*network.OrchestratorInfo{
					&network.OrchestratorInfo{
						Name:      "o1",
						Namespace: "namespace2",
					},
					&network.OrchestratorInfo{
						Name:      "o2",
						Namespace: "namesapce1",
					},
				},
			},
		}
		key := nw.MakeKey(string(apiclient.GroupNetwork))
		_, _, err := hooks.networkOrchConfigPrecommit(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, *nw)

		// Should have failed
		Assert(t, err != nil, "orch info removal should have failed")
	}
	{
		// validate remove orch config o1 - namespace2 succeeds
		nw = &network.Network{
			ObjectMeta: api.ObjectMeta{
				Name:   "nw",
				Tenant: globals.DefaultTenant,
			},
			Spec: network.NetworkSpec{
				Type: network.NetworkType_Bridged.String(),
				Orchestrators: []*network.OrchestratorInfo{
					&network.OrchestratorInfo{
						Name:      "o1",
						Namespace: "namespace1",
					},
					&network.OrchestratorInfo{
						Name:      "o2",
						Namespace: "namesapce1",
					},
				},
			},
		}
		key := nw.MakeKey(string(apiclient.GroupNetwork))
		_, _, err := hooks.networkOrchConfigPrecommit(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, *nw)

		// Should have succeeded
		AssertOk(t, err, "orch info removal should have succeeded")
	}

	{
		// validate remove orch config o2 - namespace1 should succeed
		// Adding orch config o1 namespace3 should succeed
		nw = &network.Network{
			ObjectMeta: api.ObjectMeta{
				Name:   "nw",
				Tenant: globals.DefaultTenant,
			},
			Spec: network.NetworkSpec{
				Type: network.NetworkType_Bridged.String(),
				Orchestrators: []*network.OrchestratorInfo{
					&network.OrchestratorInfo{
						Name:      "o1",
						Namespace: "namespace1",
					},
					&network.OrchestratorInfo{
						Name:      "o1",
						Namespace: "namespace3",
					},
				},
			},
		}
		key := nw.MakeKey(string(apiclient.GroupNetwork))
		_, _, err = hooks.networkOrchConfigPrecommit(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, *nw)

		// Should have succeeded
		AssertOk(t, err, "orch info removal should have succeeded")
	}

	// Remove interfaces using o1 from workloads
	// modify even numbers
	for i := 0; i < 5; i++ {
		if i%2 == 0 {
			wl := makeWorkloadObj(fmt.Sprintf("w%d", i), "h1", []workload.WorkloadIntfSpec{
				workload.WorkloadIntfSpec{
					MACAddress: "aaaa.bbbb.dddd",
				},
			})
			utils.AddOrchNameLabel(wl.Labels, "o1")
			utils.AddOrchNamespaceLabel(wl.Labels, "namespace1")

			key := wl.MakeKey(string(apiclient.GroupWorkload))
			err = kv.Update(ctx, key, wl)
			AssertOk(t, err, "kv operation failed")
		}
	}

	{
		// validate remove orch config o1 - namespace1 succeeds
		nw = &network.Network{
			ObjectMeta: api.ObjectMeta{
				Name:   "nw",
				Tenant: globals.DefaultTenant,
			},
			Spec: network.NetworkSpec{
				Type:          network.NetworkType_Bridged.String(),
				Orchestrators: []*network.OrchestratorInfo{},
			},
		}
		key := nw.MakeKey(string(apiclient.GroupNetwork))
		_, _, err := hooks.networkOrchConfigPrecommit(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, *nw)

		// Should have succeeded
		AssertOk(t, err, "orch info removal should have succeeded")
	}

	// Delete remaining workloads
	for i := 0; i < 5; i++ {
		if i%2 != 0 {
			wl := makeWorkloadObj(fmt.Sprintf("w%d", i), "h1", nil)
			key := wl.MakeKey(string(apiclient.GroupWorkload))
			kv.Delete(ctx, key, nil)
		}
	}
}

func makeWorkloadObj(name, host string, infs []workload.WorkloadIntfSpec) *workload.Workload {
	workload := &workload.Workload{
		ObjectMeta: api.ObjectMeta{
			Name:            name,
			Tenant:          globals.DefaultTenant,
			ResourceVersion: "1",
			Labels:          map[string]string{},
		},
		TypeMeta: api.TypeMeta{
			Kind:       "Workload",
			APIVersion: "v1",
		},
		Spec: workload.WorkloadSpec{
			HostName:   host,
			Interfaces: infs,
		},
	}
	return workload
}

func TestRoutingConfigResponseWriter(t *testing.T) {
	kvs := &mocks.FakeKvStore{}
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()

	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	nh := &networkHooks{
		logger: l,
	}

	existingrtcfg := network.RoutingConfig{
		Spec: network.RoutingConfigSpec{
			BGPConfig: &network.BGPConfig{
				ASNumber: 1000,
				Neighbors: []*network.BGPNeighbor{
					{
						IPAddress:             "10.1.1.1",
						RemoteAS:              62000,
						EnableAddressFamilies: []string{network.BGPAddressFamily_L2vpnEvpn.String()},
						MultiHop:              6,
					},
					{
						IPAddress:             "10.1.1.2",
						RemoteAS:              63000,
						MultiHop:              6,
						EnableAddressFamilies: []string{network.BGPAddressFamily_L2vpnEvpn.String()},
						Password:              "testPassword",
					},
				},
			},
		},
	}

	kvs.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		r := into.(*network.RoutingConfig)
		*r = existingrtcfg
		return nil
	}

	kvs.Listfn = func(ctx context.Context, key string, into runtime.Object) error {
		to := into.(*network.RoutingConfigList)
		to.Items = []*network.RoutingConfig{&existingrtcfg}
		return nil
	}

	validateStatus := func(i interface{}) error {
		switch i.(type) {
		case network.RoutingConfig:
			in := i.(network.RoutingConfig)
			if in.Status.AuthConfigStatus[0].Status != network.BGPAuthStatus_Disabled.String() || in.Status.AuthConfigStatus[1].Status != network.BGPAuthStatus_Enabled.String() {
				return fmt.Errorf("did not match [%v]", in.Status.AuthConfigStatus)
			}
		case *network.RoutingConfig:
			in := i.(*network.RoutingConfig)
			if in.Status.AuthConfigStatus[0].Status != network.BGPAuthStatus_Disabled.String() || in.Status.AuthConfigStatus[1].Status != network.BGPAuthStatus_Enabled.String() {
				return fmt.Errorf("did not match [%v]", in.Status.AuthConfigStatus)
			}
		}
		in := i.(network.RoutingConfig)
		if in.Status.AuthConfigStatus[0].Status != network.BGPAuthStatus_Disabled.String() || in.Status.AuthConfigStatus[1].Status != network.BGPAuthStatus_Enabled.String() {
			return fmt.Errorf("did not match [%v]", in.Status.AuthConfigStatus)
		}
		return nil
	}

	validateList := func(i interface{}) error {
		in := i.(network.RoutingConfigList)
		for _, r := range in.Items {
			err := validateStatus(*r)
			if r != nil {
				return err
			}
		}
		return nil
	}
	existingrtList := network.RoutingConfigList{
		Items: []*network.RoutingConfig{&existingrtcfg},
	}

	ret, err := nh.updateAuthStatus(ctx, kvs, "network", network.RoutingConfig{}, network.RoutingConfig{}, network.RoutingConfig{}, apiintf.CreateOper)
	AssertOk(t, err, "responseWriter returned error (%s)", err)
	AssertOk(t, validateStatus(ret), "create oper response failed (%s)", err)

	ret, err = nh.updateAuthStatus(ctx, kvs, "network", network.RoutingConfig{}, network.RoutingConfig{}, network.RoutingConfig{}, apiintf.UpdateOper)
	AssertOk(t, err, "responseWriter returned error (%s)", err)
	AssertOk(t, validateStatus(ret), "create oper response failed (%s)", err)

	ret, err = nh.updateAuthStatus(ctx, kvs, "network", network.RoutingConfig{}, network.RoutingConfig{}, network.RoutingConfig{}, apiintf.GetOper)
	AssertOk(t, err, "responseWriter returned error (%s)", err)
	AssertOk(t, validateStatus(ret), "create oper response failed (%s)", err)

	ret, err = nh.updateAuthStatus(ctx, kvs, "network", existingrtList, nil, existingrtList, apiintf.ListOper)
	AssertOk(t, err, "responseWriter returned error (%s)", err)
	AssertOk(t, validateList(ret), "create oper response failed (%s)", err)
}
