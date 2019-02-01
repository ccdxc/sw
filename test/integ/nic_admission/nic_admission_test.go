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
	api_cache "github.com/pensando/sw/api/cache"
	apicache "github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	pencluster "github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	nmd "github.com/pensando/sw/nic/agent/nmd"
	"github.com/pensando/sw/nic/agent/nmd/platform"
	proto "github.com/pensando/sw/nic/agent/nmd/protos"
	nmdstate "github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/nic/agent/nmd/upg"
	testutils "github.com/pensando/sw/test/utils"
	apiserver "github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/cmd/cache"
	cmdenv "github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore/etcd/integration"
	store "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
	ventrace "github.com/pensando/sw/venice/utils/trace"
	"github.com/pensando/sw/venice/utils/tsdb"
)

const (
	smartNICServerURL = "localhost:0"
	resolverURLs      = ":" + globals.CMDResolverPort
	minAgents         = 1
	maxAgents         = 5000
	nicRegIntvl       = 1 * time.Second
	nicUpdIntvl       = 1 * time.Second
)

var (
	numNaples   = flag.Int("num-naples", 100, fmt.Sprintf("Number of Naples instances [%d..%d]", minAgents, maxAgents))
	cmdRegURL   = flag.String("cmd-reg-url", smartNICServerURL, "CMD Registration URL")
	cmdUpdURL   = flag.String("cmd-upd-url", smartNICServerURL, "CMD Updates URL")
	resolverURL = flag.String("resolver-url", resolverURLs, "Resolver URLs")
	mode        = flag.String("mode", "host", "Naples mode, host or network")
	rpcTrace    = flag.Bool("rpc-trace", false, "Enable gRPC tracing")

	logger = log.GetNewLogger(log.GetDefaultConfig("nic_admission_test"))

	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "nic_admission_test"},
		EvtTypes:      append(pencluster.GetEventTypes(), evtsapi.GetEventTypes()...),
		BackupDir:     "/tmp",
		SkipEvtsProxy: true}, logger)
)

type testInfo struct {
	l              log.Logger
	apiServerPort  string
	apiServer      apiserver.Server
	apiClient      apiclient.Services
	regRPCServer   *rpckit.RPCServer
	updRPCServer   *rpckit.RPCServer
	smartNICServer *smartnic.RPCServer
	resolverServer *rpckit.RPCServer
}

type nmdInfo struct {
	agent          *nmd.Agent
	resolverClient resolver.Interface
	dbPath         string
}

var tInfo testInfo

func (t testInfo) APIClient() pencluster.ClusterV1Interface {
	return t.apiClient.ClusterV1()
}

func getSmartNICMAC(index int) string {
	return fmt.Sprintf("44:44:44:44:%02x:%02x", index/256, index%256)
}

func getHost(index int) *pencluster.Host {
	return &pencluster.Host{
		ObjectMeta: api.ObjectMeta{
			Name: fmt.Sprintf("e2e-host-%02d", index),
		},
		Spec: pencluster.HostSpec{
			SmartNICs: []pencluster.SmartNICID{
				{
					MACAddress: getSmartNICMAC(index),
				},
			},
		},
		Status: pencluster.HostStatus{},
	}
}

func getRESTUrl(index int) string {
	return fmt.Sprintf(":%d", 20000+index)
}

func getDBPath(index int) string {
	return fmt.Sprintf("/tmp/nmd-%d.db", index)
}

