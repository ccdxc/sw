package service

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/archive"
	"github.com/pensando/sw/venice/utils/archive/mock"
	"github.com/pensando/sw/venice/utils/archive/testutils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"

	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
)

const (
	clientName = "ArchiveServiceTest"
)

type tInfo struct {
	apiSrv     apiserver.Server
	apiSrvAddr string
	apicl      apiclient.Services
	objstorecl objstore.Client
	logger     log.Logger
}

func (ti *tInfo) setup() {
	var err error

	config := log.GetDefaultConfig(clientName)
	config.Filter = log.AllowAllFilter
	ti.logger = log.GetNewLogger(config)
	recorder.Override(mockevtsrecorder.NewRecorder(clientName, ti.logger))

	// api server
	ti.apiSrv, ti.apiSrvAddr, err = serviceutils.StartAPIServer("localhost:0", clientName, ti.logger)
	if err != nil {
		panic("Unable to start API Server")
	}

	// api server client
	ti.apicl, err = apiclient.NewGrpcAPIClient(clientName, ti.apiSrvAddr, ti.logger)
	if err != nil {
		panic("Error creating api client")
	}
	// create cluster
	MustCreateCluster(ti.apicl)
	// create tenant
	MustCreateTenant(ti.apicl, globals.DefaultTenant)

}

func (ti *tInfo) shutdown() {
	MustDeleteTenant(ti.apicl, globals.DefaultTenant)
	MustDeleteCluster(ti.apicl)
	// stop api server
	ti.apiSrv.Stop()
}

func TestCancelRequest(t *testing.T) {
	ti := &tInfo{}
	ti.setup()
	defer ti.shutdown()
	mockJobFn := func(req *monitoring.ArchiveRequest, exporter archive.Exporter, rslver resolver.Interface, logger log.Logger) archive.Job {
		return mock.GetJob(req)
	}
	svc := GetService(clientName, ti.apiSrvAddr, nil, ti.logger, mockJobFn, WithExporter(monitoring.ArchiveRequestSpec_AuditEvent.String(), mock.GetExporter()))
	defer svc.Stop()
	svcImpl := svc.(*serviceImpl)

	req := &monitoring.ArchiveRequest{}
	req.Defaults("All")
	req.Name = "test1"
	req.Spec.Type = monitoring.ArchiveRequestSpec_AuditEvent.String()
	req, err := testutils.CreateArchiveRequest(context.TODO(), ti.apicl, req)
	AssertOk(t, err, "failed to create archive request")
	fetchedReq := req
	AssertEventually(t, func() (bool, interface{}) {
		fetchedReq, err = ti.apicl.MonitoringV1().ArchiveRequest().Get(context.TODO(), &req.ObjectMeta)
		if err != nil {
			return false, err
		}
		if fetchedReq.Status.Status != monitoring.ArchiveRequestStatus_Running.String() {
			return false, fetchedReq
		}
		return true, nil
	}, fmt.Sprintf("archive request is not in running state: %#v", *fetchedReq))

	defer testutils.MustDeleteArchiveRequest(context.TODO(), ti.apicl, &req.ObjectMeta)
	// check running jobs
	runningJobs := svcImpl.queue.ListJobs()
	Assert(t, len(runningJobs) == 1, fmt.Sprintf("no running jobs in queue: %v", testutils.PrintJobs(runningJobs)))
	Assert(t, runningJobs[0].ID() == fetchedReq.UUID, fmt.Sprintf("expected job ID %v", fetchedReq.UUID))
	// cancel request
	_, err = svc.CancelRequest(context.TODO(), req)
	AssertOk(t, err, "canceling archive request failed")
	AssertEventually(t, func() (bool, interface{}) {
		fetchedReq, err = ti.apicl.MonitoringV1().ArchiveRequest().Get(context.TODO(), &req.ObjectMeta)
		if err != nil {
			return false, err
		}
		if fetchedReq.Status.Status != monitoring.ArchiveRequestStatus_Canceled.String() {
			return false, fetchedReq
		}
		return true, nil
	}, fmt.Sprintf("archive request is not in canceled state: %#v", *fetchedReq))

	AssertEventually(t, func() (bool, interface{}) {
		if len(svcImpl.queue.ListJobs()) != 0 {
			return false, nil
		}
		return true, nil
	}, fmt.Sprintf("unexpected job found in queue: %v", testutils.PrintJobs(svcImpl.queue.ListJobs())))
	// cancel again should return error
	_, err = svc.CancelRequest(context.TODO(), req)
	Assert(t, reflect.DeepEqual(err, errors.New("cannot cancel archive request it was already processed")), fmt.Sprintf("unexpected error cancelling non-running job: %v", err))
}

