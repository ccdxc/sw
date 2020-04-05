// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package nicadmission

import (
	"context"
	"crypto/tls"
	"flag"
	"fmt"
	"net"
	"net/http"
	"os"
	"path"
	"reflect"
	gorun "runtime"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/tokenauth/readutils"

	"github.com/pensando/netlink"

	"github.com/pensando/sw/nic/agent/nmd/state/ipif"

	"golang.org/x/net/trace"
	rpc "google.golang.org/grpc"
	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api"
	api_cache "github.com/pensando/sw/api/cache"
	apicache "github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	pencluster "github.com/pensando/sw/api/generated/cluster"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	"github.com/pensando/sw/api/generated/tokenauth"
	"github.com/pensando/sw/nic/agent/nmd"
	nmdstate "github.com/pensando/sw/nic/agent/nmd/state"
	nmdutils "github.com/pensando/sw/nic/agent/nmd/utils"
	proto "github.com/pensando/sw/nic/agent/protos/nmd"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	cmdapi "github.com/pensando/sw/venice/cmd/apiclient"
	"github.com/pensando/sw/venice/cmd/cache"
	cmdenv "github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	tokenauthsrv "github.com/pensando/sw/venice/cmd/grpc/server/tokenauth"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	cmdsvc "github.com/pensando/sw/venice/cmd/services"
	"github.com/pensando/sw/venice/cmd/services/mock"
	tokenauthsvc "github.com/pensando/sw/venice/cmd/services/tokenauth"
	cmdtypes "github.com/pensando/sw/venice/cmd/types"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certmgr"
	diagmock "github.com/pensando/sw/venice/utils/diagnostics/mock"
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
	smartNICRegURL  = "localhost:" + globals.CMDSmartNICRegistrationPort
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
	cmdAuthURL  = flag.String("cmd-upd-url", smartNICUpdURL, "CMD Updates URL")
	resolverURL = flag.String("resolver-url", resolverURLs, "Resolver URLs")
	mode        = flag.String("mode", "network", "Naples mode, host or network")
	rpcTrace    = flag.Bool("rpc-trace", false, "Enable gRPC tracing")

	logger = log.GetNewLogger(log.GetDefaultConfig("nic_admission_test"))

	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("nic_admission_test", logger))
)

