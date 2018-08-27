package apigwpkg

import (
	"encoding/json"
	"fmt"
	"net/http"
	neturl "net/url"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

type stagingBufferID struct{}

func writeError(code int, result, message string, w http.ResponseWriter) {
	status := api.Status{
		TypeMeta: api.TypeMeta{Kind: "Status"},
		Code:     int32(code),
		Result:   api.StatusResult{Str: result},
		Message:  []string{message},
	}
	buf, err := json.MarshalIndent(&status, "", "  ")
	if err != nil {
		buf = []byte("unknown error")
	}
	w.WriteHeader(int(status.Code))
	w.Write(buf)
}

// HandleStaging is the handler for all staging routes. This recirculates the request
//  back to the unstaged path
// Example: /staging/<buffer-id>/network/v1/networks...
func HandleStaging(mux *http.ServeMux) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		url := strings.Split(strings.TrimPrefix(r.URL.Path, "/"), "/")
		if len(url) < 2 {
			writeError(http.StatusNotFound, "invalid path", "", w)
			return
		}
		if url[1] == "tenant" && len(url) < 5 {
			writeError(http.StatusNotFound, "invalid path", "", w)
			return
		}
		var bufferID, tenantID, stripPath string
		ctx := r.Context()
		if url[1] == "tenant" && url[3] == "buffers" {
			// this is a tenant specific path
			bufferID = url[4]
			tenantID = url[2]
			stripPath = "/" + globals.StagingURIPrefix + "/tenant/" + url[2] + "/buffers/" + url[4]
		} else {
			bufferID = url[1]
			stripPath = "/" + globals.StagingURIPrefix + "/" + url[1]
			tenantID = globals.DefaultTenant
		}

		r2 := new(http.Request)
		*r2 = *r
		r2.URL = new(neturl.URL)
		r2.URL.Path = "/" + globals.ConfigURIPrefix + strings.TrimPrefix(r.URL.Path, stripPath)
		log.Infof("HandleStaging got BufferId [%s] tenant [%s] path [%v]", bufferID, tenantID, r2.URL.Path)
		// XXX-TODO(sanjayt): Once RBAC is in place add RBAC resource for buffer
		//  to the context.
		r2.Header.Set(apigw.GrpcMDRequestMethod, r.Method)
		r2.Header.Set(apigw.GrpcMDStagingBufferID, bufferID)
		if tenantID != "" {
			r2.Header.Set(apigw.GrpcMDRequestTenant, tenantID)
		}
		handler, path := mux.Handler(r2)
		if !strings.HasPrefix(r2.URL.Path, path) {
			fmt.Printf("got wrong path [%v]/[%v]", path, r2.URL.Path)
			return
		}
		handler.ServeHTTP(w, r2.WithContext(ctx))
	}
}
