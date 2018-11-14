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
		newCtx, out, skipBackendCall, err := ah.addUserInfoToContext(test.ctx, test.in)
		Assert(t, reflect.DeepEqual(test.err, err), "tc %v: resolveAlert hook failed, err: %v", test.name, err)
		Assert(t, reflect.DeepEqual(test.in, out), "tc %v: config object should not change, expected: %v, got: %v", test.name, test.in, out)
		Assert(t, test.skipBackendCall == skipBackendCall, "tc %v: expected skipBackend: %v, got: %v", test.name, test.skipBackendCall, skipBackendCall)

		if err == nil {
			userMeta, ok := authzgrpcctx.UserMetaFromIncomingContext(newCtx)
			Assert(t, !ok, "user meta not found in the context")
			Assert(t, userMeta == nil, "empty user meta")
		}
	}
}
