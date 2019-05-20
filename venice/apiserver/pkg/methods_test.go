package apisrvpkg

import (
	"context"
	"testing"

	"github.com/opentracing/opentracing-go"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api/errors"

	"github.com/pensando/sw/api/api_test"
	"github.com/pensando/sw/api/requirement"
	"github.com/pensando/sw/venice/utils/runtime"

	"github.com/pensando/sw/api/cache"

	"github.com/pensando/sw/api"
	cachemocks "github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/interfaces"
	apisrv "github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore"
	compliance "github.com/pensando/sw/venice/utils/kvstore/compliance"
)

// setStagingBufferInGrpcMD sets the GRPC metadata with buffer ID
func setStagingBufferInGrpcMD(ctx context.Context, id string) context.Context {
	pair := metadata.Pairs(apisrv.RequestParamStagingBufferID, id)
	inMd, ok := metadata.FromIncomingContext(ctx)
	if ok {
		nMd := metadata.Join(inMd, pair)
		return metadata.NewIncomingContext(ctx, nMd)
	}

	return metadata.NewIncomingContext(ctx, pair)
}

var globFakeOverlay *cachemocks.FakeOverlay

func fakeGetOverlay(tenant, id string) (apiintf.OverlayInterface, error) {
	return globFakeOverlay, nil
}

func fakeIsDryRun(ctx context.Context) bool {
	return false
}

// TestMethodWiths
// Test registration of various hooks to method.
// Check all precommit and post commit functions are called and skipping
// KV operation feedback from hook is honored
func TestMethodWiths(t *testing.T) {
	MustGetAPIServer()
	singletonAPISrv.runstate.running = true
	singletonAPISrv.config.IsDryRun = fakeIsDryRun
	singletonAPISrv.config.GetOverlay = fakeGetOverlay
	fsvc := mocks.NewFakeService()

	req := mocks.NewFakeMessage("TestType1", "TestType1", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("TestType2", "TestType2", true).(*mocks.FakeMessage)
	f := mocks.NewFakeMethod(true).(*mocks.FakeMethod)
	// Add a few Pres and Posts and skip KV for testing
	m := NewMethod(fsvc, req, resp, "testm", "TestMethodWiths").WithVersion("v1").WithPreCommitHook(f.PrecommitFunc).WithPreCommitHook(f.PrecommitFunc).WithPreCommitHook(f.PrecommitFunc)
	m = m.WithPostCommitHook(f.PostcommitfFunc).WithPostCommitHook(f.PostcommitfFunc).WithResponseWriter(f.RespWriterFunc).WithMakeURI(f.MakeURIFunc)
	m = m.WithOper("POST").WithVersion("Vtest")

	reqmsg := TestType1{}
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "GET")
	mhdlr := m.(*MethodHdlr)
	if mhdlr.version != "Vtest" || mhdlr.oper != "POST" {
		t.Errorf("Flags not set correction ver[%v] oper[%v]", mhdlr.version, mhdlr.oper)
	}
	ctx := metadata.NewIncomingContext(context.Background(), md)
	respmsg, _ := m.HandleInvocation(ctx, reqmsg)

	if r, ok := respmsg.(string); ok {
		if r != "TestResponse" {
			t.Errorf("Expecting a result of [TestResponse] got [%v]", r)
		}
	} else {
		t.Errorf("Result not overridden")
	}

	if f.Pres != 3 {
		t.Errorf("Expecting 3 precommit invocations found %v", f.Pres)
	}
	if f.Posts != 2 {
		t.Errorf("Expecting 2 postcommit invocations found %v", f.Posts)
	}
	// There should be no KV operations involved since the precommit skipped KV
	if req.Kvreads != 0 {
		t.Errorf("Expecting no KV reads but found %v", req.Kvreads)
	}

	if f.MakeURIs != 1 {
		t.Errorf("expecting 1 MakeURI request got %d", f.MakeURIs)
	}
	m.Disable()
	if mhdlr.enabled == true {
		t.Errorf("method should have been disabled")
	}
	m.Enable()
	if mhdlr.enabled == false {
		t.Errorf("method should have been enabled")
	}

	msg1 := m.GetRequestType()
	if msg1 != req {
		t.Errorf("retrieved wrong request type")
	}
	msg1 = m.GetResponseType()
	if msg1 != resp {
		t.Errorf("retrieved wrong response type")
	}
	if m.GetPrefix() != "testm" {
		t.Errorf("wrong prefix")
	}
	if m.GetService() != fsvc {
		t.Errorf("could not retrieve sercvice")
	}
}

