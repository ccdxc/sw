// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package watcher

import (
	"context"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	_ "github.com/pensando/sw/api/hooks/apiserver"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/ctrler/npm/statemgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/orch"
	vchserver "github.com/pensando/sw/venice/orch/vchub/server"
	vchstore "github.com/pensando/sw/venice/orch/vchub/store"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/debug"
	"github.com/pensando/sw/venice/utils/events/recorder"
	kvs "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

var (
	logger = log.GetNewLogger(log.GetDefaultConfig("api_watcher_test"))

	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "api_watcher_test"},
		EvtTypes:      evtsapi.GetEventTypes(),
		BackupDir:     "/tmp",
		SkipEvtsProxy: true})
)

func TestApiWatcher(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create api server
	apiSrv, apisrvURL, err := serviceutils.StartAPIServer("", t.Name(), logger)
	AssertOk(t, err, "Error starting api server")

	// create memkvstore
	kvs, err := vchstore.Init("vchub", kvs.KVStoreTypeMemkv)
	AssertOk(t, err, "Error initing kvstore")
	Assert(t, (kvs != nil), "Error creating kvstore")

	// generate an available port for vchub to use.
	vchubSrvListener := netutils.TestListenAddr{}
	err = vchubSrvListener.GetAvailablePort()
	if err != nil {
		t.Errorf("could not find an available port for the vchub")
	}
	vmmURL := vchubSrvListener.ListenURL.String()

	// create vchub
	vcs, err := vchserver.NewVCHServer(vmmURL)
	AssertOk(t, err, "Error creating vchub")
	time.Sleep(time.Millisecond * 10)

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, apisrvURL, vmmURL, nil, debug.New(t.Name()).Build())
	AssertOk(t, err, "Error creating watchr")
	Assert(t, (watcher != nil), "Error creating watcher", watcher)
	time.Sleep(time.Millisecond * 10)

	// create an api server client
	l := log.GetNewLogger(log.GetDefaultConfig("NpmApiWatcher"))
	apicl, err := apiclient.NewGrpcAPIClient(globals.Npm, apisrvURL, l)
	AssertOk(t, err, "Error creating api server client")

	// create a default tenant
	tenant := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "testTenant",
		},
	}

	tn, err := apicl.ClusterV1().Tenant().Create(context.Background(), &tenant)
	AssertOk(t, err, "failed to create tenant")
	AssertEquals(t, "testTenant", tn.Name, "tenant names did not match")

	// verify the tenant got created
	AssertEventually(t, func() (bool, interface{}) {
		_, terr := stateMgr.FindTenant("testTenant")
		return terr == nil, nil
	}, "Tenant not found in statemgr")
	ts, err := stateMgr.FindTenant("testTenant")
	AssertOk(t, err, "Could not find the tenant")
	AssertEquals(t, "testTenant", ts.Name, "tenant names did not match")

	//create a network in api server
	net := network.Network{
		TypeMeta: api.TypeMeta{Kind: "network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testNetwork",
		},
		Spec: network.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}
	ns, err := apicl.NetworkV1().Network().Create(context.Background(), &net)
	AssertOk(t, err, "Error creating network")
	Assert(t, (ns.Spec.IPv4Subnet == "10.1.1.0/24"), "Got invalid network", ns)

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindNetwork("testTenant", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found in statemgr")
	nw, err := stateMgr.FindNetwork("testTenant", "testNetwork")
	AssertOk(t, err, "Could not find the network")
	Assert(t, (nw.Spec.IPv4Subnet == "10.1.1.0/24"), "Got invalid network", nw)

	// create a vmm nwif
	nwif := orch.NwIF{
		ObjectKind: "NwIF",
		ObjectMeta: &api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "test-nwif",
			UUID:   "test-nwif",
		},
		Config: &orch.NwIF_Config{
			LocalVLAN: 22,
		},
		Status: &orch.NwIF_Status{
			IpAddress:   "10.1.1.1",
			MacAddress:  "11:11:11:11:11:11",
			PortGroup:   "test",
			Network:     "testNetwork",
			SmartNIC_ID: "test-host",
		},
	}
	err = vchstore.NwIFCreate(context.Background(), "test-nwif", &nwif)
	AssertOk(t, err, "Error creating nw if")

	// verify endpoint got created
	AssertEventually(t, func() (bool, interface{}) {
		_, perr := stateMgr.FindEndpoint("testTenant", "test-nwif")
		return (perr == nil), nil
	}, "Endpoint not found in statemgr")
	ep, err := stateMgr.FindEndpoint("testTenant", "test-nwif")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (ep.Status.MicroSegmentVlan == 22), "Endpoint did not match", ep)

	// create sg policy
	rules := []security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}

	// sg policy
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}
	sgps, err := apicl.SecurityV1().SGPolicy().Create(context.Background(), &sgp)
	AssertOk(t, err, "Error creating security policy")
	AssertEquals(t, rules, sgps.Spec.Rules, "rules did not match")

	// security group object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testSg",
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}),
		},
	}
	_, err = apicl.SecurityV1().SecurityGroup().Create(context.Background(), &sg)
	AssertOk(t, err, "Error creating security group")

	// delete the nwif
	err = vchstore.NwIFDelete(context.Background(), "test-nwif")
	AssertOk(t, err, "Error deleting nw if")

	// verify endpoint is gone
	AssertEventually(t, func() (bool, interface{}) {
		_, perr := stateMgr.FindEndpoint("testTenant", "test-nwif")
		return (perr != nil), nil
	}, "Endpoint still found in statemgr")

	// delete the tenant
	_, err = apicl.ClusterV1().Tenant().Delete(context.Background(), &tenant.ObjectMeta)
	AssertOk(t, err, "Error deleting tenant")

	// delete the network
	_, err = apicl.NetworkV1().Network().Delete(context.Background(), &net.ObjectMeta)
	AssertOk(t, err, "Error deleting network")

	// delete the sg policy
	_, err = apicl.SecurityV1().SGPolicy().Delete(context.Background(), &sgp.ObjectMeta)
	AssertOk(t, err, "Error deleting sg policy")

	// verify network is gone
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindNetwork("testTenant", "test")
		return (nerr != nil), nil
	}, "Endpoint still found in statemgr")

	// stop the api server
	apicl.Close()
	watcher.Stop()
	apiSrv.Stop()
	vcs.StopServer()
	kvs.Close()
}

