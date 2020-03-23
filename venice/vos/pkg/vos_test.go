package vospkg

import (
	"context"
	"io"
	"testing"

	minioclient "github.com/minio/minio-go"
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/vos"

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
	listenObjFn  func() <-chan minioclient.NotificationInfo
	makeBucketFn func(bucketName string, location string) error
	delBucketFn  func(bucketname string) error
	delObjFn     func(b, n string) error
	statFunc     func(bucketName, objectName string, opts minioclient.StatObjectOptions) (minioclient.ObjectInfo, error)
	bExistsFunc  func(in string) (bool, error)
	fObj         *fakeStoreObj
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
func (f *mockBackend) GetObjectWithContext(ctx context.Context, bucketName, objectName string, opts minioclient.GetObjectOptions) (*minioclient.Object, error) {
	return nil, nil
}
func (f *mockBackend) ListenBucketNotification(bucketName, prefix, suffix string, events []string, doneCh <-chan struct{}) <-chan minioclient.NotificationInfo {
	if f.listenObjFn != nil {
		return f.listenObjFn()
	}
	return nil
}

func (f *mockBackend) GetStoreObject(ctx context.Context, bucketName, objectName string, opts minioclient.GetObjectOptions) (vos.StoreObject, error) {
	return f.fObj, f.retErr
}

type fakeStoreObj struct {
	closeErr, statErr error
	statObjInfo       minioclient.ObjectInfo

	readFn func([]byte) (int, error)
}

// Close is a mock implementation
func (f *fakeStoreObj) Close() (err error) {
	return f.closeErr
}

// Stat is a mock implementation
func (f *fakeStoreObj) Stat() (minioclient.ObjectInfo, error) {
	return f.statObjInfo, f.statErr

}

// Read is a mock implementation
func (f *fakeStoreObj) Read(b []byte) (n int, err error) {
	if f.readFn != nil {
		return f.readFn(b)
	}
	return 0, nil
}

func TestCreateBuckets(t *testing.T) {
	fb := &mockBackend{}
	bmap := make(map[string]int)
	var makeErr error
	fb.makeBucketFn = func(name, loc string) error {
		if v, ok := bmap[name]; ok {
			bmap[name] = v + 1
		} else {
			bmap[name] = 1
		}
		return makeErr
	}
	maxCreateBucketRetries = 1
	inst := &instance{}
	inst.Init(fb)
	err := inst.createDefaultBuckets(fb)
	AssertOk(t, err, "create buckets failed")
	Assert(t, fb.bucketExists == len(objstore.Buckets_name), "did not find correct number of calls [%v][%v]", fb.bucketExists, len(objstore.Buckets_name))
	Assert(t, fb.makeBucket == len(objstore.Buckets_name), "did not find correct number of calls [%v][%v]", fb.makeBucket, len(objstore.Buckets_name))
	for k := range objstore.Buckets_value {
		tenantName := "default"
		if v1, ok := bmap[tenantName+"."+k]; !ok {
			t.Errorf("did not find key [%v]", k)
		} else {
			if v1 != 1 {
				t.Errorf("duplicate calls to create buckets [%d]", v1)
			}
		}
	}

	fb.bExists = true
	fb.makeBucket = 0
	err = inst.createDefaultBuckets(fb)
	AssertOk(t, err, "create buckets should have failed")
	Assert(t, fb.makeBucket == 0, "no calls to MakeBucket expected")

	fb.bExists = false
	makeErr = errors.New("someError")
	err = inst.createDefaultBuckets(fb)
	Assert(t, err != nil, "create buckets should have failed")

	err = inst.createDefaultBuckets(fb)
	Assert(t, err != nil, "create buckets should have failed")
	inst.Close()
}

// TestOptions tests the Vos's option functions
func TestOptions(t *testing.T) {
	inst := &instance{}
	f := WithBootupArgs([]string{"dummyarg"})
	Assert(t, f != nil, "bootup option function is nil")
	f(inst)
	Assert(t, inst.bootupArgs[0] == "dummyarg", "bootup arg is missing")

	f = WithBucketDiskThresholds(GetBucketDiskThresholds())
	Assert(t, f != nil, "bucket diskthreshold option function is nil")
	f(inst)
	Assert(t, len(inst.bucketDiskThresholds) == 2, "fwlogs bucket diskthreshold is missing")

	dth := GetBucketDiskThresholds()
	Assert(t, len(dth) == 2, "incorrect disk threshold arguments, expected /disk1/default.fwlogs, /disk2/default.fwlogs")
	th, ok := dth["/disk1/default.fwlogs"]
	Assert(t, th == 50.00, "incorrect threshold")
	Assert(t, ok, "fwlogs bucket threshold missing")
	th, ok = dth["/disk2/default.fwlogs"]
	Assert(t, th == 50.00, "incorrect threshold")
	Assert(t, ok, "fwlogs bucket threshold missing")
}
