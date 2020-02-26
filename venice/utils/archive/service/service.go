package service

import (
	"context"
	"errors"
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/archive"
	"github.com/pensando/sw/venice/utils/archive/exporter"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	watcherutils "github.com/pensando/sw/venice/utils/watcher"
)

// serviceImpl is a singleton that implements Service interface
var gService *serviceImpl
var serviceOnce sync.Once

// Option fills the optional params for serviceImpl
type Option func(*serviceImpl)

type serviceImpl struct {
	sync.RWMutex
	name      string // module name using the archiver
	apiServer string // api server address
	rslvr     resolver.Interface
	watcher   *watcherutils.Watcher
	queue     archive.JobQueue
	exporters map[string]archive.Exporter
	logger    log.Logger
	stopped   bool
	cb        archive.CreateJobCb
}

// WithExporter option configures archive service with a given exporter. This is currently meant for testing.
func WithExporter(name string, exporter archive.Exporter) Option {
	return func(svc *serviceImpl) {
		svc.exporters[name] = exporter
	}
}

func (s *serviceImpl) Stop() {
	defer s.Unlock()
	s.Lock()
	s.logger.DebugLog("method", "Stop", "msg", "stopping archive service")
	s.watcher.Stop()
	s.queue.Stop()
	s.stopped = true
}

func (s *serviceImpl) start(name, apiServer string, rslver resolver.Interface, cb archive.CreateJobCb) {
	defer s.Unlock()
	s.Lock()
	if s.stopped {
		s.logger.DebugLog("method", "start", "msg", "starting archive service")
		s.name = name
		s.apiServer = apiServer
		s.rslvr = rslver
		s.cb = cb //callback could change across restarts for testing purposes
		s.queue = GetJobQueue(context.Background(), s.name, defaultQueueSize, s.apiServer, s.rslvr, s.logger)
		s.watcher.Start(s.name, s.apiServer, s.rslvr)
		s.stopped = false
	}
}

func (s *serviceImpl) Start() {
	s.start(s.name, s.apiServer, s.rslvr, s.cb)
}

// CancelRequest cancels a running job. Returns an error if job is not found
func (s *serviceImpl) CancelRequest(ctx context.Context, in *monitoring.ArchiveRequest) (*monitoring.ArchiveRequest, error) {
	j, err := s.queue.CancelJob(in.UUID)
	if err != nil {
		return nil, err
	}
	return j.GetArchiveRequest(), nil
}

// RegisterExporter registers an exporter
func (s *serviceImpl) RegisterExporter(name string, exporter archive.Exporter) error {
	defer s.Unlock()
	s.Lock()
	s.exporters[name] = exporter
	return nil
}

// UnregisterExporter unregisters an exporter
func (s *serviceImpl) UnregisterExporter(name string) archive.Exporter {
	defer s.Unlock()
	s.Lock()
	exporter, ok := s.exporters[name]
	if ok {
		delete(s.exporters, name)
	}
	return exporter
}

func (s *serviceImpl) pickExporter(req *monitoring.ArchiveRequest) (archive.Exporter, error) {
	// in future we can pick an exporter based on archive policy
	evtExpter, ok := gService.exporters[req.Spec.Type]
	if !ok {
		return exporter.NewObjstoreExporter(req.Spec.Type, req.Tenant, s.rslvr, s.logger)
	}
	return evtExpter, nil

}
func (s *serviceImpl) processEventCb(evt *kvstore.WatchEvent) error {
	defer s.RUnlock()
	s.RLock()
	switch evt.Type {
	case kvstore.Created:
		req, ok := evt.Object.(*monitoring.ArchiveRequest)
		if !ok {
			// unexpected error
			s.logger.ErrorLog("method", "processEventCb", "msg", fmt.Sprintf("watcher found object of invalid type: %+v", evt.Object))
			return errors.New("watcher found object of invalid type")
		}
		exporter, err := s.pickExporter(req)
		if err != nil {
			s.logger.ErrorLog("method", "processEventCb", "msg", "unable to create exporter", "error", err)
		}
		job := s.cb(req, exporter, s.rslvr, s.logger)
		if err := s.queue.AddJob(job); err != nil {
			s.logger.ErrorLog("method", "processEventCb", "msg", "error adding archive job to queue", "error", err)
			return err
		}
	}
	return nil
}

// callback when watch on api server is re-established
func (s *serviceImpl) initiateWatchCb() {
	// cancel existing jobs as watch on api server is re-established
	s.queue.CancelJobs()
}

// GetService returns a singleton implementation of archive service
func GetService(name, apiServer string, rslvr resolver.Interface, logger log.Logger, cb archive.CreateJobCb, options ...Option) archive.Service {
	if gService != nil {
		gService.start(name, apiServer, rslvr, cb)
		for _, o := range options {
			o(gService)
		}
	}
	serviceOnce.Do(func() {
		if logger == nil {
			logger = log.GetNewLogger(log.GetDefaultConfig(name))
		}
		gService = &serviceImpl{
			name:      name,
			apiServer: apiServer,
			rslvr:     rslvr,
			logger:    logger,
			stopped:   false,
			cb:        cb,
			exporters: make(map[string]archive.Exporter),
		}
		for _, o := range options {
			o(gService)
		}
		gService.logger.DebugLog("method", "GetService", "msg", "starting archive service")
		// start job queue
		gService.queue = GetJobQueue(context.Background(), name, defaultQueueSize, apiServer, rslvr, logger) // TODO: should this be a singleton?
		// start watcher
		gService.watcher = watcherutils.NewWatcher(name, apiServer, rslvr, logger, gService.initiateWatchCb, gService.processEventCb,
			[]rpckit.Option{rpckit.WithTLSClientIdentity(name)},
			&watcherutils.KindOptions{
				Kind: string(monitoring.KindArchiveRequest),
				Options: &api.ListWatchOptions{
					FieldChangeSelector: []string{"Spec"}, // only watch for spec changes
				},
			})

	})
	return gService
}
