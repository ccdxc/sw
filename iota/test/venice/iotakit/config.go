// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

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
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	loginctx "github.com/pensando/sw/api/login/context"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

const maxVeniceUpWait = 300

// VeniceLoggedInCtx returns loggedin context for venice taking a context
func (tb *TestBed) VeniceLoggedInCtx(ctx context.Context) (context.Context, error) {
	var err error

	if tb.authToken == "" {
		_, err = tb.VeniceNodeLoggedInCtx(tb.GetVeniceURL()[0])
	}
	if err != nil {
		return nil, err
	}
	return loginctx.NewContextWithAuthzHeader(ctx, tb.authToken), nil
}

// VeniceNodeLoggedInCtx logs in to a specified node and returns loggedin context
func (tb *TestBed) VeniceNodeLoggedInCtx(nodeURL string) (context.Context, error) {
	// local user credentials
	userCred := auth.PasswordCredential{
		Username: "admin",
		Password: common.UserPassword,
		Tenant:   "default",
	}

	// overwrite user-id/password in mock mode to match venice integ user cred
	if tb.mockMode {
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
		tb.authToken = authToken
	} else {
		return nil, fmt.Errorf("auth token not available in logged-in context")
	}

	return ctx, nil
}

// GetAuthorizationHeader gets and returns the authorization header from login context
func (tb *TestBed) GetAuthorizationHeader() (string, error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
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
func (tb *TestBed) VeniceRestClient() ([]apiclient.Services, error) {
	// if we are already connected, just return the client
	if tb.veniceRestClient != nil {
		return tb.veniceRestClient, nil
	}

	var restcls []apiclient.Services
	for _, url := range tb.GetVeniceURL() {
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
func (tb *TestBed) VeniceNodeRestClient(nodeURL string) (apiclient.Services, error) {
	// connect to Venice
	restcl, err := apiclient.NewRestAPIClient(nodeURL)
	if err != nil {
		log.Errorf("Error connecting to Venice %v. Err: %v", nodeURL, err)
		return nil, err
	}
	return restcl, nil
}

// WaitForVeniceClusterUp wait for venice cluster to come up
func (tb *TestBed) WaitForVeniceClusterUp(ctx context.Context) error {
	// wait for cluster to come up
	for i := 0; i < maxVeniceUpWait; i++ {
		restcls, err := tb.VeniceRestClient()
		if err == nil {
			ctx2, err := tb.VeniceLoggedInCtx(ctx)
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
func (tb *TestBed) InitVeniceConfig(ctx context.Context) error {
	// base configs
	cfgMsg := &iota.InitConfigMsg{
		ApiResponse:    &iota.IotaAPIResponse{},
		EntryPointType: iota.EntrypointType_VENICE_REST,
		Endpoints:      tb.GetVeniceURL(),
		Vlans:          tb.allocatedVlans,
	}

	// Push base configs
	cfgClient := iota.NewConfigMgmtApiClient(tb.iotaClient.Client)
	cfgInitResp, err := cfgClient.InitCfgService(ctx, cfgMsg)
	if err != nil || cfgInitResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Config service Init failed. API Status: %v , Err: %v", cfgInitResp.ApiResponse.ApiStatus, err)
		return fmt.Errorf("Config service init failed")
	}
	log.Debugf("Got init config Resp: %+v", cfgInitResp)

	log.Infof("Setting up Auth on Venice cluster...")

	for i := 0; i < maxVeniceUpWait; i++ {
		err = tb.SetupAuth("admin", common.UserPassword)
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
	return tb.WaitForVeniceClusterUp(ctx)
}

// SetupAuth bootstraps default tenant, authentication policy, local user and super admin role
func (tb *TestBed) SetupAuth(userID, password string) error {
	// no need to setup auth in mock mode
	if tb.mockMode {
		return nil
	}

	apicl, err := apiclient.NewRestAPIClient(tb.GetVeniceURL()[0])
	if err != nil {
		return fmt.Errorf("cannot create rest client, err: %v", err)
	}

	// create tenant. default roles (admin role) are created automatically when a tenant is created
	_, err = testutils.CreateTenant(apicl, globals.DefaultTenant)
	if err != nil {
		// 412 is returned when tenant and default roles already exist. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(412)") && !strings.HasPrefix(err.Error(), "Status:(401)") {
			return fmt.Errorf("CreateTenant failed with err: %v", err)
		}
	}

	// create authentication policy with local auth enabled
	_, err = testutils.CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false})
	if err != nil {
		// 409 is returned when authpolicy already exists. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(409)") && !strings.HasPrefix(err.Error(), "Status:(401)") {
			return fmt.Errorf("CreateAuthenticationPolicy failed with err: %v", err)
		}
	}

	// create user is only allowed after auth policy is created and local auth is enabled
	_, err = testutils.CreateTestUser(context.TODO(), apicl, userID, password, globals.DefaultTenant)
	if err != nil {
		// 409 is returned when user already exists. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(409)") && !strings.HasPrefix(err.Error(), "Status:(401)") {
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

// CreateHost creates host object in venice
func (tb *TestBed) CreateHost(host *cluster.Host) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}

	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) ListHost() (objs []*cluster.Host, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) DeleteHost(wrkld *cluster.Host) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.ClusterV1().Host().Delete(ctx, &wrkld.ObjectMeta)
		if err == nil {
			break
		}
	}
	return err
}

