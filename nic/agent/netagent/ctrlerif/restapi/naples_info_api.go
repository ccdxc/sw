package restapi

import (
	"net/http"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/httputils"
)

// addSystemStatusAPIRoutes adds App routes
func addNaplesInfoAPIRoutes(r *mux.Router, srv *RestServer) {
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(srv.getNaplesInfoHandler))
}

func (s *RestServer) getNaplesInfoHandler(r *http.Request) (interface{}, error) {
	return s.agent.GetNaplesInfo()
}
