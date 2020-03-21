package vospkg

import (
	"archive/zip"
	"context"
	"fmt"
	"io"
	"strings"

	"github.com/minio/minio-go"
	"github.com/pkg/errors"
	"google.golang.org/grpc/codes"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/vos"
	vosinternalprotos "github.com/pensando/sw/venice/vos/protos"
)

func newGrpcServer(instance *instance, client vos.BackendClient) (*grpcBackend, error) {
	log.InfoLog("msg", "creating new gRPC backend", "port", globals.VosGRPcPort)
	grpcSvc, err := rpckit.NewRPCServer(globals.Vos, ":"+globals.VosGRPcPort)
	if err != nil {
		return nil, err
	}
	ret := &grpcBackend{client: client, server: grpcSvc, instance: instance}
	objstore.RegisterObjstoreV1Server(grpcSvc.GrpcServer, ret)
	vosinternalprotos.RegisterObjstoreInternalServiceServer(grpcSvc.GrpcServer, ret)

	return ret, nil
}

type grpcBackend struct {
	client   vos.BackendClient
	server   *rpckit.RPCServer
	instance *instance
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
	// The namespace name will be considered valid if its prefix matches the
	// namespaces defined in the model
	if _, ok := objstore.Buckets_value[strings.ToLower(in.Namespace)]; !ok {
		// For fwlogs, all the buckets are prefixed with 'fwlogs' keyword, its not the
		// tenant name but it comes populated as tenantName in the objectstore object.
		// This also means that fwlogs name is not reserved and cannot be used for
		// creating a tenant.
		if _, ok := objstore.Buckets_value[strings.ToLower(in.Tenant)]; !ok {
			str := ""
			for k := range objstore.Buckets_value {
				str = str + "," + k
			}
			return str, errors.New("invalid type")
		}
	}
	return "", nil
}

