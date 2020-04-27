package apigwpkg

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"net/http/httptest"
	"os"
	"reflect"
	"sort"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/pensando/grpc-gateway/runtime"
	gwruntime "github.com/pensando/grpc-gateway/runtime"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/bulkedit"
	apierrors "github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/staging"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apigw"
	cmdtypes "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	auditutils "github.com/pensando/sw/venice/utils/audit"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	authnmgr "github.com/pensando/sw/venice/utils/authn/manager"
	"github.com/pensando/sw/venice/utils/authz"
	authzmgr "github.com/pensando/sw/venice/utils/authz/manager"
	diagmock "github.com/pensando/sw/venice/utils/diagnostics/mock"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	hooksCalled int

	logger = log.GetNewLogger(log.GetDefaultConfig("apigw_test"))

	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("apigw_test", logger))
)

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

func (t *testGwService) GetCrudServiceProfile(object string, oper apiintf.APIOperType) (apigw.ServiceProfile, error) {
	return nil, nil
}

func (t *testGwService) GetProxyServiceProfile(path string) (apigw.ServiceProfile, error) {
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
	logConfig.Filter = log.AllowAllFilter
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
			name:     "cross origin GET request not TLS",
			method:   "GET",
			origin:   "http://test.com",
			host:     "127.0.0.1",
			expected: http.StatusOK,
		},
		{
			name:     "cross origin GET request",
			method:   "GET",
			origin:   "https://test.com",
			host:     "127.0.0.1",
			expected: http.StatusOK,
		},
		{
			name:     "cross origin POST request not TLS",
			method:   "POST",
			origin:   "http://test.com",
			host:     "127.0.0.1",
			expected: http.StatusForbidden,
		},
		{
			name:     "cross origin POST request",
			method:   "POST",
			origin:   "https://test.com",
			host:     "127.0.0.1",
			expected: http.StatusForbidden,
		},
		{
			name:     "same origin POST request not TLS",
			method:   "POST",
			origin:   "http://127.0.0.1",
			host:     "127.0.0.1",
			expected: http.StatusForbidden,
		},
		{
			name:     "same origin POST request",
			method:   "POST",
			origin:   "https://127.0.0.1",
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
	req, err := http.NewRequest("POST", "http://127.0.0.1/"+globals.ConfigURIPrefix+"/TestSvc/v1/Resource", nil)
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
	if v, ok := hdr[apigw.GrpcMDRequestURI]; !ok || v[0] != "/"+globals.ConfigURIPrefix+"/TestSvc/Resource" {
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
		HTTPAddr:           ":0",
		DebugMode:          true,
		Logger:             l,
		Resolvers:          []string{""},
		Auditor:            auditmgr.WithAuditors(auditmgr.NewLogAuditor(context.TODO(), l)),
		ModuleWatcher:      diagmock.GetModuleWatcher(),
		DiagnosticsService: diagmock.GetDiagnosticsService(),
	}
	_ = MustGetAPIGateway()
	a := singletonAPIGw
	hooks := hooksCalled
	go a.Run(config)

	AssertEventually(t,
		func() (bool, interface{}) {
			if strings.Contains(buf.String(), "Http Listen Start") {
				return true, ""
			}
			return false, buf.String()
		},
		"Rest server should have started")

	err := errors.New("Testing Exit for ApiGateway")
	a.doneCh <- err

	AssertEventually(t,
		func() (bool, interface{}) {
			if strings.Contains(buf.String(), "Testing Exit for ApiGateway") {
				return true, ""
			}
			return false, buf.String()
		},
		"apigw should have called close on error")
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

func TestCleanupObjMeta(t *testing.T) {
	_ = MustGetAPIGateway()
	a := singletonAPIGw

	creationTime := &api.Timestamp{}
	creationTime.Parse("2018-11-09T23:16:17Z")
	systemLabel := fmt.Sprintf("%s%s", globals.SystemLabelPrefix, "systemLabel")

	input := struct {
		api.ObjectMeta
		api.TypeMeta
	}{
		ObjectMeta: api.ObjectMeta{
			Name:         "input",
			CreationTime: *creationTime,
			ModTime:      *creationTime,
			SelfLink:     "selfLink",
			Labels: map[string]string{
				"userLabel": "v1",
				systemLabel: "v1",
			},
		},
	}
	a.cleanupObjMeta(&input)

	AssertEquals(t, input.CreationTime, api.Timestamp{}, "Creation time was not cleared")
	AssertEquals(t, input.ModTime, api.Timestamp{}, "Mod time was not cleared")
	AssertEquals(t, "", input.SelfLink, "Self link was not cleared")
	AssertEquals(t, 1, len(input.Labels), "Number of labels did not match expected")
	AssertEquals(t, "v1", input.Labels["userLabel"], "Number of labels did not match expected")
	_, ok := input.Labels[systemLabel]
	Assert(t, !ok, "System label was not removed")
}

func TestHandleRequest(t *testing.T) {
	prof := NewServiceProfile(nil, "", "", apiintf.UnknownOper)
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
	a.runstate.running = true
	a.runstate.addr = &mockAddr{}
	buf := &bytes.Buffer{}
	logConfig := log.GetDefaultConfig("TestApiGw")
	l := log.GetNewLogger(logConfig).SetOutput(buf)
	a.logger = l
	a.authnMgr = authnmgr.NewMockAuthenticationManager()
	a.authzMgr = authzmgr.NewAlwaysAllowAuthorizer()
	// create authenticated context
	ctx := metadata.NewOutgoingContext(context.TODO(), metadata.Pairs(strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, apigw.CookieHeader)), login.SessionID+"=jwt",
		"req-method", "GET", "audit-req-uri", "/configs/security/v1/tenant/default/roles"))
	// context with authz operations
	mock.retAuthzCtx = NewContextWithOperations(ctx, authz.NewOperation(authz.NewResource(
		globals.DefaultTenant,
		string(apiclient.GroupAuth),
		string(auth.KindUser),
		globals.DefaultNamespace,
		""), auth.Permission_Create.String()))
	a.auditor = auditmgr.NewLogAuditor(ctx, l)
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
		t.Errorf("expecting 4 pre authZ hooks got %d/%d", len(pz), mock.preAuthZCnt)
	}
	pc := prof.PreCallHooks()
	if len(pc) != 2 || mock.preCallCnt != len(pc) {
		t.Errorf("expecting 2 pre call hooks got %d", len(pc))
	}
	tc := prof.PostCallHooks()
	if len(tc) != 1 || mock.postCallCnt != len(tc) {
		t.Errorf("expecting 1 pre call hooks got %d", len(tc))
	}

	// Test with the one preCall hook returning skip
	skipfn := func() bool {
		return mock.preCallCnt == 1
	}
	mock.skipCallFn = skipfn
	mock.preCallCnt, mock.postCallCnt, mock.preAuthNCnt, mock.preAuthZCnt = 0, 0, 0, 0
	called = 0
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
	// test events in case of auditing failure
	// create mock events recorder
	recorder.Override(mockevtsrecorder.NewRecorder("apigw_test", a.logger))
	defer recorder.Override(mockevtsrecorder.NewRecorder("apigw_test", logger))
	a.auditor = auditmgr.WithMockErrorSimulator(ctx, a.logger)
	out, err = a.HandleRequest(ctx, &input, prof, call)
	if !strings.Contains(buf.String(), "Failure in recording audit event") {
		t.Errorf("expecting event for auditing failure")
	}
	// test api.Status returned by pre-call
	prof = NewServiceProfile(nil, "", "", apiintf.UnknownOper)
	s := &api.Status{
		TypeMeta: api.TypeMeta{Kind: "Status"},
		Code:     int32(codes.InvalidArgument),
		Result:   api.StatusResult{Str: "invalid argument"},
		Message:  []string{"invalid argument"},
	}
	mock.retErr = s
	prof.AddPreCallHook(mock.preCallHook)
	mock.preCallCnt, mock.postCallCnt, mock.preAuthNCnt, mock.preAuthZCnt = 0, 0, 0, 0
	called = 0
	a.skipAuth = true
	a.skipAuthz = true
	out, err = a.HandleRequest(ctx, &input, prof, call)
	if !reflect.DeepEqual(apierrors.AddDetails(s), err) {
		t.Errorf("expecting api.Status returned from pre call hook")
	}
	prof.ClearPreCallHooks()
	prof.AddPostCallHook(mock.postCallHook)
	out, err = a.HandleRequest(ctx, &input, prof, call)
	if !reflect.DeepEqual(apierrors.AddDetails(s), err) {
		t.Errorf("expecting api.Status returned from post call hook")
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
			name: "nil req-method header in metadata",
			md: metadata.Join(metadata.Pairs(strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, apigw.CookieHeader)), login.SessionID+"=jwt"),
				metadata.MD{"req-method": nil}),
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
		a.authnMgr = authnmgr.NewMockAuthenticationManager()
		ctx := context.TODO()
		if test.md != nil {
			ctx = metadata.NewOutgoingContext(ctx, test.md)
		}
		_, ok := a.isRequestAuthenticated(ctx)
		Assert(t, test.expected == ok, fmt.Sprintf("[%v] test failed", test.name))
	}
}

