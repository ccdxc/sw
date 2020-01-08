package testutils

import (
	"bytes"
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/login"
	loginCtx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/testutils"
)

const (
	// ExpiryDuration is default duration for token expiry in authentication policy
	ExpiryDuration = "144h"
)

// RadiusConfig to define test config for ACS or OpenRadius
type RadiusConfig struct {
	// URL is the host:port of the radius server
	URL string `json:",omitempty"`
	// NasID of venice
	NasID string `json:",omitempty"`
	// NasSecret is the shared secret between venice and radius server
	NasSecret string `json:",omitempty"`
	// User is the radius username
	User string `json:",omitempty"`
	// Password is the radius user password
	Password string `json:",omitempty"`
	// UserGroups contain groups to which user belongs
	UserGroups []string `json:",omitempty"`
	// Tenant to which user belongs
	Tenant string `json:",omitempty"`
}

func encodeHTTPRequest(req *http.Request, request interface{}) error {
	var buf bytes.Buffer
	err := json.NewEncoder(&buf).Encode(request)
	if err != nil {
		return err
	}
	req.Body = ioutil.NopCloser(&buf)
	return nil
}

// getHTTPRequest creates a http request
func getHTTPRequest(instance string, in interface{}, method, path string) (*http.Request, error) {
	target, err := url.Parse(instance)
	if err != nil {
		return nil, fmt.Errorf("invalid instance %s", instance)
	}
	target.Path = path
	req, err := http.NewRequest(method, target.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("could not create request (%s)", err)
	}
	if err = encodeHTTPRequest(req, in); err != nil {
		return nil, fmt.Errorf("could not encode request (%s)", err)
	}
	return req, nil
}

// Login sends a login request to API Gateway and returns a *http.Response
func Login(apiGW string, in *auth.PasswordCredential) (*http.Response, error) {
	path := login.LoginURLPath
	req, err := getHTTPRequest(apiGW, in, "POST", path)
	if err != nil {
		return nil, err
	}
	client := &http.Client{
		Transport: &http.Transport{
			TLSClientConfig: &tls.Config{
				InsecureSkipVerify: true,
			},
		},
	}
	ctx := context.Background()
	resp, err := client.Do(req.WithContext(ctx))
	return resp, err
}

// NewLoggedInContext authenticates user and returns a new context derived from given context with Authorization header set to JWT.
// Returns original context in case of error.
func NewLoggedInContext(ctx context.Context, apiGW string, in *auth.PasswordCredential) (context.Context, error) {
	// When doing operations, we want to keep retrying every 2 seconds till we succeed (or)
	// till the calling context is Done
	return NewLoggedInContextWithTimeout(ctx, apiGW, in, 2*time.Second)
}

// NewLoggedInContextWithTimeout authenticates user and returns a new context derived from given context with Authorization header set to JWT.
// Returns original context in case of error.
func NewLoggedInContextWithTimeout(ctx context.Context, apiGW string, in *auth.PasswordCredential, d time.Duration) (context.Context, error) {
	var err error
	var nctx context.Context

	t, ok := ctx.Deadline()

	var timeout string
	if !ok {
		timeout = "100s" // seconds
	} else {
		timeout = time.Until(t).String()
	}

	if testutils.CheckEventually(func() (bool, interface{}) {
		tctx, cancel := context.WithTimeout(ctx, d)
		defer cancel()
		nctx, err = login.NewLoggedInContext(tctx, apiGW, in)
		if err == nil {
			// nctx is based on tctx which is short-lived. copy token from it for a new context based on user provided context
			token, ok := loginCtx.AuthzHeaderFromContext(nctx)
			if ok {
				nctx = loginCtx.NewContextWithAuthzHeader(ctx, token)
				return true, nil
			}
		}
		log.Errorf("unable to get logged in context (%v)", err)
		return false, fmt.Sprintf("unable to get logged in context (%v)", err)
	}, d.String(), timeout) {
		return nctx, err
	}

	return ctx, err
}

