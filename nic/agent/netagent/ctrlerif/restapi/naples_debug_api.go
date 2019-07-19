package restapi

import (
	"net/http"
	"net/http/pprof"

	debugStats "github.com/pensando/sw/venice/utils/debug/stats"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/httputils"
)

// addSystemDebugRoutes adds App routes
func addSystemDebugRoutes(r *mux.Router, srv *RestServer) {
	r.Methods("GET").Subrouter().HandleFunc("/", pprof.Index)
	r.Methods("GET").Subrouter().HandleFunc("/cmdline", pprof.Cmdline)
	r.Methods("GET").Subrouter().HandleFunc("/profile", pprof.Profile)
	r.Methods("GET").Subrouter().HandleFunc("/symbol", pprof.Symbol)
	r.Methods("GET").Subrouter().HandleFunc("/trace", pprof.Trace)
	r.Methods("GET").Subrouter().HandleFunc("/allocs", pprof.Handler("allocs").ServeHTTP)
	r.Methods("GET").Subrouter().HandleFunc("/block", pprof.Handler("block").ServeHTTP)
	r.Methods("GET").Subrouter().HandleFunc("/heap", pprof.Handler("heap").ServeHTTP)
	r.Methods("GET").Subrouter().HandleFunc("/mutex", pprof.Handler("mutex").ServeHTTP)
	r.Methods("GET").Subrouter().HandleFunc("/goroutine", pprof.Handler("goroutine").ServeHTTP)
	r.Methods("GET").Subrouter().HandleFunc("/threadcreate", pprof.Handler("threadcreate").ServeHTTP)
	r.Methods("GET").Subrouter().HandleFunc("/uptime", httputils.MakeHTTPHandler(srv.getNetAgentUptime))
	r.Methods("DELETE").Subrouter().Handle("/", debugStats.ClearHandler())

}

func (s *RestServer) getNetAgentUptime(r *http.Request) (interface{}, error) {
	return s.agent.GetNetagentUptime()
}