func TestAuthzFailures(t *testing.T) {
	// create authenticated context
	ctx := metadata.NewOutgoingContext(context.TODO(), metadata.Pairs(strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, apigw.CookieHeader)), login.SessionID+"=jwt",
		"req-method", "GET"))

	tests := []struct {
		name       string
		hooks      *testHooks
		authorizer authz.Authorizer
		err        error
	}{
		{
			name: "error in pre authz hook",
			hooks: &testHooks{
				retErr: errors.New("error in pre authz hook"),
			},
			authorizer: authzmgr.NewAlwaysAllowAuthorizer(),
			err:        apierrors.ToGrpcError("Pre authorization processing failed", []string{"error in pre authz hook"}, int32(codes.InvalidArgument), "", nil),
		},
		{
			name: "no operations in returns context",
			hooks: &testHooks{
				retAuthzCtx: context.TODO(),
			},
			authorizer: authzmgr.NewAlwaysAllowAuthorizer(),
			err:        apierrors.ToGrpcError("Authorization failed", []string{"not authorized"}, int32(codes.PermissionDenied), "", nil),
		},
		{
			name: "operation not authorized",
			hooks: &testHooks{
				retAuthzCtx: NewContextWithOperations(ctx, nil),
			},
			authorizer: authzmgr.NewAlwaysDenyAuthorizer(),
			err:        apierrors.ToGrpcError("Authorization failed", []string{"not authorized"}, int32(codes.PermissionDenied), "", nil),
		},
	}
	for _, test := range tests {
		prof := NewServiceProfile(nil, "", "", apiintf.UnknownOper)
		prof.AddPreAuthZHook(test.hooks.preAuthZHook)
		called := 0
		input := struct {
			Test string
		}{"testing"}
		call := func(ctx context.Context, in interface{}) (interface{}, error) {
			called++
			return in, nil
		}
		test.hooks.retObj = &input
		_ = MustGetAPIGateway()

		a := singletonAPIGw

		logConfig := log.GetDefaultConfig("TestApiGw")
		l := log.GetNewLogger(logConfig)
		a.logger = l
		a.authnMgr = authnmgr.NewMockAuthenticationManager()
		a.authzMgr = test.authorizer

		out, err := a.HandleRequest(ctx, &input, prof, call)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, out == nil, fmt.Sprintf("obj returned should be nil, [%v] test failed", test.name))
	}
}

