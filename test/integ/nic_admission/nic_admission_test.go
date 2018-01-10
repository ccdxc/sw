// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package nicadmission

import (
	"context"
	"flag"
	"fmt"
	"net"
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
	apicache "github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cmd"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	nmd "github.com/pensando/sw/nic/agent/nmd"
	"github.com/pensando/sw/nic/agent/nmd/platform"
	proto "github.com/pensando/sw/nic/agent/nmd/protos"
	nmdstate "github.com/pensando/sw/nic/agent/nmd/state"
	apiserver "github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/cmd/cache"
	cmdenv "github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore/etcd/integration"
	store "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/testenv"
	. "github.com/pensando/sw/venice/utils/testutils"
	ventrace "github.com/pensando/sw/venice/utils/trace"
	"github.com/pensando/sw/venice/utils/tsdb"
)

const (
	smartNICServerURL = "localhost:9199"
	resolverURLs      = ":" + globals.CMDGRPCPort
	minAgents         = 1
	maxAgents         = 5000
	// TLS keys and certificates used by mock CKM endpoint to generate control-plane certs
	certPath  = "../../../venice/utils/certmgr/testdata/ca.cert.pem"
	keyPath   = "../../../venice/utils/certmgr/testdata/ca.key.pem"
	rootsPath = "../../../venice/utils/certmgr/testdata/roots.pem"
)

var (
	numNaples   = flag.Int("num-naples", 100, fmt.Sprintf("Number of Naples instances [%d..%d]", minAgents, maxAgents))
	cmdURL      = flag.String("cmd-url", smartNICServerURL, "CMD URL")
	resolverURL = flag.String("resolver-url", resolverURLs, "Resolver URLs")
	mode        = flag.String("mode", "classic", "Naples mode, classic or managed")
	rpcTrace    = flag.Bool("rpc-trace", false, "Enable gRPC tracing")
)

type testInfo struct {
	l              log.Logger
	apiServerPort  string
	apiServer      apiserver.Server
	apiClient      apiclient.Services
	certSrv        *certsrv.CertSrv
	rpcServer      *rpckit.RPCServer
	smartNICServer *smartnic.RPCServer
}

var tInfo testInfo

func (t testInfo) APIClient() cmd.CmdV1Interface {
	return t.apiClient.CmdV1()
}

func getNodeID(index int) string {
	return fmt.Sprintf("44.44.44.44.%02x.%02x", index/256, index%256)
}

func getRESTUrl(index int) string {
	return fmt.Sprintf(":%d", 10000+index)
}

func getDBPath(index int) string {
	return fmt.Sprintf("/tmp/nmd-%d.db", index)
}

// launchCMDServer creates a smartNIC CMD server for SmartNIC service.
func launchCMDServer(m *testing.M, url string) (*rpckit.RPCServer, error) {

	// create an RPC server.
	rpcServer, err := rpckit.NewRPCServer("smartNIC", url, rpckit.WithTLSProvider(nil))
	if err != nil {
		fmt.Printf("Error creating RPC-server: %v", err)
		return nil, err
	}
	tInfo.rpcServer = rpcServer
	cmdenv.RPCServer = rpcServer

	// create and register the RPC handler for SmartNIC service
	tInfo.smartNICServer, err = smartnic.NewRPCServer(tInfo,
		smartnic.HealthWatchInterval,
		smartnic.DeadInterval,
		globals.NmdRESTPort,
		cache.NewStatemgr())

	if err != nil {
		fmt.Printf("Error creating Smart NIC server: %v", err)
		return nil, err
	}
	grpc.RegisterSmartNICServer(rpcServer.GrpcServer, tInfo.smartNICServer)
	rpcServer.Start()

	return rpcServer, nil
}

// createRPCServerClient creates rpc server for SmartNIC service
func createRPCServer(m *testing.M) *rpckit.RPCServer {

	// start the rpc server
	rpcServer, err := launchCMDServer(m, *cmdURL)
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
	var resolverClient resolver.Interface
	if resolverURL != nil && *resolverURL != "" {
		resolverClient = resolver.New(&resolver.Config{Servers: strings.Split(*resolverURL, ",")})
	}
	// create the new NMD
	ag, err := nmd.NewAgent(pa, dbPath, nodeID, *cmdURL, restURL, *mode, resolverClient)
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
		log.Fatalf("Error deleting emDB file: %s, err: %v", dbPath, err)
	}
}

