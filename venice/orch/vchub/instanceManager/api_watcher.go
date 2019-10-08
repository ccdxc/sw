package instanceManager

import (
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// watchOrchestrationConfiguration watches orchestration events from API server
func (instance *InstanceManager) watchOrchestrationConfiguration(apicl apiclient.Services) {
	log.Info("Handling API Server watch")
	instance.waitGrp.Add(1)
	defer instance.waitGrp.Done()

	orchestrationWatcher, err := apicl.OrchestratorV1().Orchestrator().Watch(instance.watchCtx, &api.ListWatchOptions{})
	if err != nil {
		log.Errorf("Failed to start orchestrator event watcher. Err : %v", err)
		return
	}
	defer orchestrationWatcher.Stop()

	for {
		select {
		case evt, ok := <-orchestrationWatcher.EventChan():
			if ok {
				orchConfig := evt.Object.(*orchestration.Orchestrator)
				instance.handleConfigEvent(evt.Type, orchConfig)
			}
		case <-instance.watchCtx.Done():
			log.Info("Exiting watch for orchestration configuration")
			return
		}
	}
}

func (instance *InstanceManager) runApisrvWatcher() {
	instance.waitGrp.Add(1)
	defer instance.waitGrp.Done()
	log.Info("Running API Server Watcher")

	for {
		apicl, err := apiclient.NewGrpcAPIClient(globals.VCHub, instance.apisrvURL, instance.logger, rpckit.WithBalancer(balancer.New(instance.resolver)))
		if err != nil {
			log.Warnf("Failed to connect to gRPC Server [%s]\n", instance.apisrvURL)
		} else {
			log.Infof("Successfully connected to {%v}", apicl)
			instance.watchOrchestrationConfiguration(apicl)
			apicl.Close()
		}

		if instance.stopped() {
			log.Info("Exiting API server watcher")
			return
		}

		log.Error("Lost connection with the API server. Retrying...")
		// Pause for a second before retrying connection with the API server
		time.Sleep(time.Second)
	}
}
