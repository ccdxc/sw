package impl

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/utils/authz"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestSearchOperationsHook(t *testing.T) {
	tests := []struct {
		name               string
		user               *auth.User
		in                 interface{}
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
		{
			name: "search query",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in: &search.SearchRequest{},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_Search.String(),
					"", ""),
					auth.Permission_Read.String()),
			},
			out: &search.SearchRequest{},
			err: false,
		},
		{
			name: "policy search query",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in: &search.PolicySearchRequest{
				Tenant:    "testTenant",
				Namespace: "default",
				SGPolicy:  "testSGPolicy",
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupSecurity), string(security.KindSGPolicy),
					"default", "testSGPolicy"),
					auth.Permission_Read.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_Search.String(),
					"", ""),
					auth.Permission_Read.String()),
			},
			out: &search.PolicySearchRequest{
				Tenant:    "testTenant",
				Namespace: "default",
				SGPolicy:  "testSGPolicy",
			},
			err: false,
		},
		{
			name: "no tenant in policy search query",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in: &search.PolicySearchRequest{
				Namespace: "default",
				SGPolicy:  "testSGPolicy",
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupSecurity), string(security.KindSGPolicy),
					"default", "testSGPolicy"),
					auth.Permission_Read.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_Search.String(),
					"", ""),
					auth.Permission_Read.String()),
			},
			out: &search.PolicySearchRequest{
				Tenant:    "testTenant",
				Namespace: "default",
				SGPolicy:  "testSGPolicy",
			},
			err: false,
		},
		{
			name:               "no user in context",
			user:               nil,
			in:                 &search.SearchRequest{},
			expectedOperations: nil,
			out:                &search.SearchRequest{},
			err:                true,
		},
		{
			name: "invalid object",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in:                 &struct{ name string }{name: "invalid object type"},
			expectedOperations: nil,
			out:                &struct{ name string }{name: "invalid object type"},
			err:                true,
		},
	}
	r := &searchHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwSearchHooks")
	r.logger = log.GetNewLogger(logConfig)
	for _, test := range tests {
		nctx := apigwpkg.NewContextWithUser(context.TODO(), test.user)
		nctx, out, err := r.operations(nctx, test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		operations, _ := apigwpkg.OperationsFromContext(nctx)
		Assert(t, areOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("[%s] test failed, expected operations [%s], got [%s]", test.name, login.PrintOperations(test.expectedOperations), login.PrintOperations(operations)))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("[%s] test failed, expected returned object [%v], got [%v]", test.name, test.out, out))
	}
}

func TestUserContextHook(t *testing.T) {
	testUserRole := login.NewRole("UserRole", "testTenant", login.NewPermission(
		"testTenant",
		"",
		auth.Permission_Search.String(),
		"",
		"",
		auth.Permission_Read.String()),
		login.NewPermission(
			"testTenant",
			string(apiclient.GroupSecurity),
			string(security.KindSGPolicy),
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String()))

	testUserRoleBinding := login.NewRoleBinding("UserRoleBinding", "testTenant", "UserRole", "testUser", "")
	tests := []struct {
		name          string
		user          *auth.User
		in            interface{}
		expectedPerms []auth.Permission
		out           interface{}
		skipCall      bool
		err           bool
	}{
		{
			name: "search query",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in: &search.SearchRequest{},
			expectedPerms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					"",
					auth.Permission_Search.String(),
					"",
					"",
					auth.Permission_Read.String()),
				login.NewPermission(
					"testTenant",
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			out: &search.SearchRequest{
				Tenants: []string{"testTenant"},
			},
			skipCall: false,
			err:      false,
		},
		{
			name: "policy search query",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in: &search.PolicySearchRequest{
				Tenant:    "testTenant",
				Namespace: "default",
				SGPolicy:  "testSGPolicy",
			},
			expectedPerms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					"",
					auth.Permission_Search.String(),
					"",
					"",
					auth.Permission_Read.String()),
				login.NewPermission(
					"testTenant",
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			out: &search.PolicySearchRequest{
				Tenant:    "testTenant",
				Namespace: "default",
				SGPolicy:  "testSGPolicy",
			},
			skipCall: false,
			err:      false,
		},
		{
			name: "no tenant in policy search query",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in: &search.PolicySearchRequest{
				Namespace: "default",
				SGPolicy:  "testSGPolicy",
			},
			expectedPerms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					"",
					auth.Permission_Search.String(),
					"",
					"",
					auth.Permission_Read.String()),
				login.NewPermission(
					"testTenant",
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			out: &search.PolicySearchRequest{
				Tenant:    "testTenant",
				Namespace: "default",
				SGPolicy:  "testSGPolicy",
			},
			skipCall: false,
			err:      false,
		},
		{
			name:          "no user in context",
			user:          nil,
			in:            &search.SearchRequest{},
			expectedPerms: nil,
			out:           &search.SearchRequest{},
			skipCall:      true,
			err:           true,
		},
		{
			name: "invalid object",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in:            &struct{ name string }{name: "invalid object type"},
			expectedPerms: nil,
			out:           &struct{ name string }{name: "invalid object type"},
			skipCall:      true,
			err:           true,
		},
		{
			name: "user with no perms",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "noPermsUser",
				},
				Spec: auth.UserSpec{
					Fullname: "No Perms User",
					Password: "password",
					Email:    "nopermsuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in:            &search.SearchRequest{},
			expectedPerms: nil,
			out: &search.SearchRequest{
				Tenants: []string{"testTenant"},
			},
			skipCall: false,
			err:      false,
		},
	}
	r := &searchHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwSearchHooks")
	r.logger = log.GetNewLogger(logConfig)
	r.permissionGetter = rbac.NewMockPermissionGetter([]*auth.Role{testUserRole}, []*auth.RoleBinding{testUserRoleBinding}, nil, nil)
	for _, test := range tests {
		nctx := apigwpkg.NewContextWithUser(context.TODO(), test.user)
		nctx, out, skipCall, err := r.userContext(nctx, test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		Assert(t, skipCall == test.skipCall, fmt.Sprintf("[%s] test failed", test.name))
		perms, _, _ := authzgrpcctx.PermsFromOutgoingContext(nctx)
		Assert(t, rbac.ArePermsEqual(test.expectedPerms, perms),
			fmt.Sprintf("[%s] test failed, expected perms [%s], got [%s]", test.name, rbac.PrintPerms(test.name, test.expectedPerms), rbac.PrintPerms(test.name, perms)))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("[%s] test failed, expected returned object [%v], got [%v]", test.name, test.out, out))
	}
}

func TestSearchHooksRegistration(t *testing.T) {
	r := &searchHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwSearchHooks")
	r.logger = log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(r.logger, false)
	err := r.registerSearchHooks(svc)
	AssertOk(t, err, "apigw search hook registration failed")

	methods := []string{"Query", "PolicyQuery"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for method [%s]", method))
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for method [%s]", len(prof.PreAuthZHooks()), method))
		Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre-call hooks [%d] for method [%s]", len(prof.PreCallHooks()), method))
	}

	// test error
	svc = mocks.NewFakeAPIGwService(r.logger, true)
	err = r.registerSearchHooks(svc)
	Assert(t, err != nil, "expected error in search hook registration")
}
