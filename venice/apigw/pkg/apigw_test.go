package apigwpkg

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"io"
	"net/http"
	"net/http/httptest"
	"reflect"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/pensando/grpc-gateway/runtime"
	gwruntime "github.com/pensando/grpc-gateway/runtime"
	"google.golang.org/grpc"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/authn/manager"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var hooksCalled int

type testGwService struct {
	regevents int
}

func (t *testGwService) CompleteRegistration(ctx context.Context,
	logger log.Logger, grpcserver *grpc.Server, mux *http.ServeMux, rslvr resolver.Interface, wg *sync.WaitGroup) error {
	t.regevents = t.regevents + 1
	return nil
}

func (t *testGwService) GetServiceProfile(method string) (apigw.ServiceProfile, error) {
	return nil, nil
}

func (t *testGwService) GetCrudServiceProfile(object string, oper apiserver.APIOperType) (apigw.ServiceProfile, error) {
	return nil, nil
}

// TestInitOnce
// Tests that multiple MustGetAPiGateway initializes the singleton only once.
func TestInitOnce(t *testing.T) {
	gw := MustGetAPIGateway().(*apiGw)
	s := testGwService{}

	gw.svcmap["Testsvc"] = &s

	gw1 := MustGetAPIGateway().(*apiGw)
	if _, ok := gw1.svcmap["Testsvc"]; !ok {
		t.Errorf("Did not find service in new Gateway")
	}
	delete(gw.svcmap, "Testsvc")
}

// TestRegistration
// Tests the registration sequesce
func TestRegistration(t *testing.T) {
	// Parallel is not needed since init happens in a single thread.
	// Make sure to initialize the singleton.
	_ = MustGetAPIGateway()
	a := singletonAPIGw
	s := testGwService{}
	s1 := testGwService{}
	a.Register("register-test1", "/test1", &s)
	a.Register("register-test2", "/test2", &s1)
	hookscb := func(svc apigw.APIGatewayService, l log.Logger) error {
		hooksCalled++
		return nil
	}
	a.RegisterHooksCb("register-test1", hookscb)
	if len(a.svcmap) != 2 {
		t.Errorf("Mismatched number of registered services Want 2 found [%v]", len(a.svcmap))
	}

	for name := range a.svcmap {
		if name != "register-test1" && name != "register-test2" {
			t.Errorf("invalid name %s", name)
		}
	}

	if len(a.hooks) != 1 {
		t.Errorf("hook not registered")
	}

	s2 := a.GetService("register-test1")
	if s2 == nil || s2 != &s {
		t.Errorf("could not find registered service [register-test1]")
	}
	s2 = a.GetService("register-test2")
	if s2 == nil || s2 != &s1 {
		t.Errorf("could not find registered service  [register-test2]")
	}
	s2 = a.GetService("Dummy-Service")
	if s2 != nil {
		t.Errorf("found unregistered service  [Dummy-Service]")
	}
}

func dummyHandler(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusOK)
	io.WriteString(w, `Was Called`)
}

// TestPreflightHandler
// Test the preflight handler functionality
func TestPreflightHandler(t *testing.T) {
	_ = MustGetAPIGateway()
	a := singletonAPIGw
	buf := &bytes.Buffer{}

	logConfig := log.GetDefaultConfig("TestApiGw")
	l := log.GetNewLogger(logConfig).SetOutput(buf)
	a.logger = l
	req, err := http.NewRequest("OPTIONS", "http://127.0.0.1/test/a", nil)
	if err != nil {
		t.Fatal("could not create request")
	}
	req.Header.Set("Origin", "test.com")
	req.Header.Set("Access-Control-Request-Method", "GET")
	resp := httptest.NewRecorder()
	dummyfn := http.HandlerFunc(dummyHandler)
	handler := a.checkCORS(dummyfn)
	handler.ServeHTTP(resp, req)

	if resp.Code != http.StatusOK {
		t.Errorf("Wrong Status")
	}
	hdr := resp.Header()
	// Check that all the right headers have been set.
	if v, ok := hdr["Access-Control-Allow-Headers"]; ok {
		found := false
		for _, h := range v {
			if h == "Content-Type,Accept" {
				found = true
				break
			}
		}

		if !found {
			t.Errorf("Content-Type not set [%v][%s]", ok, v)
		}
	}

	// allowed methods should be [GET, OPTIONS]
	if v, ok := hdr["Access-Control-Allow-Methods"]; ok {
		for _, h := range v {
			if !strings.Contains(h, "GET") || !strings.Contains(h, "OPTIONS") {
				t.Errorf("Allow Methods not set [%v]", h)
			}
		}
	}
}

