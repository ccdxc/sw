// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package smartnic_test

import (
	"context"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"fmt"
	mathrand "math/rand"
	"os"
	"strings"
	"testing"
	"time"

	"google.golang.org/grpc/grpclog"

	api "github.com/pensando/sw/api"
	api_cache "github.com/pensando/sw/api/cache"
	apicache "github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	cmd "github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	nmd "github.com/pensando/sw/nic/agent/nmd"
	"github.com/pensando/sw/nic/agent/nmd/platform"
	proto "github.com/pensando/sw/nic/agent/nmd/protos"
	"github.com/pensando/sw/nic/agent/nmd/upg"
	apiserver "github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	cmdapi "github.com/pensando/sw/venice/cmd/apiclient"
	"github.com/pensando/sw/venice/cmd/cache"
	cmdenv "github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	. "github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	cmdsvc "github.com/pensando/sw/venice/cmd/services"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/events/recorder"
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
	smartNICServerURL = "localhost:9199"
	resolverURLs      = ":" + globals.CMDResolverPort
	healthInterval    = 1 * time.Second
	deadtimeInterval  = 3 * time.Second
)

var (
	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "smartnic_test"},
		EvtTypes:      append(cmd.GetEventTypes(), evtsapi.GetEventTypes()...),
		BackupDir:     "/tmp",
		SkipEvtsProxy: true})
)

type testInfo struct {
	l              log.Logger
	apiServerAddr  string
	apiServer      apiserver.Server
	apiClient      apiclient.Services
	rpcServer      *rpckit.RPCServer
	rpcClient      *rpckit.RPCClient
	smartNICServer *RPCServer
}

func (t testInfo) APIClient() cmd.ClusterV1Interface {
	return t.apiClient.ClusterV1()
}

var tInfo testInfo

// returns true if it is a well-formed error response from the server
func isErrorResponse(r *grpc.RegisterNICResponse) bool {
	if r == nil || r.AdmissionResponse == nil {
		return false
	}
	phase := r.AdmissionResponse.Phase
	return phase == cmd.SmartNICStatus_REJECTED.String() || phase == cmd.SmartNICStatus_UNKNOWN.String()
}

// runRPCServer creates a smartNIC server for SmartNIC service.
func createRPCServer(url, certFile, keyFile, caFile string) (*rpckit.RPCServer, error) {
	var err error

	// set cmd logger, statemgr, certmgr & quorum nodes
	cmdenv.Logger = tInfo.l
	cmdenv.QuorumNodes = []string{"localhost"}
	cmdenv.StateMgr = cache.NewStatemgr()
	cmdenv.CertMgr, err = certmgr.NewTestCertificateMgr("smartnic-test")
	if err != nil {
		return nil, fmt.Errorf("Error creating CertMgr instance: %v", err)
	}

	// Start CMD config watcher
	l := mock.NewLeaderService("testMaster")
	s := cmdsvc.NewSystemdService(cmdsvc.WithSysIfSystemdSvcOption(&mock.SystemdIf{}))
	cw := cmdapi.NewCfgWatcherService(tInfo.l, tInfo.apiServerAddr, cmdenv.StateMgr)
	cmdenv.MasterService = cmdsvc.NewMasterService(
		cmdsvc.WithLeaderSvcMasterOption(l),
		cmdsvc.WithSystemdSvcMasterOption(s),
		cmdsvc.WithConfigsMasterOption(&mock.Configs{}),
		cmdsvc.WithCfgWatcherMasterOption(cw))
	cw.Start()

	// create an RPC server.
	rpcServer, err := rpckit.NewRPCServer("smartNIC", url)
	if err != nil {
		fmt.Printf("Error creating RPC-server: %v", err)
		return nil, err
	}
	tInfo.rpcServer = rpcServer
	cmdenv.UnauthRPCServer = rpcServer

	// create and register the RPC handler for SmartNIC service
	tInfo.smartNICServer, err = NewRPCServer(tInfo, healthInterval, deadtimeInterval, getRESTPort(1), cmdenv.StateMgr)
	if err != nil {
		fmt.Printf("Error creating SmartNIC RPC server: %v", err)
		return nil, err
	}

	// Register self as rpc handler for both NIC registration and watch/updates on the server.
	// In reality, CMD uses two different servers because watches and updates APIs are
	// exposed over TLS, whereas NIC registration is not.
	grpc.RegisterSmartNICRegistrationServer(rpcServer.GrpcServer, tInfo.smartNICServer)
	grpc.RegisterSmartNICUpdatesServer(rpcServer.GrpcServer, tInfo.smartNICServer)
	rpcServer.Start()
	cmdenv.NICService = tInfo.smartNICServer

	// Launch go routine to monitor health updates of smartNIC objects and update status
	go func() {
		tInfo.smartNICServer.MonitorHealth()
	}()

	return rpcServer, nil
}

// createRPCServerClient creates rpc client and server for SmartNIC service
func createRPCServerClient() (*rpckit.RPCServer, *rpckit.RPCClient) {

	// start the rpc server
	rpcServer, err := createRPCServer(smartNICServerURL, "", "", "")
	if err != nil {
		fmt.Printf("Error connecting to grpc server. Err: %v", err)
		return nil, nil
	}

	// create a grpc client
	rpcClient, err := rpckit.NewRPCClient("smartNIC", smartNICServerURL)
	if err != nil {
		fmt.Printf("Error connecting to grpc server. Err: %v", err)
		rpcServer.Stop()
		return nil, nil
	}

	return rpcServer, rpcClient
}

