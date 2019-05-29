package apigwpkg

import (
	"context"
	"reflect"
	"testing"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/testutils"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/globals"
)

func TestUserFromContext(t *testing.T) {
	tests := []struct {
		name string
		user *auth.User
		ok   bool
	}{
		{
			name: "no user in context",
			ok:   false,
		},
		{
			name: "user in context",
			user: &auth.User{
				ObjectMeta: api.ObjectMeta{
					Name:   "testuser",
					Tenant: globals.DefaultTenant,
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
				},
			},
			ok: true,
		},
	}
	for _, test := range tests {
		ctx := context.TODO()
		if test.user != nil {
			ctx = NewContextWithUser(ctx, test.user)
		}
		user, ok := UserFromContext(ctx)
		if ok != test.ok {
			t.Errorf("[%s] test failed", test.name)
		}
		if !reflect.DeepEqual(test.user, user) {
			t.Errorf("[%s] test failed, expected user[%#v] got [%#v]", test.name, test.user, user)
		}
	}
}

func TestAddOperationsToContext(t *testing.T) {
	tests := []struct {
		name        string
		ops         []authz.Operation
		existingOps []authz.Operation
	}{
		{
			name: "no existing operations",
			ops:  []authz.Operation{authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindRole), "", "testrole"), auth.Permission_Create.String())},
		},
		{
			name:        "with existing operations",
			ops:         []authz.Operation{authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindRole), "", "testrole"), auth.Permission_Create.String())},
			existingOps: []authz.Operation{authz.NewOperation(authz.NewResource(globals.DefaultTenant, string(apiclient.GroupAuth), string(auth.KindRoleBinding), "", "testrb"), auth.Permission_Create.String())},
		},
	}
	for _, test := range tests {
		var tOps []authz.Operation
		tOps = append(tOps, test.ops...)
		ctx := context.TODO()
		if test.existingOps != nil {
			ctx = context.WithValue(ctx, authzOperationsKey{}, test.existingOps)
			tOps = append(tOps, test.existingOps...)
		}
		ctx = AddOperationsToContext(ctx, test.ops...)
		ops := ctx.Value(authzOperationsKey{}).([]authz.Operation)
		testutils.AreOperationsEqual(tOps, ops)
	}
}