func TestCheckCORS(t *testing.T) {
	_ = MustGetAPIGateway()
	a := singletonAPIGw

	logConfig := log.GetDefaultConfig("TestApiGw")
	l := log.GetNewLogger(logConfig)
	a.logger = l
	tests := []struct {
		name     string
		method   string
		origin   string
		host     string
		expected int
	}{
		{
			name:     "cross origin GET request",
			method:   "GET",
			origin:   "http://test.com",
			host:     "127.0.0.1",
			expected: http.StatusOK,
		},
		{
			name:     "cross origin POST request",
			method:   "POST",
			origin:   "http://test.com",
			host:     "127.0.0.1",
			expected: http.StatusForbidden,
		},
		{
			name:     "same origin POST request",
			method:   "POST",
			origin:   "http://127.0.0.1",
			host:     "127.0.0.1",
			expected: http.StatusOK,
		},
	}
	for _, test := range tests {
		req, err := http.NewRequest(test.method, "http://127.0.0.1/test/a", nil)
		if err != nil {
			t.Fatal("could not create request")
		}
		req.Header.Set("Origin", test.origin)
		req.Header.Set("Host", test.host)
		resp := httptest.NewRecorder()
		dummyfn := http.HandlerFunc(dummyHandler)
		handler := a.checkCORS(dummyfn)
		handler.ServeHTTP(resp, req)
		Assert(t, resp.Code == test.expected, fmt.Sprintf("[%v] test failed", test.name))
	}
}

// TestExtractHdrInfo
// Test Extracting of the header information.
func TestExtractHdrInfo(t *testing.T) {
	_ = MustGetAPIGateway()
	a := singletonAPIGw
	buf := &bytes.Buffer{}

	logConfig := log.GetDefaultConfig("TestApiGw")
	l := log.GetNewLogger(logConfig).SetOutput(buf)
	a.logger = l
	req, err := http.NewRequest("POST", "http://127.0.0.1/v1/TestSvc/Resource", nil)
	if err != nil {
		t.Fatal("could not create request")
	}
	resp := httptest.NewRecorder()
	hfunc := http.HandlerFunc(dummyHandler)
	handler := a.extractHdrInfo(&hfunc)
	handler.ServeHTTP(resp, req)

	if resp.Code != http.StatusOK {
		t.Errorf("Wrong Status")
	}
	hdr := req.Header
	// Check that all the right headers have been set.
	if v, ok := hdr[apigw.GrpcMDRequestVersion]; !ok || v[0] != "v1" {
		t.Errorf("Version is not right. expecting [v1] got [%v][%s]", ok, v)

	}
	if v, ok := hdr[apigw.GrpcMDRequestURI]; !ok || v[0] != "/TestSvc/Resource" {
		t.Errorf("Error recovering URI. expecting [/TestSvc/Resource] got [%v][%s]", ok, v)

	}
	if v, ok := hdr[apigw.GrpcMDRequestMethod]; !ok || v[0] != "POST" {
		t.Errorf("Error recovering method. expecting [POST] got [%v][%s]", ok, v)

	}
}

// TestDupRegistration
// Tests the duplicate registration path for the API Gateway.
// Expectation is to panic.
func TestDupRegistration(t *testing.T) {
	// Make sure to initialize the singleton.
	_ = MustGetAPIGateway()
	a := singletonAPIGw
	s := testGwService{}
	defer func() {
		if r := recover(); r == nil {
			t.Errorf("Expecting panic but did not")
		}
	}()
	a.Register("dup-test1", "/test1", &s)
	a.Register("dup-test1", "/test2", &s)
}

// TestDupPathRegistration
// Tests duplicate registrations with same path which is allowed.
func TestDupPathRegistration(t *testing.T) {
	// Make sure to initialize the singleton.
	_ = MustGetAPIGateway()
	a := singletonAPIGw
	s := testGwService{}

	// This is allowed and should not panic
	a.Register("duppath-test1", "/test1", &s)
	a.Register("duppath-test2", "/test1", &s)
}

