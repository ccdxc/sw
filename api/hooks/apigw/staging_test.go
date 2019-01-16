package impl

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/staging"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
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
		nctx, out, skipCall, err := r.userContext(nctx, test.in)
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
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerUserContextHook(svc)
	Assert(t, err != nil, "expected error in userContext hook registration")
}
