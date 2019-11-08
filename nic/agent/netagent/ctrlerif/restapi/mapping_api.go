package restapi

import (
	"net/http"
	"strconv"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/httputils"
)

// addAPIMappingRoutes adds mapping API routes
func addAPIMappingRoutes(r *mux.Router, srv *RestServer) {
	r.Methods("GET").Subrouter().HandleFunc("/interfaces/{id}", httputils.MakeHTTPHandler(srv.getInterfaceMappingHandler))
}

func (s *RestServer) getInterfaceMappingHandler(r *http.Request) (interface{}, error) {
	var res Response
	id, _ := mux.Vars(r)["id"]
	intfID, err := strconv.ParseInt(id, 10, 64)
	if err != nil {
		res.StatusCode = http.StatusBadRequest
		res.Error = err.Error()
		return res, err
	}
	intf, err := s.agent.GetInterfaceByID(uint64(intfID))

	if err != nil {
		res.StatusCode = http.StatusNotFound
		res.Error = err.Error()
		return res, err
	}

	res.StatusCode = http.StatusOK
	return intf, err
}