// TestMethodKvWrite
// Validate KV operation on Method invocation
func TestMethodKvWrite(t *testing.T) {
	req := mocks.NewFakeMessage("reqmsgA", "/requestmsg/A", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("reqmsgB", "/responsmsg/A", true).(*mocks.FakeMessage)
	fsvc := mocks.NewFakeService()
	fcache := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}}
	ftxn := &cachemocks.FakeTxn{}
	fcache.Txn = ftxn

	txnSuccess := true
	ftxn.Commitfn = func(ctx context.Context) (kvstore.TxnResponse, error) {
		return kvstore.TxnResponse{Succeeded: txnSuccess}, nil
	}
	fcache.Kvconn = fcache
	MustGetAPIServer()
	singletonAPISrv.runstate.running = true
	singletonAPISrv.config.IsDryRun = fakeIsDryRun
	singletonAPISrv.config.GetOverlay = fakeGetOverlay
	singletonAPISrv.apiGraph = &cachemocks.FakeGraphInterface{}
	singletonAPISrv.newLocalOverlayFunc = func(tenant, id, baseKey string, c apiintf.CacheInterface, asrv apisrv.Server) (apiintf.OverlayInterface, error) {
		return globFakeOverlay, nil
	}
	globFakeOverlay = &cachemocks.FakeOverlay{Interface: fcache, Reqs: &cachemocks.FakeRequirementSet{}}
	globFakeOverlay.Txn = ftxn

	singletonAPISrv.apiCache = fcache
	oldkvPool := singletonAPISrv.kvPool
	oldkvPoolSize := singletonAPISrv.kvPoolsize
	singletonAPISrv.kvPool = nil
	singletonAPISrv.kvPool = []kvstore.Interface{fcache}
	defer func() { singletonAPISrv.kvPool, singletonAPISrv.kvPoolsize = oldkvPool, oldkvPoolSize }()
	singletonAPISrv.kvPoolsize = 1

	m := NewMethod(fsvc, req, resp, "testm", "TestMethodKvWrite")
	reqmsg := apitest.TestObj{}
	req.RuntimeObj = &reqmsg
	req.IgnoreTxnWrite = true
	// Set the same version as the apiServer
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "GET")
	ctx := metadata.NewIncomingContext(context.Background(), md)
	// this call should fail because the data is not added to the kvstore yet
	// fakemessage implements a dummy kv read/write
	if respmsg, _ := m.HandleInvocation(ctx, reqmsg); respmsg != nil {
		t.Errorf("Expecting err but succeeded")
	}
	if req.Kvreads != 1 {
		t.Errorf("Expecting [1] read but found [%v]", req.Kvreads)
	}
	// Now add the object and check
	md1 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "POST")
	ftxn.Empty = true
	ctx1 := metadata.NewIncomingContext(context.Background(), md1)
	m.HandleInvocation(ctx1, reqmsg)
	if globFakeOverlay.CreatePrimaries != 1 && globFakeOverlay.Commits != 1 {
		t.Errorf("Expecting [1] CreatePrimary and [1] Commit got [%v/%v]", globFakeOverlay.CreatePrimaries, globFakeOverlay.Commits)
	}
	// Now modify the object and check
	md2 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "PUT")
	ctx2 := metadata.NewIncomingContext(context.Background(), md2)
	m.HandleInvocation(ctx2, reqmsg)
	if globFakeOverlay.UpdatePrimaries != 1 && globFakeOverlay.Commits != 2 {
		t.Errorf("Expecting [2] CreatePrimary and [2] Commit got [%v/%v]", globFakeOverlay.UpdatePrimaries, globFakeOverlay.Commits)
	}
	// Now delete the object and check
	md3 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "DELETE")
	ctx3 := metadata.NewIncomingContext(context.Background(), md3)
	span := opentracing.StartSpan("delete")
	statMap := map[string]apiintf.ObjectStat{
		"/requestmsg/A": {Key: "/requestmsg/A", Valid: true, Revision: 9},
	}

	fcache.StatFn = func(keys []string) []apiintf.ObjectStat {
		var ret []apiintf.ObjectStat
		for i := range keys {
			ret = append(ret, statMap[keys[i]])
		}
		return ret
	}
	fcache.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		ret := into.(*apitest.TestObj)
		*ret = reqmsg
		return nil
	}
	ftxn.Empty = false
	ctx3 = opentracing.ContextWithSpan(ctx3, span)
	if _, err := m.HandleInvocation(ctx3, reqmsg); err != nil {
		t.Errorf("Expecting success but failed %v", err)
	}
	if globFakeOverlay.DeletePrimaries != 1 {
		t.Errorf("Expecting [1] TxnOp but found [%v]", globFakeOverlay.DeletePrimaries)
	}

	// Test with staging set
	req.Kvreads, req.Kvwrites, req.Kvdels = 0, 0, 0
	globFakeOverlay = &cachemocks.FakeOverlay{Interface: fcache, Reqs: &cachemocks.FakeRequirementSet{}}
	// sctx := setStagingBufferInGrpcMD(ctx, "testBuffer")
	md = metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "GET",
		apisrv.RequestParamStagingBufferID, "testBuffer")
	sctx := metadata.NewIncomingContext(ctx, md)
	if _, err := m.HandleInvocation(sctx, reqmsg); err != nil {
		t.Errorf("Expecting to suceed but failed (%+v)", err)
	}
	if req.Kvreads != 1 {
		t.Errorf("Expecting [1] read but found [%v]", req.Kvreads)
	}

	md = metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "LIST",
		apisrv.RequestParamStagingBufferID, "testBuffer")
	sctx = metadata.NewIncomingContext(ctx, md)
	lopst := api.ListWatchOptions{}
	if _, err := m.HandleInvocation(sctx, lopst); err != nil {
		t.Errorf("Expecting to suceed but failed (%+v)", err)
	}
	if req.Kvreads != 1 {
		t.Errorf("Expecting [1] read but found [%v]", req.Kvreads)
	}

	md = metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "POST",
		apisrv.RequestParamStagingBufferID, "testBuffer")
	sctx = metadata.NewIncomingContext(ctx, md)
	if _, err := m.HandleInvocation(sctx, reqmsg); err != nil {
		t.Errorf("Expecting to suceed but failed (%+v)", err)
	}
	if req.Kvwrites != 0 {
		t.Errorf("Expecting [0] writes but found [%v]", req.Kvwrites)
	}
	if globFakeOverlay.CreatePrimaries != 1 {
		t.Errorf("Expecting [1] writes but found [%v]", globFakeOverlay.CreatePrimaries)
	}

	md = metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "PUT",
		apisrv.RequestParamStagingBufferID, "testBuffer")
	sctx = metadata.NewIncomingContext(ctx, md)
	if _, err := m.HandleInvocation(sctx, reqmsg); err != nil {
		t.Errorf("Expecting to suceed but failed (%+v)", err)
	}
	if req.Kvwrites != 0 {
		t.Errorf("Expecting [0] writes but found [%v]", req.Kvwrites)
	}
	if globFakeOverlay.UpdatePrimaries != 1 {
		t.Errorf("Expecting [1] writes but found [%v]", globFakeOverlay.UpdatePrimaries)
	}

	req.RuntimeObj = &reqmsg
	md = metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "DELETE",
		apisrv.RequestParamStagingBufferID, "testBuffer")
	sctx = metadata.NewIncomingContext(ctx, md)
	if _, err := m.HandleInvocation(sctx, reqmsg); err != nil {
		t.Errorf("Expecting to suceed but failed (%+v)", err)
	}
	if req.Kvdels != 0 {
		t.Errorf("Expecting [0] deletes but found [%v]", req.Kvwrites)
	}
	if globFakeOverlay.DeletePrimaries != 1 {
		t.Errorf("Expecting [1] deletes but found [%v]", globFakeOverlay.DeletePrimaries)
	}
}

