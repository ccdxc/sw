// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"testing"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/events/generated/eventtypes"
	orchutils "github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	// create mock events recorder
	eventRecorder = mockevtsrecorder.NewRecorder("alert_test",
		log.GetNewLogger(log.GetDefaultConfig("alert_test")))
	_ = recorder.Override(eventRecorder)
)

const (
	InsertionProfile   = "InsertionEnforcePolicy"
	TransparentProfile = "TransparentBasenetPolicy"
	OrchName           = "test-orchestrator"
)

func createProfilesOrch(stateMgr *Statemgr, t *testing.T) {
	dscprof := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: TransparentProfile,
		},
		Spec: cluster.DSCProfileSpec{
			Features: cluster.FeatureSet{
				InterVMServices: false,
				FlowAware:       false,
				Firewall:        false,
			},
		},
	}
	err := stateMgr.ctrler.DSCProfile().Create(&dscprof)
	AssertOk(t, err, "Could not create the smartNic profile")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindDSCProfile("", TransparentProfile)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Did not find DSCProfile", "1ms", "2s")

	dscprof = cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: InsertionProfile,
		},
		Spec: cluster.DSCProfileSpec{
			Features: cluster.FeatureSet{
				InterVMServices: true,
				FlowAware:       true,
				Firewall:        true,
			},
		},
	}
	err = stateMgr.ctrler.DSCProfile().Create(&dscprof)
	AssertOk(t, err, "Could not create the smartNic profile")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindDSCProfile("", TransparentProfile)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Did not find DSCProfile", "1ms", "2s")

	createOrchestrator(stateMgr, "default", OrchName, nil)

}

func TestAlertWorkloadCreationEvents(t *testing.T) {
	stateMgr, _, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	createProfilesOrch(stateMgr, t)
	eventRecorder.ClearEvents()

	orchLabels := make(map[string]string)
	orchLabels[orchutils.OrchNameKey] = OrchName

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{
			DSCProfile: TransparentProfile,
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: cluster.DistributedServiceCardStatus_ADMITTED.String(),
			PrimaryMAC:     "0001.0203.0405",
			Host:           "dsc-source",
			IPConfig: &cluster.IPConfig{
				IPAddress: "10.20.30.11/16",
			},
		},
	}
	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
			Labels: orchLabels,
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "1001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	AssertEventually(t, func() (bool, interface{}) {
		for _, evt := range eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.ORCH_DSC_MODE_INCOMPATIBLE.String() {
				return true, nil
			}
		}

		return false, nil
	}, "Failed to find mode conflict  event", "1s", "10s")
}

func TestAlertWorkloadCreationEventDecommissioned(t *testing.T) {
	stateMgr, _, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	createProfilesOrch(stateMgr, t)
	eventRecorder.ClearEvents()

	orchLabels := make(map[string]string)
	orchLabels[orchutils.OrchNameKey] = OrchName

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{
			DSCProfile: TransparentProfile,
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: cluster.DistributedServiceCardStatus_DECOMMISSIONED.String(),
			PrimaryMAC:     "0001.0203.0405",
			Host:           "dsc-source",
			IPConfig: &cluster.IPConfig{
				IPAddress: "10.20.30.11/16",
			},
		},
	}
	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
			Labels: orchLabels,
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: "testHost",
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "1001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
		},
	}

	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	AssertEventually(t, func() (bool, interface{}) {
		for _, evt := range eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.ORCH_DSC_NOT_ADMITTED.String() {
				return true, nil
			}
		}

		return false, nil
	}, "Failed to find mode conflict  event", "1s", "10s")
}

func TestAlertWorkloadCreationEventDSCProfileChanged(t *testing.T) {
	stateMgr, _, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	createProfilesOrch(stateMgr, t)
	eventRecorder.ClearEvents()

	orchLabels := make(map[string]string)
	orchLabels[orchutils.OrchNameKey] = OrchName

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{
			DSCProfile: InsertionProfile,
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: cluster.DistributedServiceCardStatus_ADMITTED.String(),
			PrimaryMAC:     "0001.0203.0405",
			Host:           "dsc-source",
			IPConfig: &cluster.IPConfig{
				IPAddress: "10.20.30.11/16",
			},
		},
	}
	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
			Labels: orchLabels,
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")

	latestSnic, err := stateMgr.ctrler.DistributedServiceCard().Find(&snic.ObjectMeta)
	AssertOk(t, err, "Could not find the DSC")
	Assert(t, len(latestSnic.DistributedServiceCard.ObjectMeta.Labels) > 0, "DSC does not have labels")
	_, ok := latestSnic.DistributedServiceCard.ObjectMeta.Labels[orchutils.OrchNameKey]
	Assert(t, ok, "DSC does not have orch label")

	latestSnic.DistributedServiceCard.Spec.DSCProfile = TransparentProfile

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Update(&latestSnic.DistributedServiceCard)
	AssertOk(t, err, "Could not create the smartNic")
	AssertEventually(t, func() (bool, interface{}) {
		for _, evt := range eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.ORCH_DSC_MODE_INCOMPATIBLE.String() {
				return true, nil
			}
		}

		return false, nil
	}, "Failed to find mode conflict  event", "1s", "10s")
}

