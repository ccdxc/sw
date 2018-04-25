// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package services

import (
	"context"
	"flag"
	"fmt"
	"net/http"
	"os"
	gorun "runtime"
	"strings"
	"testing"
	"time"

	trace "golang.org/x/net/trace"
	rpc "google.golang.org/grpc"
	"google.golang.org/grpc/grpclog"

	api "github.com/pensando/sw/api"
	apicache "github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	cmd "github.com/pensando/sw/api/generated/cluster"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	nmd "github.com/pensando/sw/nic/agent/nmd"
	"github.com/pensando/sw/nic/agent/nmd/platform"
	proto "github.com/pensando/sw/nic/agent/nmd/protos"
	apiserver "github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	cmdapi "github.com/pensando/sw/venice/cmd/apiclient"
	"github.com/pensando/sw/venice/cmd/cache"
	cmdenv "github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/globals"
	store "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
	ventrace "github.com/pensando/sw/venice/utils/trace"
	"github.com/pensando/sw/venice/utils/tsdb"
)

const (
	smartNICServerURL = "localhost:" + globals.CMDSmartNICRegistrationAPIPort
	resolverURLs      = ":" + globals.CMDResolverPort
	minAgents         = 1
	maxAgents         = 100
	healthInterval    = 1 * time.Second
	deadtimeInterval  = 3 * time.Second
)

var (
	numNaples   = flag.Int("num-naples", 1, fmt.Sprintf("Number of Naples instances [%d..%d]", minAgents, maxAgents))
	cmdURL      = flag.String("cmd-url", smartNICServerURL, "CMD URL")
	resolverURL = flag.String("resolver-url", resolverURLs, "Resolver URLs")
	rpcTrace    = flag.Bool("rpc-trace", false, "Enable gRPC tracing")
)

type testInfo struct {
	l              log.Logger
	apiServerAddr  string
	apiServer      apiserver.Server
	apiClient      apiclient.Services
	rpcServer      *rpckit.RPCServer
	smartNICServer *smartnic.RPCServer
}

var tInfo testInfo

func (t testInfo) APIClient() cmd.ClusterV1Interface {
	return t.apiClient.ClusterV1()
}

func getNodeID(index int) string {
	return fmt.Sprintf("44.44.44.44.%02x.%02x", index/256, index%256)
}

func getRESTPort(index int) string {
	return fmt.Sprintf("%d", 16000+index)
}

func getRESTUrl(index int) string {
	return fmt.Sprintf("localhost:%s", getRESTPort(index))
}

func getDBPath(index int) string {
	return fmt.Sprintf("/tmp/nmd-%d.db", index)
}

// launchCMDServer creates a smartNIC CMD server for SmartNIC service.
func launchCMDServer(m *testing.M, url, certFile, keyFile, caFile string) (*rpckit.RPCServer, error) {

	// create an RPC server.
	rpcServer, err := rpckit.NewRPCServer("smartNIC", url)
	if err != nil {
		fmt.Printf("Error creating RPC-server: %v", err)
		return nil, err
	}
	tInfo.rpcServer = rpcServer
	cmdenv.UnauthRPCServer = rpcServer

	// create and register the RPC handler for SmartNIC service
	tInfo.smartNICServer, err = smartnic.NewRPCServer(tInfo,
		healthInterval,
		deadtimeInterval,
		getRESTPort(1),
		cmdenv.StateMgr)
	if err != nil {
		fmt.Printf("Error creating SmartNIC RPC server: %v", err)
		return nil, err
	}

	grpc.RegisterSmartNICRegistrationServer(rpcServer.GrpcServer, tInfo.smartNICServer)
	rpcServer.Start()
	cmdenv.NICService = tInfo.smartNICServer
	return rpcServer, nil
}

