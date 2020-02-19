package apisrvpkg

import (
	"context"
	"reflect"
	"testing"

	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// TestMethodKvWrite
// Test registering of methods to service and retrieval
func TestSvcAddMethod(t *testing.T) {
	svc := NewService("testSvc").(*ServiceHdlr)
	svc.AddMethod("Method1", mocks.NewFakeMethod(true))
	svc.AddMethod("Method2", mocks.NewFakeMethod(true))
	svc.AddMethod("AutoWatchMethod2", mocks.NewFakeMethod(true))

	if len(svc.Methods) != 3 {
		t.Errorf("Expecting [3] methods found [%v]", len(svc.Methods))
	}

	if svc.GetMethod("Method1") == nil {
		t.Errorf("Get method [Method1] failed")
	}
	if svc.GetMethod("Method2") == nil {
		t.Errorf("Get method [Method2] failed")
	}
	if svc.GetCrudService("Method2", apiintf.WatchOper) == nil {
		t.Errorf("Get method [Method2] failed")
	}

	m1 := svc.GetMethod("Method1").(*mocks.FakeMethod)
	svc.Disable()
	if m1.Enabled == true {
		t.Errorf("method should be Disabled")
	}
	svc.Enable()
	if m1.Enabled == false {
		t.Errorf("method should be Enabled")
	}
}
func TestGetCrudServiceName(t *testing.T) {
	// Test different options of GetCrudService
	for _, c := range []struct {
		oper   apiintf.APIOperType
		output string
	}{
		{oper: apiintf.CreateOper, output: "AutoAddTestMethod"},
		{oper: apiintf.UpdateOper, output: "AutoUpdateTestMethod"},
		{oper: apiintf.GetOper, output: "AutoGetTestMethod"},
		{oper: apiintf.DeleteOper, output: "AutoDeleteTestMethod"},
		{oper: apiintf.ListOper, output: "AutoListTestMethod"},
		{oper: apiintf.WatchOper, output: "AutoWatchTestMethod"},
		{oper: "junk", output: ""},
	} {
		o := apiserver.GetCrudServiceName("TestMethod", c.oper)
		if c.output != o {
			t.Errorf("Expected [%s] got [%s]", c.output, o)
		}
	}
}

type dummyStream struct {
	ctx context.Context
	err error
}

func (s dummyStream) Context() context.Context     { return s.ctx }
func (s dummyStream) SendMsg(m interface{}) error  { return s.err }
func (s dummyStream) RecvMsg(m interface{}) error  { return s.err }
func (s dummyStream) SetHeader(metadata.MD) error  { return s.err }
func (s dummyStream) SendHeader(metadata.MD) error { return s.err }
func (s dummyStream) SetTrailer(metadata.MD)       {}

func TestSvcWatch(t *testing.T) {
	svc := NewService("testSvc").(*ServiceHdlr)
	ver := "v1"
	msgs := []apiserver.Message{mocks.NewFakeMessage("test.obj1", "/obj1", false), mocks.NewFakeMessage("test.obj2", "/obj2", false)}
	svc.WithCrudServices(msgs)
	if len(svc.prepMsgMap) != 2 || len(svc.crudMsgMap) != 2 {
		t.Errorf("prepMap got %d want 2, crudMsgMap got %d want 2", len(svc.prepMsgMap), len(svc.crudMsgMap))
	}
	var pass bool
	var called int
	var err error
	watchFn := func(l log.Logger, options *api.ListWatchOptions, kvs kvstore.Interface, stream interface{}, txfnMap map[string]func(from, to string, i interface{}) (interface{}, error), version, svcprefix string) error {
		pass = reflect.DeepEqual(txfnMap, svc.prepMsgMap)
		if !pass {
			t.Errorf("Maps does not match got [%+v] want [%+v]", txfnMap, svc.prepMsgMap)
		}
		pass = pass && (version == ver)
		if !pass {
			t.Errorf("got wrong version got [%v] want [%v]", version, ver)
		}
		called++
		return err
	}
	_ = MustGetAPIServer()
	a := &singletonAPISrv
	a.runstate.running = true
	a.kvPool = append(a.kvPool, nil)

	stream := &dummyStream{ctx: context.TODO(), err: nil}

	err1 := svc.WatchFromKv(nil, stream, "test")
	if err1 == nil {
		t.Errorf("Expecting Watch go fail")
	}
	svc.WithKvWatchFunc(watchFn)
	err1 = svc.WatchFromKv(nil, stream, "test")
	if err1 == nil {
		t.Errorf("Expecting Watch to fail")
	}
	md := metadata.Pairs(apiserver.RequestParamVersion, ver, apiserver.RequestParamMethod, "WATCH")
	ctx := metadata.NewIncomingContext(context.Background(), md)
	stream.ctx = ctx
	err1 = svc.WatchFromKv(nil, stream, "test")
	if err1 != nil {
		t.Errorf("Expecting Watch to pass got (%s)", err1)
	}
	if !pass {
		t.Errorf("did not get right arguments to Watch function")
	}
	if called != 1 {
		t.Errorf("not called correct number of times exp 1 got %d ", called)
	}
}
