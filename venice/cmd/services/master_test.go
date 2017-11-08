package services

import (
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/testutils"
)

func setupMaster(t *testing.T) (*mock.LeaderService, types.SystemdService, *mock.CfgWatcherService, types.MasterService) {
	testIP := "11.1.1.1"

	l := mock.NewLeaderService(t.Name())
	s := NewSystemdService(WithSysIfSystemdSvcOption(&mock.SystemdIf{}))
	cw := mock.CfgWatcherService{}
	m := NewMasterService(testIP, WithLeaderSvcMasterOption(l), WithSystemdSvcMasterOption(s), WithConfigsMasterOption(&mock.Configs{}),
		WithCfgWatcherMasterOption(&cw), WithK8sSvcMasterOption(&mock.K8sService{}))

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

	testutils.Assert(t, cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == "", "Leader is non-nil at start of test")
	s.Start()
	l.Start()
	m.Start()

	testutils.AssertEventually(t,
		func() (bool, []interface{}) {
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
		func() (bool, []interface{}) {
			return cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == t.Name(), nil
		},
		"Leader is not this node at the end of test")
}

func TestMasterServiceSetStatusOnConfigWatch(t *testing.T) {
	t.Parallel()
	l, s, cw, m := setupMaster(t)
	clusterStatusUpdateTime = time.Second

	s.Start()
	l.Start()
	m.Start()

	// master should update the Leader to himself if he sees a kvstore Created/Updated event on Cluster
	cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader = "dummy"
	cw.ClusterHandler(kvstore.Created, &cmd.Cluster{})
	testutils.AssertEventually(t,
		func() (bool, []interface{}) {
			return cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == t.Name(), nil
		}, "Leader is not this node at the end of test")

	cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader = "dummy"
	cw.ClusterHandler(kvstore.Updated, &cmd.Cluster{})
	testutils.AssertEventually(t,
		func() (bool, []interface{}) {
			return cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == t.Name(), nil
		}, "Leader is not this node at the end of test")

	cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader = "dummy"
	cw.ClusterHandler(kvstore.Updated, &cmd.Cluster{})
	testutils.AssertEventually(t,
		func() (bool, []interface{}) {
			return cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == t.Name(), nil
		}, "Leadership not corrected in timer at the end of the test")

	l.GiveupLeadership()
	cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader = "dummy"
	cw.ClusterHandler(kvstore.Updated, &cmd.Cluster{})
	testutils.Assert(t, cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == "dummy", "Non-Leader reacting to cluster object creation")

	testutils.AssertEventually(t,
		func() (bool, []interface{}) {
			return cw.DummyAPIClient.DummyCluster.DummyCluster.Status.Leader == "dummy", nil
		}, "Non-Leader reacting to cluster object creation")
}
