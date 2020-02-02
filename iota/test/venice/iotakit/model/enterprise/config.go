// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package enterprise

import (
	"context"
	"crypto/tls"
	"fmt"
	"strings"
	"time"

	"github.com/onsi/ginkgo"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

const maxVeniceUpWait = 300

// VeniceLoggedInCtx returns loggedin context for venice taking a context
func (sm *SysModel) VeniceLoggedInCtx(ctx context.Context) (context.Context, error) {
	var err error

	for _, url := range sm.GetVeniceURL() {
		if sm.authToken == "" {
			_, err = sm.veniceNodeLoggedInCtxWithURL(url)
		}
		if err == nil {
			break
		}
	}
	if err != nil {
		return nil, err
	}
	return loginctx.NewContextWithAuthzHeader(ctx, sm.authToken), nil

}

// VeniceNodeLoggedInCtx logs in to a specified node and returns loggedin context
func (sm *SysModel) veniceNodeLoggedInCtxWithURL(nodeURL string) (context.Context, error) {
	// local user credentials

	userCred := auth.PasswordCredential{
		Username: "admin",
		Password: common.UserPassword,
		Tenant:   "default",
	}

	// overwrite user-id/password in mock mode to match venice integ user cred
	if sm.Tb.IsMockMode() {
		userCred.Username = "test"
		userCred.Password = common.UserPassword
	}

	// try to login
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), nodeURL, &userCred)
	if err != nil {
		log.Errorf("Error logging into Venice URL %v. Err: %v", nodeURL, err)
		return nil, err
	}
	authToken, ok := loginctx.AuthzHeaderFromContext(ctx)
	if ok {
		sm.authToken = authToken
	} else {
		return nil, fmt.Errorf("auth token not available in logged-in context")
	}

	return ctx, nil
}

// GetAuthorizationHeader gets and returns the authorization header from login context
func (sm *SysModel) GetAuthorizationHeader() (string, error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return "", err
	}

	// get authz header
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return "", fmt.Errorf("failed to get authorization header from context")
	}

	return authzHeader, nil
}

// VeniceRestClient returns the REST client for venice
func (sm *SysModel) VeniceRestClient() ([]apiclient.Services, error) {
	var restcls []apiclient.Services
	for _, url := range sm.GetVeniceURL() {
		// connect to Venice
		restcl, err := apiclient.NewRestAPIClient(url)
		if err != nil {
			log.Errorf("Error connecting to Venice %v. Err: %v", url, err)
			return nil, err
		}

		restcls = append(restcls, restcl)
	}

	return restcls, nil
}

// VeniceNodeRestClient returns the REST client for venice node
func (sm *SysModel) VeniceNodeRestClient(nodeURL string) (apiclient.Services, error) {
	// connect to Venice
	restcl, err := apiclient.NewRestAPIClient(nodeURL)
	if err != nil {
		log.Errorf("Error connecting to Venice %v. Err: %v", nodeURL, err)
		return nil, err
	}
	return restcl, nil
}

