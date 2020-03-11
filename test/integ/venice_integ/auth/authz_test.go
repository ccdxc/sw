package auth

import (
	"context"
	"fmt"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/staging"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"

	. "github.com/pensando/sw/test/utils"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestAuthorization(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "error creating logged in context")
	// retrieve auth policy
	AssertConsistently(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.AuthV1().AuthenticationPolicy().Get(ctx, &api.ObjectMeta{Name: "AuthenticationPolicy"})
		return err != nil, nil
	}, "authorization error expected while retrieving authentication policy", "100ms", "1s")
	// retrieve tenant
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.ClusterV1().Tenant().Get(ctx, &api.ObjectMeta{Name: testTenant})
		return err != nil, nil
	}, "authorization error expected while retrieving tenant", "100ms", "1s")
	// retrieve role
	var role *auth.Role
	AssertEventually(t, func() (bool, interface{}) {
		role, err = tinfo.restcl.AuthV1().Role().Get(ctx, &api.ObjectMeta{Name: globals.AdminRole, Tenant: testTenant})
		return err == nil, nil
	}, fmt.Sprintf("error while retrieving role: Err: %v", err))
	Assert(t, role.Name == globals.AdminRole && role.Tenant == testTenant, fmt.Sprintf("incorrect role retrieved [%#v]", role))
	const testTenant2 = "testtenant2"

	// create testtenant2 and admin user
	MustCreateTenant(tinfo.apicl, testTenant2)
	defer MustDeleteTenant(tinfo.apicl, testTenant2)
	MustCreateTestUser(tinfo.apicl, testUser, testPassword, testTenant2)
	defer MustDeleteUser(tinfo.apicl, testUser, testTenant2)
	MustUpdateRoleBinding(tinfo.apicl, globals.AdminRoleBinding, testTenant2, globals.AdminRole, []string{testUser}, nil)
	defer MustUpdateRoleBinding(tinfo.apicl, globals.AdminRoleBinding, testTenant2, globals.AdminRole, nil, nil)
	ctx, err = NewLoggedInContext(context.Background(), tinfo.apiGwAddr, &auth.PasswordCredential{Username: testUser, Password: testPassword, Tenant: testTenant2})
	AssertOk(t, err, "error creating logged in context for testtenant2 admin user")
	// tenant boundaries should be respected; retrieving testtenant AdminRole should fail
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Get(ctx, &api.ObjectMeta{Name: globals.AdminRole, Tenant: testTenant})
		return err != nil, nil
	}, "authorization error expected while retrieve other tenant's AdminRole", "100ms", "1s")
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().User().Get(ctx, &api.ObjectMeta{Name: testUser, Tenant: testTenant})
		return err != nil, nil
	}, "authorization error expected while retrieve other tenant's admin user", "100ms", "1s")
}

func TestAdminRole(t *testing.T) {
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, adminCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, adminCred, tinfo.l)

	superAdminCtx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")

	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Delete(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRole, Tenant: globals.DefaultTenant})
		tinfo.l.Infof("admin role delete error: %v", err)
		return err != nil, err
	}, "admin role shouldn't be deleted", "100ms", "1s")
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Create(superAdminCtx, login.NewRole(globals.AdminRole, globals.DefaultTenant, login.NewPermission(
			authz.ResourceTenantAll,
			authz.ResourceGroupAll,
			authz.ResourceKindAll,
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String())))
		tinfo.l.Infof("admin role create error: %v", err)
		return err != nil, err
	}, "admin role shouldn't be created", "100ms", "1s")
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Update(superAdminCtx, login.NewRole(globals.AdminRole, globals.DefaultTenant, login.NewPermission(
			authz.ResourceTenantAll,
			authz.ResourceGroupAll,
			authz.ResourceKindAll,
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String())))
		tinfo.l.Infof("admin role update error: %v", err)
		return err != nil, err
	}, "admin role shouldn't be updated", "100ms", "1s")
	currTime := time.Now()
	MustCreateTenant(tinfo.apicl, testTenant)
	var adminRole *auth.Role
	AssertEventually(t, func() (bool, interface{}) {
		adminRole, err = tinfo.restcl.AuthV1().Role().Get(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRole, Tenant: testTenant})
		return err == nil, err
	}, "admin role should be created when a tenant is created")
	creationTime, err := adminRole.CreationTime.Time()
	AssertOk(t, err, "error getting role creation time")
	Assert(t, creationTime.After(currTime), "admin role creation time is not set")
	Assert(t, adminRole.UUID != "", "admin role UUID is not set")
	MustDeleteTenant(tinfo.apicl, testTenant)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Get(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRole, Tenant: testTenant})
		return err != nil, err
	}, "admin role should be deleted when a tenant is deleted")
}

