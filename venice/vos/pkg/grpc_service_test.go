package vospkg

import (
	"bytes"
	"compress/gzip"
	"context"
	"encoding/csv"
	"fmt"
	"io"
	"net/http"
	"strconv"
	"strings"
	"testing"
	"time"

	"github.com/minio/minio-go"
	minioclient "github.com/minio/minio-go"
	"github.com/pkg/errors"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/objstore"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/vos"
)

func TestValidateNamespace(t *testing.T) {
	good := []string{"images", "techsupport"}
	bad := []string{"invalid", " images", "tech-support"}
	obj := &objstore.Object{}
	mbackend := &mockBackend{}
	inst := &instance{}
	inst.Init(mbackend)
	srv, err := newGrpcServer(inst, mbackend)
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
	inst := &instance{}
	inst.Init(fb)
	srv := grpcBackend{client: fb, instance: inst}
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

	_, err = srv.AutoLabelBucket(ctx, &api.Label{})
	Assert(t, err != nil, "not implemented")

	err = srv.AutoWatchBucket(&api.ListWatchOptions{}, nil)
	Assert(t, err != nil, "not implemented")
}

func TestObjectOps(t *testing.T) {
	fb := &mockBackend{}
	inst := &instance{}
	inst.Init(fb)
	srv := grpcBackend{client: fb, instance: inst}
	ctx := context.Background()
	obj := &objstore.Object{
		ObjectMeta: api.ObjectMeta{
			Name:      "testobj",
			Namespace: "images",
		},
	}

	// Add plugin funcs
	callMap1 := make(map[vos.ObjectOper]int)
	callMap2 := make(map[vos.ObjectOper]int)
	cbFunc1 := func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		callMap1[oper] = callMap1[oper] + 1
		return nil
	}
	cbFunc2 := func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		callMap2[oper] = callMap2[oper] + 1
		return nil
	}
	inst.RegisterCb("images", vos.PreOp, vos.Upload, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Upload, cbFunc2)
	inst.RegisterCb("images", vos.PreOp, vos.Download, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Download, cbFunc2)
	inst.RegisterCb("images", vos.PreOp, vos.Get, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Get, cbFunc2)
	inst.RegisterCb("images", vos.PreOp, vos.Delete, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Delete, cbFunc2)
	inst.RegisterCb("images", vos.PreOp, vos.Update, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Update, cbFunc2)
	inst.RegisterCb("images", vos.PreOp, vos.Watch, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Watch, cbFunc2)

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

	_, err = srv.AutoLabelObject(ctx, &api.Label{})
	Assert(t, err != nil, "not implemented")

	_, err = srv.AutoListBucket(ctx, &api.ListWatchOptions{})
	Assert(t, err != nil, "not implemented")

	cctx, cancel := context.WithCancel(ctx)
	fw := &fakeWatchServer{ctx: cctx}
	go func() {
		time.Sleep(100 * time.Millisecond)
		cancel()
	}()
	err = srv.AutoWatchObject(&api.ListWatchOptions{}, fw)

	err = srv.AutoWatchSvcObjstoreV1(nil, nil)
	Assert(t, err != nil, "not implemented")
}

