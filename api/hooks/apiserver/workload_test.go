package impl

import (
	"context"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestIPAddressEmptyInWorkload(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
	s := &workloadHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}

	work := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "workload-1",
		},
		Spec: workload.WorkloadSpec{
			Interfaces: []workload.WorkloadIntfSpec{},
		},
	}

	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}

	kvs, err := store.New(storecfg)
	AssertOk(t, err, "Failed to create kv store. Err: %v", err)

	_, _, err = s.validateIPAddressHook(context.Background(), kvs, kvs.NewTxn(), "", apiintf.CreateOper, false, work)
	Assert(t, err == nil, "failed to create workload Error: %v", err)

}

func TestIPAddressCheckInWorkload(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
	s := &workloadHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}

	work := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "workload-1",
		},
		Spec: workload.WorkloadSpec{
			Interfaces: []workload.WorkloadIntfSpec{
				workload.WorkloadIntfSpec{
					IpAddresses: []string{"1.2.3.4"},
				},
			},
		},
	}

	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}

	kvs, err := store.New(storecfg)
	AssertOk(t, err, "Failed to create kv store. Err: %v", err)

	_, _, err = s.validateIPAddressHook(context.Background(), kvs, kvs.NewTxn(), "", apiintf.CreateOper, false, work)
	Assert(t, err == nil, "failed to create workload Error: %v", err)

	work = workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "workload-1",
		},
		Spec: workload.WorkloadSpec{
			Interfaces: []workload.WorkloadIntfSpec{
				workload.WorkloadIntfSpec{},
			},
		},
	}

	_, _, err = s.validateIPAddressHook(context.Background(), kvs, kvs.NewTxn(), "", apiintf.UpdateOper, false, work)
	Assert(t, err == nil, "failed to create workload Error: %v", err)

	work = workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "workload-1",
		},
		Spec: workload.WorkloadSpec{
			Interfaces: []workload.WorkloadIntfSpec{
				workload.WorkloadIntfSpec{
					IpAddresses: []string{"1.2.3.4", "2.3.4.5"},
				},
			},
		},
	}

	_, _, err = s.validateIPAddressHook(context.Background(), kvs, kvs.NewTxn(), "", apiintf.CreateOper, false, work)
	Assert(t, err != nil, "Created workload Error: %v", err)

	work = workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "workload-1",
		},
		Spec: workload.WorkloadSpec{
			Interfaces: []workload.WorkloadIntfSpec{
				workload.WorkloadIntfSpec{
					IpAddresses: []string{"1.2.3.4", "2.3.4.5"},
				},
			},
		},
	}
	_, _, err = s.validateIPAddressHook(context.Background(), kvs, kvs.NewTxn(), "", apiintf.UpdateOper, false, work)
	Assert(t, err != nil, "Created workload Error: %v", err)
}

func TestIPAddressValidityCheck(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
	s := &workloadHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}

	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}

	kvs, err := store.New(storecfg)
	AssertOk(t, err, "Failed to create kv store. Err: %v", err)

	work := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "workload-1",
		},
		Spec: workload.WorkloadSpec{
			Interfaces: []workload.WorkloadIntfSpec{
				workload.WorkloadIntfSpec{
					IpAddresses: []string{"1.2.3.1222"},
				},
			},
		},
	}

	_, _, err = s.validateIPAddressHook(context.Background(), kvs, kvs.NewTxn(), "", apiintf.CreateOper, false, work)
	Assert(t, err != nil, "Created workload Error: %v", err)

	work = workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "workload-1",
		},
		Spec: workload.WorkloadSpec{
			Interfaces: []workload.WorkloadIntfSpec{
				workload.WorkloadIntfSpec{
					IpAddresses: []string{"a.d.f.d"},
				},
			},
		},
	}

	_, _, err = s.validateIPAddressHook(context.Background(), kvs, kvs.NewTxn(), "", apiintf.CreateOper, false, work)
	Assert(t, err != nil, "Created workload Error: %v", err)
}
