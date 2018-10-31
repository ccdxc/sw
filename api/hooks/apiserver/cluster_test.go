package impl

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestHostObject(t *testing.T) {
	cl := &clusterHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Host-Hooks-Test")),
	}

	// Testcases for various host configs
	hostTestcases := []struct {
		obj cluster.Host
		err []error
	}{
		// invalid host name
		{
			cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: ".naples1-host.local",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
			},
			[]error{
				cl.errInvalidHostConfig(".naples1-host.local"),
			},
		},
		// invalid mac addr #1
		{
			cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "naples2-host.local",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					Interfaces: map[string]cluster.HostIntfSpec{
						"hello-world": cluster.HostIntfSpec{},
					},
				},
			},
			[]error{
				cl.errInvalidMacConfig("hello-world"),
			},
		},
		// invalid mac addr #2
		{
			cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "10.5.5.5",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					Interfaces: map[string]cluster.HostIntfSpec{
						"01.02.03.04.05.06": cluster.HostIntfSpec{
							MacAddrs: []string{"0102.0304.05.06"},
						},
					},
				},
			},
			[]error{
				cl.errInvalidMacConfig("01.02.03.04.05.06"),
				cl.errInvalidMacConfig("0102.0304.05.06"),
			},
		},
		// valid host object #1
		{
			cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "naples3-host.local",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					Interfaces: map[string]cluster.HostIntfSpec{
						"0102.0304.0506": cluster.HostIntfSpec{
							MacAddrs: []string{"0102.0304.0506"},
						},
					},
				},
			},
			[]error{},
		},
		// valid host object #2
		{
			cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "20.5.5.5",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					Interfaces: map[string]cluster.HostIntfSpec{
						"01-02-03-04-05-06": cluster.HostIntfSpec{
							MacAddrs: []string{"0102.0304.0506"},
						},
					},
				},
			},
			[]error{},
		},
		// invalid tenant name
		{
			cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name:   "naples3-host.local",
					Tenant: "audi",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
			},
			[]error{
				cl.errInvalidTenantConfig(),
			},
		},
	}

	// Execute the host config testcases
	for _, tc := range hostTestcases {
		t.Run(tc.obj.Name, func(t *testing.T) {
			err := cl.validateHostConfig(tc.obj, "", true)
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
			err := cl.validateNodeConfig(tc.obj, "", true)
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
					AutoAdmitNICs: true,
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
					AutoAdmitNICs: true,
				},
			},
			[]error{},
		},
	}

	// Execute the node config testcases
	for _, tc := range clusterTestcases {
		t.Run(tc.obj.Name, func(t *testing.T) {
			err := cl.validateClusterConfig(tc.obj, "", true)
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

func TestCreateDefaultRoles(t *testing.T) {
	tests := []struct {
		name     string
		oper     apiserver.APIOperType
		in       interface{}
		out      interface{}
		txnEmpty bool
		result   bool
		err      bool
	}{
		{
			name: "invalid input object for create tenant",
			oper: apiserver.CreateOper,
			in: struct {
				Test string
			}{"testing"},
			out: struct {
				Test string
			}{"testing"},
			txnEmpty: true,
			result:   false,
			err:      true,
		},
		{
			name: "create admin role for tenant",
			oper: apiserver.CreateOper,
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
			name: "invalid operation type for create tenant",
			oper: apiserver.DeleteOper,
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
			result:   false,
			err:      true,
		},
	}

	ctx := context.TODO()
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
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, test.err == (err != nil), fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
		Assert(t, test.txnEmpty == txn.IsEmpty(), fmt.Sprintf("[%v] test failed, expected txn empty to be [%v], got [%v]", test.name, test.txnEmpty, txn.IsEmpty()))
	}
}

func TestDeleteDefaultRoles(t *testing.T) {
	tests := []struct {
		name     string
		oper     apiserver.APIOperType
		in       interface{}
		out      interface{}
		txnEmpty bool
		result   bool
		err      bool
	}{
		{
			name: "invalid input object for delete tenant",
			oper: apiserver.DeleteOper,
			in: struct {
				Test string
			}{"testing"},
			out: struct {
				Test string
			}{"testing"},
			txnEmpty: true,
			result:   false,
			err:      true,
		},
		{
			name: "delete admin role for tenant",
			oper: apiserver.DeleteOper,
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
			oper: apiserver.CreateOper,
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
			result:   false,
			err:      true,
		},
	}

	ctx := context.TODO()
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
		oper     apiserver.APIOperType
		in       interface{}
		existing *cluster.Cluster
		out      interface{}
		result   bool
		err      error
	}{
		{
			name: "invalid input object for update cluster",
			oper: apiserver.UpdateOper,
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
			name: "unset bootstrap flag of already bootstrapped cluster",
			oper: apiserver.UpdateOper,
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
			oper: apiserver.UpdateOper,
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
			oper: apiserver.CreateOper,
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
			oper: apiserver.DeleteOper,
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
			result: false,
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
		ctx := context.TODO()
		txn := kvs.NewTxn()
		kvs.Delete(ctx, clusterKey, nil)
		if test.existing != nil {
			if err := kvs.Create(ctx, clusterKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with cluster, Err: %v", test.name, err)
			}
		}
		out, ok, err := clusterHooks.checkAuthBootstrapFlag(ctx, kvs, txn, clusterKey, test.oper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed", test.name))
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
					Name: "testCluster",
				},
				Status: cluster.ClusterStatus{
					AuthBootstrapped: false,
				},
			},
			out: cluster.Cluster{
				TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
				ObjectMeta: api.ObjectMeta{
					Name:            "testCluster",
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
		ctx := context.TODO()
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
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
	}
}