func TestNewProxyHandler(t *testing.T) {
	goodcases := []struct {
		path, destination string
		useResolver       bool
	}{
		{path: "/test/path", destination: "pen-apiserver", useResolver: true},
		{path: "/test/path1", destination: "localhost:1111", useResolver: false},
		{path: "/test/path1", destination: "http://localhost:1111", useResolver: false},
		{path: "/test/path1", destination: "https://localhost:1111", useResolver: false},
		{path: "/test/path1", destination: "unknowns://localhost:1111", useResolver: true},
	}
	for _, c := range goodcases {
		p, err := NewRProxyHandler(c.path, "", "", c.destination, nil)
		if err != nil {
			t.Fatalf("failed get proxy handler")
		}
		if p.destination != c.destination || p.path != c.path || p.useResolver != c.useResolver {
			t.Fatalf("returned proxy handler does not match expected values [%v]/[%v]/[%v]", p.destination, p.path, p.useResolver)
		}
		if p.useResolver && p.proxy.Director == nil {
			t.Fatalf("director not set up for resolved proxy")
		}
	}
	badcases := []string{":ahtt#@Sc---ada", "-http:\\aadadaada"}
	for _, c := range badcases {
		_, err := NewRProxyHandler("/test/path1", "", "", c, nil)
		if err == nil {
			t.Fatalf("expecting to fail [%s]", c)
		}
	}
}