// Create NMD and Agent
func createNMD(t *testing.T, dbPath, hostID, restURL string) (*nmd.Agent, error) {

	// create a platform agent
	pa, err := platform.NewNaplesPlatformAgent()
	if err != nil {
		log.Fatalf("Error creating platform agent. Err: %v", err)
	}

	uc, err := upg.NewNaplesUpgradeClient(nil)
	if err != nil {
		log.Fatalf("Error creating Upgrade client . Err: %v", err)
	}

	r := resolver.New(&resolver.Config{Name: t.Name(), Servers: strings.Split(resolverURLs, ",")})
	// create the new NMD
	ag, err := nmd.NewAgent(pa,
		uc,
		dbPath,
		hostID,
		hostID,
		smartNICServerURL,
		smartNICServerURL,
		restURL,
		"", // no local certs endpoint
		"", // no remote certs endpoint
		"host",
		globals.NicRegIntvl*time.Second,
		globals.NicUpdIntvl*time.Second,
		r,
		nil)
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

func getHostID(index int) string {
	return fmt.Sprintf("44.44.44.44.%02x.%02x", index/256, index%256)
}

func getRESTPort(index int) string {
	return fmt.Sprintf("%d", 15000+index)
}

func getRESTUrl(index int) string {
	return fmt.Sprintf("localhost:%s", getRESTPort(index))
}

func getDBPath(index int) string {
	return fmt.Sprintf("/tmp/nmd-%d.db", index)
}

func verifySmartNICObj(t *testing.T, name string, exists bool) {
	ometa := api.ObjectMeta{Name: name}
	f1 := func() (bool, interface{}) {
		nicObj, err := tInfo.smartNICServer.GetSmartNIC(ometa)
		if exists {
			if err != nil {
				t.Errorf("Error getting NIC object: %s", name)
				return false, nil
			}
			if nicObj.ObjectMeta.Name != name {
				t.Errorf("Got incorrect smartNIC object, expected: %s obtained: %s",
					nicObj.ObjectMeta.Name, name)
				return false, nil
			}
			return true, nil
		} else {
			if err != nil || nicObj == nil {
				return true, nil
			}
			t.Errorf("SmartNIC object should not exist for name:%s err:%v", name, err)
			return false, nil
		}
	}
	AssertEventually(t, f1, fmt.Sprintf("Failed to verify presence of smartNIC object, name: %v, expected: %v", name, exists))
}

// doPhase1Exchange performs the first phase of the admission sequence.
// It forms the admission request, send it to the server and receive
// the response with the challenge.
// It returns the original request that was send (containing the NIC object) and
// if the request is accepted by the server, it returns the challenge response to be sent in Phase 2,
// otherwise it returns the error message sent by the server
func doPhase1Exchange(t *testing.T, stream grpc.SmartNICRegistration_RegisterNICClient, nicName, hostName string, validCert, expectChallenge bool) (*grpc.NICAdmissionRequest, *grpc.AuthenticationResponse, *grpc.NICAdmissionResponse) {
	// create a platform agent and NMD instance to access factory key/cert
	pa, err := platform.NewNaplesPlatformAgent()
	AssertOk(t, err, "Error creating platform agent. Err: %v", err)

	// Construct SmartNIC object with valid cert/signature
	ometa := api.ObjectMeta{Name: nicName}
	nic := cmd.SmartNIC{
		TypeMeta:   api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: ometa,
		Spec: cmd.SmartNICSpec{
			Hostname: hostName,
			IPConfig: &cmd.IPConfig{
				IPAddress: "0.0.0.0/0",
			},
			MgmtMode: cmd.SmartNICSpec_NETWORK.String(),
		},
		Status: cmd.SmartNICStatus{
			AdmissionPhase: "UNKNOWN",
			SerialNum:      "TestNIC",
		},
	}

	cert := []byte("invalid")
	if validCert {
		cert, err = pa.GetPlatformCertificate(&nic)
		if err != nil {
			log.Fatalf("Error getting platform cert. NIC: %+v, error: %v", nic, err)
		}
	}

	// Create cluster cert key and CSR
	key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error creating private key")
	csr, err := certs.CreateCSR(key, nil, []string{hostName}, nil)
	AssertOk(t, err, "Error creating CSR")

	admReq := &grpc.RegisterNICRequest{
		AdmissionRequest: &grpc.NICAdmissionRequest{
			Nic:  nic,
			Cert: cert,
			ClusterCertSignRequest: csr.Raw,
		},
	}
	err = stream.Send(admReq)
	AssertOk(t, err, "Error sending request")

	if expectChallenge {
		// receive the authentication request
		authReq, err := stream.Recv()
		AssertOk(t, err, "Error receiving response")
		Assert(t, authReq.AuthenticationRequest != nil, "expected auth request")

		// prepare response
		signer, err := pa.GetPlatformSigner(&nic)
		AssertOk(t, err, "Error getting platform signer")
		claimantRandom, challengeResponse, err := certs.GeneratePoPChallengeResponse(signer, authReq.AuthenticationRequest.Challenge)
		AssertOk(t, err, "Error getting challenge response")

		authResp := &grpc.RegisterNICRequest{
			AuthenticationResponse: &grpc.AuthenticationResponse{
				ClaimantRandom:    claimantRandom,
				ChallengeResponse: challengeResponse,
			},
		}
		return admReq.AdmissionRequest, authResp.AuthenticationResponse, nil
	} else {
		// receive the error response
		errResp, err := stream.Recv()
		AssertOk(t, err, "Error receiving response")
		Assert(t, errResp.AdmissionResponse != nil, "expected auth request")
		return admReq.AdmissionRequest, nil, errResp.AdmissionResponse
	}
}

// TestRegisterSmartNICByNaples tests RegisterNIC
// functionality initiated by User
func TestRegisterSmartNICByNaples(t *testing.T) {

	// Init required components
	testSetup()
	defer testTeardown()

	testCases := []struct {
		name        string
		mac         string
		autoAdmit   bool
		validCert   bool
		expected    string
		condition   cmd.SmartNICCondition
		approvedNIC string
		hostName    string
	}{
		{
			"TestAutoAdmitValidNIC",
			"4444.4444.0001",
			true,
			true,
			cmd.SmartNICStatus_ADMITTED.String(),
			cmd.SmartNICCondition{
				Type:               cmd.SmartNICCondition_HEALTHY.String(),
				Status:             cmd.ConditionStatus_TRUE.String(),
				LastTransitionTime: time.Now().Format(time.RFC3339),
			},
			"",
			"esx-1",
		},
		{
			"TestAutoAdmitInvalidNIC",
			"4444.4444.0002",
			true,
			false,
			cmd.SmartNICStatus_REJECTED.String(),
			cmd.SmartNICCondition{},
			"",
			"esx-2",
		},
		{
			"TestManualValidNIC",
			"4444.4444.0003",
			false,
			true,
			cmd.SmartNICStatus_PENDING.String(),
			cmd.SmartNICCondition{
				Type:               cmd.SmartNICCondition_HEALTHY.String(),
				Status:             cmd.ConditionStatus_FALSE.String(),
				LastTransitionTime: time.Now().Format(time.RFC3339),
			},
			"",
			"esx-3",
		},
		{
			"TestManualValidApprovedNIC",
			"4444.4444.0004",
			false,
			true,
			cmd.SmartNICStatus_PENDING.String(),
			cmd.SmartNICCondition{
				Type:               cmd.SmartNICCondition_HEALTHY.String(),
				Status:             cmd.ConditionStatus_TRUE.String(),
				LastTransitionTime: time.Now().Format(time.RFC3339),
			},
			"4444.4444.0004",
			"esx-4",
		},
		{
			"TestManualInvalidNIC",
			"4444.4444.0005",
			false,
			false,
			cmd.SmartNICStatus_REJECTED.String(),
			cmd.SmartNICCondition{},
			"",
			"esx-5",
		},
	}

	// Pre-create some Hosts to simulate the case of prior
	// Host creation by Orchestrator (VC-hub)
	for ii, _ := range testCases {
		host := &cmd.Host{
			ObjectMeta: api.ObjectMeta{
				Name: testCases[ii].hostName,
			},
			Spec: cmd.HostSpec{
				Interfaces: map[string]cmd.HostIntfSpec{
					testCases[ii].mac: cmd.HostIntfSpec{},
				},
			},
		}
		_, err := tInfo.smartNICServer.CreateHost(host)
		AssertOk(t, err, "Error creating Host object")
	}

	// Execute the testcases
	for _, tc := range testCases {

		t.Run(tc.name, func(t *testing.T) {

			// create API clients
			smartNICRegistrationRPCClient := grpc.NewSmartNICRegistrationClient(tInfo.rpcClient.ClientConn)
			smartNICUpdatesRPCClient := grpc.NewSmartNICUpdatesClient(tInfo.rpcClient.ClientConn)

			// Set cluster admission mode to auto-admit or manual based on test input
			refObj, err := tInfo.smartNICServer.GetCluster()
			if err != nil {
				t.Fatalf("Error getting Cluster object, err: %v", err)
			}

			var clRef cmd.Cluster
			clRef.ObjectMeta.Name = "testCluster"
			clRef.ObjectMeta.ResourceVersion = refObj.ObjectMeta.ResourceVersion
			clRef.Spec.AutoAdmitNICs = tc.autoAdmit
			clObj, err := tInfo.apiClient.ClusterV1().Cluster().Update(context.Background(), &clRef)
			if err != nil || clObj == nil {
				t.Fatalf("Error updating cluster auto-admit status: %v %v", clObj, err)
			}

			// register NIC call
			stream, err := smartNICRegistrationRPCClient.RegisterNIC(context.Background())
			AssertOk(t, err, "Error creating stream")

			expectChallenge := tc.expected == cmd.SmartNICStatus_ADMITTED.String() || tc.expected == cmd.SmartNICStatus_PENDING.String()
			admReq, challengeResp, errResp := doPhase1Exchange(t, stream, tc.mac, tc.hostName, tc.validCert, expectChallenge)
			nic := admReq.Nic
			ometa := nic.ObjectMeta

			if !expectChallenge {
				Assert(t, errResp != nil, "Server did not return expected error")
				verifySmartNICObj(t, tc.mac, false)
				return
			}

			// respond
			Assert(t, errResp == nil, fmt.Sprintf("Server returned unexpected error: %+v", err))
			err = stream.Send(&grpc.RegisterNICRequest{AuthenticationResponse: challengeResp})
			AssertOk(t, err, "Error sending request")

			// receive the admission response
			admResp, err := stream.Recv()
			AssertOk(t, err, "Error receiving response")

			t.Logf("Testcase: %s MAC: %s expected: %v obtained: %v err: %v", tc.name, tc.mac, tc.expected, admResp, err)

			AssertOk(t, err, "Error registering NIC")
			Assert(t, admResp.AdmissionResponse.Phase == tc.expected, "\nexpected:\n%v\nobtained:\n%v",
				tc.expected, admResp.AdmissionResponse.Phase)

			// verify smartNIC is created
			verifySmartNICObj(t, tc.mac, true)

			// verify watch api is invoked
			f2 := func() (bool, interface{}) {
				stream, err := smartNICUpdatesRPCClient.WatchNICs(context.Background(), &ometa)
				if err != nil {
					t.Errorf("Error watching smartNIC, mac: %s, err: %v", tc.mac, err)
					return false, nil
				}
				evt, err := stream.Recv()
				if err != nil {
					t.Errorf("Error receiving from stream, mac: %s err: %v", tc.mac, err)
					return false, nil
				}
				if evt.Nic.Name != tc.mac {
					t.Errorf("Got incorrect smartNIC watch event: %v expected: %s err: %v", evt, tc.mac, err)
					return false, nil
				}

				return true, nil
			}
			AssertEventually(t, f2, fmt.Sprintf("Failed to verify watch for smartNIC event"))

			// Verify UpdateNIC RPC
			f3 := func() (bool, interface{}) {
				var phase string
				if nic.ObjectMeta.Name == tc.approvedNIC {
					phase = cmd.SmartNICStatus_ADMITTED.String()
				}
				ometa = api.ObjectMeta{Name: tc.mac}
				nic = cmd.SmartNIC{
					TypeMeta:   api.TypeMeta{Kind: "SmartNIC"},
					ObjectMeta: ometa,
					Status: cmd.SmartNICStatus{
						AdmissionPhase: phase,
						Conditions: []cmd.SmartNICCondition{
							tc.condition,
						},
					},
				}

				req := &grpc.UpdateNICRequest{
					Nic: nic,
				}

				resp, err := smartNICUpdatesRPCClient.UpdateNIC(context.Background(), req)
				if err != nil {
					t.Logf("Testcase: %s Failed to update NIC, mac: %s req: %+v resp: %+v", tc.name, tc.mac, req.Nic, resp.Nic)
					return false, nil
				}

				if resp.Nic.Status.Conditions[0].Type != tc.condition.Type || resp.Nic.Status.Conditions[0].Status != tc.condition.Status {
					t.Logf("Testcase: %s,  Condition expected:\n%+v\nobtained:%+v", tc.name, tc.condition, resp.Nic.Status.Conditions[0])
					return false, nil
				}

				if nic.ObjectMeta.Name == tc.approvedNIC {
					if resp.Nic.Status.AdmissionPhase != cmd.SmartNICStatus_ADMITTED.String() {
						t.Logf("Testcase: %s \nPhase expected:\n%+v\nobtained:\n%+v", tc.name, cmd.SmartNICStatus_ADMITTED.String(), resp.Nic.Status.AdmissionPhase)
						return false, nil
					}
				}

				return true, nil
			}
			AssertEventually(t, f3, fmt.Sprintf("Failed to verify update for smartNIC"))

			// Verify NIC health status goes to UNKNOWN after deadtimeInterval
			f4 := func() (bool, interface{}) {

				nicObj, err := tInfo.smartNICServer.GetSmartNIC(ometa)
				if err != nil {
					t.Errorf("Error getting NIC object for mac:%s", tc.mac)
					return false, nil
				}
				if nicObj.ObjectMeta.Name != tc.mac {
					t.Errorf("Got incorrect smartNIC object, expected: %s obtained: %s",
						nicObj.ObjectMeta.Name, tc.mac)
					return false, nil
				}

				if nicObj.Status.Conditions[0].Type != tc.condition.Type || nicObj.Status.Conditions[0].Status != cmd.ConditionStatus_UNKNOWN.String() {
					t.Logf("Testcase: %s,  Condition expected:\n%+v\nobtained:%+v", tc.name, cmd.ConditionStatus_UNKNOWN, nicObj.Status.Conditions[0])
					return false, nil
				}

				return true, nil
			}
			AssertEventually(t, f4, fmt.Sprintf("Failed to verify NIC health status going to UNKNOWN"))

			// Verify Host object has its status updated with list of registered NICs
			f5 := func() (bool, interface{}) {
				ometa = api.ObjectMeta{Name: tc.hostName}
				hostObj, err := tInfo.smartNICServer.GetHost(ometa)
				if err != nil {
					t.Errorf("Error getting Host object for host:%s", tc.hostName)
					return false, nil
				}
				t.Logf("\n++++++ Host NIC list, host: %+v \n", hostObj)

				for ii := range hostObj.Status.SmartNICs {
					if hostObj.Status.SmartNICs[ii] == tc.mac {
						return true, nil
					}

				}
				return false, nil
			}
			AssertEventually(t, f5, fmt.Sprintf("Failed to verify that Host object is updated with registered nic"))

			// Verify Deletion of SmartNIC object
			f6 := func() (bool, interface{}) {
				ometa = api.ObjectMeta{Name: tc.mac}
				err = tInfo.smartNICServer.DeleteSmartNIC(ometa)
				if err != nil {
					return false, nil
				}
				return true, nil
			}
			AssertEventually(t, f6, fmt.Sprintf("Failed to verify deletion of smartNIC object"))

			if err != nil {
				t.Fatalf("Error deleteing SmartNIC object mac:%s err: %v", tc.mac, err)
			}

			// Verify Deletion of Host object
			f7 := func() (bool, interface{}) {
				ometa = api.ObjectMeta{Name: tc.hostName}
				err = tInfo.smartNICServer.DeleteHost(ometa)
				if err != nil {
					return false, nil
				}
				return true, nil
			}
			AssertEventually(t, f7, fmt.Sprintf("Failed to verify deletion of Host object"))
		})
	}
}

// TestRegisterSmartNICTimeouts tests that RegisterNIC server times out properly if client is stuck
func TestRegisterSmartNICTimeouts(t *testing.T) {

	// Init required components
	testSetup()
	defer testTeardown()

	baseMac := "44.44.44.44.00."

	// set server-side timeout to a small value to speed-up tests
	srvTimeout := 300
	SetNICRegTimeout(time.Duration(srvTimeout) * time.Millisecond)

	for i := 0; i < 50; i++ {
		nicName := fmt.Sprintf("%s%02d", baseMac, i)
		// Create Host
		host := &cmd.Host{
			ObjectMeta: api.ObjectMeta{
				Name: fmt.Sprintf("esxt-%d", i),
			},
			Spec: cmd.HostSpec{
				Interfaces: map[string]cmd.HostIntfSpec{
					nicName: cmd.HostIntfSpec{},
				},
			},
		}

		_, err := tInfo.smartNICServer.CreateHost(host)
		AssertOk(t, err, "Error creating Host object")

		// create API client
		smartNICRegistrationRPCClient := grpc.NewSmartNICRegistrationClient(tInfo.rpcClient.ClientConn)
		// register NIC call
		stream, err := smartNICRegistrationRPCClient.RegisterNIC(context.Background())
		AssertOk(t, err, "Error creating stream")

		minSleep := 100 // 100ms margin to avoid cases where sleep time and timout are too close
		sleepTimeDelta := mathrand.Intn(srvTimeout-minSleep) + minSleep
		expectSuccess := (sleepTimeDelta % 2) == 0
		var sleepTime time.Duration
		if expectSuccess {
			sleepTime = time.Duration(srvTimeout-sleepTimeDelta) * time.Millisecond
		} else {
			sleepTime = time.Duration(srvTimeout+sleepTimeDelta) * time.Millisecond
		}

		_, challengeResp, errResp := doPhase1Exchange(t, stream, nicName, host.Name, true, true)
		Assert(t, errResp == nil, fmt.Sprintf("Server returned unexpected error: %+v", err))

		time.Sleep(sleepTime)

		// Server-side failure can be detected either by Send() or Recv()
		err = stream.Send(&grpc.RegisterNICRequest{AuthenticationResponse: challengeResp})
		if !expectSuccess && err != nil {
			// failed as expected
			verifySmartNICObj(t, nicName, false)
			continue
		}

		admResp, err := stream.Recv()
		if !expectSuccess {
			Assert(t, err != nil, fmt.Sprintf("request did not fail as expected: sleeptime: %v, err %v", sleepTime, err))
			verifySmartNICObj(t, nicName, false)
			continue
		}

		Assert(t, err == nil, fmt.Sprintf("Unexpected error with challenge response, %v", err))
		Assert(t, admResp.AdmissionResponse.Phase == cmd.SmartNICStatus_ADMITTED.String(), fmt.Sprintf("unexpected phase: %v", admResp.AdmissionResponse.Phase))

		// verify smartNIC is created
		verifySmartNICObj(t, nicName, true)
	}
}

func TestRegisterSmartNICProtocolErrors(t *testing.T) {

	// Init required components
	testSetup()
	defer testTeardown()

	hostName := "4444.4444.0001"
	nicName := "4444.4444.0001"

	// cluster certificate key and csr
	key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error creating private key")
	csr, err := certs.CreateCSR(key, nil, []string{hostName}, nil)
	AssertOk(t, err, "Error creating CSR")

	// Construct SmartNIC object with valid cert/signature
	ometa := api.ObjectMeta{Name: nicName}
	nic := cmd.SmartNIC{
		TypeMeta:   api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: ometa,
		Spec: cmd.SmartNICSpec{
			Hostname: hostName,
			IPConfig: &cmd.IPConfig{
				IPAddress: "0.0.0.0/0",
			},
			MgmtMode: cmd.SmartNICSpec_NETWORK.String(),
		},
		Status: cmd.SmartNICStatus{
			AdmissionPhase: cmd.SmartNICStatus_REGISTERING.String(),
			SerialNum:      "TestNIC",
		},
	}

	// create a platform agent and NMD instance to access factory key/cert
	pa, err := platform.NewNaplesPlatformAgent()
	AssertOk(t, err, "Error creating platform agent. Err: %v", err)

	// platform key and certificate
	cert, err := pa.GetPlatformCertificate(&nic)
	AssertOk(t, err, "Error getting platform cert")

	// create API client
	smartNICRegistrationRPCClient := grpc.NewSmartNICRegistrationClient(tInfo.rpcClient.ClientConn)

	invalidAdmReqs := []grpc.RegisterNICRequest{
		grpc.RegisterNICRequest{},
		grpc.RegisterNICRequest{
			AdmissionRequest: &grpc.NICAdmissionRequest{
				Nic:  cmd.SmartNIC{},
				Cert: cert,
				ClusterCertSignRequest: csr.Raw,
			},
		},
		grpc.RegisterNICRequest{
			AdmissionRequest: &grpc.NICAdmissionRequest{
				Nic:  nic,
				Cert: nil,
				ClusterCertSignRequest: csr.Raw,
			},
		},
		grpc.RegisterNICRequest{
			AdmissionRequest: &grpc.NICAdmissionRequest{
				Nic:  nic,
				Cert: cert,
				ClusterCertSignRequest: nil,
			},
		},
	}

	for i, req := range invalidAdmReqs {
		stream, err := smartNICRegistrationRPCClient.RegisterNIC(context.Background())
		AssertOk(t, err, "Error creating stream")
		err = stream.Send(&req)
		AssertOk(t, err, "Error sending request")
		resp, err := stream.Recv()
		Assert(t, resp.AuthenticationRequest == nil, "Authentication request despite invalid admission request %v: %v", i, resp)
		Assert(t, isErrorResponse(resp), fmt.Sprintf("No error with invalid request %v: %v", i, resp))
		verifySmartNICObj(t, nicName, false)
	}

	// now test protocols errors in Phase 2
	invalidChallengeResp := []grpc.RegisterNICRequest{
		grpc.RegisterNICRequest{},
		grpc.RegisterNICRequest{
			AuthenticationResponse: nil,
		},
		grpc.RegisterNICRequest{
			AuthenticationResponse: &grpc.AuthenticationResponse{},
		},
		grpc.RegisterNICRequest{
			AuthenticationResponse: &grpc.AuthenticationResponse{
				ChallengeResponse: []byte("invalid"),
			},
		},
	}

	for i, chResp := range invalidChallengeResp {
		stream, err := smartNICRegistrationRPCClient.RegisterNIC(context.Background())
		AssertOk(t, err, "Error creating stream")
		_, _, errResp := doPhase1Exchange(t, stream, nicName, hostName, true, true)
		Assert(t, errResp == nil, fmt.Sprintf("Server returned unexpected error, index: %d, resp: %+v", i, errResp))
		err = stream.Send(&chResp)
		AssertOk(t, err, "Error sending request")
		resp, err := stream.Recv()
		AssertOk(t, err, "Error receiving response")
		Assert(t, isErrorResponse(resp), fmt.Sprintf("Unexpected positive response, index: %d, resp: %+v", i, resp))
		verifySmartNICObj(t, nicName, false)
	}
}

func TestUpdateSmartNIC(t *testing.T) {

	// Init required components
	testSetup()
	defer testTeardown()

	// Verify create nic
	nic := cmd.SmartNIC{
		TypeMeta:   api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{Name: "2222.2222.2222"},
		Spec: cmd.SmartNICSpec{
			IPConfig: &cmd.IPConfig{
				IPAddress: "10.1.1.1/24",
			},
			MgmtMode: cmd.SmartNICSpec_NETWORK.String(),
		},
		Status: cmd.SmartNICStatus{
			AdmissionPhase: "UNKNOWN",
		},
	}
	nicObj, err := tInfo.smartNICServer.UpdateSmartNIC(&nic)
	AssertOk(t, err, "Failed to create nic object, 2222.2222.222")

	// Verify update nic
	nic = cmd.SmartNIC{
		TypeMeta:   nicObj.TypeMeta,
		ObjectMeta: nicObj.ObjectMeta,
		Status: cmd.SmartNICStatus{
			Conditions: []cmd.SmartNICCondition{
				{
					Type:   cmd.SmartNICCondition_HEALTHY.String(),
					Status: cmd.ConditionStatus_FALSE.String(),
				},
			},
		},
	}
	nicObj, err = tInfo.smartNICServer.UpdateSmartNIC(&nic)
	AssertOk(t, err, "Failed to update nic object, 2222.2222.222")

	// delete nic
	err = tInfo.smartNICServer.DeleteSmartNIC(nic.ObjectMeta)
	AssertOk(t, err, "Failed to delete nic object, 2222.2222.222")
}

func TestDeleteSmartNIC(t *testing.T) {

	testSetup()
	defer testTeardown()

	// Verify Delete SmartNIC object
	ometa := api.ObjectMeta{Name: "1111.1111.1111"}
	err := tInfo.smartNICServer.DeleteSmartNIC(ometa)
	Assert(t, err != nil, "SmartNIC object - 1111.1111.1111 - should not exist")
}

// TestSmartNICConfigByUser tests the following scenario & actions
// - SmartNIC config by user with Mgmt-IP
// - CMD would post naples config via REST to initiate nic registration
//   in managed mode
// - NMD would next do NIC registration
// - CMD would validate NIC and admit NIC into cluster.
func TestSmartNICConfigByUser(t *testing.T) {

	// Init required components
	testSetup()
	defer testTeardown()

	hostID := getHostID(1)
	dbPath := getDBPath(1)
	restURL := getRESTUrl(1)
	testMac := "44.44.44.44.44.01"

	// Cleanup any prior DB files
	os.Remove(dbPath)

	// create Agent and NMD
	ag, err := createNMD(t, dbPath, hostID, restURL)
	defer stopNMD(t, ag, dbPath)
	Assert(t, (err == nil && ag != nil), "Failed to create agent", err)

	nm := ag.GetNMD()

	// Validate default host mode
	f1 := func() (bool, interface{}) {

		cfg := nm.GetNaplesConfig()
		if cfg.Spec.Mode == proto.MgmtMode_HOST && nm.GetListenURL() != "" &&
			nm.GetUpdStatus() == false && nm.GetRegStatus() == false && nm.GetRestServerStatus() == true {
			return true, nil
		}
		return false, nil
	}
	AssertEventually(t, f1, "Failed to verify mode is in host")

	// Create Host in Venice
	host := &cmd.Host{
		ObjectMeta: api.ObjectMeta{
			Name: "esxc",
		},
		Spec: cmd.HostSpec{
			Interfaces: map[string]cmd.HostIntfSpec{
				testMac: cmd.HostIntfSpec{},
			},
		},
	}
	_, err = tInfo.smartNICServer.CreateHost(host)
	AssertOk(t, err, "Error creating Host object")

	// Create SmartNIC object in Venice
	nic := cmd.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name: testMac,
		},
		Spec: cmd.SmartNICSpec{
			IPConfig: &cmd.IPConfig{
				IPAddress: "127.0.0.1/32",
			},
			Hostname: hostID,
			MgmtMode: cmd.SmartNICSpec_NETWORK.String(),
		},
		Status: cmd.SmartNICStatus{
			AdmissionPhase: "UNKNOWN",
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
		if cfg.Spec.Mode != proto.MgmtMode_NETWORK {
			log.Errorf("Failed to switch to managed mode")
			return false, nil
		}

		// Fetch smartnic object
		nic, err := nm.GetSmartNIC()
		if nic == nil || err != nil {
			log.Errorf("NIC not found in nicDB, mac:%s", hostID)
			return false, nil
		}

		// Verify NIC is admitted
		if nic.Status.AdmissionPhase != cmd.SmartNICStatus_ADMITTED.String() {
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
			Name: testMac,
		}
		nicObj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), &meta)
		if err != nil || nicObj == nil || nicObj.Status.AdmissionPhase != cmd.SmartNICStatus_ADMITTED.String() {
			log.Errorf("Failed to validate phase of SmartNIC object, mac:%s, phase: %s err: %v",
				testMac, nicObj.Status.AdmissionPhase, err)
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f5, "Failed to verify creation of required SmartNIC object", string("10ms"), string("30s"))

	// Validate Host object is updated
	f6 := func() (bool, interface{}) {

		meta := api.ObjectMeta{
			Name: "esxc",
		}
		hostObj, err := tInfo.apiClient.ClusterV1().Host().Get(context.Background(), &meta)
		if err != nil || hostObj == nil {
			log.Errorf("Failed to GET Host object :%s, %v", meta.Name, err)
			return false, nil
		}

		for ii := range hostObj.Status.SmartNICs {
			if hostObj.Status.SmartNICs[ii] == testMac {
				return true, nil
			}
		}
		return false, nil
	}
	AssertEventually(t, f6, "Failed to verify update of required Host object", string("10ms"), string("30s"))

	// Verify Deletion of SmartNIC object
	f7 := func() (bool, interface{}) {
		ometa := api.ObjectMeta{Name: testMac}
		err = tInfo.smartNICServer.DeleteSmartNIC(ometa)
		if err != nil {
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f7, fmt.Sprintf("Failed to verify deletion of smartNIC object"))

	// Verify update of Host object
	f8 := func() (bool, interface{}) {
		meta := api.ObjectMeta{
			Name: "esxc",
		}
		hostObj, err := tInfo.apiClient.ClusterV1().Host().Get(context.Background(), &meta)
		if err != nil || hostObj == nil {
			log.Errorf("Failed to GET Host object, mac:%s, %v", hostID, err)
			return false, nil
		}
		for ii := range hostObj.Status.SmartNICs {
			if hostObj.Status.SmartNICs[ii] == testMac {
				return false, nil
			}
		}
		return true, nil
	}
	AssertEventually(t, f8, fmt.Sprintf("Failed to verify deletion of Host object"))
}

