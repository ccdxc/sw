package impl

import (
	"context"
	"errors"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apigw/pkg"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestAlertHooks(t *testing.T) {
	tests := []struct {
		name            string
		ctx             context.Context
		in              interface{}
		skipBackendCall bool
		err             error
	}{
		{
			name: "valid context for the hook",
			ctx: apigwpkg.NewContextWithUser(context.TODO(), &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
			}),
			in:              &monitoring.Alert{ObjectMeta: api.ObjectMeta{Name: "test-alert1"}},
			skipBackendCall: false,
			err:             nil,
		},
		{
			name:            "no user in the context",
			ctx:             context.TODO(),
			in:              nil,
			skipBackendCall: true,
			err:             apigwpkg.ErrNoUserInContext,
		},
		{
			name: "user with empty tenant",
			ctx: apigwpkg.NewContextWithUser(context.TODO(), &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Name: "testUser",
				},
			}),
			in:              nil,
			skipBackendCall: true,
			err:             errors.New("tenant not populated in user"),
		},
	}

	ah := &alertHooks{logger: log.GetNewLogger(log.GetDefaultConfig("TestAPIGwAlertHooks"))}
	for _, test := range tests {
		newCtx, out, _, skipBackendCall, err := ah.addUserInfoToContext(test.ctx, test.in, nil)
		Assert(t, reflect.DeepEqual(test.err, err), "tc {%s}: resolveAlert hook failed, err: %v", test.name, err)
		Assert(t, reflect.DeepEqual(test.in, out), "tc {%s}: config object should not change, expected: %v, got: %v", test.name, test.in, out)
		Assert(t, test.skipBackendCall == skipBackendCall, "tc {%s}: expected skipBackend: %v, got: %v", test.name, test.skipBackendCall, skipBackendCall)

		if err == nil {
			userMeta, ok := authzgrpcctx.UserMetaFromOutgoingContext(newCtx)
			Assert(t, ok, "tc {%s}: user meta not found in the context", test.name)
			Assert(t, userMeta != nil, "tc {%s}: empty user meta", test.name)

			userFromCtx, ok := apigwpkg.UserFromContext(newCtx)
			Assert(t, ok, "failed to read user from context")
			Assert(t, userFromCtx.GetTenant() == userMeta.GetTenant(), "expected user tenant: %s, got: %s", userFromCtx.GetTenant(), userMeta.GetTenant())
			Assert(t, userFromCtx.GetName() == userMeta.GetName(), "expected user name: %s, got: %s", userFromCtx.GetName(), userMeta.GetName())
		}
	}
}