func TestObjectOpsWError(t *testing.T) {
	fb := &mockBackend{}
	inst := &instance{}
	inst.Init(fb)
	srv := grpcBackend{client: fb, instance: inst}
	ctx := context.Background()
	obj := &objstore.Object{
		ObjectMeta: api.ObjectMeta{
			Name:      "testobj",
			Namespace: "images",
		},
	}
	var cbErr1, cbErr2 error
	// Add plugin funcs
	cbErr1 = errors.New("some error")
	cbFunc1 := func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		return cbErr1
	}
	cbFunc2 := func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		return cbErr2
	}
	inst.RegisterCb("images", vos.PreOp, vos.Upload, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Upload, cbFunc2)
	inst.RegisterCb("images", vos.PreOp, vos.Download, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Download, cbFunc2)
	inst.RegisterCb("images", vos.PreOp, vos.Get, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Get, cbFunc2)
	inst.RegisterCb("images", vos.PreOp, vos.List, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.List, cbFunc2)
	inst.RegisterCb("images", vos.PreOp, vos.Delete, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Delete, cbFunc2)
	inst.RegisterCb("images", vos.PreOp, vos.Update, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Update, cbFunc2)
	inst.RegisterCb("images", vos.PreOp, vos.Watch, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Watch, cbFunc2)

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
	srv.client = fb
	fb.retErr = nil
	obj.Namespace = "images"
	_, err := srv.AutoDeleteObject(ctx, obj)
	Assert(t, err != nil, "Delete should have failed")

	_, err = srv.AutoGetObject(ctx, obj)
	Assert(t, err != nil, "Get Object should have failed")

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

	_, err = srv.AutoListObject(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Namespace: "images", Tenant: "default"}})
	Assert(t, err != nil, "list objects should have failed")

	cctx, cancel := context.WithCancel(ctx)
	fw := &fakeWatchServer{ctx: cctx}
	go func() {
		time.Sleep(100 * time.Millisecond)
		cancel()
	}()
	err = srv.AutoWatchObject(&api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Namespace: "images"}}, fw)
	Assert(t, err != nil, "watch objects should have failed")

	err = srv.AutoWatchSvcObjstoreV1(nil, nil)
	Assert(t, err != nil, "not implemented")

	cbErr2 = cbErr1
	cbErr1 = nil
	_, err = srv.AutoDeleteObject(ctx, obj)
	Assert(t, err != nil, "Delete should have failed")

	_, err = srv.AutoGetObject(ctx, obj)
	Assert(t, err != nil, "Get Object should have failed")

	_, err = srv.AutoListObject(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Namespace: "images", Tenant: "default"}})
	Assert(t, err != nil, "list objects should have failed")

	inst.Close()
}

type fakeDownloadFileServer struct {
	ctx     context.Context
	sendCnt int
}

// SetHeader implements a mock interface
func (f *fakeDownloadFileServer) Send(chunk *objstore.StreamChunk) error {
	f.sendCnt++
	return nil
}

// SetHeader implements a mock interface
func (f *fakeDownloadFileServer) SetHeader(metadata.MD) error { return nil }

// SendHeader implements a mock interface
func (f *fakeDownloadFileServer) SendHeader(metadata.MD) error { return nil }

// SetTrailer implements a mock interface
func (f *fakeDownloadFileServer) SetTrailer(metadata.MD) {}

// Context implements a mock interface
func (f *fakeDownloadFileServer) Context() context.Context {
	return f.ctx
}

// SendMsg implements a mock interface
func (f *fakeDownloadFileServer) SendMsg(m interface{}) error { return nil }

// RecvMsg implements a mock interface
func (f *fakeDownloadFileServer) RecvMsg(m interface{}) error { return nil }

func TestListFileByPrefix(t *testing.T) {
	fb := &mockBackend{}
	inst := &instance{}
	inst.Init(fb)

	objCh := make(chan minioclient.ObjectInfo)
	fb.listObjFn = func() <-chan minioclient.ObjectInfo {
		return objCh
	}

	httphdr := http.Header{}
	objs := []minioclient.ObjectInfo{
		{
			ETag:     "abcdef",
			Key:      "file1",
			Metadata: httphdr,
		},
	}
	go func() {
		for _, v := range objs {
			objCh <- v
		}
		close(objCh)
	}()

	ctx := context.Background()
	srv := grpcBackend{client: fb, instance: inst}
	fso := &fakeStoreObj{}
	fb.fObj = fso
	_, err := srv.ListObjectsByPrefix(ctx, "default", "images", "ba")
	Assert(t, err == nil, "List by prefix did not work")

	_, err = srv.ListObjectsByPrefix(ctx, "", "", "ba")
	Assert(t, err != nil, "List by prefix worked")

	cbFunc1 := func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		return nil
	}
	inst.RegisterCb("images", vos.PreOp, vos.List, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.List, cbFunc1)

	_, err = srv.ListObjectsByPrefix(ctx, "default", "images", "ba")
	Assert(t, err == nil, "List by prefix didn't work")
}

