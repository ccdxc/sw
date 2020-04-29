package statemgr

import (
	"context"
	"fmt"
	"os"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

var (
	logger log.Logger
)

func (w *MockInstanceManager) watchOrchestratorConfig() {
	for {
		select {
		case <-w.watchCtx.Done():
			log.Info("Exiting watch for orchestration configuration")
			return
		case evt, ok := <-w.instanceManagerCh:
			if ok {
				log.Infof("Instance manager got event. %v", evt)
			}
		}
	}
}

func TestNetworkWatcher(t *testing.T) {
	config := log.GetDefaultConfig("test-network")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)
	sm, im, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed creating state manager. Err : %v", err)
		return
	}

	if im == nil {
		t.Fatalf("Failed to create instance manger.")
		return
	}

	orchName := "TestOrch"
	probeChl := make(chan *kvstore.WatchEvent, 10)
	sm.AddProbeChannel(orchName, probeChl)

	orch := GetOrchestratorConfig(orchName, "user", "pass")
	orchInfo := []*network.OrchestratorInfo{
		{
			Name:      orch.Name,
			Namespace: orch.Namespace,
		},
	}

	expNets := map[string]bool{}
	_, err = CreateNetwork(sm, "default", "prod-beef-vlan100", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo)
	Assert(t, (err == nil), "network could not be created")
	expNets["prod-beef-vlan100"] = true

	_, err = CreateNetwork(sm, "default", "prod-bebe-vlan200", "10.2.1.0/24", "10.2.1.1", 200, nil, orchInfo)
	Assert(t, (err == nil), "network could not be created")
	expNets["prod-bebe-vlan200"] = true

	np := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      "duplicate_200",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: network.NetworkSpec{
			Type:          network.NetworkType_Bridged.String(),
			VlanID:        200,
			Orchestrators: orchInfo,
		},
		Status: network.NetworkStatus{
			OperState: network.OperState_Rejected.String(),
		},
	}
	// create a network
	err = sm.ctrler.Network().Create(&np)
	Assert(t, (err == nil), "network could not be created")

	_, err = CreateNetwork(sm, "default", "prod-cece-vlan300", "10.3.1.0/24", "10.3.1.1", 300, nil, orchInfo)
	Assert(t, (err == nil), "network could not be created")
	expNets["prod-cece-vlan300"] = true

	receiveNetworks := func(pChl chan *kvstore.WatchEvent, rcvNets map[string]bool) {
		for {
			msg, ok := <-pChl
			if !ok {
				logger.Infof("probe chl closed")
				return
			}
			logger.Infof("Received net %s on probe chl", msg.Object.(*network.Network).Name)
			rcvNets[msg.Object.(*network.Network).Name] = true
		}
	}
	defer close(probeChl)
	rcvNets := map[string]bool{}
	go receiveNetworks(probeChl, rcvNets)

	AssertEventually(t, func() (bool, interface{}) {
		for net := range expNets {
			if _, found := rcvNets[net]; !found {
				return false, fmt.Errorf("Net %s not found", net)
			}
		}
		for net := range rcvNets {
			if _, found := expNets[net]; !found {
				return false, fmt.Errorf("Net %s not enxpected", net)
			}
		}
		return true, nil
	}, "All networks were not received", "1s", "10s")

	err = DeleteNetwork(sm, &np)
	Assert(t, err == nil, "delete network failed")

	// Increase test coverage
	sm.OnNetworkReconnect()
	return
}

func TestClusterCreateList(t *testing.T) {

	sm, im, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed creating state manager. Err : %v", err)
		return
	}

	if im == nil {
		t.Fatalf("Failed to create instance manger.")
		return
	}

	go im.watchOrchestratorConfig()
	defer im.watchCancel()

	clusterConfig := &cluster.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Cluster",
		},
		Spec: cluster.ClusterSpec{
			AutoAdmitDSCs: true,
		},
	}

	err = sm.Controller().Cluster().Create(clusterConfig)
	AssertOk(t, err, "failed to create cluster config")

	err = sm.Controller().Cluster().Update(clusterConfig)
	AssertOk(t, err, "failed to create cluster config")

	// Increase test coverage
	sm.OnClusterReconnect()
}

