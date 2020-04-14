package vospkg

import (
	"context"
	"net/http"
	"testing"

	minioclient "github.com/minio/minio-go"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/venice/utils/watchstream"
	"github.com/pensando/sw/venice/utils/watchstream/mocks"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/objstore"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type fakeWatchServer struct {
	ctx context.Context
}

// Send implements a mock interface
func (f *fakeWatchServer) Send(*objstore.AutoMsgObjectWatchHelper) error {
	return nil
}

// SetHeader implements a mock interface
func (f *fakeWatchServer) SetHeader(metadata.MD) error { return nil }

// SendHeader implements a mock interface
func (f *fakeWatchServer) SendHeader(metadata.MD) error { return nil }

// SetTrailer implements a mock interface
func (f *fakeWatchServer) SetTrailer(metadata.MD) {}

// Context implements a mock interface
func (f *fakeWatchServer) Context() context.Context {
	return f.ctx
}

// SendMsg implements a mock interface
func (f *fakeWatchServer) SendMsg(m interface{}) error { return nil }

// RecvMsg implements a mock interface
func (f *fakeWatchServer) RecvMsg(m interface{}) error { return nil }

func TestStoreImpl(t *testing.T) {
	fb := mockBackend{}
	s := storeImpl{&fb}
	objCh := make(chan minioclient.ObjectInfo)
	fb.listObjFn = func() <-chan minioclient.ObjectInfo {
		return objCh
	}
	fb.listFwLogObjFn = func(prefix string) <-chan minioclient.ObjectInfo {
		return objCh
	}
	httphdr := http.Header{}
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
	ret, err := s.List("test.test", "", api.ListWatchOptions{})
	AssertOk(t, err, "List failed on store")
	Assert(t, len(ret) == 3, "received wrong number of objects")
	// these are not implemented and should not cause any cores
	s.Set("", 0, nil, nil)
	s.Get("")
	s.Delete("", 0, nil)
	s.Mark("")
	s.Sweep("", nil)
	s.PurgeDeleted(0)
	s.Stat(nil)

	// Test fwlogs listing in storeimpl
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector =
		[]string{"00ae.cd00.112e/2020/3/17/2/2020-03-17T02:45:09_2020-03-17T02:45:10.csv.gzip"}
	ret, err = s.List("default.fwlogs", "", opts)
	AssertOk(t, err, "list failed on fwlogs bucket")
	Assert(t, len(ret) == 0, "list should not have returned any results")
}

func TestStoreWatcher(t *testing.T) {
	fb := mockBackend{}
	fwq := &mocks.FakeWatchEventQ{}
	fwp := &mocks.FakeWatchPrefixes{}
	fwp.Qmap = make(map[string]*mocks.FakeWatchEventQ)
	fwp.Getfn = func(path string) []watchstream.WatchEventQ {
		return []watchstream.WatchEventQ{fwq}
	}
	w := storeWatcher{client: &fb, bucket: "test", watchPrefixes: fwp}
	objCh := make(chan minioclient.ObjectInfo)
	fb.listObjFn = func() <-chan minioclient.ObjectInfo {
		return objCh
	}
	httphdr := http.Header{}
	objs := []minioclient.ObjectInfo{
		{
			ETag:     "abcdef",
			Key:      "test.file1",
			Metadata: httphdr,
		},
		{
			ETag:     "abcdef",
			Key:      "test.file2",
			Metadata: httphdr,
		},
		{
			ETag:     "abcdef",
			Key:      "test.file3",
			Metadata: httphdr,
		},
	}
	go func() {
		for _, v := range objs {
			objCh <- v
		}
		close(objCh)
	}()
	ctx, cancel := context.WithCancel(context.Background())
	var cleanCalled int
	cleanFn := func() { cleanCalled++ }
	evCh := make(chan minioclient.NotificationInfo)
	fb.listenObjFn = func() <-chan minioclient.NotificationInfo {
		return evCh
	}
	go w.Watch(ctx, cleanFn)

	notif := minioclient.NotificationInfo{
		Err: nil,
		Records: []minioclient.NotificationEvent{
			{
				EventTime: "2019-02-22T23:20:49Z",
				EventName: "s3:ObjectCreated:MultiPartUploadComoplete",
			},
			{
				EventTime: "2019-02-22T23:20:50Z",
				EventName: "s3:ObjectCreated:MultiPartUploadComoplete",
			},
		},
	}
	evCh <- notif
	AssertEventually(t, func() (bool, interface{}) {
		return fwq.Enqueues == 2, nil
	}, "failed to recieve 2 enqueues")
	cancel()
}