type testInfo struct {
	l                log.Logger
	apiServerPort    string
	apiServer        apiserver.Server
	apiClient        apiclient.Services
	regRPCServer     *rpckit.RPCServer
	cmdAuthRPCServer *rpckit.RPCServer
	smartNICServer   *smartnic.RPCServer
	resolverServer   *rpckit.RPCServer
	stateMgr         *cache.Statemgr
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
			DSCs: []pencluster.DistributedServiceCardID{
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

func getProxyURL(index int) string {
	return fmt.Sprintf(":%d", 21000+index)
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
	cmdenv.SmartNICRegRPCServer = regRPCServer
	tInfo.stateMgr = cache.NewStatemgr(tInfo, func() cmdtypes.LeaderService { return cmdenv.LeaderService })
	cmdenv.StateMgr = tInfo.stateMgr

	// create and register the RPC handler for SmartNIC service
	tInfo.smartNICServer = smartnic.NewRPCServer(
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
	cmdAuthRPCServer, err := rpckit.NewRPCServer("smartNIC", updURL)
	if err != nil {
		fmt.Printf("Error creating NIC updates RPC-server: %v", err)
		return nil, nil, err
	}
	tInfo.cmdAuthRPCServer = cmdAuthRPCServer
	cmdenv.AuthRPCServer = cmdAuthRPCServer
	grpc.RegisterSmartNICUpdatesServer(cmdAuthRPCServer.GrpcServer, tInfo.smartNICServer)

	cmdenv.TokenAuthService = tokenauthsvc.NewTokenAuthService("testCluster", cmdenv.CertMgr.Ca())
	tokenAuthRPCHandler := tokenauthsrv.NewRPCHandler(cmdenv.TokenAuthService)
	tokenauth.RegisterTokenAuthV1Server(cmdenv.AuthRPCServer.GrpcServer, tokenAuthRPCHandler)

	cmdAuthRPCServer.Start()
	*cmdAuthURL = cmdAuthRPCServer.GetListenURL()

	// Start CMD config watcher
	cmdenv.Logger = tInfo.l
	cmdenv.QuorumNodes = []string{"localhost"}
	l := mock.NewLeaderService("testMaster")
	l.LeaderID = "testMaster"
	cmdenv.LeaderService = l
	s := cmdsvc.NewSystemdService(cmdsvc.WithSysIfSystemdSvcOption(&mock.SystemdIf{}))
	cw := cmdapi.NewCfgWatcherService(tInfo.l, "localhost:"+tInfo.apiServerPort)
	cmdenv.MasterService = cmdsvc.NewMasterService(
		"testNode",
		cmdsvc.WithLeaderSvcMasterOption(l),
		cmdsvc.WithSystemdSvcMasterOption(s),
		cmdsvc.WithConfigsMasterOption(&mock.Configs{}),
		cmdsvc.WithCfgWatcherMasterOption(cw),
		cmdsvc.WithDiagModuleUpdaterSvcOption(diagmock.GetModuleUpdater()),
		cmdsvc.WithClusterHealthMonitor(&mock.ClusterHealthMonitor{}))
	cw.Start()

	return regRPCServer, cmdAuthRPCServer, nil
}

// Create NMD and Agent
func createNMD(t *testing.T, dbPath, priMac, restURL, revProxyURL, mgmtMode string) (*nmdInfo, error) {

	venicetestutils.CreateFruJSON(priMac)

	// override location of trust roots for reverse proxy
	globals.VeniceTrustRootsFile = "/tmp/clusterTrustRoots.pem"

	// // create a platform agent
	// pa, err := platform.NewNaplesPlatformAgent()
	// if err != nil {
	// 	log.Fatalf("Error creating platform agent. Err: %v", err)
	// }
	// uc, err := upg.NewNaplesUpgradeClient(nil)
	// if err != nil {
	// 	log.Fatalf("Error creating Upgrade client . Err: %v", err)
	// }

	var resolverClient resolver.Interface
	if resolverURL != nil && *resolverURL != "" {
		resolverCfg := &resolver.Config{
			Name:    "TestNMD",
			Servers: strings.Split(*resolverURL, ","),
		}
		resolverClient = resolver.New(resolverCfg)
	}

	// create the new NMD
	ag, err := nmd.NewAgent(
		nil,
		dbPath,
		restURL,
		revProxyURL,
		nicRegIntvl,
		nicUpdIntvl,
	)
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

	// Fake IPConfig
	ipConfig := &pencluster.IPConfig{
		IPAddress: "1.2.3.4",
	}

	cfg := nmdHandle.GetNaplesConfig()
	cfg.Spec.Controllers = []string{"localhost"}
	cfg.Spec.NetworkMode = proto.NetworkMode_INBAND.String()
	cfg.Spec.IPConfig = ipConfig
	cfg.Spec.MgmtVlan = 0
	cfg.Spec.ID = priMac

	nmdHandle.SetNaplesConfig(cfg.Spec)
	nmdHandle.UpdateNaplesConfig(nmdHandle.GetNaplesConfig())

	err = createNaplesOOBInterface()

	// set the NMD reverse proxy config to point to corresponding REST port
	err = nmdutils.ClearVeniceTrustRoots()
	Assert(t, err == nil || os.IsNotExist(err), "Error removing stale trust roots: %v", err)
	proxyConfig := map[string]string{
		"/api/v1/naples": "http://127.0.0.1" + restURL,
	}
	os.MkdirAll(path.Dir(globals.NmdBackupDBPath), 0664)
	nmdHandle.StopReverseProxy()
	nmdHandle.UpdateReverseProxyConfig(proxyConfig)
	nmdHandle.StartReverseProxy()

	return ni, nil
}

// stopAgent stops NMD server and deletes emDB file
func stopNMD(t *testing.T, i *nmdInfo) {
	i.agent.Stop()
	i.resolverClient.Stop()
	err := os.Remove(i.dbPath)
	if err != nil {
		log.Fatalf("Error deleting emDB file: %s, err: %v", i.dbPath, err)
	}
	_ = deleteNaplesOOBInterfaces()
	nmdutils.ClearVeniceTrustRoots()
}

func createNaplesOOBInterface() error {
	oobMAC, _ := net.ParseMAC("42:42:42:42:42:42")
	dhcpClientMock := &netlink.Veth{
		LinkAttrs: netlink.LinkAttrs{
			Name:         ipif.NaplesOOBInterface,
			TxQLen:       1000,
			HardwareAddr: oobMAC,
		},
		PeerName: "dhcp-peer",
	}

	// Create the veth pair
	if err := netlink.LinkAdd(dhcpClientMock); err != nil {
		return err
	}
	if err := netlink.LinkSetARPOn(dhcpClientMock); err != nil {
		return err
	}

	return netlink.LinkSetUp(dhcpClientMock)
}

func deleteNaplesOOBInterfaces() error {
	oobIntf, err := netlink.LinkByName(ipif.NaplesOOBInterface)
	if err != nil {
		log.Errorf("TearDown Failed to look up the interfaces. Err: %v", err)
		return err
	}

	return netlink.LinkDel(oobIntf)
}

func TestCreateNMDs(t *testing.T) {
	// Init tsdb
	t.Skip("Skipped since this test needs to be re-written after auto-discovery")
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
				revProxyURL := getProxyURL(i)

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
					nmdInst, err := createNMD(t, dbPath, priMac, restURL, revProxyURL, *mode)
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

						var naplesCfg proto.DistributedServiceCard

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
						naplesCfg = proto.DistributedServiceCard{
							ObjectMeta: api.ObjectMeta{Name: "DistributedServiceCardConfig"},
							TypeMeta:   api.TypeMeta{Kind: "DistributedServiceCard"},
							Spec: proto.DistributedServiceCardSpec{
								Mode:        proto.MgmtMode_NETWORK.String(),
								NetworkMode: proto.NetworkMode_OOB.String(),
								Controllers: []string{"127.0.0.1"},
								ID:          priMac,
								IPConfig: &pencluster.IPConfig{
									IPAddress: "127.0.0.10/16",
								},
								PrimaryMAC: priMac,
								DSCProfile: globals.DefaultDSCProfile,
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
						if nic.Status.AdmissionPhase != pencluster.DistributedServiceCardStatus_ADMITTED.String() {
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

					nicMeta := &api.ObjectMeta{
						Name: priMac,
					}
					hostMeta := &host.ObjectMeta

					// Validate DistributedServiceCard object is created
					f5 := func() (bool, interface{}) {

						nicObj, err := tInfo.apiClient.ClusterV1().DistributedServiceCard().Get(context.Background(), nicMeta)
						if err != nil || nicObj == nil {
							log.Errorf("Failed to GET DistributedServiceCard object, mac:%s, %v", priMac, err)
							return false, nil
						}

						return true, nil
					}
					AssertEventually(t, f5, "Failed to verify creation of required DistributedServiceCard object", string("100ms"), string("30s"))

					// Validate Host object is created and paired
					checkHostNICPair(t, hostMeta, nicMeta, true)

					// delete host object
					tInfo.apiClient.ClusterV1().Host().Delete(context.Background(), hostMeta)
					f6 := func() (bool, interface{}) {
						nic, err := tInfo.apiClient.ClusterV1().DistributedServiceCard().Get(context.Background(), nicMeta)
						if err != nil {
							log.Errorf("Failed to GET DistributedServiceCard object:%s, %v", nicMeta.Name, err)
							return false, nil
						}
						if nic.Status.Host != "" {
							log.Errorf("Host not cleaned up from DistributedServiceCard object as expected: %+v", nic)
							return false, nil
						}
						return true, nil
					}
					AssertEventually(t, f6, "Failed to verify update of DistributedServiceCard object", string("100ms"), string("30s"))

					// Create again
					_, err = tInfo.apiClient.ClusterV1().Host().Create(context.Background(), host)
					AssertOk(t, err, fmt.Sprintf("Error creating host: %v", host.Name))
					checkHostNICPair(t, hostMeta, nicMeta, true)

					// Delete again
					tInfo.apiClient.ClusterV1().Host().Delete(context.Background(), hostMeta)
					AssertEventually(t, f6, "Failed to verify update of DistributedServiceCard object", string("100ms"), string("30s"))

					log.Infof("#### Completed TC: %s NodeID: %s DB: %s GoRoutines: %d CGoCalls: %d ",
						tcName, priMac, dbPath, gorun.NumGoroutine(), gorun.NumCgoCall())

				})
			}
		})
		log.Infof("#### Completed TestGroup")
	}
}