func TestStopStartService(t *testing.T) {
	ti := &tInfo{}
	ti.setup()
	defer ti.shutdown()
	mockJobFn := func(req *monitoring.ArchiveRequest, exporter archive.Exporter, rslver resolver.Interface, logger log.Logger) archive.Job {
		return mock.GetJob(req)
	}
	svc := GetService(clientName, ti.apiSrvAddr, nil, ti.logger, mockJobFn, WithExporter(monitoring.ArchiveRequestSpec_AuditEvent.String(), mock.GetExporter()))
	svcImpl := svc.(*serviceImpl)

	req := &monitoring.ArchiveRequest{}
	req.Defaults("All")
	req.Name = "test1"
	req.Spec.Type = monitoring.ArchiveRequestSpec_AuditEvent.String()
	req, err := testutils.CreateArchiveRequest(context.TODO(), ti.apicl, req)
	AssertOk(t, err, "failed to create archive request")
	fetchedReq := req
	AssertEventually(t, func() (bool, interface{}) {
		fetchedReq, err = ti.apicl.MonitoringV1().ArchiveRequest().Get(context.TODO(), &req.ObjectMeta)
		if err != nil {
			return false, err
		}
		if fetchedReq.Status.Status != monitoring.ArchiveRequestStatus_Running.String() {
			return false, fetchedReq
		}
		return true, nil
	}, fmt.Sprintf("archive request is not in running state: %#v", *fetchedReq))

	defer testutils.MustDeleteArchiveRequest(context.TODO(), ti.apicl, &req.ObjectMeta)
	// check running jobs
	runningJobs := svcImpl.queue.ListJobs()
	Assert(t, len(runningJobs) == 1, fmt.Sprintf("no running jobs in queue %#v", runningJobs))
	Assert(t, runningJobs[0].ID() == fetchedReq.UUID, fmt.Sprintf("expected job ID %v", fetchedReq.UUID))
	svc.Stop()
	svc.Start()
	defer svc.Stop()
	AssertConsistently(t, func() (bool, interface{}) {
		runningJobs := svcImpl.queue.ListJobs()
		if len(runningJobs) != 0 {
			return false, fmt.Errorf("there is a  running job in queue %#v", runningJobs)
		}
		return true, nil
	}, "job queue should not pick a canceled job", "10ms", "100ms")
	AssertEventually(t, func() (bool, interface{}) {
		fetchedReq, err = ti.apicl.MonitoringV1().ArchiveRequest().Get(context.TODO(), &req.ObjectMeta)
		if err != nil {
			return false, err
		}
		if fetchedReq.Status.Status != monitoring.ArchiveRequestStatus_Canceled.String() {
			return false, fetchedReq
		}
		return true, nil
	}, fmt.Sprintf("archive request is not in canceled state: %#v", *fetchedReq))
}

