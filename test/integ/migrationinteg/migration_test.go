package migrationinteg

import (
	"context"
	"fmt"
	"os"
	"strings"
	"time"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/workload"
	fakehal "github.com/pensando/sw/nic/agent/cmd/fakehal/hal"
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/venice/ctrler/npm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/featureflags"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/ref"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// Default test parameters
const (
	networkName      = "Migration-Vlan-1000"
	networkVlan      = 1000
	hostCount        = 3
	workloadPerHost  = 1
	ifsPerWorkload   = 1
	timeout          = 30
	migrationTimeout = "30s"
)

func (it *migrationTestSuite) setupDefaultTopo(seed int, c *C) ([]*cluster.Host, map[string][]*workload.Workload, error) {
	hosts := []*cluster.Host{}
	var hostWorkloadMap map[string][]*workload.Workload
	hostWorkloadMap = make(map[string][]*workload.Workload)

	it.createTenant("default")

	_, err := it.createNetwork(networkName, networkVlan, nil)
	AssertOk(c, err, "error creating network")

	for i := 0; i < hostCount; i++ {
		hostName := fmt.Sprintf("Host-%d-%d", seed, i)
		dscMac := fmt.Sprintf("00ae.cd0%d.000%x", seed, i+2)
		dscIP := fmt.Sprintf("10.20.30.%d/16", i+2)
		h, err := it.createHost(hostName, dscMac, dscIP)
		AssertOk(c, err, "error creating host")
		hosts = append(hosts, h)

		for j := 0; j < workloadPerHost; j++ {
			ifs := []workload.WorkloadIntfSpec{}
			for k := 0; k < ifsPerWorkload; k++ {
				ifMac := fmt.Sprintf("000%x.000%x.000%x", i, j, k)
				// VLAN value should at least be 1
				usegVlan := i*100 + j*10 + k + 1
				ifs = append(ifs, getInterfaceSpec(ifMac, networkName, usegVlan))
			}
			wName := fmt.Sprintf("workload-%d-%d-%d", seed, i, j)

			w, err := it.createWorkload(wName, hostName, ifs)
			AssertOk(c, err, "error creating workload")

			hostWorkloadMap[hostName] = append(hostWorkloadMap[hostName], w)
		}
	}

	AssertEventually(c, func() (bool, interface{}) {
		opts := &api.ListWatchOptions{}
		wrks, err := it.apiCl.WorkloadV1().Workload().List(context.Background(), opts)
		if err != nil || len(wrks) != hostCount*workloadPerHost {
			return false, nil
		}

		return true, nil
	}, fmt.Sprintf("failed to create %v workloads", hostCount*workloadPerHost), "1s", "5s")

	return hosts, hostWorkloadMap, nil
}

func (it *migrationTestSuite) setupDestinationHost(hostName, dscMac, dscIP string, c *C) *cluster.Host {
	h, err := it.createHost(hostName, dscMac, dscIP)
	AssertOk(c, err, "error creating host")

	log.Infof("Created Destination host : %v", h)
	it.hub = gosdk.NewFakeHub()
	it.hub.Start()

	var halLis netutils.TestListenAddr
	halLis.GetAvailablePort()
	it.fakehal = fakehal.NewFakeHalServer(halLis.ListenURL.String())
	if err := os.Setenv("HAL_GRPC_PORT", strings.Split(halLis.ListenURL.String(), ":")[1]); err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	os.Remove(globals.NetAgentDBPath)

	it.agent, err = createAgent(it.logger, it.resolverSrv.GetListenURL(), dscMac)
	c.Assert(err, IsNil)

	AssertEventually(c, func() (bool, interface{}) {
		return it.agent.dscAgent.InfraAPI.GetConfig().IsConnectedToVenice, nil
	}, "default agent is not connected to NPM", "1s", "10s")

	time.Sleep(2 * time.Second)
	log.Infof("Destination agent successfully created.")

	return h
}

func (it *migrationTestSuite) deleteDestinationHost() {
	it.hub.Stop()
	it.agent.dscAgent.Stop()
	it.fakehal.Stop()
	time.Sleep(time.Millisecond * 100)
	os.Remove(globals.NetAgentDBPath)
	log.Infof("Successfully stopped destination host")
}

func (it *migrationTestSuite) TestMigrationTimeout(c *C) {
	hosts, hostWorkloadMap, err := it.setupDefaultTopo(1, c)
	AssertOk(c, err, "failed to setup default topology")

	// Trigger Migration for all Workloads from one host to other
	sourceHost := hosts[0].Name
	destHost := hosts[1].Name

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		nwrk.Spec.HostName = destHost

		// Optionally update the useg
		err := it.startWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to start workload migration")

		// Ensure the workload moves to migration-start state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Ensure NPM has started migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
		AssertOk(c, err, "workload migration did not move to started status")
	}

	// Wait till the timeout expires for all Workloads
	time.Sleep((timeout + 5) * time.Second)

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		// Ensure the workload moves to migration-abort state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Ensure NPM has timedout migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_TIMED_OUT.String())
		AssertOk(c, err, "workload migration did not move to timedout status")
	}
}

