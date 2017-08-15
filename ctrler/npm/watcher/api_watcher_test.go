// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package watcher

import (
	"context"
	"testing"
	"time"

	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	"github.com/pensando/sw/api/generated/network"
	_ "github.com/pensando/sw/api/hooks"
	"github.com/pensando/sw/apiserver"
	apisrvpkg "github.com/pensando/sw/apiserver/pkg"
	"github.com/pensando/sw/ctrler/npm/statemgr"
	"github.com/pensando/sw/orch"
	vchserver "github.com/pensando/sw/orch/vchub/server"
	vchstore "github.com/pensando/sw/orch/vchub/store"
	"github.com/pensando/sw/utils/kvstore/store"
	kvs "github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/runtime"
	. "github.com/pensando/sw/utils/testutils"
)

const (
	apisrvURL = "localhost:9182"
	vmmURL    = "localhost:9192"
)

func createAPIServer(url string) (apiserver.Server, apiserver.Config) {
	logger := log.GetNewLogger(log.GetDefaultConfig("api_watcher_test"))

	// api server config
	sch := runtime.NewScheme()
	apisrvConfig := apiserver.Config{
		GrpcServerPort: url,
		Logger:         logger,
		Version:        "v1",
		Scheme:         sch,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeMemkv,
			Servers: []string{""},
			Codec:   runtime.NewJSONCodec(sch),
		},
	}
	// create api server
	apiSrv := apisrvpkg.MustGetAPIServer()
	go apiSrv.Run(apisrvConfig)
	time.Sleep(time.Millisecond * 100)

	return apiSrv, apisrvConfig
}

func TestApiWatcher(t *testing.T) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(&dummyWriter{})
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	// create api server
	apiSrv, _ := createAPIServer(apisrvURL)
	Assert(t, (apiSrv != nil), "Error creating api server", apiSrv)

	// create memkvstore
	kvs, err := vchstore.Init("", kvs.KVStoreTypeMemkv)
	AssertOk(t, err, "Error initing kvstore")
	Assert(t, (kvs != nil), "Error creating kvstore")

	// create vchub
	vcs, err := vchserver.NewVCHServer(vmmURL)
	AssertOk(t, err, "Error creating vchub")
	time.Sleep(time.Millisecond * 10)

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, apisrvURL, vmmURL)
	AssertOk(t, err, "Error creating watchr")
	Assert(t, (watcher != nil), "Error creating watcher", watcher)
	time.Sleep(time.Millisecond * 10)

	// create an api server client
	l := log.GetNewLogger(log.GetDefaultConfig("NpmApiWatcher"))
	apicl, err := apiclient.NewGrpcAPIClient(apisrvURL, l, grpc.WithInsecure(), grpc.WithTimeout(time.Second))
	AssertOk(t, err, "Error creating api server client")

	// create a network in api server
	net := network.Network{
		TypeMeta: api.TypeMeta{Kind: "network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "test",
		},
		Spec: network.NetworkSpec{
			IPv4Subnet:  "10.1.1.1/24",
			IPv4Gateway: "10.1.1.254",
		},
	}
	ns, err := apicl.NetworkV1().Network().Create(context.Background(), &net)
	AssertOk(t, err, "Error creating network")
	Assert(t, (ns.Spec.IPv4Subnet == "10.1.1.1/24"), "Got invalid network", ns)

	// verify network got created
	AssertEventually(t, func() bool {
		_, nerr := stateMgr.FindNetwork("default", "test")
		return (nerr == nil)
	}, "Network not found in statemgr")
	nw, err := stateMgr.FindNetwork("default", "test")
	AssertOk(t, err, "Could not find the network")
	Assert(t, (nw.Spec.IPv4Subnet == "10.1.1.1/24"), "Got invalid network", nw)

	// create a vmm nwif
	nwif := orch.NwIF{
		ObjectKind: "NwIF",
		ObjectMeta: &api.ObjectMeta{
			Tenant: "default",
			Name:   "test-nwif",
		},
		Config: &orch.NwIF_Config{
			LocalVLAN: 22,
		},
		Status: &orch.NwIF_Status{
			MacAddress:  "11:11:11:11:11:11",
			PortGroup:   "test",
			SmartNIC_ID: "test-host",
		},
	}
	err = vchstore.NwIFCreate(context.Background(), "test-nwif", &nwif)
	AssertOk(t, err, "Error creating nw if")

	// verify endpoint got created
	AssertEventually(t, func() bool {
		_, perr := stateMgr.FindEndpoint("default", "test-nwif")
		return (perr == nil)
	}, "Endpoint not found in statemgr")
	ep, err := stateMgr.FindEndpoint("default", "test-nwif")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (ep.Status.MicroSegmentVlan == 22), "Endpoint did not match", ep)

	// delete the nwif
	err = vchstore.NwIFDelete(context.Background(), "test-nwif")
	AssertOk(t, err, "Error deleting nw if")

	// verify endpoint is gone
	AssertEventually(t, func() bool {
		_, perr := stateMgr.FindEndpoint("default", "test-nwif")
		return (perr != nil)
	}, "Endpoint still found in statemgr")

	// delete the network
	_, err = apicl.NetworkV1().Network().Delete(context.Background(), &net.ObjectMeta)
	AssertOk(t, err, "Error deleting network")

	// verify network is gone
	AssertEventually(t, func() bool {
		_, nerr := stateMgr.FindNetwork("default", "test")
		return (nerr != nil)
	}, "Endpoint still found in statemgr")

	// stop the api server
	apiSrv.Stop()
	vcs.StopServer()
	watcher.Stop()
	time.Sleep(time.Millisecond * 10)
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

	// create watcher on api server
	watcher, err := NewWatcher(stateMgr, "", vmmURL)
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
	time.Sleep(time.Millisecond * 10)
}
