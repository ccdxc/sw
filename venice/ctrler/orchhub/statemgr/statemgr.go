package statemgr

import (
	"context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Statemgr struct
type Statemgr struct {
	apicl     apiclient.Services
	apiSrvURL string
	resolver  resolver.Interface
	logger    log.Logger
}

// NewStatemgr creates a new state mgr
func NewStatemgr(apiSrvURL string, resolver resolver.Interface, logger log.Logger) (*Statemgr, error) {
	apicl, err := apiclient.NewGrpcAPIClient(globals.OrchHub, apiSrvURL, logger, rpckit.WithBalancer(balancer.New(resolver)))
	if err != nil {
		log.Warnf("Failed to connect to gRPC Server [%s]\n", apiSrvURL)
	}
	return &Statemgr{
		apicl:    apicl,
		resolver: resolver,
		logger:   logger,
	}, nil
}

// SetAPIClient sets the apiclient
func (s *Statemgr) SetAPIClient(cl apiclient.Services) {
	s.apicl = cl
}

// WatchOrchestrator watches for orch config
func (s *Statemgr) WatchOrchestrator(ch chan *kvstore.WatchEvent) {
	orchestrationWatcher, err := s.apicl.OrchestratorV1().Orchestrator().Watch(context.Background(), &api.ListWatchOptions{})

	if err != nil {
		log.Errorf("Failed to start orchestrator event watcher. Err : %v", err)
		return
	}
	for {
		select {
		case evt, ok := <-orchestrationWatcher.EventChan():
			if ok {
				ch <- evt
			} else {
				return
			}
		}
	}
}

// GetWorkload gets the workload
func (s *Statemgr) GetWorkload(meta *api.ObjectMeta) (*workload.Workload, error) {
	return s.apicl.WorkloadV1().Workload().Get(context.Background(), meta)
}

// UpdateWorkload updates the workload
func (s *Statemgr) UpdateWorkload(workloadObj *workload.Workload) (*workload.Workload, error) {
	return s.apicl.WorkloadV1().Workload().Update(context.Background(), workloadObj)
}

// CreateWorkload creates the workload
func (s *Statemgr) CreateWorkload(workloadObj *workload.Workload) (*workload.Workload, error) {
	return s.apicl.WorkloadV1().Workload().Create(context.Background(), workloadObj)
}

// DeleteWorkload deletes the workload
func (s *Statemgr) DeleteWorkload(meta *api.ObjectMeta) (*workload.Workload, error) {
	return s.apicl.WorkloadV1().Workload().Delete(context.Background(), meta)
}

// GetHost gets the Host
func (s *Statemgr) GetHost(meta *api.ObjectMeta) (*cluster.Host, error) {
	return s.apicl.ClusterV1().Host().Get(context.Background(), meta)
}

// UpdateHost updates the Host
func (s *Statemgr) UpdateHost(HostObj *cluster.Host) (*cluster.Host, error) {
	return s.apicl.ClusterV1().Host().Update(context.Background(), HostObj)
}

// CreateHost creates the Host
func (s *Statemgr) CreateHost(HostObj *cluster.Host) (*cluster.Host, error) {
	return s.apicl.ClusterV1().Host().Create(context.Background(), HostObj)
}

// DeleteHost deletes the Host
func (s *Statemgr) DeleteHost(meta *api.ObjectMeta) (*cluster.Host, error) {
	return s.apicl.ClusterV1().Host().Delete(context.Background(), meta)
}