// TestRunApiGw
// Test the Run function of API Gw to ensure it exits on failure.
func TestRunApiGw(t *testing.T) {
	buf := &bytes.Buffer{}
	logConfig := log.GetDefaultConfig("TestApiGw")
	l := log.GetNewLogger(logConfig).SetOutput(buf)
	config := apigw.Config{
		HTTPAddr:  ":0",
		DebugMode: true,
		Logger:    l,
		Resolvers: []string{""},
	}
	_ = MustGetAPIGateway()
	a := singletonAPIGw
	hooks := hooksCalled
	go a.Run(config)
	err := errors.New("Testing Exit for ApiGateway")
	a.doneCh <- err
	time.Sleep(100 * time.Millisecond)
	if !strings.Contains(buf.String(), "Testing Exit for ApiGateway") {
		t.Errorf("APiGateway Run did not close on error")
	}
	a.WaitRunning()
	_, err = a.GetAddr()
	if err != nil {
		t.Fatalf("failed to get API gateway address")
	}
	if hooksCalled != hooks+len(a.hooks) {
		t.Errorf("hooks not called")
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
}

func TestErrorHandlers(t *testing.T) {
	marshaller := &gwruntime.JSONBuiltin{}
	ctx := context.Background()
	respWr := httptest.NewRecorder()
	err := errors.New("test error")
	gw := MustGetAPIGateway().(*apiGw)
	gw.HTTPErrorHandler(ctx, marshaller, respWr, nil, err)
	if respWr.Header().Get("Content-Type") != "application/json" {
		t.Errorf("wrong header")
	}
	gw.HTTPOtherErrorHandler(respWr, nil, "test Message", 404)
	if respWr.Header().Get("Content-Type") != "application/json" {
		t.Errorf("wrong header")
	}
}

func TestHandleRequest(t *testing.T) {
	prof := NewServiceProfile(nil)
	mock := &testHooks{}
	prof.AddPreAuthNHook(mock.preAuthNHook)
	prof.AddPreAuthNHook(mock.preAuthNHook)
	prof.AddPreAuthNHook(mock.preAuthNHook)
	prof.AddPreAuthZHook(mock.preAuthZHook)
	prof.AddPreAuthZHook(mock.preAuthZHook)
	prof.AddPreAuthZHook(mock.preAuthZHook)
	prof.AddPreAuthZHook(mock.preAuthZHook)
	prof.AddPreCallHook(mock.preCallHook)
	prof.AddPreCallHook(mock.preCallHook)
	prof.AddPostCallHook(mock.postCallHook)
	called := 0
	input := struct {
		Test string
	}{"testing"}
	call := func(ctx context.Context, in interface{}) (interface{}, error) {
		called++
		return in, nil
	}
	mock.retObj = &input
	_ = MustGetAPIGateway()

	a := singletonAPIGw
	buf := &bytes.Buffer{}
	logConfig := log.GetDefaultConfig("TestApiGw")
	l := log.GetNewLogger(logConfig).SetOutput(buf)
	a.logger = l
	a.authnMgr = manager.NewMockAuthenticationManager()

	// create authenticated context
	ctx := metadata.NewOutgoingContext(context.TODO(), metadata.Pairs(strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, apigw.CookieHeader)), login.SessionID+"=jwt",
		"req-method", "GET"))

	out, err := a.HandleRequest(ctx, &input, prof, call)
	if !reflect.DeepEqual(&input, out) {
		t.Errorf("returned object does not match [%v]/[%v]", input, out)
	}
	if err != nil {
		t.Errorf("expecting nil, got error (%v)", err)
	}
	if called != 1 {
		t.Errorf("Expecting 1 call, got %d", called)
	}
	pa := prof.PreAuthNHooks()
	if len(pa) != 3 || mock.preAuthNCnt != len(pa) {
		t.Errorf("expecting 3 pre authN hooks got %d", len(pa))
	}
	pz := prof.PreAuthZHooks()
	if len(pz) != 4 || mock.preAuthZCnt != len(pz) {
		t.Errorf("expecting 4 pre authZ hooks got %d", len(pz))
	}
	pc := prof.PreCallHooks()
	if len(pc) != 2 || mock.preCallCnt != len(pc) {
		t.Errorf("expecting 2 pre call hooks got %d", len(pc))
	}
	tc := prof.PostCallHooks()
	if len(tc) != 1 || mock.postCallCnt != len(tc) {
		t.Errorf("expecting 1 pre call hooks got %d", len(tc))
	}

	// Test with the one preAuthn hook returning skip
	skipfn := func() bool {
		return mock.preCallCnt == 1
	}
	mock.skipCallFn = skipfn
	mock.preCallCnt, mock.postCallCnt, mock.preAuthNCnt, mock.preAuthZCnt = 0, 0, 0, 0
	called = 0
	// create authenticated context
	ctx = metadata.NewOutgoingContext(context.TODO(), metadata.Pairs(strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, apigw.CookieHeader)), login.SessionID+"=jwt",
		"req-method", "GET"))
	out, err = a.HandleRequest(ctx, &input, prof, call)
	if !reflect.DeepEqual(&input, out) {
		t.Errorf("returned object does not match [%v]/[%v]", input, out)
	}
	if err != nil {
		t.Errorf("expecting nil, got error (%v)", err)
	}
	if called != 0 {
		t.Errorf("Expecting 1 call, got %d", called)
	}
	pa = prof.PreAuthNHooks()
	if len(pa) != 3 || mock.preAuthNCnt != len(pa) {
		t.Errorf("expecting 3 pre authN hooks got %d/%d", len(pa), mock.preAuthNCnt)
	}
	pz = prof.PreAuthZHooks()
	if len(pz) != 4 || mock.preAuthZCnt != len(pz) {
		t.Errorf("expecting 4 pre authZ hooks got %d/%d", len(pz), mock.preAuthZCnt)
	}
	pc = prof.PreCallHooks()
	if len(pc) != 2 || mock.preCallCnt != len(pc) {
		t.Errorf("expecting 2 pre call hooks got %d/%d", len(pc), mock.preCallCnt)
	}
	tc = prof.PostCallHooks()
	if len(tc) != 1 || mock.postCallCnt != len(tc) {
		t.Errorf("expecting 1 pre call hooks got %d/%d", len(tc), mock.postCallCnt)
	}
}

