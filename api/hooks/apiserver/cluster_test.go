package impl

import (
	"context"
	"errors"
	"fmt"

	"io"
	"reflect"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/featureflags"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestNodeObject(t *testing.T) {
	cl := &clusterHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Node-Hooks-Test")),
	}

	// Testcases for various Node configs
	nodeTestcases := []struct {
		obj cluster.Node
		err []error
	}{
		// invalid tenant name
		{
			cluster.Node{
				ObjectMeta: api.ObjectMeta{
					Name:   "venice-node1.local",
					Tenant: "audi",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Node",
					APIVersion: "v1",
				},
			},
			[]error{
				cl.errInvalidTenantConfig(),
			},
		},
		// valid node object #1
		{
			cluster.Node{
				ObjectMeta: api.ObjectMeta{
					Name:   "10.1.1.2",
					Tenant: "",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Node",
					APIVersion: "v1",
				},
				Spec: cluster.NodeSpec{},
			},
			[]error{},
		},
		// valid node object #2
		{
			cluster.Node{
				ObjectMeta: api.ObjectMeta{
					Name: "venice-node1.local",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.NodeSpec{},
			},
			[]error{},
		},
	}

	// Execute the node config testcases
	for _, tc := range nodeTestcases {
		t.Run(tc.obj.Name, func(t *testing.T) {
			err := cl.validateNodeConfig(tc.obj, "", true, false)
			if len(err) != len(tc.err) {
				t.Errorf("Expected errors: [%d] actual errors: [%+v]", len(tc.err), len(err))
			}
			for i := 0; i < len(err); i++ {
				if tc.err[i].Error() != err[i].Error() {
					t.Errorf("[%s] error[%d] - expected: [%+v] actual: [%+v]", tc.obj.Name, i, tc.err[i], err[i])
				}
			}
		})
	}

	// Test Precommit Hook
	kvs := &mocks.FakeKvStore{}
	txn := &mocks.FakeTxn{}
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()

	rcfg := network.RoutingConfig{}
	kvs.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		inO := into.(*network.RoutingConfig)
		*inO = rcfg
		return nil
	}

	nd := cluster.Node{
		Spec: cluster.NodeSpec{
			RoutingConfig: "xyz",
		},
	}

	_, kvw, err := cl.nodePreCommitHook(ctx, kvs, txn, "/test/key1", apiintf.CreateOper, false, nd)
	AssertOk(t, err, "expecting to succeed")
	Assert(t, kvw, "expecging kvwrite to be true")
	Assert(t, len(txn.Cmps) == 1, "expecting one comparator on the treansaction [%v]", txn.Cmps)
}

func TestClusterObject(t *testing.T) {
	cl := &clusterHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Cluster-Hooks-Test")),
	}

	// Testcases for various Cluster configs
	clusterTestcases := []struct {
		obj cluster.Cluster
		err []error
	}{
		// invalid tenant name
		{
			cluster.Cluster{
				ObjectMeta: api.ObjectMeta{
					Name:   "TestCluster1",
					Tenant: "audi",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Cluster",
					APIVersion: "v1",
				},
			},
			[]error{
				cl.errInvalidTenantConfig(),
			},
		},
		// valid cluster object #1
		{
			cluster.Cluster{
				ObjectMeta: api.ObjectMeta{
					Name:   "TestCluster2",
					Tenant: "",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Cluster",
					APIVersion: "v1",
				},
				Spec: cluster.ClusterSpec{
					QuorumNodes:   []string{"node1", "node2", "node3"},
					NTPServers:    []string{"1.pool.ntp.org", "2.pool.ntp.org"},
					AutoAdmitDSCs: true,
				},
			},
			[]error{},
		},
		// valid cluster object #2
		{
			cluster.Cluster{
				ObjectMeta: api.ObjectMeta{
					Name: "TestCluster2",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Cluster",
					APIVersion: "v1",
				},
				Spec: cluster.ClusterSpec{
					QuorumNodes:   []string{"node1", "node2", "node3"},
					NTPServers:    []string{"1.pool.ntp.org", "2.pool.ntp.org"},
					AutoAdmitDSCs: true,
				},
			},
			[]error{},
		},
	}

	// Execute the node config testcases
	for _, tc := range clusterTestcases {
		t.Run(tc.obj.Name, func(t *testing.T) {
			err := cl.validateClusterConfig(tc.obj, "", true, false)
			if len(err) != len(tc.err) {
				t.Errorf("Expected errors: [%d] actual errors: [%+v]", len(tc.err), len(err))
			}
			for i := 0; i < len(err); i++ {
				if tc.err[i].Error() != err[i].Error() {
					t.Errorf("[%s] error[%d] - expected: [%+v] actual: [%+v]", tc.obj.Name, i, tc.err[i], err[i])
				}
			}
		})
	}
}

func TestTenantObject(t *testing.T) {
	cl := &clusterHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Cluster-Hooks-Test")),
	}

	// various tenant configs
	tenantObjs := []struct {
		obj cluster.Tenant
		err error
	}{
		// tenant name > 48 characters
		{
			cluster.Tenant{
				ObjectMeta: api.ObjectMeta{
					Name: "qvbtgbie3nzpk81bc4hgr4xfplzsygw0tnid7h95xgwpzmd2fjo202wzqm1z",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Tenant",
					APIVersion: "v1",
				},
			},
			errors.New("tenant name too long (max 48 chars)"),
		},
		// invalid tenant name
		{
			cluster.Tenant{
				ObjectMeta: api.ObjectMeta{
					Name: "TestCluster1",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Tenant",
					APIVersion: "v1",
				},
			},
			errors.New("tenant name does not meet naming requirements"),
		},
		// valid cluster object #1
		{
			cluster.Tenant{
				ObjectMeta: api.ObjectMeta{
					Name: "testcluster2",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Tenant",
					APIVersion: "v1",
				},
			},
			nil,
		},
		// valid cluster object #2
		{
			cluster.Tenant{
				ObjectMeta: api.ObjectMeta{
					Name: "95110",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Tenant",
					APIVersion: "v1",
				},
			},
			nil,
		},
	}

	// Execute the node config testcases
	for _, tc := range tenantObjs {
		t.Run(tc.obj.Name, func(t *testing.T) {
			err := cl.validateTenantConfig(tc.obj)
			Assert(t, (tc.err != nil && err.Error() == tc.err.Error()) || err == tc.err, "expected: %v, got: %v", tc.err, err)
		})
	}
}