func TestDuplicateRequest(t *testing.T) {
	ti := &tInfo{}
	ti.setup()
	defer ti.shutdown()
	mockJobFn := func(req *monitoring.ArchiveRequest, exporter archive.Exporter, rslver resolver.Interface, logger log.Logger) archive.Job {
		return mock.GetJob(req)
	}
	svc := GetService(clientName, ti.apiSrvAddr, nil, ti.logger, mockJobFn, WithExporter(monitoring.ArchiveRequestSpec_AuditEvent.String(), mock.GetExporter()))
	defer svc.Stop()
	svcImpl := svc.(*serviceImpl)
	Assert(t, !svcImpl.stopped, "service should not be in a stopped state")
	jq := svcImpl.queue.(*jobQueue)
	Assert(t, !jq.stopped, "job queue should not be in a stopped state")
	req1 := &monitoring.ArchiveRequest{}
	req1.Defaults("All")
	req1.Name = "test1"
	req1.Spec.Type = monitoring.ArchiveRequestSpec_AuditEvent.String()
	req1, err := testutils.CreateArchiveRequest(context.TODO(), ti.apicl, req1)
	AssertOk(t, err, "failed to create archive request")
	fetchedReq := req1
	AssertEventually(t, func() (bool, interface{}) {
		fetchedReq, err = ti.apicl.MonitoringV1().ArchiveRequest().Get(context.TODO(), &req1.ObjectMeta)
		if err != nil {
			return false, err
		}
		if fetchedReq.Status.Status != monitoring.ArchiveRequestStatus_Running.String() {
			return false, fetchedReq
		}
		return true, nil
	}, fmt.Sprintf("archive request is not in running state: %#v", *fetchedReq))

	defer testutils.MustDeleteArchiveRequest(context.TODO(), ti.apicl, &req1.ObjectMeta)
	// check running jobs
	runningJobs := svcImpl.queue.ListJobs()
	Assert(t, len(runningJobs) == 1, fmt.Sprintf("no running jobs in queue %#v", runningJobs))
	Assert(t, runningJobs[0].ID() == fetchedReq.UUID, fmt.Sprintf("expected job ID %v", fetchedReq.UUID))
	// create a duplicate request to archive audit events
	req2 := &monitoring.ArchiveRequest{}
	req2.Defaults("All")
	req2.Name = "test2"
	req2.Spec.Type = monitoring.ArchiveRequestSpec_AuditEvent.String()
	req2, err = testutils.CreateArchiveRequest(context.TODO(), ti.apicl, req2)
	AssertOk(t, err, "failed to create duplicate archive request")
	AssertEventually(t, func() (bool, interface{}) {
		fetchedReq, err = ti.apicl.MonitoringV1().ArchiveRequest().Get(context.TODO(), &req2.ObjectMeta)
		if err != nil {
			return false, err
		}
		if fetchedReq.Status.Status == monitoring.ArchiveRequestStatus_Failed.String() &&
			fetchedReq.Status.Reason == fmt.Sprintf("archive request [%s] already running for log type %s", req1.Name, req1.Spec.Type) {
			return true, fetchedReq
		}
		return false, nil
	}, fmt.Sprintf("archive request is not in failed state: %#v", *fetchedReq))
	defer testutils.MustDeleteArchiveRequest(context.TODO(), ti.apicl, &req2.ObjectMeta)
	// cancel first request
	_, err = svc.CancelRequest(context.TODO(), req1)
	AssertOk(t, err, "canceling archive request failed")
	AssertEventually(t, func() (bool, interface{}) {
		fetchedReq, err = ti.apicl.MonitoringV1().ArchiveRequest().Get(context.TODO(), &req1.ObjectMeta)
		if err != nil {
			return false, err
		}
		if fetchedReq.Status.Status != monitoring.ArchiveRequestStatus_Canceled.String() {
			return false, fetchedReq
		}
		return true, nil
	}, fmt.Sprintf("archive request is not in canceled state: %#v", *fetchedReq))
	AssertEventually(t, func() (bool, interface{}) {
		if len(svcImpl.queue.ListJobs()) != 0 {
			return false, nil
		}
		return true, nil
	}, fmt.Sprintf("unexpected job found in queue: %v", testutils.PrintJobs(svcImpl.queue.ListJobs())))
}