// CreateTestUser creates a test user
func CreateTestUser(ctx context.Context, apicl apiclient.Services, username, password, tenant string) (*auth.User, error) {
	// user object
	user := &auth.User{}
	user.Defaults("all")
	user.Name = username
	user.Tenant = tenant
	user.Namespace = "default"
	user.Spec.Password = password
	user.Spec.Fullname = "Test User"
	user.Spec.Email = "testuser@pensandio.io"
	user.Spec.Type = auth.UserSpec_Local.String()

	// create the user object in api server
	var err error
	var createdUser *auth.User
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdUser, err = apicl.AuthV1().User().Create(ctx, user)
		// 409 is returned when user already exists. 401 when auth is already bootstrapped.
		if err == nil || strings.HasPrefix(err.Error(), "Status:(409)") || strings.HasPrefix(err.Error(), "Status:(401)") ||
			strings.Contains(err.Error(), "Status:(409)") {
			err = nil
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error creating user, Err: %v", err)
		return nil, err
	}
	return createdUser, err
}

// MustCreateTestUser creates testuser and panics if fails
func MustCreateTestUser(apicl apiclient.Services, username, password, tenant string) *auth.User {
	u, err := CreateTestUser(context.TODO(), apicl, username, password, tenant)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateTestUser", err))
	}
	return u
}

// MustCreateUserWithCtx creates user and panics if fails
func MustCreateUserWithCtx(ctx context.Context, apicl apiclient.Services, username, password, tenant string) *auth.User {
	u, err := CreateTestUser(ctx, apicl, username, password, tenant)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateTestUser", err))
	}
	return u
}

// DeleteUser deletes an user
func DeleteUser(apicl apiclient.Services, username, tenant string) error {
	ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelFunc()
	// delete user object in api server
	_, err := apicl.AuthV1().User().Delete(ctx, &api.ObjectMeta{Name: username, Tenant: tenant})
	return err
}

// MustDeleteUser deletes an user and throws a panic in case of error
func MustDeleteUser(apicl apiclient.Services, username, tenant string) {
	if err := DeleteUser(apicl, username, tenant); err != nil {
		panic(fmt.Sprintf("DeleteUser failed with err: %v", err))
	}
}

// CreateAuthenticationPolicy creates an authentication policy with local and ldap auth config. secret and radius config is set to nil in the policy
func CreateAuthenticationPolicy(apicl apiclient.Services, local *auth.Local, ldap *auth.Ldap) (*auth.AuthenticationPolicy, error) {
	return CreateAuthenticationPolicyWithOrder(apicl, local, ldap, nil, []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()}, ExpiryDuration)
}

// MustCreateAuthenticationPolicy creates an authentication policy with local, ldap and radius auth config
func MustCreateAuthenticationPolicy(apicl apiclient.Services, local *auth.Local, ldap *auth.Ldap, radius *auth.Radius) *auth.AuthenticationPolicy {
	var authOrder []string
	if local != nil {
		authOrder = append(authOrder, auth.Authenticators_LOCAL.String())
	}
	if ldap != nil {
		authOrder = append(authOrder, auth.Authenticators_LDAP.String())
	}
	if radius != nil {
		authOrder = append(authOrder, auth.Authenticators_RADIUS.String())
	}
	pol, err := CreateAuthenticationPolicyWithOrder(apicl, local, ldap, radius, authOrder, ExpiryDuration)
	if err != nil {
		panic(fmt.Sprintf("CreateAuthenticationPolicy failed with err %s", err))
	}
	return pol
}