func TestCreateDefaultAlertPolicy(t *testing.T) {
	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		out      interface{}
		txnEmpty bool
		result   bool
		err      bool
	}{
		{
			name: "invalid input object for create tenant",
			oper: apiintf.CreateOper,
			in: struct {
				Test string
			}{"testing"},
			out: struct {
				Test string
			}{"testing"},
			txnEmpty: true,
			result:   true,
			err:      true,
		},
		{
			name: "create default alert policy for tenant",
			oper: apiintf.CreateOper,
			in: cluster.Tenant{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testtenant",
					Name:   "testtenant",
				},
			},
			out: cluster.Tenant{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testtenant",
					Name:   "testtenant",
				},
			},
			txnEmpty: false,
			result:   true,
			err:      false,
		},
	}

	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	logConfig := log.GetDefaultConfig("TestClusterHooks")
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
	clusterHooks := &clusterHooks{
		logger: l,
	}
	for _, test := range tests {
		txn := kvs.NewTxn()
		out, ok, err := clusterHooks.createDefaultAlertPolicy(ctx, kvs, txn, "", test.oper, false, test.in)
		fmt.Println(err)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, test.err == (err != nil), fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
		Assert(t, test.txnEmpty == txn.IsEmpty(), fmt.Sprintf("[%v] test failed, expected txn empty to be [%v], got [%v]", test.name, test.txnEmpty, txn.IsEmpty()))
	}
}

func TestCreateDefaultRoles(t *testing.T) {
	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		out      interface{}
		txnEmpty bool
		result   bool
		err      bool
	}{
		{
			name: "invalid input object for create tenant",
			oper: apiintf.CreateOper,
			in: struct {
				Test string
			}{"testing"},
			out: struct {
				Test string
			}{"testing"},
			txnEmpty: true,
			result:   true,
			err:      true,
		},
		{
			name: "create admin role for tenant",
			oper: apiintf.CreateOper,
			in: cluster.Tenant{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testtenant",
					Name:   "testtenant",
				},
			},
			out: cluster.Tenant{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testtenant",
					Name:   "testtenant",
				},
			},
			txnEmpty: false,
			result:   true,
			err:      false,
		},
		{
			name: "invalid operation type for create tenant",
			oper: apiintf.DeleteOper,
			in: cluster.Tenant{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testtenant",
					Name:   "testtenant",
				},
			},
			out: cluster.Tenant{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testtenant",
					Name:   "testtenant",
				},
			},
			txnEmpty: true,
			result:   true,
			err:      true,
		},
	}

	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	logConfig := log.GetDefaultConfig("TestClusterHooks")
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
	clusterHooks := &clusterHooks{
		logger: l,
	}
	for _, test := range tests {
		txn := kvs.NewTxn()
		out, ok, err := clusterHooks.createDefaultRoles(ctx, kvs, txn, "", test.oper, false, test.in)
		fmt.Println(err)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, test.err == (err != nil), fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
		Assert(t, test.txnEmpty == txn.IsEmpty(), fmt.Sprintf("[%v] test failed, expected txn empty to be [%v], got [%v]", test.name, test.txnEmpty, txn.IsEmpty()))
	}
}

func TestDeleteDefaultRoles(t *testing.T) {
	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		out      interface{}
		txnEmpty bool
		result   bool
		err      bool
	}{
		{
			name: "invalid input object for delete tenant",
			oper: apiintf.DeleteOper,
			in: struct {
				Test string
			}{"testing"},
			out: struct {
				Test string
			}{"testing"},
			txnEmpty: true,
			result:   true,
			err:      true,
		},
		{
			name: "delete admin role for tenant",
			oper: apiintf.DeleteOper,
			in: cluster.Tenant{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testTenant",
				},
			},
			out: cluster.Tenant{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testTenant",
				},
			},
			txnEmpty: false,
			result:   true,
			err:      false,
		},
		{
			name: "invalid operation type for delete tenant",
			oper: apiintf.CreateOper,
			in: cluster.Tenant{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testTenant",
				},
			},
			out: cluster.Tenant{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testTenant",
				},
			},
			txnEmpty: true,
			result:   true,
			err:      true,
		},
	}

	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	logConfig := log.GetDefaultConfig("TestClusterHooks")
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
	// create tenant admin role
	adminRole := login.NewRole(globals.AdminRole, "testTenant", login.NewPermission(
		"testTenant",
		authz.ResourceGroupAll,
		authz.ResourceKindAll,
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_AllActions.String()))
	adminRoleKey := adminRole.MakeKey("auth")
	if err := kvs.Create(ctx, adminRoleKey, adminRole); err != nil {
		t.Fatalf("unable to populate kvstore with admin role, Err: %v", err)
	}
	clusterHooks := &clusterHooks{
		logger: l,
	}
	for _, test := range tests {
		txn := kvs.NewTxn()
		out, ok, err := clusterHooks.deleteDefaultRoles(ctx, kvs, txn, "", test.oper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, test.err == (err != nil), fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v], ", test.name, test.out, out))
		Assert(t, test.txnEmpty == txn.IsEmpty(), fmt.Sprintf("[%v] test failed, expected txn empty to be [%v], got [%v]", test.name, test.txnEmpty, txn.IsEmpty()))
	}
}

