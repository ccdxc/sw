package statemgr

import (
	"context"
	"os"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
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

	_, err = CreateNetwork(sm, "default", "prod-beef-vlan100", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo)
	Assert(t, (err == nil), "network could not be created")

	_, err = CreateNetwork(sm, "default", "prod-bebe-vlan200", "10.2.1.0/24", "10.2.1.1", 200, nil, orchInfo)
	Assert(t, (err == nil), "network could not be created")

	labels := map[string]string{"color": "green"}
	_, err = CreateNetwork(sm, "default", "dev-caca-vlan300", "10.3.1.0/24", "10.3.1.1", 300, labels, orchInfo)
	Assert(t, (err == nil), "network could not be created")

	nw, err := sm.ctrler.Network().List(context.Background(), &api.ListWatchOptions{})
	Assert(t, len(nw) == 2, "did not find all networks, found [%v] networks", len(nw))

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
			Tenant:    tenant,
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
	Assert(t, len(nw) == 2, "did not find all Workloads, found [%v] Workloads", len(nw))

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

	err = createHost(sm, "default", "prod-beef", nil)
	Assert(t, (err == nil), "Host could not be created")

	err = createHost(sm, "default", "prod-bebe", nil)
	Assert(t, (err == nil), "Host could not be created")

	labels := map[string]string{"color": "green"}
	err = createHost(sm, "default", "dev-caca", labels)
	Assert(t, (err == nil), "Host could not be created")

	nw, err := sm.ctrler.Host().List(context.Background(), &api.ListWatchOptions{})
	Assert(t, len(nw) == 2, "did not find all Hosts, found [%v] Hosts", len(nw))

	opts := api.ListWatchOptions{}
	opts.LabelSelector = "color=green"
	nw, err = sm.ctrler.Host().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "found more Hosts than expected. [%v]", len(nw))

	meta := api.ObjectMeta{
		Name:      "prod-bebe",
		Namespace: "default",
		//Host  is cluster object no need for tenant
		//Tenant:    "default",
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

	return
}

// createEndpoint utility function to create a Endpoint
func createEndpoint(stateMgr *Statemgr, tenant, name string, labels map[string]string) error {
	// Endpoint params
	np := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			Tenant:    tenant,
			Labels:    labels,
		},
		Spec:   workload.EndpointSpec{},
		Status: workload.EndpointStatus{},
	}

	// create a Endpoint
	return stateMgr.ctrler.Endpoint().Create(&np)
}

func TestEndpointCreateList(t *testing.T) {
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

	err = createEndpoint(sm, "default", "prod-beef", nil)
	Assert(t, (err == nil), "Endpoint could not be created")

	err = createEndpoint(sm, "default", "prod-bebe", nil)
	Assert(t, (err == nil), "Endpoint could not be created")

	labels := map[string]string{"color": "green"}
	err = createEndpoint(sm, "default", "dev-caca", labels)
	Assert(t, (err == nil), "Endpoint could not be created")

	nw, err := sm.ctrler.Endpoint().List(context.Background(), &api.ListWatchOptions{})
	Assert(t, len(nw) == 2, "did not find all Endpoints, found [%v] Endpoints", len(nw))

	opts := api.ListWatchOptions{}
	opts.LabelSelector = "color=green"
	nw, err = sm.ctrler.Endpoint().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "found more Endpoints than expected. [%v]", len(nw))

	meta := api.ObjectMeta{
		Name:      "prod-bebe",
		Namespace: "default",
		Tenant:    "default",
	}

	nobj, err := sm.Controller().Endpoint().Find(&meta)
	Assert(t, err == nil, "did not find the Endpoint")

	nobj.Endpoint.ObjectMeta.Labels = labels
	err = sm.Controller().Endpoint().Update(&nobj.Endpoint)
	Assert(t, err == nil, "unable to update the Endpoint object")

	nw, err = sm.Controller().Endpoint().List(context.Background(), &opts)
	Assert(t, len(nw) == 2, "expected 2 Endpoints found [%v]", len(nw))

	err = sm.Controller().Endpoint().Delete(&nobj.Endpoint)
	Assert(t, err == nil, "deletion was not successful")

	nw, err = sm.ctrler.Endpoint().List(context.Background(), &opts)
	Assert(t, len(nw) == 1, "expected 1, got %v Endpoints", len(nw))

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
		Spec:   cluster.DistributedServiceCardSpec{},
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

	err = createDistributedServiceCard(sm, "default", "prod-beef", nil)
	Assert(t, (err == nil), "DistributedServiceCard could not be created")

	err = createDistributedServiceCard(sm, "default", "prod-bebe", nil)
	Assert(t, (err == nil), "DistributedServiceCard could not be created")

	labels := map[string]string{"color": "green"}
	err = createDistributedServiceCard(sm, "default", "dev-caca", labels)
	Assert(t, (err == nil), "DistributedServiceCard could not be created")

	nw, err := sm.ctrler.DistributedServiceCard().List(context.Background(), &api.ListWatchOptions{})
	Assert(t, len(nw) == 2, "did not find all DistributedServiceCards, found [%v] DistributedServiceCards", len(nw))

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
	Assert(t, (err == nil), "Orchestrator could not be created")

	err = createOrchestrator(sm, "default", "prod-beef", nil)

	err = createOrchestrator(sm, "default", "prod-bebe", nil)
	Assert(t, (err == nil), "Orchestrator could not be created")

	labels := map[string]string{"color": "green"}
	err = createOrchestrator(sm, "default", "dev-caca", labels)
	Assert(t, (err == nil), "Orchestrator could not be created")

	nw, err := sm.ctrler.Orchestrator().List(context.Background(), &api.ListWatchOptions{})
	Assert(t, len(nw) == 2, "did not find all Orchestrators, found [%v] Orchestrators", len(nw))

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

	return
}

func TestMain(m *testing.M) {
	tsdb.Init(context.Background(), &tsdb.Opts{})
	os.Exit(m.Run())
}
