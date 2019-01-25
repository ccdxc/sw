package impl

import (
	"context"
	"errors"
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
	"github.com/pensando/sw/venice/utils/kvstore"
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
					SmartNICs: []cluster.SmartNICID{
						{
							MACAddress: "hello-world",
						},
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
					SmartNICs: []cluster.SmartNICID{
						{
							MACAddress: "01.02.03.04.05.06",
						},
					},
				},
			},
			[]error{
				cl.errInvalidMacConfig("01.02.03.04.05.06"),
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
					SmartNICs: []cluster.SmartNICID{
						{
							MACAddress: "0102.0304.0506",
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
					SmartNICs: []cluster.SmartNICID{
						{
							MACAddress: "01-02-03-04-05-06",
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
			oper: apiserver.DeleteOper,
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

func TestPopulateExistingTLSConfig(t *testing.T) {
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
			name: "invalid input object for populate TLS config in cluster",
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
			name: "populate existing certs and key",
			oper: apiserver.UpdateOper,
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
			oper: apiserver.CreateOper,
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
			result: false,
			err:    fmt.Errorf("invalid input type"),
		},
		{
			name: "missing cluster obj",
			oper: apiserver.UpdateOper,
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
			result: false,
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
		ctx := context.TODO()
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
		out, ok, err := clusterHooks.setTLSConfig(ctx, kvs, txn, clusterKey, "UpdateTLSConfig", false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
	}
}