func TestCheckAuthBootstrapFlag(t *testing.T) {
	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		existing *cluster.Cluster
		out      interface{}
		result   bool
		err      error
	}{
		{
			name: "invalid input object for update cluster",
			oper: apiintf.UpdateOper,
			in: struct {
				Test string
			}{"testing"},
			out: struct {
				Test string
			}{"testing"},
			result: true,
			err:    fmt.Errorf("invalid input type"),
		},
		{
			name: "unset bootstrap flag of already bootstrapped cluster",
			oper: apiintf.UpdateOper,
			in: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: false,
				},
			},
			existing: &cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: true,
				},
			},
			out: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: true,
				},
			},
			result: true,
			err:    nil,
		},
		{
			name: "set bootstrap flag of un-bootstrapped cluster",
			oper: apiintf.UpdateOper,
			in: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: true,
				},
			},
			existing: &cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: false,
				},
			},
			out: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: true,
				},
			},
			result: true,
			err:    nil,
		},
		{
			name: "set bootstrap flag through create cluster",
			oper: apiintf.CreateOper,
			in: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: true,
				},
			},
			out: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: false,
				},
			},
			result: true,
			err:    nil,
		},
		{
			name: "invalid operation type for check bootstrap hook",
			oper: apiintf.DeleteOper,
			in: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: true,
				},
			},
			out: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: true,
				},
			},
			result: true,
			err:    fmt.Errorf("invalid input type"),
		},
	}

	logConfig := log.GetDefaultConfig("TestClusterHooks")
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
	cluster := &cluster.Cluster{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
	}
	clusterKey := cluster.MakeKey("cluster")
	clusterHooks := &clusterHooks{
		logger: l,
	}
	for _, test := range tests {
		ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
		defer cancelFunc()
		txn := kvs.NewTxn()
		kvs.Delete(ctx, clusterKey, nil)
		if test.existing != nil {
			if err := kvs.Create(ctx, clusterKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with cluster, Err: %v", test.name, err)
			}
		}
		out, ok, err := clusterHooks.checkAuthBootstrapFlag(ctx, kvs, txn, clusterKey, test.oper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed [%v/%v]", test.name, test.err, err))
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
	}
}

func TestSetAuthBootstrapFlag(t *testing.T) {
	tests := []struct {
		name     string
		in       interface{}
		existing *cluster.Cluster
		out      interface{}
		result   bool
		err      error
	}{
		{
			name: "invalid input object",
			in: struct {
				Test string
			}{"testing"},
			out: struct {
				Test string
			}{"testing"},
			result: false,
			err:    fmt.Errorf("invalid input type"),
		},
		{
			name: "set bootstrap flag of un-bootstrapped cluster",
			in:   cluster.ClusterAuthBootstrapRequest{},
			existing: &cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name:         "testCluster",
					GenerationID: "1",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: false,
				},
			},
			out: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name:            "testCluster",
					GenerationID:    "2",
					ResourceVersion: "2",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: true,
				},
			},
			result: false,
			err:    nil,
		},
	}

	logConfig := log.GetDefaultConfig("TestClusterHooks")
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
	cl := &cluster.Cluster{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
	}
	clusterKey := cl.MakeKey("cluster")
	clusterHooks := &clusterHooks{
		logger: l,
	}
	for _, test := range tests {
		ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
		defer cancelFunc()
		txn := kvs.NewTxn()
		kvs.Delete(ctx, clusterKey, nil)
		if test.existing != nil {
			if err := kvs.Create(ctx, clusterKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with cluster, Err: %v", test.name, err)
			}
		}
		out, ok, err := clusterHooks.setAuthBootstrapFlag(ctx, kvs, txn, clusterKey, "AuthBootstrapComplete", false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed", test.name))
		if test.err == nil {
			o1, o2 := out.(cluster.Cluster), test.out.(cluster.Cluster)
			o1.ModTime, o1.CreationTime = o2.ModTime, o2.CreationTime
			Assert(t, reflect.DeepEqual(o2, o1), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
		}
	}
}

func TestPopulateExistingTLSConfig(t *testing.T) {
	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		existing *cluster.Cluster
		out      interface{}
		result   bool
		err      error
	}{
		{
			name: "invalid input object for populate TLS config in cluster",
			oper: apiintf.UpdateOper,
			in: struct {
				Test string
			}{"testing"},
			out: struct {
				Test string
			}{"testing"},
			result: true,
			err:    fmt.Errorf("invalid input type"),
		},
		{
			name: "populate existing certs and key",
			oper: apiintf.UpdateOper,
			in: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
			},
			existing: &cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Spec: cluster.ClusterSpec{
					Certs: "testcert",
					Key:   "testkey",
				},
			},
			out: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
				Spec: cluster.ClusterSpec{
					Certs: "testcert",
					Key:   "testkey",
				},
			},
			result: true,
			err:    nil,
		},
		{
			name: "invalid operation type for populate TLS Config hook",
			oper: apiintf.CreateOper,
			in: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
			},
			out: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
			},
			result: true,
			err:    fmt.Errorf("invalid input type"),
		},
		{
			name: "missing cluster obj",
			oper: apiintf.UpdateOper,
			in: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
			},
			existing: nil,
			out: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name: "testCluster",
				},
			},
			result: true,
			err:    kvstore.NewKeyNotFoundError("/venice/config/cluster/cluster/Singleton", 0),
		},
	}

	logConfig := log.GetDefaultConfig("TestClusterHooks")
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
	cluster := &cluster.Cluster{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
	}
	clusterKey := cluster.MakeKey("cluster")
	clusterHooks := &clusterHooks{
		logger: l,
	}
	for _, test := range tests {
		ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
		defer cancelFunc()
		txn := kvs.NewTxn()
		kvs.Delete(ctx, clusterKey, nil)
		if test.existing != nil {
			// encrypt private key as it is stored as secret
			if err := test.existing.ApplyStorageTransformer(ctx, true); err != nil {
				t.Fatalf("[%s] test failed, error encrypting password, Err: %v", test.name, err)
			}

			if err := kvs.Create(ctx, clusterKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with cluster, Err: %v", test.name, err)
			}
		}
		out, ok, err := clusterHooks.populateExistingTLSConfig(ctx, kvs, txn, clusterKey, test.oper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
	}
}