func TestMethodKvList(t *testing.T) {
	req := mocks.NewFakeMessage("reqmsgA", "/requestmsg/A", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("reqmsgB", "/responsmsg/A", true).(*mocks.FakeMessage)

	MustGetAPIServer()
	singletonAPISrv.runstate.running = true
	singletonAPISrv.config.IsDryRun = fakeIsDryRun
	singletonAPISrv.config.GetOverlay = fakeGetOverlay

	m := NewMethod(nil, req, resp, "testm", "TestMethodKvWrite")
	reqmsg := api.ListWatchOptions{}

	// Set the same version as the apiServer
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "LIST")
	ctx := metadata.NewIncomingContext(context.Background(), md)
	if respmsg, _ := m.HandleInvocation(ctx, reqmsg); respmsg != nil {
		t.Errorf("Expecting err but succeeded")
	}
	if resp.Kvlists != 1 {
		t.Errorf("Expecting [1] kvlist but found [%v]", req.Kvlists)
	}
}

func TestMapOper(t *testing.T) {
	req := mocks.NewFakeMessage("reqmsgA", "/requestmsg/A", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("reqmsgA", "/responsmsg/A", true).(*mocks.FakeMessage)

	MustGetAPIServer()
	singletonAPISrv.runstate.running = true
	singletonAPISrv.config.IsDryRun = fakeIsDryRun
	singletonAPISrv.config.GetOverlay = fakeGetOverlay

	m := NewMethod(nil, req, resp, "testm", "TestMethodKvWrite")
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "GET")
	// Test that the oper method is correct
	mhdlr := m.(*MethodHdlr)
	if mhdlr.mapOper(md) != apiintf.GetOper {
		t.Errorf("Found wrong oper type")
	}
	md = metadata.Pairs(apisrv.RequestParamVersion, "v1")
	m.WithOper("create")
	if mhdlr.mapOper(md) != apiintf.CreateOper {
		t.Errorf("Found wrong oper type")
	}
	m.WithOper("update")
	if mhdlr.mapOper(md) != apiintf.UpdateOper {
		t.Errorf("Found wrong oper type")
	}
	m.WithOper("get")
	if mhdlr.mapOper(md) != apiintf.GetOper {
		t.Errorf("Found wrong oper type")
	}
	m.WithOper("delete")
	if mhdlr.mapOper(md) != apiintf.DeleteOper {
		t.Errorf("Found wrong oper type")
	}
	m.WithOper("watch")
	if mhdlr.mapOper(md) != apiintf.WatchOper {
		t.Errorf("Found wrong oper type")
	}
	md = metadata.Pairs(apisrv.RequestParamVersion, "v1", apisrv.RequestParamMethod, "create")
	m.WithOper("")
	if mhdlr.mapOper(md) != apiintf.CreateOper {
		t.Errorf("Found wrong oper type")
	}
	md = metadata.Pairs(apisrv.RequestParamVersion, "v1", apisrv.RequestParamMethod, "update")
	if mhdlr.mapOper(md) != apiintf.UpdateOper {
		t.Errorf("Found wrong oper type")
	}
	md = metadata.Pairs(apisrv.RequestParamVersion, "v1", apisrv.RequestParamMethod, "get")
	if mhdlr.mapOper(md) != apiintf.GetOper {
		t.Errorf("Found wrong oper type")
	}
	md = metadata.Pairs(apisrv.RequestParamVersion, "v1", apisrv.RequestParamMethod, "delete")
	if mhdlr.mapOper(md) != apiintf.DeleteOper {
		t.Errorf("Found wrong oper type")
	}
	md = metadata.Pairs(apisrv.RequestParamVersion, "v1", apisrv.RequestParamMethod, "watch")
	if mhdlr.mapOper(md) != apiintf.WatchOper {
		t.Errorf("Found wrong oper type")
	}
}

