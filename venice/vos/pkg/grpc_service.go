package vos

import (
	"context"
	"strings"

	"github.com/pkg/errors"
	"google.golang.org/grpc/codes"

	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"

	"github.com/minio/minio-go"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/globals"
)

func newGrpcServer(client backendClient) (*grpcBackend, error) {
	log.InfoLog("msg", "creating new gRPC backend", "port", globals.VosGRPcPort)
	grpcSvc, err := rpckit.NewRPCServer(globals.Vos, ":"+globals.VosGRPcPort)
	if err != nil {
		return nil, err
	}
	ret := &grpcBackend{client: client, server: grpcSvc}
	objstore.RegisterObjstoreV1Server(grpcSvc.GrpcServer, ret)

	return ret, nil
}

type grpcBackend struct {
	client backendClient
	server *rpckit.RPCServer
}

func (g *grpcBackend) start(ctx context.Context) {
	log.InfoLog("msg", "starting gRPC backend")
	g.server.Start()
	done := make(chan error)
	go func() {
		close(done)
		<-ctx.Done()
		g.server.Stop()
	}()
	<-done
}

var errNotImplemented = errors.New("Not implemented")

func (g *grpcBackend) validateNamespace(in *objstore.Object) (string, error) {
	if _, ok := objstore.Buckets_value[strings.ToLower(in.Namespace)]; !ok {
		str := ""
		for k := range objstore.Buckets_value {
			str = str + "," + k
		}
		return str, errors.New("invalid type")
	}
	return "", nil
}

func (g *grpcBackend) AutoAddBucket(ctx context.Context, in *objstore.Bucket) (*objstore.Bucket, error) {
	log.Infof("got call to AutoAddBucket")
	if g.client == nil {
		return nil, errors.New("not initialized")
	}
	if in == nil || in.Name == "" {
		return nil, apierrors.ToGrpcError(errors.New("invalid input"), []string{"invalid inputs"}, int32(codes.InvalidArgument), "", nil)
	}

	name := in.Tenant + "." + in.Name
	if in.Tenant == "" {
		name = "default." + in.Name
	}
	ok, err := g.client.BucketExists(name)
	if err != nil {
		return nil, apierrors.ToGrpcError(err, []string{"client error"}, int32(codes.Internal), "", nil)
	}
	if ok {
		return nil, apierrors.ToGrpcError(errors.New("bucket already exists"), []string{"bucket exists"}, int32(codes.AlreadyExists), "", nil)
	}
	// XXX-TODO(sanjayt): add a internal file in the store with the contents of the Spec.
	err = g.client.MakeBucket(name, defaultLocation)
	if err != nil {
		return nil, apierrors.ToGrpcError(err, []string{"bucket creation"}, int32(codes.Internal), "", nil)
	}
	ret := *in
	return &ret, nil
}

// Delete Bucket object
func (g *grpcBackend) AutoDeleteBucket(ctx context.Context, in *objstore.Bucket) (*objstore.Bucket, error) {
	log.Infof("got call to AutoDeleteBucket")
	if g.client == nil {
		return nil, errors.New("not initialized")
	}
	if in == nil || in.Name == "" {
		return nil, apierrors.ToGrpcError(errors.New("invalid input"), []string{"invalid inputs"}, int32(codes.InvalidArgument), "", nil)
	}
	name := in.Tenant + "." + in.Name
	if in.Tenant == "" {
		name = "default." + in.Name
	}
	ok, err := g.client.BucketExists(name)
	if err != nil {
		return nil, apierrors.ToGrpcError(err, []string{"client error"}, int32(codes.Internal), "", nil)
	}
	if !ok {
		return nil, apierrors.ToGrpcError(errors.New("does not exist"), []string{"bucket does not exist"}, int32(codes.NotFound), "", nil)
	}

	err = g.client.RemoveBucket(name)
	if err != nil {
		return nil, apierrors.ToGrpcError(err, []string{"bucket deletion"}, int32(codes.Internal), "", nil)
	}
	ret := *in
	return &ret, nil
}

// Get Bucket object
func (g *grpcBackend) AutoGetBucket(ctx context.Context, in *objstore.Bucket) (*objstore.Bucket, error) {
	log.Infof("got call to AutoGetBucket")
	return nil, errNotImplemented
}

// List Bucket objects
func (g *grpcBackend) AutoListBucket(ctx context.Context, in *api.ListWatchOptions) (*objstore.BucketList, error) {
	log.Infof("got call to AutoListBucket")
	return nil, errNotImplemented
}

// Update Bucket object
func (g *grpcBackend) AutoUpdateBucket(context.Context, *objstore.Bucket) (*objstore.Bucket, error) {
	log.Infof("got call to AutoUpdateBucket")
	return nil, errNotImplemented
}

// Watch Bucket objects. Supports WebSockets or HTTP long poll
func (g *grpcBackend) AutoWatchBucket(*api.ListWatchOptions, objstore.ObjstoreV1_AutoWatchBucketServer) error {
	log.Infof("got call to AutoWatchBucket")
	return errNotImplemented
}

// Create Object object
func (g *grpcBackend) AutoAddObject(context.Context, *objstore.Object) (*objstore.Object, error) {
	log.Infof("got call to AutoAddObject")
	return nil, errNotImplemented
}

