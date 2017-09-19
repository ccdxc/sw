package services

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types"
)

func setupMaster(t *testing.T) (*mock.LeaderService, types.SystemdService, types.MasterService) {
	testIP := "11.1.1.1"

	l := mock.NewLeaderService(t.Name())
	s := NewSystemdService(WithSysIfSystemdSvcOption(&mock.SystemdIf{}))
	m := NewMasterService(testIP, WithLeaderSvcMasterOption(l), WithSystemdSvcMasterOption(s), WithConfigsMasterOption(&mock.Configs{}))
	return l, s, m
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
	l, s, m := setupMaster(t)
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
	l, s, m := setupMaster(t)

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
	l, s, m := setupMaster(t)

	// yet another start sequence
	s.Start()
	m.Start()
	l.Start()

	checkAllMasterServiceStarted(t, s)
}
