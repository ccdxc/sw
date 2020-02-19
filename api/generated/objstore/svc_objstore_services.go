// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package objstore is a auto generated package.
Input file: svc_objstore.proto
*/
package objstore

import (
	"context"

	"github.com/pensando/sw/api"
)

// Dummy definitions to suppress nonused warnings
var _ api.ObjectMeta

// ServiceObjstoreV1Client  is the client interface for the service.
type ServiceObjstoreV1Client interface {
	AutoWatchSvcObjstoreV1(ctx context.Context, in *api.ListWatchOptions) (ObjstoreV1_AutoWatchSvcObjstoreV1Client, error)

	AutoAddBucket(ctx context.Context, t *Bucket) (*Bucket, error)
	AutoAddObject(ctx context.Context, t *Object) (*Object, error)
	AutoDeleteBucket(ctx context.Context, t *Bucket) (*Bucket, error)
	AutoDeleteObject(ctx context.Context, t *Object) (*Object, error)
	AutoGetBucket(ctx context.Context, t *Bucket) (*Bucket, error)
	AutoGetObject(ctx context.Context, t *Object) (*Object, error)
	AutoLabelBucket(ctx context.Context, t *api.Label) (*Bucket, error)
	AutoLabelObject(ctx context.Context, t *api.Label) (*Object, error)
	AutoListBucket(ctx context.Context, t *api.ListWatchOptions) (*BucketList, error)
	AutoListObject(ctx context.Context, t *api.ListWatchOptions) (*ObjectList, error)
	AutoUpdateBucket(ctx context.Context, t *Bucket) (*Bucket, error)
	AutoUpdateObject(ctx context.Context, t *Object) (*Object, error)

	AutoWatchBucket(ctx context.Context, in *api.ListWatchOptions) (ObjstoreV1_AutoWatchBucketClient, error)
	AutoWatchObject(ctx context.Context, in *api.ListWatchOptions) (ObjstoreV1_AutoWatchObjectClient, error)

	DownloadFile(ctx context.Context, in *Object) (ObjstoreV1_DownloadFileClient, error)
	DownloadFileByPrefix(ctx context.Context, in *Object) (ObjstoreV1_DownloadFileByPrefixClient, error)
}

// ServiceObjstoreV1Server is the server interface for the service.
type ServiceObjstoreV1Server interface {
	AutoWatchSvcObjstoreV1(in *api.ListWatchOptions, stream ObjstoreV1_AutoWatchSvcObjstoreV1Server) error

	AutoAddBucket(ctx context.Context, t Bucket) (Bucket, error)
	AutoAddObject(ctx context.Context, t Object) (Object, error)
	AutoDeleteBucket(ctx context.Context, t Bucket) (Bucket, error)
	AutoDeleteObject(ctx context.Context, t Object) (Object, error)
	AutoGetBucket(ctx context.Context, t Bucket) (Bucket, error)
	AutoGetObject(ctx context.Context, t Object) (Object, error)
	AutoLabelBucket(ctx context.Context, t api.Label) (Bucket, error)
	AutoLabelObject(ctx context.Context, t api.Label) (Object, error)
	AutoListBucket(ctx context.Context, t api.ListWatchOptions) (BucketList, error)
	AutoListObject(ctx context.Context, t api.ListWatchOptions) (ObjectList, error)
	AutoUpdateBucket(ctx context.Context, t Bucket) (Bucket, error)
	AutoUpdateObject(ctx context.Context, t Object) (Object, error)

	AutoWatchBucket(in *api.ListWatchOptions, stream ObjstoreV1_AutoWatchBucketServer) error
	AutoWatchObject(in *api.ListWatchOptions, stream ObjstoreV1_AutoWatchObjectServer) error

	DownloadFile(*Object, ObjstoreV1_DownloadFileServer) error
	DownloadFileByPrefix(*Object, ObjstoreV1_DownloadFileByPrefixServer) error
}