// createCMD creates rpc server for SmartNIC service
func createCMD(m *testing.M) *rpckit.RPCServer {

	// set cmd logger & quorum nodes
	cmdenv.Logger = tInfo.l
	cmdenv.QuorumNodes = []string{"localhost"}
	cmdenv.StateMgr = cache.NewStatemgr()

	// Start CMD config watcher
	l := mock.NewLeaderService("testMaster")
	s := NewSystemdService(WithSysIfSystemdSvcOption(&mock.SystemdIf{}))
	cw := cmdapi.NewCfgWatcherService(tInfo.l, tInfo.apiServerAddr, cmdenv.StateMgr)
	cmdenv.MasterService = NewMasterService(
		WithLeaderSvcMasterOption(l),
		WithSystemdSvcMasterOption(s),
		WithConfigsMasterOption(&mock.Configs{}),
		WithCfgWatcherMasterOption(cw))
	cw.Start()

	// start the rpc server
	rpcServer, err := launchCMDServer(m, *cmdURL, "", "", "")
	if err != nil {
		fmt.Printf("Error connecting to grpc server. Err: %v", err)
		return nil
	}

	return rpcServer
}

// Create NMD and Agent
func createNMD(t *testing.T, dbPath, nodeID, restURL string) (*nmd.Agent, error) {

	// create a platform agent
	pa, err := platform.NewNaplesPlatformAgent()
	if err != nil {
		log.Fatalf("Error creating platform agent. Err: %v", err)
	}

	r := resolver.New(&resolver.Config{Name: t.Name(), Servers: strings.Split(*resolverURL, ",")})
	// create the new NMD
	ag, err := nmd.NewAgent(pa, dbPath, nodeID, *cmdURL, "", restURL, "classic", r)
	if err != nil {
		t.Errorf("Error creating NMD. Err: %v", err)
	}

	return ag, err
}

// stopAgent stops NMD server and deletes emDB file
func stopNMD(t *testing.T, ag *nmd.Agent, dbPath string) {

	ag.Stop()
	err := os.Remove(dbPath)
	if err != nil {
		log.Errorf("Error deleting emDB file: %s, err: %v", dbPath, err)
	}
}

