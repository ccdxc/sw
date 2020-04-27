package impl

import (
	"context"
	"fmt"
	"testing"

	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/browser"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestBrowserPreCallHooks_EmptyURI(t *testing.T) {
	ctx := context.Background()

	mdata := map[string]string{
		apiserver.RequestParamsRequestURI: "",
	}
	md := metadata.New(mdata)
	ctx = metadata.NewOutgoingContext(ctx, md)
	req := &browser.BrowseRequest{}
	req.Defaults("v1")
	h := browserHooks{logger: log.GetNewLogger(log.GetDefaultConfig("browserAPIGwHooks"))}
	sch := runtime.GetDefaultScheme()
	paths := map[string][]api.PathsMap{
		"test.Object": {
			{Key: "/testgrp/testobj/{Name}", URI: "/configs/testgrp/{version}/testobj/{Name}"},
		},
	}
	sch.AddPaths(paths)

	_, _, _, _, err := h.refereesPreCallHook(ctx, req, nil)
	Assert(t, err != nil, "getURI expected to fail, but passed", nil)
}

func TestBrowserPreCallHooks_NoURIMap(t *testing.T) {
	ctx := context.Background()

	mdata := map[string]string{}
	md := metadata.New(mdata)
	ctx = metadata.NewOutgoingContext(ctx, md)
	req := &browser.BrowseRequest{}
	req.Defaults("v1")
	h := browserHooks{logger: log.GetNewLogger(log.GetDefaultConfig("browserAPIGwHooks"))}
	sch := runtime.GetDefaultScheme()
	paths := map[string][]api.PathsMap{
		"test.Object": {
			{Key: "/testgrp/testobj/{Name}", URI: "/configs/testgrp/{version}/testobj/{Name}"},
		},
	}
	sch.AddPaths(paths)

	_, _, _, _, err := h.refereesPreCallHook(ctx, req, nil)
	Assert(t, err != nil, "getURI expected to fail, but passed", nil)
}

func TestBrowserPreCallHooks(t *testing.T) {
	ctx := context.Background()
	ctx = apigwpkg.NewContextWithUser(ctx, &auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testUser",
		},
		Spec: auth.UserSpec{
			Fullname: "Test User",
			Password: "password",
			Email:    "testuser@pensandio.io",
			Type:     auth.UserSpec_Local.String(),
		},
	})

	mdata := map[string]string{
		apiserver.RequestParamsRequestURI: "/configs/browser/dependencies/testgrp/v1/testobj/testname",
	}
	md := metadata.New(mdata)
	ctx = metadata.NewOutgoingContext(ctx, md)
	req := &browser.BrowseRequest{}
	req.Defaults("v1")
	h := browserHooks{logger: log.GetNewLogger(log.GetDefaultConfig("browserAPIGwHooks"))}
	sch := runtime.GetDefaultScheme()
	paths := map[string][]api.PathsMap{
		"test.Object": {
			{Key: "/testgrp/testobj/{Name}", URI: "/configs/testgrp/{version}/testobj/{Name}"},
		},
	}
	sch.AddPaths(paths)

	ctx, reti, _, skip, err := h.refereesPreCallHook(ctx, req, nil)
	ret := reti.(*browser.BrowseRequest)
	Assert(t, skip == false, "expecting false for skip", nil)
	AssertOk(t, err, "expecting to succeed got(%s)", err)
	Assert(t, ret.URI == "/testgrp/testobj/testname", fmt.Sprintf("got wrong URI[%v]", ret.URI), nil)
	Assert(t, ret.MaxDepth == 1, fmt.Sprintf("got wrong depth [%d]", ret.MaxDepth), nil)
	Assert(t, ret.QueryType == browser.QueryType_DependedBy.String(), fmt.Sprintf("got wrong type [%v]", ret.QueryType), nil)

	req1 := &browser.BrowseRequest{}
	req1.Defaults("v1")
	ctx, reti, _, skip, err = h.referencesPreCallHook(ctx, req, nil)
	ret = reti.(*browser.BrowseRequest)
	Assert(t, skip == false, "expecting false for skip", nil)
	AssertOk(t, err, "expecting to succeed got(%s)", err)
	Assert(t, ret.URI == "/testgrp/testobj/testname", fmt.Sprintf("got wrong URI[%v]", ret.URI), nil)
	Assert(t, ret.MaxDepth == 1, fmt.Sprintf("got wrong depth [%d]", ret.MaxDepth), nil)
	Assert(t, ret.QueryType == browser.QueryType_Dependencies.String(), fmt.Sprintf("got wrong type [%v]", ret.QueryType), nil)

	queryList := &browser.BrowseRequestList{}
	reqObj := browser.BrowseRequestObject{}
	reqObj.URI = "/configs/testgrp/v1/testobj/testname"
	queryList.RequestList = append(queryList.RequestList, reqObj)
	ctx, reti, _, skip, err = h.queryPreCallHook(ctx, queryList, nil)
	requestList := reti.(*browser.BrowseRequestList).GetRequestList()
	Assert(t, skip == false, "expecting false for skip", nil)
	AssertOk(t, err, "expecting to succeed got(%s)", err)
	Assert(t, requestList[0].URI == "/testgrp/testobj/testname", fmt.Sprintf("got wrong URI[%v]", requestList[0].URI), nil)

	nctx, _, err := h.addOperations(ctx, req)
	AssertOk(t, err, "expecting to succeed got(%s)", err)
	ops, ok := apigwpkg.OperationsFromContext(nctx)
	Assert(t, ok, "expecting to pass")
	Assert(t, len(ops) == 1, "expecting 1 op got [%v]", len(ops))

	nctx, _, err = h.addOperations(nctx, req)
	AssertOk(t, err, "expecting to succeed got(%s)", err)
	ops, ok = apigwpkg.OperationsFromContext(nctx)
	Assert(t, ok, "expecting to pass")
	Assert(t, len(ops) == 2, "expecting 2 ops got [%v]", len(ops))
}
