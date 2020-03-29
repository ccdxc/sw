// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/events/generated/eventtypes"
	orchutils "github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/ref"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	// create mock events recorder
	eventRecorder = mockevtsrecorder.NewRecorder("migration_test",
		log.GetNewLogger(log.GetDefaultConfig("migration_test")))
	_ = recorder.Override(eventRecorder)
)

const (
	InsertionProfile   = "InsertionEnforcePolicy"
	TransparentProfile = "TransparentBasenetPolicy"
)

// TestMigrationWorkloadMigration tests for successful workload migration
// All workload migration requests are received by NPM as a workload update
// which is translated into endpoint update
func TestMigrationWorkloadMigration(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	sourceHost := "testHost"
	destHost := "testHost-2"

	setupTopo(stateMgr, sourceHost, destHost, t)

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

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")
	start := time.Now()

	done := false
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			if !done {
				timeTrack(start, "Network create took")
				done = true
			}
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "5s")

	// verify we can find the endpoint associated with the workload
	foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	// update workload external vlan
	nwr := ref.DeepCopy(wr).(workload.Workload)
	nwr.Spec.HostName = "testHost-2"
	nwr.Spec.Interfaces[0].MicroSegVlan = 200
	nwr.Status.HostName = "testHost"
	nwr.Status.MigrationStatus = &workload.WorkloadMigrationStatus{
		Stage:  "migration-start",
		Status: workload.WorkloadMigrationStatus_NONE.String(),
	}
	nwr.Status.Interfaces = []workload.WorkloadIntfStatus{
		{
			MACAddress:   "1001.0203.0405",
			MicroSegVlan: 100,
			ExternalVlan: 1,
		},
	}
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		ep, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil && ep.Endpoint.Status.Migration.Status == workload.EndpointMigrationStatus_START.String() {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "10s")

	// Send final sync
	nwr.Status.MigrationStatus.Stage = "migration-final-sync"
	nwr.Status.MigrationStatus.Status = workload.WorkloadMigrationStatus_STARTED.String()
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		ep, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil && ep.Endpoint.Status.Migration.Status == workload.EndpointMigrationStatus_FINAL_SYNC.String() {
			return true, nil
		}
		return false, nil
	}, "Endpoint not in correct stage", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		ep, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil && ep.Endpoint.Spec.NodeUUID == ep.Endpoint.Status.NodeUUID {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1s", "20s")

	AssertEventually(t, func() (bool, interface{}) {
		wrk, err := stateMgr.FindWorkload("default", "testWorkload")
		if err == nil && wrk.Workload.Status.MigrationStatus.Status == workload.WorkloadMigrationStatus_DONE.String() {
			return true, nil
		}
		return false, nil
	}, "Workload not found", "1ms", "10s")
}

// TestMigrationWorkloadMigrationAbort starts migration and later aborts it.
// We check if the status in workload object is correctly set
func TestMigrationWorkloadMigrationAbort(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	sourceHost := "testHost"
	destHost := "testHost-2"

	setupTopo(stateMgr, sourceHost, destHost, t)

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

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")
	start := time.Now()

	done := false
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			if !done {
				timeTrack(start, "Network create took")
				done = true
			}
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "5s")

	// verify we can find the endpoint associated with the workload
	foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	// update workload external vlan
	nwr := ref.DeepCopy(wr).(workload.Workload)
	nwr.Spec.HostName = "testHost-2"
	nwr.Spec.Interfaces[0].MicroSegVlan = 200
	nwr.Status.HostName = "testHost"
	nwr.Status.MigrationStatus = &workload.WorkloadMigrationStatus{
		Stage:  "migration-start",
		Status: workload.WorkloadMigrationStatus_NONE.String(),
	}
	nwr.Status.Interfaces = []workload.WorkloadIntfStatus{
		{
			MACAddress:   "1001.0203.0405",
			MicroSegVlan: 100,
			ExternalVlan: 1,
		},
	}
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		ep, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil && ep.Endpoint.Status.Migration.Status == workload.EndpointMigrationStatus_START.String() {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	eventRecorder.ClearEvents()
	// Abort Migration
	nwr = ref.DeepCopy(wr).(workload.Workload)
	nwr.Spec.HostName = "testHost"
	nwr.Spec.Interfaces[0].MicroSegVlan = 100
	nwr.Status.HostName = "testHost"
	nwr.Status.MigrationStatus = &workload.WorkloadMigrationStatus{
		Stage:  workload.WorkloadMigrationStatus_MIGRATION_ABORT.String(),
		Status: workload.WorkloadMigrationStatus_STARTED.String(),
	}
	nwr.Status.Interfaces = []workload.WorkloadIntfStatus{
		{
			MACAddress:   "1001.0203.0405",
			MicroSegVlan: 100,
			ExternalVlan: 1,
		},
	}
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	time.Sleep(12 * time.Second)

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil {
			return true, nil
		}

		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		wrk, err := stateMgr.FindWorkload("default", "testWorkload")
		if err == nil && wrk.Workload.Status.MigrationStatus.Status == workload.WorkloadMigrationStatus_FAILED.String() {
			for _, evt := range eventRecorder.GetEvents() {
				if evt.EventType == eventtypes.MIGRATION_FAILED.String() {
					return true, nil
				}
			}
		}
		return false, nil
	}, "Workload not found", "1ms", "5s")
}