func TestAdminRoleBinding(t *testing.T) {
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, adminCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, adminCred, tinfo.l)

	superAdminCtx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	// Deleting admin role binding should fail
	_, err = tinfo.restcl.AuthV1().RoleBinding().Delete(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRoleBinding, Tenant: globals.DefaultTenant})
	Assert(t, err != nil, "expected error while deleting admin role binding")
	// updating super admin role binding to not have any users and groups should fail
	var superAdminRoleBinding *auth.RoleBinding
	AssertEventually(t, func() (bool, interface{}) {
		superAdminRoleBinding, err = tinfo.restcl.AuthV1().RoleBinding().Get(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRoleBinding, Tenant: globals.DefaultTenant})
		return err == nil, err
	}, "error fetching super admin role binding")
	superAdminRoleBinding.Spec.Users = []string{}
	_, err = tinfo.restcl.AuthV1().RoleBinding().Update(superAdminCtx, superAdminRoleBinding)
	Assert(t, err != nil && strings.Contains(err.Error(), "AdminRoleBinding in default tenant should have at least one user or group"),
		"expected updating super admin role binding with no subject to fail")
	// test admin role binding creation and deletion
	currTime := time.Now()
	MustCreateTenant(tinfo.apicl, testTenant)
	var adminRoleBinding *auth.RoleBinding
	AssertEventually(t, func() (bool, interface{}) {
		adminRoleBinding, err = tinfo.restcl.AuthV1().RoleBinding().Get(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRoleBinding, Tenant: testTenant})
		return err == nil, err
	}, "admin role binding should be created when a tenant is created")
	creationTime, err := adminRoleBinding.CreationTime.Time()
	AssertOk(t, err, "error getting role binding creation time")
	Assert(t, creationTime.After(currTime), "admin role binding creation time is not set")
	Assert(t, adminRoleBinding.UUID != "", "admin role binding UUID is not set")
	// updating AdminRoleBinding to refer to a non-admin role should fail
	MustCreateRole(tinfo.apicl, "NetworkAdminRole", testTenant, login.NewPermission(
		testTenant,
		string(apiclient.GroupNetwork),
		string(network.KindNetwork),
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_AllActions.String()),
		login.NewPermission(
			testTenant,
			string(apiclient.GroupAuth),
			authz.ResourceKindAll,
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String()))
	adminRoleBinding.Spec.Role = "NetworkAdminRole"
	_, err = tinfo.restcl.AuthV1().RoleBinding().Update(superAdminCtx, adminRoleBinding)
	// remove NetworkAdminRole
	MustDeleteRole(tinfo.apicl, "NetworkAdminRole", testTenant)
	Assert(t, err != nil, "expected error while updating AdminRoleBinding to have non-admin role")
	Assert(t, strings.Contains(err.Error(), "AdminRoleBinding can bind to only AdminRole"), fmt.Sprintf("unexpected error: %v", err))
	// test if admin role binding for a non default tenant can have no users and groups
	MustCreateTestUser(tinfo.apicl, testUser, testPassword, testTenant)
	adminRoleBinding.Spec.Role = globals.AdminRole
	adminRoleBinding.Spec.Users = []string{testUser}
	var updatedAdminRoleBinding *auth.RoleBinding
	AssertEventually(t, func() (bool, interface{}) {
		updatedAdminRoleBinding, err = tinfo.restcl.AuthV1().RoleBinding().Update(superAdminCtx, adminRoleBinding)
		return err == nil, err
	}, "error updating admin role binding with user")
	Assert(t, updatedAdminRoleBinding.Spec.Users[0] == testUser, fmt.Sprintf("admin role binding not updated with user: %#v", updatedAdminRoleBinding))
	adminRoleBinding.Spec.Users = []string{}
	AssertEventually(t, func() (bool, interface{}) {
		updatedAdminRoleBinding, err = tinfo.restcl.AuthV1().RoleBinding().Update(superAdminCtx, adminRoleBinding)
		return err == nil, err
	}, "error removing user from admin role binding")
	Assert(t, len(updatedAdminRoleBinding.Spec.Users) == 0, fmt.Sprintf("users not cleared from admin role binding: %#v", updatedAdminRoleBinding))
	MustDeleteUser(tinfo.apicl, testUser, testTenant)
	// deleting tenant should delete AdminRoleBinding
	MustDeleteTenant(tinfo.apicl, testTenant)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().RoleBinding().Get(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRoleBinding, Tenant: testTenant})
		return err != nil, err
	}, "admin role binding should be deleted when a tenant is deleted")
}

func TestPrivilegeEscalation(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "error creating logged in context")

	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Update(ctx, login.NewRole(globals.AdminRole, testTenant, login.NewPermission(
			authz.ResourceTenantAll,
			authz.ResourceGroupAll,
			authz.ResourceKindAll,
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String())))
		tinfo.l.Infof("global admin role update error: %v", err)
		return err != nil, err
	}, "tenant admin should not be able to update tenant admin role to global admin role", "100ms", "1s")

	// create testUser2 as network admin with permissions to create RBAC objects also
	MustCreateTestUser(tinfo.apicl, "testUser2", testPassword, testTenant)
	defer MustDeleteUser(tinfo.apicl, "testUser2", testTenant)
	MustCreateRole(tinfo.apicl, "NetworkAdminRole", testTenant, login.NewPermission(
		testTenant,
		string(apiclient.GroupNetwork),
		string(network.KindNetwork),
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_AllActions.String()),
		login.NewPermission(
			testTenant,
			string(apiclient.GroupAuth),
			authz.ResourceKindAll,
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String()))
	defer MustDeleteRole(tinfo.apicl, "NetworkAdminRole", testTenant)
	roleBinding := MustCreateRoleBinding(tinfo.apicl, "NetworkAdminRoleBinding", testTenant, "NetworkAdminRole", []string{"testUser2"}, nil)
	defer MustDeleteRoleBinding(tinfo.apicl, "NetworkAdminRoleBinding", testTenant)
	// login as network admin
	ctx, err = NewLoggedInContext(context.Background(), tinfo.apiGwAddr, &auth.PasswordCredential{
		Username: "testUser2",
		Password: testPassword,
		Tenant:   testTenant,
	})
	AssertOk(t, err, "error creating logged in context")
	// assign tenant admin role
	roleBinding.Spec.Role = globals.AdminRole
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().RoleBinding().Update(ctx, roleBinding)
		tinfo.l.Infof("network admin role binding update error: %v", err)
		return err != nil, err
	}, "network admin should not be able to update its role binding to assign himself tenant admin role", "100ms", "1s")
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Create(ctx, login.NewRole("NetworkGroupAdminRole", testTenant, login.NewPermission(
			testTenant,
			string(apiclient.GroupNetwork),
			authz.ResourceKindAll,
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String())))
		tinfo.l.Infof("network group admin role create error: %v", err)
		return err != nil, err
	}, "network admin should not be able to create role with privileges to all kinds within network group", "100ms", "1s")
	AssertEventually(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Create(ctx, login.NewRole("LimitedNetworkRole", testTenant, login.NewPermission(
			testTenant,
			string(apiclient.GroupNetwork),
			string(network.KindNetwork),
			authz.ResourceNamespaceAll,
			"testnetwork",
			auth.Permission_Read.String())))
		if err == nil {
			MustDeleteRole(tinfo.apicl, "LimitedNetworkRole", testTenant)
		}
		return err == nil, err
	}, "network admin should be able to create role with lesser privileges")
}

