// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package testbed

import (
	"context"
	"fmt"
	"os"
	"os/exec"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	loginctx "github.com/pensando/sw/api/login/context"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/log"
)

// mockVeniceURL is the venice URL when we are in mock mode
const mockVeniceURL = "localhost:9443"
const maxVeniceUpWait = 300

type mockIotaServer struct {
	listenURL string
	tb        *TestBed
	authToken string // authToken obtained after logging in
}

// newMockIotaServer creates a mock iota server
func newMockIotaServer(listenURL string, tb *TestBed) (*mockIotaServer, error) {
	serverSvc, err := common.CreateNewGRPCServer("Mock IOTA Server", listenURL, common.GrpcMaxMsgSize)
	if err != nil {
		log.Errorf("Could not start IOTA GRPC server. Err: %v", err)
		return nil, err
	}

	mockIota := mockIotaServer{
		listenURL: listenURL,
		tb:        tb,
	}

	iota.RegisterTopologyApiServer(serverSvc.Srv, &mockIota)
	iota.RegisterConfigMgmtApiServer(serverSvc.Srv, &mockIota)

	go serverSvc.Start()

	// start palazzo simulator
	mockIota.startPalazzo()

	return &mockIota, nil
}

func (ms *mockIotaServer) startPalazzo() {
	// start palazzo if we can find it
	gopath := os.Getenv("GOPATH")
	if gopath != "" {
		// verify palazzo binary exists
		if _, err := os.Stat(gopath + "/bin/palazzo"); err == nil {
			go func() {
				// kill old instance of palazzo if its running
				exec.Command("pkill", "palazzo").CombinedOutput()

				palazzoCmd := fmt.Sprintf("%s/bin/palazzo -hosts %d > %s/src/github.com/pensando/sw/iota/palazzo.log", gopath, ms.tb.numNaples(), gopath)
				log.Infof("Starting palazzo (venice emulator): %s", palazzoCmd)
				time.Sleep(time.Second * 5)

				// run new palazzo instance
				cmd := exec.Command("bash", "-c", palazzoCmd)
				out, err := cmd.CombinedOutput()
				if err != nil {
					fmt.Printf("palazzo out:\n%s\n", string(out))
					log.Fatalf("running palazzo failed with %s\n", err)
				}
				fmt.Printf("combined out:\n%s\n", string(out))
			}()
		} else {
			log.Fatalf("Can not find palazzo binary. Err: %v", err)
		}
	}
}