// launchCMDServers creates smartNIC CMD servers for registration and updates
func launchCMDServers(m *testing.M, regURL, updURL string) (*rpckit.RPCServer, *rpckit.RPCServer, error) {

	// create an RPC server for SmartNIC service
	regRPCServer, err := rpckit.NewRPCServer("smartNIC", regURL, rpckit.WithTLSProvider(nil))
	if err != nil {
		fmt.Printf("Error creating NIC registration RPC-server: %v", err)
		return nil, nil, err
	}
	tInfo.regRPCServer = regRPCServer
	cmdenv.CertMgr, err = certmgr.NewTestCertificateMgr("smartnic-test")
	if err != nil {
		return nil, nil, fmt.Errorf("Error creating CertMgr instance: %v", err)
	}
	cmdenv.UnauthRPCServer = regRPCServer
	cmdenv.StateMgr = cache.NewStatemgr()

	// create and register the RPC handler for SmartNIC service
	tInfo.smartNICServer, err = smartnic.NewRPCServer(tInfo,
		smartnic.HealthWatchInterval,
		smartnic.DeadInterval,
		globals.NmdRESTPort,
		cmdenv.StateMgr)

	if err != nil {
		fmt.Printf("Error creating Smart NIC server: %v", err)
		return nil, nil, err
	}
	grpc.RegisterSmartNICRegistrationServer(regRPCServer.GrpcServer, tInfo.smartNICServer)
	regRPCServer.Start()
	*cmdRegURL = regRPCServer.GetListenURL()

	// Also create a mock resolver
	rs := mock.NewResolverService()
	resolverHandler := service.NewRPCHandler(rs)
	resolverServer, err := rpckit.NewRPCServer(globals.Cmd, *resolverURL, rpckit.WithTracerEnabled(true))
	types.RegisterServiceAPIServer(resolverServer.GrpcServer, resolverHandler)
	resolverServer.Start()
	tInfo.resolverServer = resolverServer

	// create separate provider running on auth port for health updates
	updRPCServer, err := rpckit.NewRPCServer("smartNIC", updURL)
	if err != nil {
		fmt.Printf("Error creating NIC updates RPC-server: %v", err)
		return nil, nil, err
	}
	tInfo.updRPCServer = updRPCServer
	cmdenv.AuthRPCServer = updRPCServer
	grpc.RegisterSmartNICUpdatesServer(updRPCServer.GrpcServer, tInfo.smartNICServer)
	updRPCServer.Start()
	*cmdUpdURL = updRPCServer.GetListenURL()

	return regRPCServer, updRPCServer, nil
}

// Create NMD and Agent
func createNMD(t *testing.T, dbPath, priMac, restURL, mgmtMode string) (*nmdInfo, error) {

	// create a platform agent
	pa, err := platform.NewNaplesPlatformAgent()
	if err != nil {
		log.Fatalf("Error creating platform agent. Err: %v", err)
	}
	uc, err := upg.NewNaplesUpgradeClient(nil)
	if err != nil {
		log.Fatalf("Error creating Upgrade client . Err: %v", err)
	}

	var resolverClient resolver.Interface
	if resolverURL != nil && *resolverURL != "" {
		resolverCfg := &resolver.Config{
			Name:    "TestNMD",
			Servers: strings.Split(*resolverURL, ","),
		}
		resolverClient = resolver.New(resolverCfg)
	}

	// create the new NMD
	ag, err := nmd.NewAgent(pa,
		uc,
		dbPath,
		priMac,
		priMac,
		*cmdRegURL,
		*cmdUpdURL,
		restURL,
		"", // no local certs endpoint
		"", // no remote certs endpoint
		mgmtMode,
		nicRegIntvl,
		nicUpdIntvl,
		resolverClient)
	if err != nil {
		t.Errorf("Error creating NMD. Err: %v", err)
		return nil, err
	}

	ni := &nmdInfo{
		agent:          ag,
		resolverClient: resolverClient,
		dbPath:         dbPath,
	}
	// Ensure the NMD's rest server is started
	nmdHandle := ag.GetNMD()
	nmdHandle.CreateIPClient(nil)
	nmdHandle.UpdateMgmtIP()

	return ni, err
}

// stopAgent stops NMD server and deletes emDB file
func stopNMD(t *testing.T, i *nmdInfo) {
	i.agent.Stop()
	i.resolverClient.Stop()
	err := os.Remove(i.dbPath)
	if err != nil {
		log.Fatalf("Error deleting emDB file: %s, err: %v", i.dbPath, err)
	}
}