// CreateAuthenticationPolicyWithOrder creates an authentication policy
func CreateAuthenticationPolicyWithOrder(apicl apiclient.Services, local *auth.Local, ldap *auth.Ldap, radius *auth.Radius, order []string, tokenExpiry string) (*auth.AuthenticationPolicy, error) {
	ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelFunc()
	// authn policy object
	policy := &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap:               ldap,
				Local:              local,
				Radius:             radius,
				AuthenticatorOrder: order,
			},
			TokenExpiry: tokenExpiry,
		},
	}

	// create authentication policy object in api server
	var err error
	var createdPolicy *auth.AuthenticationPolicy
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdPolicy, err = apicl.AuthV1().AuthenticationPolicy().Create(ctx, policy)
		// 409 is returned when authpolicy already exists. 401 when auth is already bootstrapped.
		if err == nil || strings.HasPrefix(err.Error(), "Status:(409)") || strings.HasPrefix(err.Error(), "Status:(401)") ||
			strings.Contains(err.Error(), "Status:(409)") {
			err = nil
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error creating authentication policy, Err: %v", err)
		return nil, err
	}
	return createdPolicy, err
}

// DeleteAuthenticationPolicy deletes an authentication policy
func DeleteAuthenticationPolicy(apicl apiclient.Services) error {
	ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelFunc()
	// delete authentication policy object in api server
	_, err := apicl.AuthV1().AuthenticationPolicy().Delete(ctx, &api.ObjectMeta{Name: "AuthenticationPolicy"})
	if err != nil {
		log.Errorf("Error deleting AuthenticationPolicy: %v", err)
		return err
	}
	return nil
}

// MustDeleteAuthenticationPolicy deletes an authentication policy and throws a panic in case of error
func MustDeleteAuthenticationPolicy(apicl apiclient.Services) {
	if err := DeleteAuthenticationPolicy(apicl); err != nil {
		panic(fmt.Sprintf("DeleteAuthenticationPolicy failed with err: %v", err))
	}
}

// MustCreateVersion creates a version and panics if fails
func MustCreateVersion(apicl apiclient.Services, name string) *cluster.Version {
	version, err := CreateVersion(apicl, name)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateVersion", err))
	}
	return version
}

// MustDeleteVersion deletes a version
func MustDeleteVersion(apicl apiclient.Services, version string) {
	if err := DeleteVersion(apicl, version); err != nil {
		panic(fmt.Sprintf("DeleteVersion failed with err: %v", err))
	}
}

// DeleteVersion deletes a version
func DeleteVersion(apicl apiclient.Services, version string) error {
	ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelFunc()
	// delete version object in api server
	_, err := apicl.ClusterV1().Version().Delete(ctx, &api.ObjectMeta{Name: version})
	if err != nil {
		return err
	}
	deletedVersion, err := apicl.ClusterV1().Version().Get(ctx, &api.ObjectMeta{Name: version})
	if err == nil {
		log.Errorf("Error deleting version, found [%#v]", deletedVersion)
		return fmt.Errorf("deleted version still exists")
	}

	return nil
}

// CreateVersion creates a version
func CreateVersion(apicl apiclient.Services, name string) (*cluster.Version, error) {
	ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelFunc()
	version := &cluster.Version{
		TypeMeta: api.TypeMeta{Kind: "Version"},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
	}
	var err error
	var createdVersion *cluster.Version
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdVersion, err = apicl.ClusterV1().Version().Create(ctx, version)
		if (err == nil) || strings.Contains(err.Error(), "AlreadyExists") {
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error creating version, Err: %v", err)
		return nil, err
	}
	return createdVersion, err
}