// TestMigrationWorkloadMigrationTimeout sets a low timeout value for migration, and checks if the status in workload object is correctly set
func TestMigrationWorkloadMigrationTimeout(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	sourceHost := "testHost"
	destHost := "testHost-2"

	setupTopo(stateMgr, sourceHost, destHost, t)

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
			// Set a low migration timeout
			MigrationTimeout: "3s",
		},
	}
	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")
	start := time.Now()

	done := false
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			if !done {
				timeTrack(start, "Network create took")
				done = true
			}
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "5s")

	// verify we can find the endpoint associated with the workload
	foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")

	// update workload external vlan
	nwr := ref.DeepCopy(wr).(workload.Workload)
	nwr.Spec.HostName = "testHost-2"
	nwr.Spec.Interfaces[0].MicroSegVlan = 200
	nwr.Status.HostName = "testHost"
	nwr.Status.MigrationStatus = &workload.WorkloadMigrationStatus{
		Stage:  "migration-start",
		Status: workload.WorkloadMigrationStatus_NONE.String(),
	}
	nwr.Status.Interfaces = []workload.WorkloadIntfStatus{
		{
			MACAddress:   "1001.0203.0405",
			MicroSegVlan: 100,
			ExternalVlan: 1,
		},
	}
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		ep, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil && ep.Endpoint.Status.Migration.Status == workload.EndpointMigrationStatus_START.String() {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	eventRecorder.ClearEvents()
	time.Sleep(10 * time.Second)

	AssertEventually(t, func() (bool, interface{}) {
		ep, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil && ep.Endpoint.Status.Migration.Status == workload.WorkloadMigrationStatus_DONE.String() {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		wrk, err := stateMgr.FindWorkload("default", "testWorkload")
		if err == nil && wrk.Workload.Status.MigrationStatus.Status == workload.WorkloadMigrationStatus_TIMED_OUT.String() {
			for _, evt := range eventRecorder.GetEvents() {
				if evt.EventType == eventtypes.MIGRATION_TIMED_OUT.String() {
					return true, nil
				}
			}
		}
		return false, nil
	}, "Workload not found", "1ms", "1s")
}

func setupTopo(stateMgr *Statemgr, sourceHost, destHost string, t *testing.T) {
	// create tenant
	err := createTenant(t, stateMgr, "default")
	AssertOk(t, err, "Error creating the tenant")

	// create DSC profile
	dscprof := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: InsertionProfile,
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        cluster.DSCProfileSpec_INSERTION.String(),
			FlowPolicyMode: cluster.DSCProfileSpec_ENFORCED.String(),
		},
	}

	err = stateMgr.ctrler.DSCProfile().Create(&dscprof)
	AssertOk(t, err, "Could not create the smartNic profile")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindDSCProfile("", InsertionProfile)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Did not find DSCProfile", "1ms", "2s")

	dscprof = cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: TransparentProfile,
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        cluster.DSCProfileSpec_TRANSPARENT.String(),
			FlowPolicyMode: cluster.DSCProfileSpec_BASENET.String(),
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
			Name:   sourceHost,
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

	// smartNic params
	snicDest := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "testDistributedServiceCard2",
		},
		Spec: cluster.DistributedServiceCardSpec{
			DSCProfile: InsertionProfile,
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: cluster.DistributedServiceCardStatus_ADMITTED.String(),
			PrimaryMAC:     "0001.0203.0406",
			Host:           "dsc-destintion",
			IPConfig: &cluster.IPConfig{
				IPAddress: "10.20.30.12/16",
			},
		},
	}

	// create the smartNic
	err = stateMgr.ctrler.DistributedServiceCard().Create(&snicDest)
	AssertOk(t, err, "Could not create the smartNic")

	// host params
	hostDest := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:   destHost,
			Tenant: "default",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: "0001.0203.0406",
				},
			},
		},
	}

	// create the host
	err = stateMgr.ctrler.Host().Create(&hostDest)
	AssertOk(t, err, "Could not create the destination host")

}

