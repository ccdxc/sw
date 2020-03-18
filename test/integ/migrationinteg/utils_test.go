package migrationinteg

import (
	"context"
	"fmt"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

func (it *migrationTestSuite) createTenant(tenantName string) error {
	tenant := &cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
		ObjectMeta: api.ObjectMeta{
			Name: tenantName,
		},
	}

	_, err := it.apiCl.ClusterV1().Tenant().Create(context.Background(), tenant)
	return err
}

// NETWORK APIs
func (it *migrationTestSuite) createNetwork(name string, vlan int, orchInfo []*network.OrchestratorInfo) (*network.Network, error) {
	nw := &network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
		Spec: network.NetworkSpec{
			Type:          network.NetworkType_Bridged.String(),
			VlanID:        uint32(vlan),
			Orchestrators: orchInfo,
		},
		Status: network.NetworkStatus{},
	}

	nw, err := it.apiCl.NetworkV1().Network().Create(context.Background(), nw)
	return nw, err
}

func (it *migrationTestSuite) deleteNetwork(name string) error {
	nwMeta := &api.ObjectMeta{
		Name:      name,
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}

	_, err := it.apiCl.NetworkV1().Network().Delete(context.Background(), nwMeta)
	return err
}

// HOST APIs
func (it *migrationTestSuite) createHost(name, dscMac, ipAddress string) (*cluster.Host, error) {
	dsc := &cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: dscMac,
		},
		Spec: cluster.DistributedServiceCardSpec{
			MgmtMode:    "NETWORK",
			NetworkMode: "OOB",
			DSCProfile:  globals.DefaultDSCProfile,
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: "ADMITTED",
			PrimaryMAC:     dscMac,
			Conditions: []cluster.DSCCondition{
				{
					Type:   "HEALTHY",
					Status: cluster.ConditionStatus_TRUE.String(),
				},
			},
			IPConfig: &cluster.IPConfig{
				IPAddress: ipAddress,
			},
		},
	}

	_, err := it.apiCl.ClusterV1().DistributedServiceCard().Create(context.Background(), dsc)
	if err != nil {
		return nil, err
	}

	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: dscMac,
				},
			},
		},
		Status: cluster.HostStatus{},
	}

	h, err := it.apiCl.ClusterV1().Host().Create(context.Background(), &host)
	return h, err
}

func (it *migrationTestSuite) deleteHost(name string) error {
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
	}

	h, err := it.apiCl.ClusterV1().Host().Delete(context.Background(), &host.ObjectMeta)
	if err != nil {
		return err
	}

	// Delete the DSC associated with the host
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: h.Spec.DSCs[0].MACAddress,
		},
		Spec: cluster.DistributedServiceCardSpec{
			Admit:       false,
			MgmtMode:    cluster.DistributedServiceCardSpec_NETWORK.String(),
			NetworkMode: cluster.DistributedServiceCardSpec_OOB.String(),
			DSCProfile:  globals.DefaultDSCProfile,
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: cluster.DistributedServiceCardStatus_PENDING.String(),
		},
	}

	_, err = it.apiCl.ClusterV1().DistributedServiceCard().Update(context.Background(), &snic)
	if err != nil {
		log.Errorf("Failed to de-admit the DSC %v. Err : %v", snic, err)
		return err
	}

	_, err = it.apiCl.ClusterV1().DistributedServiceCard().Delete(context.Background(), &snic.ObjectMeta)
	return err
}

// WORKLOAD API
func getInterfaceSpec(macAddr, networkName string, microSeg int) workload.WorkloadIntfSpec {
	return workload.WorkloadIntfSpec{
		MACAddress:   macAddr,
		MicroSegVlan: uint32(microSeg),
		Network:      networkName,
	}
}

func (it *migrationTestSuite) createWorkload(workloadName, hostName string, ifs []workload.WorkloadIntfSpec) (*workload.Workload, error) {
	ws := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      workloadName,
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
		Spec: workload.WorkloadSpec{
			HostName:         hostName,
			Interfaces:       ifs,
			MigrationTimeout: migrationTimeout, // Set the default migration timeout as 15s
		},
	}

	w, err := it.apiCl.WorkloadV1().Workload().Create(context.Background(), &ws)
	return w, err
}

