package impl

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/login"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestNewContextWithUserPerms(t *testing.T) {
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
		expectedPerms []auth.Permission
		expectedAdmin bool
		err           bool
	}{
		{
			name: "successful context creation",
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
			expectedPerms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					"",
					auth.Permission_AuditEvent.String(),
					"",
					"",
					auth.Permission_Read.String()),
			},
			expectedAdmin: false,
			err:           false,
		},
		{
			name:          "no user in context",
			user:          nil,
			expectedPerms: nil,
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
			expectedPerms: nil,
			err:           false,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwHookUtils")
	logger := log.GetNewLogger(logConfig)
	permissionGetter := rbac.NewMockPermissionGetter([]*auth.Role{testUserRole}, []*auth.RoleBinding{testUserRoleBinding}, nil, nil)
	for _, test := range tests {
		nctx := apigwpkg.NewContextWithUser(context.TODO(), test.user)
		nctx, err := newContextWithUserPerms(nctx, permissionGetter, logger)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		perms, _, _ := authzgrpcctx.PermsFromOutgoingContext(nctx)
		Assert(t, rbac.ArePermsEqual(test.expectedPerms, perms),
			fmt.Sprintf("[%s] test failed, expected perms [%s], got [%s]", test.name, rbac.PrintPerms(test.name, test.expectedPerms), rbac.PrintPerms(test.name, perms)))

		isAdmin, _ := authzgrpcctx.UserIsAdminFromOutgoingContext(nctx)
		Assert(t, reflect.DeepEqual(test.expectedAdmin, isAdmin),
			fmt.Sprintf("[%s] test failed, expected isAdmin to be [%v], got [%v]", test.name, test.expectedAdmin, isAdmin))
	}
}
