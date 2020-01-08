package impl

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
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

func TestAuditOperationsHook(t *testing.T) {
	tests := []struct {
		name               string
		user               *auth.User
		in                 interface{}
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
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
			in: &audit.AuditEventRequest{
				UUID: "event1",
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_AuditEvent.String(),
					globals.DefaultNamespace, ""),
					auth.Permission_Read.String()),
			},
			out: &audit.AuditEventRequest{
				UUID: "event1",
			},
			err: false,
		},
		{
			name: "no user in context",
			user: nil,
			in: &audit.AuditEventRequest{
				UUID: "event1",
			},
			expectedOperations: nil,
			out: &audit.AuditEventRequest{
				UUID: "event1",
			},
			err: true,
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
	r := &auditHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwAuditHooks")
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

func TestAuditUserContextHook(t *testing.T) {
	testUserRole := login.NewRole("UserRole", "testTenant",
		login.NewPermission(
			"testTenant",
			"",
			auth.Permission_AuditEvent.String(),
			"",
			"",
			auth.Permission_Read.String()))

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
			in: &audit.AuditEventRequest{},
			expectedPerms: []auth.Permission{login.NewPermission(
				"testTenant",
				"",
				auth.Permission_AuditEvent.String(),
				"",
				"",
				auth.Permission_Read.String()),
			},
			out:      &audit.AuditEventRequest{},
			skipCall: false,
			err:      false,
		},
		{
			name:          "no user in context",
			user:          nil,
			in:            &audit.AuditEventRequest{},
			expectedPerms: nil,
			out:           &audit.AuditEventRequest{},
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
			in:            &audit.AuditEventRequest{},
			expectedPerms: nil,
			out:           &audit.AuditEventRequest{},
			skipCall:      false,
			err:           false,
		},
	}
	r := &auditHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwAuditHooks")
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

func TestAuditHooksRegistration(t *testing.T) {
	r := &auditHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwAuditHooks")
	r.logger = log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(r.logger, false)
	err := r.registerAuditHooks(svc)
	AssertOk(t, err, "apigw audit events hook registration failed")

	methods := []string{"GetEvent"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for method [%s]", method))
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for method [%s]", len(prof.PreAuthZHooks()), method))
		Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre-call hooks [%d] for method [%s]", len(prof.PreCallHooks()), method))
	}

	// test error
	svc = mocks.NewFakeAPIGwService(r.logger, true)
	err = r.registerAuditHooks(svc)
	Assert(t, err != nil, "expected error in audit hook registration")
}
