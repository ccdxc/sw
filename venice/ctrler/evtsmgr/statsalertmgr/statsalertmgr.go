package statsalertmgr

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/statsalertmgr/alertengine"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/statsalertmgr/policyhdr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	memdb2 "github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

//
// StatsAlertMgr is responsible for watching alert policies from memdb and creating policy handlers for them.
// There is a policy handler created for each stats alert policy. And, it is responsible for converting
// the policy to metrics query, passing the metrics query result to alert engine for further action (
// create/update/resolve).
//

// StatsAlertMgr represents the stats alert manager.
type StatsAlertMgr struct {
	sync.RWMutex
	memDb            *memdb.MemDb                             // in-memory db/cache
	resolverClient   resolver.Interface                       // resolver
	metricsRPCClient *rpckit.RPCClient                        // RPC client to connect to the metrics/telemetry service
	metricsClient    telemetry_query.TelemetryV1Client        // metrics/telemetry query service client
	policyHandlers   map[string]*policyhdr.StatsPolicyHandler // map of policy id to handlers
	alertEngine      *alertengine.StatsAlertEngine            // stats alert engine
	logger           log.Logger                               // logger
	ctx              context.Context                          // context to cancel goroutines
	cancelFunc       context.CancelFunc                       // context to cancel goroutines
	wg               sync.WaitGroup
}

var (
	maxConnRetries = 20
)

// NewStatsAlertMgr creates the new stats alert mgr with the given parameters.
func NewStatsAlertMgr(parentCtx context.Context, memDb *memdb.MemDb,
	resolverClient resolver.Interface, logger log.Logger) (*StatsAlertMgr, error) {
	if nil == logger {
		return nil, errors.New("all parameters are required")
	}

	ctx, cancelFunc := context.WithCancel(parentCtx)
	sam := &StatsAlertMgr{
		memDb:          memDb,
		resolverClient: resolverClient,
		policyHandlers: make(map[string]*policyhdr.StatsPolicyHandler),
		ctx:            ctx,
		cancelFunc:     cancelFunc,
		logger:         logger}

	// create metrics client
	if err := sam.createMetricClient(); err != nil {
		sam.logger.Errorf("failed to create metrics RPC client, err: %v", err)
		return nil, err
	}

	// start alert engine
	sam.alertEngine = alertengine.NewStatsAlertEngine(ctx, memDb, resolverClient, logger)

	sam.wg.Add(1)
	go sam.watchStatsAlertPolicies()
	return sam, nil
}

// Stop stops the stats alert manager.
func (s *StatsAlertMgr) Stop() {
	s.logger.Infof("stopping stats alert mgr")
	s.cancelFunc()
	s.wg.Wait()

	// stop all the policy handlers
	s.Lock()
	for _, ph := range s.policyHandlers {
		ph.Stop()
		delete(s.policyHandlers, ph.Name())
	}
	s.Unlock()

	// stop alert engine
	if s.alertEngine != nil {
		s.alertEngine.Stop()
		s.alertEngine = nil
	}

	// close the metrics client
	if s.metricsRPCClient != nil {
		s.metricsRPCClient.Close()
		s.metricsRPCClient = nil
	}

	s.logger.Infof("stopped stats alert mgr")
}

// Watches the stats alert policies by establishing a watch with memdb.
func (s *StatsAlertMgr) watchStatsAlertPolicies() {
	s.logger.Infof("starting stats alert policy watcher (memdb)")
	defer s.wg.Done()

	watcher := s.memDb.WatchStatsAlertPolicy()
	defer s.memDb.StopWatchStatsAlertPolicy(watcher)

	for {
		select {
		case <-s.ctx.Done():
			s.logger.Infof("context cancelled; exiting from stats policy watcher (memdb)")
			return
		case evt, ok := <-watcher.Channel:
			if !ok {
				s.logger.Errorf("error reading stats alert policy from the memdb channel, exiting")
				return
			}
			sapObj := evt.Obj.(*monitoring.StatsAlertPolicy)
			s.handleStatsAlertPolicy(evt.EventType, sapObj)
		}
	}
}

// Creates/Deletes the policy handler based on the watch event type. Currently, we do not support
// updates on the stats alert policy. Any update on the policy will have to go through delete + create.
func (s *StatsAlertMgr) handleStatsAlertPolicy(evtType memdb2.EventType, sap *monitoring.StatsAlertPolicy) {
	s.Lock()
	defer s.Unlock()

	policyName := fmt.Sprintf("%s/%s", sap.GetName(), sap.GetUUID())
	switch evtType {
	case memdb.CreateEvent: // create new policy handler
		ph, err := policyhdr.NewStatsPolicyHandler(s.ctx, policyName, sap, s.metricsClient, s.alertEngine, s.logger.WithContext("stats-pol-handler", policyName))
		if err != nil {
			s.logger.Errorf("failed to create stats policy handler, err: %v", err)
			return
		}
		ph.Start()
		s.policyHandlers[policyName] = ph
		s.logger.Infof("created stats policy handler for the policy: %s", policyName)
	case memdb.DeleteEvent: // delete existing policy handler
		if ph, found := s.policyHandlers[policyName]; found {
			ph.Stop()
			delete(s.policyHandlers, policyName)
			s.logger.Infof("deleted existing stats policy handler for the policy: %s", policyName)
		}
	case memdb.UpdateEvent:
		s.logger.Errorf("update operation not supported for stats alert policy, " +
			"but received an update event. something went wrong")
	}
}

// open a connection to citadel service with retries
func (s *StatsAlertMgr) createMetricClient() error {
	rpcOptions := []rpckit.Option{
		rpckit.WithLogger(s.logger),
		rpckit.WithBalancer(balancer.New(s.resolverClient))}

	client, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		return rpckit.NewRPCClient("statsalertmgr", globals.Citadel, rpcOptions...)
	}, 2*time.Second, maxConnRetries)
	if err != nil {
		return err
	}

	s.metricsRPCClient = client.(*rpckit.RPCClient)
	s.metricsClient = telemetry_query.NewTelemetryV1Client(s.metricsRPCClient.ClientConn)
	return nil
}
