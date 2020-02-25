package impl

import (
	"context"
	"testing"

	"github.com/gogo/protobuf/types"

	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/labels"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	apisrvmocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

var testBadMirrorSessions = []monitoring.MirrorSession{
	{
		// 0 collectors
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 1",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_PKTS.String()},
			Collectors:    []monitoring.MirrorCollector{},

			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{
						IPAddresses: []string{"192.168.100.2"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"1234"},
					},
				},
				{
					Src: &monitoring.MatchSelector{
						IPAddresses: []string{"192.168.100.2"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"TCP/1234"},
					},
				},
			},
		},
	},
	{
		// 2 venice collectors
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 2",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_DROPS.String()},
			StartConditions: monitoring.MirrorStartConditions{
				// schedule *after* 10 sec - Fix it based on current time when creating a session
				ScheduleTime: &api.Timestamp{
					Timestamp: types.Timestamp{
						Seconds: 10,
					},
				},
			},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
				{
					Type: "VENICE",
				},
			},

			MatchRules: []monitoring.MatchRule{
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"UDP"},
					},
				},
			},
		},
	},
	{
		// bad max pkt count
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 3",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_DROPS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"UDP"},
					},
				},
			},
		},
	},
	{
		// matchAll
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 4",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_PKTS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"UDP"},
					},
				},
			},
		},
	},
	{
		// Too many collectors
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 5",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_DROPS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
				{
					Type: "ERSPAN",
					ExportCfg: &monitoring.MirrorExportConfig{
						Destination: "111.1.1.1",
					},
				},
				{
					Type: "ERSPAN",
					ExportCfg: &monitoring.MirrorExportConfig{
						Destination: "111.1.1.2",
					},
				},
			},
			MatchRules: []monitoring.MatchRule{},
		},
	},
	{
		// bad max pkt count
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 6",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_DROPS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"UDP"},
					},
				},
			},
		},
	},
	{
		// both match rules and interface selector
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 7",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_DROPS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"UDP"},
					},
				},
			},

			InterfaceSelector: labels.SelectorFromSet(labels.Set{"app": "procurement"}),
		},
	},

	{
		// Interface selector not with ALL_PKTS filter
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 8",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_DROPS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			InterfaceSelector: labels.SelectorFromSet(labels.Set{"app": "procurement"}),
		},
	},
	{
		// Empty src/dst rules
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 9",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_PKTS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{},
					Dst: &monitoring.MatchSelector{},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"UDP"},
					},
				},
			},
		},
	},
	{
		// Match-all + specific rule
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 10",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_DROPS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{},
					Dst: &monitoring.MatchSelector{},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"UDP"},
					},
				},
				{
					Src: &monitoring.MatchSelector{
						IPAddresses: []string{"192.168.100.2"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"1234"},
					},
				},
			},
		},
	},
	{
		// drop reason + drop all
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 11",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_DROPS.String(), monitoring.MirrorSessionSpec_NETWORK_POLICY_DROP.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{},
					Dst: &monitoring.MatchSelector{},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"UDP"},
					},
				},
				{
					Src: &monitoring.MatchSelector{
						IPAddresses: []string{"192.168.100.2"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"1234"},
					},
				},
			},
		},
	},
	{
		// max pkt size
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 12",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    1024,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_PKTS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{
						IPAddresses: []string{"192.168.100.2"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"1234"},
					},
				},
			},
		},
	},
	{
		// matchAll-2 empty match rule
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 13",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_PKTS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{},
		},
	},
}

var testGoodMirrorSession = []monitoring.MirrorSession{
	{
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 1",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_PKTS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "ERSPAN",
					ExportCfg: &monitoring.MirrorExportConfig{
						Destination: "127.0.0.1",
					},
				},
			},

			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{
						IPAddresses: []string{"192.168.100.2"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"TCP/1234"},
					},
				},
			},
		},
	},
}

func TestMirrorSessions(t *testing.T) {
	ctx := context.Background()
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
	service := apisrvmocks.NewFakeService()
	s := &mirrorSessionHooks{
		svc:    service,
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
	for _, ms := range testBadMirrorSessions {
		_, ok, err := s.validateMirrorSession(ctx, kvs, txn, ms.MakeKey(""), apiintf.CreateOper, false, ms)
		if ok {
			t.Errorf("validation passed, expecting to fail for %v", ms.Name)
			continue
		}
		l.Infof("Session %v : Error %v", ms.Name, err)
	}
	ms := &testGoodMirrorSession[0]
	ms.Normalize()
	_, ok, err := s.validateMirrorSession(ctx, kvs, txn, ms.MakeKey(""), apiintf.CreateOper, false, *ms)
	if !ok && err != nil {
		t.Errorf("Failed to create a good mirror session (%s)", err)
	}
}