func TestNetworkCreateList(t *testing.T) {
	sm, im, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed creating state manager. Err : %v", err)
		return
	}

	if im == nil {
		t.Fatalf("Failed to create instance manger.")
		return
	}

	go im.watchOrchestratorConfig()
	defer im.watchCancel()

	orch := GetOrchestratorConfig("myorchestrator", "user", "pass")
	orchInfo := []*network.OrchestratorInfo{
		{
			Name:      orch.Name,
			Namespace: orch.Namespace,
		},
	}

	clusterConfig := &cluster.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Cluster",
		},
		Spec: cluster.ClusterSpec{
			AutoAdmitDSCs: true,
		},
	}

	err = sm.Controller().Cluster().Create(clusterConfig)
	AssertOk(t, err, "failed to create cluster config")

	_, err = CreateNetwork(sm, "default", "prod-beef-vlan100", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo)
	Assert(t, (err == nil), "network could not be created")

	_, err = CreateNetwork(sm, "default", "prod-bebe-vlan200", "10.2.1.0/24", "10.2.1.1", 200, nil, orchInfo)
	Assert(t, (err == nil), "network could not be created")

	labels := map[string]string{"color": "green"}
	_, err = CreateNetwork(sm, "default", "dev-caca-vlan300", "10.3.1.0/24", "10.3.1.1", 300, labels, orchInfo)
	Assert(t, (err == nil), "network could not be created")

	nw, err := sm.ctrler.Network().List(context.Background(), &api.ListWatchOptions{})
	AssertEquals(t, 3, len(nw), "did not find all networks")

	opts := api.ListWatchOptions{}
	opts.LabelSelector = "color=green"
	nw, err = sm.ctrler.Network().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "found more networks than expected. [%v]", len(nw))

	meta := api.ObjectMeta{
		Name:      "prod-bebe-vlan200",
		Namespace: "default",
		Tenant:    "default",
	}

	nobj, err := sm.Controller().Network().Find(&meta)
	Assert(t, err == nil, "did not find the network")

	nobj.Network.ObjectMeta.Labels = labels
	err = sm.Controller().Network().Update(&nobj.Network)
	Assert(t, err == nil, "unable to update the network object")

	nw, err = sm.Controller().Network().List(context.Background(), &opts)
	Assert(t, len(nw) == 2, "expected 2 networks found [%v]", len(nw))

	err = sm.Controller().Network().Delete(&nobj.Network)
	Assert(t, err == nil, "deletion was not successful")

	nw, err = sm.ctrler.Network().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "expected 1, got %v networks", len(nw))

	clusterItems, err := sm.Controller().Cluster().List(context.Background(), &api.ListWatchOptions{})
	Assert(t, len(clusterItems) == 1, "failed to get cluster config")

	err = sm.Controller().Cluster().Delete(clusterConfig)
	AssertOk(t, err, "failed to create cluster config")

	return
}

// createWorkload utility function to create a Workload
func createWorkload(stateMgr *Statemgr, tenant, name string, labels map[string]string) error {
	// Workload params
	np := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    "default",
			Labels:    labels,
		},
		Spec:   workload.WorkloadSpec{},
		Status: workload.WorkloadStatus{},
	}

	// create a Workload
	return stateMgr.ctrler.Workload().Create(&np)
}

