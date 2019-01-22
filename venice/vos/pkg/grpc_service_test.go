package vos

import (
	"context"
	"fmt"
	"io"
	"net/http"
	"testing"
	"time"

	minioclient "github.com/minio/minio-go"
	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/objstore"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type mockBackend struct {
	bucketExists, makeBucket, removeBucket, putBucket int
	putObject, removeObject, listObject, statObject   int

	retErr       error
	bExists      bool
	putSize      int64
	putErr       error
	objInfo      minioclient.ObjectInfo
	listObjFn    func() <-chan minioclient.ObjectInfo
	makeBucketFn func(bucketName string, location string) error
	delBucketFn  func(bucketname string) error
	delObjFn     func(b, n string) error
	statFunc     func(bucketName, objectName string, opts minioclient.StatObjectOptions) (minioclient.ObjectInfo, error)
	bExistsFunc  func(in string) (bool, error)
}

func (f *mockBackend) BucketExists(bucketName string) (bool, error) {
	f.bucketExists++
	if f.bExistsFunc != nil {
		return f.bExistsFunc(bucketName)
	}
	return f.bExists, f.retErr
}
func (f *mockBackend) MakeBucket(bucketName string, location string) (err error) {
	f.makeBucket++
	if f.makeBucketFn != nil {
		return f.makeBucketFn(bucketName, location)
	}
	return f.retErr
}
func (f *mockBackend) RemoveBucket(bucketName string) error {
	f.removeBucket++
	if f.delBucketFn != nil {
		return f.delBucketFn(bucketName)
	}
	return f.retErr
}

func (f *mockBackend) PutObject(bucketName, objectName string, reader io.Reader, objectSize int64, opts minioclient.PutObjectOptions) (n int64, err error) {
	f.putObject++
	return f.putSize, f.putErr
}
func (f *mockBackend) RemoveObject(bucketName, objectName string) error {
	f.removeObject++
	return f.retErr
}
func (f *mockBackend) ListObjectsV2(bucketName, objectPrefix string, recursive bool, doneCh <-chan struct{}) <-chan minioclient.ObjectInfo {
	f.listObject++
	return f.listObjFn()
}
func (f *mockBackend) StatObject(bucketName, objectName string, opts minioclient.StatObjectOptions) (minioclient.ObjectInfo, error) {
	f.statObject++
	if f.statFunc != nil {
		return f.statFunc(bucketName, objectName, opts)
	}
	return f.objInfo, f.retErr
}

func TestValidateNamespace(t *testing.T) {
	good := []string{"images", "techsupport", "packetcapture"}
	bad := []string{"invalid", " images", "tech-support"}
	obj := &objstore.Object{}
	mbackend := &mockBackend{}
	srv, err := newGrpcServer(mbackend)
	AssertOk(t, err, "could not create client")
	for _, v := range good {
		obj.Namespace = v
		_, err := srv.validateNamespace(obj)
		if err != nil {
			t.Errorf("[%v] failed validation", v)
		}
	}
	for _, v := range bad {
		obj.Namespace = v
		_, err := srv.validateNamespace(obj)
		if err == nil {
			t.Errorf("[%v] should have failed validation", v)
		}
	}
}

