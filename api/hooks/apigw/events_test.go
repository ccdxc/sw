package impl

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/pensando/sw/venice/utils/authz/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestEventsOperationsHook(t *testing.T) {
	tests := []struct {
		name               string
		user               *auth.User
		in                 interface{}
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
		{
			name: "get events with ListWatchOptions",
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
			in: &api.ListWatchOptions{
				ObjectMeta: api.ObjectMeta{
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				},
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					"", auth.Permission_Event.String(),
					globals.DefaultNamespace, ""),
					auth.Permission_Read.String()),
			},
			out: &api.ListWatchOptions{
				ObjectMeta: api.ObjectMeta{
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				},
			},
			err: false,
		},
		{
			name: "no tenant in ListWatchOptions",
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
			in: &api.ListWatchOptions{
				ObjectMeta: api.ObjectMeta{},
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_Event.String(),
					globals.DefaultNamespace, ""),
					auth.Permission_Read.String()),
			},
			out: &api.ListWatchOptions{
				ObjectMeta: api.ObjectMeta{
					Tenant:    "testTenant",
					Namespace: globals.DefaultNamespace,
				},
			},
			err: false,
		},
		{
			name: "get event given UUID",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "testTenant",
					Name:      "testUser",
					Namespace: globals.DefaultNamespace,
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in: &events.GetEventRequest{
				UUID: "event1",
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_Event.String(),
					globals.DefaultNamespace, ""),
					auth.Permission_Read.String()),
			},
			out: &events.GetEventRequest{
				UUID: "event1",
			},
			err: false,
		},
		{
			name:               "no user in context",
			user:               nil,
			in:                 &api.ListWatchOptions{},
			expectedOperations: nil,
			out:                &api.ListWatchOptions{},
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
	r := &eventsHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwEventsHooks")
	r.logger = log.GetNewLogger(logConfig)
	for _, test := range tests {
		nctx := apigwpkg.NewContextWithUser(context.TODO(), test.user)
		nctx, out, err := r.operations(nctx, test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		operations, _ := apigwpkg.OperationsFromContext(nctx)
		Assert(t, AreOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("unexpected operations, [%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", test.out, out, test.name))
	}
}

func TestEventsUserContextHook(t *testing.T) {
	testUserRole := login.NewRole("UserRole", "testTenant",
		login.NewPermission(
			"testTenant",
			string(apiclient.GroupSecurity),
			string(security.KindNetworkSecurityPolicy),
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
			name: "get events",
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
			in: &api.ListWatchOptions{},
			expectedPerms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					string(apiclient.GroupSecurity),
					string(security.KindNetworkSecurityPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			out:      &api.ListWatchOptions{},
			skipCall: false,
			err:      false,
		},
		{
			name: "get event by UUID",
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
			in: &events.GetEventRequest{},
			expectedPerms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					string(apiclient.GroupSecurity),
					string(security.KindNetworkSecurityPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			out:      &events.GetEventRequest{},
			skipCall: false,
			err:      false,
		},
		{
			name:          "no user in context",
			user:          nil,
			in:            &api.ListWatchOptions{},
			expectedPerms: nil,
			out:           &api.ListWatchOptions{},
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
			in:            &api.ListWatchOptions{},
			expectedPerms: nil,
			out:           &api.ListWatchOptions{},
			skipCall:      false,
			err:           false,
		},
	}
	r := &eventsHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwEventsHooks")
	r.logger = log.GetNewLogger(logConfig)
	r.permissionGetter = rbac.NewMockPermissionGetter([]*auth.Role{testUserRole}, []*auth.RoleBinding{testUserRoleBinding}, nil, nil)
	for _, test := range tests {
		nctx := apigwpkg.NewContextWithUser(context.TODO(), test.user)
		nctx, out, _, skipCall, err := r.userContext(nctx, test.in, nil)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		Assert(t, skipCall == test.skipCall, fmt.Sprintf("[%s] test failed", test.name))
		perms, _, _ := authzgrpcctx.PermsFromOutgoingContext(nctx)
		Assert(t, rbac.ArePermsEqual(test.expectedPerms, perms),
			fmt.Sprintf("[%s] test failed, expected perms [%s], got [%s]", test.name, rbac.PrintPerms(test.name, test.expectedPerms), rbac.PrintPerms(test.name, perms)))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("[%s] test failed, expected returned object [%v], got [%v]", test.name, test.out, out))
	}
}

func TestEventsHooksRegistration(t *testing.T) {
	r := &eventsHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwEventsHooks")
	r.logger = log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(r.logger, false)
	err := r.registerEventsHooks(svc)
	AssertOk(t, err, "apigw events hook registration failed")

	methods := []string{"GetEvent", "GetEvents"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for method [%s]", method))
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for method [%s]", len(prof.PreAuthZHooks()), method))
		Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre-call hooks [%d] for method [%s]", len(prof.PreCallHooks()), method))
	}

	// test error
	svc = mocks.NewFakeAPIGwService(r.logger, true)
	err = r.registerEventsHooks(svc)
	Assert(t, err != nil, "expected error in events hook registration")
}