func (g *grpcBackend) AutoAddBucket(ctx context.Context, in *objstore.Bucket) (*objstore.Bucket, error) {
	log.Infof("got call to AutoAddBucket")
	if g.client == nil {
		return nil, errors.New("not initialized")
	}
	if in == nil || in.Name == "" {
		return nil, apierrors.ToGrpcError("invalid input", []string{"invalid inputs"}, int32(codes.InvalidArgument), "", nil)
	}

	name := in.Tenant + "." + in.Name
	if in.Tenant == "" {
		name = "default." + in.Name
	}
	ok, err := g.client.BucketExists(name)
	if err != nil {
		return nil, apierrors.ToGrpcError("client error", []string{err.Error()}, int32(codes.Internal), "", nil)
	}
	if ok {
		return nil, apierrors.ToGrpcError("bucket already exists", []string{"bucket exists"}, int32(codes.AlreadyExists), "", nil)
	}
	// XXX-TODO(sanjayt): add a internal file in the store with the contents of the Spec.
	err = g.instance.createBucket(name)
	if err != nil {
		return nil, apierrors.ToGrpcError("bucket creation", []string{err.Error()}, int32(codes.Internal), "", nil)
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
		return nil, apierrors.ToGrpcError("invalid input", []string{"invalid inputs"}, int32(codes.InvalidArgument), "", nil)
	}
	name := in.Tenant + "." + in.Name
	if in.Tenant == "" {
		name = "default." + in.Name
	}
	ok, err := g.client.BucketExists(name)
	if err != nil {
		return nil, apierrors.ToGrpcError("client error", []string{err.Error()}, int32(codes.Internal), "", nil)
	}
	if !ok {
		return nil, apierrors.ToGrpcError("does not exist", []string{"bucket does not exist"}, int32(codes.NotFound), "", nil)
	}
	// We are not currently keeping a ref count and removing the plugin set on reaching 0, as the complication seems unneeded.
	err = g.client.RemoveBucket(name)
	if err != nil {
		return nil, apierrors.ToGrpcError("bucket deletion error", []string{err.Error()}, int32(codes.Internal), "", nil)
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

// Label Bucket object
func (g *grpcBackend) AutoLabelBucket(context.Context, *api.Label) (*objstore.Bucket, error) {
	log.Infof("got call to AutoLabelBucket")
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
		return nil, apierrors.ToGrpcError("invalid input", []string{"invalid inputs"}, int32(codes.InvalidArgument), "", nil)
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
		return nil, apierrors.ToGrpcError("failed to delete object", []string{err.Error()}, int32(codes.NotFound), "", nil)
	}
	errs := g.instance.RunPlugins(ctx, in.Namespace, vos.PreOp, vos.Delete, in, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}
		return nil, apierrors.ToGrpcError("failed to complete PreOp checks", strs, int32(codes.FailedPrecondition), "", nil)
	}
	err = g.client.RemoveObject(bucket, in.Name)
	if err != nil {
		return nil, apierrors.ToGrpcError("failed to delete object", []string{err.Error()}, int32(codes.NotFound), "", nil)
	}
	errs = g.instance.RunPlugins(ctx, in.Namespace, vos.PostOp, vos.Delete, in, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}
		return nil, apierrors.ToGrpcError("failed to complete PostOp checks", strs, int32(codes.Internal), "", nil)
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
		return nil, apierrors.ToGrpcError("invalid input", []string{"invalid inputs"}, int32(codes.InvalidArgument), "", nil)
	}
	if str, err := g.validateNamespace(in); err != nil {
		return nil, apierrors.ToGrpcError(err, []string{"valid types: " + str}, int32(codes.InvalidArgument), "", nil)
	}
	bucket := in.Tenant + "." + in.Namespace
	if in.Tenant == "" {
		bucket = "default." + in.Namespace
	}
	errs := g.instance.RunPlugins(ctx, in.Namespace, vos.PreOp, vos.Get, in, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}
		return nil, apierrors.ToGrpcError("failed to complete PreOp checks", strs, int32(codes.FailedPrecondition), "", nil)
	}
	obj, err := g.client.StatObject(bucket, in.Name, minio.StatObjectOptions{})
	if err != nil {
		return nil, apierrors.ToGrpcError("failed to get object", []string{err.Error()}, int32(codes.NotFound), "", nil)
	}
	errs = g.instance.RunPlugins(ctx, in.Namespace, vos.PostOp, vos.Get, in, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}
		return nil, apierrors.ToGrpcError("failed to complete PostOp checks", strs, int32(codes.Internal), "", nil)
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
		return nil, errors.New("ListWatchOptions is nil")
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
	errs := g.instance.RunPlugins(ctx, in.Namespace, vos.PreOp, vos.List, nil, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}
		return nil, apierrors.ToGrpcError("failed to complete PreOp checks", strs, int32(codes.FailedPrecondition), "", nil)
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
	errs = g.instance.RunPlugins(ctx, in.Namespace, vos.PostOp, vos.List, nil, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}
		return nil, apierrors.ToGrpcError("failed to complete PostOp checks", strs, int32(codes.Internal), "", nil)
	}
	return ret, nil
}

// Update Object object
func (g *grpcBackend) AutoUpdateObject(context.Context, *objstore.Object) (*objstore.Object, error) {
	log.Infof("got call to AutoUpdateObject")
	return nil, errNotImplemented
}

// Label Object object
func (g *grpcBackend) AutoLabelObject(context.Context, *api.Label) (*objstore.Object, error) {
	log.Infof("got call to AutoLabelObject")
	return nil, errNotImplemented
}

// Watch Object objects. Supports WebSockets or HTTP long poll
func (g *grpcBackend) AutoWatchObject(opts *api.ListWatchOptions, stream objstore.ObjstoreV1_AutoWatchObjectServer) error {
	log.Infof("got call to AutoWatchObject")
	peer := ctxutils.GetContextID(stream.Context())
	bucket := opts.Tenant + "." + opts.Namespace
	if opts.ResourceVersion != "" ||
		opts.LabelSelector != "" ||
		opts.FieldSelector != "" ||
		(bucket != fwlogsBucketName && opts.FieldChangeSelector != nil) {
		return errors.New("filtering is not supported")
	}
	errs := g.instance.RunPlugins(stream.Context(), opts.Namespace, vos.PreOp, vos.Watch, nil, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}
		return apierrors.ToGrpcError("failed to complete PreOp checks", strs, int32(codes.FailedPrecondition), "", nil)
	}
	handleFn := func(inctx context.Context, evType kvstore.WatchEventType, item, prev runtime.Object) {
		evs := objstore.AutoMsgObjectWatchHelper{}
		evs.Events = []*objstore.AutoMsgObjectWatchHelper_WatchEvent{
			{
				Type:   string(evType),
				Object: item.(*objstore.Object),
			},
		}
		stream.Send(&evs)
	}
	err := g.instance.Watch(stream.Context(), bucket, peer, handleFn, opts)
	return err
}