func TestMigrationOnCreateWithMigrationStageFinalSync(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	sourceHost := "testHost"
	destHost := "testHost-2"

	setupTopo(stateMgr, sourceHost, destHost, t)
	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: sourceHost,
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "1001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
			// Simulating case where 5s are left before
			MigrationTimeout: "5s",
		},
		Status: workload.WorkloadStatus{
			HostName: destHost,
			Interfaces: []workload.WorkloadIntfStatus{
				{
					MACAddress:   "1001.0203.0405",
					MicroSegVlan: 200,
					ExternalVlan: 1,
				},
			},
			MigrationStatus: &workload.WorkloadMigrationStatus{
				StartedAt: &api.Timestamp{},
				Stage:     workload.WorkloadMigrationStatus_MIGRATION_FINAL_SYNC.String(),
				Status:    workload.WorkloadMigrationStatus_STARTED.String(),
			},
		},
	}

	wr.Status.MigrationStatus.StartedAt.SetTime(time.Now())

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")
	start := time.Now()

	done := false
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			if !done {
				timeTrack(start, "Network create took")
				done = true
			}
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	wrFound, err := stateMgr.FindWorkload("default", "testWorkload")
	AssertOk(t, err, "Did not find workload")
	logger.Infof("Got Workload %v", wrFound.Workload)
	Assert(t, (wrFound.Workload.Status.MigrationStatus != nil), "workload migration status is nil")
	Assert(t, (wrFound.Workload.Status.MigrationStatus.Stage == workload.WorkloadMigrationStatus_MIGRATION_FINAL_SYNC.String()), fmt.Sprintf("migration final sync state not found %v", wrFound.Workload))

	// verify we can find the endpoint associated with the workload
	foundEp, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
	AssertOk(t, err, "Could not find the endpoint")
	Assert(t, (foundEp.Endpoint.Status.WorkloadName == wr.Name), "endpoint params did not match")
	Assert(t, (foundEp.Endpoint.Status.Migration != nil), fmt.Sprintf("Migration status is nil, EP : %v", foundEp.Endpoint))
	Assert(t, (foundEp.Endpoint.Status.Migration.Status == workload.EndpointMigrationStatus_FINAL_SYNC.String()), fmt.Sprintf("endpoint not in final sync phase. EP : %v", foundEp.Endpoint))

	// Wait for timer to expire
	time.Sleep(10 * time.Second)

	AssertEventually(t, func() (bool, interface{}) {
		wrFound, err := stateMgr.FindWorkload("default", "testWorkload")
		if err != nil {
			log.Errorf("Workload not found. :( Err : %v", err)
			return false, nil
		}

		if wrFound.Workload.Status.MigrationStatus.Status == workload.WorkloadMigrationStatus_TIMED_OUT.String() {
			return true, nil
		}

		log.Errorf("Worklaod found is [%v]", wrFound)
		return false, nil
	}, fmt.Sprintf("Workload not in expected status [%v]", workload.WorkloadMigrationStatus_TIMED_OUT.String()), "1s", "20s")
}

