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
		"good-cluster-with-dns-names": {
			isExpectedFailure: false,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{"node1", "node2", "node3"},
					VirtualIP:   "192.168.30.10",
				},
				&cmd.ClusterStatus{},
			),
		},
		"good-cluster-with-fqdns": {
			isExpectedFailure: false,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{"node1.pensando.io", "node2.pensando.io", "node3.pensando.io"},
					VirtualIP:   "192.168.30.10",
				},
				&cmd.ClusterStatus{},
			),
		},
		"good-cluster-with-ip-addrs": {
			isExpectedFailure: false,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{"192.168.30.11", "192.168.30.12", "192.168.30.13"},
					VirtualIP:   "192.168.30.10",
				},
				&cmd.ClusterStatus{},
			),
		},
		"good-cluster-with-mixed-ids": {
			isExpectedFailure: false,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{"node1", "192.168.30.12", "node3.pensando.io"},
					VirtualIP:   "192.168.30.10",
				},
				&cmd.ClusterStatus{},
			),
		},
		"good-cluster-with-empty-quorum-name": {
			isExpectedFailure: true,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{"node1", "", "node3"},
					VirtualIP:   "192.168.30.10",
				},
				&cmd.ClusterStatus{},
			),
		},
		"bad-cluster-with-invalid-quorum-name": {
			isExpectedFailure: true,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{"node1", "node2_2", "node3"},
					VirtualIP:   "192.168.30.10",
				},
				&cmd.ClusterStatus{},
			),
		},
		"bad-cluster-with-localhost-ip-address": {
			isExpectedFailure: true,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{"192.168.30.11", "192.168.30.12", "127.0.0.1"},
					VirtualIP:   "192.168.30.10",
				},
				&cmd.ClusterStatus{},
			),
		},
		"bad-cluster-with-broadcast-ip-address": {
			isExpectedFailure: true,
			cluster: testCluster("foo",
				&cmd.ClusterSpec{
					QuorumNodes: []string{"255.255.255.255", "192.168.30.12", "102.168.30.13"},
					VirtualIP:   "192.168.30.10",
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