// getAgentRESTClient returns a http/https REST client for NAPLES agents
func getAgentRESTClient(t *testing.T, auth bool) (*netutils.HTTPClient, error) {
	client := netutils.NewHTTPClient()
	if auth {
		// proxy should be running HTTPs, client auth token required
		token, err := cmdenv.TokenAuthService.GenerateNodeToken([]string{"*"}, nil, nil)
		if err != nil {
			return nil, fmt.Errorf("Error generating node auth token: %s", token)
		}
		tlsCert, err := readutils.ParseNodeToken(token)
		if err != nil {
			return nil, fmt.Errorf("Error parsing node auth token")
		}
		client.WithTLSConfig(
			&tls.Config{
				InsecureSkipVerify: true, // do not check agent's certificate
				Certificates:       []tls.Certificate{tlsCert},
			})
	} else {
		client.WithTLSConfig(
			&tls.Config{
				InsecureSkipVerify: true, // do not check agent's certificate
			})
	}
	return client, nil
}

// setNICAdmitState sets the value for the DistributedServiceCard.Spec.Admit field in ApiServer
func setNICAdmitState(t *testing.T, meta *api.ObjectMeta, admit bool) {
	nicObj, err := tInfo.apiClient.ClusterV1().DistributedServiceCard().Get(context.Background(), meta)
	AssertOk(t, err, "Error getting NIC from ApiServer")
	nicObj.Spec.Admit = admit
	nicObj, err = tInfo.apiClient.ClusterV1().DistributedServiceCard().Update(context.Background(), nicObj)
	AssertOk(t, err, "Error updating NIC in ApiServer")
}

// setNICMgmtMode sets the value for the DistributedServiceCard.Spec.MgmtMode field in ApiServer
func setNICMgmtMode(t *testing.T, meta *api.ObjectMeta, mode string) {
	nicObj, err := tInfo.apiClient.ClusterV1().DistributedServiceCard().Get(context.Background(), meta)
	AssertOk(t, err, "Error getting NIC from ApiServer")
	nicObj.Spec.MgmtMode = mode
	nicObj, err = tInfo.apiClient.ClusterV1().DistributedServiceCard().Update(context.Background(), nicObj)
	AssertOk(t, err, "Error updating NIC in ApiServer")
}