// CreateTenant creates a tenant
func CreateTenant(apicl apiclient.Services, name string) (*cluster.Tenant, error) {
	tenant := &cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Spec: cluster.TenantSpec{},
	}
	var err error
	var createdTenant *cluster.Tenant
	if !testutils.CheckEventually(func() (bool, interface{}) {
		ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
		createdTenant, err = apicl.ClusterV1().Tenant().Create(ctx, tenant)
		cancelFunc()
		// 412 is returned when tenant and default roles already exist. 401 when auth is already bootstrapped.
		if err == nil || strings.HasPrefix(err.Error(), "Status:(412)") || strings.HasPrefix(err.Error(), "Status:(401)") ||
			strings.Contains(err.Error(), "Status:(409)") {
			//log.Errorf("Creating tenant, Err: %v", err.Error())
			err = nil
			return true, nil
		}
		return false, nil
	}, "500ms", "90s") {
		log.Errorf("Error creating tenant, Err: %v", err)
		return nil, err
	}
	return createdTenant, err
}

// MustCreateTenant creates a tenant and panics if fails
func MustCreateTenant(apicl apiclient.Services, name string) *cluster.Tenant {
	tenant, err := CreateTenant(apicl, name)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateTenant", err))
	}
	return tenant
}

// DeleteTenant deletes a tenant
func DeleteTenant(apicl apiclient.Services, tenant string) error {
	ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelFunc()
	// delete all alerts belonging to default alert policy
	alertPolicies, _ := apicl.MonitoringV1().Alert().List(ctx,
		&api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenant}})
	for _, ap := range alertPolicies {
		apicl.MonitoringV1().Alert().Delete(ctx, &ap.ObjectMeta)
	}

	// delete default alert policy
	apicl.MonitoringV1().AlertPolicy().Delete(ctx, &api.ObjectMeta{Tenant: tenant, Name: "default-event-based-alerts"})

	// delete tenant object in api server
	_, err := apicl.ClusterV1().Tenant().Delete(ctx, &api.ObjectMeta{Name: tenant})
	if err != nil {
		return err
	}
	deletedTenant, err := apicl.ClusterV1().Tenant().Get(ctx, &api.ObjectMeta{Name: tenant})
	if err == nil {
		log.Errorf("Error deleting tenant, found [%#v]", deletedTenant)
		return fmt.Errorf("deleted tenant still exists")
	}

	return nil
}

// MustDeleteTenant deletes a tenant
func MustDeleteTenant(apicl apiclient.Services, tenant string) {
	if err := DeleteTenant(apicl, tenant); err != nil {
		panic(fmt.Sprintf("DeleteTenant failed with err: %v", err))
	}
}

// CreateRole creates a role
func CreateRole(ctx context.Context, apicl apiclient.Services, name, tenant string, permissions ...auth.Permission) (*auth.Role, error) {
	role := login.NewRole(name, tenant, permissions...)
	var err error
	var createdRole *auth.Role
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdRole, err = apicl.AuthV1().Role().Create(ctx, role)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error creating role, Err: %v", err)
		return nil, err
	}
	return createdRole, err
}

// MustCreateRole creates a role and panics if fails
func MustCreateRole(apicl apiclient.Services, name, tenant string, permissions ...auth.Permission) *auth.Role {
	role, err := CreateRole(context.TODO(), apicl, name, tenant, permissions...)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateRole", err))
	}
	return role
}

// MustCreateRoleWithCtx creates a role and panics if fails
func MustCreateRoleWithCtx(ctx context.Context, apicl apiclient.Services, name, tenant string, permissions ...auth.Permission) *auth.Role {
	role, err := CreateRole(ctx, apicl, name, tenant, permissions...)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateRole", err))
	}
	return role
}

// DeleteRole deletes a role
func DeleteRole(apicl apiclient.Services, name, tenant string) error {
	ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelFunc()
	// delete role binding object in api server
	_, err := apicl.AuthV1().Role().Delete(ctx, &api.ObjectMeta{Name: name, Tenant: tenant})
	return err
}

// MustDeleteRole deletes a role
func MustDeleteRole(apicl apiclient.Services, name, tenant string) {
	if err := DeleteRole(apicl, name, tenant); err != nil {
		panic(fmt.Sprintf("DeleteRole failed with err: %v", err))
	}
}