func TestBootstrapFlag(t *testing.T) {
	// create cluster object
	clusterObj := MustCreateCluster(tinfo.apicl)
	defer MustDeleteCluster(tinfo.apicl)
	// create tenant
	MustCreateTenant(tinfo.restcl, globals.DefaultTenant)
	defer MustDeleteTenant(tinfo.apicl, globals.DefaultTenant)
	AssertConsistently(t, func() (bool, interface{}) {
		clusterObj, err := tinfo.restcl.ClusterV1().Cluster().AuthBootstrapComplete(context.TODO(), &cluster.ClusterAuthBootstrapRequest{})
		tinfo.l.Infof("set bootstrap flag error: %v", err)
		return err != nil, clusterObj
	}, "bootstrap flag shouldn't be set till auth policy is created", "100ms", "1s")
	// create auth policy
	MustCreateAuthenticationPolicy(tinfo.restcl, &auth.Local{}, nil, nil)
	defer MustDeleteAuthenticationPolicy(tinfo.apicl)
	AssertConsistently(t, func() (bool, interface{}) {
		clusterObj, err := tinfo.restcl.ClusterV1().Cluster().AuthBootstrapComplete(context.TODO(), &cluster.ClusterAuthBootstrapRequest{})
		tinfo.l.Infof("set bootstrap flag error: %v", err)
		return err != nil, clusterObj
	}, "bootstrap flag shouldn't be set till admin role binding is updated with an user", "100ms", "1s")
	MustCreateTestUser(tinfo.restcl, testUser, testPassword, globals.DefaultTenant)
	defer MustDeleteUser(tinfo.apicl, testUser, globals.DefaultTenant)
	MustUpdateRoleBinding(tinfo.restcl, globals.AdminRoleBinding, globals.DefaultTenant, globals.AdminRole, []string{testUser}, nil)
	defer MustUpdateRoleBinding(tinfo.apicl, globals.AdminRoleBinding, globals.DefaultTenant, globals.AdminRole, nil, nil)
	// set bootstrap flag
	AssertEventually(t, func() (bool, interface{}) {
		var err error
		clusterObj, err = tinfo.restcl.ClusterV1().Cluster().AuthBootstrapComplete(context.TODO(), &cluster.ClusterAuthBootstrapRequest{})
		return err == nil, err
	}, "error setting bootstrap flag")
	Assert(t, clusterObj.Status.AuthBootstrapped, "bootstrap flag should be set to true")
	// once set, setting bootstrap flag will now need proper authorization
	AssertEventually(t, func() (bool, interface{}) {
		clusterObj, err := tinfo.restcl.ClusterV1().Cluster().AuthBootstrapComplete(context.TODO(), &cluster.ClusterAuthBootstrapRequest{})
		tinfo.l.Infof("set bootstrap flag error: %v", err)
		return err != nil, clusterObj
	}, "unauthenticated request to set bootstrap flag should fail")
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// login in as global admin
	superAdminCtx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	AssertEventually(t, func() (bool, interface{}) {
		clusterObj, err = tinfo.restcl.ClusterV1().Cluster().AuthBootstrapComplete(superAdminCtx, &cluster.ClusterAuthBootstrapRequest{})
		return err == nil, err
	}, "error setting bootstrap flag")
	Assert(t, clusterObj.Status.AuthBootstrapped, "bootstrap flag should be set to true")
	// try to unset bootstrap flag by updating cluster obj through API server
	clusterObj.Status.AuthBootstrapped = false
	updatedClusterObj, err := tinfo.apicl.ClusterV1().Cluster().Update(context.TODO(), clusterObj)
	AssertOk(t, err, "error updating cluster obj")
	Assert(t, updatedClusterObj.Status.AuthBootstrapped, "bootstrap flag should be un-settable once set to true")
}

func TestAPIGroupAuthorization(t *testing.T) {
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, adminCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, adminCred, tinfo.l)

	// create testUser2 as user admin with permissions to create RBAC objects
	MustCreateTestUser(tinfo.apicl, "testUser2", testPassword, globals.DefaultTenant)
	defer MustDeleteUser(tinfo.apicl, "testUser2", globals.DefaultTenant)
	MustCreateRole(tinfo.apicl, "UserAdminRole", globals.DefaultTenant,
		login.NewPermission(
			globals.DefaultTenant,
			string(apiclient.GroupAuth),
			authz.ResourceKindAll,
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String()))
	defer MustDeleteRole(tinfo.apicl, "UserAdminRole", globals.DefaultTenant)
	// login as user admin
	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, &auth.PasswordCredential{
		Username: "testUser2",
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	})
	AssertOk(t, err, "error creating logged in context")
	// retrieving role should result in authorization error at this point
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Get(ctx, &api.ObjectMeta{Name: "UserAdminRole", Tenant: globals.DefaultTenant})
		return err != nil, nil
	}, "authorization error expected while retrieving role", "100ms", "1s")
	MustCreateRoleBinding(tinfo.apicl, "UserAdminRoleBinding", globals.DefaultTenant, "UserAdminRole", []string{"testUser2"}, nil)
	defer MustDeleteRoleBinding(tinfo.apicl, "UserAdminRoleBinding", globals.DefaultTenant)
	// retrieving role should succeed now
	AssertEventually(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.AuthV1().Role().Get(ctx, &api.ObjectMeta{Name: "UserAdminRole", Tenant: globals.DefaultTenant})
		return err == nil, nil
	}, fmt.Sprintf("error while retrieving role: %v", err))
	AssertEventually(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.AuthV1().RoleBinding().Get(ctx, &api.ObjectMeta{Name: "UserAdminRoleBinding", Tenant: globals.DefaultTenant})
		return err == nil, nil
	}, fmt.Sprintf("error while retrieving role binding: %v", err))
	// retrieving tenant should fail due to authorization error
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.ClusterV1().Tenant().Get(ctx, &api.ObjectMeta{Name: globals.DefaultTenant})
		return err != nil, nil
	}, "authorization error expected while retrieving tenant", "100ms", "1s")
	MustCreateRole(tinfo.apicl, "ClusterScopedAllAPIGroupsRole", globals.DefaultTenant,
		login.NewPermission(
			"",
			authz.ResourceGroupAll,
			"",
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String()))
	defer MustDeleteRole(tinfo.apicl, "ClusterScopedAllAPIGroupsRole", globals.DefaultTenant)
	MustCreateRoleBinding(tinfo.apicl, "ClusterScopedAllAPIGroupsRoleBinding", globals.DefaultTenant, "ClusterScopedAllAPIGroupsRole", []string{"testUser2"}, nil)
	defer MustDeleteRoleBinding(tinfo.apicl, "ClusterScopedAllAPIGroupsRoleBinding", globals.DefaultTenant)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.ClusterV1().Cluster().Get(ctx, &api.ObjectMeta{})
		return err == nil, err
	}, "expected Cluster object retrieval to succeed")
	_, err = tinfo.restcl.SecurityV1().NetworkSecurityPolicy().List(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
	Assert(t, err != nil, "authorization error expected while listing NetworkSecurityPolicy")
}