func setHostSmartNICIDs(t *testing.T, meta *api.ObjectMeta, ids []pencluster.DistributedServiceCardID) {
	hostObj, err := tInfo.apiClient.ClusterV1().Host().Get(context.Background(), meta)
	AssertOk(t, err, "Error getting host object")
	hostObj.Spec.DSCs = ids
	_, err = tInfo.apiClient.ClusterV1().Host().Update(context.Background(), hostObj)
	AssertOk(t, err, "Error updating Host %+v in ApiServer", hostObj)
}

// setNAPLESConfigMode sets the value for the NaplesConfig.Spec.Mgmt in NAPLES
func setNAPLESConfigMode(t *testing.T, nmdURL, mode string) {
	var err error
	f := func() (bool, interface{}) {
		var naplesCfg proto.DistributedServiceCard
		err = netutils.HTTPGet(nmdURL+"/", &naplesCfg)
		if err != nil {
			log.Errorf("Failed to get naples config via REST, err:%+v", err)
			return false, nil
		}
		naplesCfg.Spec.Mode = mode
		var resp nmdstate.NaplesConfigResp
		err = netutils.HTTPPost(nmdURL, &naplesCfg, &resp)
		if err != nil {
			log.Errorf("Failed to post naples config, err:%+v resp:%+v", err, resp)
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f, fmt.Sprintf("Failed to set mode %s on NAPLES %s", mode, nmdURL), "50ms", "20s")
}

// checkNAPLESConfigMode checks the value for the NaplesConfig.Spec.Mgmt in NAPLES
func checkNAPLESConfigMode(t *testing.T, nmdURL, mode string) {
	var err error
	f := func() (bool, interface{}) {
		var naplesCfg proto.DistributedServiceCard
		err = netutils.HTTPGet(nmdURL+"/", &naplesCfg)
		if err != nil {
			log.Errorf("Failed to get naples config via REST, err:%+v", err)
			return false, nil
		}
		if naplesCfg.Spec.Mode != mode {
			log.Errorf("Unexpected MgmtMode in NAPLES %s. Have: %s, want: %s", nmdURL, naplesCfg.Spec.Mode, mode)
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f, fmt.Sprintf("Failed to check mgmt mode config %s on NAPLES %s", mode, nmdURL), "50ms", "20s")
}

func checkHealthStatus(t *testing.T, meta *api.ObjectMeta, admPhase string) {
	f := func() (bool, interface{}) {
		apiSrvObj, err := tInfo.apiClient.ClusterV1().DistributedServiceCard().Get(context.Background(), meta)
		AssertOk(t, err, "Failed to retrieve NIC from ApiServer")

		cmdObj, err := cmdenv.StateMgr.FindSmartNIC(meta.Name)
		AssertOk(t, err, "Failed to retrieve NIC from local cache")

		for index, nicObj := range []*pencluster.DistributedServiceCard{apiSrvObj, cmdObj.DistributedServiceCard} {
			if nicObj.Status.AdmissionPhase != admPhase {
				log.Errorf("NIC not in expected admission phase. Have: %s, want: %s, index: %d", nicObj.Status.AdmissionPhase, admPhase, index)
				return false, nil
			}
			if admPhase == pencluster.DistributedServiceCardStatus_ADMITTED.String() {
				if len(nicObj.Status.Conditions) == 0 {
					log.Errorf("NIC Object does not have any condition, nic:%s, %v, index: %d", meta.Name, nicObj.Status, index)
					return false, nil
				}
				hc := nicObj.Status.Conditions[0]
				if hc.Type != pencluster.DSCCondition_HEALTHY.String() || hc.Status != pencluster.ConditionStatus_TRUE.String() || hc.LastTransitionTime == "" {
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

// checkReverseProxy checks that the reverse proxy hosted by NMD correctly switches
// between HTTP (unauthenticated), when NAPLES is not part of a cluster and
// HTTPs with client certificate, when it is
func checkReverseProxy(t *testing.T, nmd *nmdstate.NMD, admPhase string) error {
	revProxyURL := nmd.GetReverseProxyListenURL()
	if revProxyURL == "" {
		return fmt.Errorf("nmd.GetReverseProxyListenURL returned empty URL")
	}
	auth := (admPhase == pencluster.DistributedServiceCardStatus_ADMITTED.String())
	client, err := getAgentRESTClient(t, auth)
	if err != nil {
		return fmt.Errorf("Error getting REST client: %v", err)
	}
	client.DisableKeepAlives()
	defer client.CloseIdleConnections()
	url := "https://" + revProxyURL + "/api/v1/naples/"

	var naplesCfg proto.DistributedServiceCard
	resp, err := client.Req("GET", url, nil, &naplesCfg)
	if err != nil || resp != http.StatusOK {
		return fmt.Errorf("Failed to get naples config via reverse proxy. URL: %s, AdmPhase: %v, Resp: %+v, err: %v", url, admPhase, resp, err)
	}
	if !reflect.DeepEqual(naplesCfg, nmd.GetNaplesConfig()) {
		return fmt.Errorf("Unexpected NAPLES config response. Have: %+v, want: %+v", naplesCfg, nmd.GetNaplesConfig())
	}
	if auth {
		// make a call without client cert and check that it is rejected
		client, err := getAgentRESTClient(t, auth)
		if err != nil {
			return fmt.Errorf("Error getting REST client: %v", err)
		}
		tlsConfig := &tls.Config{InsecureSkipVerify: true}
		client.WithTLSConfig(tlsConfig)
		client.DisableKeepAlives()
		defer client.CloseIdleConnections()
		resp, err := client.Req("GET", url, nil, &naplesCfg)
		if err == nil || resp == http.StatusOK {
			return fmt.Errorf("Proxy request did not fail without valid certificate, err: %v, resp: %v", err, resp)
		}
	}
	return nil
}

func checkE2EState(t *testing.T, nmd *nmdstate.NMD, admPhase string) {
	var expRegStatus, expUpdStatus, expUpdWatchStatus, expRoWatchStatus bool

	switch admPhase {
	case pencluster.DistributedServiceCardStatus_ADMITTED.String():
		expRegStatus, expUpdStatus, expUpdWatchStatus, expRoWatchStatus = false, true, true, true
	case pencluster.DistributedServiceCardStatus_PENDING.String():
		expRegStatus, expUpdStatus, expUpdWatchStatus, expRoWatchStatus = true, false, false, false
	case pencluster.DistributedServiceCardStatus_DECOMMISSIONED.String():
		expRegStatus, expUpdStatus, expUpdWatchStatus, expRoWatchStatus = false, false, false, false
	case pencluster.DistributedServiceCardStatus_REGISTERING.String():
		expRegStatus, expUpdStatus, expUpdWatchStatus, expRoWatchStatus = true, false, false, false
	default:
		panic(fmt.Sprintf("Unexpected admission phase %s, NMD: %+v", admPhase, nmd))
	}

	f := func() (bool, interface{}) {
		// Check object state on NMD, CMD and ApiServer
		nmdObj, err := nmd.GetSmartNIC()
		name := nmdObj.GetObjectMeta().Name
		if err != nil {
			return false, fmt.Errorf("NIC %s not found in NMD nicDB, phase: %s", name, admPhase)
		}
		cmdObj, err := cmdenv.StateMgr.FindSmartNIC(name)
		if err != nil {
			return false, fmt.Errorf("NIC %s not found in CMD local state, phase: %s", name, admPhase)
		}
		apiSrvObj, err := tInfo.apiClient.ClusterV1().DistributedServiceCard().Get(context.Background(), nmdObj.GetObjectMeta())
		if err != nil {
			return false, fmt.Sprintf("NIC %s not found in ApiServer, phase: %s", name, admPhase)
		}

		// Phase on CMD must match phase on NMD, except for the "registering case", where Venice has admitted but
		// NAPLES refused to join and stays in REGISTERING
		if admPhase != pencluster.DistributedServiceCardStatus_REGISTERING.String() {
			for i, nicObj := range []*pencluster.DistributedServiceCard{nmdObj, cmdObj.DistributedServiceCard, apiSrvObj} {
				if nicObj.Status.AdmissionPhase != admPhase {
					log.Errorf("NIC not in expected admission phase, index: %d, have: %s, want: %s", i, nicObj.Status.AdmissionPhase, admPhase)
					return false, nil
				}
			}
		} else {
			if nmdObj.Status.AdmissionPhase != admPhase {
				log.Errorf("NIC not in expected admission phase, have: %s, want: %s", nmdObj.Status.AdmissionPhase, admPhase)
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

		if err = checkReverseProxy(t, nmd, admPhase); err != nil {
			log.Errorf("Reverse proxy check failed, err: %v", err)
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f, fmt.Sprintf("NMD did not reach expected state for phase %s, status: %+v", admPhase, nmd), "500ms", "30s")
}

func setAutoAdmit(v bool) error {
	clusterObj, err := tInfo.apiClient.ClusterV1().Cluster().Get(context.Background(), &api.ObjectMeta{Name: "testCluster"})
	if err != nil {
		return fmt.Errorf("Error getting cluster object: %v", err)
	}
	clusterObj.Spec.AutoAdmitDSCs = v
	_, err = tInfo.apiClient.ClusterV1().Cluster().Update(context.Background(), clusterObj)
	if err != nil {
		return fmt.Errorf("Error updating cluster object: %v", err)
	}
	return nil
}

func checkHostNICPair(t *testing.T, hostMeta, nicMeta *api.ObjectMeta, established bool) {
	var host *pencluster.Host
	var nic *pencluster.DistributedServiceCard
	var err error

	f := func() (bool, interface{}) {
		host, err = tInfo.apiClient.ClusterV1().Host().Get(context.Background(), hostMeta)
		if err != nil {
			log.Errorf("Failed to GET Host object:%s, %v", hostMeta.Name, err)
			return false, nil
		}
		nic, err = tInfo.apiClient.ClusterV1().DistributedServiceCard().Get(context.Background(), nicMeta)
		if err != nil {
			log.Errorf("Failed to GET DistributedServiceCard object:%s, %v", nicMeta.Name, err)
			return false, nil
		}
		nicInHostStatus := false
		for ii := range host.Status.AdmittedDSCs {
			if host.Status.AdmittedDSCs[ii] == nic.Name {
				nicInHostStatus = true
				break
			}
		}
		if established && (!nicInHostStatus || nic.Status.Host != host.Name) {
			log.Errorf("NIC and Host object pairing not complete. Host: %+v, NIC: %+v", host, nic)
			return false, nil
		}
		if !established && (nicInHostStatus || nic.Status.Host != "") {
			log.Errorf("NIC and Host object pairing not cleaned up. Host: %+v, NIC: %+v", host, nic)
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f, fmt.Sprintf("Failed to validate Host-NIC pairing. Host: %+v, DistributedServiceCard: %+v, err: %+v", host, nic, err))
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
	revProxyURL := getProxyURL(i)

	meta := api.ObjectMeta{
		Name: priMac,
	}

	// Create Host
	_, err := tInfo.apiClient.ClusterV1().Host().Create(context.Background(), host)
	AssertOk(t, err, fmt.Sprintf("Error creating host: %v", host.Name))
	defer tInfo.apiClient.ClusterV1().Host().Delete(context.Background(), &host.ObjectMeta)

	// Cleanup any prior DB files
	os.Remove(dbPath)

	// create Agent and NMD
	nmdInst, err := createNMD(t, dbPath, priMac, restURL, revProxyURL, "network")
	defer stopNMD(t, nmdInst)
	Assert(t, (err == nil && nmdInst.agent != nil), "Failed to create agent", err)

	nmd := nmdInst.agent.GetNMD()

	// Switch to Managed mode
	naplesCfg := proto.DistributedServiceCard{
		ObjectMeta: api.ObjectMeta{Name: "DistributedServiceCardConfig"},
		TypeMeta:   api.TypeMeta{Kind: "DistributedServiceCard"},
		Spec: proto.DistributedServiceCardSpec{
			Mode:        proto.MgmtMode_NETWORK.String(),
			NetworkMode: proto.NetworkMode_OOB.String(),
			Controllers: []string{"127.0.0.1"},
			ID:          priMac,
			IPConfig: &pencluster.IPConfig{
				IPAddress: "127.0.0.10/16",
			},
			PrimaryMAC: priMac,
			DSCProfile: globals.DefaultDSCProfile,
		},
	}

	log.Infof("Naples config: %+v", naplesCfg)

	var resp nmdstate.NaplesConfigResp

	// Post the mode change config
	f3 := func() (bool, interface{}) {
		err = netutils.HTTPPost(nmd.GetNMDUrl(), &naplesCfg, &resp)
		if err != nil {
			log.Errorf("Failed to post naples config, err:%+v resp:%+v", err, resp)
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f3, "Failed to post the naples config")

	// Validate Managed Mode
	checkNAPLESConfigMode(t, nmd.GetNMDUrl(), proto.MgmtMode_NETWORK.String())

	time.Sleep(time.Second * 5)

	// Validate SmartNIC object is created in ApiServer and CMD local state
	fmt.Println("Validating for the initial admit")
	checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_ADMITTED.String())

	// Turn off auto-admit
	err = setAutoAdmit(false)
	AssertOk(t, err, "Failed to set autoAdmit to false")

	//Verify that Status info (except phase, host and conditions) does not change in apiServer as NIC gets admitted/de-admitted
	validateStatus := func(refObj *pencluster.DistributedServiceCard) {
		obj, err := tInfo.apiClient.ClusterV1().DistributedServiceCard().Get(context.Background(), &meta)
		AssertOk(t, err, "Error getting NIC from ApiServer")
		eq := runtime.FilterUpdate(obj.Status, refObj.Status, []string{"Conditions", "AdmissionPhase", "Host"}, nil)
		Assert(t, eq, "NIC immutable information should not change with admission status. Have:\n%+v, want:\n%+v", obj.Status, refObj.Status)
	}

	refObj, err := tInfo.apiClient.ClusterV1().DistributedServiceCard().Get(context.Background(), &meta)
	AssertOk(t, err, "Error getting reference object")
	fmt.Println("RefObj: ", refObj)

	for i := 0; i < 6; i++ {
		// De-admit NIC. NMD is supposed to receive a notification and go back to "pending" state.
		fmt.Println("Setting NIC Admit State as false")
		setNICAdmitState(t, &meta, false)

		// Verify that NMD responds to de-admission (stop watchers, stop sending updates, restart registration loop)
		fmt.Println("Verifying if the smartnic is in pending state")
		checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_PENDING.String())

		// Verify health updates are NOT received on CMD and the NIC is not marked as UNKNOWN
		time.Sleep(2 * nicDeadIntvl)
		fmt.Println("Verifying if the smartnic health is in pending state")
		checkHealthStatus(t, &meta, pencluster.DistributedServiceCardStatus_PENDING.String())

		// Verify that status information was preserved
		validateStatus(refObj)

		// Re-admit NIC. NMD should get admitted the next registration attempt
		fmt.Println("Setting NIC Admit State as true")
		setNICAdmitState(t, &meta, true)

		// Verify that NIC gets readmitted, NMD exits registration loop and starts sending updates again
		fmt.Println("Verifying if the smartnic is in admitted state")
		checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_ADMITTED.String())

		// Verify health updates are received on CMD
		checkHealthStatus(t, &meta, pencluster.DistributedServiceCardStatus_ADMITTED.String())

		// Verify that status information was preserved
		validateStatus(refObj)
	}

	// When the NIC gets admitted to a Venice cluster for the first time, it is supposed to form a persistent bond.
	// If later on the NIC performs a new admission sequence (for example after a reboot) and finds a different Venice
	// (as identified by the Venice CA trust chain) it must refuse to join.
	setNICAdmitState(t, &meta, false)
	checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_PENDING.String())

	err = nmdutils.StoreVeniceTrustRoots(cmdenv.CertMgr.Ca().TrustRoots())
	AssertOk(t, err, "Error updating trust roots file")

	origCertMgr := cmdenv.CertMgr
	tmpCertMgr, err := certmgr.NewTestCertificateMgr("readmit-test")
	AssertOk(t, err, "Failed to create tmp CertificateMgr")
	cmdenv.CertMgr = tmpCertMgr
	defer tmpCertMgr.Close()

	setNICAdmitState(t, &meta, true)
	checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_REGISTERING.String())

	// Make sure that proper error message is present both in object and config
	nicObj, err := nmd.GetSmartNIC()
	AssertOk(t, err, "Error getting NIC obj from NMD")
	// cluster identity is checked twice during the admission sequence:
	// - at the beginning, when CMD sends authentication request
	// - at the end, after CMD has signed the CSR
	// Regardless of which check fails, we expect the same error
	Assert(t, strings.Contains(nicObj.Status.AdmissionPhaseReason, "Cluster trust chain failed validation"),
		"Did not find expected admission phase reason. Have: %v", nicObj.Status.AdmissionPhaseReason)
	naplesConf := nmd.GetNaplesConfig()
	Assert(t, strings.Contains(naplesConf.Status.AdmissionPhaseReason, "Cluster trust chain failed validation"),
		"Did not find expected admission phase reason. Have: %v", naplesConf.Status.AdmissionPhaseReason)

	// Now switch back to the original CA. This time it should go through.
	cmdenv.CertMgr = origCertMgr
	checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_ADMITTED.String())
}

// TestNICDecommissionFlow tests the sequence in which a NIC is first admitted in the cluster, then decommissioned
// from Venice (switched to host-managed mode) and then re-commissioned (switched to network-managed mode again)
func TestNICDecommissionFlow(t *testing.T) {
	t.Skip("Disabled temporarily. This test needs changes in how we move to network mode from decomissioned.")
	ctx, cancel := context.WithCancel(context.Background())
	tsdb.Init(ctx, &tsdb.Opts{ClientName: t.Name(), ResolverClient: &rmock.ResolverClient{}})
	defer cancel()

	i := 11
	priMac := getSmartNICMAC(i)
	dbPath := getDBPath(i)
	restURL := getRESTUrl(i)
	revProxyURL := getProxyURL(i)

	meta := api.ObjectMeta{
		Name: priMac,
	}

	// Turn off auto-admit
	err := setAutoAdmit(false)
	AssertOk(t, err, "Failed to set autoAdmit to false")

	// Cleanup any prior DB files
	os.Remove(dbPath)

	// create Agent and NMD
	nmdInst, err := createNMD(t, dbPath, priMac, restURL, revProxyURL, "network")
	defer stopNMD(t, nmdInst)
	Assert(t, (err == nil && nmdInst.agent != nil), "Failed to create agent", err)

	nmd := nmdInst.agent.GetNMD()
	nmdURL := nmd.GetNMDUrl()

	// Switch to Managed mode
	naplesCfg := proto.DistributedServiceCard{
		ObjectMeta: api.ObjectMeta{Name: "DistributedServiceCardConfig"},
		TypeMeta:   api.TypeMeta{Kind: "DistributedServiceCard"},
		Spec: proto.DistributedServiceCardSpec{
			Mode:        proto.MgmtMode_NETWORK.String(),
			NetworkMode: proto.NetworkMode_OOB.String(),
			Controllers: []string{"127.0.0.1"},
			ID:          priMac,
			IPConfig: &pencluster.IPConfig{
				IPAddress: "127.0.0.10/16",
			},
			PrimaryMAC: priMac,
			DSCProfile: globals.DefaultDSCProfile,
		},
	}

	log.Infof("Naples config: %+v", naplesCfg)

	var resp nmdstate.NaplesConfigResp

	// Post the mode change config
	f3 := func() (bool, interface{}) {
		err = netutils.HTTPPost(nmd.GetNMDUrl(), &naplesCfg, &resp)
		if err != nil {
			log.Errorf("Failed to post naples config, err:%+v resp:%+v", err, resp)
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f3, "Failed to post the naples config")

	// Validate network-managed mode
	checkNAPLESConfigMode(t, nmdURL, proto.MgmtMode_NETWORK.String())
	time.Sleep(time.Second * 5)
	// Validate SmartNIC object is created in ApiServer and CMD local state
	checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_PENDING.String())

	for i := 0; i < 6; i++ {
		setNICAdmitState(t, &meta, true)
		// check NIC is admitted in NMD
		checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_ADMITTED.String())

		// check that NIC cannot be deleted in this state
		_, err = tInfo.apiClient.ClusterV1().DistributedServiceCard().Delete(context.Background(), &meta)
		Assert(t, err != nil, "Did not get expected error deleting DistributedServiceCard object in ADMITTED phase")

		// Switch to host-managed mode
		setNICMgmtMode(t, &meta, pencluster.DistributedServiceCardSpec_HOST.String())
		checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_DECOMMISSIONED.String())
		checkNAPLESConfigMode(t, nmdURL, proto.MgmtMode_HOST.String())

		// Switch to network-managed mode
		setNAPLESConfigMode(t, nmdURL, proto.MgmtMode_NETWORK.String())
		// We should go to ADMITTED, because the DistributedServiceCard.Spec.Admit = true
		checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_ADMITTED.String())
		checkNAPLESConfigMode(t, nmdURL, proto.MgmtMode_NETWORK.String())

		// Decommission again
		setNICMgmtMode(t, &meta, pencluster.DistributedServiceCardSpec_HOST.String())
		checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_DECOMMISSIONED.String())
		checkNAPLESConfigMode(t, nmdURL, proto.MgmtMode_HOST.String())

		_, err := tInfo.apiClient.DiagnosticsV1().Module().Get(context.TODO(), &api.ObjectMeta{Name: fmt.Sprintf("%s-%s", meta.Name, globals.Netagent)})
		AssertOk(t, err, "Error retrieving module object")
		// Delete DistributedServiceCard object
		_, err = tInfo.apiClient.ClusterV1().DistributedServiceCard().Delete(context.Background(), &meta)
		AssertOk(t, err, "Error deleting DistributedServiceCard object")
		_, err = tInfo.apiClient.DiagnosticsV1().Module().Get(context.TODO(), &api.ObjectMeta{Name: fmt.Sprintf("%s-%s", meta.Name, globals.Netagent)})
		Assert(t, err != nil, "expected module object to be deleted")
		// Switch again to network-managed mode
		setNAPLESConfigMode(t, nmdURL, proto.MgmtMode_NETWORK.String())
		// We should go to PENDING, because the Cluster.Spec.AutoAdmitDSCs = false
		checkE2EState(t, nmd, pencluster.DistributedServiceCardStatus_PENDING.String())
		checkNAPLESConfigMode(t, nmdURL, proto.MgmtMode_NETWORK.String())

		// Try to swtch to host-managed mode. It should fail because NIC is not admitted
		n, err := tInfo.apiClient.ClusterV1().DistributedServiceCard().Get(context.Background(), &meta)
		AssertOk(t, err, "Error getting NIC from ApiServer")
		n.Spec.MgmtMode = pencluster.DistributedServiceCardSpec_HOST.String()
		n, err = tInfo.apiClient.ClusterV1().DistributedServiceCard().Update(context.Background(), n)
		Assert(t, err != nil, "Did not get expected error changing mode form DistributedServiceCard object in PENDING phase")
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

	// We need a fairly high limit because all clients are collapsed into a single process
	// so they hit the same rate limiter
	rpckit.SetDefaultListenerConnectionRateLimit(50)

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
		GetOverlay:       api_cache.GetOverlay,
		IsDryRun:         api_cache.IsDryRun,
		AllowMultiTenant: true,
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
	tInfo.regRPCServer, tInfo.cmdAuthRPCServer, err = launchCMDServices(m, *cmdRegURL, *cmdAuthURL)
	if err != nil {
		panic(fmt.Sprintf("Err creating rpc server: %v", err))
	}

	// Check if no cluster exists to start with - negative test
	_, err = tInfo.stateMgr.GetCluster()
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
			AutoAdmitDSCs: true,
		},
	}
	_, err = tInfo.apiClient.ClusterV1().Cluster().Create(context.Background(), clRef)
	if err != nil {
		fmt.Printf("Error creating Cluster object, %v", err)
		os.Exit(-1)
	}

	defaultProfile := &pencluster.DSCProfile{
		ObjectMeta: api.ObjectMeta{
			Name: globals.DefaultDSCProfile,
		},
		Spec: pencluster.DSCProfileSpec{
			FwdMode:        "TRANSPARENT",
			FlowPolicyMode: "BASENET",
		},
	}

	_, err = tInfo.apiClient.ClusterV1().DSCProfile().Create(context.Background(), defaultProfile)
	if err != nil {
		fmt.Printf("Error creating default DSCProfile object, %v", err)
		log.Fatalf("Failed to create default DSCProfile")
	}

	log.Infof("#### ApiServer and CMD smartnic server is UP")
}

func Teardown(m *testing.M) {

	// stop the CMD smartnic RPC server
	tInfo.regRPCServer.Stop()

	// stop the CMD smartnic RPC server
	tInfo.cmdAuthRPCServer.Stop()

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

	nmdutils.ClearVeniceTrustRoots()

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

	// tune timeouts to speed-up tests
	cache.SetAPIServerRPCTimeout(3 * time.Second)

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