func TestCreateNMDs(t *testing.T) {
	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})

	for i := 1; i <= *numNaples; {

		// Testgroup to run sub-tests in parallel, bounded by runtime.NumCPU
		t.Run("testgroup", func(t *testing.T) {

			batchSize := gorun.NumCPU()

			log.Infof("#### Staring Tests [%d..%d]", i, i+batchSize-1)
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

					if *mode == "classic" {
						// Validate default classic mode
						f1 := func() (bool, []interface{}) {

							cfg := nm.GetNaplesConfig()
							if cfg.Spec.Mode == proto.NaplesMode_CLASSIC_MODE && nm.GetListenURL() != "" &&
								nm.GetUpdStatus() == false && nm.GetRegStatus() == false && nm.GetRestServerStatus() == true {
								return true, nil
							}
							return false, nil
						}
						AssertEventually(t, f1, "Failed to verify mode is in Classic")

						var naplesCfg proto.Naples

						// Validate REST endpoint
						f2 := func() (bool, []interface{}) {

							err := netutils.HTTPGet(nm.GetNMDUrl()+"/", &naplesCfg)
							if err != nil {
								log.Errorf("Failed to get naples config via REST, err:%+v", err)
								return false, nil
							}

							if naplesCfg.Spec.Mode != proto.NaplesMode_CLASSIC_MODE {
								return false, nil
							}
							return true, nil
						}
						AssertEventually(t, f2, "Failed to get the default naples config via REST")

						// Switch to Managed mode
						naplesCfg = proto.Naples{
							ObjectMeta: api.ObjectMeta{Name: "NaplesConfig"},
							TypeMeta:   api.TypeMeta{Kind: "Naples"},
							Spec: proto.NaplesSpec{
								Mode:           proto.NaplesMode_MANAGED_MODE,
								PrimaryMac:     nodeID,
								ClusterAddress: []string{*cmdURL},
								NodeName:       nodeID,
							},
						}

						log.Infof("Naples config: %+v", naplesCfg)

						var resp nmdstate.NaplesConfigResp

						// Post the mode change config
						f3 := func() (bool, []interface{}) {
							err = netutils.HTTPPost(nm.GetNMDUrl(), &naplesCfg, &resp)
							if err != nil {
								log.Errorf("Failed to post naples config, err:%+v resp:%+v", err, resp)
								return false, nil
							}
							return true, nil
						}
						AssertEventually(t, f3, "Failed to post the naples config")
					}

					// Validate Managed Mode
					f4 := func() (bool, []interface{}) {

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
					AssertEventually(t, f4, "Failed to verify mode is in Managed Mode", string("10ms"), string("60s"))

					// Validate SmartNIC object is created
					f5 := func() (bool, []interface{}) {

						meta := api.ObjectMeta{
							Name: nodeID,
						}
						nicObj, err := tInfo.apiClient.CmdV1().SmartNIC().Get(context.Background(), &meta)
						if err != nil || nicObj == nil {
							log.Errorf("Failed to GET SmartNIC object, mac:%s, %v", nodeID, err)
							return false, nil
						}

						return true, nil
					}
					AssertEventually(t, f5, "Failed to verify creation of required SmartNIC object", string("10ms"), string("30s"))

					// Validate Workload Node object is created
					f6 := func() (bool, []interface{}) {

						meta := api.ObjectMeta{
							Name: nodeID,
						}
						nodeObj, err := tInfo.apiClient.CmdV1().Node().Get(context.Background(), &meta)
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

	// Init etcd cluster
	var t testing.T

	// start certificate server
	// need to do this before Chdir() so that it finds the certificates on disk
	certSrv, err := certsrv.NewCertSrv("localhost:0", certPath, keyPath, rootsPath)
	if err != nil {
		log.Errorf("Failed to create certificate server: %v", err)
		os.Exit(-1)
	}
	tInfo.certSrv = certSrv
	log.Infof("Created cert endpoint at %s", globals.CMDCertAPIPort)

	// instantiate a CKM-based TLS provider and make it default for all rpckit clients and servers
	testenv.EnableRpckitTestMode()
	tlsProvider := func(svcName string) (rpckit.TLSProvider, error) {
		return tlsproviders.NewDefaultCKMBasedProvider(certSrv.GetListenURL(), svcName)
	}
	rpckit.SetTestModeDefaultTLSProvider(tlsProvider)

	// cluster bind mounts in local directory. certain filesystems (like vboxsf, nfs) dont support unix binds.
	os.Chdir("/tmp")
	cluster := integration.NewClusterV3(&t)

	// Disable open trace
	ventrace.DisableOpenTrace()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "Nic-Admission-Test",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
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
		KVPoolSize:     8,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeEtcd,
			Servers: strings.Split(cluster.ClientURL(), ","),
			Codec:   runtime.NewJSONCodec(scheme),
		},
	}
	grpclog.SetLogger(pl)
	tInfo.apiServer = apiserverpkg.MustGetAPIServer()
	go tInfo.apiServer.Run(srvConfig)
	tInfo.apiServer.WaitRunning()
	addr, err := tInfo.apiServer.GetAddr()
	if err != nil {
		os.Exit(-1)
	}
	_, port, err := net.SplitHostPort(addr)
	if err != nil {
		os.Exit(-1)
	}

	tInfo.apiServerPort = port

	// Create api client
	apiServerAddr := "localhost" + ":" + tInfo.apiServerPort
	apiCl, err := apicache.NewGrpcUpstream(apiServerAddr, tInfo.l)
	if err != nil {
		fmt.Printf("Cannot create gRPC client - %v", err)
		os.Exit(-1)
	}
	tInfo.apiClient = apiCl

	// create gRPC server for smartNIC service
	tInfo.rpcServer = createRPCServer(m)
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
	_, err = tInfo.apiClient.CmdV1().Cluster().Create(context.Background(), clRef)
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
	tInfo.apiClient.Close()

	// stop the apiServer
	tInfo.apiServer.Stop()

	// stop certificate server
	tInfo.certSrv.Stop()

	log.Infof("#### ApiServer and CMD smartnic server is STOPPED")
}

func waitForTracer() {
	for {
		log.Infof("#### Sleep,  GoRoutines: %d CGoCalls: %d", gorun.NumGoroutine(), gorun.NumCgoCall())
		time.Sleep(1 * time.Second)
	}
}

// Here are some examples to run this testcase :
//
// $ go test ./test/integ/nic_admission/
// ok  github.com/pensando/sw/test/integ/nic_admission	10.031s
//
// $ go test -p 8 ./test/integ/nic_admission/ -args -num-naples=50
// ok  github.com/pensando/sw/test/integ/nic_admission	1.709s
//
// $  go test -p 8 ./test/integ/nic_admission/ -args -num-naples=100 -rpc-trace=true
// ok  	github.com/pensando/sw/test/integ/nic_admission	9.570s
//
// $  go test -p 8 ./test/integ/nic_admission/ -args -num-naples=100 -mode=managed
// ok  github.com/pensando/sw/test/integ/nic_admission	7.558s

// TODO : Need to investigate goroutine blocking & gRPC timeout with -p <N> option when #agents > 100
// TODO : This is work in progress.

func TestMain(m *testing.M) {

	flag.Parse()
	log.Infof("#### TestMain num-Agents:%d #CPU: %d", *numNaples, gorun.NumCPU())
	log.Infof("#### TestMain Start GoRoutines: %d CGoCalls: %d", gorun.NumGoroutine(), gorun.NumCgoCall())

	// Validate args
	if *numNaples < minAgents || *numNaples > maxAgents {
		log.Errorf("Invalid numNaples, supported range is [%d..%d]", minAgents, maxAgents)
		os.Exit(-1)
	}

	if *mode != "classic" && *mode != "managed" {
		log.Errorf("Invalid mode, supported mode is classic or managed")
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