// Delete Object object
func (g *grpcBackend) AutoDeleteObject(ctx context.Context, in *objstore.Object) (*objstore.Object, error) {
	log.Infof("got call to AutoDeleteObject")
	if g.client == nil {
		return nil, errors.New("not initialized")
	}
	if in == nil || in.Name == "" {
		return nil, apierrors.ToGrpcError(errors.New("invalid input"), []string{"invalid inputs"}, int32(codes.InvalidArgument), "", nil)
	}
	if str, err := g.validateNamespace(in); err != nil {
		return nil, apierrors.ToGrpcError(err, []string{"valid types: " + str}, int32(codes.InvalidArgument), "", nil)
	}
	bucket := in.Tenant + "." + in.Namespace
	if in.Tenant == "" {
		bucket = "default." + in.Namespace
	}
	obj, err := g.client.StatObject(bucket, in.Name, minio.StatObjectOptions{})
	if err != nil {
		return nil, apierrors.ToGrpcError(err, []string{"failed to delete object"}, int32(codes.NotFound), "", nil)
	}
	err = g.client.RemoveObject(bucket, in.Name)
	if err != nil {
		return nil, apierrors.ToGrpcError(err, []string{"failed to delete object"}, int32(codes.NotFound), "", nil)
	}
	ret := &objstore.Object{}
	ret.Defaults("v1")
	ret.ObjectMeta = in.ObjectMeta
	updateObjectMeta(&obj, &ret.ObjectMeta)
	ret.Spec.ContentType = obj.ContentType
	ret.Status.Size_ = obj.Size
	ret.Status.Digest = obj.ETag
	return ret, nil
}

// Get Object object
func (g *grpcBackend) AutoGetObject(ctx context.Context, in *objstore.Object) (*objstore.Object, error) {
	log.Infof("got call to AutoGetObject")
	if g.client == nil {
		return nil, errors.New("not initialized")
	}
	if in == nil || in.Name == "" {
		return nil, apierrors.ToGrpcError(errors.New("invalid input"), []string{"invalid inputs"}, int32(codes.InvalidArgument), "", nil)
	}
	if str, err := g.validateNamespace(in); err != nil {
		return nil, apierrors.ToGrpcError(err, []string{"valid types: " + str}, int32(codes.InvalidArgument), "", nil)
	}
	bucket := in.Tenant + "." + in.Namespace
	if in.Tenant == "" {
		bucket = "default." + in.Namespace
	}
	obj, err := g.client.StatObject(bucket, in.Name, minio.StatObjectOptions{})
	if err != nil {
		return nil, apierrors.ToGrpcError(err, []string{"failed to get object"}, int32(codes.NotFound), "", nil)
	}
	ret := &objstore.Object{}
	ret.Defaults("v1")
	ret.ObjectMeta = in.ObjectMeta
	updateObjectMeta(&obj, &ret.ObjectMeta)
	ret.Spec.ContentType = obj.ContentType
	ret.Status.Size_ = obj.Size
	ret.Status.Digest = obj.ETag
	return ret, nil
}

// List Object objects
func (g *grpcBackend) AutoListObject(ctx context.Context, in *api.ListWatchOptions) (*objstore.ObjectList, error) {
	log.Infof("got call to AutoListObject [%+v]", in)
	if g.client == nil {
		return nil, errors.New("not initialized")
	}
	if in == nil {
		return nil, apierrors.ToGrpcError(errors.New("invalid input"), []string{"invalid inputs"}, int32(codes.InvalidArgument), "", nil)
	}
	t := objstore.Object{}
	t.Namespace = in.Namespace
	if str, err := g.validateNamespace(&t); err != nil {
		return nil, apierrors.ToGrpcError(err, []string{"valid types: " + str}, int32(codes.InvalidArgument), "", nil)
	}
	bucket := in.Tenant + "." + in.Namespace
	if in.Tenant == "" {
		bucket = "default." + in.Namespace
	}
	// Options are ignore for now.
	ret := &objstore.ObjectList{
		TypeMeta: api.TypeMeta{Kind: "ObjectList"},
	}
	doneCh := make(chan struct{})
	objCh := g.client.ListObjectsV2(bucket, "", true, doneCh)
	for mobj := range objCh {
		// List does not seem to be returing with UserMeta populated. Workaround by doing a stat.
		stat, err := g.client.StatObject(bucket, mobj.Key, minio.StatObjectOptions{})
		if err != nil {
			log.Errorf("failed to get stat for object [%v.%v](%s)", bucket, mobj.Key, err)
			continue
		}
		lobj := &objstore.Object{
			TypeMeta:   api.TypeMeta{Kind: "Object"},
			ObjectMeta: api.ObjectMeta{Name: mobj.Key},
			Spec:       objstore.ObjectSpec{ContentType: stat.ContentType},
			Status: objstore.ObjectStatus{
				Size_:  stat.Size,
				Digest: stat.ETag,
			},
		}
		updateObjectMeta(&stat, &lobj.ObjectMeta)
		ret.Items = append(ret.Items, lobj)
	}
	close(doneCh)
	return ret, nil
}

// Update Object object
func (g *grpcBackend) AutoUpdateObject(context.Context, *objstore.Object) (*objstore.Object, error) {
	log.Infof("got call to AutoUpdateObject")
	return nil, errNotImplemented
}

// Watch Object objects. Supports WebSockets or HTTP long poll

func (g *grpcBackend) AutoWatchObject(*api.ListWatchOptions, objstore.ObjstoreV1_AutoWatchObjectServer) error {
	log.Infof("got call to AutoWatchObject")
	return errNotImplemented
}
func (g *grpcBackend) AutoWatchSvcObjstoreV1(*api.ListWatchOptions, objstore.ObjstoreV1_AutoWatchSvcObjstoreV1Server) error {
	log.Infof("got call to AutoWatchSvcObjstoreV1")
	return errNotImplemented
}