func TestAPIServerRestarts(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create api server
	apiSrv, apisrvURL, err := serviceutils.StartAPIServer("", t.Name(), logger)
	AssertOk(t, err, "Error starting api server")

	// create memkvstore
	kvs, err := vchstore.Init("vchub", kvs.KVStoreTypeMemkv)
	AssertOk(t, err, "Error initing kvstore")
	Assert(t, (kvs != nil), "Error creating kvstore")

	// generate an available port for vchub to use.
	vchubSrvListener := netutils.TestListenAddr{}
	err = vchubSrvListener.GetAvailablePort()
	if err != nil {
		t.Errorf("could not find an available port for the vchub")
	}
	vmmURL := vchubSrvListener.ListenURL.String()

	// create vchub
	vcs, err := vchserver.NewVCHServer(vmmURL)
	AssertOk(t, err, "Error creating vchub")
	time.Sleep(time.Millisecond * 10)

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, apisrvURL, vmmURL, nil, debug.New(t.Name()).Build())
	AssertOk(t, err, "Error creating watchr")
	Assert(t, (watcher != nil), "Error creating watcher", watcher)
	time.Sleep(time.Millisecond * 10)

	// create an api server client
	l := log.GetNewLogger(log.GetDefaultConfig("NpmApiWatcher"))
	apicl, err := apiclient.NewGrpcAPIClient(globals.Npm, apisrvURL, l)
	AssertOk(t, err, "Error creating api server client")

	// create a default tenant
	tenant := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "testTenant",
		},
	}

	tn, err := apicl.ClusterV1().Tenant().Create(context.Background(), &tenant)
	AssertOk(t, err, "failed to create tenant")
	AssertEquals(t, "testTenant", tn.Name, "tenant names did not match")

	// verify the tenant got created
	AssertEventually(t, func() (bool, interface{}) {
		_, terr := stateMgr.FindTenant("testTenant")
		return terr == nil, nil
	}, "Tenant not found in statemgr")
	ts, err := stateMgr.FindTenant("testTenant")
	AssertOk(t, err, "Could not find the tenant")
	AssertEquals(t, "testTenant", ts.Name, "tenant names did not match")

	//create a network in api server
	net := network.Network{
		TypeMeta: api.TypeMeta{Kind: "network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testNetwork",
		},
		Spec: network.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}
	ns, err := apicl.NetworkV1().Network().Create(context.Background(), &net)
	AssertOk(t, err, "Error creating network")
	Assert(t, (ns.Spec.IPv4Subnet == "10.1.1.0/24"), "Got invalid network", ns)

	// verify network got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindNetwork("testTenant", "testNetwork")
		return (nerr == nil), nil
	}, "Network not found in statemgr")
	nw, err := stateMgr.FindNetwork("testTenant", "testNetwork")
	AssertOk(t, err, "Could not find the network")
	Assert(t, (nw.Spec.IPv4Subnet == "10.1.1.0/24"), "Got invalid network", nw)
	//
	// create a vmm nwif
	nwif := orch.NwIF{
		ObjectKind: "NwIF",
		ObjectMeta: &api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "test-nwif",
			UUID:   "test-nwif",
		},
		Config: &orch.NwIF_Config{
			LocalVLAN: 22,
		},
		Status: &orch.NwIF_Status{
			IpAddress:   "10.1.1.1",
			MacAddress:  "11:11:11:11:11:11",
			PortGroup:   "test",
			Network:     "testNetwork",
			SmartNIC_ID: "test-host",
		},
	}
	err = vchstore.NwIFCreate(context.Background(), "test-nwif", &nwif)
	AssertOk(t, err, "Error creating nw if")

	// verify endpoint got created
	AssertEventually(t, func() (bool, interface{}) {
		_, perr := stateMgr.FindEndpoint("testTenant", "test-nwif")
		return (perr == nil), nil
	}, "Endpoint not found in statemgr")
	ep, err := stateMgr.FindEndpoint("testTenant", "test-nwif")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (ep.Status.MicroSegmentVlan == 22), "Endpoint did not match", ep)

	// create sg policy
	rules := []security.SGRule{
		{
			Apps:            []string{"tcp/80"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"10.0.0.1", "10.0.0.2"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}
	sgps, err := apicl.SecurityV1().SGPolicy().Create(context.Background(), &sgp)
	AssertOk(t, err, "Error creating security policy")
	AssertEquals(t, rules, sgps.Spec.Rules, "rules did not match")

	// security group object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testSg",
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}),
		},
	}
	_, err = apicl.SecurityV1().SecurityGroup().Create(context.Background(), &sg)
	AssertOk(t, err, "Error creating security group")

	// smartNic params
	snic := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testSmartNIC",
			Namespace: "",
		},
		Spec: cluster.SmartNICSpec{
			MgmtMode: cluster.SmartNICSpec_NETWORK.String(),
		},
		Status: cluster.SmartNICStatus{
			AdmissionPhase: "ADMITTED",
			PrimaryMAC:     "00:01:02:03:04:05",
		},
	}

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: "testHost",
		},
		Spec: cluster.HostSpec{
			SmartNICs: []cluster.SmartNICID{
				{
					MACAddress: "00:01:02:03:04:05",
				},
			},
		},
		Status: cluster.HostStatus{
			Type: "HYPERVISOR",
		},
	}

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "",
			Tenant:    "testTenant",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "00:01:02:03:04:05",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	// create snic object
	_, err = apicl.ClusterV1().SmartNIC().Create(context.Background(), &snic)
	AssertOk(t, err, "Error creating snic")

	// verify snic got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindSmartNIC("", "testSmartNIC")
		return (nerr == nil), nil
	}, "SmartNIC not found in statemgr")

	// create host object
	_, err = apicl.ClusterV1().Host().Create(context.Background(), &host)
	AssertOk(t, err, "Error creating host")

	// verify host got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindHost("", "testHost")
		return (nerr == nil), nil
	}, "Host not found in statemgr")

	// create workload
	_, err = apicl.WorkloadV1().Workload().Create(context.Background(), &wr)
	AssertOk(t, err, "Error creating workload")

	// verify endpoint got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindEndpoint("testTenant", "testWorkload-00:01:02:03:04:05")
		return (nerr == nil), nil
	}, "Endpoint not found in statemgr")

	// firewall profile
	fwp := security.FirewallProfile{
		TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testProfile",
			Namespace: "",
			Tenant:    "testTenant",
		},
		Spec: security.FirewallProfileSpec{
			SessionIdleTimeout:    "3m",
			IPNormalizationEnable: true,
		},
	}

	// app
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testApp",
			Namespace: "",
			Tenant:    "testTenant",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "21",
				},
			},
			Timeout: "5m",
			ALG: security.ALG{
				Type: "FTP",
				FtpAlg: security.FtpAlg{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	_, err = apicl.SecurityV1().FirewallProfile().Create(context.Background(), &fwp)
	AssertOk(t, err, "Error creating firewall profile")

	_, err = apicl.SecurityV1().App().Create(context.Background(), &app)
	AssertOk(t, err, "Error creating app alg")

	time.Sleep(time.Millisecond * 100)

	// stop api server and watchers
	apiSrv.Stop()
	watcher.Stop()
	apicl.Close()

	// restart api server and watchers
	apiSrv, apisrvURL, err = serviceutils.StartAPIServer(apisrvURL, t.Name(), logger)
	AssertOk(t, err, "Error restarting api server")
	apicl, err = apiclient.NewGrpcAPIClient(globals.Npm, apisrvURL, l)
	AssertOk(t, err, "Error restarting api server client")
	watcher, err = NewWatcher(stateMgr, apisrvURL, vmmURL, nil, debug.New(t.Name()).Build())
	AssertOk(t, err, "Error restarting watcher")

	// wait for the objects to come back
	time.Sleep(time.Millisecond * 100)
	AssertEventually(t, func() (bool, interface{}) {
		_, terr := stateMgr.FindTenant("testTenant")
		if terr != nil {
			return false, nil
		}
		_, nerr := stateMgr.FindNetwork("testTenant", "testNetwork")
		if nerr != nil {
			return false, nil
		}
		_, perr := stateMgr.FindEndpoint("testTenant", "test-nwif")
		if perr != nil {
			return false, nil
		}
		return true, nil
	}, "Tenant not found in statemgr")

	// delete all the objects
	_, err = apicl.ClusterV1().Tenant().Delete(context.Background(), &tenant.ObjectMeta)
	AssertOk(t, err, "could not delete tenant")

	_, err = apicl.NetworkV1().Network().Delete(context.Background(), &net.ObjectMeta)
	AssertOk(t, err, "could not delete network")

	err = vchstore.NwIFDelete(context.Background(), "test-nwif")
	AssertOk(t, err, "could not delete ep")

	_, err = apicl.SecurityV1().SGPolicy().Delete(context.Background(), &sgp.ObjectMeta)
	AssertOk(t, err, "could not delete sg policy")

	// delete the workload
	_, err = apicl.WorkloadV1().Workload().Delete(context.Background(), &wr.ObjectMeta)
	AssertOk(t, err, "Error deleting workload")

	// delete host object
	_, err = apicl.ClusterV1().Host().Delete(context.Background(), &host.ObjectMeta)
	AssertOk(t, err, "Error deleting host")

	// delete snic object
	_, err = apicl.ClusterV1().SmartNIC().Delete(context.Background(), &snic.ObjectMeta)
	AssertOk(t, err, "Error deleting snic")

	// stop the api server
	apicl.Close()
	watcher.Stop()
	apiSrv.Stop()
	vcs.StopServer()
	kvs.Close()
}

func TestApiServerClient(t *testing.T) {
	testCount := 5
	for i := 0; i < testCount; i++ {
		// create api server
		apiSrv, apisrvURL, err := serviceutils.StartAPIServer("", t.Name(), logger)
		AssertOk(t, err, "Error starting api server")

		// create an api server client
		l := log.GetNewLogger(log.GetDefaultConfig("NpmApiWatcher"))
		apicl, err := apiclient.NewGrpcAPIClient(globals.Npm, apisrvURL, l)
		AssertOk(t, err, "Error creating api server client")

		// network watcher
		opts := api.ListWatchOptions{}
		watchCtx, watchCancel := context.WithCancel(context.Background())
		netWatcher, err := apicl.NetworkV1().Network().Watch(watchCtx, &opts)
		AssertOk(t, err, "Error creating api server watcher")
		Assert(t, netWatcher != nil, "Netwatcher is nil")

		// stop api client & server
		watchCancel()
		apicl.Close()
		apiSrv.Stop()
		netWatcher.Stop()
	}
}

func TestApiWatcherConnectDisconnect(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create memkvstore
	kvs, err := vchstore.Init("", kvs.KVStoreTypeMemkv)
	AssertOk(t, err, "Error initing kvstore")
	Assert(t, (kvs != nil), "Error creating kvstore")

	// generate an available port for vchub to use.
	vchubSrvListener := netutils.TestListenAddr{}
	err = vchubSrvListener.GetAvailablePort()
	if err != nil {
		t.Errorf("could not find an available port for the vchub")
	}
	vmmURL := vchubSrvListener.ListenURL.String()

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, "", vmmURL, nil, debug.New(t.Name()).Build())
	AssertOk(t, err, "Error creating watchr")
	Assert(t, (watcher != nil), "Error creating watcher", watcher)
	time.Sleep(time.Millisecond * 10)

	// create vchub
	vcs, err := vchserver.NewVCHServer(vmmURL)
	AssertOk(t, err, "Error creating vchub")

	// stop the server and restart it
	vcs.StopServer()
	time.Sleep(time.Second * 3)
	vcs, err = vchserver.NewVCHServer(vmmURL)
	AssertOk(t, err, "Error creating vchub")
	time.Sleep(time.Millisecond * 10)

	// stop the vchub
	vcs.StopServer()
	watcher.Stop()
	kvs.Close()
}

