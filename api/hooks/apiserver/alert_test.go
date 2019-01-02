package impl

import (
	"context"
	"errors"
	"reflect"
	"testing"

	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/memkv"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// tests the behavior of different alert state changes
func TestAlertHooks(t *testing.T) {
	userCtx, kvs, ap1Key, a1Key := setup(t)

	tests := []struct {
		name        string          // user info
		ctx         context.Context // context to be passed to the hook
		key         string          // alert key that's getting changed
		acknowledge bool            // true - change to acknowledged state
		resolve     bool            // true - change to resolved state
		open        bool            // true - change to open state
		skipBackend bool            // indicates whether the response from hook should skip the backend call (backend will be skipped on failure)
		err         error           // any expected error from the hook
	}{
		{
			name:        "open to acknowledged state",
			ctx:         userCtx,
			key:         a1Key,
			acknowledge: true,
			skipBackend: false,
			err:         nil,
		},
		{
			name:        "acknowledged to resolved state",
			ctx:         userCtx,
			key:         a1Key,
			resolve:     true,
			skipBackend: false,
			err:         nil,
		},
		{
			name:        "resolved to acknowledged state",
			ctx:         userCtx,
			key:         a1Key,
			acknowledge: true,
			skipBackend: false,
			err:         nil,
		},
		{
			name:        "acknowledged to resolved state",
			ctx:         userCtx,
			key:         a1Key,
			resolve:     true,
			skipBackend: false,
			err:         nil,
		},
		{
			name:        "resolved to open state",
			ctx:         userCtx,
			key:         a1Key,
			open:        true,
			skipBackend: false,
			err:         nil,
		},
		{
			name:        "open to resolved state",
			ctx:         userCtx,
			key:         a1Key,
			resolve:     true,
			skipBackend: false,
			err:         nil,
		},
		{
			name:        "resolved to acknowledged state",
			ctx:         userCtx,
			key:         a1Key,
			acknowledge: true,
			skipBackend: false,
			err:         nil,
		},
		{
			name:        "acknowledged to open state",
			ctx:         userCtx,
			key:         a1Key,
			open:        true,
			skipBackend: false,
			err:         nil,
		},
		// negative cases
		{
			name:        "invalid alert key",
			ctx:         userCtx,
			key:         "invalid",
			skipBackend: false,
			err:         kvstore.NewKeyNotFoundError("invalid", 0),
		},
		{
			name:        "no user in the context",
			ctx:         context.Background(),
			key:         a1Key,
			skipBackend: false,
			err:         errors.New("no user found in context"),
		},
	}

	ah := &alertHooks{logger: log.GetNewLogger(log.GetDefaultConfig("TestAPIServerAlertHooks"))}
	for _, tc := range tests {
		alertObj := &monitoring.Alert{}
		err := kvs.Get(context.Background(), a1Key, alertObj)
		AssertOk(t, err, "failed to get alert from kv store, err: %v", err)

		// get the current alert policy status
		ap := &monitoring.AlertPolicy{}
		err = kvs.Get(context.Background(), ap1Key, ap)
		AssertOk(t, err, "failed to get alert policy, err: %v", err)

		if tc.acknowledge {
			alertObj.Spec.State = monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_ACKNOWLEDGED)]

			out, skipBackend, err := ah.updateStatus(tc.ctx, kvs, kvs.NewTxn(), tc.key, apiserver.UpdateOper, false, *alertObj)
			Assert(t, reflect.DeepEqual(tc.err, err), "tc {%s}: updateStatus hook failed, expected err: %v, got: %v", tc.name, tc.err, err)
			Assert(t, tc.skipBackend == skipBackend, "tc {%s}: expected to skip backend: %v, got: %v", tc.name, tc.skipBackend, skipBackend)

			if err == nil {
				outObj := out.(monitoring.Alert)
				Assert(t, outObj.Status.Acknowledged != nil, "tc %v: failed to update status", tc.name)

				updatedAP := &monitoring.AlertPolicy{}
				err = kvs.Get(context.Background(), ap1Key, updatedAP)
				AssertOk(t, err, "tc {%s}: failed to get alert policy from kvs, err: %v", tc.name, err)
				Assert(t, (ap.Status.AcknowledgedAlerts+1) == updatedAP.Status.AcknowledgedAlerts,
					"tc {%s}: expected acknowledged alerts to be :%d, got: %d", tc.name, ap.Status.AcknowledgedAlerts+1, updatedAP.Status.AcknowledgedAlerts)
				// OPEN     -> ACK ==> (ap.Status.OpenAlerts == updatedAP.Status.OpenAlerts)
				// RESOLVED -> ACK ==> ((ap.Status.OpenAlerts +1) == updatedAP.Status.OpenAlerts)
				Assert(t, ap.Status.OpenAlerts == updatedAP.Status.OpenAlerts || (ap.Status.OpenAlerts+1) == updatedAP.Status.OpenAlerts,
					"tc {%s}: expected open alerts to be :%d or %d , got: %d", tc.name, ap.Status.OpenAlerts, ap.Status.OpenAlerts+1, updatedAP.Status.OpenAlerts)
			}
		} else if tc.resolve {
			alertObj.Spec.State = monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_RESOLVED)]

			out, skipBackend, err := ah.updateStatus(tc.ctx, kvs, kvs.NewTxn(), tc.key, apiserver.UpdateOper, false, *alertObj)
			Assert(t, reflect.DeepEqual(tc.err, err), "tc {%s}: updateStatus hook failed, expected err: %v, got: %v", tc.name, tc.err, err)
			Assert(t, tc.skipBackend == skipBackend, "tc {%s}: expected to skip backend: %v, got: %v", tc.name, tc.skipBackend, skipBackend)

			if err == nil {
				outObj := out.(monitoring.Alert)
				Assert(t, outObj.Status.Resolved != nil, "tc {%s}: failed to update status", tc.name)

				updatedAP := &monitoring.AlertPolicy{}
				err = kvs.Get(context.Background(), ap1Key, updatedAP)
				AssertOk(t, err, "failed to get alert policy from kvs, err: %v", err)
				// OPEN -> RESOLVED ==> (ap.Status.AcknowledgedAlerts == updatedAP.Status.AcknowledgedAlerts)
				// ACK  -> RESOLVED ==> ((ap.Status.AcknowledgedAlerts-1) == updatedAP.Status.AcknowledgedAlerts)
				Assert(t, ap.Status.AcknowledgedAlerts == updatedAP.Status.AcknowledgedAlerts || (ap.Status.AcknowledgedAlerts-1) == updatedAP.Status.AcknowledgedAlerts,
					"tc {%s}: expected acknowledged alerts to be :%d or %d, got: %d", tc.name, ap.Status.AcknowledgedAlerts, ap.Status.AcknowledgedAlerts-1, updatedAP.Status.AcknowledgedAlerts)
				Assert(t, ap.Status.OpenAlerts-1 == updatedAP.Status.OpenAlerts,
					"tc {%s}: expected open alerts to be :%d, got: %d", tc.name, ap.Status.OpenAlerts-1, updatedAP.Status.OpenAlerts)
			}
		} else if tc.open {
			alertObj.Spec.State = monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_OPEN)]

			out, skipBackend, err := ah.updateStatus(tc.ctx, kvs, kvs.NewTxn(), tc.key, apiserver.UpdateOper, false, *alertObj)
			Assert(t, reflect.DeepEqual(tc.err, err), "tc {%s}: updateStatus hook failed, expected err: %v, got: %v", tc.name, tc.err, err)
			Assert(t, tc.skipBackend == skipBackend, "tc {%s}: expected to skip backend: %v, got: %v", tc.name, tc.skipBackend, skipBackend)

			if err == nil {
				outObj := out.(monitoring.Alert)
				Assert(t, outObj.Status.Resolved == nil, "tc {%s}: failed to update status, resolved: %v", tc.name, outObj.Status.Resolved)
				Assert(t, outObj.Status.Acknowledged == nil, "tc {%s}: failed to update status, acknowledged: %v", tc.name, outObj.Status.Acknowledged)

				updatedAP := &monitoring.AlertPolicy{}
				err = kvs.Get(context.Background(), ap1Key, updatedAP)
				AssertOk(t, err, "failed to get alert policy from kvs, err: %v", err)
				// RESOLVED -> OPEN ==> (ap.Status.AcknowledgedAlerts == updatedAP.Status.AcknowledgedAlerts)
				// ACK      -> OPEN ==> ((ap.Status.AcknowledgedAlerts-1) == updatedAP.Status.AcknowledgedAlerts)
				Assert(t, ap.Status.AcknowledgedAlerts == updatedAP.Status.AcknowledgedAlerts || (ap.Status.AcknowledgedAlerts-1) == updatedAP.Status.AcknowledgedAlerts,
					"tc {%s}: expected acknowledged alerts to be :%d or %d, got: %d", tc.name, ap.Status.AcknowledgedAlerts, ap.Status.AcknowledgedAlerts-1, updatedAP.Status.AcknowledgedAlerts)
				// RESOLVED -> OPEN ==> ((ap.Status.OpenAlerts+1) == updatedAP.Status.OpenAlerts)
				// ACK 		-> OPEN ==> ap.Status.OpenAlerts == updatedAP.Status.OpenAlerts
				Assert(t, ap.Status.OpenAlerts == updatedAP.Status.OpenAlerts || (ap.Status.OpenAlerts+1) == updatedAP.Status.OpenAlerts,
					"tc {%s}: expected open alerts to be :%d or %d, got: %d", tc.name, ap.Status.OpenAlerts, ap.Status.OpenAlerts+1, updatedAP.Status.OpenAlerts)
			}
		} else {
			_, skipBackend, err := ah.updateStatus(tc.ctx, kvs, kvs.NewTxn(), tc.key, apiserver.UpdateOper, false, *alertObj)
			Assert(t, reflect.DeepEqual(tc.err, err), "tc {%s}: updateStatus hook failed, expected err: %v, got: %v", tc.name, tc.err, err)
			Assert(t, tc.skipBackend == skipBackend, "tc {%s}: expected to skip backend: %v, got: %v", tc.name, tc.skipBackend, skipBackend)
		}
	}
}