func TestGetVersionInfo(t *testing.T) {
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, adminCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, adminCred, tinfo.l)

	MustCreateVersion(tinfo.apicl, "version")
	defer MustDeleteVersion(tinfo.apicl, "version")

	// create normalUser with limited access
	MustCreateTestUser(tinfo.apicl, "normalUser", testPassword, globals.DefaultTenant)
	defer MustDeleteUser(tinfo.apicl, "normalUser", globals.DefaultTenant)

	// logged in as an normal user with no access
	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, &auth.PasswordCredential{
		Username: "normalUser",
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	})
	AssertOk(t, err, "error creating logged in context")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.ClusterV1().Version().Get(ctx, &api.ObjectMeta{Name: "version", Tenant: globals.DefaultTenant})
		return err == nil, err
	}, fmt.Sprintf("error while retrieving version"))
}

func TestGetUserPreference(t *testing.T) {
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, adminCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, adminCred, tinfo.l)

	// create normalUser2 with limited access
	MustCreateTestUser(tinfo.apicl, "normalUser2", testPassword, globals.DefaultTenant)
	defer MustDeleteUser(tinfo.apicl, "normalUser2", globals.DefaultTenant)

	// logged in as an normal user with no access
	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, &auth.PasswordCredential{
		Username: "normalUser2",
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	})
	AssertOk(t, err, "error creating logged in context")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().UserPreference().Get(ctx, &api.ObjectMeta{Name: "normalUser2", Tenant: globals.DefaultTenant})
		return err == nil, err
	}, fmt.Sprintf("error while retrieving user preference"))
}

func TestWatchUserPreference(t *testing.T) {
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, adminCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, adminCred, tinfo.l)

	// create normalUser2 with limited access
	MustCreateTestUser(tinfo.apicl, "normalUser", testPassword, globals.DefaultTenant)
	defer MustDeleteUser(tinfo.apicl, "normalUser", globals.DefaultTenant)
	MustCreateTestUser(tinfo.apicl, "normalUser2", testPassword, globals.DefaultTenant)
	defer MustDeleteUser(tinfo.apicl, "normalUser2", globals.DefaultTenant)

	// logged in as an normal user with no access
	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, &auth.PasswordCredential{
		Username: "normalUser",
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	})
	AssertOk(t, err, "error creating logged in context")

	AssertEventually(t, func() (bool, interface{}) {
		opt := &api.ListWatchOptions{FieldSelector: "meta.name=normalUser"}
		opt.ObjectMeta.Tenant = "default"
		_, err := tinfo.restcl.AuthV1().UserPreference().Watch(ctx, opt)
		return err == nil, err
	}, fmt.Sprintf("error while retrieving user preference"))

	opt := &api.ListWatchOptions{FieldSelector: "meta.name=normalUser2"}
	opt.ObjectMeta.Tenant = "default"
	_, err = tinfo.restcl.AuthV1().UserPreference().Watch(ctx, opt)
	Assert(t, err != nil, "Should not be able to access different user's preference object")

}

func TestUserSelfOperations(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	MustCreateTestUser(tinfo.apicl, "testUser2", testPassword, testTenant)
	defer MustDeleteUser(tinfo.apicl, "testUser2", testTenant)
	MustCreateTestUser(tinfo.apicl, "testUser3", testPassword, testTenant)
	defer MustDeleteUser(tinfo.apicl, "testUser3", testTenant)
	// login as testUser2 who has no roles assigned
	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, &auth.PasswordCredential{Username: "testUser2", Password: testPassword, Tenant: testTenant})
	AssertOk(t, err, "error creating logged in context")

	// test get user
	var user *auth.User
	AssertEventually(t, func() (bool, interface{}) {
		user, err = tinfo.restcl.AuthV1().User().Get(ctx, &api.ObjectMeta{Name: "testUser2", Tenant: testTenant})
		return err == nil, err
	}, "unable to get user")
	Assert(t, user.Name == "testUser2" && user.Tenant == testTenant, fmt.Sprintf("user get failed: %#v", *user))
	// test update user
	AssertEventually(t, func() (bool, interface{}) {
		user, err = tinfo.restcl.AuthV1().User().Update(ctx, &auth.User{
			TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
			ObjectMeta: api.ObjectMeta{
				Tenant: testTenant,
				Name:   "testUser2",
			},
			Spec: auth.UserSpec{
				Fullname: "Test User2",
				Email:    "testuser@pensandio.io",
				Type:     auth.UserSpec_Local.String(),
			},
		})
		return err == nil, err
	}, "unable to update user")
	Assert(t, user.Spec.Fullname == "Test User2", fmt.Sprintf("user update failed: %#v", *user))
	// test change password
	const newPassword = "Newpassword1#"
	AssertEventually(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().User().PasswordChange(ctx, &auth.PasswordChangeRequest{
			TypeMeta: api.TypeMeta{
				Kind: string(auth.KindUser),
			},
			ObjectMeta: api.ObjectMeta{
				Name:   "testUser2",
				Tenant: testTenant,
			},
			OldPassword: testPassword,
			NewPassword: newPassword,
		})
		return err == nil, err
	}, "unable to change password")
	ctx, err = NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, &auth.PasswordCredential{Username: "testUser2", Password: newPassword, Tenant: testTenant})
	AssertOk(t, err, "unable to get logged in context with new password")
	// test reset password
	AssertEventually(t, func() (bool, interface{}) {
		user, err = tinfo.restcl.AuthV1().User().PasswordReset(ctx, &auth.PasswordResetRequest{
			TypeMeta: api.TypeMeta{
				Kind: string(auth.KindUser),
			},
			ObjectMeta: api.ObjectMeta{
				Name:   "testUser2",
				Tenant: testTenant,
			},
		})
		return err == nil, err
	}, "unable to reset password")
	ctx, err = NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, &auth.PasswordCredential{Username: "testUser2", Password: user.Spec.Password, Tenant: testTenant})
	AssertOk(t, err, "unable to get logged in context with new reset password")
	// test reset password of other user
	user, err = tinfo.restcl.AuthV1().User().PasswordReset(ctx, &auth.PasswordResetRequest{
		TypeMeta: api.TypeMeta{
			Kind: string(auth.KindUser),
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "testUser3",
			Tenant: testTenant,
		},
	})
	Assert(t, err != nil, "testUser2 was able to reset password for testUser3")
	user, err = tinfo.restcl.AuthV1().User().PasswordChange(ctx, &auth.PasswordChangeRequest{
		TypeMeta: api.TypeMeta{
			Kind: string(auth.KindUser),
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "testUser3",
			Tenant: testTenant,
		},
		OldPassword: testPassword,
		NewPassword: newPassword,
	})
	Assert(t, err != nil, "testUser2 was able to change password for testUser3")
}