func (it *migrationTestSuite) TestMigrationStartSyncFinish(c *C) {
	c.Skip("Skipping this test till we debug the intermittent failure")
	hosts, hostWorkloadMap, err := it.setupDefaultTopo(2, c)
	AssertOk(c, err, "failed to setup default topology")

	// Trigger Migration for all Workloads from one host to other
	sourceHost := hosts[0].Name
	destHost := hosts[1].Name

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		nwrk.Spec.HostName = destHost
		// Optionally update the useg
		err := it.startWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to start workload migration")

		// Ensure the workload moves to migration-start state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Ensure NPM has started migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
		AssertOk(c, err, "workload migration did not move to started status")
	}

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		//nwrk.Spec.HostName = destHost
		// Optionally update the useg
		err := it.syncWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to start final sync for workload")

		// Ensure the workload moves to migration-abort state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_FINAL_SYNC.String())
		AssertOk(c, err, "workload migration did not move to final sync stage")

		// Ensure NPM has started migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
		AssertOk(c, err, "workload migration status changed from started")
	}

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		//nwrk.Spec.HostName = destHost
		// Optionally update the useg
		err := it.finishWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to finish workload migration")

		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_DONE.String())
		AssertOk(c, err, "workload migration did not move to migration-done stage")
	}
	time.Sleep(20 * time.Second)

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		// Ensure the workload moves to migration-abort state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_DONE.String())
		AssertOk(c, err, "workload migration did not move to migration-done stage")

		// Ensure NPM has failed migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_DONE.String())
		AssertOk(c, err, "workload migration did not move to done status")
	}
}

func (it *migrationTestSuite) TestMigrationAbort(c *C) {
	hosts, hostWorkloadMap, err := it.setupDefaultTopo(3, c)
	AssertOk(c, err, "failed to setup default topology")

	// Trigger Migration for all Workloads from one host to other
	sourceHost := hosts[0].Name
	destHost := hosts[1].Name

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		nwrk.Spec.HostName = destHost
		// Optionally update the useg
		err := it.startWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to start workload migration")

		// Ensure the workload moves to migration-start state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Ensure NPM has started migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
		AssertOk(c, err, "workload migration did not move to started status")
	}

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		//nwrk.Spec.HostName = destHost
		// Optionally update the useg
		err := it.abortWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to finish workload migration")

		// Ensure the workload moves to migration-abort state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_ABORT.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Ensure NPM has failed migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_FAILED.String())
		AssertOk(c, err, "workload migration did not move to started status")
	}
}

