package impl

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"sort"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/ldap"
	"github.com/pensando/sw/venice/utils/authn/manager"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/bootstrapper"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	testNetworkAdminRole = login.NewRole("NetworkAdmin", "testTenant", login.NewPermission(
		"testTenant",
		string(apiclient.GroupNetwork),
		string(network.KindNetwork),
		authz.ResourceNamespaceAll,
		"network1,network2",
		fmt.Sprintf("%s,%s,%s", auth.Permission_Create.String(), auth.Permission_Update.String(), auth.Permission_Delete.String())),
		login.NewPermission(
			"testTenant",
			string(apiclient.GroupNetwork),
			string(network.KindLbPolicy),
			authz.ResourceNamespaceAll,
			"",
			fmt.Sprintf("%s,%s,%s", auth.Permission_Create.String(), auth.Permission_Update.String(), auth.Permission_Delete.String())))

	testNetworkAdminRoleBinding = login.NewRoleBinding("NetworkAdminRoleBinding", "testTenant", "NetworkAdmin", "testuser", "")
)

func sortOperations(operations []authz.Operation) {
	sort.Slice(operations, func(i, j int) bool {
		if operations[i] == nil {
			return true
		}
		if operations[j] == nil {
			return false
		}
		return fmt.Sprintf("%s%s", operations[i].GetResource().GetName(), operations[i].GetAction()) < fmt.Sprintf("%s%s", operations[j].GetResource().GetName(), operations[j].GetAction())
	})
}

func areOperationsEqual(expected []authz.Operation, returned []authz.Operation) bool {
	sortOperations(expected)
	sortOperations(returned)
	return reflect.DeepEqual(expected, returned)
}

func TestPrivilegeEscalationCheck(t *testing.T) {
	tests := []struct {
		name               string
		in                 interface{}
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
		{
			name: "when creating role",
			in:   testNetworkAdminRole,
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_Create.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_Create.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_Update.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_Update.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_Delete.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_Delete.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindLbPolicy),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_Create.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindLbPolicy),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_Update.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindLbPolicy),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_Delete.String()),
			},
			out: testNetworkAdminRole,
			err: false,
		},
		{
			name: "when creating role binding",
			in:   testNetworkAdminRoleBinding,
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_Create.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_Create.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_Update.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_Update.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_Delete.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_Delete.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindLbPolicy),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_Create.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindLbPolicy),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_Update.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindLbPolicy),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_Delete.String()),
			},
			out: testNetworkAdminRoleBinding,
			err: false,
		},
		{
			name:               "incorrect object type",
			in:                 struct{ name string }{"testing"},
			expectedOperations: nil,
			out:                struct{ name string }{"testing"},
			err:                true,
		},
		{
			name:               "incorrect role name in role binding",
			in:                 login.NewRoleBinding("IncorrectRoleBinding", "testTenant", "incorrectRoleName", "testUser", ""),
			expectedOperations: nil,
			out:                login.NewRoleBinding("IncorrectRoleBinding", "testTenant", "incorrectRoleName", "testUser", ""),
			err:                true,
		},
	}
	r := &authHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	r.logger = log.GetNewLogger(logConfig)
	r.permissionGetter = rbac.NewMockPermissionGetter([]*auth.Role{testNetworkAdminRole}, []*auth.RoleBinding{testNetworkAdminRoleBinding}, nil, nil)
	for _, test := range tests {
		nctx, out, err := r.privilegeEscalationCheck(context.TODO(), test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		operations, _ := apigwpkg.OperationsFromContext(nctx)
		Assert(t, areOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("unexpected operations, [%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", test.out, out, test.name))
	}
}

func TestAuthzRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerPrivilegeEscalationHook(svc)
	AssertOk(t, err, "privilege escalation hook registration failed")
	// test authz hooks
	ids := []serviceID{
		{"Role", apiserver.CreateOper},
		{"Role", apiserver.UpdateOper},
		{"RoleBinding", apiserver.CreateOper},
		{"RoleBinding", apiserver.UpdateOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for [%s] [%s]", id.kind, id.action))
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for [%s] [%s] profile", len(prof.PreAuthZHooks()), id.kind, id.action))
	}
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerPrivilegeEscalationHook(svc)
	Assert(t, err != nil, "expected error in privilege escalation hook registration")
}

func TestAuthnRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerAuthBootstrapHook(svc)
	AssertOk(t, err, "authBootstrap hook registration failed")

	// test authn hooks
	ids := []serviceID{
		{"AuthenticationPolicy", apiserver.CreateOper},
		{"AuthenticationPolicy", apiserver.UpdateOper},
		{"AuthenticationPolicy", apiserver.GetOper},
		{"User", apiserver.CreateOper},
		{"User", apiserver.GetOper},
		{"RoleBinding", apiserver.CreateOper},
		{"RoleBinding", apiserver.UpdateOper},
		{"RoleBinding", apiserver.GetOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		AssertOk(t, err, "error getting service profile for [%s] [%s]", id.kind, id.action)
		Assert(t, len(prof.PreAuthNHooks()) == 1, fmt.Sprintf("unexpected number of pre authn hooks [%d] for [%s] [%s] profile", len(prof.PreAuthNHooks()), id.kind, id.action))
	}
	methods := []string{"LdapConnectionCheck", "LdapBindCheck"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		AssertOk(t, err, "error getting service profile for method [%s]", method)
		Assert(t, len(prof.PreAuthNHooks()) == 1, fmt.Sprintf("unexpected number of pre authn hooks [%d] for [%s] profile", len(prof.PreAuthNHooks()), method))
	}
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerAuthBootstrapHook(svc)
	Assert(t, err != nil, "expected error in authBootstrap hook registration")
}

func TestAddRoles(t *testing.T) {
	tests := []struct {
		name string
		in   interface{}
		err  bool
	}{
		{
			name: "User object",
			in: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testuser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			err: false,
		},
		{
			name: "User list",
			in: &auth.UserList{
				Items: []*auth.User{
					{
						TypeMeta: api.TypeMeta{Kind: "User"},
						ObjectMeta: api.ObjectMeta{
							Tenant: "testTenant",
							Name:   "testuser",
						},
						Spec: auth.UserSpec{
							Fullname: "Test User",
							Password: "password",
							Email:    "testuser@pensandio.io",
							Type:     auth.UserSpec_Local.String(),
						},
					},
				},
			},
			err: false,
		},
		{
			name: "invalid object",
			in:   &struct{ name string }{name: "invalid object type"},
			err:  true,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	r.permissionGetter = rbac.NewMockPermissionGetter([]*auth.Role{testNetworkAdminRole}, []*auth.RoleBinding{testNetworkAdminRoleBinding}, nil, nil)
	for _, test := range tests {
		_, out, err := r.addRoles(context.TODO(), test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		switch obj := out.(type) {
		case *auth.User:
			Assert(t, len(obj.Status.Roles) == 1 && obj.Status.Roles[0] == "NetworkAdmin", "user should have network admin role")
		case *auth.UserList:
			for _, user := range obj.GetItems() {
				Assert(t, len(user.Status.Roles) == 1 && user.Status.Roles[0] == "NetworkAdmin", "user should have network admin role")
			}
		}

	}
}

func TestAddRolesHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerAddRolesHook(svc)
	AssertOk(t, err, "addRoles hook registration failed")

	opers := []apiserver.APIOperType{apiserver.CreateOper, apiserver.UpdateOper, apiserver.DeleteOper, apiserver.GetOper, apiserver.ListOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("User", oper)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for oper :%v", oper))
		Assert(t, len(prof.PostCallHooks()) == 1, fmt.Sprintf("unexpected number of post call hooks [%d] for User operation [%v]", len(prof.PostCallHooks()), oper))
	}
	// test err
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerAddRolesHook(svc)
	Assert(t, err != nil, "expected error in addRoles hook registration")
}

func TestAdminRoleCheck(t *testing.T) {
	tests := []struct {
		name     string
		in       interface{}
		out      interface{}
		skipCall bool
		err      error
	}{
		{
			name: "super admin role",
			in: login.NewClusterRole(globals.AdminRole, login.NewPermission(
				authz.ResourceTenantAll,
				authz.ResourceGroupAll,
				authz.ResourceKindAll,
				authz.ResourceNamespaceAll,
				"",
				auth.Permission_AllActions.String())),
			out: login.NewClusterRole(globals.AdminRole, login.NewPermission(
				authz.ResourceTenantAll,
				authz.ResourceGroupAll,
				authz.ResourceKindAll,
				authz.ResourceNamespaceAll,
				"",
				auth.Permission_AllActions.String())),
			skipCall: true,
			err:      errors.New("admin role create, update or delete is not allowed"),
		},
		{
			name:     "network admin role",
			in:       testNetworkAdminRole,
			out:      testNetworkAdminRole,
			skipCall: false,
			err:      nil,
		},
		{
			name:     "incorrect object type",
			in:       struct{ name string }{"testing"},
			out:      struct{ name string }{"testing"},
			skipCall: true,
			err:      errors.New("invalid input type"),
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	for _, test := range tests {
		_, out, skipCall, err := r.adminRoleCheck(context.TODO(), test.in)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, skipCall == test.skipCall, fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(out, test.out), fmt.Sprintf("[%s] test failed", test.name))
	}
}

func TestAdminRoleCheckHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerAdminRoleCheckHook(svc)
	AssertOk(t, err, "adminRoleCheck hook registration failed")

	ids := []serviceID{
		{"Role", apiserver.CreateOper},
		{"Role", apiserver.UpdateOper},
		{"Role", apiserver.DeleteOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		AssertOk(t, err, "error getting service profile for [%s] [%s]", id.kind, id.action)
		Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre-call hooks [%d] for [%s] [%s] profile", len(prof.PreCallHooks()), id.kind, id.action))
	}
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerAdminRoleCheckHook(svc)
	Assert(t, err != nil, "expected error in adminRoleCheck hook registration")
}

func TestUserCreateCheck(t *testing.T) {
	tests := []struct {
		name       string
		authGetter manager.AuthGetter
		skipCall   bool
		err        error
	}{
		{
			name:       "error in fetching auth policy",
			authGetter: manager.NewMockAuthGetter(nil, true),
			skipCall:   true,
			err:        errors.New("authentication policy not found"),
		},
		{
			name: "local auth disabled",
			authGetter: manager.NewMockAuthGetter(&auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "AuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Enabled: true,
						},
						Local: &auth.Local{
							Enabled: false,
						},
						AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
					},
				},
			}, false),
			skipCall: true,
			err:      errors.New("local authentication not enabled"),
		},
		{
			name: "local auth enabled",
			authGetter: manager.NewMockAuthGetter(&auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "AuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Enabled: true,
						},
						Local: &auth.Local{
							Enabled: true,
						},
						AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
					},
				},
			}, false),
			skipCall: false,
			err:      nil,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	for _, test := range tests {
		r.authGetter = test.authGetter
		_, _, skipCall, err := r.userCreateCheck(context.TODO(), &auth.User{})
		Assert(t, skipCall == test.skipCall, fmt.Sprintf("expected skipCall [%v], got [%v], [%s] test failed", test.skipCall, skipCall, test.name))
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("expected err [%v], got [%v], %s] test failed", test.err, err, test.name))
	}
}

func TestUserCreateCheckHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerUserCreateCheckHook(svc)
	AssertOk(t, err, "userCreateCheck hook registration failed")

	ids := []serviceID{
		{"User", apiserver.CreateOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		AssertOk(t, err, "error getting service profile for [%s] [%s]", id.kind, id.action)
		Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre-call hooks [%d] for [%s] [%s] profile", len(prof.PreCallHooks()), id.kind, id.action))
	}
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerUserCreateCheckHook(svc)
	Assert(t, err != nil, "expected error in userCreateCheck hook registration")
}

func TestAuthBootstrap(t *testing.T) {
	tests := []struct {
		name         string
		in           interface{}
		bootstrapper bootstrapper.Bootstrapper
		skipAuth     bool
		err          error
	}{
		{
			name: "non default user tenant",
			in: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testuser",
				},
			},
			bootstrapper: bootstrapper.NewMockBootstrapper(false),
			skipAuth:     false,
			err:          nil,
		},
		{
			name: "non default role binding tenant",
			in: &auth.RoleBinding{
				TypeMeta: api.TypeMeta{Kind: "RoleBinding"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "AdminRoleBinding",
				},
			},
			bootstrapper: bootstrapper.NewMockBootstrapper(false),
			skipAuth:     false,
			err:          nil,
		},
		{
			name:         "invalid input type",
			in:           struct{ name string }{"testing"},
			bootstrapper: bootstrapper.NewMockBootstrapper(false),
			skipAuth:     false,
			err:          errors.New("invalid input type"),
		},
		{
			name: "auth policy",
			in: &auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "AuthenticationPolicy",
				},
			},
			bootstrapper: bootstrapper.NewMockBootstrapper(false),
			skipAuth:     false,
			err:          nil,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	for _, test := range tests {
		r.bootstrapper = test.bootstrapper
		_, _, skipAuth, err := r.authBootstrap(context.TODO(), test.in)
		Assert(t, skipAuth == test.skipAuth, fmt.Sprintf("[%s] test failed, expected skipAuth [%v], got [%v]", test.name, test.skipAuth, skipAuth))
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
	}

}