func TestAlertWorkloadHostUpdateDSCCheck(t *testing.T) {
	stateMgr, _, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	createProfilesOrch(stateMgr, t)
	eventRecorder.ClearEvents()

	orchLabels := make(map[string]string)
	orchLabels[orchutils.OrchNameKey] = OrchName

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{
			DSCProfile: InsertionProfile,
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: cluster.DistributedServiceCardStatus_ADMITTED.String(),
			PrimaryMAC:     "0001.0203.0405",
			Host:           "dsc-source",
			IPConfig: &cluster.IPConfig{
				IPAddress: "10.20.30.11/16",
			},
		},
	}
	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")
	latestSnic, err := stateMgr.ctrler.DistributedServiceCard().Find(&snic.ObjectMeta)
	AssertOk(t, err, "Could not find the DSC")
	Assert(t, len(latestSnic.DistributedServiceCard.ObjectMeta.Labels) == 0, "DSC has labels when it should not")

	host.ObjectMeta.Labels = orchLabels
	err = stateMgr.ctrler.Host().SyncUpdate(&host)
	AssertOk(t, err, "Could not update the host")

	AssertEventually(t, func() (bool, interface{}) {
		latestSnic, err = stateMgr.ctrler.DistributedServiceCard().Find(&snic.ObjectMeta)
		if err == nil && len(latestSnic.DistributedServiceCard.ObjectMeta.Labels) > 0 {
			return true, nil
		}
		return false, nil
	}, "DSC does not have labels added to it", "1s", "5s")

	host.ObjectMeta.Labels = nil
	err = stateMgr.ctrler.Host().SyncUpdate(&host)
	AssertOk(t, err, "Could not update the host")

	AssertEventually(t, func() (bool, interface{}) {
		latestSnic, err = stateMgr.ctrler.DistributedServiceCard().Find(&snic.ObjectMeta)
		if err == nil && len(latestSnic.DistributedServiceCard.ObjectMeta.Labels) == 0 {
			return true, nil
		}
		return false, nil
	}, "DSC labels were not removed", "1s", "5s")
}

func TestAlertWorkloadHostDeleteDSCCheck(t *testing.T) {
	stateMgr, _, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	createProfilesOrch(stateMgr, t)
	eventRecorder.ClearEvents()

	orchLabels := make(map[string]string)
	orchLabels[orchutils.OrchNameKey] = OrchName

	// smartNic params
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard",
		},
		Spec: cluster.DistributedServiceCardSpec{
			DSCProfile: InsertionProfile,
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: cluster.DistributedServiceCardStatus_ADMITTED.String(),
			PrimaryMAC:     "0001.0203.0405",
			Host:           "dsc-source",
			IPConfig: &cluster.IPConfig{
				IPAddress: "10.20.30.11/16",
			},
		},
	}
	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snic)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   "testHost",
			Tenant: "default",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0405",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&host)
	AssertOk(t, err, "Could not create the host")
	latestSnic, err := stateMgr.ctrler.DistributedServiceCard().Find(&snic.ObjectMeta)
	AssertOk(t, err, "Could not find the DSC")
	Assert(t, len(latestSnic.DistributedServiceCard.ObjectMeta.Labels) == 0, "DSC has labels when it should not")

	host.ObjectMeta.Labels = orchLabels
	err = stateMgr.ctrler.Host().Update(&host)
	AssertOk(t, err, "Could not update the host")

	AssertEventually(t, func() (bool, interface{}) {
		latestSnic, err = stateMgr.ctrler.DistributedServiceCard().Find(&snic.ObjectMeta)
		log.Infof("Got latestSnic [%v] ", latestSnic)
		if err == nil && len(latestSnic.DistributedServiceCard.ObjectMeta.Labels) > 0 {
			return true, nil
		}
		return false, nil
	}, "DSC does not have labels added to it", "1s", "5s")

	err = stateMgr.ctrler.Host().Delete(&host)
	AssertOk(t, err, "Could not delete the host")

	AssertEventually(t, func() (bool, interface{}) {
		latestSnic, err = stateMgr.ctrler.DistributedServiceCard().Find(&snic.ObjectMeta)
		log.Infof("Got latestSnic [%v] ", latestSnic)
		if err == nil && len(latestSnic.DistributedServiceCard.ObjectMeta.Labels) == 0 {
			return true, nil
		}
		return false, nil
	}, "DSC labels were not removed", "1s", "5s")
}