func TestProxyDirector(t *testing.T) {
	rslver := mockresolver.New()
	svcs := []cmdtypes.ServiceInstance{
		{
			ObjectMeta: api.ObjectMeta{
				Name: "node1",
			},
			Service: "dummyserver",
			URL:     "node1:111",
		},
		{
			ObjectMeta: api.ObjectMeta{
				Name: "node2",
			},
			Service: "dummyserver",
			URL:     "node2:111",
		},
	}
	buf := &bytes.Buffer{}
	logConfig := log.GetDefaultConfig("TestApiGw")
	l := log.GetNewLogger(logConfig).SetOutput(buf)
	_ = MustGetAPIGateway()
	singletonAPIGw.logger = l
	singletonAPIGw.rslver = rslver
	req, err := http.NewRequest("GET", "http://localhost", nil)
	if err != nil {
		t.Fatalf("unable to get http request(%s)", err)
	}
	p, err := NewRProxyHandler("/test/path", "", "", "dummyserver", nil)
	if err != nil {
		t.Fatalf("unable to get proxy handler (%s)", err)
	}
	// Test without any service instances
	p.director(req)
	if req.Host != "" || req.URL.Host != "" {
		t.Fatalf("request destination not cleared [%v][%v]", req.Host, req.URL.Host)
	}
	for i := range svcs {
		rslver.AddServiceInstance(&svcs[i])
	}

	var node1, node2 bool
	for i := 0; i < 1000; i++ {
		p.director(req)
		if req.Host == "" || req.URL.Host == "" {
			t.Fatalf("req not set to resolved service[%v][%v]", req.Host, req.URL.Host)
		}
		if strings.Contains(req.Host, "node1:111") {
			node1 = true
		}
		if strings.Contains(req.Host, "node2:111") {
			node2 = true
		}
	}
	if !(node1 && node2) {
		t.Fatalf("request did not get load balanced [%v],[%v]", node1, node2)
	}
}

func TestHandleProxyRequest(t *testing.T) {
	prof := NewServiceProfile(nil, "", "", apiintf.UnknownOper)
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

	input := struct {
		Test string
	}{"testing"}

	mock.retObj = &input

	req := httptest.NewRequest("GET", "http://example.com/foo", nil)

	// set user and perm headers to check if they are deleted if set
	req.Header[authz.UsernameKey] = []string{"testuser"}
	req.Header[authz.UserTenantKey] = []string{globals.DefaultTenant}
	req.Header[authz.PermsKey] = []string{"xyz"}

	w := httptest.NewRecorder()
	_ = MustGetAPIGateway()

	buf := &bytes.Buffer{}
	logConfig := log.GetDefaultConfig("TestApiGw")
	l := log.GetNewLogger(logConfig).SetOutput(buf)
	singletonAPIGw.logger = l
	singletonAPIGw.authnMgr = authnmgr.NewMockAuthenticationManager()
	singletonAPIGw.authzMgr = authzmgr.NewAlwaysAllowAuthorizer()

	// create authenticated context
	ctx := metadata.NewOutgoingContext(context.TODO(), metadata.Pairs(strings.ToLower(fmt.Sprintf("%s%s", runtime.MetadataPrefix, apigw.CookieHeader)), login.SessionID+"=jwt",
		"req-method", "GET"))
	// context with authz operations
	mock.retAuthzCtx = NewContextWithOperations(ctx, nil)
	mock.useRetAuthnCtx = true
	mock.retAuthnCtx = mock.retAuthzCtx
	p, err := NewRProxyHandler("/test/path", "", "", "dummyserver", prof)
	if err != nil {
		t.Fatalf("unable to get proxy handler (%s)", err)
	}
	req = req.WithContext(ctx)
	p.ServeHTTP(w, req)

	pa := prof.PreAuthNHooks()
	if len(pa) != 3 || mock.preAuthNCnt != len(pa) {
		t.Errorf("expecting %v pre authN hooks got %d", len(pa), mock.preAuthNCnt)
	}
	pz := prof.PreAuthZHooks()
	if len(pz) != 4 || mock.preAuthZCnt != len(pz) {
		t.Errorf("expecting %v pre authZ hooks got %d", len(pz), mock.preAuthZCnt)
	}
	pc := prof.PreCallHooks()
	if len(pc) != 2 || mock.preCallCnt != len(pc) {
		t.Errorf("expecting %v pre call hooks got %d", len(pc), mock.preCallCnt)
	}
	tc := prof.PostCallHooks()
	if len(tc) != 1 {
		t.Errorf("expecting %v post call hooks got %d", len(tc), mock.postCallCnt)
	}
	if mock.postCallCnt != 0 {
		t.Errorf("expecting post call hook to be not called, got called %d times", mock.postCallCnt)
	}
	if !strings.Contains(buf.String(), "Operation failed to complete with status code: 502") {
		t.Errorf("Expected log file to contain failure for failed reverse proxy operation")
	}
	// Test with the one preAuthn hook returning skip
	skipfn := func() bool {
		return mock.preCallCnt == 1
	}
	mock.skipCallFn = skipfn
	mock.preCallCnt, mock.postCallCnt, mock.preAuthNCnt, mock.preAuthZCnt = 0, 0, 0, 0
	p.ServeHTTP(w, req)
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
	// check if user and perm info headers are deleted
	_, ok := req.Header[authz.UsernameKey]
	if ok {
		t.Errorf("expecting user key to be not present")
	}
	_, ok = req.Header[authz.UserTenantKey]
	if ok {
		t.Errorf("expecting user tenant key to be not present")
	}
	_, ok = req.Header[authz.PermsKey]
	if ok {
		t.Errorf("expecting user perms key to be not present")
	}
}

