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
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/pensando/sw/venice/utils/authz/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestTelemetryOperationsHook(t *testing.T) {
	testuser := &auth.User{
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
	}
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
			user: testuser,
			in:   &telemetry_query.MetricsQueryList{},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResourceWithOwner("testTenant",
					"", auth.Permission_MetricsQuery.String(),
					"", "", testuser),
					auth.Permission_Read.String()),
			},
			out: &telemetry_query.MetricsQueryList{Tenant: "testTenant"},
			err: false,
		},
		{
			name: "fwlogs query operations hook test with no tenant",
			user: testuser,
			in:   &telemetry_query.FwlogsQueryList{},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					"", auth.Permission_FwLog.String(),
					"", ""),
					auth.Permission_Read.String()),
			},
			out: &telemetry_query.FwlogsQueryList{Tenant: "testTenant"},
			err: false,
		},
		{
			name: "metrics query operations hook test with different tenant than user's",
			user: testuser,
			in:   &telemetry_query.MetricsQueryList{Tenant: "differentTenant"},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResourceWithOwner("differentTenant",
					"", auth.Permission_MetricsQuery.String(),
					"", "", testuser),
					auth.Permission_Read.String()),
			},
			out: &telemetry_query.MetricsQueryList{Tenant: "differentTenant"},
			err: false,
		},
		{
			name: "fwlogs query operations hook test with different tenant than user's",
			user: testuser,
			in:   &telemetry_query.FwlogsQueryList{Tenant: "differentTenant"},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("differentTenant",
					"", auth.Permission_FwLog.String(),
					"", ""),
					auth.Permission_Read.String()),
			},
			out: &telemetry_query.FwlogsQueryList{Tenant: "differentTenant"},
			err: false,
		},
		{
			name: "metrics query operations hook test with kinds in query list",
			user: testuser,
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
				authz.NewOperation(authz.NewResource("",
					string(apiclient.GroupCluster), string(cluster.KindNode),
					"", ""),
					auth.Permission_Read.String()),
				authz.NewOperation(authz.NewResourceWithOwner("testTenant",
					"", auth.Permission_MetricsQuery.String(),
					"", "", testuser),
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
		Assert(t, AreOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("unexpected operations, [%s] test failed, expected opertaions:%+v, got:%+v", test.name, authz.PrintOperations(test.expectedOperations),
				authz.PrintOperations(operations)))
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

	methods := []string{"Metrics", "Fwlogs"}
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