// WaitForVeniceClusterUp wait for venice cluster to come up
func (sm *SysModel) WaitForVeniceClusterUp(ctx context.Context) error {
	// wait for cluster to come up
	for i := 0; i < maxVeniceUpWait; i++ {
		restcls, err := sm.VeniceRestClient()
		if err == nil {
			ctx2, err := sm.VeniceLoggedInCtx(ctx)
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

// InitVeniceConfig initializes base configuration for venice
func (sm *SysModel) InitVeniceConfig(ctx context.Context) error {
	// base configs
	/*cfgMsg := &iota.InitConfigMsg{
		ApiResponse:    &iota.IotaAPIResponse{},
		EntryPointType: iota.EntrypointType_VENICE_REST,
		Endpoints:      sm.GetVeniceURL(),
		Vlans:          sm.Tb.allocatedVlans,
	}

	// Push base configs
	cfgClient := iota.NewConfigMgmtApiClient(sm.Tb.iotaClient.Client)
	cfgInitResp, err := cfgClient.InitCfgService(ctx, cfgMsg)
	if err != nil || cfgInitResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Config service Init failed. API Status: %v , Err: %v", cfgInitResp.ApiResponse.ApiStatus, err)
		return fmt.Errorf("Config service init failed")
	}
	log.Debugf("Got init config Resp: %+v", cfgInitResp)*/

	log.Infof("Setting up Auth on Venice cluster...")

	var err error
	for i := 0; i < maxVeniceUpWait; i++ {
		err = sm.SetupAuth("admin", common.UserPassword)
		if err == nil {
			break
		}
		time.Sleep(time.Second)
		if ctx.Err() != nil {
			return ctx.Err()
		}
	}
	if err != nil {
		log.Errorf("Setting up Auth failed. Err: %v", err)
		return err
	}

	log.Infof("Auth setup complete...")

	// wait for venice cluster to come up
	return sm.WaitForVeniceClusterUp(ctx)
}

// SetupAuth bootstraps default tenant, authentication policy, local user and super admin role
func (sm *SysModel) SetupAuth(userID, password string) error {
	// no need to setup auth in mock mode
	if sm.Tb.IsMockMode() {
		return nil
	}

	apicl, err := apiclient.NewRestAPIClient(sm.GetVeniceURL()[0])
	if err != nil {
		return fmt.Errorf("cannot create rest client, err: %v", err)
	}

	// create tenant. default roles (admin role) are created automatically when a tenant is created
	_, err = testutils.CreateTenant(apicl, globals.DefaultTenant)
	if err != nil {
		// 412 is returned when tenant and default roles already exist. 401 when auth is already bootstrapped. we are ok with that
		// already exists
		if !strings.HasPrefix(err.Error(), "Status:(412)") && !strings.HasPrefix(err.Error(), "Status:(401)") &&
			!strings.HasPrefix(err.Error(), "already exists") {
			return fmt.Errorf("CreateTenant failed with err: %v", err)
		}
	}

	// create authentication policy with local auth enabled
	_, err = testutils.CreateAuthenticationPolicyWithOrder(apicl, &auth.Local{}, nil, nil, []string{auth.Authenticators_LOCAL.String()}, testutils.ExpiryDuration)
	if err != nil {
		// 409 is returned when authpolicy already exists. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(409)") && !strings.HasPrefix(err.Error(), "Status:(401)") &&
			!strings.HasPrefix(err.Error(), "already exists") {
			return fmt.Errorf("CreateAuthenticationPolicy failed with err: %v", err)
		}
	}

	// create user is only allowed after auth policy is created and local auth is enabled
	_, err = testutils.CreateTestUser(context.TODO(), apicl, userID, password, globals.DefaultTenant)
	if err != nil {
		// 409 is returned when user already exists. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(409)") && !strings.HasPrefix(err.Error(), "Status:(401)") &&
			!strings.HasPrefix(err.Error(), "already exists") {
			return fmt.Errorf("CreateTestUser failed with err: %v", err)
		}
	}

	// update admin role binding
	_, err = testutils.UpdateRoleBinding(context.TODO(), apicl, globals.AdminRoleBinding, globals.DefaultTenant, globals.AdminRole, []string{userID}, nil)
	if err != nil {
		// 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(401)") {
			return fmt.Errorf("UpdateRoleBinding failed with err: %v", err)
		}
	}

	// set bootstrap flag
	_, err = testutils.SetAuthBootstrapFlag(apicl)
	if err != nil {
		// 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(401)") {
			ginkgo.Fail(fmt.Sprintf("SetAuthBootstrapFlag failed with err: %v", err))
		}
	}

	return nil
}

/*
// CreateHost creates host object in venice
func (sm *SysModel) CreateHost(host *cluster.Host) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}

	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.ClusterV1().Host().Create(ctx, host)
		if err == nil {
			break
		} else if strings.Contains(err.Error(), "already exists") {
			_, err = restcl.ClusterV1().Host().Update(ctx, host)
			if err == nil {
				break
			}
		}
	}
	return err
}

// ListHost gets all hosts from venice cluster
func (sm *SysModel) ListHost() (objs []*cluster.Host, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}}

	for _, restcl := range restcls {
		objs, err = restcl.ClusterV1().Host().List(ctx, &opts)
		if err == nil {
			break
		}
	}

	return objs, err
}

//DeleteHost deletes host object
func (sm *SysModel) DeleteHost(wrkld *cluster.Host) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.ClusterV1().Host().Delete(ctx, &wrkld.ObjectMeta)
		if err == nil {
			break
		} else {
			log.Errorf("Error deleting object %v", err)
		}
	}
	return err
}

func (sm *SysModel) workloadOPer(wrklds []*workload.Workload, opFunc workfarm.WorkFunc) error {

	numOfWorkers := 50

	wctx := &workCtx{
		workloads: wrklds,
		sm:        sm,
	}

	wctx.restCls = make(map[int][]apiclient.Services)
	for i := 0; i < numOfWorkers; i++ {
		restcls, err := wctx.sm.NewVeniceRestClient()
		if err != nil {
			return err
		}
		wctx.restCls[i] = restcls
	}

	defer func() {
		for _, restClients := range wctx.restCls {
			for _, restClient := range restClients {
				go restClient.Close()
			}
		}
	}()

	farm := workfarm.New(int(numOfWorkers), time.Minute*20, opFunc)

	log.Infof("Number of workers %v", numOfWorkers)
	ch, err := farm.Run(context.Background(), len(wrklds), 0, math.MaxUint32, wctx)
	if err != nil {
		fmt.Printf("failed to start work (%s)\n", err)
	}

	rslts := <-ch

	if rslts.WorkerErrors != 0 {
		return fmt.Errorf("Workload create failed stats %+v", rslts)
	}
	return nil
}

// CreateWorkloads creates workloads
func (sm *SysModel) CreateWorkloads(wrklds []*workload.Workload) error {
	return sm.workloadOPer(wrklds, workloadWork)
}

// DeleteWorkloads deletes workload
func (sm *SysModel) DeleteWorkloads(wrklds []*workload.Workload) error {
	return sm.workloadOPer(wrklds, workloadDeleteWork)
}

// CreateWorkload creates workload
func (sm *SysModel) CreateWorkload(wrkld *workload.Workload) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.WorkloadV1().Workload().Create(ctx, wrkld)
		if err == nil {
			break
		} else if strings.Contains(err.Error(), "already exists") {
			_, err = restcl.WorkloadV1().Workload().Update(ctx, wrkld)
			if err == nil {
				break
			}
		}
	}
	return err
}

// GetWorkload returns venice workload by object meta
func (sm *SysModel) GetWorkload(meta *api.ObjectMeta) (w *workload.Workload, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	for _, restcl := range restcls {
		w, err = restcl.WorkloadV1().Workload().Get(ctx, meta)
		if err == nil {
			break
		}
	}

	return w, err
}

//DeleteWorkload deletes workload
func (sm *SysModel) DeleteWorkload(wrkld *workload.Workload) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.WorkloadV1().Workload().Delete(ctx, &wrkld.ObjectMeta)
		if err == nil {
			break
		}
	}
	return err
}

// ListWorkload gets all workloads from venice cluster
func (sm *SysModel) ListWorkload() (objs []*workload.Workload, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}}

	for _, restcl := range restcls {
		objs, err = restcl.WorkloadV1().Workload().List(ctx, &opts)
		if err == nil {
			break
		}
	}

	return objs, err
}


// CreateMirrorSession creates Mirror policy
func (sm *SysModel) CreateMirrorSession(msp *monitoring.MirrorSession) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.MonitoringV1().MirrorSession().Create(ctx, msp)
		if err == nil {
			break
		} else if strings.Contains(err.Error(), "already exists") {
			_, err = restcl.MonitoringV1().MirrorSession().Update(ctx, msp)
			if err == nil {
				break
			}
		}
	}

	return err
}

// UpdateMirrorSession updates an Mirror policy
func (sm *SysModel) UpdateMirrorSession(msp *monitoring.MirrorSession) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.MonitoringV1().MirrorSession().Update(ctx, msp)
		if err == nil {
			break
		}
	}
	return err
}

// UpdateSmartNIC updates an SmartNIC object
func (sm *SysModel) UpdateSmartNIC(sn *cluster.DistributedServiceCard) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.ClusterV1().DistributedServiceCard().Update(ctx, sn)
		if err == nil {
			break
		}
	}
	return err
}

// GetMirrorSession gets MirrorSession from venice cluster
func (sm *SysModel) GetMirrorSession(meta *api.ObjectMeta) (msp *monitoring.MirrorSession, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	for _, restcl := range restcls {
		msp, err = restcl.MonitoringV1().MirrorSession().Get(ctx, meta)
		if err == nil {
			break
		}
	}

	return msp, err
}

// ListMirrorSession gets all MirrorPolicies from venice cluster
func (sm *SysModel) ListMirrorSession() (objs []*monitoring.MirrorSession, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}}

	for _, restcl := range restcls {
		objs, err = restcl.MonitoringV1().MirrorSession().List(ctx, &opts)
		if err == nil {
			break
		}
	}

	return objs, err
}

// DeleteMirrorSession deletes Mirror policy
func (sm *SysModel) DeleteMirrorSession(msp *monitoring.MirrorSession) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.MonitoringV1().MirrorSession().Delete(ctx, &msp.ObjectMeta)
		if err == nil {
			break
		}
	}

	return err
}

/*
// CreateNetworkSecurityPolicy creates SG policy
func (sm *SysModel) CreateNetworkSecurityPolicy(sgp *security.NetworkSecurityPolicy) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().NetworkSecurityPolicy().Create(ctx, sgp)
		if err == nil {
			break
		} else if strings.Contains(err.Error(), "already exists") {
			_, err = restcl.SecurityV1().NetworkSecurityPolicy().Update(ctx, sgp)
			if err == nil {
				break
			}
		}
	}

	return err
}

// UpdateNetworkSecurityPolicy updates an SG policy
func (sm *SysModel) UpdateNetworkSecurityPolicy(sgp *security.NetworkSecurityPolicy) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().NetworkSecurityPolicy().Update(ctx, sgp)
		if err == nil {
			break
		}
	}
	return err
}

// GetNetworkSecurityPolicy gets NetworkSecurityPolicy from venice cluster
func (sm *SysModel) GetNetworkSecurityPolicy(meta *api.ObjectMeta) (sgp *security.NetworkSecurityPolicy, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	for _, restcl := range restcls {
		sgp, err = restcl.SecurityV1().NetworkSecurityPolicy().Get(ctx, meta)
		if err == nil {
			break
		}
	}

	return sgp, err
}

//GetNpmDebugModuleURLs gets npm debug module
func (sm *SysModel) GetNpmDebugModuleURLs() (urls []string, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	for _, restcl := range restcls {
		data, err := restcl.DiagnosticsV1().Module().List(ctx, &api.ListWatchOptions{})
		if err == nil {
			for _, module := range data {
				if strings.Contains(module.ObjectMeta.Name, "pen-npm") {
					for _, veniceURL := range sm.GetVeniceURL() {
						urls = append(urls, "https://"+veniceURL+module.GetSelfLink()+"/Debug")
					}
				}
			}
		}
	}

	if len(urls) == 0 {
		return nil, errors.New("Could not find NPM debug URL")
	}
	return urls, nil
}

func (sm *SysModel) doConfigPostAction(action string, configStatus interface{}) error {

	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}

	npmURLs, err := sm.GetNpmDebugModuleURLs()
	if err != nil {
		return errors.New("Npm debug URL not found")
	}

	req := &diagnostics.DiagnosticsRequest{
		Query:      "action",
		Parameters: map[string]string{"action": action}}

	restcl := netutils.NewHTTPClient()
	restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	restcl.DisableKeepAlives()
	defer restcl.CloseIdleConnections()

	// get authz header
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return fmt.Errorf("no authorization header in context")
	}
	restcl.SetHeader("Authorization", authzHeader)
	for _, url := range npmURLs {
		_, err = restcl.Req("POST", url, req, configStatus)
		if err == nil {
			return nil
		}
		fmt.Printf("Error in request %+v\n", err)

	}

	return fmt.Errorf("Failed Request for config push : %v", err)
}

//PullConfigStatus pulls config status
func (sm *SysModel) PullConfigStatus(configStatus interface{}) error {

	return sm.doConfigPostAction("config-status", configStatus)
}

//PullConfigStats pulls config stats
func (sm *SysModel) PullConfigStats(configStats interface{}) error {

	return sm.doConfigPostAction("config-stats", configStats)
}

//ResetConfigStats reset config stats
func (sm *SysModel) resetConfigStats(configReset interface{}) error {

	return sm.doConfigPostAction("reset-stats", configReset)
}

// ListNetworkSecurityPolicy gets all SGPolicies from venice cluster
func (sm *SysModel) ListNetworkSecurityPolicy() (objs []*security.NetworkSecurityPolicy, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}}

	for _, restcl := range restcls {
		objs, err = restcl.SecurityV1().NetworkSecurityPolicy().List(ctx, &opts)
		if err == nil {
			break
		}
	}

	return objs, err
}

// DeleteNetworkSecurityPolicy deletes SG policy
func (sm *SysModel) DeleteNetworkSecurityPolicy(sgp *security.NetworkSecurityPolicy) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().NetworkSecurityPolicy().Delete(ctx, &sgp.ObjectMeta)
		if err == nil {
			break
		} else {
			log.Errorf("Error deleting object %v", err)
		}
	}

	return err
}

// GetCluster gets the venice cluster object
func (sm *SysModel) GetCluster() (cl *cluster.Cluster, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	for _, restcl := range restcls {
		cl, err = restcl.ClusterV1().Cluster().Get(ctx, &api.ObjectMeta{Name: "iota-cluster"})
		if err == nil {
			break
		}
	}

	return cl, err
}

// GetClusterWithRestClient gets the venice cluster object
func (sm *SysModel) GetClusterWithRestClient(restcl apiclient.Services) (cl *cluster.Cluster, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	return restcl.ClusterV1().Cluster().Get(ctx, &api.ObjectMeta{Name: "iota-cluster"})
}

// TakeConfigSnapshot preforms a snapshot operation
func (sm *SysModel) ConfigureSnapshot(restcl apiclient.Services) error {
	cfg := &cluster.ConfigurationSnapshot{
		ObjectMeta: api.ObjectMeta{
			Name: "GlobalSnapshotConfig",
		},
		Spec: cluster.ConfigurationSnapshotSpec{
			Destination: cluster.SnapshotDestination{
				Type: cluster.SnapshotDestinationType_ObjectStore.String(),
			},
		},
	}
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	_, err = restcl.ClusterV1().ConfigurationSnapshot().Create(ctx, cfg)
	if err != nil {
		_, err = restcl.ClusterV1().ConfigurationSnapshot().Update(ctx, cfg)
	}
	return err
}

// TakeConfigSnapshot preforms a snapshot operation
func (sm *SysModel) TakeConfigSnapshot(restcl apiclient.Services, reqname string) (uri string, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return "", err
	}
	req := &cluster.ConfigurationSnapshotRequest{}
	req.Name = reqname
	_, err = restcl.ClusterV1().ConfigurationSnapshot().Save(ctx, req)
	if err != nil {
		return "", err
	}
	snaps, err := restcl.ClusterV1().ConfigurationSnapshot().Get(ctx, &api.ObjectMeta{})
	if err != nil {
		return "", err
	}
	return snaps.Status.LastSnapshot.URI, nil
}

// RestoreConfig restores config to snapshot specified in filename
func (sm *SysModel) RestoreConfig(restcl apiclient.Services, filename string) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	req := &cluster.SnapshotRestore{
		ObjectMeta: api.ObjectMeta{
			Name: "IOTARestoreOp",
		},
		Spec: cluster.SnapshotRestoreSpec{
			SnapshotPath: filename,
		},
	}

	resp, err := restcl.ClusterV1().SnapshotRestore().Restore(ctx, req)
	if err != nil {
		return err
	}
	if resp.Status.Status != cluster.SnapshotRestoreStatus_Completed.String() {
		return errors.New("Restore operation did not complete")
	}
	return nil
}
*/

// GetVeniceNode gets venice node state from venice cluster
func (sm *SysModel) GetVeniceNode(name string) (n *cluster.Node, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	meta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      name,
	}

	for _, restcl := range restcls {
		n, err = restcl.ClusterV1().Node().Get(ctx, &meta)
		if err == nil {
			break
		}
	}

	return n, err
}