func TestBucketOps(t *testing.T) {
	fb := &mockBackend{}
	srv := grpcBackend{client: fb}
	ctx := context.Background()
	bucket := objstore.Bucket{
		ObjectMeta: api.ObjectMeta{
			Name: "images",
		},
	}
	fb.makeBucketFn = func(bucketName string, location string) error {
		bname := bucket.Tenant + "." + bucket.Name
		if bucket.Tenant == "" {
			bname = "default." + bucket.Name
		}
		if bucketName != bname {
			return errors.New("wrong Name")
		}
		return nil
	}
	fb.bExists = false
	_, err := srv.AutoAddBucket(ctx, nil)
	Assert(t, err != nil, "Add bucket should have failed")

	_, err = srv.AutoAddBucket(ctx, &bucket)
	AssertOk(t, err, "failed to add bucket (%s)", err)

	bucket.Tenant = "nondefault"
	_, err = srv.AutoAddBucket(ctx, &bucket)
	AssertOk(t, err, "failed to add bucket (%s)", err)

	srv.client = nil
	_, err = srv.AutoAddBucket(ctx, &bucket)
	Assert(t, err != nil, "Add bucket should have failed")

	srv.client = fb
	fb.retErr = errors.New("some error")
	_, err = srv.AutoAddBucket(ctx, &bucket)
	Assert(t, err != nil, "Add bucket should have failed")
	fb.retErr = nil
	fb.bExists = true
	_, err = srv.AutoAddBucket(ctx, &bucket)
	Assert(t, err != nil, "Duplicate add bucket should have failed")

	fb.delBucketFn = func(name string) error {
		bname := bucket.Tenant + "." + bucket.Name
		if bucket.Tenant == "" {
			bname = "default." + bucket.Name
		}
		if name != bname {
			return errors.New("wrong Name")
		}
		return nil
	}

	_, err = srv.AutoDeleteBucket(ctx, nil)
	Assert(t, err != nil, "del bucket should have failed")

	srv.client = nil
	_, err = srv.AutoDeleteBucket(ctx, &bucket)
	Assert(t, err != nil, "del bucket should have failed")
	srv.client = fb

	bucket.Tenant = ""
	_, err = srv.AutoDeleteBucket(ctx, &bucket)
	AssertOk(t, err, "delete bucket failed")
	bucket.Tenant = "nondefault"
	_, err = srv.AutoDeleteBucket(ctx, &bucket)
	AssertOk(t, err, "delete bucket failed")

	fb.bExists = false
	_, err = srv.AutoDeleteBucket(ctx, &bucket)
	Assert(t, err != nil, "delete non-existent bucket should have failed")

	_, err = srv.AutoGetBucket(ctx, &bucket)
	Assert(t, err != nil, "not implemented")

	_, err = srv.AutoListBucket(ctx, &api.ListWatchOptions{})
	Assert(t, err != nil, "not implemented")

	_, err = srv.AutoUpdateBucket(ctx, &bucket)
	Assert(t, err != nil, "not implemented")

	err = srv.AutoWatchBucket(&api.ListWatchOptions{}, nil)
	Assert(t, err != nil, "not implemented")
}