func (it *migrationTestSuite) deleteWorkload(workloadName string) error {
	ws := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      workloadName,
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
		},
	}

	_, err := it.apiCl.WorkloadV1().Workload().Delete(context.Background(), &ws.ObjectMeta)
	return err
}

func (it *migrationTestSuite) startWorkloadMigration(nwrk *workload.Workload) error {
	_, err := it.apiCl.WorkloadV1().Workload().StartMigration(context.Background(), nwrk)
	return err
}

func (it *migrationTestSuite) finishWorkloadMigration(nwrk *workload.Workload) error {
	_, err := it.apiCl.WorkloadV1().Workload().FinishMigration(context.Background(), nwrk)
	return err
}

func (it *migrationTestSuite) syncWorkloadMigration(nwrk *workload.Workload) error {
	_, err := it.apiCl.WorkloadV1().Workload().FinalSyncMigration(context.Background(), nwrk)
	return err
}

func (it *migrationTestSuite) abortWorkloadMigration(nwrk *workload.Workload) error {
	_, err := it.apiCl.WorkloadV1().Workload().AbortMigration(context.Background(), nwrk)
	return err
}

func (it *migrationTestSuite) checkWorkloadMigrationStage(w workload.Workload, expectedStage string) (err error) {
	var wrk *workload.Workload
	ctx := context.Background()

	for i := 0; i < 10; i++ {
		wrk, err = it.apiCl.WorkloadV1().Workload().Get(ctx, &w.ObjectMeta)
		if err != nil || wrk == nil {
			err = fmt.Errorf("Could not find workload [%v]", w.Name)
			continue
		}

		if wrk.Status.MigrationStatus.Stage == expectedStage {
			return nil
		}

		err = fmt.Errorf("Workload [%v] expected stage [%v] got [%v]", w, expectedStage, wrk.Status.MigrationStatus.Stage)
		time.Sleep(time.Second)
	}

	return err
}

func (it *migrationTestSuite) checkWorkloadMigrationStatus(w workload.Workload, expectedStatus string) (err error) {
	var wrk *workload.Workload
	ctx := context.Background()

	for i := 0; i < 10; i++ {
		wrk, err = it.apiCl.WorkloadV1().Workload().Get(ctx, &w.ObjectMeta)
		if err != nil || wrk == nil {
			err = fmt.Errorf("Could not find workload [%v]", w.Name)
			continue
		}

		if wrk.Status.MigrationStatus.Status == expectedStatus {
			return nil
		}

		err = fmt.Errorf("Workload [%v] expected status [%v] got [%v]", w, expectedStatus, wrk.Status.MigrationStatus.Status)
		time.Sleep(time.Second)
	}

	return err
}

// UTILITIES
func (it *migrationTestSuite) deleteAllObjects() error {
	opts := &api.ListWatchOptions{}

	// Delete Workloads
	wrks, err := it.apiCl.WorkloadV1().Workload().List(context.Background(), opts)
	if err != nil {
		log.Errorf("Failed to get list of all workloads")
	}

	log.Infof("ALL WORKLOADS : %v", wrks)

	for _, w := range wrks {
		err := it.deleteWorkload(w.Name)
		if err != nil {
			log.Errorf("Failed to delete workload %v. Err : %v", w.Name, err)
		}
	}

	// Delete Hosts
	hosts, err := it.apiCl.ClusterV1().Host().List(context.Background(), opts)
	if err == nil {
		log.Errorf("Failed to get host list. Err : %v", err)
	}

	log.Infof("ALL HOSTS : %v", hosts)

	for _, host := range hosts {
		err := it.deleteHost(host.Name)
		if err != nil {
			log.Errorf("failed to delete host %v. Err : %v", host.Name, err)
		}
	}

	// Delete Networks
	nws, err := it.apiCl.NetworkV1().Network().List(context.Background(), opts)
	if err == nil {
		log.Errorf("Failed to get network list. Err : %v", err)
	}

	log.Infof("ALL NETWORKS : %v", nws)

	for _, nw := range nws {
		err := it.deleteNetwork(nw.Name)
		if err != nil {
			log.Errorf("failed to delete network %v. Err : %v", nw.Name, err)
		}
	}

	log.Infof("Workloads, Hosts and Networks successfully deleted from API server.")
	return nil
}