var cmpVer int64

func testTxnPreCommithook(ctx context.Context,
	kv kvstore.Interface,
	txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool,
	i interface{}) (interface{}, bool, error) {
	txn.AddComparator(kvstore.Compare(kvstore.WithVersion("/requestmsg/A/NotThere"), "=", cmpVer))
	return i, true, nil
}

func TestDisabledMethod(t *testing.T) {

}
func TestTxn(t *testing.T) {
	svc := mocks.NewFakeService()
	req := mocks.NewFakeMessage("reqmsgA", "/requestmsg/A", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("reqmsgA", "/responsmsg/A", true).(*mocks.FakeMessage)
	req = req.WithTransform("v1", "v2", req.TransformCb).(*mocks.FakeMessage)
	resp = resp.WithTransform("v2", "v1", resp.TransformCb).(*mocks.FakeMessage)
	fkv := cachemocks.FakeKvStore{}
	fcache := &cachemocks.FakeCache{FakeKvStore: fkv}
	ftxn := &cachemocks.FakeTxn{}
	fcache.Txn = ftxn

	MustGetAPIServer()
	singletonAPISrv.runstate.running = true
	singletonAPISrv.config.IsDryRun = fakeIsDryRun
	singletonAPISrv.config.GetOverlay = fakeGetOverlay
	singletonAPISrv.apiCache = fcache
	singletonAPISrv.apiGraph = &cachemocks.FakeGraphInterface{}
	singletonAPISrv.newLocalOverlayFunc = func(tenant, id, baseKey string, c apiintf.CacheInterface, asrv apisrv.Server) (apiintf.OverlayInterface, error) {
		return globFakeOverlay, nil
	}
	globFakeOverlay = &cachemocks.FakeOverlay{Interface: fcache, Reqs: &cachemocks.FakeRequirementSet{}}
	globFakeOverlay.Txn = ftxn

	statMap := map[string]apiintf.ObjectStat{
		"/txn/testobj": {Key: "/txn/testobj", Valid: true, Revision: 9},
	}

	fcache.StatFn = func(keys []string) []apiintf.ObjectStat {
		var ret []apiintf.ObjectStat
		for i := range keys {
			ret = append(ret, statMap[keys[i]])
		}
		return ret
	}

	// Add a few Pres and Posts and skip KV for testing
	m := NewMethod(svc, req, resp, "testm", "TestMethodKvWrite").WithPreCommitHook(testTxnPreCommithook)
	reqmsg := compliance.TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj1"}}
	req.RuntimeObj = &reqmsg
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "POST")
	ctx := metadata.NewIncomingContext(context.Background(), md)
	_, err := m.HandleInvocation(ctx, reqmsg)
	if err != nil {
		t.Fatalf("failed handle invocation (%v)", apierrors.FromError(err))
	}
	if globFakeOverlay.CreatePrimaries != 1 {
		t.Fatalf("Txn Write: expecting [1] saw [%d]", globFakeOverlay.CreatePrimaries)
	}
	// Modify the same object
	md1 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "PUT")
	ctx1 := metadata.NewIncomingContext(context.Background(), md1)
	req.Kvpath = mocks.TxnTestKey
	m.HandleInvocation(ctx1, reqmsg)
	if globFakeOverlay.UpdatePrimaries != 1 {
		t.Fatalf("Txn Write: expecting [1] saw [%d]", globFakeOverlay.UpdatePrimaries)
	}
	// Try a transaction which should fail.
	ftxn.Commitfn = func(ctx context.Context) (kvstore.TxnResponse, error) {
		return kvstore.TxnResponse{Succeeded: false}, nil
	}

	md1 = metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "PUT")
	ctx1 = metadata.NewIncomingContext(context.Background(), md1)
	_, err = m.HandleInvocation(ctx1, reqmsg)
	if globFakeOverlay.UpdatePrimaries != 2 {
		t.Fatalf("Txn Write: expecting [2] saw [%d]", globFakeOverlay.UpdatePrimaries)
	}
	if err == nil {
		t.Fatalf("should have failed")
	}
	ftxn.Commitfn = nil
	// Delete the Object
	md2 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "DELETE")
	ctx2 := metadata.NewIncomingContext(context.Background(), md2)
	cmpVer = 0
	fcache.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		ret := into.(*compliance.TestObj)
		*ret = reqmsg
		return nil
	}
	ret, err := m.HandleInvocation(ctx2, reqmsg)
	if err != nil {
		t.Fatalf("Invocation failed (%+v)", apierrors.FromError(err))
	}
	if globFakeOverlay.DeletePrimaries != 1 {
		t.Fatalf("Txn Del: expecting [1] saw [%d]", globFakeOverlay.DeletePrimaries)
	}
	if _, ok := ret.(compliance.TestObj); !ok {
		t.Fatalf("returned object is incorrect (%v)", ret)
	}
}

