package impl

import (
	"context"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"fmt"
	"net"
	"reflect"
	"testing"
	"time"

	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/memkv"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestValidateAlertDestination(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute*5)
	defer cancel()

	logConfig := &log.Config{
		Module:      "TestValidateAlertDestination",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	l := log.SetConfig(logConfig)
	s := &alertHooks{
		logger: l,
	}
	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}

	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}
	txn := kvs.NewTxn()

	testAlertDestList := []struct {
		name   string
		policy monitoring.AlertDestination
		fail   bool
	}{
		{
			name: "invalid DNS",
			fail: true,
			policy: monitoring.AlertDestination{
				TypeMeta: api.TypeMeta{
					Kind: "alertDestination",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-DNS",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.AlertDestinationSpec{
					SyslogExport: &monitoring.SyslogExport{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "test.pensando.iox",
								Transport:   "TCP/1234",
							},
						},
						Config: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
		},

		{
			name: "duplicate targets",
			fail: true,
			policy: monitoring.AlertDestination{
				TypeMeta: api.TypeMeta{
					Kind: "alertDestination",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "duplicate-targets",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.AlertDestinationSpec{
					SyslogExport: &monitoring.SyslogExport{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "10.1.1.100",
								Transport:   "TCP/1234",
							},
							{
								Destination: "10.1.1.100",
								Transport:   "TCP/1234",
							},
						},
						Config: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
		},

		{
			name: "invalid destination (empty)",
			fail: true,
			policy: monitoring.AlertDestination{
				TypeMeta: api.TypeMeta{
					Kind: "alertDestination",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "empty-destination",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.AlertDestinationSpec{
					SyslogExport: &monitoring.SyslogExport{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "",
								Transport:   "TCP/1234",
							},
						},
						Config: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
		},

		{
			name: "invalid transport (empty)",
			fail: true,
			policy: monitoring.AlertDestination{
				TypeMeta: api.TypeMeta{
					Kind: "alertDestination",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "empty-transport",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.AlertDestinationSpec{
					SyslogExport: &monitoring.SyslogExport{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "10.1.1.100",
								Transport:   "",
							},
						},
						Config: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
		},

		{
			name: "invalid targets (empty)",
			fail: true,
			policy: monitoring.AlertDestination{
				TypeMeta: api.TypeMeta{
					Kind: "alertDestination",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "empty-targets",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.AlertDestinationSpec{
					SyslogExport: &monitoring.SyslogExport{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Config: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
		},

		{
			name: "invalid Transport, missing port",
			fail: true,
			policy: monitoring.AlertDestination{
				TypeMeta: api.TypeMeta{
					Kind: "alertDestination",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-Transport-missing-port",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.AlertDestinationSpec{
					SyslogExport: &monitoring.SyslogExport{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "10.1.1.100",
								Transport:   "TCP",
							},
						},
						Config: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
		},

		{
			name: "invalid Transport, missing protocol",
			fail: true,
			policy: monitoring.AlertDestination{
				TypeMeta: api.TypeMeta{
					Kind: "alertDestination",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-Transport-missing-protocol",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.AlertDestinationSpec{
					SyslogExport: &monitoring.SyslogExport{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "10.1.1.100",
								Transport:   "1234",
							},
						},
						Config: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
		},

		{
			name: "invalid protocol, only TCP/UDP is supported",
			fail: true,
			policy: monitoring.AlertDestination{
				TypeMeta: api.TypeMeta{
					Kind: "alertDestination",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-protocol",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.AlertDestinationSpec{
					SyslogExport: &monitoring.SyslogExport{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "10.1.1.100",
								Transport:   "ICMP/1234",
							},
						},
						Config: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
		},

		{
			name: "invalid port (aaaa)",
			fail: true,
			policy: monitoring.AlertDestination{
				TypeMeta: api.TypeMeta{
					Kind: "alertDestination",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-port-aaaa",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.AlertDestinationSpec{
					SyslogExport: &monitoring.SyslogExport{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "10.1.1.100",
								Transport:   "TCP/aaaa",
							},
						},
						Config: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
		},

		{
			name: "invalid port (65536)",
			fail: true,
			policy: monitoring.AlertDestination{
				TypeMeta: api.TypeMeta{
					Kind: "alertDestination",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "invalid-port-65536",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.AlertDestinationSpec{
					SyslogExport: &monitoring.SyslogExport{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "10.1.1.100",
								Transport:   "TCP/65536",
							},
						},
						Config: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
		},

		{
			name: "valid policy",
			fail: false,
			policy: monitoring.AlertDestination{
				TypeMeta: api.TypeMeta{
					Kind: "alertDestination",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "valid-policy",
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.AlertDestinationSpec{
					SyslogExport: &monitoring.SyslogExport{
						Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
						Targets: []*monitoring.ExportConfig{
							{
								Destination: "10.1.1.100",
								Transport:   "TCP/1234",
							},
						},
						Config: &monitoring.SyslogExportConfig{
							FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
						},
					},
				},
			},
		},
	}

	for i := range testAlertDestList {
		_, ok, err := s.validateAlertDestination(ctx, kvs, txn, "", apiintf.CreateOper, false,
			testAlertDestList[i].policy)

		if testAlertDestList[i].fail == true {
			t.Logf(fmt.Sprintf("test [%v] returned %v", testAlertDestList[i].name, err))
			Assert(t, ok == false, "test [%v] returned %v", testAlertDestList[i].name, err)
		} else {
			t.Log(fmt.Sprintf("test [%v] returned %v", testAlertDestList[i].name, err))
			Assert(t, ok == true, "test [%v] returned %v", testAlertDestList[i].name, err)
		}
	}
}

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
			ctx:         context.Background(), // not a API gw context
			key:         a1Key,
			skipBackend: true,
			err:         nil,
		},
	}

	ah := &alertHooks{logger: log.GetNewLogger(log.GetDefaultConfig("TestAPIServerAlertHooks"))}
	alertObj := &monitoring.Alert{}
	err := kvs.Get(context.Background(), a1Key, alertObj)
	AssertOk(t, err, "failed to get alert from kv store, err: %v", err)

	// get the current alert policy status
	ap := &monitoring.AlertPolicy{}
	err = kvs.Get(context.Background(), ap1Key, ap)
	for _, tc := range tests {

		AssertOk(t, err, "failed to get alert policy, err: %v", err)
		freqs := &mocks.FakeRequirementSet{}
		ctx := apiutils.SetRequirements(tc.ctx, freqs)
		kvAlert := &monitoring.Alert{}
		kvAPol := &monitoring.AlertPolicy{}
		*kvAlert = *alertObj
		*kvAPol = *ap
		switch {
		case tc.acknowledge:
			alertObj.Spec.State = monitoring.AlertState_ACKNOWLEDGED.String()
		case tc.resolve:
			alertObj.Spec.State = monitoring.AlertState_RESOLVED.String()
		case tc.open:
			alertObj.Spec.State = monitoring.AlertState_OPEN.String()
		}

		_, skipBackend, err := ah.updateStatus(ctx, kvs, kvs.NewTxn(), tc.key, apiintf.UpdateOper, false, *alertObj)
		Assert(t, tc.skipBackend == skipBackend, "tc {%s}: expected to skip backend: %v, got: %v", tc.name, tc.skipBackend, skipBackend)
		if err != nil {
			Assert(t, reflect.DeepEqual(tc.err, err), "tc {%s}: updateStatus hook failed, expected err: %v, got: %v", tc.name, tc.err, err)
			continue
		}
		if !tc.skipBackend {
			Assert(t, len(freqs.ConsUpdates) == 1, "tc {%s}: expecting 1 consistent update requirement got [%v]", tc.name, len(freqs.ConsUpdates))
			cis := freqs.ConsUpdates[0].Data.([]apiintf.ConstUpdateItem)
			Assert(t, len(cis) == 2, "tc {%s}: expecting 2 consistent update keys, got [%v]", tc.name, len(cis))
			var uap runtime.Object
			oObj, err := cis[0].Func(kvAlert)
			if err == nil {
				uap, err = cis[1].Func(kvAPol)
			}
			if err != nil {
				Assert(t, reflect.DeepEqual(tc.err, err), "tc {%s}: updateStatus hook failed, expected err: %v, got: %v", tc.name, tc.err, err)
				continue
			}

			updatedAP := uap.(*monitoring.AlertPolicy)
			outObj := oObj.(*monitoring.Alert)
			if tc.acknowledge {
				Assert(t, outObj.Status.Acknowledged != nil, "tc {%s}: failed to update status", tc.name)
				Assert(t, (ap.Status.AcknowledgedAlerts+1) == updatedAP.Status.AcknowledgedAlerts,
					"tc {%s}: expected acknowledged alerts to be :%d, got: %d", tc.name, ap.Status.AcknowledgedAlerts+1, updatedAP.Status.AcknowledgedAlerts)
				// OPEN     -> ACK ==> (ap.Status.OpenAlerts == updatedAP.Status.OpenAlerts)
				// RESOLVED -> ACK ==> ((ap.Status.OpenAlerts +1) == updatedAP.Status.OpenAlerts)
				Assert(t, ap.Status.OpenAlerts == updatedAP.Status.OpenAlerts || (ap.Status.OpenAlerts+1) == updatedAP.Status.OpenAlerts,
					"tc {%s}: expected open alerts to be :%d or %d , got: %d", tc.name, ap.Status.OpenAlerts, ap.Status.OpenAlerts+1, updatedAP.Status.OpenAlerts)
			} else if tc.resolve {
				Assert(t, outObj.Status.Resolved != nil, "tc {%s}: failed to update status", tc.name)
				// OPEN -> RESOLVED ==> (ap.Status.AcknowledgedAlerts == updatedAP.Status.AcknowledgedAlerts)
				// ACK  -> RESOLVED ==> ((ap.Status.AcknowledgedAlerts-1) == updatedAP.Status.AcknowledgedAlerts)
				Assert(t, ap.Status.AcknowledgedAlerts == updatedAP.Status.AcknowledgedAlerts || (ap.Status.AcknowledgedAlerts-1) == updatedAP.Status.AcknowledgedAlerts,
					"tc {%s}: expected acknowledged alerts to be :%d or %d, got: %d", tc.name, ap.Status.AcknowledgedAlerts, ap.Status.AcknowledgedAlerts-1, updatedAP.Status.AcknowledgedAlerts)
				Assert(t, ap.Status.OpenAlerts-1 == updatedAP.Status.OpenAlerts,
					"tc {%s}: expected open alerts to be :%d, got: %d", tc.name, ap.Status.OpenAlerts-1, updatedAP.Status.OpenAlerts)
			} else if tc.open {
				Assert(t, outObj.Status.Resolved == nil, "tc {%s}: failed to update status, resolved: %v", tc.name, outObj.Status.Resolved)
				Assert(t, outObj.Status.Acknowledged == nil, "tc {%s}: failed to update status, acknowledged: %v", tc.name, outObj.Status.Acknowledged)
				// RESOLVED -> OPEN ==> (ap.Status.AcknowledgedAlerts == updatedAP.Status.AcknowledgedAlerts)
				// ACK      -> OPEN ==> ((ap.Status.AcknowledgedAlerts-1) == updatedAP.Status.AcknowledgedAlerts)
				Assert(t, ap.Status.AcknowledgedAlerts == updatedAP.Status.AcknowledgedAlerts || (ap.Status.AcknowledgedAlerts-1) == updatedAP.Status.AcknowledgedAlerts,
					"tc {%s}: expected acknowledged alerts to be :%d or %d, got: %d", tc.name, ap.Status.AcknowledgedAlerts, ap.Status.AcknowledgedAlerts-1, updatedAP.Status.AcknowledgedAlerts)
				// RESOLVED -> OPEN ==> ((ap.Status.OpenAlerts+1) == updatedAP.Status.OpenAlerts)
				// ACK 		-> OPEN ==> ap.Status.OpenAlerts == updatedAP.Status.OpenAlerts
				Assert(t, ap.Status.OpenAlerts == updatedAP.Status.OpenAlerts || (ap.Status.OpenAlerts+1) == updatedAP.Status.OpenAlerts,
					"tc {%s}: expected open alerts to be :%d or %d, got: %d", tc.name, ap.Status.OpenAlerts, ap.Status.OpenAlerts+1, updatedAP.Status.OpenAlerts)
			}
			*alertObj = *outObj
			*ap = *updatedAP
		}
	}
}

func setup(t *testing.T) (context.Context, kvstore.Interface, string, string) {
	userTenantKey := "pensando-venice-user-tenant-key"
	usernameKey := "pensando-venice-user-key"

	// create context
	privateKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		t.Fatalf("error generating key: %v", err)
	}
	cert, err := certs.SelfSign(globals.APIGw, privateKey, certs.WithValidityDays(1))
	if err != nil {
		t.Fatalf("error generating certificate: %v", err)
	}

	mockAPIGWCtx := ctxutils.MakeMockContext(&net.IPAddr{IP: net.ParseIP("1.2.3.4")}, cert)
	userCtx := metadata.NewIncomingContext(mockAPIGWCtx, metadata.MD{usernameKey: []string{"testuser"}, userTenantKey: []string{"default"}})

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
	ap := &monitoring.AlertPolicy{
		TypeMeta: api.TypeMeta{Kind: "AlertPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:   "ap1",
			Tenant: globals.DefaultTenant,
		},
		Status: monitoring.AlertPolicyStatus{
			OpenAlerts: 1,
		},
	}
	err = kvs.Create(context.Background(), ap1Key, ap)
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
				State: monitoring.AlertState_OPEN.String(),
			},
			Status: monitoring.AlertStatus{
				Reason: monitoring.AlertReason{
					PolicyID: fmt.Sprintf("%s/%s", ap.GetName(), ap.GetUUID()),
				},
			},
		},
	)
	AssertOk(t, err, "failed to create alert, err: %v", err)

	return userCtx, kvs, ap1Key, a1Key
}
