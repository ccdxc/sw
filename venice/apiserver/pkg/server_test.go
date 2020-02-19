package apisrvpkg

import (
	"bytes"
	"context"
	"errors"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache"
	apiintf "github.com/pensando/sw/api/interfaces"
	apisrv "github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

var (
	logger = log.GetNewLogger(log.GetDefaultConfig("apiserver_test"))

	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("apiserver_test", logger))
)

type testAPISrvBackend struct {
	regevents int
}

func (t *testAPISrvBackend) CompleteRegistration(ctx context.Context,
	logger log.Logger, grpcserver *rpckit.RPCServer, scheme *runtime.Scheme) error {
	t.regevents = t.regevents + 1
	return nil
}

func (t *testAPISrvBackend) Reset() {
}

type testAPISrvService struct {
	hookcbCalled int
}

func (t *testAPISrvService) hooksCb(srv apisrv.Service, logger log.Logger) {
	t.hookcbCalled++
}

func (t *testAPISrvService) Enable()                          {}
func (t *testAPISrvService) Disable()                         {}
func (t *testAPISrvService) GetMethod(n string) apisrv.Method { return nil }
func (t *testAPISrvService) GetCrudService(in string, oper apiintf.APIOperType) apisrv.Method {
	return nil
}
func (t *testAPISrvService) AddMethod(n string, m apisrv.Method) apisrv.Method { return nil }
func (t *testAPISrvService) WatchFromKv(options *api.ListWatchOptions, stream grpc.ServerStream, svcprefix string) error {
	return nil
}
func (t *testAPISrvService) WithKvWatchFunc(fn apisrv.WatchSvcKvFunc) apisrv.Service { return t }
func (t *testAPISrvService) WithCrudServices(msgs []apisrv.Message) apisrv.Service   { return t }
func (t *testAPISrvService) Name() string                                            { return "" }

func TestRegistration(t *testing.T) {
	// Parallel is not needed since init happens in a single thread.
	// Make sure to initialize the singleton.
	_ = MustGetAPIServer()
	a := &singletonAPISrv
	s := testAPISrvBackend{}
	s1 := testAPISrvBackend{}
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

	srv1 := testAPISrvService{}
	a.RegisterService("test-service", &srv1)
	s2 := a.GetService("test-service")
	if s2 == nil || s2 != &srv1 {
		t.Errorf("could not retrieve registered service")
	}

	m1 := mocks.NewFakeMessage("test.TestType1", "", true)
	m2 := mocks.NewFakeMessage("test.TestType2", "", true)
	msgs := make(map[string]apisrv.Message)
	msgs["test-service.msg1"] = m1
	msgs["test-service.msg2"] = m2
	a.RegisterMessages("test-service", msgs)
	if len(a.messages) != 2 {
		t.Errorf("incorrect number of messages expected[2] found [%d]", len(a.messages))
	}
	m3 := a.GetMessage("test-service", "msg1")
	if m3 == nil {
		t.Errorf("could not get registered message")
	}
	if m3 != m1 {
		t.Errorf("did not get the right message")
	}
}

func TestDupRegistration(t *testing.T) {
	// Make sure to initialize the singleton.
	_ = MustGetAPIServer()
	a := &singletonAPISrv
	s := testAPISrvBackend{}
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
	_ = MustGetAPIServer()
	a := &singletonAPISrv
	s := testAPISrvBackend{}

	// This is allowed and should not panic
	a.Register("duppath-test1", &s)
	a.Register("duppath-test2", &s)
}

// TestInitOnce
// Tests that multiple MustGetAPIServer initializes the singleton only once.
func TestInitOnce(t *testing.T) {
	srv := MustGetAPIServer().(*apiSrv)
	s := testAPISrvBackend{}

	singletonAPISrv.svcmap["Testsvc"] = &s

	srv1 := MustGetAPIServer().(*apiSrv)
	if _, ok := srv1.svcmap["Testsvc"]; !ok {
		t.Errorf("Did not find service in new API server")
	}
	delete(srv.svcmap, "Testsvc")
}