func TestGetClientIPs(t *testing.T) {
	tests := []struct {
		name     string
		md       metadata.MD
		expected []string
	}{
		{
			name:     "no metadata in context",
			md:       nil,
			expected: nil,
		},
		{
			name:     "no headers in metadata",
			md:       metadata.Pairs(),
			expected: nil,
		},
		{
			name:     "X-Forwarded-For header in metadata",
			md:       metadata.Pairs(apigw.XForwardedFor, "192.168.30.9"),
			expected: []string{"192.168.30.9"},
		},
		{
			name:     "Forwarded header in metadata",
			md:       metadata.Pairs(apigw.Forwarded, "for=192.168.30.9"),
			expected: []string{"192.168.30.9"},
		},
		{
			name:     "Multiple ips in Forwarded header",
			md:       metadata.Pairs(apigw.Forwarded, "for=192.168.30.9", apigw.Forwarded, "for=192.168.30.10"),
			expected: []string{"192.168.30.9", "192.168.30.10"},
		},
		{
			name:     "X-Real-IP header in metadata",
			md:       metadata.Pairs(apigw.XRealIP, "192.168.30.9"),
			expected: []string{"192.168.30.9"},
		},
	}
	for _, test := range tests {
		ctx := context.TODO()
		if test.md != nil {
			ctx = metadata.NewOutgoingContext(ctx, test.md)
		}
		clientIPs := getClientIPs(ctx)
		sort.Strings(test.expected)
		sort.Strings(clientIPs)
		Assert(t, reflect.DeepEqual(test.expected, clientIPs), fmt.Sprintf("[%v] test failed", test.name))
	}
}