func TestDownloadFile(t *testing.T) {
	fb := &mockBackend{}
	inst := &instance{}
	inst.Init(fb)

	ctx := context.Background()
	obj := &objstore.Object{
		ObjectMeta: api.ObjectMeta{
			Name:      "testobj",
			Namespace: "images",
		},
	}
	ffs := &fakeDownloadFileServer{ctx: ctx}
	var cbErr1, cbErr2 error
	var cbcalled1, cbcalled2 int
	// Add plugin funcs
	cbFunc1 := func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		cbcalled1++
		return cbErr1
	}
	cbFunc2 := func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		cbcalled2++
		return cbErr2
	}
	inst.RegisterCb("images", vos.PreOp, vos.Download, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Download, cbFunc2)
	srv := grpcBackend{client: fb, instance: inst}
	fso := &fakeStoreObj{}
	fb.fObj = fso
	fb.retErr = errors.New("some error")
	err := srv.DownloadFile(obj, ffs)
	Assert(t, err != nil, "expecting download to error out")
	fb.retErr = nil
	cnt := 0
	testStr := []byte("aabababababababaabababababaababababababaabababababa")
	readErr := io.EOF
	fso.readFn = func(in []byte) (int, error) {
		in = testStr
		var err error
		if cnt == 3 {
			err = readErr
		}

		if cnt > 3 {
			return 0, io.EOF
		}
		cnt++
		return len(testStr), err
	}
	err = srv.DownloadFile(obj, ffs)
	AssertOk(t, err, "DownloadFile failed")
	Assert(t, cbcalled1 == 1, "exepecting 1 call for preop got [%d]", cbcalled1)
	Assert(t, cbcalled2 == 1, "exepecting 1 call for postop got [%d]", cbcalled2)
	Assert(t, ffs.sendCnt == 4, "should have seen 3 sends got [%d]", ffs.sendCnt)
	cnt = 0
	readErr = errors.New("some error")
	err = srv.DownloadFile(obj, ffs)
	Assert(t, err != nil, "DownloadFile should have failed")
	cnt = 0
	cbErr1 = readErr
	readErr = nil
	err = srv.DownloadFile(obj, ffs)
	Assert(t, err != nil, "DownloadFile should have failed")
	cnt = 0
	cbErr2 = cbErr1
	cbErr1 = nil
	err = srv.DownloadFile(obj, ffs)
	Assert(t, err != nil, "DownloadFile should have failed")
}

func TestDownloadFileByPrefix(t *testing.T) {
	fb := &mockBackend{}
	inst := &instance{}
	inst.Init(fb)

	ctx := context.Background()
	obj := &objstore.Object{
		ObjectMeta: api.ObjectMeta{
			Name:      "testobj",
			Namespace: "images",
		},
	}
	ffs := &fakeDownloadFileServer{ctx: ctx}
	var cbErr1, cbErr2 error
	var cbcalled1, cbcalled2 int
	// Add plugin funcs
	cbFunc1 := func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		cbcalled1++
		return cbErr1
	}
	cbFunc2 := func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		cbcalled2++
		return cbErr2
	}
	inst.RegisterCb("images", vos.PreOp, vos.Download, cbFunc1)
	inst.RegisterCb("images", vos.PostOp, vos.Download, cbFunc2)
	srv := grpcBackend{client: fb, instance: inst}
	fso := &fakeStoreObj{}
	fb.fObj = fso
	fb.retErr = errors.New("some error")
	err := srv.DownloadFile(obj, ffs)
	Assert(t, err != nil, "expecting download to error out")
	fb.retErr = nil
	cnt := 0
	testStr := []byte("aabababababababaabababababaababababababaabababababa")
	readErr := io.EOF
	fso.readFn = func(in []byte) (int, error) {
		in = testStr
		var err error
		if cnt == 3 {
			err = readErr
		}

		if cnt > 3 {
			return 0, io.EOF
		}
		cnt++
		return len(testStr), err
	}
	err = srv.DownloadFileByPrefix(obj, ffs)
	Assert(t, err != nil, "DownloadFile Worked")

	obj.ObjectMeta.Namespace = "badbad"
	err = srv.DownloadFileByPrefix(obj, ffs)
	Assert(t, err != nil, "DownloadFile Worked with bad namespace")
}

