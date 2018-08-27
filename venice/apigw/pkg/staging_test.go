package apigwpkg

import (
	"net/http"
	"net/http/httptest"
	"testing"

	"github.com/pensando/sw/venice/apigw"
)

func TestHandleStaging(t *testing.T) {
	expTenant := "default"
	expPath := "/test/uri"
	expBufID := "TestBuffer1"
	count := 0
	rec := httptest.NewRecorder()
	mux := http.NewServeMux()
	handleFunc := func(w http.ResponseWriter, r *http.Request) {
		count++
		ten := r.Header.Get(apigw.GrpcMDRequestTenant)
		if ten != expTenant {
			t.Fatalf("failed to get the right tenant, got [%v]", ten)
		}
		uri := r.URL.Path
		if uri != expPath {
			t.Fatalf("failed to get the right path, got [%v]", uri)
		}
		bufid := r.Header.Get(apigw.GrpcMDStagingBufferID)
		if bufid != expBufID {
			t.Fatalf("failed to retrieve the right buffer ID, got [%v]", bufid)
		}
	}
	mux.HandleFunc("/configs/test/uri", handleFunc)

	handler := HandleStaging(mux)

	cases := []struct {
		name, reqPath, recircPath, tenant, buffer string
		bad                                       bool
	}{
		{"one", "/staging/tenant/default/buffers/XYZ/test/uri", "/configs/test/uri", "default", "XYZ", false},
		{"two", "/staging/XYZ/test/uri", "/configs/test/uri", "default", "XYZ", false},
		{"three", "/staging/tenant/nondefault/buffers/XYZ/test/uri", "/configs/test/uri", "nondefault", "XYZ", false},
		{"four", "/staging/tenant/XYZ/test/uri", "/configs/test/uri", "nondefault", "XYZ", true},
		{"five", "/staging/tenant/test/uri", "/configs/test/uri", "default", "tenant", true},
	}
	for _, c := range cases {
		prev := count
		expTenant = c.tenant
		expPath = c.recircPath
		expBufID = c.buffer
		req, err := http.NewRequest("GET", c.reqPath, nil)
		if err != nil {
			t.Fatalf("[%v] could not generate request(%s)", c.name, err)
		}
		handler(rec, req)
		if c.bad == false && count != prev+1 {
			t.Fatalf("[%v] did not get called", c.name)
		}
	}

}