func TestSetTLSConfig(t *testing.T) {
	pemkey := `
-----BEGIN EC PARAMETERS-----
BgUrgQQAIw==
-----END EC PARAMETERS-----
-----BEGIN EC PRIVATE KEY-----
MIHcAgEBBEIBw19j4zd8aEMsCqBsGfrLT93ywnovsOEmTGkHnNZxQ+9U3HZvYEZA
QMUobxlj891ioExvRwm7aY7r6Hjnb+lCkLqgBwYFK4EEACOhgYkDgYYABADqG0/0
cp2+HjmqafBSgYonsrGboMHkLfT2J7YdGKZCCyebJMoDf6JBZxwcOKJ9mFj6wUy/
x0bxRsNd/YdNH9uiQwBt7vHGUb1uyEniyoFPyoVQqn6mqdp2nY21OwkHcMQ6U6C1
Uqvhc8wvGrVwYLlrIcGNcnZxEglGXJXTFwxQWSMuQQ==
-----END EC PRIVATE KEY-----
`
	pemcert := `
-----BEGIN CERTIFICATE-----
MIIEyTCCArGgAwIBAgICEAEwDQYJKoZIhvcNAQELBQAwgcUxCzAJBgNVBAYTAlVT
MQswCQYDVQQIDAJDQTERMA8GA1UEBwwIU2FuIEpvc2UxHzAdBgNVBAoMFlBlbnNh
bmRvIFN5c3RlbXMsIEluYy4xLzAtBgNVBAsMJlBlbnNhbmRvIFN5c3RlbXMgQ2Vy
dGlmaWNhdGUgQXV0aG9yaXR5MSEwHwYDVQQDDBhQZW5zYW5kbyBTeXN0ZW1zIFJv
b3QgQ0ExITAfBgkqhkiG9w0BCQEWEnJvb3RjYUBwZW5zYW5kby5pbzAeFw0xNzA3
MDMxNjI0MTZaFw0yNzA3MDExNjI0MTZaMIGoMQswCQYDVQQGEwJVUzELMAkGA1UE
CAwCQ0ExHzAdBgNVBAoMFlBlbnNhbmRvIFN5c3RlbXMsIEluYy4xHzAdBgNVBAsM
FlBlbnNhbmRvIE1hbnVmYWN0dXJpbmcxKDAmBgNVBAMMH1BlbnNhbmRvIE1hbnVm
YWN0dXJpbmcgQ0EgKEVDQykxIDAeBgkqhkiG9w0BCQEWEW1mZ2NhQHBlbnNhbmRv
LmlvMIGbMBAGByqGSM49AgEGBSuBBAAjA4GGAAQA6htP9HKdvh45qmnwUoGKJ7Kx
m6DB5C309ie2HRimQgsnmyTKA3+iQWccHDiifZhY+sFMv8dG8UbDXf2HTR/bokMA
be7xxlG9bshJ4sqBT8qFUKp+pqnadp2NtTsJB3DEOlOgtVKr4XPMLxq1cGC5ayHB
jXJ2cRIJRlyV0xcMUFkjLkGjZjBkMB0GA1UdDgQWBBRConmZyY4hbH77zIi43hiR
oWs8hzAfBgNVHSMEGDAWgBQsUe4mAOz8wHrhlEoRonml+ZEXhTASBgNVHRMBAf8E
CDAGAQH/AgEAMA4GA1UdDwEB/wQEAwIBhjANBgkqhkiG9w0BAQsFAAOCAgEALvFq
ZT8vJiZVZSCFVH/h6jos4wLc5okfSBiJMgr74eqzqRM6BL8scr+YZuUx12Qq164t
zDRereajvlf/A4AWeLgeazUYcLTRe97iQ+bhHbKZgv4Bh0Avpr+gNurEgG6ZzU4p
R6zva9XuyQhi3f1shcAamSCCnAPUujMaNmqXxNC238JnM6zeMfEOZtyLrEFQeMwB
jmT254ufpLJRV5cfTk3l4FIfpg75JVAR+A5c1VYlKHEUsQJu2OT9EsxPBJ8YnBsG
JrnI647gViLUqjDB1fmJ/TYyvZ4YvXuuhLcl5srn4apMwWMMHuN5HLML9JTfJAKJ
y/+0CHaeinAxgKZ4r+KOjW4bR6IlGgozR6azJLq9imN/aWGyL1f4YqgQ0LVa4w0t
JwvgZy2CheVxdOAPn7UCXoU1GgbtoeAKPqpjpY+gp0qcW1xYph/2EIq9DQmp+V0B
KsOZR+BfI9Kuef+FfKuwSX6LZ/rFofIH27jcEsakHt8cvg7oeyrNhgI5PnNrarnX
UrTdh0MjOrAD8RM+oIwHusO3b16kbojjoXf0pYU1M6ZGzAJTSyj+XWe2a9pPH5Kg
50YvoytRGiwjWcmMBouviBEc3FEWTQfaBt3zvDEHo+5myYsTaaZQ4rHA3NPjhXhH
jkyfA7bgnrfYqr+pv2Y+319JpMCr6t+e+vLafbU=
-----END CERTIFICATE-----
`
	tests := []struct {
		name     string
		in       interface{}
		existing *cluster.Cluster
		out      interface{}
		result   bool
		err      error
	}{
		{
			name: "invalid input object",
			in: struct {
				Test string
			}{"testing"},
			out:    nil,
			result: false,
			err:    fmt.Errorf("invalid input type"),
		},
		{
			name: "invalid certs",
			in: cluster.UpdateTLSConfigRequest{
				Certs: "invalidcert",
				Key:   "invalidkey",
			},
			existing: &cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name:         "testCluster",
					GenerationID: "1",
				},
			},
			out:    nil,
			result: false,
			err:    errors.New("tls: failed to find any PEM data in certificate input"),
		},
		{
			name: "valid TLS config update request",
			in: cluster.UpdateTLSConfigRequest{
				Certs: pemcert,
				Key:   pemkey,
			},
			existing: &cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name:         "testCluster",
					GenerationID: "1",
				},
			},
			out: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name:            "testCluster",
					GenerationID:    "2",
					ResourceVersion: "2",
				},
				Spec: cluster.ClusterSpec{
					Certs: pemcert,
				},
			},
			result: false,
			err:    nil,
		},
	}

	logConfig := log.GetDefaultConfig("TestClusterHooks")
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
	cl := &cluster.Cluster{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
	}
	clusterKey := cl.MakeKey("cluster")
	clusterHooks := &clusterHooks{
		logger: l,
	}
	for _, test := range tests {
		ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
		defer cancelFunc()
		txn := kvs.NewTxn()
		kvs.Delete(ctx, clusterKey, nil)
		if test.existing != nil {
			if err := kvs.Create(ctx, clusterKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with cluster, Err: %v", test.name, err)
			}
		}
		out, ok, err := clusterHooks.setTLSConfig(ctx, kvs, txn, clusterKey, "UpdateTLSConfig", false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed", test.name))
		if test.err == nil {
			o1, o2 := out.(cluster.Cluster), test.out.(cluster.Cluster)
			o1.ModTime, o1.CreationTime = o2.ModTime, o2.CreationTime
			Assert(t, reflect.DeepEqual(o2, o1), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
		}
	}
}

// TestDeafaultFirewallPolicy tests create and delete of default Firewall Policy
func TestDefaultFirewallProfile(t *testing.T) {
	// Test with wrong object
	// error from txn
	// Good one.
	txn := &mocks.FakeTxn{}
	kvs := &mocks.FakeKvStore{}
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	clusterHooks := &clusterHooks{
		logger: l,
	}
	_, _, err := clusterHooks.createFirewallProfile(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, txn)
	Assert(t, err != nil, "should have failed due to wrong object")
	tenant := cluster.Tenant{}
	tenant.Name = "default"
	txn.Error = fmt.Errorf("some error")
	_, _, err = clusterHooks.createFirewallProfile(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, tenant)
	Assert(t, err != nil, "should have failed due to txn failure")
	txn.Ops = nil
	txn.Error = nil
	_, kvn, err := clusterHooks.createFirewallProfile(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, tenant)
	AssertOk(t, err, "createFireWallProfile pre-commit hook")
	Assert(t, kvn, "unexpected kvwrite returned")
	Assert(t, len(txn.Ops) == 1, "unexpected number of entries in transaction (%v)", len(txn.Ops))
	Assert(t, txn.Ops[0].Op == "create", "unexpected operation in txn (%s)", txn.Ops[0].Op)

	txn.Ops = nil
	_, _, err = clusterHooks.deleteFirewallProfile(ctx, kvs, txn, "/test/key", apiintf.DeleteOper, false, txn)
	Assert(t, err != nil, "should have failed due to wrong object")
	txn.Error = fmt.Errorf("some error")
	_, _, err = clusterHooks.deleteFirewallProfile(ctx, kvs, txn, "/test/key", apiintf.DeleteOper, false, tenant)
	Assert(t, err != nil, "should have failed due to txn failure")
	txn.Ops = nil
	txn.Error = nil
	_, kvn, err = clusterHooks.deleteFirewallProfile(ctx, kvs, txn, "/test/key", apiintf.DeleteOper, false, tenant)
	AssertOk(t, err, "createFireWallProfile pre-commit hook")
	Assert(t, kvn, "unexpected kvwrite returned")
	Assert(t, len(txn.Ops) == 1, "unexpected number of entries in transaction (%v)", len(txn.Ops))
	Assert(t, txn.Ops[0].Op == "delete", "unexpected operation in txn (%s)", txn.Ops[0].Op)
}

// TestDeafaultFirewallPolicy tests create and delete of default Firewall Policy
func TestDefaultVirtualRouter(t *testing.T) {
	// Test with wrong object
	// error from txn
	// Good one.
	txn := &mocks.FakeTxn{}
	kvs := &mocks.FakeKvStore{}
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	clusterHooks := &clusterHooks{
		logger: l,
	}
	_, _, err := clusterHooks.createDefaultVirtualRouter(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, txn)
	Assert(t, err != nil, "should have failed due to wrong object")
	tenant := cluster.Tenant{}
	tenant.Name = "default"
	txn.Error = fmt.Errorf("some error")
	_, _, err = clusterHooks.createDefaultVirtualRouter(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, tenant)
	Assert(t, err != nil, "should have failed due to txn failure")
	txn.Ops = nil
	txn.Error = nil
	_, kvn, err := clusterHooks.createDefaultVirtualRouter(ctx, kvs, txn, "/test/key", apiintf.CreateOper, false, tenant)
	AssertOk(t, err, "createFireWallProfile pre-commit hook")
	Assert(t, kvn, "unexpected kvwrite returned")
	Assert(t, len(txn.Ops) == 1, "unexpected number of entries in transaction (%v)", len(txn.Ops))
	Assert(t, txn.Ops[0].Op == "create", "unexpected operation in txn (%s)", txn.Ops[0].Op)

	txn.Ops = nil
	_, _, err = clusterHooks.deleteDefaultVirtualRouter(ctx, kvs, txn, "/test/key", apiintf.DeleteOper, false, txn)
	Assert(t, err != nil, "should have failed due to wrong object")
	txn.Error = fmt.Errorf("some error")
	_, _, err = clusterHooks.deleteDefaultVirtualRouter(ctx, kvs, txn, "/test/key", apiintf.DeleteOper, false, tenant)
	Assert(t, err != nil, "should have failed due to txn failure")
	txn.Ops = nil
	txn.Error = nil
	_, kvn, err = clusterHooks.deleteDefaultVirtualRouter(ctx, kvs, txn, "/test/key", apiintf.DeleteOper, false, tenant)
	AssertOk(t, err, "createFireWallProfile pre-commit hook")
	Assert(t, kvn, "unexpected kvwrite returned")
	Assert(t, len(txn.Ops) == 1, "unexpected number of entries in transaction (%v)", len(txn.Ops))
	Assert(t, txn.Ops[0].Op == "delete", "unexpected operation in txn (%s)", txn.Ops[0].Op)
}

func TestValidateTenant(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	clusterHooks := &clusterHooks{
		logger: l,
	}
	a := apisrvpkg.MustGetAPIServer()
	config := apiserver.Config{
		GrpcServerPort: ":0",
		DebugMode:      true,
		Logger:         l,
		Version:        "v1",
		Scheme:         runtime.NewScheme(),
		Kvstore: store.Config{
			Type:  store.KVStoreTypeMemkv,
			Codec: runtime.NewJSONCodec(runtime.NewScheme()),
		},
		KVPoolSize:       1,
		AllowMultiTenant: true,
	}
	_ = recorder.Override(mockevtsrecorder.NewRecorder("apigw_test", l))
	go a.Run(config)
	a.WaitRunning()

	tenant1 := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
		ObjectMeta: api.ObjectMeta{
			Name: "testtenant",
		},
	}
	tenant2 := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
		ObjectMeta: api.ObjectMeta{
			Name: "default",
		},
	}
	errs := clusterHooks.validateTenant(tenant1, "v1", false, false)
	if errs != nil {
		t.Errorf("Expecting validate to succeed (%s)", errs)
	}
	errs = clusterHooks.validateTenant(tenant2, "v1", false, false)
	if errs != nil {
		t.Errorf("Expecting validate to succeed (%s)", errs)
	}
	a.Stop()
	config.AllowMultiTenant = false
	go a.Run(config)
	a.WaitRunning()
	errs = clusterHooks.validateTenant(tenant1, "v1", false, false)
	if errs == nil || len(errs) == 0 {
		t.Errorf("Expecting validate to fail")
	}
	errs = clusterHooks.validateTenant(tenant2, "v1", false, false)
	if errs != nil {
		t.Errorf("Expecting validate to succeed (%s)", errs)
	}
	a.Stop()
}