func TestListFwlogObjects(t *testing.T) {
	ctx := context.Background()
	fb := &mockBackend{}
	inst := &instance{}
	inst.Init(fb)
	srv := grpcBackend{client: fb, instance: inst}
	fb.listFwLogObjFn = func(prefix string) <-chan minioclient.ObjectInfo {
		objCh := make(chan minioclient.ObjectInfo, 1000)
		go func() {
			for k := range fb.fwlogObjects {
				if strings.Contains(k, prefix) {
					objCh <- minioclient.ObjectInfo{Key: k}
				}
			}
			close(objCh)
		}()
		return objCh
	}
	fb.statFunc = func(bucketName, objectName string, opts minioclient.StatObjectOptions) (minioclient.ObjectInfo, error) {
		tokens := strings.Split(strings.Split(objectName, "/")[5], "_")
		oi := minioclient.ObjectInfo{}
		oi.Key = objectName
		meta := map[string][]string{}
		meta[metaPrefix+"Startts"] = []string{tokens[0]}
		meta[metaPrefix+"Endts"] = []string{strings.Split(tokens[1], ".")[0]}
		oi.Metadata = http.Header(meta)
		return oi, nil
	}

	timeFormat := "2006-01-02T15:04:05"
	ts, _ := time.Parse(time.RFC3339, "2006-01-02T15:00:00Z")
	t1 := ts
	for i := 0; i < 100; i++ {
		h, _, _ := t1.Clock()
		key := "aaa.bbb.ccc.ddd" + "/2006/1/2/" + strconv.Itoa(h) + "/" +
			t1.Format(timeFormat) + "_" + t1.Add(time.Minute).Format(timeFormat) + ".csv.gz"
		fb.PutObject("default"+fwlogsBucketName, key, nil, 0, minio.PutObjectOptions{})
		t1 = t1.Add(time.Minute)
	}

	res, err := srv.AutoListObject(ctx, &api.ListWatchOptions{
		ObjectMeta:    api.ObjectMeta{Namespace: "fwlogs", Tenant: "default"},
		FieldSelector: "start-time=2006-01-02T15:00:00Z,end-time=2006-01-02T15:01:00Z,dsc-id=aaa.bbb.ccc.ddd"})
	AssertOk(t, err, "List Objects failed")
	Assert(t, len(res.Items) == 1, "result is %d", len(res.Items))

	// Reset the objCh. It gets closed after every call to ListObjects.
	res, err = srv.AutoListObject(ctx, &api.ListWatchOptions{
		ObjectMeta:    api.ObjectMeta{Namespace: "fwlogs", Tenant: "default"},
		FieldSelector: "start-time=2006-01-02T15:00:00Z,end-time=2006-01-02T15:10:00Z,dsc-id=aaa.bbb.ccc.ddd"})
	AssertOk(t, err, "List Objects failed")
	Assert(t, len(res.Items) == 10, "result is %d", len(res.Items))

	// Reset the objCh. It gets closed after every call to ListObjects.
	res, err = srv.AutoListObject(ctx, &api.ListWatchOptions{
		ObjectMeta:    api.ObjectMeta{Namespace: "fwlogs", Tenant: "default"},
		FieldSelector: "start-time=2006-01-02T15:00:00Z,end-time=2006-01-02T15:59:59Z,dsc-id=aaa.bbb.ccc.ddd"})
	AssertOk(t, err, "List Objects failed")
	Assert(t, len(res.Items) == 59, "result is %d", len(res.Items))

	// Reset the objCh. It gets closed after every call to ListObjects.
	res, err = srv.AutoListObject(ctx, &api.ListWatchOptions{
		ObjectMeta:    api.ObjectMeta{Namespace: "fwlogs", Tenant: "default"},
		FieldSelector: "start-time=2006-01-02T15:00:00Z,end-time=2006-01-02T16:40:00Z,dsc-id=aaa.bbb.ccc.ddd"})
	AssertOk(t, err, "List Objects failed")
	Assert(t, len(res.Items) == 100, "result is %d", len(res.Items))

	// Reset the objCh. It gets closed after every call to ListObjects.
	res, err = srv.AutoListObject(ctx, &api.ListWatchOptions{
		ObjectMeta:    api.ObjectMeta{Namespace: "fwlogs", Tenant: "default"},
		FieldSelector: "start-time=2006-01-02T15:00:00Z,end-time=2006-01-02T16:50:00Z,dsc-id=aaa.bbb.ccc.ddd"})
	AssertOk(t, err, "List Objects failed")
	Assert(t, len(res.Items) == 100, "result is %d", len(res.Items))

	// Reset the objCh. It gets closed after every call to ListObjects.
	res, err = srv.AutoListObject(ctx, &api.ListWatchOptions{
		ObjectMeta:    api.ObjectMeta{Namespace: "fwlogs", Tenant: "default"},
		FieldSelector: "start-time=2006-01-02T15:10:00Z,end-time=2006-01-02T16:10:00Z,dsc-id=aaa.bbb.ccc.ddd"})
	AssertOk(t, err, "List Objects failed")
	Assert(t, len(res.Items) == 60, "result is %d", len(res.Items))
}

