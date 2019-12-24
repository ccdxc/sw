package startup

import (
	"context"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/persues/apiclient"
	"github.com/pensando/sw/venice/persues/env"
	"github.com/pensando/sw/venice/persues/services"
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

// OnStart restore state and start services as applicable
func OnStart(resolverURLs []string) {
	env.ResolverClient = resolver.New(&resolver.Config{Name: "2", Servers: resolverURLs})
	env.CfgWatcherService = apiclient.NewCfgWatcherService(env.Logger, resolverURLs[0])
	ServiceHandlers := services.NewServiceHandlers()
	log.Infof("ServiceHandlers registered %v", ServiceHandlers)
	env.CfgWatcherService.Start()
	go waitForAPIAndStartServices()
}
