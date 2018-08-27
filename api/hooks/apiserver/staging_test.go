package impl

import (
	"context"
	"testing"

	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/generated/staging"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrv "github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/log"
)

func TestSetStagingBuffer(t *testing.T) {
	ctx := context.Background()

	nctx := SetStagingBufferInGrpcMD(ctx, "testBuffer")
	md, ok := metadata.FromIncomingContext(nctx)
	if !ok {
		t.Errorf("failed to get Md from context")
	}
	if bufids, ok := md[apiserver.RequestParamStagingBufferID]; ok && len(bufids) == 1 {
		if bufids[0] != "testBuffer" {
			t.Errorf("did not get the right value, got [%v]", bufids[0])
		}
	} else {
		t.Errorf("Could not find Buffer id in md")
	}

	md1 := metadata.Pairs(apisrv.RequestParamVersion, "v1",
		apisrv.RequestParamMethod, "POST")
	nctx = metadata.NewIncomingContext(context.Background(), md1)

	nctx = SetStagingBufferInGrpcMD(nctx, "testBuffer")
	md, ok = metadata.FromIncomingContext(nctx)
	if bufids, ok := md[apiserver.RequestParamStagingBufferID]; ok && len(bufids) == 1 {
		if bufids[0] != "testBuffer" {
			t.Errorf("did not get the right value, got [%v]", bufids[0])
		}
	} else {
		t.Errorf("Could not find Buffer id in md")
	}
	if _, ok := md[apisrv.RequestParamVersion]; !ok {
		t.Errorf("did not find version in metadata")
	}
	if _, ok := md[apisrv.RequestParamMethod]; !ok {
		t.Errorf("did not find method in metadata")
	}
}

func TestCreateDeleteBuffer(t *testing.T) {
	req := staging.Buffer{}
	req.Name = "testBuffer1"
	req.Tenant = "default"

	fcache := mocks.FakeCache{}
	apisrvpkg.SetAPIServerCache(&fcache)
	hooks := stagingHooks{l: log.GetNewLogger(log.GetDefaultConfig("hooksTest"))}
	hooks.createBuffer(context.TODO(), apiserver.CreateOper, req, false)

	ov, err := cache.GetOverlay(req.Tenant, req.Name)
	if err != nil || ov == nil {
		t.Fatalf("failed to retrieve overlay")
	}

	hooks.deleteBuffer(context.TODO(), apiserver.CreateOper, req, false)
	ov, err = cache.GetOverlay(req.Tenant, req.Name)
	if err == nil || ov != nil {
		t.Fatalf("got overlay after delete")
	}
}

func TestGetBuffer(t *testing.T) {
	req := staging.Buffer{}
	req.Name = "testBuffer1"
	req.Tenant = "default"

	fcache := mocks.FakeCache{}
	fov := mocks.FakeOverlay{}
	apisrvpkg.SetAPIServerCache(&fcache)
	cache.SetOverlay(req.Tenant, req.Name, &fov)
	hooks := stagingHooks{l: log.GetNewLogger(log.GetDefaultConfig("hooksTest"))}
	var reterr error
	fov.VerifyFunc = func(ctx context.Context) (apiintf.OverlayStatus, error) {
		if reterr != nil {
			return apiintf.OverlayStatus{}, reterr
		}
		return apiintf.OverlayStatus{
			Items: []apiintf.OverlayItem{
				{Key: apiintf.OverlayKey{URI: "/key1"}, Object: &req},
				{Key: apiintf.OverlayKey{URI: "/key2"}, Object: &req},
			},
			Failed: []apiintf.FailedVerification{
				{
					Key: apiintf.OverlayKey{URI: "/key1"},
				},
			},
		}, nil
	}

	ret, err := hooks.getBuffer(context.TODO(), nil, "", req, nil, req, apisrv.GetOper)
	if ret == nil || err != nil {
		t.Fatalf("failed to get response for get buffer")
	}
	if fov.Verifies != 1 {
		t.Fatalf("verify not called on Get()")
	}
	resp := ret.(staging.Buffer)
	if len(resp.Status.Items) != 2 || len(resp.Status.Errors) != 1 {
		t.Fatalf("returned getBuffer() result not as expected [%+v]", ret)

	}
}

func TestListBuffer(t *testing.T) {
	name1 := "testBuffer1"
	name2 := "testBuffer2"
	tenant := "default"

	fcache := mocks.FakeCache{}
	fov1 := mocks.FakeOverlay{}
	fov2 := mocks.FakeOverlay{}
	apisrvpkg.SetAPIServerCache(&fcache)
	cache.SetOverlay(tenant, name1, &fov1)
	cache.SetOverlay(tenant, name2, &fov2)
	hooks := stagingHooks{l: log.GetNewLogger(log.GetDefaultConfig("hooksTest"))}
	opts := api.ListWatchOptions{}
	resp := staging.BufferList{}
	resp.Items = []*staging.Buffer{
		{
			TypeMeta:   api.TypeMeta{Kind: "Buffer"},
			ObjectMeta: api.ObjectMeta{Name: name1, Tenant: tenant},
		},
		{
			TypeMeta:   api.TypeMeta{Kind: "Buffer"},
			ObjectMeta: api.ObjectMeta{Name: name2, Tenant: tenant},
		},
	}
	ret, err := hooks.listBuffer(context.TODO(), nil, "", opts, nil, resp, apisrv.ListOper)
	if ret == nil || err != nil {
		t.Fatalf("failed to get response for get buffer")
	}
	if fov1.Verifies != 1 {
		t.Fatalf("verify not called on Get() on overlay1")
	}
	if fov2.Verifies != 1 {
		t.Fatalf("verify not called on Get() on overlay2")
	}
}

func TestCommitAction(t *testing.T) {
	req := staging.CommitAction{}
	req.Name = "testBuffer1"
	req.Tenant = "default"

	fcache := mocks.FakeCache{}
	fov := mocks.FakeOverlay{}
	apisrvpkg.SetAPIServerCache(&fcache)
	cache.SetOverlay(req.Tenant, req.Name, &fov)
	hooks := stagingHooks{l: log.GetNewLogger(log.GetDefaultConfig("hooksTest"))}
	ret, skip, err := hooks.commitAction(context.TODO(), nil, nil, "key", apisrv.CreateOper, false, req)
	if ret == nil || err != nil {
		t.Fatalf("failed exec commitAction [%v](%s)", ret, err)
	}
	if skip != false {
		t.Fatalf("kvwrite enabled on commit")
	}
	if fov.Commits != 1 {
		t.Fatalf("Commit not called from commitAction()")
	}
}

func TestClearAction(t *testing.T) {
	req := staging.ClearAction{}
	req.Name = "testBuffer1"
	req.Tenant = "default"

	fcache := mocks.FakeCache{}
	fov := mocks.FakeOverlay{}
	apisrvpkg.SetAPIServerCache(&fcache)
	cache.SetOverlay(req.Tenant, req.Name, &fov)
	hooks := stagingHooks{l: log.GetNewLogger(log.GetDefaultConfig("hooksTest"))}
	ret, skip, err := hooks.clearAction(context.TODO(), nil, nil, "key", apisrv.CreateOper, false, req)
	if ret == nil || err != nil {
		t.Fatalf("failed exec commitAction [%v](%s)", ret, err)
	}
	if skip != false {
		t.Fatalf("kvwrite enabled on commit")
	}
	if fov.Clears != 1 {
		t.Fatalf("Commit not called from commitAction()")
	}
}
