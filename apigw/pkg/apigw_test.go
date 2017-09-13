package apigwpkg

import (
	"bytes"
	"context"
	"errors"
	"io"
	"net/http"
	"net/http/httptest"
	"testing"
	"time"

	"google.golang.org/grpc"

	"strings"

	"github.com/pensando/sw/apigw"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/resolver"
)

type testGwService struct {
	regevents int
}

func (t *testGwService) CompleteRegistration(ctx context.Context,
	logger log.Logger, grpcserver *grpc.Server, mux *http.ServeMux, rslvr resolver.Interface) error {
	t.regevents = t.regevents + 1
	return nil
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
	a := sinletonAPIGw
	s := testGwService{}
	s1 := testGwService{}
	a.Register("register-test1", "/test1", &s)
	a.Register("register-test2", "/test2", &s1)
	if len(a.svcmap) != 2 {
		t.Errorf("Mismatched number of registered services Want 2 found [%v]", len(a.svcmap))
	}

	for name := range a.svcmap {
		if name != "register-test1" && name != "register-test2" {
			t.Errorf("invalid name %s", name)
		}
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
	a := sinletonAPIGw
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
	handler := a.allowCORS(dummyfn)
	handler.ServeHTTP(resp, req)

	if resp.Code != http.StatusOK {
		t.Errorf("Wrong Status")
	}
	hdr := resp.Header()
	// Check that all the right headers have been set.
	if v, ok := hdr["Access-Control-Allow-Headers"]; !ok {
		found := false
		for _, h := range v {
			if h == "Content-Type" {
				found = true
			}
		}
		if !found {
			t.Errorf("Content-Type not set [%v][%s]", ok, v)
		}
	}

	if v, ok := hdr["Access-Control-Allow-Methods"]; !ok {
		for _, h := range v {
			if strings.Contains(h, "GET") || !strings.Contains(h, "POST") || !strings.Contains(h, "PUT") ||
				!strings.Contains(h, "DELETE") || !strings.Contains(h, "OPTIONS") {
				t.Errorf("Allow Methods not set [%v]", h)
			}
		}
	}
}

// TestExtractHdrInfo
// Test Extracting of the header information.
func TestExtractHdrInfo(t *testing.T) {
	_ = MustGetAPIGateway()
	a := sinletonAPIGw
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
	a := sinletonAPIGw
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
	a := sinletonAPIGw
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
	}
	_ = MustGetAPIGateway()
	a := sinletonAPIGw
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