func TestLdapConnectionCheck(t *testing.T) {
	authpolicy := &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
					Servers: []*auth.LdapServer{
						{
							Url: "localhost:389",
							TLSOptions: &auth.TLSOptions{
								StartTLS:                   true,
								SkipServerCertVerification: false,
								ServerName:                 "testserver",
								TrustedCerts:               "certs",
							},
						},
					},

					BaseDN:       "DC=io,DC=pensando",
					BindDN:       "DN=admin",
					BindPassword: "password",
					AttributeMapping: &auth.LdapAttributeMapping{
						User:             "samAccount",
						UserObjectClass:  "user",
						Group:            "group",
						GroupObjectClass: "ou",
					},
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
		},
	}
	tests := []struct {
		name        string
		in          interface{}
		connChecker ldap.ConnectionChecker
		status      *auth.LdapServerStatus
		err         error
	}{
		{
			name:        "invalid object",
			in:          &struct{ name string }{name: "invalid object type"},
			connChecker: ldap.NewMockConnectionChecker(ldap.SuccessfulAuth),
			status:      nil,
			err:         errors.New("invalid input type"),
		},
		{
			name:        "successful ldap connection",
			in:          authpolicy,
			connChecker: ldap.NewMockConnectionChecker(ldap.SuccessfulAuth),
			status: &auth.LdapServerStatus{
				Result: auth.LdapServerStatus_Connect_Success.String(),
			},
			err: nil,
		},
		{
			name:        "failed ldap connection",
			in:          authpolicy,
			connChecker: ldap.NewMockConnectionChecker(ldap.ConnectionError),
			status: &auth.LdapServerStatus{
				Result: auth.LdapServerStatus_Connect_Failure.String(),
			},
			err: nil,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	for _, test := range tests {
		r.ldapChecker = test.connChecker
		_, out, skipCall, err := r.ldapConnectionCheck(context.TODO(), test.in)
		Assert(t, skipCall, fmt.Sprintf("[%s] test failed, expected skipCall to be true", test.name))
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		if err == nil {
			policy, ok := out.(*auth.AuthenticationPolicy)
			Assert(t, ok, fmt.Sprintf("[%s] test failed, expected AuthenticationPolicy object", test.name))
			Assert(t, len(policy.Status.LdapServers) == 1, fmt.Sprintf("[%s] test failed, unexpected number of ldap servers", test.name))
			Assert(t, policy.Status.LdapServers[0].Result == test.status.Result,
				fmt.Sprintf("[%s] test failed, expected result [%s], got [%s]", test.name, test.status.Result, policy.Status.LdapServers[0].Result))
		}
	}
}

func TestLdapBindCheck(t *testing.T) {
	authpolicy := &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
					Servers: []*auth.LdapServer{
						{
							Url: "localhost:389",
							TLSOptions: &auth.TLSOptions{
								StartTLS:                   true,
								SkipServerCertVerification: false,
								ServerName:                 "testserver",
								TrustedCerts:               "certs",
							},
						},
					},

					BaseDN:       "DC=io,DC=pensando",
					BindDN:       "DN=admin",
					BindPassword: "password",
					AttributeMapping: &auth.LdapAttributeMapping{
						User:             "samAccount",
						UserObjectClass:  "user",
						Group:            "group",
						GroupObjectClass: "ou",
					},
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
		},
	}
	tests := []struct {
		name        string
		in          interface{}
		connChecker ldap.ConnectionChecker
		status      *auth.LdapServerStatus
		err         error
	}{
		{
			name:        "invalid object",
			in:          &struct{ name string }{name: "invalid object type"},
			connChecker: ldap.NewMockConnectionChecker(ldap.SuccessfulAuth),
			status:      nil,
			err:         errors.New("invalid input type"),
		},
		{
			name:        "failed ldap bind",
			in:          authpolicy,
			connChecker: ldap.NewMockConnectionChecker(ldap.IncorrectBindDN),
			status: &auth.LdapServerStatus{
				Result: auth.LdapServerStatus_Bind_Failure.String(),
			},
			err: nil,
		},
		{
			name:        "successful ldap bind",
			in:          authpolicy,
			connChecker: ldap.NewMockConnectionChecker(ldap.SuccessfulAuth),
			status: &auth.LdapServerStatus{
				Result: auth.LdapServerStatus_Bind_Success.String(),
			},
			err: nil,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	for _, test := range tests {
		r.ldapChecker = test.connChecker
		_, out, skipCall, err := r.ldapBindCheck(context.TODO(), test.in)
		Assert(t, skipCall, fmt.Sprintf("[%s] test failed, expected skipCall to be true", test.name))
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		if err == nil {
			policy, ok := out.(*auth.AuthenticationPolicy)
			Assert(t, ok, fmt.Sprintf("[%s] test failed, expected AuthenticationPolicy object", test.name))
			Assert(t, len(policy.Status.LdapServers) == 1, fmt.Sprintf("[%s] test failed, unexpected number of ldap servers", test.name))
			Assert(t, policy.Status.LdapServers[0].Result == test.status.Result,
				fmt.Sprintf("[%s] test failed, expected result [%s], got [%s]", test.name, test.status.Result, policy.Status.LdapServers[0].Result))
		}
	}
}

func TestAddOwner(t *testing.T) {
	tests := []struct {
		name               string
		in                 interface{}
		operations         []authz.Operation
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
		{
			name: "password change",
			in:   &auth.PasswordChangeRequest{},
			operations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupAuth), string(auth.KindUser),
					"", "test"),
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResourceWithOwner(globals.DefaultTenant,
					string(apiclient.GroupAuth), string(auth.KindUser),
					"", "test",
					&auth.User{
						ObjectMeta: api.ObjectMeta{Name: "test", Tenant: globals.DefaultTenant},
					}),
					auth.Permission_Create.String()),
			},
			out: &auth.PasswordChangeRequest{},
			err: false,
		},
		{
			name: "password reset",
			in:   &auth.PasswordResetRequest{},
			operations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupAuth), string(auth.KindUser),
					"", "test"),
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResourceWithOwner(globals.DefaultTenant,
					string(apiclient.GroupAuth), string(auth.KindUser),
					"", "test",
					&auth.User{
						ObjectMeta: api.ObjectMeta{Name: "test", Tenant: globals.DefaultTenant},
					}),
					auth.Permission_Create.String()),
			},
			out: &auth.PasswordResetRequest{},
			err: false,
		},
		{
			name: "user update",
			in:   &auth.User{},
			operations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupAuth), string(auth.KindUser),
					"", "test"),
					auth.Permission_Update.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResourceWithOwner(globals.DefaultTenant,
					string(apiclient.GroupAuth), string(auth.KindUser),
					"", "test",
					&auth.User{
						ObjectMeta: api.ObjectMeta{Name: "test", Tenant: globals.DefaultTenant},
					}),
					auth.Permission_Update.String()),
			},
			out: &auth.User{},
			err: false,
		},
		{
			name: "invalid object",
			in:   &struct{ name string }{name: "invalid object type"},
			operations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupAuth), string(auth.KindUser),
					"", "test"),
					auth.Permission_Update.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupAuth), string(auth.KindUser),
					"", "test"),
					auth.Permission_Update.String()),
			},
			out: &struct{ name string }{name: "invalid object type"},
			err: true,
		},
		{
			name:               "nil operations slice",
			in:                 &auth.User{},
			operations:         nil,
			expectedOperations: nil,
			out:                &auth.User{},
			err:                true,
		},
		{
			name:               "nil operation",
			in:                 &auth.User{},
			operations:         []authz.Operation{nil, nil},
			expectedOperations: []authz.Operation{nil, nil},
			out:                &auth.User{},
			err:                true,
		},
		{
			name: "nil resource",
			in:   &auth.PasswordResetRequest{},
			operations: []authz.Operation{
				authz.NewOperation(nil,
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(nil,
					auth.Permission_Create.String()),
			},
			out: &auth.PasswordResetRequest{},
			err: true,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	for _, test := range tests {
		ctx := apigwpkg.NewContextWithOperations(context.TODO(), test.operations...)
		nctx, out, err := r.addOwner(ctx, test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		operations, _ := apigwpkg.OperationsFromContext(nctx)
		Assert(t, areOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("unexpected operations, [%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", test.out, out, test.name))
	}
}

func TestAddOwnerHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerAddOwnerHook(svc)
	AssertOk(t, err, "addOwner hook registration failed")

	opers := []apiserver.APIOperType{apiserver.UpdateOper, apiserver.GetOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("User", oper)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for oper :%v", oper))
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for User operation [%v]", len(prof.PreAuthZHooks()), oper))
	}
	methods := []string{"PasswordChange", "PasswordReset"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for method [%s]", method))
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for method [%s]", len(prof.PreAuthZHooks()), method))
	}
	// test err
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerAddOwnerHook(svc)
	Assert(t, err != nil, "expected error in addOwner hook registration")
}