func TestIsSkipped(t *testing.T) {
	config := apigw.Config{
		SkipBackends: []string{"valid", "valid2"},
	}
	cases := []struct {
		svc string
		ret bool
	}{
		{svc: "valid.ServiceV1", ret: true},
		{svc: "valid1.ServiceV1", ret: false},
		{svc: "vali.dServiceV1", ret: false},
		{svc: "valid2.ServiceV1", ret: true},
	}
	for _, c := range cases {
		ret := isSkipped(config, c.svc)
		if ret != c.ret {
			t.Errorf("for svc [%v] expecting [%v] got [%v]", c.svc, c.ret, ret)
		}
	}
}

func TestIsRequestAuthenticated(t *testing.T) {
	tests := []struct {
		name     string
		md       metadata.MD
		expected bool
	}{
		{
			name:     "no metadata in context",
			md:       nil,
			expected: false,
		},
		{
			name:     "no Cookie and Authorization headers in metadata",
			md:       metadata.Pairs(),
			expected: false,
		},
		{
			name: "Cookie header in metadata",
			md: metadata.Pairs(strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, apigw.CookieHeader)), login.SessionID+"=jwt",
				"req-method", "GET"),
			expected: true,
		},
		{
			name: "sid cookie missing in Cookie header",
			md: metadata.Pairs(strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, apigw.CookieHeader)), "random=jwt",
				"req-method", "GET"),
			expected: false,
		},
		{
			name: "Authorization header in metadata",
			md: metadata.Pairs(strings.ToLower(apigw.GrpcMDAuthorizationHeader), "Bearer jwt",
				"req-method", "GET"),
			expected: true,
		},
		{
			name: "incorrect (without 'Bearer') Authorization header in metadata",
			md: metadata.Pairs(strings.ToLower(apigw.GrpcMDAuthorizationHeader), "jwt",
				"req-method", "GET"),
			expected: false,
		},
		{
			name: "invalid JWT in Authorization header in metadata",
			md: metadata.Pairs(strings.ToLower(apigw.GrpcMDAuthorizationHeader), "Bearer "+"invalidToken",
				"req-method", "GET"),
			expected: false,
		},
		{
			name:     "no req-method header in metadata",
			md:       metadata.Pairs(strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, apigw.CookieHeader)), login.SessionID+"=jwt"),
			expected: false,
		},
		{
			name: "CORS request with no CSRF token in metadata",
			md: metadata.Pairs(strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, apigw.CookieHeader)), login.SessionID+"=jwt",
				strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, "Origin")), "somedomain.com",
				"req-method", "POST"),
			expected: false,
		},
		{
			name: "CORS request with invalid CSRF token in metadata",
			md: metadata.Pairs(strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, apigw.CookieHeader)), login.SessionID+"=jwt",
				strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, "Origin")), "somedomain.com",
				strings.ToLower(apigw.GrpcMDCsrfHeader[len(runtime.MetadataHeaderPrefix):]), "csrfToken",
				"req-method", "POST"),
			expected: false,
		},
	}
	for _, test := range tests {
		_ = MustGetAPIGateway()

		a := singletonAPIGw

		logConfig := log.GetDefaultConfig("TestApiGw")
		l := log.GetNewLogger(logConfig)
		a.logger = l
		a.authnMgr = manager.NewMockAuthenticationManager()
		ctx := context.TODO()
		if test.md != nil {
			ctx = metadata.NewOutgoingContext(ctx, test.md)
		}
		_, ok := a.isRequestAuthenticated(ctx)
		Assert(t, test.expected == ok, fmt.Sprintf("[%v] test failed", test.name))
	}
}