func TestWorkloadWatcher(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create api server
	apiSrv, apisrvURL, err := serviceutils.StartAPIServer("", t.Name(), logger)
	AssertOk(t, err, "Error starting api server")
	defer apiSrv.Stop()

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, apisrvURL, "", nil, debug.New(t.Name()).Build())
	AssertOk(t, err, "Error creating watchr")
	Assert(t, (watcher != nil), "Error creating watcher", watcher)
	time.Sleep(time.Millisecond * 10)

	// create an api server client
	l := log.GetNewLogger(log.GetDefaultConfig("NpmApiWatcher"))
	apicl, err := apiclient.NewGrpcAPIClient(globals.Npm, apisrvURL, l)
	AssertOk(t, err, "Error creating api server client")

	// smartNic params
	snic := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testSmartNIC",
			Namespace: "",
		},
		Spec: cluster.SmartNICSpec{
			Hostname: "testSmartNIC",
			MgmtMode: cluster.SmartNICSpec_NETWORK.String(),
		},
		Status: cluster.SmartNICStatus{
			AdmissionPhase: cluster.SmartNICStatus_ADMITTED.String(),
			PrimaryMAC:     "00:01:02:03:04:05",
		},
	}

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: "testHost",
		},
		Spec: cluster.HostSpec{
			SmartNICs: []cluster.SmartNICID{
				{
					MACAddress: "00:01:02:03:04:05",
				},
			},
		},
		Status: cluster.HostStatus{
			Type: "HYPERVISOR",
		},
	}

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "",
			Tenant:    "testTenant",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "00:01:02:03:04:05",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	// create snic object
	_, err = apicl.ClusterV1().SmartNIC().Create(context.Background(), &snic)
	AssertOk(t, err, "Error creating snic")

	// verify snic got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindSmartNIC("", "testSmartNIC")
		return (nerr == nil), nil
	}, "SmartNIC not found in statemgr")

	// create host object
	_, err = apicl.ClusterV1().Host().Create(context.Background(), &host)
	AssertOk(t, err, "Error creating host")

	// verify host got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindHost("", "testHost")
		return (nerr == nil), nil
	}, "Host not found in statemgr")

	// create workload
	_, err = apicl.WorkloadV1().Workload().Create(context.Background(), &wr)
	AssertOk(t, err, "Error creating workload")

	// verify endpoint got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindEndpoint("testTenant", "testWorkload-00:01:02:03:04:05")
		return (nerr == nil), nil
	}, "Endpoint not found in statemgr")
	ep, err := stateMgr.FindEndpoint("testTenant", "testWorkload-00:01:02:03:04:05")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (ep.Status.HomingHostName == "testHost"), "Got invalid endpoint", ep)

	// delete the workload
	_, err = apicl.WorkloadV1().Workload().Delete(context.Background(), &wr.ObjectMeta)
	AssertOk(t, err, "Error deleting workload")

	// verify endpoint got deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindEndpoint("testTenant", "testWorkload-00:01:02:03:04:05")
		return (nerr != nil), nil
	}, "Endpoint still found in statemgr")

	// delete host object
	_, err = apicl.ClusterV1().Host().Delete(context.Background(), &host.ObjectMeta)
	AssertOk(t, err, "Error deleting host")

	// verify host got deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindHost("", "testHost")
		return (nerr != nil), nil
	}, "Host still found in statemgr")

	// delete snic object
	_, err = apicl.ClusterV1().SmartNIC().Delete(context.Background(), &snic.ObjectMeta)
	AssertOk(t, err, "Error deleting snic")

	// verify snic got deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindSmartNIC("", "testSmartNIC")
		return (nerr != nil), nil
	}, "SmartNIC still found in statemgr")
}

