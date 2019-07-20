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

func TestValidateFwlogPolicy(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute*5)
	defer cancel()

	logConfig := &log.Config{
		Module:      "fwlog-hooks",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	l := log.SetConfig(logConfig)
	s := &fwlogHooks{
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

	testFwlogSpecList := []struct {
		name   string
		policy monitoring.FwlogPolicy
		fail   bool
	}{
		{
			name: "invalid protocol",
			fail: true,
			policy: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{
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
			policy: monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "FwlogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FwlogPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Targets: []monitoring.ExportConfig{
						{
							Destination: "10.1.1.100",
							Transport:   "TCP/1234",
						},
					},
				},
			},
		},
	}

	for i := range testFwlogSpecList {
		_, ok, err := s.validateFwlogPolicy(ctx, kvs, txn, "", apiintf.CreateOper, false,
			testFwlogSpecList[i].policy)

		if testFwlogSpecList[i].fail == true {
			t.Logf(fmt.Sprintf("test [%v] returned %v", testFwlogSpecList[i].name, err))
			testutils.Assert(t, ok == false, "test [%v] returned %v", testFwlogSpecList[i].name, err)
		} else {
			t.Log(fmt.Sprintf("test [%v] returned %v", testFwlogSpecList[i].name, err))
			testutils.Assert(t, ok == true, "test [%v] returned %v", testFwlogSpecList[i].name, err)
		}
	}
}
