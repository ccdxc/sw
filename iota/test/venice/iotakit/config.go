// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"fmt"
	"strings"
	"time"

	"github.com/onsi/ginkgo"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/log"
)

// VeniceLoggedInCtx returns loggedin context for venice
func (tb *TestBed) VeniceLoggedInCtx() (context.Context, error) {
	if tb.veniceLoggedinCtx != nil {
		return tb.veniceLoggedinCtx, nil
	}

	// local user credentials
	userCred := auth.PasswordCredential{
		Username: "admin",
		Password: "password",
		Tenant:   "default",
	}

	// overwrite user-id/password in mock mode to match venice integ user cred
	if tb.mockMode {
		userCred.Username = "test"
		userCred.Password = "pensando"
	}

	// try to login
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), tb.GetVeniceURL()[0], &userCred)
	if err != nil {
		log.Errorf("Error logging into Venice URL %v. Err: %v", tb.GetVeniceURL()[0], err)
		return nil, err
	}
	tb.veniceLoggedinCtx = ctx

	return ctx, nil
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

// WaitForVeniceClusterUp wait for venice cluster to come up
func (tb *TestBed) WaitForVeniceClusterUp() error {
	// wait for cluster to come up
	for i := 0; i < 100; i++ {
		restcls, err := tb.VeniceRestClient()
		if err == nil {
			ctx, err := tb.VeniceLoggedInCtx()
			if err == nil {
				for _, restcl := range restcls {
					_, err = restcl.ClusterV1().Cluster().Get(ctx, &api.ObjectMeta{Name: "iota-cluster"})
					if err == nil {
						return nil
					}
				}
			}
		}

		time.Sleep(time.Second)
	}

	// if we reached here, it means we werent able to connect to Venice API GW
	return fmt.Errorf("Failed to connect to Venice")
}

// InitVeniceConfig initializes base configuration for venice
func (tb *TestBed) InitVeniceConfig() error {
	// base configs
	cfgMsg := &iota.InitConfigMsg{
		ApiResponse:    &iota.IotaAPIResponse{},
		EntryPointType: iota.EntrypointType_VENICE_REST,
		Endpoints:      tb.GetVeniceURL(),
		Vlans:          tb.allocatedVlans,
	}

	// Push base configs
	cfgClient := iota.NewConfigMgmtApiClient(tb.iotaClient.Client)
	cfgInitResp, err := cfgClient.InitCfgService(context.Background(), cfgMsg)
	if err != nil || cfgInitResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Config service Init failed. API Status: %v , Err: %v", cfgInitResp.ApiResponse.ApiStatus, err)
		return fmt.Errorf("Config service init failed")
	}
	log.Debugf("Got init config Resp: %+v", cfgInitResp)

	log.Infof("Setting up Auth on Venice cluster...")

	err = tb.SetupAuth("admin", "password")
	if err != nil {
		log.Errorf("Setting up Auth failed. Err: %v", err)
		return err
	}

	log.Infof("Auth setup complete...")

	// wait for venice cluster to come up
	return tb.WaitForVeniceClusterUp()
}