/*
// GetSmartNIC returns venice smartnic object
func (sm *SysModel) GetSmartNIC(name string) (sn *cluster.DistributedServiceCard, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	meta := api.ObjectMeta{
		Name: name,
	}

	for _, restcl := range restcls {
		sn, err = restcl.ClusterV1().DistributedServiceCard().Get(ctx, &meta)
		if err == nil {
			break
		}
	}

	return sn, err
}

type workCtx struct {
	workloads []*workload.Workload
	sm        *SysModel
	restCls   map[int][]apiclient.Services
}

func workloadWork(ctx context.Context, id, iter int, userCtx interface{}) error {

	wctx := userCtx.(*workCtx)
	ctx, err := wctx.sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}

	if restcls, ok := wctx.restCls[id]; ok {
		for _, restcl := range restcls {
			_, err = restcl.WorkloadV1().Workload().Create(ctx, wctx.workloads[iter])
			if err == nil {
				break
			} else if strings.Contains(err.Error(), "already exists") {
				_, err = restcl.WorkloadV1().Workload().Update(ctx, wctx.workloads[iter])
				if err == nil {
					break
				}
			}
		}
	}

	if err != nil {
		log.Errorf("Workload create %v failed  with error %v", wctx.workloads[iter], err.Error())
	}
	return err
}

func workloadDeleteWork(ctx context.Context, id, iter int, userCtx interface{}) error {

	wctx := userCtx.(*workCtx)
	ctx, err := wctx.sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	if restcls, ok := wctx.restCls[id]; ok {
		for _, restcl := range restcls {
			_, err = restcl.WorkloadV1().Workload().Delete(ctx, &wctx.workloads[iter].ObjectMeta)
			if err == nil {
				break
			}
		}
		if err != nil {
			log.Errorf("Workload  delete %v failed  with error %v", wctx.workloads[iter], err.Error())
			return err
		}
	}
	return err
}

// ListSmartNIC gets a list of smartnics
func (sm *SysModel) ListSmartNIC() (snl []*cluster.DistributedServiceCard, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
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

// ListClusterNodes gets a list of nodes
func (sm *SysModel) ListClusterNodes() (snl []*cluster.Node, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	opts := api.ListWatchOptions{}

	for _, restcl := range restcls {
		snl, err = restcl.ClusterV1().Node().List(ctx, &opts)
		if err == nil {
			break
		}
	}
	return snl, err
}

// DeleteClusterNode gets a list of nodes
func (sm *SysModel) DeleteClusterNode(node *cluster.Node) (err error) {
	log.Info("Getting loggd in context")
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		log.Errorf("Error getting login ctx %v", err)
		return err
	}
	log.Info("Getting  rest client in context")
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		log.Errorf("Error getting rest client %v", err)
		return err
	}

	log.Info("Initiating delete..")
	for _, restcl := range restcls {
		_, err = restcl.ClusterV1().Node().Delete(ctx, &node.ObjectMeta)
		if err == nil {
			break
		}
	}
	log.Info("Initiating deleted competed..")
	if err != nil {
		log.Errorf("Error deleting cluster nodeß %v", err)
	}
	return err
}

// AddClusterNode gets a list of nodes
func (sm *SysModel) AddClusterNode(node *cluster.Node) (err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		newNode := &cluster.Node{
			ObjectMeta: api.ObjectMeta{
				Name: node.ObjectMeta.Name,
			},
		}
		_, err = restcl.ClusterV1().Node().Create(ctx, newNode)
		if err == nil {
			break
		}
	}
	return err
}

// GetSmartNICInMacRange returns a smartnic object in mac address range
func (sm *SysModel) GetSmartNICInMacRange(macAddr string) (sn *cluster.DistributedServiceCard, err error) {
	const maxMacDiff = 24
	snicList, err := sm.ListSmartNIC()
	if err != nil {
		return nil, err
	}

	// walk all smartnics and see if the mac addr range matches
	for _, snic := range snicList {
		snicMacNum := macAddrToUint64(snic.Status.PrimaryMAC)
		reqMacNum := macAddrToUint64(macAddr)
		if (snicMacNum == reqMacNum) || ((reqMacNum - snicMacNum) < maxMacDiff) {
			return snic, nil
		}
	}

	return nil, fmt.Errorf("Could not find smartnic with mac addr %s", macAddr)
}

func (sm *SysModel) GetSmartNICByName(snicName string) (sn *cluster.DistributedServiceCard, err error) {
	snicList, err := sm.ListSmartNIC()
	if err != nil {
		return nil, err
	}

	// walk all smartnics and see if the mac addr range matches
	for _, snic := range snicList {
		if snic.Spec.ID == snicName {
			return snic, nil
		}
	}

	return nil, fmt.Errorf("Could not find smartnic with name %s", snicName)
}

// GetEndpoint returns the endpoint
func (sm *SysModel) GetEndpoint(meta *api.ObjectMeta) (ep *workload.Endpoint, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	for _, restcl := range restcls {
		ep, err = restcl.WorkloadV1().Endpoint().Get(ctx, meta)
		if err == nil {
			break
		}
	}
	return ep, err
}

// ListEndpoints returns list of endpoints known to Venice
func (sm *SysModel) ListEndpoints(tenant string) (eps []*workload.Endpoint, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	opts := api.ListWatchOptions{}
	opts.Tenant = tenant
	for _, restcl := range restcls {
		eps, err = restcl.WorkloadV1().Endpoint().List(ctx, &opts)
		if err == nil {
			break
		}
	}
	return eps, err
}

// UpdateFirewallProfile updates firewall profile
func (sm *SysModel) UpdateFirewallProfile(fwp *security.FirewallProfile) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().FirewallProfile().Update(ctx, fwp)
		if err == nil {
			break
		} else if strings.Contains(err.Error(), "already exists") {
			_, err = restcl.SecurityV1().FirewallProfile().Update(ctx, fwp)
			if err == nil {
				break
			}
		}
	}

	return err
}

// ListFirewallProfile gets all fw profile apps from venice cluster
func (sm *SysModel) ListFirewallProfile() (objs []*security.FirewallProfile, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}}

	for _, restcl := range restcls {
		objs, err = restcl.SecurityV1().FirewallProfile().List(ctx, &opts)
		if err == nil {
			break
		}
	}

	return objs, err
}

// DeleteFirewallProfile deletes FirewallProfile object
func (sm *SysModel) DeleteFirewallProfile(fwprofile *security.FirewallProfile) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().FirewallProfile().Delete(ctx, &fwprofile.ObjectMeta)
		if err == nil {
			break
		}
	}

	return err
}

// CreateApp creates an app in venice
func (sm *SysModel) CreateApp(app *security.App) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().App().Create(ctx, app)
		if err == nil {
			break
		} else if strings.Contains(err.Error(), "already exists") {
			_, err = restcl.SecurityV1().App().Update(ctx, app)
			if err == nil {
				break
			}
		}
	}

	return err
}

// ListApp gets all apps from venice cluster
func (sm *SysModel) ListApp() (objs []*security.App, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}}

	for _, restcl := range restcls {
		objs, err = restcl.SecurityV1().App().List(ctx, &opts)
		if err == nil {
			break
		}
	}

	return objs, err
}

// DeleteApp deletes App object
func (sm *SysModel) DeleteApp(app *security.App) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().App().Delete(ctx, &app.ObjectMeta)
		if err == nil {
			break
		} else {
			log.Errorf("Error deleting object %v", err)
		}
	}

	return err
}

// CreateNetwork creates an Network in venice
func (sm *SysModel) CreateNetwork(obj *network.Network) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.NetworkV1().Network().Create(ctx, obj)
		if err == nil {
			break
		}
	}

	return err
}

// ListNetwork gets all networks from venice cluster
func (sm *SysModel) ListNetwork() (objs []*network.Network, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}}

	for _, restcl := range restcls {
		objs, err = restcl.NetworkV1().Network().List(ctx, &opts)
		if err == nil {
			break
		}
	}

	return objs, err
}

// DeleteNetwork deletes Network object
func (sm *SysModel) DeleteNetwork(net *network.Network) error {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.NetworkV1().Network().Delete(ctx, &net.ObjectMeta)
		if err == nil {
			break
		}
	}

	return err
}
*/

