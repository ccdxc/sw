package impl

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/testutils"
)

func TestValidateFlowExportPolicy(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute*5)
	defer cancel()

	logConfig := &log.Config{
		Module:      "Mirror-hooks",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	l := log.SetConfig(logConfig)
	s := &flowExpHooks{
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

	testFlowExpSpecList := []struct {
		name   string
		policy monitoring.FlowExportPolicy
		fail   bool
	}{
		{
			name: "invalid protocol",
			fail: true,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval: "15s",
					Format:   "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "IP/1234",
						},
					},
				},
			},
		},

		{
			name: "valid policy",
			fail: false,
			policy: monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					MatchRules: []monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.1"},
							},

							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.1.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1000"},
							},
						},

						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.1"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"1.1.2.2"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"TCP/1010"},
							},
						},
					},

					Interval: "15s",
					Format:   "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "TCP/1234",
						},
					},
				},
			},
		},
	}

	for i := range testFlowExpSpecList {
		_, ok, err := s.validateFlowExportPolicy(ctx, kvs, txn, "", apiintf.CreateOper, false,
			testFlowExpSpecList[i].policy)

		if testFlowExpSpecList[i].fail == true {
			t.Logf(fmt.Sprintf("test [%v] returned %v", testFlowExpSpecList[i].name, err))
			testutils.Assert(t, ok == false, "test [%v] returned %v", testFlowExpSpecList[i].name, err)
		} else {
			t.Log(fmt.Sprintf("test [%v] returned %v", testFlowExpSpecList[i].name, err))
			testutils.Assert(t, ok == true, "test [%v] returned %v", testFlowExpSpecList[i].name, err)
		}
	}
}