func TestWorkloadCreateList(t *testing.T) {
	sm, im, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed creating state manager. Err : %v", err)
		return
	}

	if im == nil {
		t.Fatalf("Failed to create instance manger.")
		return
	}

	go im.watchOrchestratorConfig()
	defer im.watchCancel()

	err = createWorkload(sm, "default", "prod-beef", nil)
	Assert(t, (err == nil), "Workload could not be created")

	err = createWorkload(sm, "default", "prod-bebe", nil)
	Assert(t, (err == nil), "Workload could not be created")

	labels := map[string]string{"color": "green"}
	err = createWorkload(sm, "default", "dev-caca", labels)
	Assert(t, (err == nil), "Workload could not be created")

	nw, err := sm.ctrler.Workload().List(context.Background(), &api.ListWatchOptions{})
	AssertEquals(t, 3, len(nw), "did not find all Workloads")

	opts := api.ListWatchOptions{}
	opts.LabelSelector = "color=green"
	nw, err = sm.ctrler.Workload().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "found more Workloads than expected. [%v]", len(nw))

	meta := api.ObjectMeta{
		Name:      "prod-bebe",
		Namespace: "default",
		Tenant:    "default",
	}

	nobj, err := sm.Controller().Workload().Find(&meta)
	Assert(t, err == nil, "did not find the Workload")

	nobj.Workload.ObjectMeta.Labels = labels
	err = sm.Controller().Workload().Update(&nobj.Workload)
	Assert(t, err == nil, "unable to update the Workload object")

	nw, err = sm.Controller().Workload().List(context.Background(), &opts)
	Assert(t, len(nw) == 2, "expected 2 Workloads found [%v]", len(nw))

	err = sm.Controller().Workload().Delete(&nobj.Workload)
	Assert(t, err == nil, "deletion was not successful")

	nw, err = sm.ctrler.Workload().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "expected 1, got %v Workloads", len(nw))

	sm.OnWorkloadReconnect()
	select {
	case kind := <-sm.ctkitReconnectCh:
		AssertEquals(t, "Workload", kind, "Wrong kind sent over channel")
	default:
		t.Fatalf("Channel had no event")
	}
	return
}

// createHost utility function to create a Host
func createHost(stateMgr *Statemgr, tenant, name string, labels map[string]string) error {
	// Host params
	np := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    tenant,
			Labels:    labels,
		},
		Spec:   cluster.HostSpec{},
		Status: cluster.HostStatus{},
	}

	// create a Host
	return stateMgr.ctrler.Host().Create(&np)
}

func TestHostCreateList(t *testing.T) {
	sm, im, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed creating state manager. Err : %v", err)
		return
	}

	if im == nil {
		t.Fatalf("Failed to create instance manger.")
		return
	}

	go im.watchOrchestratorConfig()
	defer im.watchCancel()

	prodMap := make(map[string]string)
	prodMap[utils.OrchNameKey] = "prod"
	prodMap[utils.NamespaceKey] = "dev"

	err = createHost(sm, "default", "prod-beef", prodMap)
	Assert(t, (err == nil), "Host could not be created")

	err = createHost(sm, "default", "prod-bebe", prodMap)
	Assert(t, (err == nil), "Host could not be created")

	labels := map[string]string{"color": "green"}
	err = createHost(sm, "default", "dev-caca", labels)
	Assert(t, (err == nil), "Host could not be created")

	nw, err := sm.ctrler.Host().List(context.Background(), &api.ListWatchOptions{})
	AssertEquals(t, 3, len(nw), "did not find all Hosts")

	opts := api.ListWatchOptions{}
	opts.LabelSelector = "color=green"
	nw, err = sm.ctrler.Host().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "found more Hosts than expected. [%v]", len(nw))

	meta := api.ObjectMeta{
		Name:      "prod-bebe",
		Namespace: "default",
	}

	nobj, err := sm.Controller().Host().Find(&meta)
	Assert(t, err == nil, "did not find the Host")

	nobj.Host.ObjectMeta.Labels = labels
	err = sm.Controller().Host().Update(&nobj.Host)
	Assert(t, err == nil, "unable to update the Host object")

	nw, err = sm.Controller().Host().List(context.Background(), &opts)
	Assert(t, len(nw) == 2, "expected 2 Hosts found [%v]", len(nw))

	err = sm.Controller().Host().Delete(&nobj.Host)
	Assert(t, err == nil, "deletion was not successful")

	nw, err = sm.ctrler.Host().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "expected 1, got %v Hosts", len(nw))

	nw, err = sm.ctrler.Host().List(context.Background(), &api.ListWatchOptions{})
	Assert(t, len(nw) == 2, "expected 2, got %v Hosts", len(nw))

	err = sm.DeleteHostByNamespace("prod", "dev")
	Assert(t, err == nil, "did not delete the Host")

	nw, err = sm.ctrler.Host().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "expected 1, got %v Hosts", len(nw))

	sm.OnHostReconnect()
	select {
	case kind := <-sm.ctkitReconnectCh:
		AssertEquals(t, "Host", kind, "Wrong kind sent over channel")
	default:
		t.Fatalf("Channel had no event")
	}

	return
}

