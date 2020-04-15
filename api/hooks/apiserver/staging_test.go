package impl

import (
	"context"

	"testing"

	"google.golang.org/grpc/metadata"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/bulkedit"
	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/staging"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrv "github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	mocks2 "github.com/pensando/sw/venice/apiserver/pkg/mocks"
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
	hooks.createBuffer(context.TODO(), apiintf.CreateOper, req, false)

	ov, err := cache.GetOverlay(req.Tenant, req.Name)
	if err != nil || ov == nil {
		t.Fatalf("failed to retrieve overlay")
	}

	hooks.deleteBuffer(context.TODO(), apiintf.CreateOper, req, false)
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

	ret, err := hooks.getBuffer(context.TODO(), nil, "", req, nil, req, apiintf.GetOper)
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
	ret, err := hooks.listBuffer(context.TODO(), nil, "", opts, nil, resp, apiintf.ListOper)
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
	ret, skip, err := hooks.commitAction(context.TODO(), nil, nil, "key", apiintf.CreateOper, false, req)
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
	ret, skip, err := hooks.clearAction(context.TODO(), nil, nil, "key", apiintf.CreateOper, false, req)
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

func TestStagingRegistration(t *testing.T) {
	srv := mocks2.NewFakeService()
	meth := mocks2.NewFakeMethod(true)
	srv.AddMethod("Buffer", meth)
	srv.AddMethod("Commit", meth)
	srv.AddMethod("Clear", meth)
	srv.AddMethod("Bulkedit", meth)
	logger := log.GetDefaultInstance()
	registerStagingHooks(srv, logger)
	fmeth := meth.(*mocks2.FakeMethod)
	if fmeth.Pres != 3 || fmeth.Posts != 2 || fmeth.RWriters != 2 {
		t.Fatalf("unexpected number of hooks registered [%+v]", fmeth)
	}
}

func TestBulkeditAction(t *testing.T) {

	fcache := mocks.FakeCache{}
	fov := mocks.FakeOverlay{}
	apisrvpkg.SetAPIServerCache(&fcache)

	bufName := "TestBuffer1"

	netw := &network.Network{
		TypeMeta: api.TypeMeta{
			Kind:       "Network",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "TestNtwork1",
			Tenant: "default",
		},
		Spec: network.NetworkSpec{
			Type:        network.NetworkType_Bridged.String(),
			IPv4Subnet:  "10.1.1.1/24",
			IPv4Gateway: "10.1.1.1",
		},
		Status: network.NetworkStatus{},
	}

	n1, err := types.MarshalAny(netw)

	netw.Spec.IPv4Gateway = "110.1.1.1"
	netw.Spec.IPv4Subnet = "110.1.1.1/24"

	n2, err := types.MarshalAny(netw)

	cache.SetOverlay("default", bufName, &fov)

	netw.Name = "testDelNetw"
	netw.Spec.IPv4Gateway = "120.2.1.1"
	netw.Spec.IPv4Subnet = "120.2.1.1/24"
	n3, err := types.MarshalAny(netw)

	req := staging.BulkEditAction{
		TypeMeta: api.TypeMeta{
			Kind:       "BulkEditAction",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   bufName,
			Tenant: "default",
		},
		Spec: bulkedit.BulkEditActionSpec{
			Items: []*bulkedit.BulkEditItem{
				&bulkedit.BulkEditItem{
					URI:    "/configs/network/v1/tenant/default/networks/TestNtwork1",
					Method: "create",
					Object: &api.Any{Any: *n1},
				},
				&bulkedit.BulkEditItem{
					URI:    "/configs/network/v1/tenant/default/networks/TestNtwork1",
					Method: "update",
					Object: &api.Any{Any: *n2},
				},
				&bulkedit.BulkEditItem{
					URI:    "/configs/network/v1/tenant/default/networks/testDelNetw",
					Method: "delete",
					Object: &api.Any{Any: *n3},
				},
			},
		},
	}

	hooks := stagingHooks{l: log.GetNewLogger(log.GetDefaultConfig("hooksTest"))}
	ret, skip, err := hooks.bulkeditAction(context.TODO(), nil, nil, "key", apiintf.CreateOper, false, req)
	if ret == nil || err != nil {
		t.Fatalf("failed exec commitAction [%v](%s)", ret, err)
	}
	if skip != false {
		t.Fatalf("kvwrite enabled on commit")
	}

	retBuf := ret.(staging.BulkEditAction)
	if retBuf.Status.ValidationResult != staging.BufferStatus_SUCCESS.String() {
		t.Fatalf("Expected SUCCESS Validation status")
	}

	if fov.CreatePrimaries != 1 {
		t.Fatalf("Number of createprimary calls %d did not mtch expected\n", fov.CreatePrimaries)
	}
	if fov.UpdatePrimaries != 1 {
		t.Fatalf("Number of updateprimary calls %d did not mtch expected\n", fov.UpdatePrimaries)
	}
	if fov.DeletePrimaries != 1 {
		t.Fatalf("Number of deleteprimary calls %d did not mtch expected\n", fov.DeletePrimaries)
	}

	// Negative test cases
	// 1. Invalid bufferName

	req = staging.BulkEditAction{
		TypeMeta: api.TypeMeta{
			Kind:       "BulkEditAction",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "NewStagingBuffer",
			Tenant: "default",
		},
		Spec: bulkedit.BulkEditActionSpec{
			Items: []*bulkedit.BulkEditItem{
				&bulkedit.BulkEditItem{
					URI:    "/configs/network/v1/tenant/default/networks/TestNtwork1",
					Method: "create",
					Object: &api.Any{Any: *n1},
				},
			},
		},
	}

	ret, _, err = hooks.bulkeditAction(context.TODO(), nil, nil, "key", apiintf.CreateOper, false, req)
	if err == nil {
		t.Fatalf("Expected Unknwon Buffer Error!\n")
	}

	retBuf = ret.(staging.BulkEditAction)
	if retBuf.Status.ValidationResult != staging.BufferStatus_FAILED.String() {
		t.Fatalf("Expected FAILED Validation status, got %s\n", retBuf.Status.ValidationResult)
	}

	// 2. Send some junk type as input
	req1 := staging.BufferStatus{}
	_, _, err = hooks.bulkeditAction(context.TODO(), nil, nil, "key", apiintf.CreateOper, false, req1)
	if err == nil {
		t.Fatalf("Expected Invalid object type Error!\n")
	}

	// 3. Unknown Method type
	req = staging.BulkEditAction{
		TypeMeta: api.TypeMeta{
			Kind:       "BulkEditAction",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   bufName,
			Tenant: "default",
		},
		Spec: bulkedit.BulkEditActionSpec{
			Items: []*bulkedit.BulkEditItem{
				&bulkedit.BulkEditItem{
					URI:    "/configs/network/v1/tenant/default/networks/TestNtwork1",
					Method: "asdfghjkl0987654321",
					Object: &api.Any{Any: *n1},
				},
			},
		},
	}

	_, _, err = hooks.bulkeditAction(context.TODO(), nil, nil, "key", apiintf.CreateOper, false, req)
	if err == nil {
		t.Fatalf("Expected Invalid method type Error!\n")
	}

	retBuf = ret.(staging.BulkEditAction)
	if retBuf.Status.ValidationResult != staging.BufferStatus_FAILED.String() {
		t.Fatalf("Expected FAILED Validation status")
	}
}