type fakeOclient struct {
	written, read int64
	retErr        error
	puts          int
}

// PutObject is a mock client implementation
func (m *fakeOclient) PutObject(ctx context.Context, objectName string, reader io.Reader, metaData map[string]string) (int64, error) {
	m.puts++
	return m.written, m.retErr

}

// PutObjectOfSize is a mock client implementation
func (m *fakeOclient) PutObjectOfSize(ctx context.Context, objectName string, reader io.Reader, size int64, metaData map[string]string) (int64, error) {
	return m.written, m.retErr
}

// PutStreamObject is a mock client implementation
func (m *fakeOclient) PutStreamObject(ctx context.Context, objectName string, metaData map[string]string) (io.WriteCloser, error) {
	return m, m.retErr
}

// GetObjectis a mock client implementation
func (m *fakeOclient) GetObject(ctx context.Context, objectName string) (io.ReadCloser, error) {
	return m, m.retErr
}

// GetStreamObjectAtOffset is a mock client implementation
func (m *fakeOclient) GetStreamObjectAtOffset(ctx context.Context, objectName string, offset int) (io.ReadCloser, error) {
	return m, nil
}

// StatObject is a mock client implementation
func (m *fakeOclient) StatObject(objectName string) (*objstore.ObjectStats, error) {
	return nil, nil
}