func TestFirewallWatcher(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create api server
	apiSrv, apisrvURL, err := serviceutils.StartAPIServer("", t.Name(), logger)
	AssertOk(t, err, "Error starting api server")
	defer apiSrv.Stop()

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, apisrvURL, "", nil, debug.New(t.Name()).Build())
	AssertOk(t, err, "Error creating watchr")
	Assert(t, (watcher != nil), "Error creating watcher", watcher)
	time.Sleep(time.Millisecond * 10)

	// create an api server client
	l := log.GetNewLogger(log.GetDefaultConfig("NpmApiWatcher"))
	apicl, err := apiclient.NewGrpcAPIClient(globals.Npm, apisrvURL, l)
	AssertOk(t, err, "Error creating api server client")

	// firewall profile
	fwp := security.FirewallProfile{
		TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testProfile",
			Namespace: "",
			Tenant:    "testTenant",
		},
		Spec: security.FirewallProfileSpec{
			SessionIdleTimeout:    "3m",
			IPNormalizationEnable: true,
		},
	}

	// app
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testApp",
			Namespace: "",
			Tenant:    "testTenant",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "21",
				},
			},
			Timeout: "5m",
			ALG: security.ALG{
				Type: "FTP",
				FtpAlg: security.FtpAlg{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	_, err = apicl.SecurityV1().FirewallProfile().Create(context.Background(), &fwp)
	AssertOk(t, err, "Error creating firewall profile")

	// verify firewall profile got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindFirewallProfile("testTenant", "testProfile")
		return (nerr == nil), nil
	}, "Firewall profile not found in statemgr")

	_, err = apicl.SecurityV1().App().Create(context.Background(), &app)
	AssertOk(t, err, "Error creating app alg")

	// verify app got created
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindApp("testTenant", "testApp")
		return (nerr == nil), nil
	}, "App not found in statemgr")

	// delete firewall profile object
	_, err = apicl.SecurityV1().FirewallProfile().Delete(context.Background(), &fwp.ObjectMeta)
	AssertOk(t, err, "Error deleting firewall profile")

	// verify firewall profile got deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindFirewallProfile("testTenant", "testProfile")
		return (nerr != nil), nil
	}, "Firewall profile still found in statemgr")

	// delete app object
	_, err = apicl.SecurityV1().App().Delete(context.Background(), &app.ObjectMeta)
	AssertOk(t, err, "Error deleting app")

	// verify app got deleted
	AssertEventually(t, func() (bool, interface{}) {
		_, nerr := stateMgr.FindApp("testTenant", "testApp")
		return (nerr != nil), nil
	}, "App still found in statemgr")
}
