package services

import (
	"fmt"
	"testing"
	"time"

	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/server/options"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types"
	diagmock "github.com/pensando/sw/venice/utils/diagnostics/mock"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/testutils"
)

func setupMaster(t *testing.T) (*mock.LeaderService, types.SystemdService, *mock.CfgWatcherService, types.MasterService) {
	env.Options = &options.ServerRunOptions{} // use empty options so that we don't bind to fixed ports
	ClusterStatusUpdateInterval = time.Second
	l := mock.NewLeaderService(t.Name())
	s := NewSystemdService(WithSysIfSystemdSvcOption(&mock.SystemdIf{}))
	n := NewNtpService(nil, []string{t.Name()}, t.Name())
	cw := mock.CfgWatcherService{}
	m := NewMasterService(
		"testNode",
		WithLeaderSvcMasterOption(l),
		WithSystemdSvcMasterOption(s),
		WithConfigsMasterOption(&mock.Configs{}),
		WithCfgWatcherMasterOption(&cw),
		WithNtpSvcMasterOption(n),
		WithK8sSvcMasterOption(&mock.K8sService{}),
		WithResolverSvcMasterOption(mock.NewResolverService()),
		WithDiagModuleUpdaterSvcOption(diagmock.GetModuleUpdater()),
		WithClusterHealthMonitor(&mock.ClusterHealthMonitor{}))

	return l, s, &cw, m
}

func checkAllServiceStarted(t *testing.T, s types.SystemdService, serviceNames []string) {
	units := s.Units()
	startedUnitsMap := make(map[string]interface{})
	for _, i := range units {
		startedUnitsMap[i] = nil
	}
	for _, i := range serviceNames {
		_, ok := startedUnitsMap[fmt.Sprintf("%s.service", i)]
		if !ok {
			t.Errorf("Service %v has not been started by master", i)
		}
	}
}

func checkAllMasterServiceStarted(t *testing.T, s types.SystemdService) {
	checkAllServiceStarted(t, s, masterServices)
}

func checkAllServiceStopped(t *testing.T, s types.SystemdService) {
	if u := s.Units(); len(u) != 0 {
		t.Errorf("All Services should have been stopped. Instead %v are running", u)
	}
}

// NOTE: master service starts systemd, leader and other required service.
// So, the order of starting the services should not impact the behavior.
func TestMasterServiceGiveupLeadership(t *testing.T) {
	t.Parallel()
	l, s, _, m := setupMaster(t)
	s.Start()
	l.Start()
	m.Start()

	checkAllMasterServiceStarted(t, s)

	l.GiveupLeadership()
	checkAllServiceStopped(t, s)

	l.ChangeLeadership()
	checkAllServiceStopped(t, s)
}

func TestMasterServiceStopServices(t *testing.T) {
	t.Parallel()
	l, s, _, m := setupMaster(t)

	// try a different start sequence
	l.Start()
	s.Start()
	m.Start()

	checkAllMasterServiceStarted(t, s)
	m.Stop()
	checkAllServiceStopped(t, s)
}

func TestMasterServiceStartBeforeLeaderService(t *testing.T) {
	t.Parallel()
	l, s, _, m := setupMaster(t)

	// yet another start sequence
	s.Start()
	m.Start()
	l.Start()

	checkAllMasterServiceStarted(t, s)
}

// On leadership win, set the Status in Cluster to my node
func TestMasterServiceSetStatus(t *testing.T) {
	t.Parallel()
	l, s, cw, m := setupMaster(t)

	// master status gets updated by master's cfgWatcherService handlers
	testutils.Assert(t, cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == "", "Leader is non-nil at start of test")
	s.Start()
	l.Start()
	m.Start()

	testutils.AssertEventually(t,
		func() (bool, interface{}) {
			return cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == t.Name(), nil
		}, "Leader is not this node at the end of test")
}

func TestMasterServiceSetStatusOnLeadershipWin(t *testing.T) {
	t.Parallel()
	l, s, cw, m := setupMaster(t)

	s.Start()
	l.Start()
	l.GiveupLeadership()
	m.Start()

	// Non-leader should not update the status
	testutils.Assert(t, cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == "", "Leader is non-nil at start of test")

	l.BecomeLeader()
	testutils.AssertEventually(t,
		func() (bool, interface{}) {
			return cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == t.Name(), nil
		},
		"Leader is not this node at the end of test")

}

func TestMasterServiceSetStatusOnConfigWatch(t *testing.T) {
	t.Parallel()
	l, s, cw, m := setupMaster(t)

	s.Start()
	l.Start() // node t.Name() will always become the leader. refer mock leader service
	m.Start() // leader event from above will be observed at master only when the master starts

	// leaderSvc(l).LeaderID is the source of truth. So, the cluster status should be updated to this
	// whenever the cluster/leader event is observed

	// master should update the Leader to himself if he sees a kvstore Created/Updated event on Cluster
	cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader = "dummy"
	cw.ClusterHandler(kvstore.Created, &cmd.Cluster{}) // handlers are set when master starts
	testutils.AssertEventually(t,
		func() (bool, interface{}) {
			return cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == t.Name(), nil
		}, "Leader is not this node at the end of test")

	cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader = "dummy"
	cw.ClusterHandler(kvstore.Updated, &cmd.Cluster{})
	testutils.AssertEventually(t,
		func() (bool, interface{}) {
			return cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == t.Name(), nil
		}, "Leader is not this node at the end of test")

	cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader = "dummy"
	cw.ClusterHandler(kvstore.Updated, &cmd.Cluster{})
	testutils.AssertEventually(t,
		func() (bool, interface{}) {
			return cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == t.Name(), nil
		}, "Leadership not corrected in timer at the end of the test")

	l.GiveupLeadership()

	// only leader can update the status.
	// In this test scenario, once the leadership is given up or changed there is no valid
	// master(leader) to update the status (refer mock leader service).
	// so, it will remain dummy.
	cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader = "dummy"
	cw.ClusterHandler(kvstore.Updated, &cmd.Cluster{})
	testutils.Assert(t, cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == "dummy", "Non-Leader reacting to cluster object creation")

	testutils.AssertEventually(t,
		func() (bool, interface{}) {
			return cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == "dummy", nil
		}, "Non-Leader reacting to cluster object creation")
}
