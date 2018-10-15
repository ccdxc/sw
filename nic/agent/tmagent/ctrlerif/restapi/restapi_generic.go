package restapi

import (
	"fmt"
	"net/http"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/httputils"
	//delphi "github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/venice/utils/log"
)

func init() {
	name := "/telemetry/v1/metrics/generic/"
	prefixRoutes[name] = addGenericMetricsAPIRoutes
}

// addGenericMetricsAPIRoutes adds routes for backward compatibility of penctl
func addGenericMetricsAPIRoutes(r *mux.Router, srv *RestServer) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Kind}/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(srv.runGenericMetricsGetHandler))
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Kind}/", httputils.MakeHTTPHandler(srv.runGenericMetricsListHandler))
}

// runGenericMetricsListHandler is the List Handler for backward compatibility of penctl
func (s *RestServer) runGenericMetricsListHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET LIST request in runGenericMetricsListHandler %s", mux.Vars(r)["Meta.Kind"])
	res := fmt.Sprintf("Got GET LIST request in runGenericMetricsListHandler %s", mux.Vars(r)["Meta.Kind"])
	return res, nil
}

// runGenericMetricsGetHandler is the Get Handler for backward compatibility of penctl
func (s *RestServer) runGenericMetricsGetHandler(r *http.Request) (interface{}, error) {
	//o := upgrade.UpgradeMetrics{
	//	Meta: &delphi.ObjectMeta{
	//		Kind: "GenericMetrics",
	//		Key:  mux.Vars(r)["Meta.Name"],
	//	},
	//}
	//o.Meta = new(delphi.ObjectMeta)
	//o.Meta.Kind = "GenericMetrics"
	//o.Meta.Key = mux.Vars(r)["Meta.Name"]
	log.Infof("Got GET request %s/%s/%s", mux.Vars(r)["Meta.Name"], "GenericMetrics", mux.Vars(r)["Meta.Name"])
	res := fmt.Sprintf("Got GET request %s/%s/%s", mux.Vars(r)["Meta.Name"], "GenericMetrics", mux.Vars(r)["Meta.Name"])
	return res, nil
}