func (g *grpcBackend) AutoWatchSvcObjstoreV1(*api.ListWatchOptions, objstore.ObjstoreV1_AutoWatchSvcObjstoreV1Server) error {
	log.Infof("got call to AutoWatchSvcObjstoreV1")
	return errNotImplemented
}

func (g *grpcBackend) ListObjectsByPrefix(ctx context.Context, Tenant, Namespace, Prefix string) (*objstore.ObjectList, error) {
	log.Infof("got call to ListObjectsByPrefix [%+v]", Prefix)
	bucket := Tenant + "." + Namespace
	// Options are ignore for now.
	ret := &objstore.ObjectList{
		TypeMeta: api.TypeMeta{Kind: "ObjectList"},
	}
	errs := g.instance.RunPlugins(ctx, Namespace, vos.PreOp, vos.List, nil, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}
		return nil, nil
	}

	if len(Prefix) == 0 || len(Tenant) == 0 || len(Namespace) == 0 {
		return nil, errors.New("prefix or tenant or namespace passed is empty")
	}

	doneCh := make(chan struct{})
	objCh := g.client.ListObjectsV2(bucket, Prefix, true, doneCh)
	for mobj := range objCh {
		log.Infof("Got object %v", mobj)
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
	g.instance.RunPlugins(ctx, Namespace, vos.PostOp, vos.List, nil, g.client)

	return ret, nil

}

// DownloadFile handles download of file via gRPC
func (g *grpcBackend) DownloadFile(obj *objstore.Object, stream objstore.ObjstoreV1_DownloadFileServer) error {
	log.Infof("got call to DownloadFile [%+v]", obj)
	str, err := g.validateNamespace(obj)
	if err != nil {
		log.Errorf("validation failed for bucket name [%+v]", obj)
		return apierrors.ToGrpcError(err, []string{"valid types: " + str}, int32(codes.InvalidArgument), "", nil)
	}
	bucket := obj.Tenant + "." + obj.Namespace
	buf := make([]byte, 1024*1024)
	fr, err := g.client.GetStoreObject(stream.Context(), bucket, obj.Name, minio.GetObjectOptions{})
	if err != nil {
		return apierrors.ToGrpcError("client error", []string{err.Error()}, int32(codes.Internal), "", nil)
	}
	errs := g.instance.RunPlugins(stream.Context(), obj.Namespace, vos.PreOp, vos.Download, nil, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}
		return apierrors.ToGrpcError("failed to complete PreOp checks", strs, int32(codes.FailedPrecondition), "", nil)
	}
	totsize := 0
	for {
		n, err := fr.Read(buf)
		if err != nil && err != io.EOF {
			log.Errorf("error while reading object (%s)", err)
			return apierrors.ToGrpcError("client error", []string{err.Error()}, int32(codes.Internal), "", nil)
		}
		if n == 0 {
			break
		}
		totsize += n
		chunk := &objstore.StreamChunk{
			Content: buf[:n],
		}
		if err = stream.Send(chunk); err != nil {
			log.Errorf("error writing to output file (%s)", err)
			return apierrors.ToGrpcError("client error", []string{err.Error()}, int32(codes.Internal), "", nil)

		}
	}
	errs = g.instance.RunPlugins(stream.Context(), obj.Namespace, vos.PostOp, vos.Download, nil, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}
		return apierrors.ToGrpcError("failed to complete PostOp checks", strs, int32(codes.Internal), "", nil)
	}
	log.Infof("download complete for file [%v/%v] size (%d)", bucket, obj.Name, totsize)
	return nil
}

