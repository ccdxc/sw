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

	"golang.org/x/net/trace"
	rpc "google.golang.org/grpc"
	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api"
	api_cache "github.com/pensando/sw/api/cache"
	apicache "github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	pencluster "github.com/pensando/sw/api/generated/cluster"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	"github.com/pensando/sw/nic/agent/nmd"
	"github.com/pensando/sw/nic/agent/nmd/platform"
	proto "github.com/pensando/sw/nic/agent/nmd/protos"
	nmdstate "github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/nic/agent/nmd/upg"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	cmdapi "github.com/pensando/sw/venice/cmd/apiclient"
	"github.com/pensando/sw/venice/cmd/cache"
	cmdenv "github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	cmdsvc "github.com/pensando/sw/venice/cmd/services"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certmgr"
	esmock "github.com/pensando/sw/venice/utils/elastic/mock/curator"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/kvstore/etcd/integration"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	rmock "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
	venicetestutils "github.com/pensando/sw/venice/utils/testutils"
	ventrace "github.com/pensando/sw/venice/utils/trace"
	"github.com/pensando/sw/venice/utils/tsdb"
)

const (
	smartNICRegURL  = "localhost:" + globals.CMDSmartNICRegistrationAPIPort
	smartNICUpdURL  = "localhost:" + globals.CMDSmartNICUpdatesPort
	smartNICAuthURL = "localhost:" + globals.CMDGRPCAuthPort
	resolverURLs    = ":" + globals.CMDResolverPort
	minAgents       = 1
	maxAgents       = 5000
	nicRegIntvl     = 1 * time.Second
	nicUpdIntvl     = 1 * time.Second
	nicDeadIntvl    = 3 * time.Second
)

var (
	numNaples   = flag.Int("num-naples", 100, fmt.Sprintf("Number of Naples instances [%d..%d]", minAgents, maxAgents))
	cmdRegURL   = flag.String("cmd-reg-url", smartNICRegURL, "CMD Registration URL")
	cmdUpdURL   = flag.String("cmd-upd-url", smartNICUpdURL, "CMD Updates URL")
	resolverURL = flag.String("resolver-url", resolverURLs, "Resolver URLs")
	mode        = flag.String("mode", "host", "Naples mode, host or network")
	rpcTrace    = flag.Bool("rpc-trace", false, "Enable gRPC tracing")

	logger = log.GetNewLogger(log.GetDefaultConfig("nic_admission_test"))

	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("nic_admission_test", logger))
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
func (t testInfo) CheckNICVersionForAdmission(nicSku string, nicVersion string) (string, string) {
	return "", ""
}

func getSmartNICMAC(index int) string {
	return fmt.Sprintf("00ae.cd01.%02d01", index)
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

// launchCMDServices creates smartNIC CMD servers for registration and updates
func launchCMDServices(m *testing.M, regURL, updURL string) (*rpckit.RPCServer, *rpckit.RPCServer, error) {

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
		nicDeadIntvl,
		globals.NmdRESTPort,
		cmdenv.StateMgr,
		tInfo)
	cmdenv.NICService = tInfo.smartNICServer

	if err != nil {
		fmt.Printf("Error creating Smart NIC server: %v", err)
		return nil, nil, err
	}
	grpc.RegisterSmartNICRegistrationServer(regRPCServer.GrpcServer, tInfo.smartNICServer)
	regRPCServer.Start()
	*cmdRegURL = regRPCServer.GetListenURL()

	// Also create a mock resolver
	rs := mock.NewResolverService()
	nicUpdatesSi := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.CmdNICUpdatesSvc,
		},
		Service: globals.CmdNICUpdatesSvc,
		Node:    "localhost",
		URL:     smartNICUpdURL,
	}
	rs.AddServiceInstance(&nicUpdatesSi)
	resolverHandler := service.NewRPCHandler(rs)
	resolverServer, err := rpckit.NewRPCServer(globals.Cmd, *resolverURL, rpckit.WithTracerEnabled(true))
	if err != nil {
		fmt.Printf("Error creating ResolverServer RPC-server: %v", err)
		return nil, nil, err
	}
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

	// Start CMD config watcher
	cmdenv.Logger = tInfo.l
	cmdenv.QuorumNodes = []string{"localhost"}
	l := mock.NewLeaderService("testMaster")
	l.LeaderID = "testMaster"
	cmdenv.LeaderService = l
	s := cmdsvc.NewSystemdService(cmdsvc.WithSysIfSystemdSvcOption(&mock.SystemdIf{}))
	cw := cmdapi.NewCfgWatcherService(tInfo.l, "localhost:"+tInfo.apiServerPort, cmdenv.StateMgr)
	cmdenv.MasterService = cmdsvc.NewMasterService(
		cmdsvc.WithLeaderSvcMasterOption(l),
		cmdsvc.WithSystemdSvcMasterOption(s),
		cmdsvc.WithConfigsMasterOption(&mock.Configs{}),
		cmdsvc.WithCfgWatcherMasterOption(cw),
		cmdsvc.WithElasticCuratorSvcrOption(esmock.NewMockCurator()))
	cw.Start()

	return regRPCServer, updRPCServer, nil
}

