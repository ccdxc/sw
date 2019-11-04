package statemgr

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/ctkit"
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
	orchChMutex       sync.RWMutex
	orchCh            map[string]chan *kvstore.WatchEvent
}

// NewStatemgr creates a new state mgr
func NewStatemgr(apiSrvURL string, resolver resolver.Interface, logger log.Logger, instanceManagerCh chan *kvstore.WatchEvent) (*Statemgr, error) {
	var apicl apiclient.Services
	var err error
	if resolver != nil {
		apicl, err = apiclient.NewGrpcAPIClient(globals.OrchHub, apiSrvURL, logger, rpckit.WithBalancer(balancer.New(resolver)))
		if err != nil {
			log.Warnf("Failed to connect to gRPC Server [%s]\n", apiSrvURL)
		}
	}

	logger.Infof("Creating new ctkit controller")
	ctrler, err := ctkit.NewController(globals.OrchHub, nil, apiSrvURL, resolver, logger)
	if err != nil {
		logger.Errorf("Error initiating controller kit. Err: %v", err)
		return nil, err
	}
	log.Infof("New ctkit controller created")

	stateMgr := &Statemgr{
		apicl:             apicl,
		resolver:          resolver,
		logger:            logger,
		ctrler:            ctrler,
		instanceManagerCh: instanceManagerCh,
		orchCh:            make(map[string]chan *kvstore.WatchEvent),
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
	s.orchChMutex.Lock()
	defer s.orchChMutex.Unlock()

	_, ok := s.orchCh[orchKey]

	if !ok {
		return fmt.Errorf("vc probe channel [%s] not found", orchKey)
	}

	delete(s.orchCh, orchKey)
	return nil
}

// AddProbeChannel sets the channel for communication with vcprobe
func (s *Statemgr) AddProbeChannel(orchKey string, orchOpsChannel chan *kvstore.WatchEvent) error {
	s.orchChMutex.Lock()
	defer s.orchChMutex.Unlock()

	_, ok := s.orchCh[orchKey]

	if ok {
		return fmt.Errorf("vc probe channel [%s] already exists", orchKey)
	}

	s.orchCh[orchKey] = orchOpsChannel

	return nil
}

// GetProbeChannel sets the channel for communication with vcprobe
func (s *Statemgr) GetProbeChannel(orchKey string) (chan *kvstore.WatchEvent, error) {
	s.orchChMutex.RLock()
	defer s.orchChMutex.RUnlock()

	ch, ok := s.orchCh[orchKey]
	if !ok {
		return nil, fmt.Errorf("vc probe channel [%s] not found", orchKey)
	}

	return ch, nil
}

// SendProbeEvent send probe event to appropriate orchestrator
func (s *Statemgr) SendProbeEvent(obj runtime.Object, evtType kvstore.WatchEventType) error {
	meta, err := runtime.GetObjectMeta(obj)
	if err != nil {
		return fmt.Errorf("Failed to get object meta")
	}

	orchKey := meta.Labels["orchestrator-name"]
	if len(orchKey) <= 0 {
		return fmt.Errorf("could not get orchestrator name")
	}

	ch, ok := s.orchCh[orchKey]
	if !ok {
		return fmt.Errorf("failed to get orchestrator channel for %s", orchKey)
	}

	ch <- &kvstore.WatchEvent{Object: obj, Type: evtType}
	return nil
}

func (s *Statemgr) startWatchers() error {
	err := s.ctrler.Orchestrator().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on orchestrator. Err: %v", err)
	}

	err = s.ctrler.Endpoint().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on endpoint. Err: %v", err)
	}

	err = s.ctrler.Workload().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on workload. Err: %v", err)
	}

	err = s.ctrler.Network().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on network. Err: %v", err)
	}

	err = s.ctrler.Host().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on host. Err: %v", err)
	}

	err = s.ctrler.DistributedServiceCard().Watch(s)
	if err != nil {
		return fmt.Errorf("Error establishing watch on dsc. Err: %v", err)
	}

	return nil
}