func TestMigrationStartTimeoutLastSync(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	sourceHost := "testHost"
	destHost := "testHost-2"

	setupTopo(stateMgr, sourceHost, destHost, t)
	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: sourceHost,
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "1001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
			// Simulating case where 5s are left before
			MigrationTimeout: "5s",
		},
		Status: workload.WorkloadStatus{
			HostName: destHost,
			Interfaces: []workload.WorkloadIntfStatus{
				{
					MACAddress:   "1001.0203.0405",
					MicroSegVlan: 200,
					ExternalVlan: 1,
				},
			},
			MigrationStatus: &workload.WorkloadMigrationStatus{
				StartedAt: &api.Timestamp{},
				Stage:     workload.WorkloadMigrationStatus_MIGRATION_START.String(),
				Status:    workload.WorkloadMigrationStatus_NONE.String(),
			},
		},
	}

	wr.Status.MigrationStatus.StartedAt.SetTime(time.Now())

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")
	start := time.Now()

	done := false
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			if !done {
				timeTrack(start, "Network create took")
				done = true
			}
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	// Wait for timer to expire
	time.Sleep(10 * time.Second)

	AssertEventually(t, func() (bool, interface{}) {
		wrFound, err := stateMgr.FindWorkload("default", "testWorkload")
		if err != nil {
			log.Errorf("Workload not found. :( Err : %v", err)
			return false, nil
		}

		if wrFound.Workload.Status.MigrationStatus.Status == workload.WorkloadMigrationStatus_TIMED_OUT.String() {
			return true, nil
		}

		log.Errorf("Worklaod found is [%v]", wrFound)
		return false, nil
	}, fmt.Sprintf("Workload not in expected status [%v]", workload.WorkloadMigrationStatus_TIMED_OUT.String()), "1s", "20s")

	nwr := ref.DeepCopy(wr).(workload.Workload)
	// Send final sync
	nwr.Status.MigrationStatus.Stage = workload.WorkloadMigrationStatus_MIGRATION_FINAL_SYNC.String()
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	// Workload should continue to be in TIMED_OUT status
	AssertEventually(t, func() (bool, interface{}) {
		wrFound, err := stateMgr.FindWorkload("default", "testWorkload")
		if err != nil {
			log.Errorf("Workload not found. :( Err : %v", err)
			return false, nil
		}

		if wrFound.Workload.Status.MigrationStatus.Status == workload.WorkloadMigrationStatus_TIMED_OUT.String() {
			return true, nil
		}

		log.Errorf("Worklaod found is [%v]", wrFound)
		return false, nil
	}, fmt.Sprintf("Workload not in expected status [%v]", workload.WorkloadMigrationStatus_TIMED_OUT.String()), "1s", "20s")
}

func TestMigrationStartTimeoutAbort(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	sourceHost := "testHost"
	destHost := "testHost-2"

	setupTopo(stateMgr, sourceHost, destHost, t)
	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: sourceHost,
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "1001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
			// Simulating case where 5s are left before
			MigrationTimeout: "5s",
		},
		Status: workload.WorkloadStatus{
			HostName: destHost,
			Interfaces: []workload.WorkloadIntfStatus{
				{
					MACAddress:   "1001.0203.0405",
					MicroSegVlan: 200,
					ExternalVlan: 1,
				},
			},
			MigrationStatus: &workload.WorkloadMigrationStatus{
				StartedAt: &api.Timestamp{},
				Stage:     workload.WorkloadMigrationStatus_MIGRATION_START.String(),
				Status:    workload.WorkloadMigrationStatus_NONE.String(),
			},
		},
	}

	wr.Status.MigrationStatus.StartedAt.SetTime(time.Now())

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")
	start := time.Now()

	done := false
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			if !done {
				timeTrack(start, "Network create took")
				done = true
			}
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	// Wait for timer to expire
	time.Sleep(10 * time.Second)

	AssertEventually(t, func() (bool, interface{}) {
		wrFound, err := stateMgr.FindWorkload("default", "testWorkload")
		if err != nil {
			log.Errorf("Workload not found. :( Err : %v", err)
			return false, nil
		}

		if wrFound.Workload.Status.MigrationStatus.Status == workload.WorkloadMigrationStatus_TIMED_OUT.String() {
			return true, nil
		}

		log.Errorf("Worklaod found is [%v]", wrFound)
		return false, nil
	}, fmt.Sprintf("Workload not in expected status [%v]", workload.WorkloadMigrationStatus_TIMED_OUT.String()), "1s", "20s")

	nwr := ref.DeepCopy(wr).(workload.Workload)
	// Send Abort
	nwr.Status.MigrationStatus.Stage = workload.WorkloadMigrationStatus_MIGRATION_ABORT.String()
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	// Workload should continue to be in TIMED_OUT status
	AssertEventually(t, func() (bool, interface{}) {
		wrFound, err := stateMgr.FindWorkload("default", "testWorkload")
		if err != nil {
			log.Errorf("Workload not found. :( Err : %v", err)
			return false, nil
		}

		if wrFound.Workload.Status.MigrationStatus.Status == workload.WorkloadMigrationStatus_TIMED_OUT.String() {
			return true, nil
		}

		log.Errorf("Worklaod found is [%v]", wrFound)
		return false, nil
	}, fmt.Sprintf("Workload not in expected status [%v]", workload.WorkloadMigrationStatus_TIMED_OUT.String()), "1s", "20s")
}