// ListObjects is a mock client implementation
func (m *fakeOclient) ListObjects(prefix string) ([]string, error) {
	return nil, nil
}

// RemoveObjects is a mock client implementation
func (m *fakeOclient) RemoveObjects(prefix string) error {
	return nil
}

// RemoveObject is a mock client implementation
func (m *fakeOclient) RemoveObject(path string) error {
	return nil
}

// Read is a mock implementation
func (m *fakeOclient) Read(p []byte) (n int, err error) {
	return int(m.read), nil
}

// Close is a mock implementation
func (m *fakeOclient) Close() error {
	return nil
}

// Write is a mock implementation
func (m *fakeOclient) Write(p []byte) (n int, err error) {
	return int(m.written), nil
}

func TestPerformSnapShot(t *testing.T) {
	txn := &mocks.FakeTxn{}
	kvs := &mocks.FakeKvStore{}
	fcache := mocks.FakeCache{}
	apisrvpkg.SetAPIServerCache(&fcache)
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	clusterHooks := &clusterHooks{
		logger: l,
	}
	oclnt := &fakeOclient{}
	ClusterHooksObjStoreClient = oclnt
	scfg := cluster.ConfigurationSnapshot{
		Spec: cluster.ConfigurationSnapshotSpec{
			Destination: cluster.SnapshotDestination{
				Type: cluster.SnapshotDestinationType_ObjectStore.String(),
			},
		},
	}
	var retErr error
	getfn := func(ctx context.Context, key string, into runtime.Object) error {
		sin := into.(*cluster.ConfigurationSnapshot)
		*sin = scfg
		return retErr
	}
	kvs.Getfn = getfn

	var rev uint64
	snapfn := func() uint64 {
		return rev
	}
	rev = 10
	fcache.StartSnapshotFn = snapfn
	fcache.RetSnapshotReader = oclnt
	req := cluster.ConfigurationSnapshotRequest{}
	req.Name = "TestRequest"
	clusterHooks.performSnapshotNow(ctx, kvs, txn, "/test/object", apiintf.CreateOper, false, req)
	Assert(t, oclnt.puts == 1, "expecting 1 put got %d", oclnt.puts)
}

