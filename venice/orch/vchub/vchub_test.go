package main

import (
	"os"
	"testing"
	"time"

	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/grpclog"

	api_cache "github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/orch"
	"github.com/pensando/sw/venice/orch/vchub/sim"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"

	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	apiServerAddress = ":9009"
)

var (
	logger = log.WithContext("module", "VCHubTest")

	// Create mock events recorder
	mr = mockevtsrecorder.NewRecorder("vchub_test", logger)
	_  = recorder.Override(mr)
)

type testInfo struct {
	apiServerAddr string
	apiServer     apiserver.Server
	rpcClient     *rpckit.RPCClient
}

var tInfo testInfo

func testSetup() {
	grpclog.SetLoggerV2(logger)

	scheme := runtime.GetDefaultScheme()
	srvConfig := apiserver.Config{
		GrpcServerPort: apiServerAddress,
		DebugMode:      false,
		Logger:         logger,
		Version:        "v1",
		Scheme:         scheme,
		Kvstore: store.Config{
			Type:  store.KVStoreTypeMemkv,
			Codec: runtime.NewJSONCodec(scheme),
		},
		GetOverlay:       api_cache.GetOverlay,
		IsDryRun:         api_cache.IsDryRun,
		AllowMultiTenant: true,
	}

	tInfo.apiServer = apiserverpkg.MustGetAPIServer()
	go tInfo.apiServer.Run(srvConfig)
	tInfo.apiServer.WaitRunning()
}

func testTeardown() {
	mr.ClearEvents()

	// Stop the API server
	tInfo.apiServer.Stop()
}

func TestVCHub(t *testing.T) {
	t.Skip()
	testSetup()
	defer testTeardown()

	var opts cliOpts
	err := parseOpts(&opts)
	if err == nil {
		t.Errorf("parseOpts returned success while expecting error")
	}
	os.Args = []string{"vchub", "-store-url", "blah:", "-vcenter-list", "user:pass@127.0.0.1:8990/sdk"}
	err = parseOpts(&opts)
	if err == nil {
		t.Errorf("parseOpts returned success while expecting error")
	}

	os.Args = []string{"vchub", "-store-url", "blah:", "-vcenter-list", "http://foo", "-foo", "bar"}
	err = parseOpts(&opts)
	if err == nil {
		t.Errorf("parseOpts returned success while expecting error")
	}
	sim.Setup()
	defer sim.TearDown()
	vc1, err := sim.Simulate("127.0.0.1:8990", 2, 2)
	if err != nil {
		t.Errorf("Error %v simulating vCenter", err)
		return
	}

	time.Sleep(50 * time.Millisecond)

	//os.Args = []string{"vchub", "-listen-url", ":9898", "-store-url", "memkv:", "-vcenter-list", vc1 + ",user:pass@127.0.0.1:8880/sdk", "-log-to-file", "/tmp/vchub.log"}
	os.Args = []string{"vchub", "-listen-url", ":9898", "-store-url", "memkv:", "-vcenter-list", vc1 + ",http://user:pass@127.0.0.1:8990/sdk", "-log-to-file", "/tmp/vchub.log"}
	err = parseOpts(&opts)
	if err != nil {
		t.Errorf("parseOpts returned %v", err)
		return
	}

	go launchVCHub(&opts, logger)
	time.Sleep(1100 * time.Millisecond)

	// verify nwifs are created
	// setup client
	var grpcOpts []grpc.DialOption
	grpcOpts = append(grpcOpts, grpc.WithInsecure())
	conn, err := grpc.Dial("127.0.0.1:9898", grpcOpts...)
	if err != nil {
		t.Errorf("fail to dial: %v", err)
		return
	}
	vcHubClient := orch.NewOrchApiClient(conn)
	filter := &orch.Filter{}

	AssertEventually(t, func() (bool, interface{}) {
		nicList, err := vcHubClient.ListSmartNICs(context.Background(), filter)
		if err != nil {
			return false, nil
		}
		nics := nicList.GetItems()
		if len(nics) == 0 {
			return false, nil
		}

		return true, nil
	}, "Default SmartNICs", "100ms", "20s")

	os.Args = []string{"vchub", "-vcenter-list", vc1 + ",user:pass@127.0.0.1:8880/sdk", "-log-to-file", "/tmp/vchub.log"}
	err = parseOpts(&opts)
	go main()
	time.Sleep(100 * time.Millisecond)
}