func (it *migrationTestSuite) TestMigrationNPMRestartAfterTimeout(c *C) {
	c.Skip("Skipping this test till we debug the intermittent failure")
	hosts, hostWorkloadMap, err := it.setupDefaultTopo(4, c)
	AssertOk(c, err, "failed to setup default topology")

	// Trigger Migration for all Workloads from one host to other
	sourceHost := hosts[0].Name
	destHost := hosts[1].Name

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		nwrk.Spec.HostName = destHost

		// Optionally update the useg
		err := it.startWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to start workload migration")

		// Ensure the workload moves to migration-start state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Ensure NPM has started migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
		AssertOk(c, err, "workload migration did not move to started status")
	}

	it.npmCtrler.Stop()
	it.logger.Info("NPM stopped")
	// Wait till the timeout expires for all Workloads
	time.Sleep(timeout * time.Second)

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		// Ensure the workload moves to migration-abort state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Workload should continue to say that migration has STARTED
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
		AssertOk(c, err, "workload migration did not move to started status")
	}

	log.Infof("Restarting NPM")
	featureflags.SetInitialized()
	it.npmCtrler, err = npm.NewNetctrler(integTestNpmRPCURL, integTestRESTURL, integTestApisrvURL, it.resolverClient, it.logger.WithContext("submodule", "pen-npm"), false)
	AssertOk(c, err, "failed to start NPM")
	time.Sleep(time.Second)

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		// Ensure the workload moves to migration-abort state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Ensure NPM has timedout migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_TIMED_OUT.String())
		AssertOk(c, err, "workload migration did not move to timedout status")
	}
}

func (it *migrationTestSuite) TestMigrationNPMRestartBeforeTimeout(c *C) {
	c.Skip("Skipping this test till we debug the intermittent failure")
	hosts, hostWorkloadMap, err := it.setupDefaultTopo(6, c)
	AssertOk(c, err, "failed to setup default topology")

	// Trigger Migration for all Workloads from one host to other
	sourceHost := hosts[0].Name
	destHost := hosts[1].Name

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		nwrk.Spec.HostName = destHost

		// Optionally update the useg
		err := it.startWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to start workload migration")

		// Ensure the workload moves to migration-start state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Ensure NPM has started migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
		AssertOk(c, err, "workload migration did not move to started status")
	}

	err = it.restartNpm()
	AssertOk(c, err, "failed to restart NPM")

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		// Ensure the workload moves to migration-abort state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Workload should continue to say that migration has STARTED
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
		AssertOk(c, err, "workload migration did not move to started status")
	}

	it.npmCtrler.Stop()
	log.Infof("NPM stopped")
	time.Sleep(time.Millisecond * 100)

	// Start Last Sync
	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		//nwrk.Spec.HostName = destHost
		// Optionally update the useg
		err := it.syncWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to start final sync for workload")

		// Ensure the workload moves to migration-abort state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_FINAL_SYNC.String())
		AssertOk(c, err, "workload migration did not move to final sync stage")

		// Ensure NPM has started migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
		AssertOk(c, err, "workload migration status changed from started")
	}

	featureflags.SetInitialized()
	it.npmCtrler, err = npm.NewNetctrler(integTestNpmRPCURL, integTestRESTURL, integTestApisrvURL, it.resolverClient, it.logger.WithContext("submodule", "pen-npm"), false)
	AssertOk(c, err, "failed to start NPM")
	time.Sleep(time.Second)

	// NPM should resume from where the Migration stopped
	// Wait for NPM to finish its work
	time.Sleep(5 * time.Second)

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		//nwrk.Spec.HostName = destHost
		// Optionally update the useg
		err := it.finishWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to finish workload migration")

		// Ensure the workload moves to migration-abort state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_DONE.String())
		AssertOk(c, err, "workload migration did not move to migration-done stage")
	}
}

func (it *migrationTestSuite) TestMigrationStartMultipleHosts(c *C) {
	hosts, hostWorkloadMap, err := it.setupDefaultTopo(7, c)
	AssertOk(c, err, "failed to setup default topology")

	destinationHost := it.setupDestinationHost("Destination-Host", "00ae.cd01.1000", "10.20.30.199/16", c)

	for _, h := range hosts {
		for _, w := range hostWorkloadMap[h.Name] {
			nwrk := *(ref.DeepCopy(w).(*workload.Workload))
			nwrk.Spec.HostName = destinationHost.Name
			// Optionally update the useg
			err := it.startWorkloadMigration(&nwrk)
			AssertOk(c, err, "failed to start workload migration")

			// Ensure the workload moves to migration-start state
			err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
			AssertOk(c, err, "workload migration did not move to migration-start stage")

			// Ensure NPM has started migration
			err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
			AssertOk(c, err, "workload migration did not move to started status")
		}
	}

	it.deleteDestinationHost()
}