type fakeSnapshotWriter struct {
	calls int
	wrErr error
}

func (f *fakeSnapshotWriter) Write(ctx context.Context, kvs kvstore.Interface) error {
	return f.wrErr
}

func TestPerformRestore(t *testing.T) {
	txn := &mocks.FakeTxn{}
	kvs := &mocks.FakeKvStore{}
	fov := &mocks.FakeOverlay{}
	fcache := mocks.FakeCache{}
	fov.Interface = &fcache
	apisrvpkg.SetAPIServerCache(&fcache)
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	clusterHooks := &clusterHooks{
		logger: l,
	}
	oclnt := &fakeOclient{}
	ClusterHooksObjStoreClient = oclnt

	scfg := cluster.ConfigurationSnapshot{
		Spec: cluster.ConfigurationSnapshotSpec{
			Destination: cluster.SnapshotDestination{
				Type: cluster.SnapshotDestinationType_ObjectStore.String(),
			},
		},
	}
	var retrest *cluster.SnapshotRestore
	var retErr error
	verObj := cluster.Version{
		Status: cluster.VersionStatus{
			RolloutBuildVersion: "TestVersion1.0",
		},
	}
	getfn := func(ctx context.Context, key string, into runtime.Object) error {
		switch into.(type) {
		case *cluster.SnapshotRestore:
			if retrest == nil {
				return errors.New("not found")
			}
			sin := into.(*cluster.SnapshotRestore)
			*sin = *retrest
		case *cluster.ConfigurationSnapshot:
			sin := into.(*cluster.ConfigurationSnapshot)
			*sin = scfg
		case *cluster.Version:
			sin := into.(*cluster.Version)
			*sin = verObj
		}
		return retErr
	}
	kvs.Getfn = getfn
	var rev uint64
	snapfn := func() uint64 {
		return rev
	}
	statKFn := func(group string, kind string) ([]apiintf.ObjectStat, error) {
		return []apiintf.ObjectStat{}, nil
	}
	fcache.StatKindFn = statKFn
	rev = 10
	fcache.StartSnapshotFn = snapfn
	fcache.RetSnapshotReader = oclnt
	swr := &fakeSnapshotWriter{}
	fcache.SnapWriter = swr
	req := cluster.SnapshotRestore{
		Spec: cluster.SnapshotRestoreSpec{
			SnapshotPath: "testSnapshotFile",
		},
	}

	// Perform restore while rollout is in progress
	_, _, err := clusterHooks.performRestoreNow(ctx, kvs, txn, "/test/object", apiintf.CreateOper, false, req)
	Assert(t, err != nil, "Operation should fail")

	// Reset rollout
	verObj.Status.RolloutBuildVersion = ""

	_, write, err := clusterHooks.performRestoreNow(ctx, kvs, txn, "/test/object", apiintf.CreateOper, false, req)
	Assert(t, !write, "expecting kvwrite to be false")
	AssertOk(t, err, "Operation should succeed")

	// set failure on getting snapshot object
	oclnt.retErr = errors.New("not found")
	_, write, err = clusterHooks.performRestoreNow(ctx, kvs, txn, "/test/object", apiintf.CreateOper, false, req)
	Assert(t, !write, "expecting kvwrite to be false")
	Assert(t, err != nil, "Operation should fail")

	// Set restore in progress
	oclnt.retErr = nil
	ret := cluster.SnapshotRestore{
		Status: cluster.SnapshotRestoreStatus{
			Status: cluster.SnapshotRestoreStatus_Active.String(),
		},
	}
	retrest = &ret
	_, write, err = clusterHooks.performRestoreNow(ctx, kvs, txn, "/test/object", apiintf.CreateOper, false, req)
	Assert(t, !write, "expecting kvwrite to be false")
	Assert(t, err == nil, "Operation should succeed")

	// Test Rollback
	Assert(t, fcache.RollbackCalls == 0, "expecting rollbacks to be 0")
	retrest = nil
	txnError := errors.New("failed")
	txn.Commitfn = func(ctx context.Context) (kvstore.TxnResponse, error) {
		return kvstore.TxnResponse{}, txnError
	}
	_, write, err = clusterHooks.performRestoreNow(ctx, kvs, txn, "/test/object", apiintf.CreateOper, false, req)
	Assert(t, !write, "expecting kvwrite to be false")
	Assert(t, err != nil, "Operation should Fail")
	Assert(t, fcache.RollbackCalls == 1, "expecting 1 rollback call got %d", fcache.RollbackCalls)

}