func TestAudit(t *testing.T) {
	sgPolicy := &security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "policy1",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					FromIPAddresses: []string{"10.0.0.0/24"},
					ToIPAddresses:   []string{"11.0.0.0/24"},
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: "PERMIT",
				},
			},
		},
	}
	b, _ := json.Marshal(sgPolicy)
	sgPolicyStr := string(b)
	testCtx := context.Background()

	tests := []struct {
		name     string
		user     *auth.User
		ctx      context.Context
		reqObj   interface{}
		respObj  interface{}
		ops      []authz.Operation
		level    audit.Level
		stage    audit.Stage
		outcome  audit.Outcome
		apierr   error
		reqURI   string
		eventStr string
		err      error
	}{
		{
			name:     "nil user",
			user:     nil,
			eventStr: "no user to audit",
			err:      nil,
			ctx:      testCtx,
		},
		{
			name: "nil ops",
			user: &auth.User{
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
			},
			eventStr: "no operations to audit",
			err:      nil,
			ctx:      testCtx,
		},
		{
			name: "nil ops",
			user: &auth.User{
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
			},
			eventStr: "no operations to audit",
			ctx:      testCtx,
			err:      nil,
		},
		{
			name: "log request obj",
			user: &auth.User{
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
			},
			reqObj: sgPolicy,
			ops: []authz.Operation{authz.NewOperation(
				authz.NewResource(sgPolicy.Tenant, string(apiclient.GroupSecurity), sgPolicy.Kind, sgPolicy.Namespace, sgPolicy.Name),
				auth.Permission_Create.String())},
			level:    audit.Level_Request,
			stage:    audit.Stage_RequestAuthorization,
			outcome:  audit.Outcome_Success,
			apierr:   nil,
			reqURI:   "/configs/security/v1/tenant/testTenant/networksecuritypolicies",
			eventStr: "request-object=\"" + sgPolicyStr + "\"",
			err:      nil,
			ctx:      testCtx,
		},
		{
			name: "log response obj",
			user: &auth.User{
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
			},
			respObj: sgPolicy,
			ops: []authz.Operation{authz.NewOperation(
				authz.NewResource(sgPolicy.Tenant, string(apiclient.GroupSecurity), sgPolicy.Kind, sgPolicy.Namespace, sgPolicy.Name),
				auth.Permission_Create.String())},
			level:    audit.Level_Response,
			stage:    audit.Stage_RequestProcessing,
			outcome:  audit.Outcome_Success,
			apierr:   nil,
			reqURI:   "/configs/security/v1/tenant/testTenant/networksecuritypolicies",
			eventStr: "response-object=\"" + sgPolicyStr + "\"",
			err:      nil,
			ctx:      testCtx,
		},
		{
			name: "log api error",
			user: &auth.User{
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
			},
			ops: []authz.Operation{authz.NewOperation(
				authz.NewResource(sgPolicy.Tenant, string(apiclient.GroupSecurity), sgPolicy.Kind, sgPolicy.Namespace, sgPolicy.Name),
				auth.Permission_Create.String())},
			level:    audit.Level_Response,
			stage:    audit.Stage_RequestProcessing,
			outcome:  audit.Outcome_Failure,
			apierr:   apierrors.ToGrpcError("duplicate policy", []string{"Operation failed to complete"}, int32(codes.Aborted), "", nil),
			reqURI:   "/configs/security/v1/tenant/testTenant/networksecuritypolicies",
			eventStr: "duplicate policy",
			err:      nil,
			ctx:      testCtx,
		},
		{
			name: "read operation",
			user: &auth.User{
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
			},
			reqObj: sgPolicy,
			ops: []authz.Operation{authz.NewOperation(
				authz.NewResource(sgPolicy.Tenant, string(apiclient.GroupSecurity), sgPolicy.Kind, sgPolicy.Namespace, sgPolicy.Name),
				auth.Permission_Read.String())},
			level:    audit.Level_Request,
			stage:    audit.Stage_RequestProcessing,
			outcome:  audit.Outcome_Success,
			apierr:   nil,
			eventStr: "",
			err:      nil,
			ctx:      testCtx,
		},
		{
			name: "log external request id",
			user: &auth.User{
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
			},
			ctx:    metadata.NewOutgoingContext(context.TODO(), metadata.Pairs(auditutils.ExtRequestIDHeader, "corr-id-1")),
			reqObj: sgPolicy,
			ops: []authz.Operation{authz.NewOperation(
				authz.NewResource(sgPolicy.Tenant, string(apiclient.GroupSecurity), sgPolicy.Kind, sgPolicy.Namespace, sgPolicy.Name),
				auth.Permission_Create.String())},
			level:    audit.Level_Request,
			stage:    audit.Stage_RequestAuthorization,
			outcome:  audit.Outcome_Success,
			apierr:   nil,
			reqURI:   "/configs/security/v1/tenant/testTenant/networksecuritypolicies",
			eventStr: "external-id=corr-id-1",
			err:      nil,
		},
	}
	_ = MustGetAPIGateway()

	a := singletonAPIGw
	a.runstate.running = true
	a.runstate.addr = &mockAddr{}
	for _, test := range tests {
		buf := &bytes.Buffer{}
		logConfig := log.GetDefaultConfig("TestApiGw")
		l := log.GetNewLogger(logConfig).SetOutput(buf)
		a.logger = l
		a.auditor = auditmgr.NewLogAuditor(context.TODO(), l)
		err := a.audit(test.ctx, test.user, test.reqObj, test.respObj, test.ops, test.level, test.stage, test.outcome, test.apierr, nil, test.reqURI)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected error [%v], got [%v]", test.name, test.err, err))
		bufStr := buf.String()
		bufStr = strings.Replace(bufStr, "\\", "", -1)
		Assert(t, strings.Contains(bufStr, test.eventStr), fmt.Sprintf("[%s] test failed, expected log [%s] to contain [%s]", test.name, bufStr, test.eventStr))
		if test.eventStr == "" {
			Assert(t, len(bufStr) == 0, fmt.Sprintf("[%s] test failed, no audit log expected", test.name))
		}
	}
}