// ListEvents makes a http request to the APIGw with the given list watch options and returns the response
func (sm *SysModel) ListEvents(listWatchOptions *api.ListWatchOptions) (evtsapi.EventList, error) {
	resp := evtsapi.EventList{}
	authzHdr, err := sm.GetAuthorizationHeader()
	if err != nil {
		return resp, err
	}
	httpClient := netutils.NewHTTPClient()
	httpClient.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	httpClient.SetHeader("Authorization", authzHdr)
	httpClient.DisableKeepAlives()
	defer httpClient.CloseIdleConnections()

	URL := fmt.Sprintf("https://%s/events/v1/events", sm.GetVeniceURL()[0])
	_, err = httpClient.Req("GET", URL, *listWatchOptions, &resp)
	return resp, err
}

/*
//StagingBuffer
type StagingBuffer struct {
	sm      *SysModel
	name    string
	tenant  string
	ctx     context.Context
	stagecl apiclient.Services
}

//NewStagingBuffer buffer handle
func (sm *SysModel) NewStagingBuffer() (*StagingBuffer, error) {
	buffer := &StagingBuffer{name: "TestBuffer", tenant: "default"}

	urls := sm.GetVeniceURL()

	restcl, err := apiclient.NewRestAPIClient(urls[0])
	if err != nil {
		return nil, err
	}

	ctx2, err := sm.VeniceLoggedInCtx(context.Background())
	{ // Cleanup existing
		objMeta := api.ObjectMeta{Name: buffer.name, Tenant: buffer.tenant}
		restcl.StagingV1().Buffer().Delete(ctx2, &objMeta)
	}
	fmt.Printf("creating Staging Buffer\n")
	{ // Create a buffer
		buf := staging.Buffer{}
		buf.Name = buffer.name
		buf.Tenant = buffer.tenant
		_, err := restcl.StagingV1().Buffer().Create(ctx2, &buf)
		if err != nil {
			fmt.Printf("*** Failed to create Staging Buffer(%s)\n", err)
			return nil, err
		}
	}

	// Staging Client
	stagecl, err := apiclient.NewStagedRestAPIClient(urls[0], buffer.name)
	if err != nil {
		return nil, err
	}

	buffer.stagecl = stagecl
	buffer.sm = sm
	buffer.ctx = ctx2

	return buffer, nil
}

//AddHost to statging buffer
func (buf *StagingBuffer) AddHost(host *cluster.Host) error {

	_, err := buf.stagecl.ClusterV1().Host().Create(buf.ctx, host)
	return err
}

//AddApp to statging buffer
func (buf *StagingBuffer) AddApp(app *security.App) error {

	_, err := buf.stagecl.SecurityV1().App().Create(buf.ctx, app)
	return err
}

//AddWorkload to statging buffer
func (buf *StagingBuffer) AddWorkload(workload *workload.Workload) error {

	_, err := buf.stagecl.WorkloadV1().Workload().Create(buf.ctx, workload)
	return err
}

//AddNetowrkSecurityPolicy to statging buffer
func (buf *StagingBuffer) AddNetowrkSecurityPolicy(policy *security.NetworkSecurityPolicy) error {

	_, err := buf.stagecl.SecurityV1().NetworkSecurityPolicy().Create(buf.ctx, policy)
	return err
}

//Commit the buffer
func (buf *StagingBuffer) Commit() error {

	urls := buf.sm.GetVeniceURL()

	restcl, err := apiclient.NewRestAPIClient(urls[0])
	if err != nil {
		return err
	}

	ctx2, err := buf.sm.VeniceLoggedInCtx(context.Background())

	action := &staging.CommitAction{}
	action.Name = buf.name
	action.Tenant = buf.tenant
	_, err = restcl.StagingV1().Buffer().Commit(ctx2, action)
	if err != nil {
		fmt.Printf("*** Failed to commit Staging Buffer(%s)\n", err)
		return err
	}
	return nil
}

*/
