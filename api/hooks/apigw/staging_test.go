package impl

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/bulkedit"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/staging"
	apiintf "github.com/pensando/sw/api/interfaces"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/pensando/sw/venice/utils/authz/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestStagingUserContext(t *testing.T) {
	tests := []struct {
		name     string
		user     *auth.User
		in       interface{}
		out      interface{}
		skipCall bool
		err      bool
	}{
		{
			name: "successful staging user context creation",
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
			in:       &staging.CommitAction{},
			out:      &staging.CommitAction{},
			skipCall: false,
			err:      false,
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
			in:       &struct{ name string }{name: "invalid object type"},
			out:      &struct{ name string }{name: "invalid object type"},
			skipCall: true,
			err:      true,
		},
		{
			name:     "no user in context",
			user:     nil,
			in:       &staging.CommitAction{},
			out:      &staging.CommitAction{},
			skipCall: true,
			err:      true,
		},
	}
	r := &stagingHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	r.logger = log.GetNewLogger(logConfig)
	r.permissionGetter = rbac.NewMockPermissionGetter([]*auth.Role{testNetworkAdminRole}, []*auth.RoleBinding{testNetworkAdminRoleBinding}, nil, nil)
	for _, test := range tests {
		nctx := apigwpkg.NewContextWithUser(context.TODO(), test.user)
		_, out, _, skipCall, err := r.userContext(nctx, test.in, nil)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		Assert(t, skipCall == test.skipCall, fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("[%s] test failed, expected returned object [%v], got [%v]", test.name, test.out, out))
	}
}

func TestStagingUserContextHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &stagingHooks{}
	r.logger = l
	err := r.registerUserContextHook(svc)
	AssertOk(t, err, "userContext hook registration failed")

	methods := []string{"Commit"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		AssertOk(t, err, "error getting service profile for method [%s]", method)
		Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre-call hooks [%d] for method [%s]", len(prof.PreCallHooks()), method))
	}
	ids := []serviceID{
		{"Buffer", apiintf.GetOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		AssertOk(t, err, "error getting service profile for [%s] [%s]", id.kind, id.action)
		Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre-call hooks [%d] for [%s] [%s] profile", len(prof.PreCallHooks()), id.kind, id.action))
	}
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerUserContextHook(svc)
	Assert(t, err != nil, "expected error in userContext hook registration")
}