func TestListFwlogObjectContent(t *testing.T) {
	ctx := context.Background()
	fb := &mockBackend{}
	inst := &instance{}
	inst.Init(fb)
	srv := grpcBackend{client: fb, instance: inst}
	objBuffer, rawLogs, totalLogs := getDummyFwLogs()
	timeFormat := "2006-01-02T15:04:05"
	ts, _ := time.Parse(time.RFC3339, "2006-01-02T15:00:00Z")
	h, _, _ := ts.Clock()
	key := "aaa.bbb.ccc.ddd" + "/2006/1/2/" + strconv.Itoa(h) + "/" +
		ts.Format(timeFormat) + "_" + ts.Add(time.Minute).Format(timeFormat) + ".csv.gz"
	fb.PutObject("default"+fwlogsBucketName,
		key, bytes.NewReader(objBuffer.Bytes()), int64(len(objBuffer.Bytes())), minio.PutObjectOptions{})

	obj := &objstore.Object{
		ObjectMeta: api.ObjectMeta{
			Name:      strings.Replace(key, "/", "_", 5),
			Namespace: fwlogsBucketName,
			Tenant:    "default",
		},
	}

	logs, err := srv.DownloadFwLogs(ctx, obj)
	AssertOk(t, err, "DownloadFwLogs failed")
	Assert(t, len(logs.Items) == totalLogs, "DownloadFwLogs returned incorrect number of logs", len(logs.Items))
	for i := 0; i < totalLogs; i++ {
		Assert(t, rawLogs[i+1][0] == strconv.Itoa(int(logs.Items[i].SrcVRF)), "src vrf is not matching", rawLogs[i][0], logs.Items[i].SrcVRF)
		Assert(t, rawLogs[i+1][2] == logs.Items[i].SrcIP, "src ip is not matching")
		Assert(t, rawLogs[i+1][3] == logs.Items[i].DestIP, "dest ip is not matching")
		Assert(t, rawLogs[i+1][5] == strconv.Itoa(int(logs.Items[i].SrcPort)), "src port is not matching")
		Assert(t, rawLogs[i+1][6] == strconv.Itoa(int(logs.Items[i].DestPort)), "dest port is not matching")
	}
}

func getDummyFwLogs() (bytes.Buffer, [][]string, int) {
	totalLogs := 100
	logs := [][]string{}
	logs = append(logs, []string{"svrf", "dvrf", "sip",
		"dip", "ts", "sport", "dport",
		"proto", "act", "dir", "ruleid",
		"sessionid", "sessionstate",
		"icmptype", "icmpid", "icmpcode",
		"appid", "alg", "count"})

	helper := func(csvBytes *bytes.Buffer) {
		w := csv.NewWriter(csvBytes)
		for _, l := range logs {
			w.Write(l)
		}
		w.Flush()
	}

	for i := 0; i < totalLogs; i++ {
		// CSV file format
		fwLog := []string{
			"65",
			"65",
			"10.10.10." + strconv.Itoa(i),
			"10.10.10." + strconv.Itoa(i),
			time.Now().UTC().Format(time.RFC3339),
			"8000",
			"8000",
			"TCP",
			"allow",
			"from-host",
			"1989",
			"1990",
			"create",
			"10",
			"20",
			"30",
			"32",
			"TFTP",
			"1",
		}
		logs = append(logs, fwLog)
	}

	var csvBytes, zipBytes bytes.Buffer
	helper(&csvBytes)
	zw := gzip.NewWriter(&zipBytes)
	zw.Write(csvBytes.Bytes())
	zw.Close()
	return zipBytes, logs, totalLogs
}