func TestSmartNICConfigByUserErrorCases(t *testing.T) {

	// Init required components
	testSetup()
	defer testTeardown()

	hostID := getHostID(1)
	dbPath := getDBPath(1)
	restURL := getRESTUrl(1)

	// Cleanup any prior DB files
	os.Remove(dbPath)

	// create Agent and NMD
	ag, err := createNMD(t, dbPath, hostID, restURL)
	defer stopNMD(t, ag, dbPath)
	Assert(t, (err == nil && ag != nil), "Failed to create agent", err)

	nm := ag.GetNMD()

	// Validate default host mode
	f1 := func() (bool, interface{}) {

		cfg := nm.GetNaplesConfig()
		if cfg.Spec.Mode == proto.MgmtMode_HOST && nm.GetListenURL() != "" &&
			nm.GetUpdStatus() == false && nm.GetRegStatus() == false && nm.GetRestServerStatus() == true {
			return true, nil
		}
		return false, nil
	}
	AssertEventually(t, f1, "Failed to verify mode is in host")

	// Delete the SmartNIC object if it exists
	_, err = tInfo.apiClient.ClusterV1().SmartNIC().Delete(context.Background(), &api.ObjectMeta{Name: hostID})

	// Create SmartNIC object in Venice
	nic := cmd.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name: hostID,
		},
		Spec: cmd.SmartNICSpec{
			IPConfig: &cmd.IPConfig{
				IPAddress: "remotehost", // unreachable hostname for testing error case
			},
			Hostname: hostID,
			MgmtMode: cmd.SmartNICSpec_NETWORK.String(),
		},
		Status: cmd.SmartNICStatus{
			AdmissionPhase: "UNKNOWN",
		},
	}

	_, err = tInfo.apiClient.ClusterV1().SmartNIC().Create(context.Background(), &nic)
	if err != nil {
		t.Fatalf("Failed to create smartnic: %+v, err: %v", nic, err)
	}

	// Verify SmartNIC object has UNREACHABLE condition
	f2 := func() (bool, interface{}) {

		meta := api.ObjectMeta{
			Name: hostID,
		}
		nicObj, err := tInfo.apiClient.ClusterV1().SmartNIC().Get(context.Background(), &meta)
		if err != nil || nicObj == nil || len(nicObj.Status.Conditions) == 0 ||
			nicObj.Status.Conditions[0].Type != cmd.SmartNICCondition_UNREACHABLE.String() ||
			nicObj.Status.Conditions[0].Status != cmd.ConditionStatus_TRUE.String() {
			log.Errorf("Failed to validate SmartNIC condition, nicObj: %+v, err: %v",
				nicObj, err)
			return false, nil
		}

		return true, nil
	}
	AssertEventually(t, f2, "Failed to verify SmartNIC object has UNREACHABLE condition set",
		string("10ms"), string("30s"))

	// Verify Deletion of SmartNIC object
	f3 := func() (bool, interface{}) {
		ometa := api.ObjectMeta{Name: hostID}
		err = tInfo.smartNICServer.DeleteSmartNIC(ometa)
		if err != nil {
			return false, nil
		}
		return true, nil
	}
	AssertEventually(t, f3, fmt.Sprintf("Failed to verify deletion of smartNIC object"))
}

