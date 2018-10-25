package auth

import (
	"context"
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
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
	if err := SetupAuth(tinfo.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "error creating logged in context")
	// retrieve auth policy
	AssertConsistently(t, func() (bool, interface{}) {
		_, err = tinfo.restcl.AuthV1().AuthenticationPolicy().Get(ctx, &api.ObjectMeta{Name: "AuthenticationPolicy"})
		return err != nil, nil
	}, "authorization error expected while retrieving authentication policy")
	// retrieve tenant
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.ClusterV1().Tenant().Get(ctx, &api.ObjectMeta{Name: testTenant})
		return err != nil, nil
	}, "authorization error expected while retrieving tenant")
	// retrieve role
	var role *auth.Role
	AssertEventually(t, func() (bool, interface{}) {
		role, err = tinfo.restcl.AuthV1().Role().Get(ctx, &api.ObjectMeta{Name: globals.AdminRole, Tenant: testTenant})
		return err == nil, nil
	}, fmt.Sprintf("error while retrieving role: Err: %v", err))
	Assert(t, role.Name == globals.AdminRole && role.Tenant == testTenant, fmt.Sprintf("incorrect role retrieved [%#v]", role))
}

func TestAdminRole(t *testing.T) {
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, adminCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, adminCred, tinfo.l)

	superAdminCtx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")

	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Delete(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRole, Tenant: globals.DefaultTenant})
		tinfo.l.Infof("admin role delete error: %v", err)
		return err != nil, err
	}, "admin role shouldn't be deleted")
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Create(superAdminCtx, login.NewRole(globals.AdminRole, globals.DefaultTenant, login.NewPermission(
			authz.ResourceTenantAll,
			authz.ResourceGroupAll,
			auth.Permission_AllResourceKinds.String(),
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_ALL_ACTIONS.String())))
		tinfo.l.Infof("admin role create error: %v", err)
		return err != nil, err
	}, "admin role shouldn't be created")
	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Update(superAdminCtx, login.NewRole(globals.AdminRole, globals.DefaultTenant, login.NewPermission(
			authz.ResourceTenantAll,
			authz.ResourceGroupAll,
			auth.Permission_AllResourceKinds.String(),
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_ALL_ACTIONS.String())))
		tinfo.l.Infof("admin role update error: %v", err)
		return err != nil, err
	}, "admin role shouldn't be updated")
	MustCreateTenant(tinfo.apicl, testTenant)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Get(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRole, Tenant: testTenant})
		return err == nil, err
	}, "admin role should be created when a tenant is created")
	MustDeleteTenant(tinfo.apicl, testTenant)
	AssertEventually(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Get(superAdminCtx, &api.ObjectMeta{Name: globals.AdminRole, Tenant: testTenant})
		return err != nil, err
	}, "admin role should be deleted when a tenant is deleted")
}

func TestPrivilegeEscalation(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	ctx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "error creating logged in context")

	AssertConsistently(t, func() (bool, interface{}) {
		_, err := tinfo.restcl.AuthV1().Role().Update(ctx, login.NewRole(globals.AdminRole, testTenant, login.NewPermission(
			authz.ResourceTenantAll,
			authz.ResourceGroupAll,
			auth.Permission_AllResourceKinds.String(),
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_ALL_ACTIONS.String())))
		tinfo.l.Infof("global admin role update error: %v", err)
		return err != nil, err
	}, "tenant admin should not be able to update tenant admin role to global admin role")

	// create testUser2 as network admin with permissions to create RBAC objects also
	MustCreateTestUser(tinfo.apicl, "testUser2", testPassword, testTenant)
	defer MustDeleteUser(tinfo.apicl, "testUser2", testTenant)
	MustCreateRole(tinfo.apicl, "NetworkAdminRole", testTenant, login.NewPermission(
		testTenant,
		"network",
		auth.Permission_Network.String(),
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_ALL_ACTIONS.String()),
		login.NewPermission(
			testTenant,
			"auth",
			auth.Permission_AllResourceKinds.String(),
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_ALL_ACTIONS.String()))
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
	}, "network admin should not be able to update its role binding to assign himself tenant admin role")
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
	}, "bootstrap flag shouldn't be set till auth policy is created")
	// create auth policy
	MustCreateAuthenticationPolicy(tinfo.restcl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false})
	defer MustDeleteAuthenticationPolicy(tinfo.apicl)
	AssertConsistently(t, func() (bool, interface{}) {
		clusterObj, err := tinfo.restcl.ClusterV1().Cluster().AuthBootstrapComplete(context.TODO(), &cluster.ClusterAuthBootstrapRequest{})
		tinfo.l.Infof("set bootstrap flag error: %v", err)
		return err != nil, clusterObj
	}, "bootstrap flag shouldn't be set till role binding is created")
	MustCreateTestUser(tinfo.restcl, testUser, testPassword, globals.DefaultTenant)
	defer MustDeleteUser(tinfo.apicl, testUser, globals.DefaultTenant)
	MustCreateRoleBinding(tinfo.restcl, "AdminRoleBinding", globals.DefaultTenant, globals.AdminRole, []string{testUser}, nil)
	defer MustDeleteRoleBinding(tinfo.apicl, "AdminRoleBinding", globals.DefaultTenant)
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
