package impl

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/fwlog"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	. "github.com/pensando/sw/venice/utils/authz/testutils"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestFwLogOperationsHook(t *testing.T) {
	tests := []struct {
		name               string
		user               *auth.User
		in                 interface{}
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
		{
			name: "get fwlogs",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant:    globals.DefaultTenant,
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
			in: &fwlog.FwLogQuery{
				DestIPs:    []string{"192.168.10.1"},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					"", auth.Permission_FwLog.String(),
					globals.DefaultNamespace, ""),
					auth.Permission_Read.String()),
			},
			out: &fwlog.FwLogQuery{
				DestIPs:    []string{"192.168.10.1"},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			err: false,
		},
		{
			name: "non default tenant user",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "testtenant",
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
			in: &fwlog.FwLogQuery{
				DestIPs:    []string{"192.168.10.1"},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testtenant",
					"", auth.Permission_FwLog.String(),
					globals.DefaultNamespace, ""),
					auth.Permission_Read.String()),
			},
			out: &fwlog.FwLogQuery{
				DestIPs:    []string{"192.168.10.1"},
				MaxResults: 50,
				Tenants:    []string{"testtenant"},
			},
			err: false,
		},
		{
			name: "no user in context",
			user: nil,
			in: &fwlog.FwLogQuery{
				DestIPs:    []string{"192.168.10.1"},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
			},
			expectedOperations: nil,
			out: &fwlog.FwLogQuery{
				DestIPs:    []string{"192.168.10.1"},
				MaxResults: 50,
				Tenants:    []string{globals.DefaultTenant},
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
	r := &fwlogHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwFwLogHooks")
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

func TestFwLogHooksRegistration(t *testing.T) {
	r := &fwlogHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwFwLogHooks")
	r.logger = log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(r.logger, false)
	err := r.registerFwLogHooks(svc)
	AssertOk(t, err, "apigw fwlog hook registration failed")

	methods := []string{"GetLogs"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for method [%s]", method))
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for method [%s]", len(prof.PreAuthZHooks()), method))
	}

	// test error
	svc = mocks.NewFakeAPIGwService(r.logger, true)
	err = r.registerFwLogHooks(svc)
	Assert(t, err != nil, "expected error in fwlog hook registration")
}