func TestUserDelete(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	// user should not be able to delete itself
	adminCtx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "unable to get admin logged in context")
	_, err = tinfo.restcl.AuthV1().User().Delete(adminCtx, &api.ObjectMeta{Name: userCred.Username, Tenant: userCred.Tenant})
	Assert(t, err != nil, "user should not be able to delete himself")
	Assert(t, strings.Contains(err.Error(), "self-deletion of user is not allowed"), fmt.Sprintf("unexpected error: %v", err))
	// should be able to delete another admin user
	MustCreateTestUser(tinfo.apicl, "admin", testPassword, testTenant)
	MustCreateRoleBinding(tinfo.apicl, "AdminRoleBinding2", testTenant, globals.AdminRole, []string{"admin"}, nil)
	MustDeleteRoleBinding(tinfo.apicl, "AdminRoleBinding2", testTenant)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().User().Delete(adminCtx, &api.ObjectMeta{Name: "admin", Tenant: testTenant})
		return err == nil, err
	}, "unable to delete another admin user")
}

func TestCommitBuffer(t *testing.T) {
	// setup auth
	// create staging buffer
	// stage role
	// stage role binding
	// commit buffer
	// get role, rolebinding
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "error creating logged in context")
	AssertEventually(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.StagingV1().Buffer().Create(ctx, &staging.Buffer{ObjectMeta: api.ObjectMeta{Name: "TestBuffer", Tenant: globals.DefaultTenant}})
		return err == nil, nil
	}, fmt.Sprintf("unable to create staging buffer, err: %v", err))
	defer tinfo.restcl.StagingV1().Buffer().Delete(ctx, &api.ObjectMeta{Name: "TestBuffer", Tenant: globals.DefaultTenant})
	stagecl, err := apiclient.NewStagedRestAPIClient(tinfo.apiGwAddr, "TestBuffer")
	AssertOk(t, err, "error creating staging client")
	defer stagecl.Close()
	MustCreateRoleWithCtx(ctx, stagecl, "NetworkAdminRole", globals.DefaultTenant, login.NewPermission(
		globals.DefaultTenant,
		string(apiclient.GroupNetwork),
		string(network.KindNetwork),
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_AllActions.String()),
		login.NewPermission(
			globals.DefaultTenant,
			string(apiclient.GroupAuth),
			authz.ResourceKindAll,
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String()))
	MustCreateRoleBindingWithCtx(ctx, stagecl, "NetworkAdminRoleBinding", globals.DefaultTenant, "NetworkAdminRole", []string{testUser}, nil)
	var buf *staging.Buffer
	AssertEventually(t, func() (bool, interface{}) {
		buf, err = tinfo.restcl.StagingV1().Buffer().Get(ctx, &api.ObjectMeta{Name: "TestBuffer", Tenant: globals.DefaultTenant})
		return err == nil && buf.Status.ValidationResult != staging.BufferStatus_FAILED.String(), err
	}, fmt.Sprintf("expected buffer validation to succeed: %#v", buf))
	ca := staging.CommitAction{}
	ca.Name = "TestBuffer"
	ca.Tenant = globals.DefaultTenant
	_, err = tinfo.restcl.StagingV1().Buffer().Commit(ctx, &ca)
	AssertOk(t, err, "unable to commit staging buffer")
	AssertEventually(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.AuthV1().Role().Get(ctx, &api.ObjectMeta{Name: "NetworkAdminRole", Tenant: globals.DefaultTenant})
		return err == nil, nil
	}, fmt.Sprintf("error retrieving role: %v", err))
	AssertEventually(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.AuthV1().RoleBinding().Get(ctx, &api.ObjectMeta{Name: "NetworkAdminRoleBinding", Tenant: globals.DefaultTenant})
		return err == nil, nil
	}, fmt.Sprintf("error retrieving role binding: %v", err))
	MustDeleteRoleBinding(tinfo.apicl, "NetworkAdminRoleBinding", globals.DefaultTenant)
	MustDeleteRole(tinfo.apicl, "NetworkAdminRole", globals.DefaultTenant)
}