// createDistributedServiceCard utility function to create a DistributedServiceCard
func createDistributedServiceCard(stateMgr *Statemgr, tenant, name string, labels map[string]string) error {
	// DistributedServiceCard params
	np := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    tenant,
			Labels:    labels,
		},
		Spec:   cluster.DistributedServiceCardSpec{DSCProfile: "default"},
		Status: cluster.DistributedServiceCardStatus{},
	}

	// create a DistributedServiceCard
	return stateMgr.ctrler.DistributedServiceCard().Create(&np)
}

func TestDistributedServiceCardCreateList(t *testing.T) {
	sm, im, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed creating state manager. Err : %v", err)
		return
	}

	if im == nil {
		t.Fatalf("Failed to create instance manger.")
		return
	}

	go im.watchOrchestratorConfig()
	defer im.watchCancel()

	dscProfile := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "default",
			Namespace: "",
			Tenant:    "",
		},
		Spec: cluster.DSCProfileSpec{
			Features: cluster.FeatureSet{
				InterVMServices: true,
				FlowAware:       true,
				Firewall:        true,
			},
		},
	}

	// create DSC Profile
	sm.ctrler.DSCProfile().Create(&dscProfile)
	AssertEventually(t, func() (bool, interface{}) {

		_, err := sm.FindDSCProfile("", "default")
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Profile not foud", "1ms", "1s")

	err = createDistributedServiceCard(sm, "default", "prod-beef", nil)
	Assert(t, (err == nil), "DistributedServiceCard could not be created")

	err = createDistributedServiceCard(sm, "default", "prod-bebe", nil)
	Assert(t, (err == nil), "DistributedServiceCard could not be created")

	labels := map[string]string{"color": "green"}
	err = createDistributedServiceCard(sm, "default", "dev-caca", labels)
	Assert(t, (err == nil), "DistributedServiceCard could not be created")

	nw, err := sm.ctrler.DistributedServiceCard().List(context.Background(), &api.ListWatchOptions{})
	AssertEquals(t, 3, len(nw), "did not find all DistributedServiceCards")

	opts := api.ListWatchOptions{}
	opts.LabelSelector = "color=green"
	nw, err = sm.ctrler.DistributedServiceCard().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "found more DistributedServiceCards than expected. [%v]", len(nw))

	meta := api.ObjectMeta{
		Name:      "prod-bebe",
		Namespace: "default",
		//DSC is cluster object, tenant not required
		//Tenant:    "default",
	}

	nobj, err := sm.Controller().DistributedServiceCard().Find(&meta)
	Assert(t, err == nil, "did not find the DistributedServiceCard")

	nobj.DistributedServiceCard.ObjectMeta.Labels = labels
	err = sm.Controller().DistributedServiceCard().Update(&nobj.DistributedServiceCard)
	Assert(t, err == nil, "unable to update the DistributedServiceCard object")

	nw, err = sm.Controller().DistributedServiceCard().List(context.Background(), &opts)
	Assert(t, len(nw) == 2, "expected 2 DistributedServiceCards found [%v]", len(nw))

	err = sm.Controller().DistributedServiceCard().Delete(&nobj.DistributedServiceCard)
	Assert(t, err == nil, "deletion was not successful")

	nw, err = sm.ctrler.DistributedServiceCard().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "expected 1, got %v DistributedServiceCards", len(nw))

	// Increase test coverage
	sm.OnDistributedServiceCardReconnect()

	return
}

// createOrchestrator utility function to create a Orchestrator
func createOrchestrator(stateMgr *Statemgr, tenant, name string, labels map[string]string) error {
	// Orchestrator params
	np := orchestration.Orchestrator{
		TypeMeta: api.TypeMeta{Kind: "Orchestrator"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    "default",
			Labels:    labels,
		},
		Spec:   orchestration.OrchestratorSpec{},
		Status: orchestration.OrchestratorStatus{},
	}

	// create a Orchestrator
	return stateMgr.ctrler.Orchestrator().Create(&np)
}