func TestTransforms(t *testing.T) {
	svc := mocks.NewFakeService()
	req := mocks.NewFakeMessage("reqmsgA", "/requestmsg/A", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("reqmsgA", "/responsmsg/A", true).(*mocks.FakeMessage)
	req = req.WithTransform("v1", "v2", req.TransformCb).(*mocks.FakeMessage)
	resp = resp.WithTransform("v2", "v1", resp.TransformCb).(*mocks.FakeMessage)

	MustGetAPIServer()
	singletonAPISrv.runstate.running = true
	singletonAPISrv.config.IsDryRun = fakeIsDryRun
	singletonAPISrv.config.GetOverlay = fakeGetOverlay

	singletonAPISrv.version = "v2"
	m := NewMethod(svc, req, resp, "testm", "TestMethodKvWrite")
	reqmsg := TestType1{}

	// Disable method and invoke.
	m.Disable()
	ctx := context.Background()
	_, err := m.HandleInvocation(ctx, reqmsg)
	if err == nil {
		t.Fatal("should have failed but succeeded")
	}
	m.Enable()
	// Without any MetadataHeaderPrefix
	_, err = m.HandleInvocation(ctx, reqmsg)
	if err == nil {
		t.Fatal("should have failed but succeeded")
	}
	// Set the same version as the apiServer
	md := metadata.Pairs(apisrv.RequestParamVersion, "v1",
		apisrv.RequestParamMethod, "GET")
	ctx = metadata.NewIncomingContext(ctx, md)
	m.HandleInvocation(ctx, reqmsg)
	if len(req.CalledTxfms) != 1 || len(resp.CalledTxfms) != 1 {
		t.Fatalf("transforms not called req[%v] resp[%v]", req.CalledTxfms, resp.CalledTxfms)
	}
	if req.CalledTxfms[0] != "v1-v2" || resp.CalledTxfms[0] != "v2-v1" {
		t.Errorf("transforms not called with right versions req[%v] resp[%v]", req.CalledTxfms, resp.CalledTxfms)
	}
}

func TestWithReferences(t *testing.T) {
	req := mocks.NewFakeMessage("reqmsgA", "/requestmsg/A", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("reqmsgB", "/responsmsg/A", true).(*mocks.FakeMessage)
	fsvc := mocks.NewFakeService()
	fcache := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}}
	ftxn := &cachemocks.FakeTxn{}
	globFakeOverlay = &cachemocks.FakeOverlay{Interface: fcache}
	fg := &cachemocks.FakeGraphInterface{}
	reqs := requirement.NewRequirementSet(fg, globFakeOverlay, nil)
	globFakeOverlay.Reqs = reqs
	globFakeOverlay.Kvconn = fcache
	fcache.Txn = ftxn
	globFakeOverlay.Txn = ftxn

	req = req.WithReferencesGetter(req.GetReferencesFunc).(*mocks.FakeMessage)
	txnSuccess := true
	ftxn.Commitfn = func(ctx context.Context) (kvstore.TxnResponse, error) {
		return kvstore.TxnResponse{Succeeded: txnSuccess}, nil
	}

	robjcb := func(in interface{}) runtime.Object {
		iobj := in.(apitest.TestObj)
		riobj := &iobj
		return riobj
	}
	req.WithGetRuntimeObject(robjcb)
	resp.WithGetRuntimeObject(robjcb)

	MustGetAPIServer()
	singletonAPISrv.runstate.running = true
	singletonAPISrv.config.IsDryRun = fakeIsDryRun
	singletonAPISrv.config.GetOverlay = fakeGetOverlay
	singletonAPISrv.apiGraph = &cachemocks.FakeGraphInterface{}
	singletonAPISrv.newLocalOverlayFunc = func(tenant, id, baseKey string, c apiintf.CacheInterface, asrv apisrv.Server) (apiintf.OverlayInterface, error) {
		return globFakeOverlay, nil
	}
	singletonAPISrv.apiCache = fcache
	oldkvPool := singletonAPISrv.kvPool
	oldkvPoolSize := singletonAPISrv.kvPoolsize
	singletonAPISrv.kvPool = nil
	singletonAPISrv.kvPool = []kvstore.Interface{fcache}
	defer func() { singletonAPISrv.kvPool, singletonAPISrv.kvPoolsize = oldkvPool, oldkvPoolSize }()
	singletonAPISrv.kvPoolsize = 1
	statMap := map[string]apiintf.ObjectStat{
		"/requestmsg/A": {Key: "/requestmsg/A", Valid: true, Revision: 9},
		"/requestmsg/B": {Key: "/requestmsg/B", Valid: true, Revision: 10},
		"/requestmsg/C": {Key: "/requestmsg/C", Valid: true, Revision: 11},
	}

	globFakeOverlay.StatFn = func(keys []string) []apiintf.ObjectStat {
		var ret []apiintf.ObjectStat
		for i := range keys {
			ret = append(ret, statMap[keys[i]])
		}
		return ret
	}
	m := NewMethod(fsvc, req, resp, "testm", "TestMethodKvWrite")
	reqmsg := apitest.TestObj{}
	req.IgnoreTxnWrite = true
	// Set the same version as the apiServer
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "POST")
	ctx := metadata.NewIncomingContext(context.Background(), md)
	if len(cache.GetOverlays()) != 0 {
		t.Errorf("Expecting no overlays to be created. got [%d]", len(cache.GetOverlays()))
	}
	refMap := map[string]apiintf.ReferenceObj{
		"testField": {RefType: apiintf.NamedReference, Refs: []string{"/requestmsg/B", "/requestmsg/C"}},
	}
	req.RefMap = refMap
	fcache.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		reqmsg.Clone(into)
		return nil
	}
	req.RuntimeObj = &reqmsg
	// Test with references
	if _, err := m.HandleInvocation(ctx, reqmsg); err != nil {
		t.Errorf("Expecting success but failed (%s)", err)
	}
	if req.Kvwrites != 0 {
		t.Errorf("Expecting [0] kvwrite but found [%v]", req.Kvwrites)
	}
	if globFakeOverlay.CreatePrimaries != 1 {
		t.Errorf("Expecting [1] Txnwrites but found [%v]", globFakeOverlay.CreatePrimaries)
	}
	if req.GetReferencesCalled != 1 {
		t.Errorf("expecting call to Getrefecences [1] got [%v]", req.GetReferencesCalled)
	}
	if len(cache.GetOverlays()) != 0 {
		t.Errorf("Expecting no overlays to be created. Dounc [%d]", len(cache.GetOverlays()))
	}
	if ftxn.CommitOps != 1 {
		t.Errorf("expecting call to Apply got [%v]", ftxn.CommitOps)
	}

	if len(cache.GetOverlays()) != 0 {
		t.Errorf("Expecting no overlays to be created. Dounc [%d]", len(cache.GetOverlays()))
	}
	if ftxn.CommitOps != 1 {
		t.Fatalf("Expecting [1] commit ops got [%v]", ftxn.CommitOps)
	}

	// Delete Operation
	req.Kvwrites, req.Txnwrites, req.GetReferencesCalled = 0, 0, 0
	ftxn.TouchOps, ftxn.Ops, ftxn.Cmps, ftxn.CommitOps = 0, nil, nil, 0
	req.RuntimeObj = &reqmsg
	md = metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "DELETE")
	ctx = metadata.NewIncomingContext(context.Background(), md)
	if _, err := m.HandleInvocation(ctx, reqmsg); err != nil {
		t.Errorf("Expecting success but failed (%s)", err)
	}
	if globFakeOverlay.DeletePrimaries != 1 {
		t.Errorf("Expecting [1] DeletePrimaries but found [%v]", globFakeOverlay.DeletePrimaries)
	}
	if req.GetReferencesCalled != 0 {
		t.Errorf("expecting call to Getrefecences [1] got [%v]", req.GetReferencesCalled)
	}
	if len(cache.GetOverlays()) != 0 {
		t.Errorf("Expecting no overlays to be created. Dounc [%d]", len(cache.GetOverlays()))
	}

	// Staged operation
	req.Kvwrites, req.Txnwrites, req.Txndels, req.GetReferencesCalled = 0, 0, 0, 0
	ftxn.TouchOps, ftxn.Ops, ftxn.Cmps = 0, nil, nil
	md = metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "POST",
		apisrv.RequestParamStagingBufferID, "testBuffer")
	ctx = metadata.NewIncomingContext(ctx, md)
	if _, err := m.HandleInvocation(ctx, reqmsg); err != nil {
		t.Errorf("Expecting to suceed but failed (%+v)", err)
	}
	if ftxn.CommitOps != 1 {
		t.Fatalf("Expecting [1] commit ops got [%v]", ftxn.CommitOps)
	}

	if req.Kvwrites != 0 {
		t.Errorf("Expecting [1] read but found [%v]", req.Kvreads)
	}
	if _, err := m.HandleInvocation(ctx, reqmsg); err != nil {
		t.Errorf("Expecting success but failed (%s)", err)
	}
	if req.Kvwrites != 0 {
		t.Errorf("Expecting [0] kvwrite but found [%v]", req.Kvwrites)
	}
	if req.Txndels != 0 {
		t.Errorf("Expecting [0] Txndels but found [%v]", req.Txndels)
	}
	if len(cache.GetOverlays()) != 0 {
		t.Errorf("Expecting no overlays to be created. Dounc [%d]", len(cache.GetOverlays()))
	}
}
