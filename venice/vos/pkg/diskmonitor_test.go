package vospkg

import (
	"context"
	"testing"

	minioclient "github.com/minio/minio-go/v6"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	. "github.com/pensando/sw/venice/utils/testutils"
	vosinternalprotos "github.com/pensando/sw/venice/vos/protos"
)

type fakeDiskUpdateWatchServer struct {
	ctx context.Context
}

var testChannel = make(chan interface{}, 100)

// Send implements a mock interface
func (f *fakeDiskUpdateWatchServer) Send(event *vosinternalprotos.DiskUpdate) error {
	testChannel <- event
	return nil
}

// SetHeader implements a mock interface
func (f *fakeDiskUpdateWatchServer) SetHeader(metadata.MD) error { return nil }

// SendHeader implements a mock interface
func (f *fakeDiskUpdateWatchServer) SendHeader(metadata.MD) error { return nil }

// SetTrailer implements a mock interface
func (f *fakeDiskUpdateWatchServer) SetTrailer(metadata.MD) {}

// Context implements a mock interface
func (f *fakeDiskUpdateWatchServer) Context() context.Context {
	return f.ctx
}

// SendMsg implements a mock interface
func (f *fakeDiskUpdateWatchServer) SendMsg(m interface{}) error { return nil }

// RecvMsg implements a mock interface
func (f *fakeDiskUpdateWatchServer) RecvMsg(m interface{}) error { return nil }

func TestDiskUpdateOps(t *testing.T) {
	fb := &mockBackend{}
	inst := &instance{}
	inst.Init(fb)
	srv := grpcBackend{client: fb, instance: inst}
	ctx := context.Background()

	// Add a fake listobject function.
	// The objs array is empty because diskupdates are not objects.
	// This list function is needed to pass through the watchstream's
	// Dequeue function.
	objCh := make(chan minioclient.ObjectInfo)
	fb.listObjFn = func() <-chan minioclient.ObjectInfo {
		return objCh
	}
	objs := []minioclient.ObjectInfo{}
	go func() {
		for _, v := range objs {
			objCh <- v
		}
		close(objCh)
	}()

	cctx, cancel := context.WithCancel(ctx)
	defer cancel()

	fw := &fakeDiskUpdateWatchServer{ctx: cctx}
	go srv.WatchDiskThresholdUpdates(&api.ListWatchOptions{}, fw)

	paths := map[string]float64{"./": 0.00001}
	err := inst.createDiskUpdateWatcher(paths)
	Assert(t, err == nil, "failed to create disk update watcher")

	// Start monitor disks
	storeWatcher := inst.watcherMap[diskUpdateWatchPath]
	Assert(t, storeWatcher != nil, "store watcher is nil")

	event := <-testChannel
	Assert(t, event != nil, "received event is nil")
	diskUpdate, ok := event.(*vosinternalprotos.DiskUpdate)
	Assert(t, ok == true, "event is not a disk update")
	Assert(t, diskUpdate.Status.Path == "./", "diskupdate path is not correct")
	Assert(t, diskUpdate.Status.Size_ != 0, "diskupdate size is 0")
	Assert(t, diskUpdate.Status.UsedByNamespace != 0, "diskupdate used is 0")

	opts := api.ListWatchOptions{}
	opts.ObjectMeta = api.ObjectMeta{
		ResourceVersion: "10",
	}
	err = srv.WatchDiskThresholdUpdates(&opts, fw)
	Assert(t, err != nil, "filtering is not supported")
}
