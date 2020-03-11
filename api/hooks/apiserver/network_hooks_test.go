package impl

import (
	"context"
	"fmt"
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

	rtcfg := network.RoutingConfig{
		Spec: network.RoutingConfigSpec{
			BGPConfig: &network.BGPConfig{
				RouterId: "1.1.1.1",
				ASNumber: 1000,
				Neighbors: []*network.BGPNeighbor{
					{
						IPAddress:             "0.0.0.0",
						EnableAddressFamilies: []string{"evpn"},
						RemoteAS:              1000,
					},
					{
						IPAddress:             "0.0.0.0",
						EnableAddressFamilies: []string{"ipv4-unicast"},
						RemoteAS:              1000,
					},
				},
			},
		},
	}
	errs = nh.validateRoutingConfig(rtcfg, "v1", false, false)
	Assert(t, len(errs) > 0, "Expecting errors %s", errs)
	rtcfg.Spec.BGPConfig.RouterId = "0.0.0.0"
	errs = nh.validateRoutingConfig(rtcfg, "v1", false, false)
	Assert(t, len(errs) > 0, "Expecting errors %s", errs)
	rtcfg.Spec.BGPConfig.Neighbors[1].RemoteAS = 2000
	errs = nh.validateRoutingConfig(rtcfg, "v1", false, false)
	Assert(t, len(errs) == 0, "found errors %s", errs)
	rtcfg.Spec.BGPConfig.Neighbors = append(rtcfg.Spec.BGPConfig.Neighbors, &network.BGPNeighbor{
		IPAddress:             "0.0.0.0",
		EnableAddressFamilies: []string{"ipv4-unicast"},
		RemoteAS:              1000,
	})
	errs = nh.validateRoutingConfig(rtcfg, "v1", false, false)
	Assert(t, len(errs) > 0, "Expecting errors %s", errs)
	rtcfg.Spec.BGPConfig.Neighbors[2].EnableAddressFamilies = []string{"evpn"}
	errs = nh.validateRoutingConfig(rtcfg, "v1", false, false)
	Assert(t, len(errs) > 0, "Expecting errors %s", errs)
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
				RouterId: "1.1.1.1",
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
				RouterId: "1.1.1.1",
				ASNumber: 1000,
			},
		},
	}
	_, _, err = nh.routingConfigPreCommit(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, rtCfg)
	AssertOk(t, err, "expecting to succeed (%v)", err)
	rtCfg.Spec.BGPConfig.ASNumber = 2000
	_, _, err = nh.routingConfigPreCommit(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, rtCfg)
	Assert(t, err != nil, "expecting to fail (%v)", err)

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
			Type: network.NetworkType_Bridged.String(),
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