func setup(t *testing.T) (context.Context, kvstore.Interface, string, string) {
	userTenantKey := "pensando-venice-user-tenant-key"
	usernameKey := "pensando-venice-user-key"

	// create context
	userCtx := metadata.NewIncomingContext(context.TODO(), metadata.MD{usernameKey: []string{"testuser"}, userTenantKey: []string{"default"}})

	// create kv store
	scheme := runtime.NewScheme()
	scheme.AddKnownTypes(&monitoring.Alert{})
	scheme.AddKnownTypes(&monitoring.AlertPolicy{})
	kvs, err := memkv.NewMemKv([]string{"test-cluster"}, runtime.NewJSONCodec(scheme))
	AssertOk(t, err, "failed to create in mem kv store, err: %v", err)

	// create alert policy
	ap1Key := (&monitoring.AlertPolicy{
		ObjectMeta: api.ObjectMeta{Name: "ap1", Tenant: globals.DefaultTenant},
	}).MakeKey("monitoring")
	err = kvs.Create(context.Background(),
		ap1Key,
		&monitoring.AlertPolicy{
			TypeMeta: api.TypeMeta{Kind: "AlertPolicy"},
			ObjectMeta: api.ObjectMeta{
				Name:   "ap1",
				Tenant: globals.DefaultTenant,
			},
			Status: monitoring.AlertPolicyStatus{
				OpenAlerts: 1,
			},
		},
	)
	AssertOk(t, err, "failed to create alert policy, err: %v", err)

	// create alert
	a1Key := (&monitoring.Alert{
		ObjectMeta: api.ObjectMeta{Name: "a1", Tenant: globals.DefaultTenant},
	}).MakeKey("monitoring")
	err = kvs.Create(context.Background(),
		a1Key,
		&monitoring.Alert{
			TypeMeta: api.TypeMeta{Kind: "Alert"},
			ObjectMeta: api.ObjectMeta{
				Name:   "a1",
				Tenant: globals.DefaultTenant,
			},
			Spec: monitoring.AlertSpec{
				State: monitoring.AlertSpec_AlertState_name[int32(monitoring.AlertSpec_OPEN)],
			},
			Status: monitoring.AlertStatus{
				Reason: monitoring.AlertReason{
					PolicyID: "ap1",
				},
			},
		},
	)
	AssertOk(t, err, "failed to create alert, err: %v", err)

	return userCtx, kvs, ap1Key, a1Key
}