func testSetup() {

	// Disable open trace
	ventrace.DisableOpenTrace()
	tsdb.Init(tsdb.DummyTransmitter{}, tsdb.Options{})

	// Create api server
	apiServerAddress := ":0"
	l := log.WithContext("module", "CrudOpsTest")
	tInfo.l = l
	scheme := runtime.GetDefaultScheme()
	srvConfig := apiserver.Config{
		GrpcServerPort: apiServerAddress,
		DebugMode:      false,
		Logger:         l,
		Version:        "v1",
		Scheme:         scheme,
		Kvstore: store.Config{
			Type:  store.KVStoreTypeMemkv,
			Codec: runtime.NewJSONCodec(scheme),
		},
		GetOverlay: api_cache.GetOverlay,
		IsDryRun:   api_cache.IsDryRun,
	}
	grpclog.SetLogger(l)
	tInfo.apiServer = apiserverpkg.MustGetAPIServer()
	go tInfo.apiServer.Run(srvConfig)
	tInfo.apiServer.WaitRunning()
	addr, err := tInfo.apiServer.GetAddr()
	if err != nil {
		os.Exit(-1)
	}

	// Create api client
	tInfo.apiServerAddr = addr
	apiCl, err := apicache.NewGrpcUpstream("smartnic_test", tInfo.apiServerAddr, tInfo.l)
	if err != nil {
		fmt.Printf("Cannot create gRPC client - %v", err)
		os.Exit(-1)
	}
	tInfo.apiClient = apiCl

	// create gRPC server for smartNIC service and gRPC client
	tInfo.rpcServer, tInfo.rpcClient = createRPCServerClient()
	if tInfo.rpcServer == nil || tInfo.rpcClient == nil {
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
}

func testTeardown() {

	// delete cluster object
	clRef := &cmd.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
	}
	_, err := tInfo.apiClient.ClusterV1().Cluster().Delete(context.Background(), &clRef.ObjectMeta)
	if err != nil {
		fmt.Printf("Error deleting Cluster object, %v", err)
		os.Exit(-1)
	}

	// stop the rpc client and server
	tInfo.rpcClient.Close()
	tInfo.rpcServer.Stop()

	// stop the apiServer
	tInfo.apiServer.Stop()

	// close the CMD certmgr
	if cmdenv.CertMgr != nil {
		cmdenv.CertMgr.Close()
	}
}

func TestMain(m *testing.M) {
	// Run tests
	rcode := m.Run()

	os.Exit(rcode)
}