func TestNICConfig(t *testing.T) {

	for i := 1; i <= *numNaples; {

		// Testgroup to run sub-tests in parallel, bounded by runtime.NumCPU
		t.Run("testgroup", func(t *testing.T) {

			batchSize := gorun.NumCPU()

			log.Infof("#### Starting Tests [%d..%d]", i, i+batchSize-1)
			for j := 0; j < batchSize && i <= *numNaples; i, j = i+1, j+1 {

				tcName := fmt.Sprintf("TestNMD-%d", i)
				nodeID := getNodeID(i)
				dbPath := getDBPath(i)
				restURL := getRESTUrl(i)

				// Sub-Test for a single NMD agent
				t.Run(tcName, func(t *testing.T) {

					// Execute Agent/NMD creation and registration tests in parallel
					t.Parallel()
					log.Infof("#### Started TC: %s NodeID: %s DB: %s GoRoutines: %d CGoCalls: %d",
						tcName, nodeID, dbPath, gorun.NumGoroutine(), gorun.NumCgoCall())

					// Cleanup any prior DB files
					os.Remove(dbPath)

					// create Agent and NMD
					ag, err := createNMD(t, dbPath, nodeID, restURL)
					defer stopNMD(t, ag, dbPath)
					Assert(t, (err == nil && ag != nil), "Failed to create agent", err)

					nm := ag.GetNMD()

					// Validate default classic mode
					f1 := func() (bool, interface{}) {

						cfg := nm.GetNaplesConfig()
						if cfg.Spec.Mode == proto.NaplesMode_CLASSIC_MODE && nm.GetListenURL() != "" &&
							nm.GetUpdStatus() == false && nm.GetRegStatus() == false && nm.GetRestServerStatus() == true {
							return true, nil
						}
						return false, nil
					}
					AssertEventually(t, f1, "Failed to verify mode is in Classic")

					// Create SmartNIC object in Venice
					nic := cmd.SmartNIC{
						TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
						ObjectMeta: api.ObjectMeta{
							Name: nodeID,
						},
						Spec: cmd.SmartNICSpec{
							MgmtIp:   "localhost",
							Phase:    "UNKNOWN",
							NodeName: nodeID,
						},
					}

					_, err = tInfo.apiClient.ClusterV1().SmartNIC().Create(context.Background(), &nic)
					if err != nil {
						t.Errorf("Failed to created smartnic: %+v, err: %v", nic, err)
					}

					// Verify the Naples received the config and switched to Managed Mode
					f4 := func() (bool, interface{}) {

						// validate the mode is managed
						cfg := nm.GetNaplesConfig()
						log.Infof("NaplesConfig: %v", cfg)
						if cfg.Spec.Mode != proto.NaplesMode_MANAGED_MODE {
							log.Errorf("Failed to switch to managed mode")
							return false, nil
						}

						// Fetch smartnic object
						nic, err := nm.GetSmartNIC()
						if nic == nil || err != nil {
							log.Errorf("NIC not found in nicDB, mac:%s", nodeID)
							return false, nil
						}

						// Verify NIC is admitted
						if nic.Spec.Phase != cmd.SmartNICSpec_ADMITTED.String() {
							log.Errorf("NIC is not admitted")
							return false, nil
						}

						// Verify Update NIC task is running
						if nm.GetUpdStatus() == false {
							log.Errorf("Update NIC is not in progress")
							return false, nil
						}

						// Verify REST server is not up
						if nm.GetRestServerStatus() == true {
							log.Errorf("REST server is still up")
							return false, nil
						}
						return true, nil
					}
					AssertEventually(t, f4, "Failed to verify mode is in Managed Mode", string("1s"), string("60s"))

					// Validate SmartNIC object state is updated on Venice
					f5 := func() (bool, interface{}) {

						meta := api.ObjectMeta{
							Name: nodeID,
						}
						nicObj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), &meta)
						if err != nil || nicObj == nil || nicObj.Spec.Phase != cmd.SmartNICSpec_ADMITTED.String() {
							log.Errorf("Failed to validate phase of SmartNIC object, mac:%s, phase: %s err: %v",
								nodeID, nicObj.Spec.Phase, err)
							return false, nil
						}

						return true, nil
					}
					AssertEventually(t, f5, "Failed to verify creation of required SmartNIC object", string("10ms"), string("30s"))

					// Validate Workload Node object is created
					f6 := func() (bool, interface{}) {

						meta := api.ObjectMeta{
							Name: nodeID,
						}
						nodeObj, err := tInfo.apiClient.ClusterV1().Node().Get(context.Background(), &meta)
						if err != nil || nodeObj == nil {
							log.Errorf("Failed to GET Node object, mac:%s, %v", nodeID, err)
							return false, nil
						}

						return true, nil
					}
					AssertEventually(t, f6, "Failed to verify creation of required Node object", string("10ms"), string("30s"))

					log.Infof("#### Completed TC: %s NodeID: %s DB: %s GoRoutines: %d CGoCalls: %d ",
						tcName, nodeID, dbPath, gorun.NumGoroutine(), gorun.NumCgoCall())

				})
			}
		})
		log.Infof("#### Completed TestGroup")

	}
}