// Create NMD and Agent
func createNMD(t *testing.T, dbPath, priMac, restURL, mgmtMode string) (*nmdInfo, error) {

	venicetestutils.CreateFruJSON(priMac)

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
	nmdHandle.CreateMockIPClient(nil)

	// Fake IPConfig
	ipConfig := &pencluster.IPConfig{
		IPAddress: "1.2.3.4",
	}

	cfg := nmdHandle.GetNaplesConfig()
	cfg.Spec.Controllers = []string{"localhost"}
	cfg.Spec.NetworkMode = proto.NetworkMode_INBAND.String()
	cfg.Spec.IPConfig = ipConfig
	cfg.Spec.MgmtVlan = 0
	cfg.Spec.Hostname = priMac

	nmdHandle.SetNaplesConfig(cfg.Spec)
	nmdHandle.IPClient.Update()

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
	// Init tsdb
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &rmock.ResolverClient{}})
	defer cancel()

	// This test was initially designed to run multiple NMDs in parallel
	// Currently this is not possible any more because multiple calls to createNMD
	// will cause issues due to concurrent access to fru.json. There are also
	// conflicts related to multiple instances of NMD trying to bind to the same port
	// for the REST server.
	// TODO: find workarounds for the issues above and re-enable the test as parallel
	*numNaples = 1
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
							if cfg.Spec.Mode == proto.MgmtMode_HOST.String() && nm.GetListenURL() != "" &&
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

							if naplesCfg.Spec.Mode != proto.MgmtMode_HOST.String() {
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
								Mode:        proto.MgmtMode_NETWORK.String(),
								NetworkMode: proto.NetworkMode_OOB.String(),
								Controllers: []string{"localhost"},
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
						if cfg.Spec.Mode != proto.MgmtMode_NETWORK.String() {
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
							log.Errorf("NIC is not admitted, %+v", nic)
							return false, nil
						}

						// Verify Update NIC task is running
						if nm.GetUpdStatus() == false {
							log.Errorf("Update NIC is not in progress")
							return false, nil
						}
						return true, nil
					}
					AssertEventually(t, f4, "Failed to verify mode is in Managed Mode", string("100ms"), string("60s"))

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
					AssertEventually(t, f5, "Failed to verify creation of required SmartNIC object", string("100ms"), string("30s"))

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
						log.Infof("Host object: %+v", host)
						for ii := range hostObj.Status.AdmittedSmartNICs {
							if hostObj.Status.AdmittedSmartNICs[ii] == priMac {
								return true, nil
							}
						}
						return false, nil
					}
					AssertEventually(t, f6, "Failed to verify creation of required Host object", string("100ms"), string("30s"))

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
		apiSrvObj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), meta)
		AssertOk(t, err, "Failed to retrieve NIC from ApiServer")

		cmdObj, err := cmdenv.StateMgr.FindSmartNIC(meta.Tenant, meta.Name)
		AssertOk(t, err, "Failed to retrieve NIC from local cache")

		for index, nicObj := range []*pencluster.SmartNIC{apiSrvObj, cmdObj.SmartNIC} {
			if nicObj.Status.AdmissionPhase != admPhase {
				log.Errorf("NIC not in expected admission phase. Have: %s, want: %s, index: %d", nicObj.Status.AdmissionPhase, admPhase, index)
				return false, nil
			}
			if admPhase == pencluster.SmartNICStatus_ADMITTED.String() {
				if len(nicObj.Status.Conditions) == 0 {
					log.Errorf("NIC Object does not have any condition, nic:%s, %v, index: %d", meta.Name, nicObj.Status, index)
					return false, nil
				}
				hc := nicObj.Status.Conditions[0]
				if hc.Type != "HEALTHY" || hc.Status != "TRUE" || hc.LastTransitionTime == "" {
					log.Errorf("NIC Object is not healthy, name:%s, %+v, index: %d", meta.Name, nicObj.Status, index)
					return false, nil
				}
			} else {
				if len(nicObj.Status.Conditions) != 0 {
					log.Errorf("NIC %s is not in ADMITTED state but has conditions: %+v, index: %d ", meta.Name, nicObj.Status, index)
					return false, nil
				}
			}
		}
		return true, nil
	}
	AssertEventually(t, f, "Failed to verify health updates", "500ms", "30s")
}