func (ms *mockIotaServer) InstallImage(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Debugf("InstallImage(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) InitTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Debugf("InitTestBed(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) GetTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Debugf("GetTestBed(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) CleanUpTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Debugf("CleanUpTestBed(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) MoveWorkloads(ctx context.Context, req *iota.WorkloadMoveMsg) (*iota.WorkloadMoveMsg, error) {
	log.Debugf("MoveWorkloads(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// InitNodes initializes test nodes
func (ms *mockIotaServer) InitNodes(ctx context.Context, req *iota.TestNodesMsg) (*iota.TestNodesMsg, error) {
	return nil, nil
}

// CleanNodes cleans up list of nodes and removes association
func (ms *mockIotaServer) CleanNodes(ctx context.Context, req *iota.TestNodesMsg) (*iota.TestNodesMsg, error) {
	return nil, nil
}

// VeniceRestClient returns the REST client for venice
func (ms *mockIotaServer) VeniceRestClient() ([]apiclient.Services, error) {
	var restcls []apiclient.Services
	// connect to Venice
	restcl, err := apiclient.NewRestAPIClient(mockVeniceURL)
	if err != nil {
		log.Errorf("Error connecting to Venice %v. Err: %v", mockVeniceURL, err)
		return nil, err
	}

	restcls = append(restcls, restcl)

	return restcls, nil
}

// VeniceNodeLoggedInCtx logs in to a specified node and returns loggedin context
func (ms *mockIotaServer) VeniceNodeLoggedInCtx(nodeURL string) (context.Context, error) {
	// local user credentials
	userCred := auth.PasswordCredential{
		Username: "test",
		Password: common.UserPassword,
		Tenant:   "default",
	}

	// try to login
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), nodeURL, &userCred)
	if err != nil {
		log.Errorf("Error logging into Venice URL %v. Err: %v", nodeURL, err)
		return nil, err
	}
	authToken, ok := loginctx.AuthzHeaderFromContext(ctx)
	if ok {
		ms.authToken = authToken
	} else {
		return nil, fmt.Errorf("auth token not available in logged-in context")
	}

	return ctx, nil
}

// VeniceLoggedInCtx returns loggedin context for venice taking a context
func (ms *mockIotaServer) VeniceLoggedInCtx(ctx context.Context) (context.Context, error) {
	var err error

	if ms.authToken == "" {
		_, err = ms.VeniceNodeLoggedInCtx(mockVeniceURL)
	}
	if err != nil {
		return nil, err
	}
	return loginctx.NewContextWithAuthzHeader(ctx, ms.authToken), nil
}

// WaitForVeniceClusterUp wait for venice cluster to come up
func (ms *mockIotaServer) WaitForVeniceClusterUp(ctx context.Context) error {
	// wait for cluster to come up
	for i := 0; i < maxVeniceUpWait; i++ {
		restcls, err := ms.VeniceRestClient()
		if err == nil {
			ctx2, err := ms.VeniceLoggedInCtx(ctx)
			if err == nil {
				for _, restcl := range restcls {
					_, err = restcl.ClusterV1().Cluster().Get(ctx2, &api.ObjectMeta{Name: "iota-cluster"})
					if err == nil {
						return nil
					}
				}
			}
		}

		time.Sleep(time.Second)
		if e := ctx.Err(); e != nil {
			return e
		}
	}

	// if we reached here, it means we werent able to connect to Venice API GW
	return fmt.Errorf("Failed to connect to Venice")
}

// ListSmartNIC gets a list of smartnics
func (ms *mockIotaServer) ListSmartNIC() (snl []*cluster.DistributedServiceCard, err error) {
	ctx, err := ms.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := ms.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	opts := api.ListWatchOptions{}

	for _, restcl := range restcls {
		snl, err = restcl.ClusterV1().DistributedServiceCard().List(ctx, &opts)
		if err == nil {
			break
		}
	}

	return snl, err
}

func (ms *mockIotaServer) AddNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Debugf("AddNodes(): Received Request Msg: %v", req)

	// wait for venice to come up
	err := ms.WaitForVeniceClusterUp(ctx)
	if err != nil {
		return req, err
	}

	// get smart nics from palazzo
	snicList, err := ms.ListSmartNIC()
	if err != nil {
		return req, err
	}

	// set node UUID based on smartnic mac
	idx := 0
	for _, nr := range req.Nodes {
		if nr.Type == iota.PersonalityType_PERSONALITY_NAPLES_SIM || nr.Type == iota.PersonalityType_PERSONALITY_NAPLES {
			nr.GetNaplesConfigs().Configs[0].NodeUuid = snicList[idx].Status.PrimaryMAC
			idx++
		}
	}

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) DeleteNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Debugf("DeleteNodes(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) GetNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Debugf("GetNodes(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) SaveNode(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Debugf("SaveNode(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) ReloadNodes(ctx context.Context, req *iota.ReloadMsg) (*iota.ReloadMsg, error) {
	log.Debugf("ReloadNodes(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) IpmiNodeAction(ctx context.Context, req *iota.ReloadMsg) (*iota.ReloadMsg, error) {
	log.Debugf("IpmiNodeAction(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) AddWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	log.Debugf("AddWorkloads(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) GetWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	log.Debugf("GetWorkloads(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) DeleteWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	log.Debugf("DeleteWorkloads(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) Trigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	log.Debugf("Trigger(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) CheckClusterHealth(ctx context.Context, req *iota.NodeMsg) (*iota.ClusterHealthMsg, error) {
	log.Debugf("CheckClusterHealth(): Received Request Msg: %v", req)
	resp := iota.ClusterHealthMsg{
		ApiResponse: &iota.IotaAPIResponse{},
	}
	resp.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return &resp, nil
}

// DoSwitchOperation do switch operation
func (ms *mockIotaServer) DoSwitchOperation(ctx context.Context, req *iota.SwitchMsg) (*iota.SwitchMsg, error) {
	log.Infof("TOPO SVC | DEBUG | SwitchMsg. Received Request Msg: %v", req)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) EntityCopy(ctx context.Context, req *iota.EntityCopyMsg) (*iota.EntityCopyMsg, error) {
	log.Debugf("EntityCopy(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) MakeCluster(ctx context.Context, req *iota.MakeClusterMsg) (*iota.MakeClusterMsg, error) {
	log.Debugf("MakeCluster(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) InitCfgService(ctx context.Context, req *iota.InitConfigMsg) (*iota.InitConfigMsg, error) {
	log.Debugf("InitCfgService(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) GenerateConfigs(ctx context.Context, req *iota.GenerateConfigMsg) (*iota.ConfigMsg, error) {
	log.Debugf("GenerateConfigs(): Received Request Msg: %v", req)
	resp := iota.ConfigMsg{
		ApiResponse: &iota.IotaAPIResponse{},
	}
	resp.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return &resp, nil
}

func (ms *mockIotaServer) ConfigureAuth(ctx context.Context, req *iota.AuthMsg) (*iota.AuthMsg, error) {
	log.Debugf("ConfigureAuth(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) PushConfig(ctx context.Context, req *iota.ConfigMsg) (*iota.ConfigMsg, error) {
	log.Debugf("PushConfig(): Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

func (ms *mockIotaServer) QueryConfig(ctx context.Context, req *iota.ConfigQueryMsg) (*iota.ConfigMsg, error) {
	log.Debugf("QueryConfig(): Received Request Msg: %v", req)
	resp := iota.ConfigMsg{
		ApiResponse: &iota.IotaAPIResponse{},
	}
	resp.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return &resp, nil
}
