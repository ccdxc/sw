package validation

import (
	"net"
	"testing"

	"github.com/pensando/sw/api"
)

func testCluster(name string, spec *api.ClusterSpec, status *api.ClusterStatus) *api.Cluster {
	return &api.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Spec:   *spec,
		Status: *status,
	}
}

func TestValidateCluster(t *testing.T) {
	scenarios := map[string]struct {
		isExpectedFailure bool
		cluster           *api.Cluster
	}{
		"bad-cluster-no-nodes": {
			isExpectedFailure: true,
			cluster: testCluster("foo",
				&api.ClusterSpec{
					QuorumNodes: []string{},
					VirtualIP:   net.ParseIP("192.168.30.10"),
				},
				&api.ClusterStatus{},
			),
		},
		"good-cluster": {
			isExpectedFailure: false,
			cluster: testCluster("foo",
				&api.ClusterSpec{
					QuorumNodes: []string{"node1", "node2", "node3"},
					VirtualIP:   net.ParseIP("192.168.30.10"),
				},
				&api.ClusterStatus{},
			),
		},
		"bad-cluster-wo-virtual-ip": {
			isExpectedFailure: true,
			cluster: testCluster("foo",
				&api.ClusterSpec{
					QuorumNodes: []string{"node1", "node2", "node3"},
				},
				&api.ClusterStatus{},
			),
		},
		"bad-cluster-with-status": {
			isExpectedFailure: true,
			cluster: testCluster("foo",
				&api.ClusterSpec{
					QuorumNodes: []string{"node1", "node2", "node3"},
					VirtualIP:   net.ParseIP("192.168.30.10"),
				},
				&api.ClusterStatus{
					Leader: "node1",
				},
			),
		},
	}

	for name, scenario := range scenarios {
		errs := ValidateCluster(scenario.cluster)
		if len(errs) == 0 && scenario.isExpectedFailure {
			t.Errorf("Unexpected success for scenario: %s", name)
		}
		if len(errs) > 0 && !scenario.isExpectedFailure {
			t.Errorf("Unexpected failure for scenario: %s - %+v", name, errs)
		}
	}
}
