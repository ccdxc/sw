package impl

import (
	"context"
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/generated/browser"
	"github.com/pensando/sw/api/graph"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	apisrvmocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	mocks2 "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestProcessQuery(t *testing.T) {
	fgraph := &mocks.FakeGraphInterface{}
	fcache := &mocks.FakeCache{}
	fakeKv := &mocks.FakeKvStore{}
	fakeTxn := &mocks.FakeTxn{}

	h := browserHooks{
		logger:  log.GetNewLogger(log.GetDefaultConfig("browser API Server Hook")),
		graphDb: fgraph,
		cache:   fcache,
		version: "v1",
	}

	apisrv := apisrvpkg.MustGetAPIServer()
	msgs := map[string]apiserver.Message{
		"test.TestMsg1":     apisrvmocks.NewFakeMessage("TestMsg1", "/test/path", false),
		"test.TestMsg1List": apisrvmocks.NewFakeMessage("TestMsg1List", "/test/path", false),
	}
	apisrv.RegisterMessages("test", msgs)

	statMap := map[string]apiintf.ObjectStat{
		"/test/obj1": {Key: "/test/obj1", Valid: true, Revision: 9},
		"/test/obj2": {Key: "/test/obj2", Valid: true, Revision: 10},
		"/test/obj3": {Key: "/test/obj3", Valid: true, Revision: 11},
	}

	fcache.StatFn = func(keys []string) []apiintf.ObjectStat {
		var ret []apiintf.ObjectStat
		for i := range keys {
			ret = append(ret, statMap[keys[i]])
		}
		return ret
	}
	sch := runtime.GetDefaultScheme()
	paths := map[string][]api.PathsMap{
		"test.Object": {
			{Key: "/test/obj1", URI: "/configs/test/v1/obj1"},
			{Key: "/test/obj2", URI: "/configs/test/v1/obj2"},
			{Key: "/test/obj3", URI: "/configs/test/v1/obj3"},
		},
	}
	sch.AddPaths(paths)
	// queries with no object in cache
	req := browser.BrowseRequest{
		URI:       "/test/Non-Existent",
		QueryType: browser.QueryType_Dependencies.String(),
		MaxDepth:  1,
	}
	ctx := context.Background()
	resp, kvwrite, err := h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	Assert(t, err != nil, "should have errored out but got nil", nil)
	Assert(t, resp != nil, "expecting non-nil response", nil)

	req.MaxDepth = 12
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	Assert(t, err != nil, "should have errored out but got nil", nil)
	Assert(t, resp != nil, "expecting non-nil response", nil)

	req.QueryType = browser.QueryType_DependedBy.String()
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	Assert(t, err != nil, "should have errored out but got nil", nil)
	Assert(t, resp != nil, "expecting non-nil response", nil)

	req.MaxDepth = 12
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	Assert(t, err != nil, "should have errored out but got nil", nil)
	Assert(t, resp != nil, "expecting non-nil response", nil)

	// Queries with object in cache but no refs

	req.URI, req.MaxDepth, req.QueryType = "/test/obj1", 1, browser.QueryType_Dependencies.String()
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	AssertOk(t, err, "should have succeded but got error (%s)", err)
	Assert(t, resp != nil, "expecting non-nil response", nil)
	bresp := resp.(browser.BrowseResponse)
	Assert(t, bresp.QueryType == browser.QueryType_Dependencies.String(), "invalid querytype", nil)
	Assert(t, bresp.RootURI == "/configs/test/v1/obj1", fmt.Sprintf("invalid root uri got [%v]", bresp.RootURI), nil)
	Assert(t, len(bresp.Objects) == 1, fmt.Sprintf("expecting only one Object, got [%d]", len(bresp.Objects)), nil)

	req.URI, req.MaxDepth, req.QueryType = "/test/obj1", 12, browser.QueryType_Dependencies.String()
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	AssertOk(t, err, "should have succeded but got error (%s)", err)
	Assert(t, resp != nil, "expecting non-nil response", nil)
	bresp = resp.(browser.BrowseResponse)
	Assert(t, bresp.QueryType == browser.QueryType_Dependencies.String(), "invalid querytype", nil)
	Assert(t, bresp.RootURI == "/configs/test/v1/obj1", fmt.Sprintf("invalid root uri got [%v]", bresp.RootURI), nil)
	Assert(t, len(bresp.Objects) == 1, fmt.Sprintf("expecting only one Object, got [%d]", len(bresp.Objects)), nil)

	req.URI, req.MaxDepth, req.QueryType = "/test/obj1", 1, browser.QueryType_DependedBy.String()
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	AssertOk(t, err, "should have succeded but got error (%s)", err)
	Assert(t, resp != nil, "expecting non-nil response", nil)
	bresp = resp.(browser.BrowseResponse)
	Assert(t, bresp.QueryType == browser.QueryType_DependedBy.String(), "invalid querytype", nil)
	Assert(t, bresp.RootURI == "/configs/test/v1/obj1", fmt.Sprintf("invalid root uri got [%v]", bresp.RootURI), nil)
	Assert(t, len(bresp.Objects) == 1, fmt.Sprintf("expecting only one Object, got [%d]", len(bresp.Objects)), nil)

	req.URI, req.MaxDepth, req.QueryType = "/test/obj1", 12, browser.QueryType_DependedBy.String()
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	AssertOk(t, err, "should have succeded but got error (%s)", err)
	Assert(t, resp != nil, "expecting non-nil response", nil)
	bresp = resp.(browser.BrowseResponse)
	Assert(t, bresp.QueryType == browser.QueryType_DependedBy.String(), "invalid querytype", nil)
	Assert(t, bresp.RootURI == "/configs/test/v1/obj1", fmt.Sprintf("invalid root uri got [%v]", bresp.RootURI), nil)
	Assert(t, len(bresp.Objects) == 1, fmt.Sprintf("expecting only one Object, got [%d]", len(bresp.Objects)), nil)

	// Queries with refs present
	refs1 := map[string][]string{
		"field1": {"/test/obj2", "/test/obj3"},
	}
	fgraph.Refs = refs1
	req.URI, req.MaxDepth, req.QueryType = "/test/obj1", 1, browser.QueryType_Dependencies.String()
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	AssertOk(t, err, "should have succeded but got error (%s)", err)
	Assert(t, resp != nil, "expecting non-nil response", nil)
	bresp = resp.(browser.BrowseResponse)
	Assert(t, bresp.QueryType == browser.QueryType_Dependencies.String(), "invalid querytype", nil)
	Assert(t, bresp.RootURI == "/configs/test/v1/obj1", fmt.Sprintf("invalid root uri got [%v]", bresp.RootURI), nil)
	Assert(t, len(bresp.Objects) == 1, fmt.Sprintf("expecting only one Object, got [%d]", len(bresp.Objects)), nil)
	Assert(t, len(bresp.Objects["/configs/test/v1/obj1"].Links["field1"].URI) == 2, fmt.Sprintf("expecting 2 Objects, got [%d][%v]", len(bresp.Objects["/configs/test/v1/obj1"].Links["field1"].URI), bresp.Objects["/configs/test/v1/obj1"].Links), nil)

	req.URI, req.MaxDepth, req.QueryType = "/test/obj1", 1, browser.QueryType_DependedBy.String()
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	AssertOk(t, err, "should have succeded but got error (%s)", err)
	Assert(t, resp != nil, "expecting non-nil response", nil)
	bresp = resp.(browser.BrowseResponse)
	Assert(t, bresp.QueryType == browser.QueryType_DependedBy.String(), "invalid querytype", nil)
	Assert(t, bresp.RootURI == "/configs/test/v1/obj1", fmt.Sprintf("invalid root uri got [%v]", bresp.RootURI), nil)
	Assert(t, len(bresp.Objects) == 1, fmt.Sprintf("expecting only one Object, got [%d]", len(bresp.Objects)), nil)
	Assert(t, len(bresp.Objects["/configs/test/v1/obj1"].Links["field1"].URI) == 2, fmt.Sprintf("expecting 2 Objects, got [%d][%v]", len(bresp.Objects["/configs/test/v1/obj1"].Links["field1"].URI), bresp.Objects["/configs/test/v1/obj1"].Links), nil)

	wrefs1 := map[string][]string{
		"field2": {"/test/obj3", "/test/obj4"},
	}
	srefs1 := map[string][]string{
		"field3": {"labels:test:TestMsg1:ten1:a=b"},
	}
	fgraph.WeakRefs = wrefs1
	fgraph.SelectorRefs = srefs1
	req.URI, req.MaxDepth, req.QueryType = "/test/obj1", 1, browser.QueryType_Dependencies.String()
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	AssertOk(t, err, "should have succeded but got error (%s)", err)
	Assert(t, resp != nil, "expecting non-nil response", nil)
	bresp = resp.(browser.BrowseResponse)
	Assert(t, bresp.QueryType == browser.QueryType_Dependencies.String(), "invalid querytype", nil)
	Assert(t, bresp.RootURI == "/configs/test/v1/obj1", fmt.Sprintf("invalid root uri got [%v]", bresp.RootURI), nil)
	Assert(t, len(bresp.Objects) == 1, fmt.Sprintf("expecting only one Object, got [%d]", len(bresp.Objects)), nil)
	Assert(t, len(bresp.Objects["/configs/test/v1/obj1"].Links["field1"].URI) == 2, fmt.Sprintf("expecting 2 Objects, got [%d][%v]", len(bresp.Objects["/configs/test/v1/obj1"].Links["field1"].URI), bresp.Objects["/configs/test/v1/obj1"].Links), nil)

	v1 := &graph.Vertex{This: "/test/obj1", Dir: graph.RefIn, Refs: make(map[string][]*graph.Vertex), WeakRefs: make(map[string][]*graph.Vertex)}
	v2 := &graph.Vertex{This: "/test/obj2", Dir: graph.RefIn, Refs: make(map[string][]*graph.Vertex)}
	v3 := &graph.Vertex{This: "/test/obj3", Dir: graph.RefIn, Refs: make(map[string][]*graph.Vertex)}
	v1.Refs["field"] = []*graph.Vertex{v2, v3}
	v1.WeakRefs["field2"] = []*graph.Vertex{v2, v3}
	v1.SelectorRefs = srefs1
	verts := map[string][]*graph.Vertex{
		"field2": {v1, v2, v3},
	}
	wverts := map[string][]*graph.Vertex{
		"field1": {v1, v2, v3},
	}
	fgraph.Verts = verts
	fgraph.WVerts = wverts
	req.URI, req.MaxDepth, req.QueryType = "/test/obj1", 12, browser.QueryType_Dependencies.String()
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	AssertOk(t, err, "should have succeded but got error (%s)", err)
	Assert(t, resp != nil, "expecting non-nil response", nil)
	bresp = resp.(browser.BrowseResponse)
	Assert(t, bresp.QueryType == browser.QueryType_Dependencies.String(), "invalid querytype", nil)
	Assert(t, bresp.RootURI == "/configs/test/v1/obj1", fmt.Sprintf("invalid root uri got [%v]", bresp.RootURI), nil)
	Assert(t, len(bresp.Objects) == 3, fmt.Sprintf("expecting only one Object, got [%d]", len(bresp.Objects)), nil)

	req.URI, req.MaxDepth, req.QueryType = "/test/obj1", 12, browser.QueryType_DependedBy.String()
	resp, kvwrite, err = h.processQuery(ctx, fakeKv, fakeTxn, "", apiintf.GetOper, false, req)
	Assert(t, kvwrite == false, "expecting kvwrite to be skipped", nil)
	AssertOk(t, err, "should have succeded but got error (%s)", err)
	Assert(t, resp != nil, "expecting non-nil response", nil)
	bresp = resp.(browser.BrowseResponse)
	Assert(t, bresp.QueryType == browser.QueryType_DependedBy.String(), "invalid querytype", nil)
	Assert(t, bresp.RootURI == "/configs/test/v1/obj1", fmt.Sprintf("invalid root uri got [%v]", bresp.RootURI), nil)
	Assert(t, len(bresp.Objects) == 3, fmt.Sprintf("expecting only one Object, got [%d]", len(bresp.Objects)), nil)
}

func TestBrowserRegistration(t *testing.T) {
	srv := mocks2.NewFakeService()
	meth := mocks2.NewFakeMethod(true)
	srv.AddMethod("Query", meth)
	srv.AddMethod("References", meth)
	srv.AddMethod("Referrers", meth)
	logger := log.GetDefaultInstance()
	registerBrowserHooks(srv, logger)
	fmeth := meth.(*mocks2.FakeMethod)
	if fmeth.Pres != 3 {
		t.Fatalf("unexpected number of hooks registered [%+v]", fmeth)
	}
}