func TestAuditErrorTruncation(t *testing.T) {
	errBytes := []byte(CreateAlphabetString(513))
	sgPolicy := &security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "policy1",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					FromIPAddresses: []string{"10.0.0.0/24"},
					ToIPAddresses:   []string{"11.0.0.0/24"},
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "80",
						},
					},
					Action: "PERMIT",
				},
			},
		},
	}
	tests := []struct {
		name         string
		err          error
		errStr       string
		errStrCount  int
		loggedErrStr string
		truncated    bool
	}{
		{
			name:         "error message more than 512 bytes",
			err:          apierrors.ToGrpcError("Operation failed to complete", []string{string(errBytes)}, int32(codes.Aborted), "", nil),
			errStr:       string(errBytes),
			errStrCount:  1,
			loggedErrStr: string(errBytes[:512]),
			truncated:    true,
		},
		{
			name:         "error message less than 512 bytes",
			err:          apierrors.ToGrpcError("Operation failed to complete", []string{string(errBytes[:300])}, int32(codes.Aborted), "", nil),
			errStr:       string(errBytes[:300]),
			errStrCount:  1,
			loggedErrStr: string(errBytes[:300]),
			truncated:    false,
		},
		{
			name:         "multiple error messages more than 512 bytes",
			err:          apierrors.ToGrpcError("Operation failed to complete", []string{string(errBytes[:300]), string(errBytes[:300])}, int32(codes.Aborted), "", nil),
			errStr:       string(errBytes[:301]),
			errStrCount:  2,
			loggedErrStr: string(errBytes[:212]),
			truncated:    true,
		},
		{
			name:         "multiple error messages less than 512 bytes",
			err:          apierrors.ToGrpcError("Operation failed to complete", []string{string(errBytes[:256]), string(errBytes[:256])}, int32(codes.Aborted), "", nil),
			errStr:       string(errBytes[:256]),
			errStrCount:  2,
			loggedErrStr: string(errBytes[:256]),
			truncated:    false,
		},
	}
	user := &auth.User{
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
	}
	ops := []authz.Operation{authz.NewOperation(
		authz.NewResource(sgPolicy.Tenant, string(apiclient.GroupSecurity), sgPolicy.Kind, sgPolicy.Namespace, sgPolicy.Name),
		auth.Permission_Create.String())}

	_ = MustGetAPIGateway()

	a := singletonAPIGw
	a.runstate.running = true
	a.runstate.addr = &mockAddr{}
	for _, test := range tests {
		buf := &bytes.Buffer{}
		logConfig := log.GetDefaultConfig("TestApiGw")
		l := log.GetNewLogger(logConfig).SetOutput(buf)
		a.logger = l
		a.auditor = auditmgr.NewLogAuditor(context.TODO(), l)
		err := a.audit(context.TODO(), user, sgPolicy, nil, ops, audit.Level_Response, audit.Stage_RequestProcessing, audit.Outcome_Failure, test.err, nil, "/configs/security/v1/tenant/testTenant/networksecuritypolicies")
		AssertOk(t, err, "unexpected error logging audit event")
		bufStr := buf.String()
		bufStr = strings.Replace(bufStr, "\\", "", -1)
		Assert(t, strings.Contains(bufStr, test.loggedErrStr), fmt.Sprintf("[%s] test failed, expected log [%s] to contain [%s]", test.name, bufStr, test.loggedErrStr))
		Assert(t, strings.Count(bufStr, test.loggedErrStr) == test.errStrCount,
			fmt.Sprintf("[%s] test failed, expected log [%s] to contain error string count [%d], got [%d]", test.name, bufStr, test.errStrCount, strings.Count(bufStr, test.loggedErrStr)))
		if test.truncated {
			Assert(t, !strings.Contains(bufStr, test.errStr), fmt.Sprintf("[%s] test failed, expected log [%s] to not contain [%s]", test.name, bufStr, test.errStr))
		}
	}
}

func TestModuleChangeCb(t *testing.T) {
	_ = MustGetAPIGateway()

	a := singletonAPIGw
	logConfig := log.GetDefaultConfig("TestApiGw")
	l := log.GetNewLogger(logConfig)
	a.logger = l
	modObj := &diagnostics.Module{}
	modObj.Defaults("all")
	modObj.Spec.LogLevel = diagnostics.ModuleSpec_Debug.String()

	old := os.Stdout // backup real stdout
	r, w, err := os.Pipe()
	AssertOk(t, err, "error creating stdout pipe")
	os.Stdout = w

	a.moduleChangeCb(modObj)
	a.logger.DebugLog("msg", "test moduleChangeCb")

	outCh := make(chan string)

	go func() {
		var buf bytes.Buffer
		io.Copy(&buf, r)
		outCh <- buf.String()
	}()

	w.Close()
	os.Stdout = old // restoring the real stdout
	out := <-outCh

	Assert(t, strings.Contains(out, "level=debug"), "debug log level should be set in {%s}", out)
}

type mockAddr struct{}

func (m *mockAddr) Network() string {
	return "tcp"
}

func (m *mockAddr) String() string {
	return "localhost:63000"
}

