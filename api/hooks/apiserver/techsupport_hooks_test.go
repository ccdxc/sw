package impl

import (
	"context"
	"testing"

	"github.com/pensando/sw/api/interfaces"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/labels"
	apisrvmocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

var testBadTechSupportRequests = []monitoring.TechSupportRequest{
	{
		ObjectMeta: api.ObjectMeta{
			Name: "Test Bad TSR 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "TechSupportRequest",
			APIVersion: "v1",
		},
		Spec: monitoring.TechSupportRequestSpec{},
	},
	{
		ObjectMeta: api.ObjectMeta{
			Name: "Test Bad TSR 2",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "TechSupportRequest",
			APIVersion: "v1",
		},
		Spec: monitoring.TechSupportRequestSpec{
			NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
				Names: []string{},
			},
		},
	},
	{
		ObjectMeta: api.ObjectMeta{
			Name: "Test Bad TSR 3",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "TechSupportRequest",
			APIVersion: "v1",
		},
		Spec: monitoring.TechSupportRequestSpec{
			NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
				Labels: &labels.Selector{
					Requirements: []*labels.Requirement{},
				},
			},
		},
	},
}

var testGoodTechSupportRequests = []monitoring.TechSupportRequest{
	{
		ObjectMeta: api.ObjectMeta{
			Name: "Test Techsupport Request 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "TechSupportRequest",
			APIVersion: "v1",
		},
		Spec: monitoring.TechSupportRequestSpec{
			NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
				Names: []string{"host1-dsc1"},
			},
		},
	},
	{
		ObjectMeta: api.ObjectMeta{
			Name: "Test Good TSR",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "TechSupportRequest",
			APIVersion: "v1",
		},
		Spec: monitoring.TechSupportRequestSpec{
			NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
				Labels: &labels.Selector{
					Requirements: []*labels.Requirement{labels.MustGetRequirement("star", labels.Operator_notEquals, []string{"true"})},
				},
			},
		},
	},
}

func TestTechSupportRequests(t *testing.T) {
	ctx := context.Background()
	logConfig := &log.Config{
		Module:      "TechSupportRequest-hooks",
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
	s := &techSupportRequestHooks{
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
	for _, ms := range testBadTechSupportRequests {
		_, ok, err := s.validateTechSupportRequest(ctx, kvs, txn, ms.MakeKey(""), apiintf.CreateOper, false, ms)
		if ok {
			t.Errorf("validation passed, expecting to fail for %v", ms.Name)
			continue
		}
		l.Infof("Session %v : Error %v", ms.Name, err)
	}

	for _, ms := range testGoodTechSupportRequests {
		_, ok, err := s.validateTechSupportRequest(ctx, kvs, txn, ms.MakeKey(""), apiintf.CreateOper, false, ms)
		if !ok && err != nil {
			t.Errorf("Failed to create a good techsupport request (%s)", err)
			continue
		}
		l.Infof("Session %v : Error %v", ms.Name, err)
	}
}