// CreateRoleBinding creates a role binding
func CreateRoleBinding(ctx context.Context, apicl apiclient.Services, name, tenant, roleName string, users, groups []string) (*auth.RoleBinding, error) {
	// TODO: use func from rbac utils
	roleBinding := &auth.RoleBinding{
		TypeMeta: api.TypeMeta{Kind: string(auth.KindRoleBinding)},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Tenant:    tenant,
			Namespace: globals.DefaultNamespace,
		},
		Spec: auth.RoleBindingSpec{
			Users:      users,
			UserGroups: groups,
			Role:       roleName,
		},
	}
	var err error
	var createdRoleBinding *auth.RoleBinding
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdRoleBinding, err = apicl.AuthV1().RoleBinding().Create(ctx, roleBinding)
		// 409 is returned when role binding already exists. 401 when auth is already bootstrapped.
		if err == nil || strings.HasPrefix(err.Error(), "Status:(409)") || strings.HasPrefix(err.Error(), "Status:(401)") {
			err = nil
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error creating role binding, Err: %v", err)
		return nil, err
	}
	return createdRoleBinding, err
}

// MustCreateRoleBinding creates a role binding and panics if fails
func MustCreateRoleBinding(apicl apiclient.Services, name, tenant, roleName string, users, groups []string) *auth.RoleBinding {
	roleBinding, err := CreateRoleBinding(context.TODO(), apicl, name, tenant, roleName, users, groups)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateRoleBinding", err))
	}
	return roleBinding
}

// MustCreateRoleBindingWithCtx creates a role binding and panics if fails
func MustCreateRoleBindingWithCtx(ctx context.Context, apicl apiclient.Services, name, tenant, roleName string, users, groups []string) *auth.RoleBinding {
	roleBinding, err := CreateRoleBinding(ctx, apicl, name, tenant, roleName, users, groups)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateRoleBindingWithCtx", err))
	}
	return roleBinding
}