func TestMigrationStartFinalSyncAbortAfterDone(t *testing.T) {
	// create network state manager
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}
	sourceHost := "testHost"
	destHost := "testHost-2"

	setupTopo(stateMgr, sourceHost, destHost, t)
	// workload params
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testWorkload",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: workload.WorkloadSpec{
			HostName: sourceHost,
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   "1001.0203.0405",
					MicroSegVlan: 100,
					ExternalVlan: 1,
				},
			},
			// Simulating case where 5s are left before
			MigrationTimeout: "60s",
		},
		Status: workload.WorkloadStatus{
			HostName: destHost,
			Interfaces: []workload.WorkloadIntfStatus{
				{
					MACAddress:   "1001.0203.0405",
					MicroSegVlan: 200,
					ExternalVlan: 1,
				},
			},
			MigrationStatus: &workload.WorkloadMigrationStatus{
				StartedAt: &api.Timestamp{},
				Stage:     workload.WorkloadMigrationStatus_MIGRATION_START.String(),
				Status:    workload.WorkloadMigrationStatus_NONE.String(),
			},
		},
	}

	wr.Status.MigrationStatus.StartedAt.SetTime(time.Now())

	// create the workload
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")
	start := time.Now()

	done := false
	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindNetwork("default", "Network-Vlan-1")
		if err == nil {
			if !done {
				timeTrack(start, "Network create took")
				done = true
			}
			return true, nil
		}
		return false, nil
	}, "Network not foud", "1ms", "1s")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Endpoint not found", "1ms", "1s")

	nwr := ref.DeepCopy(wr).(workload.Workload)
	// Send final sync
	nwr.Status.MigrationStatus.Stage = workload.WorkloadMigrationStatus_MIGRATION_FINAL_SYNC.String()
	nwr.Status.MigrationStatus.Status = workload.WorkloadMigrationStatus_STARTED.String()
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	AssertEventually(t, func() (bool, interface{}) {
		ep, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil && ep.Endpoint.Status.Migration.Status == workload.EndpointMigrationStatus_FINAL_SYNC.String() {
			return true, nil
		}
		return false, nil
	}, "Endpoint not in correct stage", "1s", "5s")

	// Wait for migration to be done
	AssertEventually(t, func() (bool, interface{}) {
		ep, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil && ep.Endpoint.Status.Migration.Status == workload.EndpointMigrationStatus_DONE.String() {
			return true, nil
		}
		return false, nil
	}, "Endpoint not in correct stage", "1s", "15s")

	// Send final sync
	nwr.Status.MigrationStatus.Stage = workload.WorkloadMigrationStatus_MIGRATION_ABORT.String()
	err = stateMgr.ctrler.Workload().Update(&nwr)
	AssertOk(t, err, "Could not update the workload")

	// Endpoint should continue to be in DONE stage
	AssertEventually(t, func() (bool, interface{}) {
		ep, err := stateMgr.FindEndpoint("default", "testWorkload-1001.0203.0405")
		if err == nil && ep.Endpoint.Status.Migration.Status == workload.EndpointMigrationStatus_DONE.String() {
			return true, nil
		}
		return false, nil
	}, "Endpoint not in correct stage", "1s", "5s")
}

func TestMigrationWorkloadCreationEvents(t *testing.T) {
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	orchLabels := make(map[string]string)
	orchLabels[orchutils.OrchNameKey] = "test-orchestrator"

	dscprof := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: TransparentProfile,
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        cluster.DSCProfileSpec_TRANSPARENT.String(),
			FlowPolicyMode: cluster.DSCProfileSpec_BASENET.String(),
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

	eventRecorder.ClearEvents()
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	AssertEventually(t, func() (bool, interface{}) {
		for _, evt := range eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.HOST_DSC_MODE_INCOMPATIBLE.String() {
				return true, nil
			}
		}

		return false, nil
	}, "Failed to find mode conflict  event", "1s", "10s")
}

