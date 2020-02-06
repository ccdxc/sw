package impl

import (
	"context"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestIPAddressEmptyInWorkload(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
	logConfig.LogToStdout = true
	logConfig.Filter = log.AllowAllFilter
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

func initWorkLoadHooksAndKVStore(t *testing.T) (*workloadHooks, kvstore.Interface) {
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
	return s, kvs
}

func TestMigration(t *testing.T) {
	s, kvs := initWorkLoadHooksAndKVStore(t)
	work := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "workload-1",
		},
		Spec: workload.WorkloadSpec{
			HostName: "host-1",
			Interfaces: []workload.WorkloadIntfSpec{
				workload.WorkloadIntfSpec{
					MACAddress:   "00aa.bbcc.0000",
					IpAddresses:  []string{},
					ExternalVlan: 100,
					MicroSegVlan: 1000,
				},
				workload.WorkloadIntfSpec{
					MACAddress:   "00aa.bbcc.0001",
					IpAddresses:  []string{"1.2.3.2"},
					ExternalVlan: 100,
					MicroSegVlan: 1001,
				},
			},
		},
	}
	kvs.Create(context.Background(), work.MakeKey("Workload"), &work)

	s.logger.Infof("==== Start Migration1\n")
	// create migration workload
	work2 := work
	work2.Spec.HostName = "host-2"
	work2.Spec.Interfaces[0].MicroSegVlan = 2000
	work2.Spec.Interfaces[1].MicroSegVlan = 2001
	_, _, err := s.processStartMigration(context.Background(), kvs, kvs.NewTxn(), work.MakeKey("Workload"), apiintf.UpdateOper, false, work2)
	AssertOk(t, err, "Failed to Start Migration")
	newWl := &workload.Workload{}
	kvs.Get(context.Background(), work.MakeKey("Workload"), newWl)

	Assert(t, newWl.Status.HostName == "host-1", "Status HostName is not updated")
	Assert(t, newWl.Status.MigrationStatus.Stage == stageMigrationStart, "Migration stage is not START")
	Assert(t, newWl.Status.Interfaces[0].MicroSegVlan == 1000, "Incorrect old useg vlan on interface 0")
	Assert(t, newWl.Status.Interfaces[1].MicroSegVlan == 1001, "Incorrect old useg vlan on interface 1")

	Assert(t, newWl.Spec.HostName == "host-2", "Spec HostName is not updated")
	Assert(t, newWl.Spec.Interfaces[0].MicroSegVlan == 2000, "Incorrect new useg vlan on interface 0")
	Assert(t, newWl.Spec.Interfaces[1].MicroSegVlan == 2001, "Incorrect new useg vlan on interface 1")

	s.logger.Infof("==== Finish Migration1\n")
	// finish migration
	_, _, err = s.processFinishMigration(context.Background(), kvs, kvs.NewTxn(), work.MakeKey("Workload"), apiintf.UpdateOper, false, work2)
	AssertOk(t, err, "Failed to FinishMigration")

	// newWl = obj.(workload.Workload)
	kvs.Get(context.Background(), work.MakeKey("Workload"), newWl)
	Assert(t, newWl.Status.MigrationStatus.Stage == stageMigrationDone, "Migration stage is not DONE")
	// mark it done so we can start another migration for testing
	newWl.Status.MigrationStatus.Status = statusDone
	kvs.Update(context.Background(), work.MakeKey("Workload"), newWl)

	// Abort migration - test
	// start a new migration
	s.logger.Infof("==== Start Migration2\n")
	work2.Spec.HostName = "host-3"
	work2.Spec.Interfaces[0].MicroSegVlan = 3000
	work2.Spec.Interfaces[1].MicroSegVlan = 3001
	_, _, err = s.processStartMigration(context.Background(), kvs, kvs.NewTxn(), work.MakeKey("Workload"), apiintf.UpdateOper, false, work2)
	AssertOk(t, err, "Failed to Start Migration2")
	// newWl = obj.(workload.Workload)
	kvs.Get(context.Background(), work.MakeKey("Workload"), newWl)

	Assert(t, newWl.Status.MigrationStatus.Stage == stageMigrationStart, "Migration stage is not START")

	// Abort migration
	s.logger.Infof("==== Abort Migration2\n")
	_, _, err = s.processAbortMigration(context.Background(), kvs, kvs.NewTxn(), work.MakeKey("Workload"), apiintf.UpdateOper, false, work2)
	AssertOk(t, err, "Failed to Abort Migration2")

	// newWl = obj.(workload.Workload)
	kvs.Get(context.Background(), work.MakeKey("Workload"), newWl)
	Assert(t, newWl.Spec.HostName == "host-2", "Spec HostName is not updated")
	Assert(t, newWl.Spec.Interfaces[0].MicroSegVlan == 2000, "Incorrect old useg vlan on interface 0")
	Assert(t, newWl.Spec.Interfaces[1].MicroSegVlan == 2001, "Incorrect old useg vlan on interface 1")

	Assert(t, newWl.Status.HostName == "host-2", "Spec HostName is not updated")
	Assert(t, newWl.Status.Interfaces[0].MicroSegVlan == 2000, "Incorrect new useg vlan on interface 0")
	Assert(t, newWl.Status.Interfaces[1].MicroSegVlan == 2001, "Incorrect new useg vlan on interface 1")

	// Abort migration - should fail
	s.logger.Infof("==== Abort (Again) Migration2 should fail\n")
	_, _, err = s.processAbortMigration(context.Background(), kvs, kvs.NewTxn(), work.MakeKey("Workload"), apiintf.CreateOper, false, work)
	Assert(t, err != nil, "Abort migration2 should have failed (already aborted)")

	// Finish migration - should fail
	s.logger.Infof("==== Finish (Again) Migration2 should fail\n")
	_, _, err = s.processFinishMigration(context.Background(), kvs, kvs.NewTxn(), work.MakeKey("Workload"), apiintf.CreateOper, false, work)
	Assert(t, err != nil, "Finish migration2 should have failed (already aborted)")
}
