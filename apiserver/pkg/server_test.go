package apisrvpkg

import (
	"bytes"
	"context"
	"errors"
	"strings"
	"testing"
	"time"

	"google.golang.org/grpc"

	apisrv "github.com/pensando/sw/apiserver"
	"github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/runtime"
)

type testApiSrvBackend struct {
	regevents int
}

func (t *testApiSrvBackend) CompleteRegistration(ctx context.Context,
	logger log.Logger, grpcserver *grpc.Server, scheme *runtime.Scheme) error {
	t.regevents = t.regevents + 1
	return nil
}

type testApiSrvService struct {
	hookcbCalled int
}

func (t *testApiSrvService) hooksCb(srv apisrv.Service, logger log.Logger) {
	t.hookcbCalled++
}

func (t *testApiSrvService) Enable()                                           {}
func (t *testApiSrvService) Disable()                                          {}
func (t *testApiSrvService) GetMethod(n string) apisrv.Method                  { return nil }
func (t *testApiSrvService) AddMethod(n string, m apisrv.Method) apisrv.Method { return nil }

func TestRegistration(t *testing.T) {
	// Parallel is not needed since init happens in a single thread.
	// Make sure to initialize the singleton.
	_ = MustGetApiServer()
	a := singletonApiSrv
	s := testApiSrvBackend{}
	s1 := testApiSrvBackend{}
	a.Register("register-test1", &s)
	a.Register("register-test2", &s1)
	if len(a.svcmap) != 2 {
		t.Errorf("Mismatched number of registered services Want 2 found [%v]", len(a.svcmap))
	}

	for name := range a.svcmap {
		if name != "register-test1" && name != "register-test2" {
			t.Errorf("invalid name %s", name)
		}
	}

	srv1 := testApiSrvService{}
	a.RegisterService("test-service", &srv1)
	s2 := a.GetService("test-service")
	if s2 == nil || s2 != &srv1 {
		t.Errorf("could not retrieve registered service")
	}

	m1 := newFakeMessage("", true)
	m2 := newFakeMessage("", true)
	msgs := make(map[string]apisrv.Message)
	msgs["msg1"] = m1
	msgs["msg2"] = m2
	a.RegisterMessages("test-service", msgs)
	if len(a.messages) != 2 {
		t.Errorf("incorrect number of messages expected[2] found [%d]", len(a.messages))
	}

}

func TestDupRegistration(t *testing.T) {
	// Make sure to initialize the singleton.
	_ = MustGetApiServer()
	a := singletonApiSrv
	s := testApiSrvBackend{}
	defer func() {
		if r := recover(); r == nil {
			t.Errorf("Expecting panic but did not")
		}
	}()
	a.Register("dup-test1", &s)
	a.Register("dup-test1", &s)
}

func TestDupPathRegistration(t *testing.T) {
	// Make sure to initialize the singleton.
	_ = MustGetApiServer()
	a := singletonApiSrv
	s := testApiSrvBackend{}

	// This is allowed and should not panic
	a.Register("duppath-test1", &s)
	a.Register("duppath-test2", &s)
}

// TestInitOnce
// Tests that multiple MustGetApiServer initializes the singleton only once.
func TestInitOnce(t *testing.T) {
	srv := MustGetApiServer().(*apiSrv)
	s := testApiSrvBackend{}

	singletonApiSrv.svcmap["Testsvc"] = &s

	srv1 := MustGetApiServer().(*apiSrv)
	if _, ok := srv1.svcmap["Testsvc"]; !ok {
		t.Errorf("Did not find service in new API server")
	}
	delete(srv.svcmap, "Testsvc")
}

// TestRunApiSrv
// Test the Run function of API Server to ensure it exits on failure.
func TestRunApiSrv(t *testing.T) {
	buf := &bytes.Buffer{}
	l := log.GetNewLogger(false).SetOutput(buf)
	config := apisrv.Config{
		GrpcServerPort: ":0",
		DebugMode:      true,
		Logger:         l,
		Version:        "v1",
		Scheme:         runtime.NewScheme(),
		Kvstore: store.Config{
			Type:  store.KVStoreTypeMemkv,
			Codec: runtime.NewJSONCodec(runtime.NewScheme()),
		},
	}

	_ = MustGetApiServer()
	a := singletonApiSrv
	s1 := testApiSrvService{}
	a.RegisterService("test-service1", &s1)
	a.RegisterService("test-service2", &s1)
	a.RegisterHooksCb("test-service1", s1.hooksCb)
	a.RegisterHooksCb("test-service2", s1.hooksCb)
	// Add a dummy service hook without the service.
	a.RegisterHooksCb("dumm-service2", s1.hooksCb)
	if len(a.hookregs) != 3 {
		t.Errorf("Was expecting [2] hooks found [%d]", len(a.hookregs))
	}
	go a.Run(config)
	err := errors.New("Testing Exit for Api Server")
	a.doneCh <- err
	time.Sleep(100 * time.Millisecond)
	if !strings.Contains(buf.String(), "Testing Exit for Api Server") {
		t.Errorf("APIServer Run did not close on error")
	}
	if s1.hookcbCalled != 2 {
		t.Errorf("Was expecting [2] hooks invocation found [%d]", s1.hookcbCalled)
	}
}