// UpdateRoleBinding updates a role binding
func UpdateRoleBinding(ctx context.Context, apicl apiclient.Services, name, tenant, roleName string, users, groups []string) (*auth.RoleBinding, error) {
	var err error
	var roleBinding *auth.RoleBinding
	if !testutils.CheckEventually(func() (bool, interface{}) {
		roleBinding, err = apicl.AuthV1().RoleBinding().Get(ctx, &api.ObjectMeta{Name: name, Tenant: tenant})
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error getting role binding, Err: %v", err)
		return nil, err
	}
	if roleName != "" {
		roleBinding.Spec.Role = roleName
	}
	roleBinding.Spec.Users = users
	roleBinding.Spec.UserGroups = groups
	var updatedRoleBinding *auth.RoleBinding
	if !testutils.CheckEventually(func() (bool, interface{}) {
		updatedRoleBinding, err = apicl.AuthV1().RoleBinding().Update(ctx, roleBinding)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error updating role binding, Err: %v", err)
		return nil, err
	}
	return updatedRoleBinding, err
}

// MustUpdateRoleBinding updates a role binding and panics if fails
func MustUpdateRoleBinding(apicl apiclient.Services, name, tenant, roleName string, users, groups []string) *auth.RoleBinding {
	roleBinding, err := UpdateRoleBinding(context.TODO(), apicl, name, tenant, roleName, users, groups)
	if err != nil {
		panic(fmt.Sprintf("error %s in UpdateRoleBinding", err))
	}
	return roleBinding
}

// MustUpdateRoleBindingWithCtx updates a role binding and panics if fails
func MustUpdateRoleBindingWithCtx(ctx context.Context, apicl apiclient.Services, name, tenant, roleName string, users, groups []string) *auth.RoleBinding {
	roleBinding, err := UpdateRoleBinding(ctx, apicl, name, tenant, roleName, users, groups)
	if err != nil {
		panic(fmt.Sprintf("error %s in UpdateRoleBindingWithCtx", err))
	}
	return roleBinding
}

// DeleteRoleBinding deletes a role binding
func DeleteRoleBinding(apicl apiclient.Services, name, tenant string) error {
	ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelFunc()
	// delete role binding object in api server
	_, err := apicl.AuthV1().RoleBinding().Delete(ctx, &api.ObjectMeta{Name: name, Tenant: tenant})
	return err
}

// MustDeleteRoleBinding deletes a role binding
func MustDeleteRoleBinding(apicl apiclient.Services, name, tenant string) {
	if err := DeleteRoleBinding(apicl, name, tenant); err != nil {
		panic(fmt.Sprintf("DeleteRoleBinding failed with err: %v", err))
	}
}

// SetAuthBootstrapFlag sets bootstrap flag in the cluster object
func SetAuthBootstrapFlag(apicl apiclient.Services) (*cluster.Cluster, error) {
	ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelFunc()
	var clusterObj *cluster.Cluster
	var err error
	if !testutils.CheckEventually(func() (bool, interface{}) {
		clusterObj, err = apicl.ClusterV1().Cluster().AuthBootstrapComplete(ctx, &cluster.ClusterAuthBootstrapRequest{})
		// 401 when auth is already bootstrapped.
		if err == nil || strings.HasPrefix(err.Error(), "Status:(401)") {
			err = nil
			return true, nil
		}
		return false, nil
	}, "100ms", "20s") {
		log.Errorf("Error setting bootstrap flag: %v", err)
		return nil, err
	}
	return clusterObj, err
}

// MustSetAuthBootstrapFlag sets bootstrap flag in the cluster object
func MustSetAuthBootstrapFlag(apicl apiclient.Services) *cluster.Cluster {
	clusterObj, err := SetAuthBootstrapFlag(apicl)
	if err != nil {
		panic(fmt.Sprintf("error %v in SetAuthBootstrapFlag", err))
	}
	return clusterObj
}

// CreateCluster creates a test cluster object
func CreateCluster(apicl apiclient.Services) (*cluster.Cluster, error) {
	ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelFunc()
	clusterObj := &cluster.Cluster{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
		ObjectMeta: api.ObjectMeta{
			Name: "Cluster",
		},
	}
	var err error
	var createdCluster *cluster.Cluster
	if !testutils.CheckEventually(func() (bool, interface{}) {
		createdCluster, err = apicl.ClusterV1().Cluster().Create(ctx, clusterObj)
		if (err == nil) || strings.Contains(err.Error(), "AlreadyExists") {
			return true, nil
		}
		return false, err
	}, "500ms", "90s") {
		log.Errorf("Error creating cluster, Err: %v", err)
		return nil, err
	}
	return createdCluster, nil
}

// MustCreateCluster creates a cluster and panics if fails
func MustCreateCluster(apicl apiclient.Services) *cluster.Cluster {
	cluster, err := CreateCluster(apicl)
	if err != nil {
		panic(fmt.Sprintf("error %s in CreateCluster", err))
	}
	return cluster
}

// DeleteCluster deletes a cluster object
func DeleteCluster(apicl apiclient.Services) error {
	ctx, cancelFunc := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancelFunc()
	// delete cluster object in api server
	_, err := apicl.ClusterV1().Cluster().Delete(ctx, &api.ObjectMeta{})
	if err != nil {
		return err
	}
	deletedCluster, err := apicl.ClusterV1().Cluster().Get(ctx, &api.ObjectMeta{})
	if err == nil {
		log.Errorf("Error deleting cluster, found [%#v]", deletedCluster)
		return fmt.Errorf("deleted cluster still exists")
	}

	return nil
}

// MustDeleteCluster deletes a cluster
func MustDeleteCluster(apicl apiclient.Services) {
	if err := DeleteCluster(apicl); err != nil {
		panic(fmt.Sprintf("DeleteCluster failed with err: %v", err))
	}
}
