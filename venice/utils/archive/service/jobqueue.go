package service

import (
	"context"
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/archive"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	defaultQueueSize = 100
)

// jobQueue is a singleton that implements JobQueue interface
var gJobQueue *jobQueue
var jobQueueOnce sync.Once

type jobQueue struct {
	sync.RWMutex
	name        string
	jobChan     chan archive.Job
	size        int
	runningJobs map[string]archive.Job
	waitGrp     sync.WaitGroup
	parentCtx   context.Context
	ctx         context.Context
	cancel      context.CancelFunc
	stopped     bool
	apiserver   string
	rslvr       resolver.Interface
	logger      log.Logger
}

func (jq *jobQueue) CancelJob(id string) (archive.Job, error) {
	defer jq.Unlock()
	jq.Lock()
	j, ok := jq.runningJobs[id]
	if !ok {
		jq.logger.DebugLog("method", "CancelJob", "msg", fmt.Sprintf("cannot cancel archive request [%s], it was already processed", id))
		return nil, errors.New("cannot cancel archive request it was already processed")
	}
	j.Cancel()
	req := j.GetArchiveRequest()
	jq.logger.DebugLog("method", "CancelJob", "msg", fmt.Sprintf("attempting to cancel archive request [%s|%s] for log type [%s]", req.Tenant, req.Name, req.Spec.Type))
	return j, nil
}

func (jq *jobQueue) CancelJobs() {
	defer jq.Unlock()
	jq.Lock()
	for _, j := range jq.runningJobs {
		j.Cancel()
	}
	jq.runningJobs = make(map[string]archive.Job)
	return
}

func (jq *jobQueue) AddJob(j archive.Job) error {
	req := j.GetArchiveRequest()
	if !shouldProcessJob(j) {
		jq.logger.InfoLog("method", "AddJob", "msg", fmt.Sprintf("archive request [%s|%s] for log type [%s] has already been processed, status [%s]", req.Tenant, req.Name, req.Spec.Type, req.Status.Status))
		return nil
	}
	jq.RLock()
	for _, runningJob := range jq.runningJobs {
		if runningJob.GetArchiveRequest().Spec.Type == j.GetArchiveRequest().Spec.Type {
			// assumes one instance of spyglass
			// there is an existing job running for the log type, fail the new job
			req.Status.Status = monitoring.ArchiveRequestStatus_Failed.String()
			req.Status.Reason = fmt.Sprintf("archive request [%s] already running for log type %s", runningJob.GetArchiveRequest().Name, req.Spec.Type)
			jq.updateArchiveRequestStatus(req)
			jq.logger.DebugLog("method", "AddJob", "msg", fmt.Sprintf("failed archive request [%s|%s] from job queue for log type [%s] for reason [%s]", req.Tenant, req.Name, req.Spec.Type, req.Status.Reason))
			jq.RUnlock()
			return nil
		}
	}
	jq.RUnlock()
	req.Status.Status = monitoring.ArchiveRequestStatus_Running.String()
	jq.updateArchiveRequestStatus(req)
	select {
	case jq.jobChan <- j:
		jq.logger.DebugLog("method", "AddJob", "msg", fmt.Sprintf("added archive request to job queue [%#v]", *j.GetArchiveRequest()))
	default:
		req.Status.Status = monitoring.ArchiveRequestStatus_Failed.String()
		req.Status.Reason = "archive job queue is full"
		jq.updateArchiveRequestStatus(req)
		jq.logger.ErrorLog("method", "AddJob", "msg", "archive job queue is full")
		return errors.New("archive job queue is full")
	}
	return nil
}

func (jq *jobQueue) ListJobs() []archive.Job {
	jq.RLock()
	defer jq.RUnlock()
	var jobs []archive.Job
	for _, j := range jq.runningJobs {
		jobs = append(jobs, j)
	}
	return jobs
}

func (jq *jobQueue) Start() {
	defer jq.Unlock()
	jq.Lock()
	if jq.stopped {
		jq.logger.DebugLog("method", "Start", "msg", "starting archive service job queue")
		// create context and cancel
		jq.ctx, jq.cancel = context.WithCancel(jq.parentCtx)
		// clear old queue
		jq.jobChan = make(chan archive.Job, jq.size)
		jq.runningJobs = make(map[string]archive.Job)
		// setup wait group
		jq.waitGrp.Add(1)
		go jq.worker()
		jq.stopped = false
	}
}