func (it *migrationTestSuite) TestMigrationAbortAfterTimeout(c *C) {
	hosts, hostWorkloadMap, err := it.setupDefaultTopo(3, c)
	AssertOk(c, err, "failed to setup default topology")

	// Trigger Migration for all Workloads from one host to other
	sourceHost := hosts[0].Name
	destHost := hosts[1].Name

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		nwrk.Spec.HostName = destHost
		// Optionally update the useg
		err := it.startWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to start workload migration")

		// Ensure the workload moves to migration-start state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Ensure NPM has started migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
		AssertOk(c, err, "workload migration did not move to started status")
	}

	for _, w := range hostWorkloadMap[sourceHost] {
		AssertEventually(c, func() (bool, interface{}) {
			wrk, err := it.apiCl.WorkloadV1().Workload().Get(context.Background(), &w.ObjectMeta)
			if err != nil || wrk == nil {
				return false, nil
			}

			if wrk.Status.MigrationStatus.Status == workload.WorkloadMigrationStatus_TIMED_OUT.String() {
				return true, nil
			}

			return false, nil
		}, fmt.Sprintf("Workload [%v] did not reach timeout status", w.Name), "1s", migrationTimeout)
	}

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		//nwrk.Spec.HostName = destHost
		// Optionally update the useg
		err := it.abortWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to finish workload migration")

		// Ensure the workload moves to migration-abort state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_ABORT.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Workload's migration status shoud continue to say timed out
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_TIMED_OUT.String())
		AssertOk(c, err, "workload migration did not move to started status")
	}
}

func (it *migrationTestSuite) TestMigrationLastSyncAfterTimeout(c *C) {
	hosts, hostWorkloadMap, err := it.setupDefaultTopo(3, c)
	AssertOk(c, err, "failed to setup default topology")

	// Trigger Migration for all Workloads from one host to other
	sourceHost := hosts[0].Name
	destHost := hosts[1].Name

	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		nwrk.Spec.HostName = destHost
		// Optionally update the useg
		err := it.startWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to start workload migration")

		// Ensure the workload moves to migration-start state
		err = it.checkWorkloadMigrationStage(nwrk, workload.WorkloadMigrationStatus_MIGRATION_START.String())
		AssertOk(c, err, "workload migration did not move to migration-start stage")

		// Ensure NPM has started migration
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_STARTED.String())
		AssertOk(c, err, "workload migration did not move to started status")
	}

	for _, w := range hostWorkloadMap[sourceHost] {
		AssertEventually(c, func() (bool, interface{}) {
			wrk, err := it.apiCl.WorkloadV1().Workload().Get(context.Background(), &w.ObjectMeta)
			if err != nil || wrk == nil {
				return false, nil
			}

			if wrk.Status.MigrationStatus.Status == workload.WorkloadMigrationStatus_TIMED_OUT.String() {
				return true, nil
			}

			return false, nil
		}, fmt.Sprintf("Workload [%v] did not reach timeout status", w.Name), "1s", migrationTimeout)
	}

	// Start Last Sync
	for _, w := range hostWorkloadMap[sourceHost] {
		nwrk := *(ref.DeepCopy(w).(*workload.Workload))
		//nwrk.Spec.HostName = destHost
		// Optionally update the useg
		err := it.syncWorkloadMigration(&nwrk)
		AssertOk(c, err, "failed to start final sync for workload")

		// Workload migration status should continue to stay as TIMED_OUT
		err = it.checkWorkloadMigrationStatus(nwrk, workload.WorkloadMigrationStatus_TIMED_OUT.String())
		AssertOk(c, err, "workload migration status changed from started")
	}
}