func TestFailedOpsInCommitBuffer(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "error creating logged in context")
	AssertEventually(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.StagingV1().Buffer().Create(ctx, &staging.Buffer{ObjectMeta: api.ObjectMeta{Name: "TestBuffer", Tenant: globals.DefaultTenant}})
		return err == nil, nil
	}, fmt.Sprintf("unable to create staging buffer, err: %v", err))
	defer tinfo.restcl.StagingV1().Buffer().Delete(ctx, &api.ObjectMeta{Name: "TestBuffer", Tenant: globals.DefaultTenant})
	stagecl, err := apiclient.NewStagedRestAPIClient(tinfo.apiGwAddr, "TestBuffer")
	AssertOk(t, err, "error creating staging client")
	defer stagecl.Close()
	// stage user with weak password
	MustCreateUserWithCtx(ctx, stagecl, "testuser2", "weakpassword", globals.DefaultTenant)
	AssertEventually(t, func() (bool, interface{}) {
		buf, err := tinfo.restcl.StagingV1().Buffer().Get(ctx, &api.ObjectMeta{Name: "TestBuffer", Tenant: globals.DefaultTenant})
		return err == nil && buf.Status.ValidationResult == staging.BufferStatus_FAILED.String(), buf
	}, "GET of staging buffer should have FAILED validation status")
	ca := staging.CommitAction{}
	ca.Name = "TestBuffer"
	ca.Tenant = globals.DefaultTenant
	_, err = tinfo.restcl.StagingV1().Buffer().Commit(ctx, &ca)
	Assert(t, err != nil, "expected error while committing buffer with weak password user")
}

func TestCommitBufferAuthz(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	const tuser = "tuser"
	const testTenant = "testtenant"

	MustCreateTenant(tinfo.apicl, testTenant)
	defer MustDeleteTenant(tinfo.apicl, testTenant)
	MustCreateTestUser(tinfo.apicl, tuser, testPassword, globals.DefaultTenant)
	defer MustDeleteUser(tinfo.apicl, tuser, globals.DefaultTenant)

	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, &auth.PasswordCredential{Username: tuser, Password: testPassword, Tenant: globals.DefaultTenant})
	AssertOk(t, err, "error creating logged in context")
	// user should have implicit permission to create staging buffer
	AssertEventually(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.StagingV1().Buffer().Create(ctx, &staging.Buffer{ObjectMeta: api.ObjectMeta{Name: "TestBuffer", Tenant: globals.DefaultTenant}})
		return err == nil, nil
	}, fmt.Sprintf("unable to create staging buffer, err: %v", err))
	defer tinfo.apicl.StagingV1().Buffer().Delete(ctx, &api.ObjectMeta{Name: "TestBuffer", Tenant: globals.DefaultTenant})
	_, err = tinfo.restcl.StagingV1().Buffer().Create(ctx, &staging.Buffer{ObjectMeta: api.ObjectMeta{Name: "TestBuffer", Tenant: testTenant}})
	Assert(t, err != nil, "user should not be able to create staging buffer in another tenant")
	// user should have implicit permission to clear staging buffer
	AssertEventually(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.StagingV1().Buffer().Clear(ctx, &staging.ClearAction{ObjectMeta: api.ObjectMeta{Name: "TestBuffer", Tenant: globals.DefaultTenant}})
		return err == nil, nil
	}, fmt.Sprintf("unable to clear staging buffer, err: %v", err))
	stagecl, err := apiclient.NewStagedRestAPIClient(tinfo.apiGwAddr, "TestBuffer")
	AssertOk(t, err, "error creating staging client")
	defer stagecl.Close()
	_, err = stagecl.AuthV1().Role().Create(ctx, login.NewRole("NetworkAdminRole", globals.DefaultTenant, login.NewPermission(
		globals.DefaultTenant,
		string(apiclient.GroupNetwork),
		string(network.KindNetwork),
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_AllActions.String()),
		login.NewPermission(
			globals.DefaultTenant,
			string(apiclient.GroupAuth),
			authz.ResourceKindAll,
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String())))
	Assert(t, err != nil, "user should not be authorized to add role to staging buffer")
	MustCreateRole(tinfo.apicl, "authrole", globals.DefaultTenant, login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), authz.ResourceKindAll, authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()))
	defer MustDeleteRole(tinfo.apicl, "authrole", globals.DefaultTenant)
	MustCreateRoleBinding(tinfo.apicl, "authrolebinding", globals.DefaultTenant, "authrole", []string{tuser}, nil)
	defer MustDeleteRoleBinding(tinfo.apicl, "authrolebinding", globals.DefaultTenant)
	tuser1 := &auth.User{}
	tuser1.Name = "tuser1"
	tuser1.Tenant = globals.DefaultTenant
	tuser1.Spec.Password = testPassword
	tuser1.Spec.Type = auth.UserSpec_Local.String()
	AssertEventually(t, func() (bool, interface{}) {
		_, err = stagecl.AuthV1().User().Create(ctx, tuser1)
		return err == nil, nil
	}, fmt.Sprintf("tuser should be able to add create user operation to staging buffer, err: %v", err))
	// user should have implicit permission to commit staging buffer
	AssertEventually(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.StagingV1().Buffer().Commit(ctx, &staging.CommitAction{ObjectMeta: api.ObjectMeta{Name: "TestBuffer", Tenant: globals.DefaultTenant}})
		return err == nil, nil
	}, fmt.Sprintf("unable to commit staging buffer, err: %v", err))
	MustDeleteUser(tinfo.apicl, tuser1.Name, tuser1.Tenant)
}

