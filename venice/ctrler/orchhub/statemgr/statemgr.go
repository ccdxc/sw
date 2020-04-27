package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

// Statemgr struct
type Statemgr struct {
	sync.Mutex
	apicl             apiclient.Services
	apiSrvURL         string
	resolver          resolver.Interface
	logger            log.Logger
	ctrler            ctkit.Controller
	instanceManagerCh chan *kvstore.WatchEvent
	probeChMutex      sync.RWMutex
	probeCh           map[string]chan *kvstore.WatchEvent
}

// NewStatemgr creates a new state mgr
func NewStatemgr(apiSrvURL string, resolver resolver.Interface, logger log.Logger, instanceManagerCh chan *kvstore.WatchEvent) (*Statemgr, error) {
	var apicl apiclient.Services
	var err error
	if resolver != nil {
		apicl, err = apiclient.NewGrpcAPIClient(globals.OrchHub, apiSrvURL, logger, rpckit.WithBalancer(balancer.New(resolver)))
		if err != nil {
			logger.Warnf("Failed to connect to gRPC Server [%s]\n", apiSrvURL)
		}
	}

	logger.Infof("Creating new ctkit controller")
	ctrler, _, err := ctkit.NewController(globals.OrchHub, nil, apiSrvURL, resolver, logger, false)
	if err != nil {
		logger.Errorf("Error initiating controller kit. Err: %v", err)
		return nil, err
	}
	logger.Infof("New ctkit controller created")

	stateMgr := &Statemgr{
		apicl:             apicl,
		resolver:          resolver,
		logger:            logger,
		ctrler:            ctrler,
		instanceManagerCh: instanceManagerCh,
		probeCh:           make(map[string]chan *kvstore.WatchEvent),
	}

	err = stateMgr.startWatchers()
	if err != nil {
		logger.Errorf("failed to start watchers. Err: %v", err)
		return nil, err
	}

	return stateMgr, nil
}

// Controller gets the controller associated with the statemanager
func (s *Statemgr) Controller() ctkit.Controller {
	return s.ctrler
}

// SetAPIClient sets the apiclient
func (s *Statemgr) SetAPIClient(cl apiclient.Services) {
	s.apicl = cl
}

// RemoveProbeChannel removes the channel for communication with vcprobe
func (s *Statemgr) RemoveProbeChannel(orchKey string) error {
	s.probeChMutex.Lock()
	defer s.probeChMutex.Unlock()

	_, ok := s.probeCh[orchKey]

	if !ok {
		return fmt.Errorf("vc probe channel [%s] not found", orchKey)
	}

	delete(s.probeCh, orchKey)
	return nil
}

// AddProbeChannel sets the channel for communication with vcprobe
func (s *Statemgr) AddProbeChannel(orchKey string, orchOpsChannel chan *kvstore.WatchEvent) error {
	s.probeChMutex.Lock()
	defer s.probeChMutex.Unlock()

	_, ok := s.probeCh[orchKey]

	if ok {
		return fmt.Errorf("vc probe channel [%s] already exists", orchKey)
	}

	s.probeCh[orchKey] = orchOpsChannel

	return nil
}

// GetProbeChannel sets the channel for communication with vcprobe
func (s *Statemgr) GetProbeChannel(orchKey string) (chan *kvstore.WatchEvent, error) {
	s.probeChMutex.RLock()
	defer s.probeChMutex.RUnlock()

	ch, ok := s.probeCh[orchKey]
	if !ok {
		return nil, fmt.Errorf("vc probe channel [%s] not found", orchKey)
	}

	return ch, nil
}

// SendNetworkProbeEvent sends network  event to appropriate orchestrator
func (s *Statemgr) SendNetworkProbeEvent(obj runtime.Object, evtType kvstore.WatchEventType) error {
	nw := obj.(*network.Network)
	if nw == nil {
		return fmt.Errorf("object passed is not of network type. Object : %v", obj)
	}
	err := s.SendProbeEvent(obj, evtType, "")
	if err != nil {
		s.logger.Errorf("Failed to send network probe event, Err : %v", err)
		return err
	}

	return nil
}

// SendProbeEvent send probe event to appropriate orchestrator
// If orchKey is blank, it will send to all orchestrators
func (s *Statemgr) SendProbeEvent(obj runtime.Object, evtType kvstore.WatchEventType, orchKey string) error {
	s.logger.Infof("Sending probe event - %v Type - %v to Orchestrator - %v", obj, evtType, orchKey)
	s.probeChMutex.Lock()
	defer s.probeChMutex.Unlock()
	if len(orchKey) != 0 {
		ch, ok := s.probeCh[orchKey]
		if !ok {
			return fmt.Errorf("failed to get orchestrator channel for %s", orchKey)
		}

		ch <- &kvstore.WatchEvent{Object: obj, Type: evtType}
		return nil
	}

	for _, ch := range s.probeCh {
		ch <- &kvstore.WatchEvent{Object: obj, Type: evtType}
	}

	return nil
}

// startWatchers starts ctkit watchers which reconciles all the objects in the local cache
// and APIserver
func (s *Statemgr) startWatchers() error {
	// All the Watch options ensure that they perform a diff of the objects present
	// in local cache and API server and update the local cache accordingly in
	// order to ensure the cache and the API server are in sync. This operation
	// is performed synchronously when the Watch is first setup and before a
	// goroutine servicing the Watch is created,
	// The order of Watch setup is important to ensure that the object dependencies
	// are present in the local cache when the Watch for that object is setup.
	// We setup Watch for the orchestrator at the end so that the probes created by
	// the orchestrator has all the objects for reconciliation.

	err := s.ctrler.Cluster().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on cluster. Err: %v", err)
	}

	err = s.ctrler.Network().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on network. Err: %v", err)
	}

	err = s.ctrler.DSCProfile().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on DSC Profiles. Err: %v", err)
	}

	err = s.ctrler.DistributedServiceCard().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on dsc. Err: %v", err)
	}

	err = s.ctrler.Host().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on host. Err: %v", err)
	}

	err = s.ctrler.Workload().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on workload. Err: %v", err)
	}

	err = s.ctrler.Orchestrator().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on orchestrator. Err: %v", err)
	}

	return nil
}

// FindObject looks up an object in local db
func (s *Statemgr) FindObject(kind, tenant, ns, name string) (runtime.Object, error) {
	// form network key
	ometa := api.ObjectMeta{
		Tenant:    tenant,
		Namespace: ns,
		Name:      name,
	}

	// find it in db
	return s.ctrler.FindObject(kind, &ometa)
}
