package archive

import (
	"context"
	"io"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/archive/protos"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// Exporter is an abstraction to save data to a store
type Exporter interface {
	// Export saves data to a store and returns URI and size of the object
	Export(ctx context.Context, reader TransformReader, meta map[string]string) (string, int64, error)
	// Deletes data in store to do cleanup when archive job is cancelled
	Delete(archiveReq *monitoring.ArchiveRequest) error
}

// Service is an archive service that orchestrates saving data to multiple data stores
type Service interface {
	protos.ArchiveServer
	// RegisterExporter registers an exporter
	RegisterExporter(name string, exporter Exporter) error
	// UnregisterExporter unregisters an exporter
	UnregisterExporter(name string) Exporter
	// Start starts the archive service
	Start()
	// Stop stops the archive service
	Stop()
}

// CreateJobCb is a callback to create an export job based on archive request
type CreateJobCb func(req *monitoring.ArchiveRequest, exporter Exporter, rslver resolver.Interface, logger log.Logger) Job

// TransformReader is an abstraction to transform an input before it is saved to a store
type TransformReader interface {
	io.Reader
}

// Job is an abstraction for archive job
type Job interface {
	ID() string
	Cancel()
	Run(ctx context.Context) error
	GetArchiveRequest() *monitoring.ArchiveRequest
}

// JobQueue manages archive jobs
type JobQueue interface {
	CancelJob(id string) (Job, error)
	CancelJobs()
	AddJob(Job) error
	ListJobs() []Job
	Start()
	Stop()
}

// ClientGetter interface abstracts out archive Client creation
type ClientGetter interface {
	// GetClient returns an implementation of archive Client
	GetClient() (Client, error)
}

// ClientGetterFunc implements ClientGetter interface
type ClientGetterFunc func() (Client, error)

// GetClient implements method for ClientGetter interface
func (f ClientGetterFunc) GetClient() (Client, error) {
	return f()
}

// Client abstracts out rpc client to cancel archive request
type Client interface {
	protos.ArchiveClient
	Close()
}

// Getter is an interface to fetch ArchiveRequest object
type Getter interface {
	// GetArchiveRequest returns ArchiveRequest object given the key
	GetArchiveRequest(ometa *api.ObjectMeta) (*monitoring.ArchiveRequest, error)
}
