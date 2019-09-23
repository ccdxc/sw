package watcher

import (
	"context"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

func (w *Watcher) handleApisrvWatch(ctx context.Context, apicl apiclient.Services) {
	log.Info("Handling API Server watch")
	w.waitGrp.Add(1)
	defer w.waitGrp.Done()

	for {
		select {
		// TODO : Add case for events coming in
		case <-ctx.Done():
			log.Info("Context done called. Stop handling orchestrator config operations.")
			apicl.Close()
			return
		}
	}
}

func (w *Watcher) runApisrvWatcher(ctx context.Context, apisrvURL string, resolver resolver.Interface) {
	w.waitGrp.Add(1)
	defer w.waitGrp.Done()

	config := log.GetDefaultConfig("VCHubApiWatcher")
	l := log.GetNewLogger(config)
	b := balancer.New(resolver)

	for {
		apicl, err := apiclient.NewGrpcAPIClient(globals.VCHub, apisrvURL, l, rpckit.WithBalancer(b))

		if err != nil {
			log.Warnf("Failed to connect to gRPC Server [%s]\n", apisrvURL)
		} else {
			log.Infof("Successfully connected to {%v}", apicl)
			go w.handleApisrvWatch(ctx, apicl)
			log.Info("API server watcher successfully started")
		}

		if w.stopped() {
			log.Info("Exiting API server watcher")
			return
		}

		log.Error("Lost connection with the API server. Retrying...")
		// Pause for a second before retrying connection with the API server
		time.Sleep(time.Second)
	}
}
