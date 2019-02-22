package revproxy

import (
	"io/ioutil"
	"net/http"
	"net/http/httptest"
	"testing"

	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestRevProxy(t *testing.T) {
	proxyCfg := map[string]*struct {
		prefix     []string
		httpServer *httptest.Server
		resp       *httptest.ResponseRecorder
	}{
		"nmd": {
			prefix: []string{"/api/v1/naples",
				"/monitoring/",
				"/cores/",
				"/cmd/",
				"/update/"},
		},
		"tmagent": {
			prefix: []string{"/telemetry/",
				"/api/telemetry/"},
		},
		"events": {
			prefix: []string{"/api/eventpolicies/"},
		},

		"netagent": {
			prefix: []string{"/api/"},
		},
	}

	for k, rp := range proxyCfg {
		func(agent string) {
			// start proxy http server
			rp.httpServer = httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
				log.Infof("[%s] received req: %v", agent, r.URL)
				w.Write([]byte(agent))

			}))
		}(k)
		defer rp.httpServer.Close()
	}

	proxyRte := map[string]string{}
	for _, rp := range proxyCfg {
		for _, p := range rp.prefix {
			proxyRte[p] = rp.httpServer.URL
		}
	}

	srv, err := NewRevProxyRouter("127.0.0.1:", proxyRte)
	AssertOk(t, err, "failed to create proxy server")
	defer srv.Stop()

	frontend := srv.httpServer
	log.Infof("front-end %v", frontend.Addr)

	for key, route := range proxyCfg {
		for _, prefix := range route.prefix {
			URL := "http://" + frontend.Addr + prefix
			log.Infof("GET %v", URL)
			resp, err := http.Get(URL)
			AssertOk(t, err, "failed to query %s", URL)
			defer resp.Body.Close()
			data, err := ioutil.ReadAll(resp.Body)
			AssertOk(t, err, "failed to read response %s", URL)
			Assert(t, string(data) == key, "proxy routed %v to wrong process exptreced %v got %v", URL, string(data), key)
		}
	}

	// stop & try
	resp, err := http.Get("http://" + frontend.Addr + "/stop")
	AssertOk(t, err, "failed to stop reverse proxy")
	Assert(t, resp.StatusCode == http.StatusOK, "failed to stop proxy")
	AssertEventually(t, func() (bool, interface{}) {
		if srv.httpServer != nil {
			return false, srv.httpServer
		}
		return true, nil
	}, "failed to stop http server")

}
