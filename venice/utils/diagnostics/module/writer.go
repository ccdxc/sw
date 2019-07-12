package module

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	queueSize           = 1000
	apiSrvRetryInterval = time.Second
	apiSrvMaxRetries    = 30
	// QueueFullEventMsg is the event message when module object cannot be enqueued because of full queue
	QueueFullEventMsg = "diagnostic module processing queue is full"
)

// updater is a singleton that implements Updater interface where module object is persisted in API server
var gModuleUpdater *updater
var updaterOnce sync.Once

type work struct {
	module *diagapi.Module
	oper   Oper
}

type updater struct {
	sync.RWMutex
	name      string // module name using the updater
	apiServer string // api server address
	resolver  resolver.Interface
	logger    log.Logger
	modChan   chan *work
	waitGrp   sync.WaitGroup
	ctx       context.Context
	cancel    context.CancelFunc
	stopped   bool
}

func (u *updater) Enqueue(module *diagapi.Module, oper Oper) error {
	wrk := &work{module, oper}
	select {
	case u.modChan <- wrk:
	default:
		recorder.Event(eventtypes.MODULE_CREATION_FAILED, QueueFullEventMsg, module)
		// returns error if work queue is full
		return errors.New("module processing queue is full")
	}
	return nil
}

func (u *updater) Start() {
	defer u.Unlock()
	u.Lock()
	if u.stopped {
		// create context and cancel
		u.ctx, u.cancel = context.WithCancel(context.Background())
		// clear old queue
		u.modChan = make(chan *work, queueSize)
		// setup wait group
		u.waitGrp.Add(1)
		go u.worker()
		u.stopped = false
	}
}

func (u *updater) Stop() {
	defer u.Unlock()
	u.Lock()
	u.cancel()
	if u.modChan != nil {
		close(u.modChan)
		u.modChan = nil
	}
	// wait for all go routines to return after Stop() has been called
	u.waitGrp.Wait()
	u.stopped = true
}

func (u *updater) doWork(apicl apiclient.Services) error {
	for {
		select {
		case wrk := <-u.modChan:
			if wrk == nil {
				u.logger.ErrorLog("method", "doWork", "msg", fmt.Sprintf("Received nil notification on mod channel"))
				break
			}
			switch wrk.oper {
			case Create:
				// delete module object if it exists
				apicl.DiagnosticsV1().Module().Delete(u.ctx, wrk.module.GetObjectMeta())
				_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
					return apicl.DiagnosticsV1().Module().Create(u.ctx, wrk.module)
				}, apiSrvRetryInterval, apiSrvMaxRetries)
				if err != nil {
					recorder.Event(eventtypes.MODULE_CREATION_FAILED, fmt.Sprintf("failed to create module (%s) to collect diagnostics, err: %v", wrk.module.Name, err), wrk.module)
					u.logger.ErrorLog("method", "doWork", "msg", fmt.Sprintf("failed to create module (%+v) to collect diagnositcs, err: %v", wrk.module, err))
				}
			case Update:
				_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
					_, err := apicl.DiagnosticsV1().Module().Get(ctx, wrk.module.GetObjectMeta())
					if err == nil {
						// module object exists so update
						return apicl.DiagnosticsV1().Module().Update(u.ctx, wrk.module)
					}
					return apicl.DiagnosticsV1().Module().Create(u.ctx, wrk.module)
				}, apiSrvRetryInterval, apiSrvMaxRetries)
				if err != nil {
					u.logger.ErrorLog("method", "doWork", "msg", fmt.Sprintf("failed to update module (%+v), err: %v", wrk.module, err))
				}
			case Delete:
				_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
					return apicl.DiagnosticsV1().Module().Delete(u.ctx, wrk.module.GetObjectMeta())
				}, apiSrvRetryInterval, apiSrvMaxRetries)
				if err != nil {
					u.logger.ErrorLog("method", "doWork", "msg", fmt.Sprintf("failed to delete module (%+v), err: %v", wrk.module, err))
				}
			}
		case <-u.ctx.Done():
			u.logger.ErrorLog("method", "doWork", "msg", "Exiting worker loop")
			return u.ctx.Err()
		}
	}
}

func (u *updater) worker() {
	defer u.waitGrp.Done()
	// create a grpc client
	b := balancer.New(u.resolver)
	// loop forever
	for {
		apicl, err := apiclient.NewGrpcAPIClient(u.name, u.apiServer, u.logger, rpckit.WithBalancer(b))
		if err != nil {
			u.logger.WarnLog("method", "worker", "msg", fmt.Sprintf("failed to connect to gRPC server [%s]", u.apiServer))
		} else {
			u.logger.InfoLog("method", "worker", "msg", fmt.Sprintf("API client connected {%+v}", apicl))
			u.doWork(apicl)
			apicl.Close()
		}
		// if ctx has been cancelled, we are done
		if u.ctx.Err() != nil {
			u.logger.InfoLog("method", "worker", "msg", "exiting module updater")
			return
		}
		// wait for a second and retry connecting to api server
		time.Sleep(time.Second)
	}
}

// GetUpdater returns a singleton implementation of Updater to modify module objects
func GetUpdater(name, apiServer string, rslver resolver.Interface, logger log.Logger) Updater {
	if gModuleUpdater != nil {
		gModuleUpdater.name = name
		gModuleUpdater.apiServer = apiServer
		gModuleUpdater.resolver = rslver
		gModuleUpdater.logger = logger
		gModuleUpdater.Start()
	}
	updaterOnce.Do(func() {
		// create context and cancel
		ctx, cancel := context.WithCancel(context.Background())
		if logger == nil {
			logger = log.GetNewLogger(log.GetDefaultConfig(name))
		}
		gModuleUpdater = &updater{
			name:      name,
			apiServer: apiServer,
			resolver:  rslver,
			logger:    logger,
			modChan:   make(chan *work, queueSize),
			ctx:       ctx,
			cancel:    cancel,
			stopped:   false,
		}
		// setup wait group
		gModuleUpdater.waitGrp.Add(1)
		go gModuleUpdater.worker()
	})

	return gModuleUpdater
}
