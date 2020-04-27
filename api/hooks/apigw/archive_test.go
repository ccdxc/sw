package impl

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/objstore"
	apiintf "github.com/pensando/sw/api/interfaces"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/archive"
	"github.com/pensando/sw/venice/utils/archive/mock"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"
	rslvrmock "github.com/pensando/sw/venice/utils/resolver/mock"

	. "github.com/pensando/sw/venice/utils/authz/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestCancelPreCallHook(t *testing.T) {
	tests := []struct {
		name          string
		in            interface{}
		clientGetter  archive.ClientGetter
		archiveGetter archive.Getter
		out           interface{}
		skipCall      bool
		err           error
	}{
		{
			name:     "invalid object",
			in:       &struct{ name string }{name: "invalid object type"},
			out:      nil,
			skipCall: true,
			err:      errors.New("invalid input type"),
		},
		{
			name: "error in getting client",
			in: &monitoring.CancelArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
			},
			clientGetter: mock.GetClientGetter(false, true),
			archiveGetter: mock.GetArchiveGetter(&monitoring.ArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
				},
			}, false),
			out:      nil,
			skipCall: true,
			err:      errors.New("simulated client error"),
		},
		{
			name: "error in CancelRequest rpc",
			in: &monitoring.CancelArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
			},
			archiveGetter: mock.GetArchiveGetter(&monitoring.ArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
				},
			}, false),
			clientGetter: mock.GetClientGetter(true, false),
			out:          nil,
			skipCall:     true,
			err:          errors.New("simulated CancelRequest error"),
		},
		{
			name: "successful Cancel request",
			in: &monitoring.CancelArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
			},
			clientGetter: mock.GetClientGetter(false, false),
			archiveGetter: mock.GetArchiveGetter(&monitoring.ArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
				},
			}, false),
			out: &monitoring.ArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
				},
			},
			skipCall: true,
			err:      nil,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwArchiveHooks")
	l := log.GetNewLogger(logConfig)
	r := &archiveHooks{}
	r.logger = l
	for _, test := range tests {
		r.clientGetter = test.clientGetter
		r.archiveGetter = test.archiveGetter
		ctx := context.TODO()
		rslvr := rslvrmock.New()
		r.rslvr = rslvr
		_, out, _, ok, err := r.cancelPreCallHook(ctx, test.in, nil)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected error [%v], got [%v]", test.name, test.err, err))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("[%s] test failed, expected object [%v], got [%v]", test.name, test.out, out))
		Assert(t, test.skipCall == ok, fmt.Sprintf("[%s] test failed, expected skipcall [%v], got [%v]", test.name, test.skipCall, ok))
	}
}

func TestCancelPreCallHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwArchiveHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &archiveHooks{}
	r.logger = l
	err := r.registerCancelPreCallHook(svc)
	AssertOk(t, err, "cancelPreCallHook hook registration failed")

	prof, err := svc.GetServiceProfile("Cancel")
	AssertOk(t, err, fmt.Sprintf("error getting service profile for oper :%v", "Cancel"))
	Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre call hooks [%d] for ArchiveRequest action [%v]", len(prof.PreCallHooks()), "Cancel"))

	// test err
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerCancelPreCallHook(svc)
	Assert(t, err != nil, "expected error in cancelPreCallHook hook registration")
}

func TestArchiveOperationsPreAuthzHook(t *testing.T) {
	tests := []struct {
		name               string
		user               *auth.User
		in                 interface{}
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
		{
			name: "audit event archive request for non-default tenant user",
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
			in: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "testTenant",
					Name:      "arch-req-1",
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{
						Tenants: []string{globals.DefaultTenant},
					},
				},
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_AuditEvent.String(),
					globals.DefaultNamespace, ""),
					auth.Permission_Read.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupObjstore), string(objstore.KindObject),
					objstore.Buckets_auditevents.String(), ""),
					auth.Permission_Create.String()),
			},
			out: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "testTenant",
					Name:      "arch-req-1",
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{
						Tenants: []string{"testTenant"},
					},
				},
			},
			err: false,
		},
		{
			name: "event archive request for non-default tenant user",
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
			in: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "testTenant",
					Name:      "arch-req-1",
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_Event.String(),
					Query: &monitoring.ArchiveQuery{
						Tenants: []string{globals.DefaultTenant},
					},
				},
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_Event.String(),
					globals.DefaultNamespace, ""),
					auth.Permission_Read.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupObjstore), string(objstore.KindObject),
					objstore.Buckets_events.String(), ""),
					auth.Permission_Create.String()),
			},
			out: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "testTenant",
					Name:      "arch-req-1",
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_Event.String(),
					Query: &monitoring.ArchiveQuery{
						Tenants: []string{"testTenant"},
					},
				},
			},
			err: false,
		},
		{
			name: "no user in context",
			user: nil,
			in: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "testTenant",
					Name:      "arch-req-1",
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{
						Tenants: []string{"testTenant"},
					},
				},
			},
			expectedOperations: nil,
			out: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "testTenant",
					Name:      "arch-req-1",
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{
						Tenants: []string{"testTenant"},
					},
				},
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
		{
			name: "nil query",
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
			in: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "testTenant",
					Name:      "arch-req-1",
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
				},
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_AuditEvent.String(),
					globals.DefaultNamespace, ""),
					auth.Permission_Read.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupObjstore), string(objstore.KindObject),
					objstore.Buckets_auditevents.String(), ""),
					auth.Permission_Create.String()),
			},
			out: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{Kind: string(monitoring.KindArchiveRequest)},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "testTenant",
					Name:      "arch-req-1",
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
					Query: &monitoring.ArchiveQuery{
						Tenants: []string{"testTenant"},
					},
				},
			},
			err: false,
		},
	}
	r := &archiveHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwArchiveHooks")
	r.logger = log.GetNewLogger(logConfig)
	for _, test := range tests {
		nctx := apigwpkg.NewContextWithUser(context.TODO(), test.user)
		nctx, out, err := r.operationsPreAuthzHook(nctx, test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		operations, _ := apigwpkg.OperationsFromContext(nctx)
		Assert(t, AreOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("unexpected operations, [%s] test failed, expected: %s, got: %s", test.name, authz.PrintOperations(test.expectedOperations), authz.PrintOperations(operations)))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", test.out, out, test.name))
	}
}

func TestArchiveOperationsPreAuthzHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwArchiveHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &archiveHooks{}
	r.logger = l
	err := r.registerOperationsPreAuthzHook(svc)
	AssertOk(t, err, "operationsPreAuthzHook hook registration failed")

	prof, err := svc.GetCrudServiceProfile("ArchiveRequest", apiintf.CreateOper)
	AssertOk(t, err, fmt.Sprintf("error getting service profile for method: %v,  oper :%v", "ArchiveRequest", apiintf.CreateOper))
	Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for ArchiveRequest Create", len(prof.PreAuthZHooks())))

	// test err
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerOperationsPreAuthzHook(svc)
	Assert(t, err != nil, "expected error in operationsPreAuthzHook hook registration")
}
