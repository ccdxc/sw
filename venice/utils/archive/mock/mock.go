package mock

import (
	"context"
	"errors"
	"time"

	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/archive"
)

type mockArchiveService struct{}

// RegisterExporter registers an exporter
func (m *mockArchiveService) RegisterExporter(name string, exporter archive.Exporter) error {
	return nil
}

// UnregisterExporter unregisters an exporter
func (m *mockArchiveService) UnregisterExporter(name string) archive.Exporter {
	return nil
}

// CancelRequest cancels a running job. Returns an error if job is not found
func (m *mockArchiveService) CancelRequest(ctx context.Context, in *monitoring.ArchiveRequest) (*monitoring.ArchiveRequest, error) {
	return in, nil
}

// Start starts the archive service
func (m *mockArchiveService) Start() {}

// Stop stops the archive service
func (m *mockArchiveService) Stop() {}

// GetMockArchiveService returns a mock instance of archive service
func GetMockArchiveService() archive.Service {
	return &mockArchiveService{}
}

type client struct {
	simulateError bool
}

func (c *client) Close() {}

func (c *client) CancelRequest(ctx context.Context, in *monitoring.ArchiveRequest, opts ...grpc.CallOption) (*monitoring.ArchiveRequest, error) {
	if c.simulateError {
		return nil, errors.New("simulated CancelRequest error")
	}
	return in, nil
}

// GetClientGetter returns mock client getter
func GetClientGetter(simulateCancelRequestError, simulateClientError bool) archive.ClientGetter {
	var clgetter archive.ClientGetterFunc
	clgetter = func() (archive.Client, error) {
		if simulateClientError {
			return nil, errors.New("simulated client error")
		}
		return &client{simulateError: simulateCancelRequestError}, nil
	}
	return clgetter
}

type archiveGetter struct {
	simulateError bool
	req           *monitoring.ArchiveRequest
}

func (a *archiveGetter) GetArchiveRequest(ometa *api.ObjectMeta) (*monitoring.ArchiveRequest, error) {
	if a.simulateError {
		return nil, errors.New("simulated archive error")
	}
	req := *a.req
	req.ObjectMeta = *ometa
	return &req, nil
}

// GetArchiveGetter returns mock archive getter
func GetArchiveGetter(req *monitoring.ArchiveRequest, simulateError bool) archive.Getter {
	return &archiveGetter{
		req:           req,
		simulateError: simulateError,
	}
}

type mockJob struct {
	ctx    context.Context
	cancel context.CancelFunc
	req    *monitoring.ArchiveRequest
}

func (j *mockJob) ID() string {
	return j.req.UUID
}

func (j *mockJob) Cancel() {
	if j.cancel != nil {
		j.cancel()
	}
}

func (j *mockJob) Run(ctx context.Context) error {
	nctx, cancel := context.WithCancel(ctx)
	j.ctx = nctx
	j.cancel = cancel
	for {
		select {
		case <-j.ctx.Done():
			j.req.Status.Status = monitoring.ArchiveRequestStatus_Canceled.String()
			j.req.Status.Reason = j.ctx.Err().Error()
			return j.ctx.Err()
		default:
			time.Sleep(time.Second)
		}
	}
}

func (j *mockJob) GetArchiveRequest() *monitoring.ArchiveRequest {
	return j.req
}

// GetJob returns a mock job
func GetJob(req *monitoring.ArchiveRequest) archive.Job {
	return &mockJob{
		req: req,
	}
}

type mockExporter struct{}

func (e *mockExporter) Export(ctx context.Context, reader archive.TransformReader, meta map[string]string) (string, int64, error) {
	return "", 0, nil
}

func (e *mockExporter) Delete(archiveReq *monitoring.ArchiveRequest) error {
	return nil
}

// GetExporter returns a mock archive exporter
func GetExporter() archive.Exporter {
	return &mockExporter{}
}