func TestClusterScopedAuthz(t *testing.T) {
	// create default tenant
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, adminCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, adminCred, tinfo.l)
	// create user with Host access
	const (
		testhost    = "testhost"
		clusterrole = "cluster-role"
		clusterrb   = "cluster-rb"
	)
	MustCreateTestUser(tinfo.apicl, testhost, testPassword, globals.DefaultTenant)
	defer MustDeleteUser(tinfo.apicl, testhost, globals.DefaultTenant)
	MustCreateRole(tinfo.apicl, "cluster-role", globals.DefaultTenant,
		login.NewPermission(globals.DefaultTenant, string(apiclient.GroupCluster), string(cluster.KindHost), authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String()))
	defer MustDeleteRole(tinfo.apicl, clusterrole, globals.DefaultTenant)
	MustCreateRoleBinding(tinfo.apicl, clusterrb, globals.DefaultTenant, clusterrole, []string{testhost}, nil)
	defer MustDeleteRoleBinding(tinfo.apicl, clusterrb, globals.DefaultTenant)
	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, &auth.PasswordCredential{Username: testhost, Password: testPassword, Tenant: globals.DefaultTenant})
	AssertOk(t, err, "error creating logged in context for testhost user")
	AssertEventually(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.ClusterV1().Host().List(ctx, &api.ListWatchOptions{})
		return err == nil, err
	}, "expected testhost user to retrieve Host list")

	// shouldn't be able to create cluster scoped permissions for non-default tenant user
	const testTenant = "testtenant"
	// create testtenant and admin user
	MustCreateTenant(tinfo.apicl, testTenant)
	defer MustDeleteTenant(tinfo.apicl, testTenant)
	MustCreateTestUser(tinfo.apicl, testUser, testPassword, testTenant)
	defer MustDeleteUser(tinfo.apicl, testUser, testTenant)
	MustUpdateRoleBinding(tinfo.apicl, globals.AdminRoleBinding, testTenant, globals.AdminRole, []string{testUser}, nil)
	defer MustUpdateRoleBinding(tinfo.apicl, globals.AdminRoleBinding, testTenant, globals.AdminRole, nil, nil)
	// creating tenant scoped permission for cluster scoped resource kind should fail validation
	role := &auth.Role{}
	role.Name = "invalidRole"
	role.Defaults("all")
	role.Spec.Permissions = []auth.Permission{login.NewPermission(testTenant, string(apiclient.GroupCluster), string(cluster.KindHost), authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())}
	_, err = tinfo.restcl.AuthV1().Role().Create(ctx, role)
	Assert(t, err != nil && strings.Contains(err.Error(), "Code(400)"), "permission for cluster scoped resource kind shouldn't have non-default tenant")

	ctx, err = NewLoggedInContext(context.Background(), tinfo.apiGwAddr, &auth.PasswordCredential{Username: testUser, Password: testPassword, Tenant: testTenant})
	AssertOk(t, err, "error creating logged in context for testtenant admin user")
	role = &auth.Role{}
	role.Name = "invalidRole"
	role.Defaults("all")
	role.Tenant = testTenant
	role.Spec.Permissions = []auth.Permission{login.NewPermission("", string(apiclient.GroupCluster), string(cluster.KindHost), authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())}
	_, err = tinfo.restcl.AuthV1().Role().Create(ctx, role)
	Assert(t, err != nil, "non-default tenant user cannot have permissions for cluster scoped resource kind")
	ctx, err = NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context for admin user")
	_, err = tinfo.restcl.AuthV1().Role().Create(ctx, role)
	Assert(t, err != nil, "roles in non-default tenant cannot have permissions for cluster scoped resource kind")
}

func TestLabelAuthorization(t *testing.T) {
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, adminCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, adminCred, tinfo.l)

	// create testUser2 as user with create permissions
	MustCreateTestUser(tinfo.apicl, "testUser2", testPassword, globals.DefaultTenant)
	defer MustDeleteUser(tinfo.apicl, "testUser2", globals.DefaultTenant)
	MustCreateRole(tinfo.apicl, "UserCreateRole", globals.DefaultTenant,
		login.NewPermission(
			globals.DefaultTenant,
			string(apiclient.GroupAuth),
			authz.ResourceKindAll,
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_Create.String()))
	defer MustDeleteRole(tinfo.apicl, "UserCreateRole", globals.DefaultTenant)
	MustCreateRoleBinding(tinfo.apicl, "UserCreateRoleBinding", globals.DefaultTenant, "UserCreateRole", []string{"testUser2"}, nil)
	defer MustDeleteRoleBinding(tinfo.apicl, "UserCreateRoleBinding", globals.DefaultTenant)

	// login as user
	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, &auth.PasswordCredential{
		Username: "testUser2",
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	})
	AssertOk(t, err, "error creating logged in context")
	// labeling role should result in authorization error at this point
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Label(ctx, &api.Label{ObjectMeta: api.ObjectMeta{Name: "UserUpdateRole", Tenant: globals.DefaultTenant, Labels: map[string]string{"label": "value"}}})
		return err != nil, nil
	}, "authorization error expected while labeling role", "100ms", "1s")

	// Add update permissions
	MustCreateRole(tinfo.apicl, "UserUpdateRole", globals.DefaultTenant,
		login.NewPermission(
			globals.DefaultTenant,
			string(apiclient.GroupAuth),
			authz.ResourceKindAll,
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_Update.String()))
	defer MustDeleteRole(tinfo.apicl, "UserUpdateRole", globals.DefaultTenant)
	MustCreateRoleBinding(tinfo.apicl, "UserUpdateRoleBinding", globals.DefaultTenant, "UserUpdateRole", []string{"testUser2"}, nil)
	defer MustDeleteRoleBinding(tinfo.apicl, "UserUpdateRoleBinding", globals.DefaultTenant)

	// labeling role should succeed now
	AssertEventually(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.AuthV1().Role().Label(ctx, &api.Label{ObjectMeta: api.ObjectMeta{Name: "UserUpdateRole", Tenant: globals.DefaultTenant, Labels: map[string]string{"label": "value"}}})
		return err == nil, err
	}, fmt.Sprintf("error while labeling role: %v", err))
}