func checkE2EState(t *testing.T, nmd *nmdstate.NMD, admPhase string) {
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
		// Check object state on NMD, CMD and ApiServer
		nmdObj, err := nmd.GetSmartNIC()
		AssertOk(t, err, "NIC not found in NMD nicDB")
		cmdObj, err := cmdenv.StateMgr.FindSmartNIC(nmdObj.GetObjectMeta().Tenant, nmdObj.GetObjectMeta().Name)
		AssertOk(t, err, "NIC not found in CMD local state")
		apiSrvObj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), nmdObj.GetObjectMeta())
		AssertOk(t, err, "NIC not found in ApiServer")

		for i, nicObj := range []*pencluster.SmartNIC{nmdObj, cmdObj.SmartNIC, apiSrvObj} {
			if nmdObj.Status.AdmissionPhase != admPhase {
				log.Errorf("NIC not in expected admission phase, index: %d, have: %s, want: %s", i, nicObj.Status.AdmissionPhase, admPhase)
				return false, nil
			}
		}

		// Check internal NMD state
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
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &rmock.ResolverClient{}})
	defer cancel()

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
		if cfg.Spec.Mode != proto.MgmtMode_NETWORK.String() {
			log.Errorf("Failed to switch to managed mode")
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f1, "Failed to verify mode is in Managed Mode", "500ms", "10s")

	// Validate SmartNIC object is created in ApiServer and CMD local state
	f2 := func() (bool, interface{}) {
		apiSrvObj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), &meta)
		if err != nil || apiSrvObj == nil {
			log.Errorf("Failed to GET SmartNIC object from ApiServer, mac:%s, %v", priMac, err)
			return false, nil
		}
		cmdObj, err := cmdenv.StateMgr.FindSmartNIC(meta.Tenant, meta.Name)
		if err != nil || cmdObj == nil {
			log.Errorf("Failed to GET SmartNIC object from CMD local state, mac:%s, %v", priMac, err)
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f2, "Failed to verify creation of SmartNIC object", "500ms", "30s")

	// check NIC is admitted in NMD
	checkE2EState(t, nmd, pencluster.SmartNICStatus_ADMITTED.String())

	// Turn off auto-admit
	clusterObj, err := tInfo.apiClient.ClusterV1().Cluster().Get(context.Background(), &api.ObjectMeta{Name: "testCluster"})
	AssertOk(t, err, "Error getting cluster object")
	clusterObj.Spec.AutoAdmitNICs = false
	_, err = tInfo.apiClient.ClusterV1().Cluster().Update(context.Background(), clusterObj)
	AssertOk(t, err, "Error updating cluster object")

	// The setNICAdmitState function sets the NIC state in ApiServer
	setNICAdmitState := func(admit bool) {
		nicObj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), &meta)
		AssertOk(t, err, "Error getting NIC from ApiServer")
		nicObj.Spec.Admit = admit
		nicObj, err = tInfo.apiClient.ClusterV1().SmartNIC().Update(context.Background(), nicObj)
		AssertOk(t, err, "Error updating NIC in ApiServer")
	}

	// Verify that Status info (except phase, host and conditions) does not change in apiServer as NIC gets admitted/de-admitted
	validateStatus := func(refObj *pencluster.SmartNIC) {
		obj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), &meta)
		AssertOk(t, err, "Error getting NIC from ApiServer")
		eq := runtime.FilterUpdate(obj.Status, refObj.Status, []string{"Conditions", "AdmissionPhase", "Host"}, nil)
		Assert(t, eq, "NIC immutable information should not change with admission status. Have:\n%+v, want:\n%+v", obj.Status, refObj.Status)
	}

	refObj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), &meta)
	AssertOk(t, err, "Error getting reference object")

	for i := 0; i < 10; i++ {
		// De-admit NIC. NMD is supposed to receive a notification and go back to "pending" state.
		setNICAdmitState(false)

		// Verify that NMD responds to de-admission (stop watchers, stop sending updates, restart registration loop)
		checkE2EState(t, nmd, pencluster.SmartNICStatus_PENDING.String())

		// Verify health updates are NOT received on CMD and the NIC is not marked as UNKNOWN
		time.Sleep(2 * nicDeadIntvl)
		checkHealthStatus(t, &meta, pencluster.SmartNICStatus_PENDING.String())

		// Verify that status information was preserved
		validateStatus(refObj)

		// Re-admit NIC. NMD should get admitted the next registration attempt
		setNICAdmitState(true)

		// Verify that NIC gets readmitted, NMD exits registration loop and starts sending updates again
		checkE2EState(t, nmd, pencluster.SmartNICStatus_ADMITTED.String())

		// Verify health updates are received on CMD
		checkHealthStatus(t, &meta, pencluster.SmartNICStatus_ADMITTED.String())

		// Verify that status information was preserved
		validateStatus(refObj)
	}
}

func Setup(m *testing.M) {

	// Init etcd cluster
	var t testing.T

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
	grpclog.SetLoggerV2(pl)

	// start certificate server and set up TLS provider
	// need to do this before Chdir() so that it finds the certificates on disk
	err := testutils.SetupIntegTLSProvider()
	if err != nil {
		log.Fatalf("Error setting up TLS provider: %v", err)
	}

	// cluster bind mounts in local directory. certain filesystems (like vboxsf, nfs) dont support unix binds.
	os.Chdir("/tmp")
	cluster := integration.NewClusterV3(&t)

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
	tInfo.regRPCServer, tInfo.updRPCServer, err = launchCMDServices(m, *cmdRegURL, *cmdUpdURL)
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