// TestRunApiSrv
// Test the Run function of API Server to ensure it exits on failure.
func TestRunApiSrv(t *testing.T) {
	buf := &bytes.Buffer{}

	logConfig := log.GetDefaultConfig("TestApiServer")
	l := log.GetNewLogger(logConfig).SetOutput(buf)
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
		KVPoolSize:       1,
		AllowMultiTenant: true,
	}

	_ = MustGetAPIServer()
	a := &singletonAPISrv
	a.runstate.running = false
	s1 := testAPISrvService{}
	a.RegisterService("test-service1", &s1)
	a.RegisterService("test-service2", &s1)
	a.RegisterHooksCb("test-service1", s1.hooksCb)
	a.RegisterHooksCb("test-service2", s1.hooksCb)
	// Add a dummy service hook without the service.
	a.RegisterHooksCb("dummy-service2", s1.hooksCb)
	if len(a.hookregs) != 3 {
		t.Errorf("Was expecting [3] hooks found [%d]", len(a.hookregs))
	}
	go a.Run(config)
	a.WaitRunning()
	_, err := a.GetAddr()
	if err != nil {
		t.Fatalf("failed to get API gateway address")
	}
	if a.kvPoolsize != len(a.kvPool) {
		t.Errorf("kv pool size want[%d] got [%d]", len(a.kvPool), a.kvPoolsize)
	}
	// Try again
	doneCh := make(chan bool)
	go func() {
		a.WaitRunning()
		_, err = a.GetAddr()
		if err != nil {
			t.Fatalf("failed to get API gateway address")
		}
		doneCh <- true
	}()
	select {
	case <-doneCh:
		// Good
	case <-time.After(100 * time.Millisecond):
		t.Fatal("Timeout waiting on lock")
	}

	// test if the hooks are called; only the hooks that're asssociated with the service is invoked.
	if s1.hookcbCalled != 2 {
		t.Errorf("Was expecting [2] hooks invocation found [%d]", s1.hookcbCalled)
	}

	ver := a.GetVersion()
	if ver != "v1" {
		t.Errorf("returned wrong version [%v]", ver)
	}

	flags := a.RuntimeFlags()
	if !flags.AllowMultiTenant {
		t.Errorf("Expecting AllowMultiTenant to be set")
	}

	rslvr := a.GetResolvers()
	if len(rslvr) != 0 {
		t.Errorf("expecting to receive 0 entries")
	}
	a.rslvrURIs = []string{"node1", "node2"}
	rslvr = a.GetResolvers()
	if len(rslvr) != 2 {
		t.Errorf("expecting to receive valid resolvers")
	}

	// Check we are able to create overlays
	_, err = a.CreateOverlay("default", "notused", "/test/")
	if err != nil {
		t.Errorf("error creating overlay (%s)", err)
	}
	ov, err := cache.GetOverlay("default", "notused")
	if err != nil {
		t.Errorf("error retrieving overlay (%s)", err)
	}
	if ov == nil {
		t.Errorf("overlay is nil")
	}
	c := a.getKvConn()
	if c == nil {
		t.Fatalf("Connection is nil")
	}
	// Add a few more connections
	err = a.addKvConnToPool()
	if err != nil {
		t.Errorf("Got error adding connection to pool")
	}

	err = errors.New("Testing Exit for Api Server")
	a.Stop()
	time.Sleep(100 * time.Millisecond)
	if !strings.Contains(buf.String(), "Stop called by user") {
		t.Errorf("APIServer Run did not close on error")
	}
	if len(singletonAPISrv.kvPool) != 0 {
		t.Fatalf("expecing no kv connections in pool got %d", len(singletonAPISrv.kvPool))
	}
	if a.kvPoolsize != 0 {
		t.Fatalf("expecting pool size to 0, got %d", a.kvPoolsize)
	}
	// Replenish the pool before exiting
	t.Logf("replenish kv pool %d \n", config.KVPoolSize)
	for i := 0; i < singletonAPISrv.config.KVPoolSize; i++ {
		singletonAPISrv.addKvConnToPool()
	}
	config.AllowMultiTenant = false
	go a.Run(config)
	a.WaitRunning()
	flags = a.RuntimeFlags()
	if flags.AllowMultiTenant {
		t.Errorf("Expecting AllowMultiTenant to be false")
	}
	a.Stop()
}

func TestStartStop(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestApiServer")
	l := log.GetNewLogger(logConfig)
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
		KVPoolSize:       1,
		AllowMultiTenant: true,
	}

	_ = MustGetAPIServer()
	a := &singletonAPISrv
	a.runstate.running = false

	iterCount := 200
	var runCount uint32
	var wg sync.WaitGroup
	wg.Add(2)
	go func() {
		for i := 0; i < iterCount; i++ {
			go a.Run(config)
			atomic.AddUint32(&runCount, 1)
			a.WaitRunning()
			a.Stop()
		}
		a.Run(config)
		wg.Done()
	}()
	go func() {
		for i := 0; i < iterCount; i++ {
			go a.Run(config)
			atomic.AddUint32(&runCount, 1)
			a.WaitRunning()
			a.Stop()
		}
		a.Run(config)
		wg.Done()
	}()

	// Wait for the iterations to complete. One of the go-routines will be left running,
	//  stop that instance and wait for its exit.
	var readCount uint32
	for {
		readCount = atomic.LoadUint32(&runCount)
		if readCount == uint32(iterCount*2) {
			// allow some time for the the go routine to call WaitRunning()
			time.Sleep(time.Second)
			a.Stop()
			break
		}
		time.Sleep(10 * time.Millisecond)
	}
	wg.Wait()
	// Replenish the pool before exiting
	for i := 0; i < singletonAPISrv.config.KVPoolSize; i++ {
		singletonAPISrv.addKvConnToPool()
	}
}
