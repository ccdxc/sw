package startup

import (
	"context"

	"time"

	"github.com/pensando/sw/api"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/perseus/apiclient"
	"github.com/pensando/sw/venice/perseus/env"
	"github.com/pensando/sw/venice/perseus/services"
	diagsvc "github.com/pensando/sw/venice/utils/diagnostics/service"
	"github.com/pensando/sw/venice/utils/k8s"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const (
	apiServerWaitTime = time.Second
)

func waitForAPIAndStartServices() {
	ii := 0
	for {
		select {
		case <-time.After(apiServerWaitTime):
			ii++
			apic := env.CfgWatcherService.APIClient()
			if apic == nil {
				if ii%10 == 0 {
					log.Errorf("Waiting for Pensando apiserver to come up for %v seconds", ii)
				}

				continue
			}

			opts := api.ListWatchOptions{}
			cl, err := apic.Cluster().List(context.TODO(), &opts)
			if err != nil || len(cl) == 0 {
				if ii%10 == 0 {
					log.Errorf("Waiting for Pensando apiserver to return good Cluster for %v seconds", ii)
				}
				continue
			}
			return
		}
	}
}

func registerDebugHandlers(handler *services.ServiceHandlers) {
	diagSvc := diagsvc.GetDiagnosticsService(globals.APIServer, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, log.GetDefaultInstance())
	if err := diagSvc.RegisterHandler("Debug", diagapi.DiagnosticsRequest_Stats.String(), diagsvc.NewExpVarHandler(globals.APIServer, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, log.GetDefaultInstance())); err != nil {
		log.ErrorLog("method", "GetDiagnosticsServiceWithDefaults", "msg", "failed to register expvar handler", "err", err)
	}
	diagSvc.RegisterCustomAction("list-watchers", func(action string, params map[string]string) (interface{}, error) {
		return handler.HandleDebugAction(action, params)
	})
}

// OnStart restore state and start services as applicable
func OnStart(resolverURLs []string) {
	env.ResolverClient = resolver.New(&resolver.Config{Name: "2", Servers: resolverURLs})
	env.CfgWatcherService = apiclient.NewCfgWatcherService(env.Logger, globals.APIServer)
	ServiceHandlers := services.NewServiceHandlers()
	registerDebugHandlers(ServiceHandlers)
	log.Infof("ServiceHandlers registered %v", ServiceHandlers)
	env.CfgWatcherService.Start()
	go waitForAPIAndStartServices()
}
