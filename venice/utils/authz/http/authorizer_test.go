package http

import (
	"context"
	"errors"
	"fmt"
	"net/http"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"

	. "github.com/pensando/sw/venice/utils/authz/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestIsAuthorized(t *testing.T) {
	tests := []struct {
		name      string
		user      *auth.User
		perms     []auth.Permission
		operation authz.Operation
		allowed   bool
		err       error
	}{
		{
			name:      "allowed cluster scoped operation",
			user:      &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser", Tenant: globals.DefaultTenant}},
			perms:     []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())},
			operation: authz.NewOperation(authz.NewResource("", string(apiclient.GroupAuth), string(auth.KindAuthenticationPolicy), "", "Singleton"), string(auth.Permission_Read)),
			allowed:   true,
			err:       nil,
		},
		{
			name:      "allowed tenant scoped operation",
			user:      &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser", Tenant: globals.DefaultTenant}},
			perms:     []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())},
			operation: authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindUser), globals.DefaultNamespace, "test"), string(auth.Permission_Read)),
			allowed:   true,
			err:       nil,
		},
		{
			name:      "nil user",
			user:      nil,
			perms:     []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())},
			operation: authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindUser), globals.DefaultNamespace, "test"), string(auth.Permission_Read)),
			allowed:   false,
			err:       errors.New("no user specified"),
		},
		{
			name:      "no tenant",
			user:      &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser", Tenant: ""}},
			perms:     []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())},
			operation: authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindUser), globals.DefaultNamespace, "test"), string(auth.Permission_Read)),
			allowed:   false,
			err:       errors.New("tenant not populated in user"),
		},
		{
			name:      "no username",
			user:      &auth.User{ObjectMeta: api.ObjectMeta{Name: "", Tenant: globals.DefaultTenant}},
			perms:     []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())},
			operation: authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindUser), globals.DefaultNamespace, "test"), string(auth.Permission_Read)),
			allowed:   false,
			err:       errors.New("username not populated in user object"),
		},
		{
			name:      "nil permission",
			user:      &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser", Tenant: globals.DefaultTenant}},
			perms:     nil,
			operation: authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindUser), globals.DefaultNamespace, "test"), string(auth.Permission_Read)),
			allowed:   false,
			err:       nil,
		},
	}
	for _, test := range tests {
		request, _ := http.NewRequest("GET", "http://venice.pensando.io", nil)
		err := AddUserPermsToRequest(request, test.user, false, test.perms)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected error [%v], got [%v]", test.name, test.err, err))
		if err == nil {
			authorizer, err := NewAuthorizer(context.TODO(), request)
			AssertOk(t, err, "error creating http authorizer")
			ok, _ := authorizer.IsAuthorized(test.user, test.operation)
			Assert(t, ok == test.allowed, fmt.Sprintf("[%s] test failed, expected authorization result [%v], got [%v]", test.name, test.allowed, ok))
		}
	}
}

func TestNoUserInRequest(t *testing.T) {
	request, _ := http.NewRequest("GET", "http://venice.pensando.io", nil)
	authorizer, err := NewAuthorizer(context.TODO(), request)
	Assert(t, authorizer == nil, "expected authorizer to be nil")
	Assert(t, reflect.DeepEqual(err, errors.New("no user in context")), fmt.Sprintf("unexpected err: %v", err))
}

func TestUserMismatch(t *testing.T) {
	request, _ := http.NewRequest("GET", "http://venice.pensando.io", nil)
	user := &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser", Tenant: globals.DefaultTenant}}
	err := AddUserPermsToRequest(request, user, false, nil)
	AssertOk(t, err, "unexpected error adding user to request")
	authorizer, err := NewAuthorizer(context.TODO(), request)
	AssertOk(t, err, "unexpected error creating authorizer")
	user.Name = "different user"
	ok, _ := authorizer.IsAuthorized(user)
	Assert(t, !ok, "expected authorization to fail")
	user.Name = "testuser"
	user.Tenant = "testtenant"
	ok, _ = authorizer.IsAuthorized(user)
	Assert(t, !ok, "expected authorization to fail")
}