func TestGetResposeWriters(t *testing.T) {
	kvs := &mocks.FakeKvStore{}
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	clusterHooks := &clusterHooks{
		logger: l,
	}

	clObj := cluster.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
		Spec: cluster.ClusterSpec{
			VirtualIP: "10.1.1.1",
		},
	}

	resObj := cluster.SnapshotRestore{
		ObjectMeta: api.ObjectMeta{
			Name: "testRestore",
		},
		Spec: cluster.SnapshotRestoreSpec{
			SnapshotPath: "/xyz",
		},
	}

	snapObj := cluster.ConfigurationSnapshot{
		ObjectMeta: api.ObjectMeta{
			Name: "testSnapshot",
		},
		Spec: cluster.ConfigurationSnapshotSpec{
			Destination: cluster.SnapshotDestination{
				Type: cluster.SnapshotDestinationType_ObjectStore.String(),
			},
		},
	}

	var retErr error
	getfn := func(ctx context.Context, key string, into runtime.Object) error {
		switch into.(type) {
		case *cluster.SnapshotRestore:
			sin := into.(*cluster.SnapshotRestore)
			*sin = resObj
		case *cluster.ConfigurationSnapshot:
			sin := into.(*cluster.ConfigurationSnapshot)
			*sin = snapObj
		case *cluster.Cluster:
			sin := into.(*cluster.Cluster)
			*sin = clObj
		}

		return retErr
	}
	kvs.Getfn = getfn

	cl := cluster.Cluster{}
	rt := cluster.SnapshotRestore{}
	sn := cluster.ConfigurationSnapshot{}
	ret, err := clusterHooks.getClusterObject(ctx, kvs, "test", cl, cl, cl, apiintf.CreateOper)
	AssertOk(t, err, "get should have succeeded")
	Assert(t, reflect.DeepEqual(ret, clObj), "Objects do not match[%+v]/[%+v]", ret, clObj)

	ret, err = clusterHooks.getRestoreObject(ctx, kvs, "test", rt, rt, rt, apiintf.CreateOper)
	AssertOk(t, err, "get should have succeeded")
	Assert(t, reflect.DeepEqual(ret, resObj), "Objects do not match[%+v]/[%+v]", ret, resObj)

	ret, err = clusterHooks.getSnapshotObject(ctx, kvs, "test", sn, sn, sn, apiintf.CreateOper)
	AssertOk(t, err, "get should have succeeded")
	Assert(t, reflect.DeepEqual(ret, snapObj), "Objects do not match[%+v]/[%+v]", ret, snapObj)

}

func TestFeatureFlagsHooks(t *testing.T) {
	kvs := &mocks.FakeKvStore{}
	txn := &mocks.FakeTxn{}
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	ch := &clusterHooks{
		logger: l,
	}

	ff := cluster.License{
		Spec: cluster.LicenseSpec{
			Features: []cluster.Feature{
				{FeatureKey: featureflags.OverlayRouting},
			},
		},
	}

	errs := ch.validateFFBootstrap(ff, "v1", false, false)
	Assert(t, len(errs) == 0, "expecting no errors")

	ri, kvwrite, err := ch.checkFFBootstrap(ctx, kvs, txn, "", apiintf.CreateOper, false, ff)
	AssertOk(t, err, "not expecting errors")
	Assert(t, kvwrite, "expecting kvwrite to be true")
	fstatus := ri.(cluster.License)
	found := false
	for _, v := range fstatus.Status.Features {
		if v.FeatureKey == featureflags.OverlayRouting {
			Assert(t, v.Value == "enabled", "did not get enabled for routing")
			found = true
		}
	}
	Assert(t, found, "did not find feature in status")
	ch.applyFeatureFlags(ctx, apiintf.CreateOper, fstatus, false)
	Assert(t, featureflags.IsOVerlayRoutingEnabled() == true, "expecting overlay routing to be true")
	// With Errors
	ff = cluster.License{
		Spec: cluster.LicenseSpec{
			Features: []cluster.Feature{
				{FeatureKey: featureflags.OverlayRouting},
				{FeatureKey: featureflags.OverlayRouting},
			},
		},
	}
	errs = ch.validateFFBootstrap(ff, "v1", false, false)
	Assert(t, len(errs) == 1, "expecting errors")

	ri, kvwrite, err = ch.checkFFBootstrap(ctx, kvs, txn, "", apiintf.CreateOper, false, ff)
	Assert(t, err != nil, "expecting errors")

	// With Errors
	ff = cluster.License{
		Spec: cluster.LicenseSpec{
			Features: []cluster.Feature{
				{FeatureKey: featureflags.SubnetSecurityPolicies},
			},
		},
	}

	ri, kvwrite, err = ch.checkFFBootstrap(ctx, kvs, txn, "", apiintf.CreateOper, false, ff)
	AssertOk(t, err, "not expecting errors")

	ff = ri.(cluster.License)
	ch.applyFeatureFlags(ctx, apiintf.CreateOper, ff, false)
	Assert(t, featureflags.IsOVerlayRoutingEnabled() == false, "expecing overlay routing to be false")

	ff = cluster.License{
		Spec: cluster.LicenseSpec{
			Features: []cluster.Feature{
				{FeatureKey: featureflags.OverlayRouting},
			},
		},
	}
	kvs.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		inO := into.(*cluster.License)
		*inO = ff
		return nil
	}

	ch.restoreFeatureFlags(kvs, l)
	Assert(t, featureflags.IsOVerlayRoutingEnabled() == true, "expecing overlay routing to be true")
}

func TestRouteTableHooks(t *testing.T) {
	txn := &mocks.FakeTxn{}
	kvs := &mocks.FakeKvStore{}
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	logConfig := log.GetDefaultConfig("TestClusterHooks")
	l := log.GetNewLogger(logConfig)
	clusterHooks := &clusterHooks{
		logger: l,
	}

	tn := cluster.Tenant{}

	_, kvw, err := clusterHooks.createDefaultRouteTable(ctx, kvs, txn, "/test/key1", apiintf.CreateOper, false, tn)
	AssertOk(t, err, "expecting to succeed")
	Assert(t, kvw, "expecting kvwrite to be true")
	Assert(t, len(txn.Ops) == 1, "expecting 1 txn operation to be added [%v]", txn.Ops)
	txn.Ops = nil
	txn.Cmps = nil

	_, kvw, err = clusterHooks.deleteDefaultRouteTable(ctx, kvs, txn, "/test/key1", apiintf.CreateOper, false, tn)
	AssertOk(t, err, "expecting to succeed")
	Assert(t, kvw, "expecting kvwrite to be true")
	Assert(t, len(txn.Ops) == 1, "expecting 1 txn operation to be added [%v]", txn.Ops)
}