func Setup(m *testing.M) {

	// Disable open trace
	ventrace.DisableOpenTrace()
	tsdb.Init(tsdb.DummyTransmitter{}, tsdb.Options{})

	// Fill logger config params
	os.Remove("/tmp/nicconfig.log")
	logConfig := &log.Config{
		Module:      "Nic-Config-test",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
		FileCfg: log.FileConfig{
			Filename:   "/tmp/niconfig.log",
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	pl := log.SetConfig(logConfig)

	// Create api server
	apiServerAddress := ":0"
	tInfo.l = pl
	scheme := runtime.NewScheme()
	srvConfig := apiserver.Config{
		GrpcServerPort: apiServerAddress,
		DebugMode:      false,
		Logger:         pl,
		Version:        "v1",
		Scheme:         scheme,
		Kvstore: store.Config{
			Type:  store.KVStoreTypeMemkv,
			Codec: runtime.NewJSONCodec(scheme),
		},
	}
	grpclog.SetLogger(pl)
	tInfo.apiServer = apiserverpkg.MustGetAPIServer()
	go tInfo.apiServer.Run(srvConfig)
	tInfo.apiServer.WaitRunning()
	addr, err := tInfo.apiServer.GetAddr()
	log.Infof("Api server address : %+v err: %v", addr, err)
	if err != nil {
		os.Exit(-1)
	}

	// Create api client
	tInfo.apiServerAddr = addr
	apiCl, err := apicache.NewGrpcUpstream("nic_config_test", tInfo.apiServerAddr, tInfo.l)
	if err != nil {
		fmt.Printf("Cannot create gRPC client - %v", err)
		os.Exit(-1)
	}
	tInfo.apiClient = apiCl

	// create CMD
	tInfo.rpcServer = createCMD(m)
	if tInfo.rpcServer == nil {
		fmt.Printf("Err creating rpc server & client")
		os.Exit(-1)
	}

	// Check if no cluster exists to start with - negative test
	_, err = tInfo.smartNICServer.GetCluster()
	if err == nil {
		fmt.Printf("Unexpected cluster object found, err: %s", err)
		os.Exit(-1)
	}

	// Create test cluster object
	clRef := &cmd.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
		Spec: cmd.ClusterSpec{
			AutoAdmitNICs: true,
		},
	}
	_, err = tInfo.apiClient.ClusterV1().Cluster().Create(context.Background(), clRef)
	if err != nil {
		fmt.Printf("Error creating Cluster object, %v", err)
		os.Exit(-1)
	}

	log.Infof("#### ApiServer and CMD smartnic server is UP")
}

func Teardown(m *testing.M) {

	// stop the CMD smartnic RPC server
	tInfo.rpcServer.Stop()

	// stop the apiServer
	tInfo.apiServer.Stop()

	log.Infof("#### ApiServer and CMD smartnic server is STOPPED")
}

func waitForTracer() {
	for {
		log.Infof("#### Sleep,  GoRoutines: %d CGoCalls: %d", gorun.NumGoroutine(), gorun.NumCgoCall())
		time.Sleep(1 * time.Second)
	}
}

func TestMain(m *testing.M) {

	flag.Parse()
	log.Infof("#### TestMain num-Agents:%d #CPU: %d", *numNaples, gorun.NumCPU())
	log.Infof("#### TestMain Start GoRoutines: %d CGoCalls: %d", gorun.NumGoroutine(), gorun.NumCgoCall())

	// Validate args
	if *numNaples < minAgents || *numNaples > maxAgents {
		log.Errorf("Invalid numNaples, supported range is [%d..%d]", minAgents, maxAgents)
		os.Exit(-1)
	}

	// Enable gRPC tracing and start gRPC REST debug server for tracing
	if *rpcTrace == true {
		rpc.EnableTracing = true
		go func() {
			log.Printf("Failed to launch listener at 4444, %v",
				http.ListenAndServe("localhost:4444", nil))
			os.Exit(-1)
		}()

		trace.AuthRequest = func(req *http.Request) (any, sensitive bool) {
			return true, true
		}
	}

	// Setup
	Setup(m)

	// Run tests
	rcode := m.Run()

	// Tear down
	Teardown(m)

	log.Infof("#### TestMain End GoRoutines: %d CGoCalls: %d", gorun.NumGoroutine(), gorun.NumCgoCall())

	if *rpcTrace == true {
		waitForTracer()
	}

	os.Exit(rcode)
}
