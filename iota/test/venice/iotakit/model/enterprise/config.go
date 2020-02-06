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
		if sm.AuthToken == "" {
			_, err = sm.veniceNodeLoggedInCtxWithURL(url)
		}
		if err == nil {
			break
		}
	}
	if err != nil {
		return nil, err
	}
	return loginctx.NewContextWithAuthzHeader(ctx, sm.AuthToken), nil

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
		sm.AuthToken = authToken
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
