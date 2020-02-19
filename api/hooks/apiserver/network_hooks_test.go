package impl

import (
	"context"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/interfaces"
	apisrvmocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/log"
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
			Type: network.NetworkType_Routed.String(),
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

	nwif := network.NetworkInterface{
		Spec: network.NetworkInterfaceSpec{
			Type:          network.IFType_HOST_PF.String(),
			AttachTenant:  "tenant",
			AttachNetwork: "network1",
		},
	}

	errs = nh.validateNetworkIntfConfig(nwif, "v1", false, false)
	Assert(t, len(errs) == 0, "expecting to succeed [%v]", errs)

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

}

func TestPrecommitHooks(t *testing.T) {
	kvs := &mocks.FakeKvStore{}
	txn := &mocks.FakeTxn{}
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()

	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	nh := &networkHooks{
		logger: l,
	}

	nwif := network.NetworkInterface{
		Spec: network.NetworkInterfaceSpec{
			Type:          network.IFType_HOST_PF.String(),
			AttachTenant:  "tenant",
			AttachNetwork: "network1",
		},
	}

	_, kvw, err := nh.networkIntfPrecommitHook(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, nwif)
	AssertOk(t, err, "expecting to succeed")
	Assert(t, kvw, "Expecting kv write to be true")
	Assert(t, len(txn.Cmps) == 1, "expecting one comparator to be added to txn")

	txn.Cmps = nil
	txn.Ops = nil

	nwif.Spec.AttachTenant = ""
	nwif.Spec.AttachNetwork = ""
	_, kvw, err = nh.networkIntfPrecommitHook(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, nwif)
	AssertOk(t, err, "expecting to succeed")
	Assert(t, kvw, "Expecting kv write to be true")
	Assert(t, len(txn.Cmps) == 0, "expecting no comparator to be added to txn")

	txn.Cmps = nil
	txn.Ops = nil

	vrf := network.VirtualRouter{
		Spec: network.VirtualRouterSpec{
			Type: network.VirtualRouterSpec_Infra.String(),
		},
	}

	_, kvw, err = nh.createDefaultVRFRouteTable(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, vrf)
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
}
