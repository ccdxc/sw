// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package smartnic

import (
	"context"
	"fmt"
	"net"
	"os"
	"testing"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cmd"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	apiserver "github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/cmd/grpc"
	store "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
	"google.golang.org/grpc/grpclog"
)

const (
	smartNICServerURL = "localhost:9199"
	// TODO: This is a temporary way of testing invalid cert
	invalidCertSignature = "unknown-cert"
)

type testInfo struct {
	l              log.Logger
	apiServerPort  string
	apiServer      apiserver.Server
	apiClient      apiclient.Services
	rpcServer      *rpckit.RPCServer
	rpcClient      *rpckit.RPCClient
	smartNICServer *RPCServer
}

var tInfo testInfo

// runRPCServer creates a smartNIC server for SmartNIC service.
func createRPCServer(m *testing.M, url, certFile, keyFile, caFile string) (*rpckit.RPCServer, error) {
	// create an RPC server.
	rpcServer, err := rpckit.NewRPCServer("smartNIC", url)
	if err != nil {
		fmt.Printf("Error creating RPC-server: %v", err)
		return nil, err
	}
	tInfo.rpcServer = rpcServer

	// create and register the RPC handler for SmartNIC service
	tInfo.smartNICServer = NewRPCServer(tInfo.apiClient.CmdV1().Cluster(), tInfo.apiClient.CmdV1().Node(), tInfo.apiClient.CmdV1().SmartNIC())
	grpc.RegisterSmartNICServer(rpcServer.GrpcServer, tInfo.smartNICServer)

	return rpcServer, nil
}

