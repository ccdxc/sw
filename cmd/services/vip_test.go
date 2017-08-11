package services

import (
	"testing"

	"github.com/pensando/sw/cmd/services/mock"
)

// NOTE: These tests assume the current implementation of ONE leaderService for all the virtualIPs.
// When we support per-VIP leaderService, need to add more tests

func TestVIPServiceAfterLeaderStart(t *testing.T) {
	t.Parallel()
	id := "TestVIPServiceAfterLeaderStart"
	testIP := "11.1.1.1"
	testIP2 := "11.1.1.2"

	l := mock.NewLeaderService(id)
	i := mock.NewIPService()
	vipSvc := NewVIPService(WithLeaderServiceVIPOption(l), WithIPServiceVIPOption(i))

	l.Start()

	if err := vipSvc.AddVirtualIPs(testIP); err != nil {
		t.Errorf("AddVirtualIPs(%v) returned %v", testIP, err)
	}

	allVIPs := vipSvc.GetAllVirtualIPs()
	if len(allVIPs) != 1 || allVIPs[0] != testIP {
		t.Errorf("expected %v got %v", testIP, allVIPs)
	}

	configuredVIPs := vipSvc.GetConfiguredVirtualIPs()
	if len(configuredVIPs) != 1 || configuredVIPs[0] != testIP {
		t.Errorf("expected %v not returned from GetConfiguredVirtualIPs(). got %v", testIP, configuredVIPs)
	}

	if err := vipSvc.AddVirtualIPs(testIP2); err != nil {
		t.Errorf("AddVirtualIPs(%v) returned %v", testIP2, err)
	}
	configuredVIPs = vipSvc.GetConfiguredVirtualIPs()
	if len(configuredVIPs) != 2 ||
		!((configuredVIPs[0] == testIP && configuredVIPs[1] == testIP2) || (configuredVIPs[1] == testIP && configuredVIPs[0] == testIP2)) {
		t.Errorf("GetConfiguredVirtualIPs() returned unexpected %v", configuredVIPs)
	}
	if err := vipSvc.DeleteVirtualIPs(testIP2); err != nil {
		t.Errorf("DeleteVirtualIPs(%v) returned %v", testIP2, err)
	}
	configuredVIPs = vipSvc.GetConfiguredVirtualIPs()
	if len(configuredVIPs) != 1 || configuredVIPs[0] != testIP {
		t.Errorf("expected %v not returned from GetConfiguredVirtualIPs(). got %v", testIP, configuredVIPs)
	}

	i.SetError()
	if err := vipSvc.AddVirtualIPs(testIP2); err == nil {
		t.Errorf("Expected AddVirtualIPS to return an error. Didnt get error	")
	}
	if len(configuredVIPs) != 1 || configuredVIPs[0] != testIP {
		t.Errorf("expected %v not returned from GetConfiguredVirtualIPs(). got %v", testIP, configuredVIPs)
	}

}

func TestVIPServiceBeforeLeaderStart(t *testing.T) {
	t.Parallel()
	id := "TestVIPServiceBeforeLeaderStart"
	testIP := "11.1.1.1"

	l := mock.NewLeaderService(id)
	i := mock.NewIPService()
	vipSvc := NewVIPService(WithLeaderServiceVIPOption(l), WithIPServiceVIPOption(i))
	vipSvc.AddVirtualIPs(testIP)

	allVIPs := vipSvc.GetAllVirtualIPs()
	if len(allVIPs) != 1 || allVIPs[0] != testIP {
		t.Errorf("expected %v got %v", testIP, allVIPs)
	}
	configuredVIPs := vipSvc.GetConfiguredVirtualIPs()
	if configuredVIPs != nil && len(configuredVIPs) != 0 {
		t.Errorf("expected %v got %v", nil, configuredVIPs)
	}

	l.Start()
	configuredVIPs = vipSvc.GetConfiguredVirtualIPs()
	if len(configuredVIPs) != 1 || configuredVIPs[0] != testIP {
		t.Errorf("expected %v not returned from GetConfiguredVirtualIPs(). got %v", testIP, configuredVIPs)
	}

	// When we are no longer the leader, giveup IPs
	if err := l.GiveupLeadership(); err != nil {
		t.Errorf("GiveupLeadership failed with error %v", err)
	}
	configuredVIPs = vipSvc.GetConfiguredVirtualIPs()
	if configuredVIPs != nil && len(configuredVIPs) != 0 {
		t.Errorf("expected %v got %v", nil, configuredVIPs)
	}

	// ChangeLeadership (not to us) still should not program the entries
	if err := l.ChangeLeadership(); err != nil {
		t.Errorf("ChangeLeadership failed with error %v", err)
	}
	configuredVIPs = vipSvc.GetConfiguredVirtualIPs()
	if configuredVIPs != nil && len(configuredVIPs) != 0 {
		t.Errorf("expected %v got %v", nil, configuredVIPs)
	}

}

func TestVIPServiceWithIPLayerErrors(t *testing.T) {
	t.Parallel()

	id := "TestVIPServiceWithIPLayerErrors"
	testIP := "11.1.1.1"

	l := mock.NewLeaderService(id)
	i := mock.NewIPService()
	vipSvc := NewVIPService(WithLeaderServiceVIPOption(l), WithIPServiceVIPOption(i))
	l.Start()
	vipSvc.AddVirtualIPs(testIP)

	configuredVIPs := vipSvc.GetConfiguredVirtualIPs()
	if len(configuredVIPs) != 1 || configuredVIPs[0] != testIP {
		t.Errorf("expected %v not returned from GetConfiguredVirtualIPs(). got %v", testIP, configuredVIPs)
	}

	i.SetError() // IP Layer is mocking error. VIP layer should not think IPs are configured
	if err := l.BecomeLeader(); err == nil {
		t.Errorf("expected error to be returned to leaderService when IP layer errors. Didnt get error")
	}
	configuredVIPs = vipSvc.GetConfiguredVirtualIPs()
	if configuredVIPs != nil && len(configuredVIPs) != 0 {
		t.Errorf("expected %v got %v", nil, configuredVIPs)
	}

	i.ClearError()
	if err := l.BecomeLeader(); err != nil {
		t.Errorf("BecomeLeader failed with error %v", err)
	}

}
