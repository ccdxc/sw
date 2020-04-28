package vos

import (
	"context"
	"io"

	minioclient "github.com/minio/minio-go/v6"

	"github.com/pensando/sw/api/generated/objstore"
)

// Interface defines the interface to a VOS instance
type Interface interface {
	RegisterCb(bucket string, stage OperStage, oper ObjectOper, cb CallBackFunc)
}

// BackendClient is an interface representing the backend minio service
type BackendClient interface {
	BaseBackendClient
	ExtBackendClient
}

// BaseBackendClient is an interface providing base minio functionality
type BaseBackendClient interface {
	BucketExists(bucketName string) (bool, error)
	MakeBucket(bucketName string, location string) error
	RemoveBucket(bucketName string) error

	PutObject(bucketName, objectName string, reader io.Reader, objectSize int64, opts minioclient.PutObjectOptions) (n int64, err error)
	RemoveObject(bucketName, objectName string) error
	ListObjectsV2(bucketName, objectPrefix string, recursive bool, doneCh <-chan struct{}) <-chan minioclient.ObjectInfo
	StatObject(bucketName, objectName string, opts minioclient.StatObjectOptions) (minioclient.ObjectInfo, error)
	GetObjectWithContext(ctx context.Context, bucketName, objectName string, opts minioclient.GetObjectOptions) (*minioclient.Object, error)
	ListenBucketNotification(bucketName, prefix, suffix string, events []string, doneCh <-chan struct{}) <-chan minioclient.NotificationInfo
}

// ExtBackendClient exposes extensions to the backend client
type ExtBackendClient interface {
	GetStoreObject(ctx context.Context, bucketName, objectName string, opts minioclient.GetObjectOptions) (StoreObject, error)
}

// StoreObject is a wrapper around the minio.Object to facilitate testing
type StoreObject interface {
	Close() (err error)
	Stat() (minioclient.ObjectInfo, error)
	Read(b []byte) (n int, err error)
}

// CallBackFunc defines a plugin callback function
type CallBackFunc func(ctx context.Context, oper ObjectOper, in *objstore.Object, client BackendClient) error

// ObjectOper is all operations supported on an Object
type ObjectOper string

// Values supported for ObjectOper
const (
	Upload   = ObjectOper("upload")
	Download = ObjectOper("download")
	Get      = ObjectOper("et")
	List     = ObjectOper("list")
	Delete   = ObjectOper("delete")
	Update   = ObjectOper("update")
	Watch    = ObjectOper("watch")
)

// OperStage is stage of operation
type OperStage string

// Stages of an operation
const (
	PreOp  = OperStage("pre")
	PostOp = OperStage("post")
)
