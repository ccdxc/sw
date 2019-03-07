package vospkg

import (
	"context"
	"testing"

	"github.com/pensando/sw/api/generated/objstore"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/vos"
)

func TestPlugins(t *testing.T) {
	fb := &mockBackend{}
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
	ps := newPluginSet("test")
	ps.registerPlugin(vos.PreOp, vos.Upload, cbFunc1)
	ps.registerPlugin(vos.PostOp, vos.Upload, cbFunc2)
	ps.registerPlugin(vos.PreOp, vos.Download, cbFunc1)
	ps.registerPlugin(vos.PostOp, vos.Download, cbFunc2)
	ps.registerPlugin(vos.PreOp, vos.Get, cbFunc1)
	ps.registerPlugin(vos.PostOp, vos.Get, cbFunc2)
	ps.registerPlugin(vos.PreOp, vos.List, cbFunc1)
	ps.registerPlugin(vos.PostOp, vos.List, cbFunc2)
	ps.registerPlugin(vos.PreOp, vos.Delete, cbFunc1)
	ps.registerPlugin(vos.PostOp, vos.Delete, cbFunc2)
	ps.registerPlugin(vos.PreOp, vos.Update, cbFunc1)
	ps.registerPlugin(vos.PostOp, vos.Update, cbFunc2)
	ps.registerPlugin(vos.PreOp, vos.Watch, cbFunc1)
	ps.registerPlugin(vos.PostOp, vos.Watch, cbFunc2)
	ps.registerPlugin(vos.PreOp, vos.Watch, cbFunc1)
	ps.registerPlugin(vos.PostOp, vos.Watch, cbFunc2)
	ps.registerPlugin(vos.PreOp, vos.Upload, cbFunc1)
	ps.registerPlugin(vos.PostOp, vos.Upload, cbFunc2)
	ctx := context.Background()
	ps.RunPlugins(ctx, vos.PreOp, vos.Upload, nil, fb)
	ps.RunPlugins(ctx, vos.PostOp, vos.Upload, nil, fb)
	ps.RunPlugins(ctx, vos.PreOp, vos.Download, nil, fb)
	ps.RunPlugins(ctx, vos.PostOp, vos.Download, nil, fb)
	ps.RunPlugins(ctx, vos.PreOp, vos.Get, nil, fb)
	ps.RunPlugins(ctx, vos.PostOp, vos.Get, nil, fb)
	ps.RunPlugins(ctx, vos.PreOp, vos.List, nil, fb)
	ps.RunPlugins(ctx, vos.PostOp, vos.List, nil, fb)
	ps.RunPlugins(ctx, vos.PreOp, vos.Delete, nil, fb)
	ps.RunPlugins(ctx, vos.PostOp, vos.Delete, nil, fb)
	ps.RunPlugins(ctx, vos.PreOp, vos.Update, nil, fb)
	ps.RunPlugins(ctx, vos.PostOp, vos.Update, nil, fb)
	ps.RunPlugins(ctx, vos.PreOp, vos.Watch, nil, fb)
	ps.RunPlugins(ctx, vos.PostOp, vos.Watch, nil, fb)

	Assert(t, callMap1[vos.Upload] == 2, "expecting 2 got [%v] for upload", callMap1[vos.Upload])
	Assert(t, callMap2[vos.Upload] == 2, "expecting 2 got [%v] for upload", callMap2[vos.Upload])
	Assert(t, callMap1[vos.Download] == 1, "expecting 1 got [%v] for upload", callMap1[vos.Download])
	Assert(t, callMap2[vos.Download] == 1, "expecting 1 got [%v] for upload", callMap2[vos.Get])
	Assert(t, callMap1[vos.Get] == 1, "expecting 1 got [%v] for upload", callMap1[vos.Get])
	Assert(t, callMap2[vos.Get] == 1, "expecting 1 got [%v] for upload", callMap2[vos.Get])
	Assert(t, callMap1[vos.List] == 1, "expecting 1 got [%v] for upload", callMap1[vos.List])
	Assert(t, callMap2[vos.List] == 1, "expecting 1 got [%v] for upload", callMap2[vos.List])
	Assert(t, callMap1[vos.Delete] == 1, "expecting 1 got [%v] for upload", callMap1[vos.Delete])
	Assert(t, callMap2[vos.Delete] == 1, "expecting 1 got [%v] for upload", callMap2[vos.Delete])
	Assert(t, callMap1[vos.Update] == 1, "expecting 1 got [%v] for upload", callMap1[vos.Update])
	Assert(t, callMap2[vos.Update] == 1, "expecting 1 got [%v] for upload", callMap2[vos.Update])
	Assert(t, callMap1[vos.Watch] == 2, "expecting 2 got [%v] for upload", callMap1[vos.Watch])
	Assert(t, callMap2[vos.Watch] == 2, "expecting 2 got [%v] for upload", callMap2[vos.Watch])

}