func TestMigrationWorkloadCreationEventDecommissioned(t *testing.T) {
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	orchLabels := make(map[string]string)
	orchLabels[orchutils.OrchNameKey] = "test-orchestrator"

	dscprof := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: TransparentProfile,
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        cluster.DSCProfileSpec_TRANSPARENT.String(),
			FlowPolicyMode: cluster.DSCProfileSpec_BASENET.String(),
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

	eventRecorder.ClearEvents()
	err = stateMgr.ctrler.Workload().Create(&wr)
	AssertOk(t, err, "Could not create the workload")

	AssertEventually(t, func() (bool, interface{}) {
		for _, evt := range eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.DSC_NOT_ADMITTED.String() {
				return true, nil
			}
		}

		return false, nil
	}, "Failed to find mode conflict  event", "1s", "10s")
}

func TestMigrationWorkloadCreationEventDSCProfileChanged(t *testing.T) {
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	orchLabels := make(map[string]string)
	orchLabels[orchutils.OrchNameKey] = "test-orchestrator"
	// create DSC profile
	dscprof := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: InsertionProfile,
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        cluster.DSCProfileSpec_INSERTION.String(),
			FlowPolicyMode: cluster.DSCProfileSpec_ENFORCED.String(),
		},
	}

	err = stateMgr.ctrler.DSCProfile().Create(&dscprof)
	AssertOk(t, err, "Could not create the smartNic profile")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindDSCProfile("", InsertionProfile)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Did not find DSCProfile", "1ms", "2s")

	dscprof = cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: TransparentProfile,
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        cluster.DSCProfileSpec_TRANSPARENT.String(),
			FlowPolicyMode: cluster.DSCProfileSpec_BASENET.String(),
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

	eventRecorder.ClearEvents()

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
			if evt.EventType == eventtypes.HOST_DSC_MODE_INCOMPATIBLE.String() {
				return true, nil
			}
		}

		return false, nil
	}, "Failed to find mode conflict  event", "1s", "10s")
}

func TestMigrationWorkloadHostUpdateDSCCheck(t *testing.T) {
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	orchLabels := make(map[string]string)
	orchLabels[orchutils.OrchNameKey] = "test-orchestrator"
	// create DSC profile
	dscprof := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: InsertionProfile,
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        cluster.DSCProfileSpec_INSERTION.String(),
			FlowPolicyMode: cluster.DSCProfileSpec_ENFORCED.String(),
		},
	}

	err = stateMgr.ctrler.DSCProfile().Create(&dscprof)
	AssertOk(t, err, "Could not create the smartNic profile")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindDSCProfile("", InsertionProfile)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Did not find DSCProfile", "1ms", "2s")

	dscprof = cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: TransparentProfile,
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        cluster.DSCProfileSpec_TRANSPARENT.String(),
			FlowPolicyMode: cluster.DSCProfileSpec_BASENET.String(),
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

func TestMigrationWorkloadHostDeleteDSCCheck(t *testing.T) {
	stateMgr, err := newStatemgr()
	if err != nil {
		t.Fatalf("Could not create network manager. Err: %v", err)
		return
	}

	orchLabels := make(map[string]string)
	orchLabels[orchutils.OrchNameKey] = "test-orchestrator"
	// create DSC profile
	dscprof := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: InsertionProfile,
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        cluster.DSCProfileSpec_INSERTION.String(),
			FlowPolicyMode: cluster.DSCProfileSpec_ENFORCED.String(),
		},
	}

	err = stateMgr.ctrler.DSCProfile().Create(&dscprof)
	AssertOk(t, err, "Could not create the smartNic profile")

	AssertEventually(t, func() (bool, interface{}) {
		_, err := stateMgr.FindDSCProfile("", InsertionProfile)
		if err == nil {
			return true, nil
		}
		return false, nil
	}, "Did not find DSCProfile", "1ms", "2s")

	dscprof = cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: TransparentProfile,
		},
		Spec: cluster.DSCProfileSpec{
			FwdMode:        cluster.DSCProfileSpec_TRANSPARENT.String(),
			FlowPolicyMode: cluster.DSCProfileSpec_BASENET.String(),
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