func TestStagingOpsPreAuthzHook(t *testing.T) {
	testuser := &auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: globals.DefaultTenant,
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
		in                 interface{}
		user               *auth.User
		operations         []authz.Operation
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
		{
			name: "commit buffer",
			in:   &staging.CommitAction{},
			user: testuser,
			operations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test"),
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResourceWithOwner(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test", testuser),
					auth.Permission_Commit.String()),
			},
			out: &staging.CommitAction{},
			err: false,
		},
		{
			name: "clear buffer",
			in:   &staging.ClearAction{},
			user: testuser,
			operations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test"),
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResourceWithOwner(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test", testuser),
					auth.Permission_Clear.String()),
			},
			out: &staging.ClearAction{},
			err: false,
		},
		{
			name: "commit buffer in default tenant and user in non-default tenant",
			in:   &staging.CommitAction{},
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
			operations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test"),
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test"),
					auth.Permission_Commit.String()),
			},
			out: &staging.CommitAction{},
			err: false,
		},
		{
			name: "invalid object",
			in:   &struct{ name string }{name: "invalid object type"},
			user: testuser,
			operations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test"),
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test"),
					auth.Permission_Create.String()),
			},
			out: &struct{ name string }{name: "invalid object type"},
			err: true,
		},
		{
			name:               "nil operations slice",
			in:                 &staging.CommitAction{},
			user:               testuser,
			operations:         nil,
			expectedOperations: nil,
			out:                &staging.CommitAction{},
			err:                true,
		},
		{
			name:               "nil operation",
			in:                 &staging.CommitAction{},
			user:               testuser,
			operations:         []authz.Operation{nil, nil},
			expectedOperations: []authz.Operation{nil, nil},
			out:                &staging.CommitAction{},
			err:                true,
		},
		{
			name: "nil resource",
			in:   &staging.CommitAction{},
			user: testuser,
			operations: []authz.Operation{
				authz.NewOperation(nil,
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(nil,
					auth.Permission_Create.String()),
			},
			out: &staging.CommitAction{},
			err: true,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &stagingHooks{}
	r.logger = l
	for _, test := range tests {
		ctx := apigwpkg.NewContextWithUser(context.TODO(), test.user)
		ctx = apigwpkg.NewContextWithOperations(ctx, test.operations...)
		nctx, out, err := r.opsPreAuthzHook(ctx, test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		operations, _ := apigwpkg.OperationsFromContext(nctx)
		Assert(t, AreOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("unexpected operations, [%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", test.out, out, test.name))
	}
}

func TestStagingOpsPreAuthzHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &stagingHooks{}
	r.logger = l
	err := r.registerOpsPreAuthzHook(svc)
	AssertOk(t, err, "opsPreAuthz hook registration failed")

	methods := []string{"Commit", "Clear"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		AssertOk(t, err, "error getting service profile for method [%s]", method)
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre-authz hooks [%d] for method [%s]", len(prof.PreAuthZHooks()), method))
	}
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerOpsPreAuthzHook(svc)
	Assert(t, err != nil, "expected error in opsPreAuthz hook registration")
}

func TestStagingAddOwnerPreAuthzHook(t *testing.T) {
	testuser := &auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: globals.DefaultTenant,
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
		in                 interface{}
		user               *auth.User
		operations         []authz.Operation
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
		{
			name: "create buffer",
			in:   &staging.Buffer{},
			user: testuser,
			operations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test"),
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResourceWithOwner(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test", testuser),
					auth.Permission_Create.String()),
			},
			out: &staging.Buffer{},
			err: false,
		},
		{
			name: "create buffer in default tenant and user in non-default tenant",
			in:   &staging.Buffer{},
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
			operations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test"),
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test"),
					auth.Permission_Create.String()),
			},
			out: &staging.Buffer{},
			err: false,
		},
		{
			name: "invalid object",
			in:   &struct{ name string }{name: "invalid input type"},
			user: testuser,
			operations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test"),
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource(globals.DefaultTenant,
					string(apiclient.GroupStaging), string(staging.KindBuffer),
					globals.DefaultNamespace, "test"),
					auth.Permission_Create.String()),
			},
			out: &struct{ name string }{name: "invalid input type"},
			err: true,
		},
		{
			name:               "nil operations slice",
			in:                 &staging.Buffer{},
			user:               testuser,
			operations:         nil,
			expectedOperations: nil,
			out:                &staging.Buffer{},
			err:                true,
		},
		{
			name:               "nil operation",
			in:                 &staging.Buffer{},
			user:               testuser,
			operations:         []authz.Operation{nil, nil},
			expectedOperations: []authz.Operation{nil, nil},
			out:                &staging.Buffer{},
			err:                true,
		},
		{
			name: "nil resource",
			in:   &staging.Buffer{},
			user: testuser,
			operations: []authz.Operation{
				authz.NewOperation(nil,
					auth.Permission_Create.String()),
			},
			expectedOperations: []authz.Operation{
				authz.NewOperation(nil,
					auth.Permission_Create.String()),
			},
			out: &staging.Buffer{},
			err: true,
		},
		{
			name:               "no user in context",
			user:               nil,
			in:                 &staging.Buffer{},
			expectedOperations: nil,
			out:                &staging.Buffer{},
			err:                true,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &stagingHooks{}
	r.logger = l
	for _, test := range tests {
		ctx := apigwpkg.NewContextWithUser(context.TODO(), test.user)
		ctx = apigwpkg.NewContextWithOperations(ctx, test.operations...)
		nctx, out, err := r.addOwnerPreAuthzHook(ctx, test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		operations, _ := apigwpkg.OperationsFromContext(nctx)
		Assert(t, AreOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("unexpected operations, [%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", test.out, out, test.name))
	}
}

func TestStagingAddOwnerPreAuthzHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &stagingHooks{}
	r.logger = l
	err := r.registerAddOwnerPreAuthzHook(svc)
	AssertOk(t, err, "opsPreAuthz hook registration failed")

	ids := []serviceID{
		{string(staging.KindBuffer), apiintf.CreateOper},
		{string(staging.KindBuffer), apiintf.DeleteOper},
		{string(staging.KindBuffer), apiintf.GetOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		AssertOk(t, err, "error getting service profile for obj [%s], action [%s]", id.kind, id.action)
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre-authz hooks [%d] for obj [%s], action [%s]", len(prof.PreAuthZHooks()), id.kind, id.action))
	}
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerOpsPreAuthzHook(svc)
	Assert(t, err != nil, "expected error in opsPreAuthz hook registration")
}

func TestStagingBulkEdit(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &stagingHooks{}
	r.logger = l

	ctx := context.Background()
	netw := &network.Network{
		TypeMeta: api.TypeMeta{
			Kind:       "Network",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:      "TestNtwork1",
			Tenant:    "default",
			Namespace: "default",
		},
		Spec: network.NetworkSpec{
			Type:        network.NetworkType_Bridged.String(),
			IPv4Subnet:  "10.1.1.1/24",
			IPv4Gateway: "10.1.1.1",
		},
		Status: network.NetworkStatus{},
	}

	netwAny, _ := types.MarshalAny(netw)
	netw.Spec.IPv4Subnet = "111.1.1.1/24"
	netw.Spec.IPv6Gateway = "111.1.1.1"

	netwAny2, _ := types.MarshalAny(netw)

	bEditAction := staging.BulkEditAction{
		TypeMeta: api.TypeMeta{
			Kind:       "BulkEditAction",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "TestBuffer1",
			Tenant: "default",
		},
		Spec: bulkedit.BulkEditActionSpec{
			Items: []*bulkedit.BulkEditItem{
				&bulkedit.BulkEditItem{
					Method: "create",
					Object: &api.Any{Any: *netwAny},
				},
				&bulkedit.BulkEditItem{
					Method: "update",
					Object: &api.Any{Any: *netwAny2},
				},
				&bulkedit.BulkEditItem{
					Method: "delete",
					Object: &api.Any{Any: *netwAny2},
				},
			},
		},
		Status: staging.BulkEditActionStatus{},
	}

	_, rcvdOps, err := r.processBulkeditReq(ctx, &bEditAction)
	if err != nil {
		t.Fatalf("Fatal err %s\n", err.Error())
	}
	expectedResource := authz.NewResource(
		"default",
		"network",
		"Network",
		globals.DefaultNamespace,
		"TestNtwork1",
	)
	expectedOp0 := authz.NewOperation(expectedResource, "create")
	expectedOp1 := authz.NewOperation(expectedResource, "update")
	expectedOp2 := authz.NewOperation(expectedResource, "delete")

	AssertOk(t, err, "ApiGw BulkEdit hook failed")
	Assert(t, len(rcvdOps) == 3, "Expected number of operations doesn't match!")
	Assert(t, AreOperationsEqual([]authz.Operation{expectedOp0}, []authz.Operation{rcvdOps[0]}), "Create operation comparison failed")
	Assert(t, AreOperationsEqual([]authz.Operation{expectedOp1}, []authz.Operation{rcvdOps[1]}), "Update operation comparison failed")
	Assert(t, AreOperationsEqual([]authz.Operation{expectedOp2}, []authz.Operation{rcvdOps[2]}), "Delete operation comparison failed")

	// Negative test cases
	// 1. Send a wrong request type

	wrongReq := staging.ClearAction{}
	_, _, err = r.processBulkeditReq(ctx, &wrongReq)
	Assert(t, err != nil, "Expected an invalid input type error")

	// 2. Unknown method type
	bEditAction = staging.BulkEditAction{
		TypeMeta: api.TypeMeta{
			Kind:       "BulkEditAction",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "TestBuffer1",
			Tenant: "default",
		},
		Spec: bulkedit.BulkEditActionSpec{
			Items: []*bulkedit.BulkEditItem{
				&bulkedit.BulkEditItem{
					Method: "qwertyuiop",
					Object: &api.Any{Any: *netwAny},
				},
			},
		},
		Status: staging.BulkEditActionStatus{},
	}
	_, _, err = r.processBulkeditReq(ctx, &bEditAction)
	Assert(t, err != nil, "Expected an unknwon opertype type error")
}