// DownloadFileByPrefix allows download of objects by prefix
func (g *grpcBackend) DownloadFileByPrefix(objS *objstore.Object, stream objstore.ObjstoreV1_DownloadFileByPrefixServer) error {
	log.Infof("got call to DownloadFileByPrefix [%+v]", objS)
	str, err := g.validateNamespace(objS)
	if err != nil {
		log.Errorf("validation failed for bucket name [%+v]", objS)
		return apierrors.ToGrpcError(err, []string{"valid types: " + str}, int32(codes.InvalidArgument), "", nil)
	}
	bucket := objS.Tenant + "." + objS.Namespace

	name := strings.TrimSuffix(objS.Name, ".zip")
	ret, err := g.ListObjectsByPrefix(stream.Context(), objS.Tenant, objS.Namespace, name)

	if err != nil {
		return err
	}
	// Options are ignore for now.
	outReader, outWriter := io.Pipe()
	zipWriter := zip.NewWriter(outWriter)

	errs := g.instance.RunPlugins(stream.Context(), objS.Namespace, vos.PreOp, vos.Download, nil, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}

		return apierrors.ToGrpcError("failed to complete PreOp checks", strs, int32(codes.FailedPrecondition), "", nil)
	}

	go func() {
		for _, obj := range ret.Items {
			fr, err := g.client.GetStoreObject(stream.Context(), bucket, obj.ObjectMeta.Name, minio.GetObjectOptions{})
			if err != nil {
				log.Errorf("Failed to get object [%v] from object store.", obj.ObjectMeta.Name)
				goto End
			}

			fileName := fmt.Sprintf("./%s", obj.ObjectMeta.Name)
			zipW, err := zipWriter.Create(fileName)

			if err != nil {
				log.Errorf("Failed add object [%v] to zip file.", obj.ObjectMeta.Name)
				goto End
			}

			_, err = io.Copy(zipW, fr)
			if err != nil {
				log.Errorf("Copy of object [%v] to the zip file failed.", obj.ObjectMeta.Name)
				goto End
			}
		}
	End:
		zipWriter.Close()
		outWriter.Close()
		return
	}()

	buf := make([]byte, 1024*1024)
	for {
		n, err := outReader.Read(buf)
		if err != nil && err != io.EOF {
			log.Errorf("error while reading object (%s)", err)
			return apierrors.ToGrpcError("client error", []string{err.Error()}, int32(codes.Internal), "", nil)
		}
		if n == 0 {
			break
		}
		chunk := &objstore.StreamChunk{
			Content: buf[:n],
		}
		if err = stream.Send(chunk); err != nil {
			log.Errorf("error writing to output file (%s)", err)
			return apierrors.ToGrpcError("client error", []string{err.Error()}, int32(codes.Internal), "", nil)

		}
	}

	errs = g.instance.RunPlugins(stream.Context(), objS.Namespace, vos.PostOp, vos.Download, nil, g.client)
	if errs != nil {
		var strs []string
		for _, e := range errs {
			strs = append(strs, e.Error())
		}
		return apierrors.ToGrpcError("failed to complete PostOp checks", strs, int32(codes.Internal), "", nil)
	}

	return nil
}

func (g *grpcBackend) WatchDiskThresholdUpdates(opts *api.ListWatchOptions,
	stream vosinternalprotos.ObjstoreInternalService_WatchDiskThresholdUpdatesServer) error {
	log.Infof("got call to WatchDiskThresholdUpdates")
	peer := ctxutils.GetContextID(stream.Context())
	if opts.ResourceVersion != "" || opts.LabelSelector != "" || opts.FieldSelector != "" || opts.FieldChangeSelector != nil {
		return errors.New("filtering is not supported")
	}

	handleFn := func(inctx context.Context, evType kvstore.WatchEventType, item, prev runtime.Object) {
		stream.Send(item.(*vosinternalprotos.DiskUpdate))
	}

	err := g.instance.Watch(stream.Context(), diskUpdateWatchPath, peer, handleFn, nil)
	return err
}
