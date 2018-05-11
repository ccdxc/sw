// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package watcher

import (
	"context"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// handleApisrvWatch handles api server watch events
func (w *Watcher) handleApisrvWatch(ctx context.Context, apicl apiclient.Services) {
	ctx1, cancel := context.WithCancel(ctx)
	defer cancel()

	// mirror session watcher
	opts := api.ListWatchOptions{}
	// Watch on changes to Spec only.. status updates are triggered by TS controller so no need
	// to see them (unless multiple TSCs are running)
	opts.FieldChangeSelector = []string{"Spec"}
	mirrorSessionWatcher, err := apicl.MirrorSessionV1().MirrorSession().Watch(ctx1, &opts)
	if err != nil {
		log.Errorf("Failed to start mirror session watch (%s)\n", err)
		return
	}
	defer mirrorSessionWatcher.Stop()

	// wait for events
	// api server will send events for all existing mirror session objects as well
	log.Infof("Waiting for events from api-server")

	for {
		select {
		case evt, ok := <-mirrorSessionWatcher.EventChan():
			if !ok {
				log.Errorf("Error receiving from apisrv watcher")
				return
			}
			w.statemgr.MirrorSessionWatcher <- *evt
		case <-ctx1.Done():
			return
		}
	}
}

// runApisrvWatcher run API server watcher forever
func (w *Watcher) runApisrvWatcher(ctx context.Context, apisrvURL string, resolver resolver.Interface) {
	// if we have no URL, exit
	if apisrvURL == "" {
		return
	}

	// setup wait group
	w.waitGrp.Add(1)
	defer w.waitGrp.Done()

	// create logger
	config := log.GetDefaultConfig("TsmApiWatcher")
	l := log.GetNewLogger(config)
	b := balancer.New(resolver)

	// loop forever
	for {
		// create a grpc client
		apicl, err := apiclient.NewGrpcAPIClient(globals.Tsm, apisrvURL, l, rpckit.WithBalancer(b))
		if err != nil {
			log.Warnf("Failed to connect to gRPC server [%s]\n", apisrvURL)
		} else {
			log.Infof("API client connected {%+v}", apicl)

			// handle api server watch events
			w.handleApisrvWatch(ctx, apicl)
			apicl.Close()
		}

		// if stop flag is set, we are done
		if w.stopped() {
			log.Infof("Exiting API server watcher")
			return
		}

		// wait for a second and retry connecting to api server
		time.Sleep(time.Second)
	}
}
