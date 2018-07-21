package impl

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/log"
)

func TestHostObject(t *testing.T) {
	cl := &clHooks{
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
	cl := &clHooks{
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
	cl := &clHooks{
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
