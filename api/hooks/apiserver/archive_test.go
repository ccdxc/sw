package impl

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestArchiveRequestDeleteCheck(t *testing.T) {
	tests := []struct {
		name     string
		in       interface{}
		existing *monitoring.ArchiveRequest
		result   bool
		out      interface{}
		err      error
	}{
		{
			name: "delete completed archive request",
			in: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
			existing: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
				Status: monitoring.ArchiveRequestStatus{
					Status: monitoring.ArchiveRequestStatus_Completed.String(),
				},
			},
			result: true,
			err:    nil,
			out: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
		},
		{
			name: "delete failed archive request",
			in: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
			existing: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
				Status: monitoring.ArchiveRequestStatus{
					Status: monitoring.ArchiveRequestStatus_Failed.String(),
				},
			},
			result: true,
			err:    nil,
			out: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
		},
		{
			name: "delete running archive request",
			in: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
			existing: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
				Status: monitoring.ArchiveRequestStatus{
					Status: monitoring.ArchiveRequestStatus_Running.String(),
				},
			},
			result: true,
			err:    errors.New("archive request not processed yet"),
			out: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
		},
		{
			name: "delete scheduled archive request",
			in: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
			existing: &monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
				Status: monitoring.ArchiveRequestStatus{
					Status: monitoring.ArchiveRequestStatus_Scheduled.String(),
				},
			},
			result: true,
			err:    errors.New("archive request not processed yet"),
			out: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
		},
		{
			name: "delete non-existent archive request",
			in: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
			existing: nil,
			result:   true,
			err:      kvstore.NewKeyNotFoundError("", 0),
			out: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
		},
	}

	logConfig := log.GetDefaultConfig("TestArchiveHooks")
	l := log.GetNewLogger(logConfig)
	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}

	archiveHooks := &archiveHooks{
		logger: l,
	}
	for _, test := range tests {
		ctx := context.TODO()
		txn := kvs.NewTxn()
		var reqKey string
		if test.existing != nil {
			reqKey = test.existing.MakeKey("monitoring")
			if err := kvs.Create(ctx, reqKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with archive request, Err: %v", test.name, err)
			}
		}
		out, ok, err := archiveHooks.archiveRequestDeleteCheck(ctx, kvs, txn, reqKey, apiintf.DeleteOper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected return object [%#v], got [%#v]", test.name, test.out, out))
		kvs.Delete(ctx, reqKey, nil)
	}
}

func TestArchiveRequestUpgradeCheck(t *testing.T) {
	tests := []struct {
		name     string
		in       interface{}
		verObj   *cluster.Version
		result   bool
		out      interface{}
		txnEmpty bool
		err      error
	}{
		{
			name: "upgrade in progress",
			in: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
			verObj: &cluster.Version{
				TypeMeta: api.TypeMeta{
					Kind: string(cluster.KindVersion),
				},
				ObjectMeta: api.ObjectMeta{
					Name: "Version",
				},
				Status: cluster.VersionStatus{
					RolloutBuildVersion: "1.5",
				},
			},
			result: true,
			err:    errors.New("rollout in progress, restore operation not allowed"),
			out: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
			txnEmpty: false,
		},
		{
			name: "no upgrade",
			in: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
			verObj: &cluster.Version{
				TypeMeta: api.TypeMeta{
					Kind: string(cluster.KindVersion),
				},
				ObjectMeta: api.ObjectMeta{
					Name: "Version",
				},
				Status: cluster.VersionStatus{
					RolloutBuildVersion: "",
				},
			},
			result: true,
			err:    nil,
			out: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
			txnEmpty: false,
		},
		{
			name: "no version object",
			in: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
			verObj: nil,
			result: true,
			err:    nil,
			out: monitoring.ArchiveRequest{
				TypeMeta: api.TypeMeta{
					Kind: string(monitoring.KindArchiveRequest),
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "testArchive",
					Tenant: globals.DefaultTenant,
				},
			},
			txnEmpty: true,
		},
	}

	logConfig := log.GetDefaultConfig("TestArchiveHooks")
	l := log.GetNewLogger(logConfig)
	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}

	archiveHooks := &archiveHooks{
		logger: l,
	}
	for _, test := range tests {
		ctx := context.TODO()
		txn := kvs.NewTxn()
		var reqKey string
		if test.verObj != nil {
			reqKey = test.verObj.MakeKey("cluster")
			if err := kvs.Create(ctx, reqKey, test.verObj); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with version object, Err: %v", test.name, err)
			}
		}
		out, ok, err := archiveHooks.archiveRequestUpgradeCheck(ctx, kvs, txn, reqKey, apiintf.CreateOper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected return object [%#v], got [%#v]", test.name, test.out, out))
		Assert(t, test.txnEmpty == txn.IsEmpty(), fmt.Sprintf("[%v] test failed, expected txn empty flag to be [%v], got [%v]", test.name, test.txnEmpty, txn.IsEmpty()))
		kvs.Delete(ctx, reqKey, nil)
	}
}