func TestValidatePerms(t *testing.T) {
	// create default tenant
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, adminCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, adminCred, tinfo.l)
	// create testtenant
	MustCreateTenant(tinfo.apicl, testTenant)
	defer MustDeleteTenant(tinfo.apicl, testTenant)
	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	tests := []struct {
		name   string
		perms  []auth.Permission
		valid  bool
		tenant string
		errmsg string
	}{
		{
			name:   "empty kind, tenant and group",
			perms:  []auth.Permission{login.NewPermission("", "", "", "", "", auth.Permission_AllActions.String())},
			valid:  false,
			tenant: adminCred.Tenant,
			errmsg: "invalid API group [\"\"]",
		},
		{
			name:   "empty kind, tenant and invalid group",
			perms:  []auth.Permission{login.NewPermission("", "zzz", "", "", "", auth.Permission_AllActions.String())},
			valid:  false,
			tenant: adminCred.Tenant,
			errmsg: "invalid API group [\"zzz\"]",
		},
		{
			name:   "empty kind, tenant and valid group",
			perms:  []auth.Permission{login.NewPermission("", string(apiclient.GroupAuth), "", "", "", auth.Permission_AllActions.String())},
			valid:  true,
			tenant: adminCred.Tenant,
			errmsg: "",
		},
		{
			name:   "non matching kind and group",
			perms:  []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), string(network.KindNetwork), "", "", auth.Permission_AllActions.String())},
			valid:  false,
			tenant: adminCred.Tenant,
			errmsg: fmt.Sprintf("invalid resource kind [%q] and API group [%q]", string(network.KindNetwork), string(apiclient.GroupAuth)),
		},
		{
			name:   "empty tenant and tenant scoped kind",
			perms:  []auth.Permission{login.NewPermission("", string(apiclient.GroupAuth), string(auth.KindRole), "", "", auth.Permission_AllActions.String())},
			valid:  true,
			tenant: adminCred.Tenant,
			errmsg: "",
		},
		{
			name:   "default tenant and cluster scoped kind",
			perms:  []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindAuthenticationPolicy), "", "", auth.Permission_AllActions.String())},
			valid:  true,
			tenant: adminCred.Tenant,
			errmsg: "",
		},
		{
			name:   "non default tenant and cluster scoped kind",
			perms:  []auth.Permission{login.NewPermission(testTenant, string(apiclient.GroupAuth), string(auth.KindAuthenticationPolicy), "", "", auth.Permission_AllActions.String())},
			valid:  false,
			tenant: adminCred.Tenant,
			errmsg: fmt.Sprintf("tenant should be empty or [%q] for cluster scoped resource kind [%q]", globals.DefaultTenant, string(auth.KindAuthenticationPolicy)),
		},
		{
			name:   "empty kind, and all API groups giving permissions to cluster and tenant scoped objects",
			perms:  []auth.Permission{login.NewPermission("", authz.ResourceGroupAll, "", "", "", auth.Permission_AllActions.String())},
			valid:  true,
			tenant: adminCred.Tenant,
			errmsg: "",
		},
		{
			name:   "all kinds, and all API groups giving permissions to cluster and tenant scoped objects",
			perms:  []auth.Permission{login.NewPermission("", authz.ResourceGroupAll, authz.ResourceKindAll, "", "", auth.Permission_AllActions.String())},
			valid:  true,
			tenant: adminCred.Tenant,
			errmsg: "",
		},
		{
			name:   "all kinds, and all API groups giving permissions to tenant scoped objects",
			perms:  []auth.Permission{login.NewPermission(testTenant, authz.ResourceGroupAll, authz.ResourceKindAll, "", "", auth.Permission_AllActions.String())},
			valid:  true,
			tenant: testTenant,
			errmsg: "",
		},
		{
			name:   "non existent tenant",
			perms:  []auth.Permission{login.NewPermission("All", authz.ResourceGroupAll, authz.ResourceKindAll, "", "", auth.Permission_AllActions.String())},
			valid:  false,
			tenant: "All",
			errmsg: "Commit to backend failed: KVError: Key not found, key: /venice/config/cluster/tenants/All, version: 0",
		},
		{
			name:   "all tenants",
			perms:  []auth.Permission{login.NewPermission(authz.ResourceTenantAll, authz.ResourceGroupAll, authz.ResourceKindAll, "", "", auth.Permission_AllActions.String())},
			valid:  true,
			tenant: authz.ResourceTenantAll,
			errmsg: "",
		},
		{
			name:   "all tenants for a specific tenant scoped kind",
			perms:  []auth.Permission{login.NewPermission(authz.ResourceTenantAll, string(apiclient.GroupAuth), string(auth.KindRole), "", "", auth.Permission_AllActions.String())},
			valid:  true,
			tenant: authz.ResourceTenantAll,
			errmsg: "",
		},
		{
			name:   "all tenants for a specific cluster scoped kind",
			perms:  []auth.Permission{login.NewPermission(authz.ResourceTenantAll, string(apiclient.GroupAuth), string(auth.KindAuthenticationPolicy), "", "", auth.Permission_AllActions.String())},
			valid:  false,
			tenant: authz.ResourceTenantAll,
			errmsg: fmt.Sprintf("tenant should be empty or [%q] for cluster scoped resource kind [%q]", globals.DefaultTenant, string(auth.KindAuthenticationPolicy)),
		},
		{
			name:   "role with nil permissions",
			valid:  false,
			tenant: "",
			errmsg: "role should contain at least one permission",
		},
		{
			name:   "all actions for fw logs",
			perms:  []auth.Permission{login.NewPermission(adminCred.Tenant, "", auth.Permission_FwlogsQuery.String(), "", "", auth.Permission_AllActions.String())},
			valid:  false,
			tenant: adminCred.Tenant,
			errmsg: fmt.Sprintf("invalid resource kind [%q] and action [%q]", auth.Permission_FwlogsQuery.String(), auth.Permission_AllActions.String()),
		},
		{
			name:   "read action for fw logs",
			perms:  []auth.Permission{login.NewPermission(adminCred.Tenant, "", auth.Permission_FwlogsQuery.String(), "", "", auth.Permission_Read.String())},
			valid:  true,
			tenant: adminCred.Tenant,
			errmsg: "",
		},
	}
	for i, test := range tests {
		role, err := tinfo.restcl.AuthV1().Role().Create(ctx, login.NewRole(fmt.Sprintf("testrole_%d", i), globals.DefaultTenant, test.perms...))
		if role != nil {
			for _, perm := range role.Spec.Permissions {
				Assert(t, perm.ResourceTenant == test.tenant, fmt.Sprintf("[%s] test failed, expected resource tenant [%s], got [%s]", test.name, test.tenant, perm.ResourceTenant))
			}
			MustDeleteRole(tinfo.apicl, role.Name, role.Tenant)
		}
		Assert(t, test.valid == (err == nil), fmt.Sprintf("[%s] test failed, expected perm validity to be [%v], got [%v], err [%v]", test.name, test.valid, err == nil, err))
		if !test.valid && err != nil {
			Assert(t, strings.Contains(err.Error(), test.errmsg), fmt.Sprintf("[%s] test failed, expected error to be [%v], got [%v]]", test.name, test.errmsg, err.Error()))
		}
	}
}