func TestAuthorizedOperations(t *testing.T) {
	tests := []struct {
		name      string
		user      *auth.User
		perms     []auth.Permission
		tenant    string
		namespace string
		action    auth.Permission_ActionType
		ops       []authz.Operation
	}{
		{
			name:      "all actions permission for one kind",
			user:      &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser", Tenant: globals.DefaultTenant}},
			perms:     []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindRole), authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())},
			tenant:    globals.DefaultTenant,
			namespace: globals.DefaultNamespace,
			ops:       []authz.Operation{authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindRole), globals.DefaultNamespace, ""), auth.Permission_AllActions.String())},
			action:    auth.Permission_AllActions,
		},
		{
			name:      "all actions tenant scoped operations for one api group",
			user:      &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser", Tenant: globals.DefaultTenant}},
			perms:     []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())},
			tenant:    globals.DefaultTenant,
			namespace: globals.DefaultNamespace,
			ops: []authz.Operation{authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindRole), globals.DefaultNamespace, ""), auth.Permission_AllActions.String()),
				authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindRoleBinding), globals.DefaultNamespace, ""), auth.Permission_AllActions.String()),
				authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindUserPreference), globals.DefaultNamespace, ""), auth.Permission_AllActions.String()),
				authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindUser), globals.DefaultNamespace, ""), auth.Permission_AllActions.String())},
			action: auth.Permission_AllActions,
		},
		{
			name:      "all actions cluster scoped operations for one api group",
			user:      &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser", Tenant: globals.DefaultTenant}},
			perms:     []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())},
			tenant:    "",
			namespace: "",
			ops:       []authz.Operation{authz.NewOperation(authz.NewResource("", string(apiclient.GroupAuth), string(auth.KindAuthenticationPolicy), "", ""), auth.Permission_AllActions.String())},
			action:    auth.Permission_AllActions,
		},
		{
			name:      "no authorized tenant scoped operation for one api group",
			user:      &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser", Tenant: globals.DefaultTenant}},
			perms:     []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())},
			tenant:    "testtenant",
			namespace: globals.DefaultNamespace,
			ops:       nil,
			action:    auth.Permission_AllActions,
		},
		{
			name:      "no authorized cluster scoped operation for one api group",
			user:      &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser", Tenant: "testtenant"}},
			perms:     []auth.Permission{login.NewPermission("testtenant", string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())},
			tenant:    "",
			namespace: "",
			ops:       nil,
			action:    auth.Permission_AllActions,
		},
		{
			name:      "user mismatch",
			user:      &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser1", Tenant: globals.DefaultTenant}},
			perms:     []auth.Permission{login.NewPermission(globals.DefaultTenant, string(apiclient.GroupAuth), "", authz.ResourceNamespaceAll, "", auth.Permission_AllActions.String())},
			tenant:    "",
			namespace: "",
			ops:       nil,
			action:    auth.Permission_AllActions,
		},
	}
	user := &auth.User{ObjectMeta: api.ObjectMeta{Name: "testuser", Tenant: globals.DefaultTenant}}
	for _, test := range tests {
		request, _ := http.NewRequest("GET", "http://venice.pensando.io", nil)
		err := AddUserPermsToRequest(request, user, false, test.perms)
		AssertOk(t, err, fmt.Sprintf("[%s] test failed, unexpected error adding user and perms to request", test.name))
		authorizer, err := NewAuthorizer(context.TODO(), request)
		AssertOk(t, err, "unexpected error creating authorizer")
		ops := authorizer.AuthorizedOperations(test.user, test.tenant, test.namespace, test.action)
		Assert(t, AreOperationsEqual(test.ops, ops), fmt.Sprintf("[%s] test failed, expected ops [%s], got [%s]", test.name, authz.PrintOperations(test.ops), authz.PrintOperations(ops)))
	}
}