// createRPCServerClient creates rpc client and server for SmartNIC service
func createRPCServerClient(m *testing.M) (*rpckit.RPCServer, *rpckit.RPCClient) {

	// start the rpc server
	rpcServer, err := createRPCServer(m, smartNICServerURL, "", "", "")
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

// TestRegisterSmartNIC tests RegisterNIC functionality
func TestRegisterSmartNIC(t *testing.T) {

	testCases := []struct {
		name        string
		mac         string
		autoAdmit   bool
		cert        []byte
		expected    string
		condition   cmd.SmartNICCondition
		approvedNIC string
		nodeName    string
	}{
		{
			"TestAutoAdmitValidNIC",
			"4444.4444.0001",
			true,
			[]byte(validCertSignature),
			cmd.SmartNICSpec_ADMITTED.String(),
			cmd.SmartNICCondition{
				Type:   cmd.SmartNICCondition_HEALTHY.String(),
				Status: cmd.ConditionStatus_TRUE.String(),
			},
			"",
			"esx-001",
		},
		{
			"TestAutoAdmitInvalidNIC",
			"4444.4444.0002",
			true,
			[]byte(invalidCertSignature),
			cmd.SmartNICSpec_REJECTED.String(),
			cmd.SmartNICCondition{},
			"",
			"esx-002",
		},
		{
			"TestManualValidNIC",
			"4444.4444.0003",
			false,
			[]byte(validCertSignature),
			cmd.SmartNICSpec_PENDING.String(),
			cmd.SmartNICCondition{
				Type:   cmd.SmartNICCondition_HEALTHY.String(),
				Status: cmd.ConditionStatus_FALSE.String(),
			},
			"",
			"esx-003",
		},
		{
			"TestManualValidApprovedNIC",
			"4444.4444.0004",
			false,
			[]byte(validCertSignature),
			cmd.SmartNICSpec_PENDING.String(),
			cmd.SmartNICCondition{
				Type:   cmd.SmartNICCondition_HEALTHY.String(),
				Status: cmd.ConditionStatus_TRUE.String(),
			},
			"4444.4444.0004",
			"esx-004",
		},
		{
			"TestManualInvalidNIC",
			"4444.4444.0005",
			false,
			[]byte(invalidCertSignature),
			cmd.SmartNICSpec_REJECTED.String(),
			cmd.SmartNICCondition{},
			"",
			"esx-005",
		},
	}

	// Pre-create some Nodes to simulate the case of prior
	// Node creation by Orchestrator (VC-hub)
	nodes := []string{
		"esx-003",
	}
	for _, nodeName := range nodes {
		ometa := api.ObjectMeta{
			Name: nodeName,
		}
		_, err := tInfo.smartNICServer.CreateNode(ometa)
		AssertOk(t, err, "Error creating Node object")
	}

	// Execute the testcases
	for _, tc := range testCases {

		t.Run(tc.name, func(t *testing.T) {

			// create API client
			smartNICRPCClient := grpc.NewSmartNICClient(tInfo.rpcClient.ClientConn)

			// Set cluster admission mode to auto-admit or manual based on test input
			refObj, err := tInfo.smartNICServer.GetCluster()
			if err != nil {
				t.Fatalf("Error getting Cluster object, err: %v", err)
			}

			var clRef cmd.Cluster
			clRef.ObjectMeta.Name = "testCluster"
			clRef.ObjectMeta.ResourceVersion = refObj.ObjectMeta.ResourceVersion
			clRef.Spec.AutoAdmitNICs = tc.autoAdmit
			clObj, err := tInfo.smartNICServer.ClusterAPI.Update(context.Background(), &clRef)
			if err != nil || clObj == nil {
				t.Fatalf("Error updating cluster auto-admit status: %v %v", clObj, err)
			}

			// Construct SmartNIC object with valid cert/signature
			ometa := api.ObjectMeta{Name: tc.mac}
			nic := cmd.SmartNIC{
				TypeMeta:   api.TypeMeta{Kind: "SmartNIC"},
				ObjectMeta: ometa,
				Spec: cmd.SmartNICSpec{
					Phase: cmd.SmartNICSpec_UNKNOWN.String(),
				},
				Status: cmd.SmartNICStatus{
					NodeName: tc.nodeName,
				},
			}

			req := &grpc.RegisterNICRequest{
				Nic:  nic,
				Cert: tc.cert,
			}

			// register NIC call
			resp, err := smartNICRPCClient.RegisterNIC(context.Background(), req)
			t.Logf("Testcase: %s MAC: %s expected: %v obtained: %v err: %v", tc.name, tc.mac, tc.expected, resp, err)

			AssertOk(t, err, "Error registering NIC")
			Assert(t, resp.Status == tc.expected, "\nexpected:\n%v\nobtained:\n%v",
				tc.expected, resp.Status)

			if tc.expected == cmd.SmartNICSpec_ADMITTED.String() || tc.expected == cmd.SmartNICSpec_PENDING.String() {

				// verify smartNIC is created
				f1 := func() (bool, []interface{}) {
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

					return true, nil
				}
				AssertEventually(t, f1, fmt.Sprintf("Failed to verify presence of smartNIC object"))

				// verify watch api is invoked
				f2 := func() (bool, []interface{}) {
					stream, err := smartNICRPCClient.WatchNICs(context.Background(), &ometa)
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
				f3 := func() (bool, []interface{}) {
					var phase string
					if nic.ObjectMeta.Name == tc.approvedNIC {
						phase = cmd.SmartNICSpec_ADMITTED.String()
					}
					ometa = api.ObjectMeta{Name: tc.mac}
					nic = cmd.SmartNIC{
						TypeMeta:   api.TypeMeta{Kind: "SmartNIC"},
						ObjectMeta: ometa,
						Spec: cmd.SmartNICSpec{
							Phase: phase,
						},
						Status: cmd.SmartNICStatus{
							Conditions: []*cmd.SmartNICCondition{
								&tc.condition,
							},
						},
					}

					req := &grpc.UpdateNICRequest{
						Nic: nic,
					}

					resp, err := smartNICRPCClient.UpdateNIC(context.Background(), req)
					if err != nil {
						t.Logf("Testcase: %s Failed to update NIC, mac: %s req: %+v resp: %+v", tc.name, tc.mac, req.Nic, resp.Nic)
						return false, nil
					}

					if resp.Nic.Status.Conditions[0].Type != tc.condition.Type || resp.Nic.Status.Conditions[0].Status != tc.condition.Status {
						t.Logf("Testcase: %s,  Condition expected:\n%+v\nobtained:%+v", tc.name, tc.condition, resp.Nic.Status.Conditions[0])
						return false, nil
					}

					if nic.ObjectMeta.Name == tc.approvedNIC {
						if resp.Nic.Spec.Phase != cmd.SmartNICSpec_ADMITTED.String() {
							t.Logf("Testcase: %s \nPhase expected:\n%+v\nobtained:\n%+v", tc.name, cmd.SmartNICSpec_ADMITTED.String(), resp.Nic.Spec.Phase)
							return false, nil
						}
					}

					return true, nil
				}
				AssertEventually(t, f3, fmt.Sprintf("Failed to verify update for smartNIC"))

				// Verify Node object has its status updated with list of registered NICs
				f4 := func() (bool, []interface{}) {
					ometa = api.ObjectMeta{Name: tc.nodeName}
					nodeObj, err := tInfo.smartNICServer.GetNode(ometa)
					if err != nil {
						t.Errorf("Error getting Node object for node:%s", tc.nodeName)
						return false, nil
					}
					nicList := nodeObj.Status.Nics
					t.Logf("\n++++++ Node nic list, Node: %s \nnics: %s\n", nodeObj, nicList)

					for _, nic := range nicList {
						if nic == tc.mac {
							return true, nil
						}

					}
					return false, nil
				}
				AssertEventually(t, f4, fmt.Sprintf("Failed to verify that Node object is updated with registered nic"))

				// Verify Deletion of SmartNIC object
				f5 := func() (bool, []interface{}) {
					ometa = api.ObjectMeta{Name: tc.mac}
					err = tInfo.smartNICServer.DeleteSmartNIC(ometa)
					if err != nil {
						return false, nil
					}
					return true, nil
				}
				AssertEventually(t, f5, fmt.Sprintf("Failed to verify deletion of smartNIC object"))

				if err != nil {
					t.Fatalf("Error deleteing SmartNIC object mac:%s err: %v", tc.mac, err)
				}

				// Verify Deletion of Node object
				f6 := func() (bool, []interface{}) {
					ometa = api.ObjectMeta{Name: tc.nodeName}
					err = tInfo.smartNICServer.DeleteNode(ometa)
					if err != nil {
						return false, nil
					}
					return true, nil
				}
				AssertEventually(t, f6, fmt.Sprintf("Failed to verify deletion of Node object"))

			} else {

				// Verify SmartNIC object is not created
				f1 := func() (bool, []interface{}) {
					nicObj, err := tInfo.smartNICServer.GetSmartNIC(ometa)
					if err != nil || nicObj == nil {
						return true, nil
					}
					t.Errorf("SmartNIC object should not exist for mac:%s err:%v", tc.mac, err)
					return false, nil
				}
				AssertEventually(t, f1, fmt.Sprintf("Failed to verify absence of smartNIC object"))
			}

		})
	}
}

func TestUpdateSmartNIC(t *testing.T) {

	// Verify create nic
	nic := cmd.SmartNIC{
		TypeMeta:   api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{Name: "2222.2222.2222"},
	}
	nicObj, err := tInfo.smartNICServer.UpdateSmartNIC(&nic)
	AssertOk(t, err, "Failed to create nic object, 2222.2222.222")

	// Verify update nic
	nic = cmd.SmartNIC{
		TypeMeta:   nicObj.TypeMeta,
		ObjectMeta: nicObj.ObjectMeta,
		Status: cmd.SmartNICStatus{
			Conditions: []*cmd.SmartNICCondition{
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
	// Verify Delete SmartNIC object
	ometa := api.ObjectMeta{Name: "1111.1111.1111"}
	err := tInfo.smartNICServer.DeleteSmartNIC(ometa)
	Assert(t, err != nil, "SmartNIC object - 1111.1111.1111 - should not exist")
}

func testSetup(m *testing.M) {

	// Create api server
	apiServerAddress := ":0"
	l := log.WithContext("module", "CrudOpsTest")
	tInfo.l = l
	scheme := runtime.NewScheme()
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
	}
	grpclog.SetLogger(l)
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
	apiCl, err := cache.NewGrpcUpstream(apiServerAddr, tInfo.l)
	if err != nil {
		fmt.Printf("Cannot create gRPC client - %v", err)
		os.Exit(-1)
	}
	tInfo.apiClient = apiCl

	// create gRPC server for smartNIC service and gRPC client
	tInfo.rpcServer, tInfo.rpcClient = createRPCServerClient(m)
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
			AutoAdmitNICs: false,
		},
	}
	_, err = tInfo.apiClient.CmdV1().Cluster().Create(context.Background(), clRef)
	if err != nil {
		fmt.Printf("Error creating Cluster object, %v", err)
		os.Exit(-1)
	}

}

func testTeardown(m *testing.M) {

	// stop the rpc client and server
	tInfo.rpcClient.Close()
	tInfo.rpcServer.Stop()

	// stop the apiServer
	tInfo.apiServer.Stop()
}

func TestMain(m *testing.M) {

	// Setup
	testSetup(m)

	// Run tests
	rcode := m.Run()

	// Tear down
	testTeardown(m)

	os.Exit(rcode)
}