// SetupAuth bootstraps default tenant, authentication policy, local user and super admin role
func (tb *TestBed) SetupAuth(userID, password string) error {
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
	_, err = testutils.CreateTestUser(apicl, userID, password, globals.DefaultTenant)
	if err != nil {
		// 409 is returned when user already exists. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(409)") && !strings.HasPrefix(err.Error(), "Status:(401)") {
			return fmt.Errorf("CreateTestUser failed with err: %v", err)
		}
	}

	// create admin role binding
	_, err = testutils.CreateRoleBinding(context.TODO(), apicl, "AdminRoleBinding", globals.DefaultTenant, globals.AdminRole, []string{userID}, nil)
	if err != nil {
		// 409 is returned when role binding already exists. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(409)") && !strings.HasPrefix(err.Error(), "Status:(401)") {
			return fmt.Errorf("CreateRoleBinding failed with err: %v", err)
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
	ctx, err := tb.VeniceLoggedInCtx()
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

// CreateWorkload creates workload
func (tb *TestBed) CreateWorkload(wrkld *workload.Workload) error {
	ctx, err := tb.VeniceLoggedInCtx()
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
	ctx, err := tb.VeniceLoggedInCtx()
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
	ctx, err := tb.VeniceLoggedInCtx()
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

// CreateSGPolicy creates SG policy
func (tb *TestBed) CreateSGPolicy(sgp *security.SGPolicy) error {
	ctx, err := tb.VeniceLoggedInCtx()
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().SGPolicy().Create(ctx, sgp)
		if err == nil {
			break
		} else if strings.Contains(err.Error(), "already exists") {
			_, err = restcl.SecurityV1().SGPolicy().Update(ctx, sgp)
			if err == nil {
				break
			}
		}
	}

	return err
}

// UpdateSGPolicy updates an SG policy
func (tb *TestBed) UpdateSGPolicy(sgp *security.SGPolicy) error {
	ctx, err := tb.VeniceLoggedInCtx()
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().SGPolicy().Update(ctx, sgp)
		if err == nil {
			break
		}
	}
	return err
}

// GetSGPolicy gets SGPolicy from venice cluster
func (tb *TestBed) GetSGPolicy(meta *api.ObjectMeta) (sgp *security.SGPolicy, err error) {
	ctx, err := tb.VeniceLoggedInCtx()
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	for _, restcl := range restcls {
		sgp, err = restcl.SecurityV1().SGPolicy().Get(ctx, meta)
		if err == nil {
			break
		}
	}

	return sgp, err
}

// DeleteSGPolicy deletes SG policy
func (tb *TestBed) DeleteSGPolicy(sgp *security.SGPolicy) error {
	ctx, err := tb.VeniceLoggedInCtx()
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().SGPolicy().Delete(ctx, &sgp.ObjectMeta)
		if err == nil {
			break
		}
	}

	return err
}

// GetCluster gets the venice cluster object
func (tb *TestBed) GetCluster() (cl *cluster.Cluster, err error) {
	ctx, err := tb.VeniceLoggedInCtx()
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

// GetVeniceNode gets venice node state from venice cluster
func (tb *TestBed) GetVeniceNode(name string) (n *cluster.Node, err error) {
	ctx, err := tb.VeniceLoggedInCtx()
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
func (tb *TestBed) GetSmartNIC(name string) (sn *cluster.SmartNIC, err error) {
	ctx, err := tb.VeniceLoggedInCtx()
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
		sn, err = restcl.ClusterV1().SmartNIC().Get(ctx, &meta)
		if err == nil {
			break
		}
	}

	return sn, err
}

// ListSmartNIC gets a list of smartnics
func (tb *TestBed) ListSmartNIC() (snl []*cluster.SmartNIC, err error) {
	ctx, err := tb.VeniceLoggedInCtx()
	if err != nil {
		return nil, err
	}
	restcls, err := tb.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	opts := api.ListWatchOptions{}

	for _, restcl := range restcls {
		snl, err = restcl.ClusterV1().SmartNIC().List(ctx, &opts)
		if err == nil {
			break
		}
	}

	return snl, err
}

// GetEndpoint returns the endpoint
func (tb *TestBed) GetEndpoint(meta *api.ObjectMeta) (ep *workload.Endpoint, err error) {
	ctx, err := tb.VeniceLoggedInCtx()
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

// CreateFirewallProfile creates firewall profile
func (tb *TestBed) CreateFirewallProfile(fwp *security.FirewallProfile) error {
	ctx, err := tb.VeniceLoggedInCtx()
	if err != nil {
		return err
	}
	restcls, err := tb.VeniceRestClient()
	if err != nil {
		return err
	}

	for _, restcl := range restcls {
		_, err = restcl.SecurityV1().FirewallProfile().Create(ctx, fwp)
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
