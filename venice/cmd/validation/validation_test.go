package validation

import (
	"testing"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
)

func testCluster(name string, spec *cmd.ClusterSpec, status *cmd.ClusterStatus) *cmd.Cluster {
	return &cmd.Cluster{
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
		cluster           *cmd.Cluster
	}{
		// quorum nodes cannot be empty in the request
		"bad-cluster-no-nodes": {
			isExpectedFailure: true,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{},
					VirtualIP:   "192.168.30.10",
				},
				&cmd.ClusterStatus{},
			),
		},
		"good-cluster": {
			isExpectedFailure: false,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{"node1", "node2", "node3"},
					VirtualIP:   "192.168.30.10",
				},
				&cmd.ClusterStatus{},
			),
		},
		"bad-cluster-wo-virtual-ip": {
			isExpectedFailure: true,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{"node1", "node2", "node3"},
				},
				&cmd.ClusterStatus{},
			),
		},
		// status must be empty in the request object
		"bad-cluster-with-status": {
			isExpectedFailure: true,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{"node1", "node2", "node3"},
					VirtualIP:   "192.168.30.10",
				},
				&cmd.ClusterStatus{
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
