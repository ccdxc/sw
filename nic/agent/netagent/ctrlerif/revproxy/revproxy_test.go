package revproxy

import (
	"net"
	"net/http"
	"os"
	"testing"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/httputils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

var apiPfx = "api"
var apiPort = "9007"
var apiResp = "hello from 9007"

var nicPfx = "nic"
var nicPort = "9008"
var nicResp = "hello from 9008"

var invalidBody = "nable to get a matching endpoint. Url: /test/ Path: [test"

func TestMain(m *testing.M) {
	srv1, err := newRestServer(":"+apiPort, "/"+apiPfx+"/", addRoutes)
	if err != nil {
		log.Fatalf("Test set up failed. Error: %v", err)
	}
	srv2, err := newRestServer(":"+nicPort, "/"+nicPfx+"/", addRoutes2)
	if err != nil {
		log.Fatalf("Test set up failed. Error: %v", err)
	}
	rev, err := NewRevProxyRouter(":8888")
	if err != nil {
		log.Fatalf("Could not start Rev Proxy Server. Error: %v", err)
	}

	testCode := m.Run()
	defer srv1.httpServer.Shutdown(nil)
	defer srv2.httpServer.Shutdown(nil)
	defer rev.Stop()
	os.Exit(testCode)
}

type restServer struct {
	httpServer *http.Server // HTTP server
}

func (s *restServer) responseHandler2(r *http.Request) (interface{}, error) {
	return nicResp, nil
}

func addRoutes2(r *mux.Router, srv *restServer) {
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(srv.responseHandler2))
}

func (s *restServer) responseHandler(r *http.Request) (interface{}, error) {
	return apiResp, nil
}

func addRoutes(r *mux.Router, srv *restServer) {
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(srv.responseHandler))
}

type routeAddFunc func(*mux.Router, *restServer)

func newRestServer(listenURL string, prefix string, api routeAddFunc) (*restServer, error) {
	srv := &restServer{}
	// setup the top level routes
	router := mux.NewRouter()
	prefixRoutes := map[string]routeAddFunc{
		prefix: api,
	}

	for prefix, subRouter := range prefixRoutes {
		sub := router.PathPrefix(prefix).Subrouter().StrictSlash(true)
		subRouter(sub, srv)
	}

	log.Infof("Starting server at %s", listenURL)

	// listener
	listener, err := net.Listen("tcp", listenURL)
	if err != nil {
		log.Errorf("Error starting listener. Err: %v", err)
		return nil, err
	}

	// create a http server
	httpServer := &http.Server{Addr: listenURL, Handler: router}
	go httpServer.Serve(listener)

	srv.httpServer = httpServer

	return srv, nil
}

func test(t *testing.T, pfx string, port string, url string, test string) {
	AddRevProxyDest(pfx, port)
	body, err := netutils.HTTPGetRaw(url)
	if err != nil {
		t.Errorf("Could not receive response. Error: %v", err)
	}
	s := string(body)
	s = s[1 : len(s)-2]
	if s != test {
		t.Errorf("invalid body %s", string(body))
	}
}

func TestRestServerStartStop(t *testing.T) {
	test(t, apiPfx, apiPort, "http://localhost:8888/api/", apiResp)
	test(t, nicPfx, nicPort, "http://localhost:8888/nic/", nicResp)
	test(t, nicPfx, nicPort, "http://localhost:8888/test/", invalidBody)
}