// TestBulkEditAudit Calls the audit function with the bulkOper flag set in Ctx and checks whether the number of audit logs generated
// matches the number of bulk operations being performed
func TestBulkEditAudit(t *testing.T) {

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

	n1, _ := types.MarshalAny(netw)
	netw.Spec.IPv4Gateway = "110.1.1.1"
	netw.Spec.IPv4Subnet = "110.1.1.1/24"
	n2, _ := types.MarshalAny(netw)
	netw.Name = "testDelNetw"
	netw.Spec.IPv4Gateway = "120.2.1.1"
	netw.Spec.IPv4Subnet = "120.2.1.1/24"
	n3, _ := types.MarshalAny(netw)

	bulkEditBuff := &staging.BulkEditAction{
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
	b, _ := json.Marshal(bulkEditBuff)
	bulkEditBufStr := string(b)
	testCtx := context.Background()
	testCtx = AddBulkOperationsFlagToContext(testCtx)

	tests := []struct {
		name         string
		user         *auth.User
		ctx          context.Context
		reqObj       interface{}
		respObj      interface{}
		ops          []authz.Operation
		level        audit.Level
		stage        audit.Stage
		outcome      audit.Outcome
		apierr       error
		reqURI       string
		eventStr     string
		err          error
		numAuditLogs int
	}{
		{
			name: "log multiple objs",
			user: &auth.User{
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
			},
			reqObj: bulkEditBufStr,
			ops: []authz.Operation{
				authz.NewOperation(
					authz.NewResource(bulkEditBuff.Tenant, string(apiclient.GroupNetwork), "Network", bulkEditBuff.Namespace, "TestNtwork1"),
					auth.Permission_Create.String()),
				authz.NewOperation(
					authz.NewResource(bulkEditBuff.Tenant, string(apiclient.GroupNetwork), "Network", bulkEditBuff.Namespace, "TestNtwork1"),
					auth.Permission_Update.String()),
				authz.NewOperation(
					authz.NewResource(bulkEditBuff.Tenant, string(apiclient.GroupNetwork), "Network", bulkEditBuff.Namespace, "testDelNetw"),
					auth.Permission_Delete.String()),
			},
			level:        audit.Level_Request,
			stage:        audit.Stage_RequestAuthorization,
			outcome:      audit.Outcome_Success,
			apierr:       nil,
			reqURI:       "/configs/staging/v1/tenant/default/buffers/" + bufName,
			eventStr:     bulkEditBufStr,
			err:          nil,
			ctx:          testCtx,
			numAuditLogs: 3,
		},
		{
			name: "log response obj",
			user: &auth.User{
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
			},
			respObj: bulkEditBufStr,
			ops: []authz.Operation{
				authz.NewOperation(
					authz.NewResource(bulkEditBuff.Tenant, string(apiclient.GroupNetwork), "Network", bulkEditBuff.Namespace, "TestNtwork1"),
					auth.Permission_Create.String()),
				authz.NewOperation(
					authz.NewResource(bulkEditBuff.Tenant, string(apiclient.GroupNetwork), "Network", bulkEditBuff.Namespace, "TestNtwork1"),
					auth.Permission_Update.String()),
				authz.NewOperation(
					authz.NewResource(bulkEditBuff.Tenant, string(apiclient.GroupNetwork), "Network", bulkEditBuff.Namespace, "testDelNetw"),
					auth.Permission_Delete.String())},
			level:        audit.Level_Response,
			stage:        audit.Stage_RequestProcessing,
			outcome:      audit.Outcome_Success,
			apierr:       nil,
			reqURI:       "/configs/staging/v1/tenant/default/buffers/" + bufName,
			eventStr:     bulkEditBufStr,
			err:          nil,
			ctx:          testCtx,
			numAuditLogs: 3,
		},
	}
	_ = MustGetAPIGateway()

	a := singletonAPIGw
	a.runstate.running = true
	a.runstate.addr = &mockAddr{}
	for _, test := range tests {
		buf := &bytes.Buffer{}
		logConfig := log.GetDefaultConfig("TestApiGw")
		l := log.GetNewLogger(logConfig).SetOutput(buf)
		a.logger = l
		a.auditor = auditmgr.NewLogAuditor(context.TODO(), l)
		err := a.audit(test.ctx, test.user, test.reqObj, test.respObj, test.ops, test.level, test.stage, test.outcome, test.apierr, nil, test.reqURI)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected error [%v], got [%v]", test.name, test.err, err))
		bufStr := buf.String()
		bufStr = strings.Replace(bufStr, "\\", "", -1)
		Assert(t, strings.Count(bufStr, "audit log") == test.numAuditLogs, fmt.Sprintf("[%s] test failed, expected log [%s] to contain %d audit logs", test.name, bufStr, test.numAuditLogs))
		Assert(t, strings.Contains(bufStr, test.eventStr), fmt.Sprintf("[%s] test failed, expected log [%s] to contain [%s]", test.name, bufStr, test.eventStr))
		if test.eventStr == "" {
			Assert(t, len(bufStr) == 0, fmt.Sprintf("[%s] test failed, no audit log expected", test.name))
		}
	}
}