func TestObjectOps(t *testing.T) {
	fb := &mockBackend{}
	srv := grpcBackend{client: fb}
	ctx := context.Background()
	obj := &objstore.Object{
		ObjectMeta: api.ObjectMeta{
			Name:      "testobj",
			Namespace: "images",
		},
	}
	_, err := srv.AutoAddObject(ctx, obj)
	Assert(t, err != nil, "not implemented")

	fb.delObjFn = func(b, n string) error {
		if b != obj.Namespace || n != obj.Name {
			return fmt.Errorf("wrong bucket/name [%v/%v]", b, n)
		}
		return nil
	}
	tm := time.Now()
	crKey := metaPrefix + metaCreationTime
	metadata := make(map[string]string)
	httphdr := http.Header{}
	metadata["test1"] = "One"
	httphdr.Set("X-Amz-Meta-test1", "One")
	metadata["test2"] = "two"
	httphdr.Set("X-Amz-Meta-test2", "two")
	httphdr.Set(crKey, tm.Format(time.RFC3339Nano))
	objInfo := minioclient.ObjectInfo{
		ETag:     "abcdef",
		Key:      "file",
		Metadata: httphdr,
	}
	fb.objInfo = objInfo
	srv.client = nil
	ret, err := srv.AutoDeleteObject(ctx, obj)
	Assert(t, err != nil, "Delete object should have failed")
	srv.client = fb
	ret, err = srv.AutoDeleteObject(ctx, nil)
	Assert(t, err != nil, "Delete object should have failed")
	fb.retErr = errors.New("some error")
	ret, err = srv.AutoDeleteObject(ctx, obj)
	Assert(t, err != nil, "Delete object should have failed")
	fb.retErr = nil
	obj.Namespace = "invalid"
	ret, err = srv.AutoDeleteObject(ctx, obj)
	Assert(t, err != nil, "Delete object should have failed")
	obj.Namespace = "images"
	ret, err = srv.AutoDeleteObject(ctx, obj)
	AssertOk(t, err, "failed to delete object (%s)", err)

	if ret.Name != obj.Name || ret.Namespace != obj.Namespace {
		t.Errorf("returned object meta does not match ")
	}
	if ct, err := ret.CreationTime.Time(); err != nil {
		t.Errorf("could not parse creation time")
	} else {
		Assert(t, ct.Second() == tm.Second() && ct.Nanosecond() == tm.Nanosecond(), "creation time did not match [%v][%v]", ct, tm)
	}
	if v, ok := ret.Labels["Test1"]; !ok || v != "One" {
		t.Errorf("got wrong value for label test1 [%v][%v]", ok, v)
	}
	if v, ok := ret.Labels["Test2"]; !ok || v != "two" {
		t.Errorf("got wrong value for label test2 [%v][%v]", ok, v)
	}
	srv.client = nil
	ret, err = srv.AutoGetObject(ctx, obj)
	Assert(t, err != nil, "Get object should have failed")
	srv.client = fb
	ret, err = srv.AutoGetObject(ctx, nil)
	Assert(t, err != nil, "Get object should have failed")
	obj.Namespace = "invalid"
	ret, err = srv.AutoGetObject(ctx, obj)
	Assert(t, err != nil, "Get object should have failed")
	fb.retErr = errors.New("some error")
	ret, err = srv.AutoGetObject(ctx, obj)
	Assert(t, err != nil, "Get object should have failed")
	obj.Namespace = "images"
	fb.retErr = nil
	ret, err = srv.AutoGetObject(ctx, obj)
	AssertOk(t, err, "Get Object should have passed")
	if ret.Name != obj.Name || ret.Namespace != obj.Namespace {
		t.Errorf("returned object meta does not match ")
	}
	if ct, err := ret.CreationTime.Time(); err != nil {
		t.Errorf("could not parse creation time")
	} else {
		Assert(t, ct.Second() == tm.Second() && ct.Nanosecond() == tm.Nanosecond(), "creation time did not match [%v][%v]", ct, tm)
	}
	if v, ok := ret.Labels["Test1"]; !ok || v != "One" {
		t.Errorf("got wrong value for label test1 [%v][%v]", ok, v)
	}
	if v, ok := ret.Labels["Test2"]; !ok || v != "two" {
		t.Errorf("got wrong value for label test2 [%v][%v]", ok, v)
	}

	objCh := make(chan minioclient.ObjectInfo)
	fb.listObjFn = func() <-chan minioclient.ObjectInfo {
		return objCh
	}
	objs := []minioclient.ObjectInfo{
		{
			ETag:     "abcdef",
			Key:      "file1",
			Metadata: httphdr,
		},
		{
			ETag:     "abcdef",
			Key:      "file2",
			Metadata: httphdr,
		},
		{
			ETag:     "abcdef",
			Key:      "file3",
			Metadata: httphdr,
		},
	}
	go func() {
		for _, v := range objs {
			objCh <- v
		}
		close(objCh)
	}()
	srv.client = nil
	retl, err := srv.AutoListObject(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Namespace: "images", Tenant: "default"}})
	Assert(t, err != nil, "List Objects should have failed")
	srv.client = fb
	retl, err = srv.AutoListObject(ctx, nil)
	Assert(t, err != nil, "List Objects should have failed")
	retl, err = srv.AutoListObject(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Namespace: "invlaid", Tenant: "default"}})
	Assert(t, err != nil, "List Objects should have failed")
	retl, err = srv.AutoListObject(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Namespace: "images", Tenant: "default"}})
	AssertOk(t, err, "failed to list objects (%s)", err)
	Assert(t, len(retl.Items) == 3, "wrong number of items returned [%v]", len(retl.Items))

	_, err = srv.AutoUpdateObject(ctx, obj)
	Assert(t, err != nil, "not implemented")

	_, err = srv.AutoListBucket(ctx, &api.ListWatchOptions{})
	Assert(t, err != nil, "not implemented")

	err = srv.AutoWatchObject(&api.ListWatchOptions{}, nil)
	Assert(t, err != nil, "not implemented")

	err = srv.AutoWatchSvcObjstoreV1(nil, nil)
	Assert(t, err != nil, "not implemented")
}