func TestCreateNMDs(t *testing.T) {
	t.Skip("Temporarily disabled. TODO. More investigation needed")
	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})

	for i := 1; i <= *numNaples; {

		// Testgroup to run sub-tests in parallel, bounded by runtime.NumCPU
		t.Run("testgroup", func(t *testing.T) {

			batchSize := gorun.NumCPU()

			log.Infof("#### Staring Tests [%d..%d]", i, i+batchSize-1)
			for j := 0; j < batchSize && i <= *numNaples; i, j = i+1, j+1 {

				tcName := fmt.Sprintf("TestNMD-%d", i)
				host := getHost(i)
				priMac := getSmartNICMAC(i)
				dbPath := getDBPath(i)
				restURL := getRESTUrl(i)

				// Create Host
				_, err := tInfo.apiClient.ClusterV1().Host().Create(context.Background(), host)
				AssertOk(t, err, fmt.Sprintf("Error creating host: %v", host.Name))

				// Sub-Test for a single NMD agent
				t.Run(tcName, func(t *testing.T) {

					// Execute Agent/NMD creation and registration tests in parallel
					t.Parallel()
					log.Infof("#### Started TC: %s primary MAC: %s DB: %s GoRoutines: %d CGoCalls: %d",
						tcName, priMac, dbPath, gorun.NumGoroutine(), gorun.NumCgoCall())

					// Cleanup any prior DB files
					os.Remove(dbPath)

					// create Agent and NMD
					nmdInst, err := createNMD(t, dbPath, priMac, restURL, *mode)
					defer stopNMD(t, nmdInst)
					Assert(t, (err == nil && nmdInst.agent != nil), "Failed to create agent", err)

					nm := nmdInst.agent.GetNMD()

					if *mode == "host" {
						// Validate default host mode
						f1 := func() (bool, interface{}) {

							cfg := nm.GetNaplesConfig()
							if cfg.Spec.Mode == proto.MgmtMode_HOST && nm.GetListenURL() != "" &&
								nm.GetUpdStatus() == false && nm.GetRegStatus() == false && nm.GetRestServerStatus() == true {
								return true, nil
							}
							return false, nil
						}
						AssertEventually(t, f1, "Failed to verify mode is in Classic")

						var naplesCfg proto.Naples

						// Validate REST endpoint
						f2 := func() (bool, interface{}) {

							err := netutils.HTTPGet(nm.GetNMDUrl()+"/", &naplesCfg)
							if err != nil {
								log.Errorf("Failed to get naples config via REST, err:%+v", err)
								return false, nil
							}

							if naplesCfg.Spec.Mode != proto.MgmtMode_HOST {
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
								Mode:        proto.MgmtMode_NETWORK,
								NetworkMode: proto.NetworkMode_OOB,
								Controllers: []string{*cmdRegURL},
								Hostname:    priMac,
								PrimaryMAC:  priMac,
							},
						}

						log.Infof("Naples config: %+v", naplesCfg)

						var resp nmdstate.NaplesConfigResp

						// Post the mode change config
						f3 := func() (bool, interface{}) {
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
					f4 := func() (bool, interface{}) {

						// validate the mode is managed
						cfg := nm.GetNaplesConfig()
						log.Infof("NaplesConfig: %v", cfg)
						if cfg.Spec.Mode != proto.MgmtMode_NETWORK {
							log.Errorf("Failed to switch to managed mode")
							return false, nil
						}

						// Fetch smartnic object
						nic, err := nm.GetSmartNIC()
						if nic == nil || err != nil {
							log.Errorf("NIC not found in nicDB, mac:%s", priMac)
							return false, nil
						}

						// Verify NIC is admitted
						if nic.Status.AdmissionPhase != pencluster.SmartNICStatus_ADMITTED.String() {
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
					f5 := func() (bool, interface{}) {

						meta := api.ObjectMeta{
							Name: priMac,
						}
						nicObj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), &meta)
						if err != nil || nicObj == nil {
							log.Errorf("Failed to GET SmartNIC object, mac:%s, %v", priMac, err)
							return false, nil
						}

						return true, nil
					}
					AssertEventually(t, f5, "Failed to verify creation of required SmartNIC object", string("10ms"), string("30s"))

					// Validate Host object is created
					f6 := func() (bool, interface{}) {

						meta := api.ObjectMeta{
							Name: host.Name,
						}
						hostObj, err := tInfo.apiClient.ClusterV1().Host().Get(context.Background(), &meta)
						if err != nil || hostObj == nil {
							log.Errorf("Failed to GET Host object:%s, %v", host.Name, err)
							return false, nil
						}
						for ii := range hostObj.Status.AdmittedSmartNICs {
							if hostObj.Status.AdmittedSmartNICs[ii] == priMac {
								return true, nil
							}
						}
						return false, nil
					}
					AssertEventually(t, f6, "Failed to verify creation of required Host object", string("10ms"), string("30s"))

					log.Infof("#### Completed TC: %s NodeID: %s DB: %s GoRoutines: %d CGoCalls: %d ",
						tcName, priMac, dbPath, gorun.NumGoroutine(), gorun.NumCgoCall())

				})
			}
		})
		log.Infof("#### Completed TestGroup")

	}
}

func checkHealthStatus(t *testing.T, meta *api.ObjectMeta, admPhase string) {
	f := func() (bool, interface{}) {
		nicObj, err := cmdenv.StateMgr.FindSmartNIC(meta.Tenant, meta.Name)
		AssertOk(t, err, "Failed to retrieve NIC from local cache")
		if nicObj.Status.AdmissionPhase != admPhase {
			log.Errorf("NIC not in expected admission phase. Have: %s, want: %s", nicObj.Status.AdmissionPhase, admPhase)
			return false, nil
		}
		if admPhase == pencluster.SmartNICStatus_ADMITTED.String() {
			if len(nicObj.Status.Conditions) == 0 {
				log.Errorf("NIC Object does not have any condition, nic:%s, %v", meta.Name, nicObj.Status)
				return false, nil
			}
			hc := nicObj.Status.Conditions[0]
			if hc.Type != "HEALTHY" || hc.Status != "TRUE" || hc.LastTransitionTime == "" {
				log.Errorf("NIC Object is not healthy, name:%s, %+v", meta.Name, nicObj.Status)
				return false, nil
			}
		} else {
			if len(nicObj.Status.Conditions) != 0 {
				log.Errorf("NIC %s is not in ADMITTED state but has conditions: %v", meta.Name, nicObj.Status)
				return false, nil
			}
		}
		return true, nil
	}
	AssertEventually(t, f, "Failed to verify health updates", "500ms", "30s")
}

func checkNetworkModeNMDStatus(t *testing.T, nmd *nmdstate.NMD, admPhase string) {
	var expRegStatus, expUpdStatus, expUpdWatchStatus, expRoWatchStatus bool

	switch admPhase {
	case pencluster.SmartNICStatus_ADMITTED.String():
		expRegStatus, expUpdStatus, expUpdWatchStatus, expRoWatchStatus = false, true, true, true
	case pencluster.SmartNICStatus_PENDING.String():
		expRegStatus, expUpdStatus, expUpdWatchStatus, expRoWatchStatus = true, false, false, false
	default:
		panic(fmt.Sprintf("Unexpected admission phase %s, NMD: %+v", admPhase, nmd))
	}

	f := func() (bool, interface{}) {
		nic, err := nmd.GetSmartNIC()
		AssertOk(t, err, "NIC not found in nicDB")
		if nic.Status.AdmissionPhase != admPhase {
			log.Errorf("NIC not in expected admission phase in agent %+v,  have: %s, want: %s", nmd, nic.Status.AdmissionPhase, admPhase)
			return false, nil
		}
		if nmd.GetRegStatus() != expRegStatus {
			log.Errorf("Unexpected registration status for agent %+v, phase: %s, have: %+v, want: %+v", nmd, admPhase, nmd.GetRegStatus(), expRegStatus)
			return false, nil
		}
		if nmd.GetUpdStatus() != expUpdStatus {
			log.Errorf("Unexpected NIC updates status for agent %+v, phase: %s, have: %+v, want: %+v", nmd, admPhase, nmd.GetUpdStatus(), expUpdStatus)
			return false, nil
		}
		if nmd.GetCMDSmartNICWatcherStatus() != expUpdWatchStatus {
			log.Errorf("Unexpected NIC updates watch status for agent %+v, phase: %s, have: %+v, want: %+v", nmd, admPhase, nmd.GetCMDSmartNICWatcherStatus(), expUpdWatchStatus)
			return false, nil
		}
		if nmd.GetRoSmartNICWatcherStatus() != expRoWatchStatus {
			log.Errorf("Unexpected rollout updates watch status for agent %+v, phase: %s, have: %+v, want: %+v", nmd, admPhase, nmd.GetRoSmartNICWatcherStatus(), expRoWatchStatus)
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f, fmt.Sprintf("NMD did not reach expected state for phase %s, status: %+v", admPhase, nmd), "500ms", "30s")
}

// TestNICReadmit tests the flow in which a NIC is first admitted, then explicitly de-amitted and the re-admitted.
// Changes are posted to CMD which in turn propagates them to the agent.
func TestNICReadmit(t *testing.T) {
	t.Skip("Temporarily disabled. TODO. More investigation needed")
	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})

	i := 0
	host := getHost(i)
	priMac := getSmartNICMAC(i)
	dbPath := getDBPath(i)
	restURL := getRESTUrl(i)

	meta := api.ObjectMeta{
		Name: priMac,
	}

	// Create Host
	_, err := tInfo.apiClient.ClusterV1().Host().Create(context.Background(), host)
	AssertOk(t, err, fmt.Sprintf("Error creating host: %v", host.Name))

	// Cleanup any prior DB files
	os.Remove(dbPath)

	// create Agent and NMD
	nmdInst, err := createNMD(t, dbPath, priMac, restURL, "network")
	defer stopNMD(t, nmdInst)
	Assert(t, (err == nil && nmdInst.agent != nil), "Failed to create agent", err)

	nmd := nmdInst.agent.GetNMD()

	// Validate Managed Mode
	f1 := func() (bool, interface{}) {
		// validate the mode is managed
		cfg := nmd.GetNaplesConfig()
		log.Infof("NaplesConfig: %v", cfg)
		if cfg.Spec.Mode != proto.MgmtMode_NETWORK {
			log.Errorf("Failed to switch to managed mode")
			return false, nil
		}
		// Verify REST server is not up
		if nmd.GetRestServerStatus() == true {
			log.Errorf("REST server is still up")
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f1, "Failed to verify mode is in Managed Mode", "500ms", "10s")

	// Validate SmartNIC object is created in ApiServer
	f2 := func() (bool, interface{}) {
		nicObj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), &meta)
		if err != nil || nicObj == nil {
			log.Errorf("Failed to GET SmartNIC object, mac:%s, %v", priMac, err)
			return false, nil
		}
		// Create corresponding object in local cache -- normally this is done by a watch on ApiServer
		err = cmdenv.StateMgr.CreateSmartNIC(nicObj)
		AssertOk(t, err, "Error creating nic object in local cache: %v", err)
		return true, nil
	}
	AssertEventually(t, f2, "Failed to verify creation of SmartNIC object", "500ms", "30s")

	// check NIC is admitted in NMD
	checkNetworkModeNMDStatus(t, nmd, pencluster.SmartNICStatus_ADMITTED.String())

	// Turn off auto-admit
	clusterObj, err := tInfo.apiClient.ClusterV1().Cluster().Get(context.Background(), &api.ObjectMeta{Name: "testCluster"})
	AssertOk(t, err, "Error getting cluster object")
	clusterObj.Spec.AutoAdmitNICs = false
	_, err = tInfo.apiClient.ClusterV1().Cluster().Update(context.Background(), clusterObj)
	AssertOk(t, err, "Error updating cluster object")

	// The setNICState function sets the NIC state in ApiServer and CMD in a consistent way
	setNICState := func(phase pencluster.SmartNICStatus_Phase) {
		nicObj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), &meta)
		AssertOk(t, err, "Error getting NIC from ApiServer")
		nicObj.Spec.Admit = (phase == pencluster.SmartNICStatus_ADMITTED)
		nicObj.Status.AdmissionPhase = phase.String()
		nicObj.Status.Conditions = nil
		nicObj, err = tInfo.apiClient.ClusterV1().SmartNIC().Update(context.Background(), nicObj)
		AssertOk(t, err, "Error updating NIC in ApiServer")
		err = cmdenv.StateMgr.UpdateSmartNIC(nicObj)
		AssertOk(t, err, "Failed to update NIC in local cache")
	}

	for i := 0; i < 10; i++ {
		// De-admit NIC
		// NMD is supposed to receive a notification and go back to "pending" state
		setNICState(pencluster.SmartNICStatus_PENDING)

		// Verify that NMD responds to de-admission
		// (stop watchers, stop sending updates, restart registration loop).
		checkNetworkModeNMDStatus(t, nmd, pencluster.SmartNICStatus_PENDING.String())

		// Verify health updates are NOT received on CMD
		time.Sleep(2 * nicUpdIntvl)
		checkHealthStatus(t, &meta, pencluster.SmartNICStatus_PENDING.String())

		// Re-admit NIC
		// NMD should get admitted the next registration attempt
		setNICState(pencluster.SmartNICStatus_ADMITTED)

		// Verify that NMD gets re-admitted, exits registration loop and starts sending updates again
		checkNetworkModeNMDStatus(t, nmd, pencluster.SmartNICStatus_ADMITTED.String())

		// Verify health updates are received on CMD
		checkHealthStatus(t, &meta, pencluster.SmartNICStatus_ADMITTED.String())
	}
}