// CreateWorkload creates workload
func (tb *TestBed) CreateWorkload(wrkld *workload.Workload) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) GetWorkload(meta *api.ObjectMeta) (w *workload.Workload, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) DeleteWorkload(wrkld *workload.Workload) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) ListWorkload() (objs []*workload.Workload, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) CreateMirrorSession(msp *monitoring.MirrorSession) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) UpdateMirrorSession(msp *monitoring.MirrorSession) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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

// GetMirrorSession gets MirrorSession from venice cluster
func (tb *TestBed) GetMirrorSession(meta *api.ObjectMeta) (msp *monitoring.MirrorSession, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) ListMirrorSession() (objs []*monitoring.MirrorSession, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) DeleteMirrorSession(msp *monitoring.MirrorSession) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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

// CreateNetworkSecurityPolicy creates SG policy
func (tb *TestBed) CreateNetworkSecurityPolicy(sgp *security.NetworkSecurityPolicy) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) UpdateNetworkSecurityPolicy(sgp *security.NetworkSecurityPolicy) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) GetNetworkSecurityPolicy(meta *api.ObjectMeta) (sgp *security.NetworkSecurityPolicy, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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

// ListNetworkSecurityPolicy gets all SGPolicies from venice cluster
func (tb *TestBed) ListNetworkSecurityPolicy() (objs []*security.NetworkSecurityPolicy, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) DeleteNetworkSecurityPolicy(sgp *security.NetworkSecurityPolicy) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().NetworkSecurityPolicy().Delete(ctx, &sgp.ObjectMeta)
		if err == nil {
			break
		}
	}

	return err
}

// GetCluster gets the venice cluster object
func (tb *TestBed) GetCluster() (cl *cluster.Cluster, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) GetClusterWithRestClient(restcl apiclient.Services) (cl *cluster.Cluster, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	return restcl.ClusterV1().Cluster().Get(ctx, &api.ObjectMeta{Name: "iota-cluster"})
}

// GetVeniceNode gets venice node state from venice cluster
func (tb *TestBed) GetVeniceNode(name string) (n *cluster.Node, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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

// GetSmartNIC returns venice smartnic object
func (tb *TestBed) GetSmartNIC(name string) (sn *cluster.DistributedServiceCard, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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

// ListSmartNIC gets a list of smartnics
func (tb *TestBed) ListSmartNIC() (snl []*cluster.DistributedServiceCard, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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

// GetSmartNICInMacRange returns a smartnic object in mac address range
func (tb *TestBed) GetSmartNICInMacRange(macAddr string) (sn *cluster.DistributedServiceCard, err error) {
	const maxMacDiff = 24
	snicList, err := tb.ListSmartNIC()
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

// GetSmartNICByName returns a smartnic object by its name
func (tb *TestBed) GetSmartNICByName(snicName string) (sn *cluster.DistributedServiceCard, err error) {
	snicList, err := tb.ListSmartNIC()
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
func (tb *TestBed) GetEndpoint(meta *api.ObjectMeta) (ep *workload.Endpoint, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) ListEndpoints(tenant string) (eps []*workload.Endpoint, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) UpdateFirewallProfile(fwp *security.FirewallProfile) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) ListFirewallProfile() (objs []*security.FirewallProfile, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) DeleteFirewallProfile(fwprofile *security.FirewallProfile) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) CreateApp(app *security.App) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) ListApp() (objs []*security.App, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) DeleteApp(app *security.App) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().App().Delete(ctx, &app.ObjectMeta)
		if err == nil {
			break
		}
	}

	return err
}

// CreateNetwork creates an Network in venice
func (tb *TestBed) CreateNetwork(obj *network.Network) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) ListNetwork() (objs []*network.Network, err error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
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
func (tb *TestBed) DeleteNetwork(net *network.Network) error {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
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

// ListEvents makes a http request to the APIGw with the given list watch options and returns the response
func (tb *TestBed) ListEvents(listWatchOptions *api.ListWatchOptions) (evtsapi.EventList, error) {
	resp := evtsapi.EventList{}
	authzHdr, err := tb.GetAuthorizationHeader()
	if err != nil {
		return resp, err
	}
	httpClient := netutils.NewHTTPClient()
	httpClient.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	httpClient.SetHeader("Authorization", authzHdr)
	httpClient.DisableKeepAlives()
	defer httpClient.CloseIdleConnections()

	URL := fmt.Sprintf("https://%s/events/v1/events", tb.GetVeniceURL()[0])
	_, err = httpClient.Req("GET", URL, *listWatchOptions, &resp)
	return resp, err
}