func (jq *jobQueue) Stop() {
	jq.logger.DebugLog("method", "Stop", "msg", "stopping archive service job queue")
	jq.RLock()
	jq.cancel()
	if jq.jobChan != nil {
		close(jq.jobChan)
	}
	jq.RUnlock()
	// wait for all go routines to return after Stop() has been called
	jq.waitGrp.Wait()
	defer jq.Unlock()
	jq.Lock()
	jq.jobChan = nil
	jq.runningJobs = nil
	jq.stopped = true
}

func (jq *jobQueue) worker() error {
	defer jq.waitGrp.Done()
	for {
		select {
		case wrk := <-jq.jobChan:
			if wrk == nil {
				jq.logger.ErrorLog("method", "worker", "msg", fmt.Sprintf("Received nil notification on job channel"))
				break
			}
			jq.waitGrp.Add(1)
			go func() {
				defer jq.waitGrp.Done()
				jq.Lock()
				jq.runningJobs[wrk.ID()] = wrk
				jq.Unlock()
				wrk.Run(jq.ctx)
				jq.updateArchiveRequestStatus(wrk.GetArchiveRequest())
				defer jq.Unlock()
				jq.Lock()
				delete(jq.runningJobs, wrk.ID())
				jq.logger.DebugLog("method", "worker", "msg", fmt.Sprintf("deleted archive request [%#v] from job queue", *wrk.GetArchiveRequest()))
			}()
		case <-jq.ctx.Done():
			jq.logger.ErrorLog("method", "worker", "msg", "Exiting worker loop")
			return jq.ctx.Err()
		}
	}
}

// GetJobQueue returns a singleton instance of job queue
func GetJobQueue(ctx context.Context, name string, size int, apiserver string, rslvr resolver.Interface, logger log.Logger) archive.JobQueue {
	if gJobQueue != nil {
		gJobQueue.parentCtx = ctx
		gJobQueue.name = name
		gJobQueue.size = size
		gJobQueue.logger = logger
		gJobQueue.apiserver = apiserver
		gJobQueue.rslvr = rslvr
		gJobQueue.Start()
	}
	jobQueueOnce.Do(func() {
		if size <= 0 {
			size = defaultQueueSize
		}
		// create context and cancel
		nctx, cancel := context.WithCancel(ctx)
		if logger == nil {
			logger = log.GetNewLogger(log.GetDefaultConfig(name))
		}
		gJobQueue = &jobQueue{
			name:        name,
			logger:      logger,
			jobChan:     make(chan archive.Job, size),
			size:        size,
			runningJobs: make(map[string]archive.Job),
			parentCtx:   ctx,
			ctx:         nctx,
			cancel:      cancel,
			stopped:     false,
			apiserver:   apiserver,
			rslvr:       rslvr,
		}
		gJobQueue.logger.DebugLog("method", "Start", "msg", "starting archive service job queue")
		// setup wait group
		gJobQueue.waitGrp.Add(1)
		go gJobQueue.worker()
	})
	return gJobQueue
}

func (jq *jobQueue) updateArchiveRequestStatus(req *monitoring.ArchiveRequest) (*monitoring.ArchiveRequest, error) {
	var err error
	defer func() {
		if err != nil {
			// TODO: throw an event
		}
	}()
	b := balancer.New(jq.rslvr)
	defer b.Close()
	grpcOpts := []rpckit.Option{rpckit.WithBalancer(b)}
	// create a grpc client
	result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		return apiclient.NewGrpcAPIClient(fmt.Sprintf(jq.name, "archive"), jq.apiserver, jq.logger, grpcOpts...) // TODO: cache client to optimize
	}, time.Second, 10)
	if err != nil {
		jq.logger.ErrorLog("method", "updateArchiveRequestStatus", "msg", fmt.Sprintf("failed to create api server client for archive request [%s]", req.Name), "error", err)
		return nil, err
	}
	apicl := result.(apiclient.Services)
	defer apicl.Close()
	result, err = utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		return apicl.MonitoringV1().ArchiveRequest().UpdateStatus(ctx, req)
	}, 500*time.Millisecond, 20)
	if err != nil {
		jq.logger.ErrorLog("method", "updateArchiveRequestStatus", "msg", fmt.Sprintf("failed to update archive request status for [%s]", req.Name), "error", err)
		return nil, err
	}
	jq.logger.DebugLog("method", "updateArchiveRequestStatus", "msg", fmt.Sprintf("updated archive request status [%#v]", *req))
	return result.(*monitoring.ArchiveRequest), nil
}

func shouldProcessJob(j archive.Job) bool {
	req := j.GetArchiveRequest()
	switch req.Status.Status {
	case monitoring.ArchiveRequestStatus_Scheduled.String(), monitoring.ArchiveRequestStatus_Running.String(), "":
		return true
	default:
		return false
	}
}