func Setup(m *testing.M) {

	// Init etcd cluster
	var t testing.T

	// start certificate server and set up TLS provider
	// need to do this before Chdir() so that it finds the certificates on disk
	err := testutils.SetupIntegTLSProvider()
	if err != nil {
		log.Fatalf("Error setting up TLS provider: %v", err)
	}

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
	scheme := runtime.GetDefaultScheme()
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
		GetOverlay: api_cache.GetOverlay,
		IsDryRun:   api_cache.IsDryRun,
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
	apiCl, err := apicache.NewGrpcUpstream("nic_admission_test", apiServerAddr, tInfo.l, apicache.WithServerName(globals.APIServer))
	if err != nil {
		fmt.Printf("Cannot create gRPC client - %v", err)
		os.Exit(-1)
	}
	tInfo.apiClient = apiCl

	// create gRPC server for smartNIC service
	tInfo.regRPCServer, tInfo.updRPCServer, err = launchCMDServers(m, *cmdRegURL, *cmdUpdURL)
	if err != nil {
		panic(fmt.Sprintf("Err creating rpc server: %v", err))
	}

	// Check if no cluster exists to start with - negative test
	_, err = tInfo.smartNICServer.GetCluster()
	if err == nil {
		fmt.Printf("Unexpected cluster object found, err: %s", err)
		os.Exit(-1)
	}

	// Create test cluster object
	clRef := &pencluster.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
		Spec: pencluster.ClusterSpec{
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
	tInfo.regRPCServer.Stop()

	// stop the CMD smartnic RPC server
	tInfo.updRPCServer.Stop()

	// close the apiClient
	tInfo.apiClient.Close()

	// stop the apiServer
	tInfo.apiServer.Stop()

	// stop certificate server
	testutils.CleanupIntegTLSProvider()

	// stop resolver server
	tInfo.resolverServer.Stop()

	time.Sleep(time.Millisecond * 100) // allow goroutines to cleanup and terminate gracefully

	if cmdenv.CertMgr != nil {
		cmdenv.CertMgr.Close()
		cmdenv.CertMgr = nil
	}
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

	if *mode != "host" && *mode != "network" {
		log.Errorf("Invalid mode, supported mode is host or network")
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
