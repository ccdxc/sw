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

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/network"
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
	MustCreateAuthenticationPolicy(tinfo.restcl, &auth.Local{Enabled: true}, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false})
	defer MustDeleteAuthenticationPolicy(tinfo.apicl)
	AssertConsistently(t, func() (bool, interface{}) {
		clusterObj, err := tinfo.restcl.ClusterV1().Cluster().AuthBootstrapComplete(context.TODO(), &cluster.ClusterAuthBootstrapRequest{})
		tinfo.l.Infof("set bootstrap flag error: %v", err)
		return err != nil, clusterObj
	}, "bootstrap flag shouldn't be set till role binding is created", "100ms", "1s")
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

func TestAPIGroupAuthorization(t *testing.T) {
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
}

func TestUserSelfOperations(t *testing.T) {
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

	MustCreateTestUser(tinfo.apicl, "testUser2", testPassword, testTenant)
	defer MustDeleteUser(tinfo.apicl, "testUser2", testTenant)
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
			NewPassword: "newpassword",
		})
		return err == nil, err
	}, "unable to change password")
	ctx, err = NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, &auth.PasswordCredential{Username: "testUser2", Password: "newpassword", Tenant: testTenant})
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
}
