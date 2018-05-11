package impl

import (
	"context"
	"testing"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apiserver"
	apisrvmocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

var testMirrorSessions = []monitoring.MirrorSession{
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
			PacketFilters: []string{"ALL_PKTS"},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 1000,
			},
			Collectors: []monitoring.MirrorCollector{},

			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{
						Endpoints: []string{"Endpoint1"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"1234"},
					},
				},
				{
					Src: &monitoring.MatchSelector{
						Endpoints: []string{"Endpoint1"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"TCP/1234"},
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
			PacketFilters: []string{"ALL_DROPS"},
			StartConditions: monitoring.MirrorStartConditions{
				// schedule *after* 10 sec - Fix it based on current time when creating a session
				ScheduleTime: &api.Timestamp{
					Timestamp: types.Timestamp{
						Seconds: 10,
					},
				},
			},

			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 1000,
				ExpiryDuration: "5m",
			},

			Collectors: []monitoring.MirrorCollector{
				{
					Type: "ERSPAN",
					ExportCfg: api.ExportConfig{
						Destination: "111.1.1.1",
					},
				},
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
						Ports: []string{"UDP"},
					},
				},
			},
		},
	},
	{
		// bad schedule
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
			PacketFilters: []string{"ALL_DROPS"},
			StartConditions: monitoring.MirrorStartConditions{
				// schedule *after* 10 sec - Fix it based on current time when creating a session
				ScheduleTime: &api.Timestamp{
					Timestamp: types.Timestamp{
						Seconds: 10,
					},
				},
			},

			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 1000,
				ExpiryDuration: "5m",
			},

			Collectors: []monitoring.MirrorCollector{
				{
					Type: "ERSPAN",
					ExportCfg: api.ExportConfig{
						Destination: "111.1.1.1",
					},
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"UDP"},
					},
				},
			},
		},
	},
	{
		// bad max pkt count
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
			PacketFilters: []string{"ALL_DROPS"},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 0,
				ExpiryDuration: "5m",
			},

			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"UDP"},
					},
				},
			},
		},
	},
	{
		// matchAll
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
			PacketFilters: []string{"ALL_PKTS"},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 50,
				ExpiryDuration: "5m",
			},

			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"UDP"},
					},
				},
			},
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
			PacketFilters: []string{"ALL_DROPS"},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 5000,
				ExpiryDuration: "5m",
			},

			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"UDP"},
					},
				},
			},
		},
	},
	{
		// Empty src/dst rules
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
			PacketFilters: []string{"ALL_PKTS"},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 500,
				ExpiryDuration: "5m",
			},

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
						Ports: []string{"UDP"},
					},
				},
			},
		},
	},
	{
		// Match-all + specific rule
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
			PacketFilters: []string{"ALL_DROPS"},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 500,
				ExpiryDuration: "5m",
			},

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
						Ports: []string{"UDP"},
					},
				},
				{
					Src: &monitoring.MatchSelector{
						Endpoints: []string{"Endpoint1"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"1234"},
					},
				},
			},
		},
	},
	{
		// drop reason + drop all
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
			PacketFilters: []string{"ALL_DROPS", "NETWORK_POLICY_DROP"},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 500,
				ExpiryDuration: "5m",
			},

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
						Ports: []string{"UDP"},
					},
				},
				{
					Src: &monitoring.MatchSelector{
						Endpoints: []string{"Endpoint1"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"1234"},
					},
				},
			},
		},
	},
	{
		// max pkt size
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 10",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    1024,
			PacketFilters: []string{"ALL_PKTS"},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 500,
				ExpiryDuration: "5m",
			},

			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{
						Endpoints: []string{"Endpoint1"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"1234"},
					},
				},
			},
		},
	},
	{
		// matchAll-2
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
			PacketFilters: []string{"ALL_PKTS"},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 50,
				ExpiryDuration: "5m",
			},

			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},
			MatchRules: []monitoring.MatchRule{},
		},
	},
}

func TestBadMirrorSessions(t *testing.T) {
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
	for _, ms := range testMirrorSessions {
		_, ok, err := s.validateMirrorSession(ctx, kvs, txn, ms.MakeKey(""), apiserver.CreateOper, ms)
		if ok {
			t.Errorf("hook passed, expecting to fail")
			continue
		}
		l.Infof("Session %v : Error %v", ms.Name, err)
	}
}
