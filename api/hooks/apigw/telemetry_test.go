package impl

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestTelemetryOperationsHook(t *testing.T) {
	tests := []struct {
		name               string
		user               *auth.User
		in                 interface{}
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
		{
			name: "metrics query operations hook test with no tenant",
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
			in: &telemetry_query.MetricsQueryList{},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_MetricsQuery.String(),
					"", ""),
					auth.Permission_Read.String()),
			},
			out: &telemetry_query.MetricsQueryList{Tenant: "testTenant"},
			err: false,
		},
		{
			name: "metrics query operations hook test with different tenant than user's",
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
			in: &telemetry_query.MetricsQueryList{Tenant: "differentTenant"},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("differentTenant",
					"", auth.Permission_MetricsQuery.String(),
					"", ""),
					auth.Permission_Read.String()),
			},
			out: &telemetry_query.MetricsQueryList{Tenant: "differentTenant"},
			err: false,
		},
		{
			name: "metrics query operations hook test with kinds in query list",
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
			in: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
					},
				},
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					string(apiclient.GroupCluster), string(cluster.KindNode),
					"", ""),
					auth.Permission_Read.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_MetricsQuery.String(),
					"", ""),
					auth.Permission_Read.String()),
			},
			out: &telemetry_query.MetricsQueryList{
				Tenant: "testTenant",
				Queries: []*telemetry_query.MetricsQuerySpec{
					{
						TypeMeta: api.TypeMeta{
							Kind: "Node",
						},
					},
				},
			},
			err: false,
		},
		{
			name:               "no user in context",
			user:               nil,
			in:                 &telemetry_query.MetricsQueryList{},
			expectedOperations: nil,
			out:                &telemetry_query.MetricsQueryList{},
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
	r := &telemetryHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwTelemetryHooks")
	r.logger = log.GetNewLogger(logConfig)
	for _, test := range tests {
		nctx := apigwpkg.NewContextWithUser(context.TODO(), test.user)
		nctx, out, err := r.operations(nctx, test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		operations, _ := apigwpkg.OperationsFromContext(nctx)
		Assert(t, areOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("unexpected operations, [%s] test failed, expected opertaions:%+v, got:%+v", test.name, login.PrintOperations(test.expectedOperations),
				login.PrintOperations(operations)))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", test.out, out, test.name))
	}
}

func TestTelemetryHooksRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwTelemetryHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	err := registerTelemetryHooks(svc, l)
	AssertOk(t, err, "apigw telemetry hook registration failed")

	methods := []string{"Metrics"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for method [%s]", method))
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for method [%s]", len(prof.PreAuthZHooks()), method))
	}

	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = registerTelemetryHooks(svc, l)
	Assert(t, err != nil, "expected error in telemetry hook registration")
}
