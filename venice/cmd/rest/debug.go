package rest

import (
	"encoding/json"
	"expvar"
	"net/http"
	"net/http/pprof"
	"sort"

	"github.com/go-martini/martini"

	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/installer"
	"github.com/pensando/sw/venice/cmd/utils"
	// Import utils/debug pkg to publish runtime stats as part of its pkg init
	_ "github.com/pensando/sw/venice/utils/debug/stats"
	"github.com/pensando/sw/venice/utils/errors"
	"github.com/pensando/sw/venice/utils/log"
)

// constants used by REST interface
const (
	debugPrefix = "/debug"
)

// NewDebugRESTServer creates REST server endpoints for debugging and pprof
func NewDebugRESTServer() *martini.ClassicMartini {
	m := martini.Classic()

	m.Get(uRLPrefix+servicesURL, ServiceListHandler)
	m.Get(uRLPrefix+"/debugSrvUpgrade", DebugUpgradeHandler)
	m.Get(uRLPrefix+"/debugNodeUpgrade", DebugNodeUpgradeHandler)
	m.Get(debugPrefix+expvarURL, expvar.Handler())

	m.Group("/debug/pprof", func(r martini.Router) {
		r.Any("/", pprof.Index)
		r.Any("/cmdline", pprof.Cmdline)
		r.Any("/profile", pprof.Profile)
		r.Any("/symbol", pprof.Symbol)
		r.Any("/trace", pprof.Trace)
		r.Any("/block", pprof.Handler("block").ServeHTTP)
		r.Any("/heap", pprof.Handler("heap").ServeHTTP)
		r.Any("/mutex", pprof.Handler("mutex").ServeHTTP)
		r.Any("/goroutine", pprof.Handler("goroutine").ServeHTTP)
		r.Any("/threadcreate", pprof.Handler("threadcreate").ServeHTTP)
	})

	return m
}

// ServiceListHandler returns the services running in the cluster.
func ServiceListHandler(w http.ResponseWriter, req *http.Request) {
	if env.ResolverService == nil {
		errors.SendNotFound(w, "ServiceList", "")
		return
	}

	encoder := json.NewEncoder(w)

	serviceList := env.ResolverService.List()
	if serviceList != nil {
		for s := range serviceList.Items {
			sort.Slice(serviceList.Items[s].Instances, func(i, j int) bool {
				return serviceList.Items[s].Instances[i].Name < serviceList.Items[s].Instances[j].Name
			})
		}
		sort.Slice(serviceList.Items, func(i, j int) bool {
			return serviceList.Items[i].Name < serviceList.Items[j].Name
		})
	}
	if err := encoder.Encode(serviceList); err != nil {
		log.Errorf("Failed to encode with error: %v", err)
	}
}

// DebugUpgradeHandler is a debug handler during development of upgrade
func DebugUpgradeHandler(w http.ResponseWriter, req *http.Request) {
	// read the file for the updated list of services
	err := env.K8sService.UpgradeServices(utils.GetUpgradeOrder())
	log.Debugf("UpgradeServices returned %s", err)
}

// DebugNodeUpgradeHandler is a debug handler for installing+upgrade of node services
func DebugNodeUpgradeHandler(w http.ResponseWriter, req *http.Request) {

	imageName, err := installer.DownloadImage("ignore")
	log.Infof("DownloadImage returned %s %v", imageName, err)
	err = installer.ExtractImage(imageName)
	log.Infof("extractImage returned %v ", err)
	err = installer.PreLoadImage()
	log.Infof("preLoadImage returned %v ", err)
	err = installer.LoadAndInstallImage()
	log.Infof("loadAndInstallImage returned %v ", err)
}