func TestOrchestratorCreateList(t *testing.T) {
	sm, im, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed creating state manager. Err : %v", err)
		return
	}

	if im == nil {
		t.Fatalf("Failed to create instance manger.")
		return
	}

	go im.watchOrchestratorConfig()
	defer im.watchCancel()

	err = createOrchestrator(sm, "default", "prod-beef", nil)
	AssertOk(t, err, "Orchestrator could not be created")

	err = createOrchestrator(sm, "default", "prod-bebe", nil)
	AssertOk(t, err, "Orchestrator could not be created")

	labels := map[string]string{"color": "green"}
	err = createOrchestrator(sm, "default", "dev-caca", labels)
	AssertOk(t, err, "Orchestrator could not be created")

	nw, err := sm.ctrler.Orchestrator().List(context.Background(), &api.ListWatchOptions{})
	AssertEquals(t, 3, len(nw), "did not find all Orchestrators")

	opts := api.ListWatchOptions{}
	opts.LabelSelector = "color=green"
	nw, err = sm.ctrler.Orchestrator().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "found more Orchestrators than expected. [%v]", len(nw))

	meta := api.ObjectMeta{
		Name:      "prod-bebe",
		Namespace: "default",
		//Orch is cluster object, tenant not required
		//Tenant: "default",
	}

	nobj, err := sm.Controller().Orchestrator().Find(&meta)
	Assert(t, err == nil, "did not find the Orchestrator")

	nobj.Orchestrator.ObjectMeta.Labels = labels
	err = sm.Controller().Orchestrator().Update(&nobj.Orchestrator)
	Assert(t, err == nil, "unable to update the Orchestrator object")

	nw, err = sm.Controller().Orchestrator().List(context.Background(), &opts)
	Assert(t, len(nw) == 2, "expected 2 Orchestrators found [%v]", len(nw))

	err = sm.Controller().Orchestrator().Delete(&nobj.Orchestrator)
	Assert(t, err == nil, "deletion was not successful")

	nw, err = sm.ctrler.Orchestrator().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "expected 1, got %v Orchestrators", len(nw))

	_, err = sm.GetProbeChannel(nw[0].GetName())
	AssertOk(t, err, "Failed to get probe channel")

	// Increase test coverage
	sm.OnDistributedServiceCardReconnect()

	return
}

func TestUpdateDSCProfile(t *testing.T) {
	sm, _, err := NewMockStateManager()
	if err != nil {
		t.Fatalf("Failed creating state manager. Err : %v", err)
		return
	}

	dscProfile := cluster.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "default",
			Namespace: "",
			Tenant:    "",
		},
		Spec: cluster.DSCProfileSpec{
			Features: cluster.FeatureSet{
				InterVMServices: true,
				FlowAware:       true,
				Firewall:        true,
			},
		},
	}

	// create DSC Profile
	sm.ctrler.DSCProfile().Create(&dscProfile)
	AssertEventually(t, func() (bool, interface{}) {

		_, err := sm.FindDSCProfile("", "default")
		if err == nil {
			return true, nil
		}
		fmt.Printf("Error find ten %v\n", err)
		return false, nil
	}, "Profile not foud", "1ms", "1s")

	err = createDistributedServiceCard(sm, "default", "prod-beef", nil)
	Assert(t, (err == nil), "DistributedServiceCard could not be created")

	dscProfile.Spec.Features.InterVMServices = true
	dscProfile.Spec.Features.FlowAware = true
	dscProfile.Spec.Features.Firewall = true
	err = sm.ctrler.DSCProfile().Update(&dscProfile)
	Assert(t, (err == nil), "Failed to update dscprofile")

	dscs, err := sm.ListDSCProfiles()
	Assert(t, (err == nil), "List DSC profiles failed")
	Assert(t, len(dscs) == 1, "incorrect number of dscs")

	err = sm.ctrler.DSCProfile().Delete(&dscProfile)
	Assert(t, (err == nil), "Failed to delete dscprofile")
}

func TestMain(m *testing.M) {
	tsdb.Init(context.Background(), &tsdb.Opts{})

	config := log.GetDefaultConfig("orchhub-statemgr-test")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger = log.SetConfig(config)

	os.Exit(m.Run())
}
