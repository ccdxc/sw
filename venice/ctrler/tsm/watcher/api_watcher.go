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
	mirrorOpts := api.ListWatchOptions{}
	// Watch all updates (spec and status)
	// since resourceVersion is used when doing status updates, must know the correct version#
	// this can be captured on receiving update
	mirrorSessionWatcher, err := apicl.MonitoringV1().MirrorSession().Watch(ctx1, &mirrorOpts)
	if err != nil {
		log.Errorf("Failed to start mirror session watch (%s)\n", err)
		return
	}
	defer mirrorSessionWatcher.Stop()

	tsrOpts := api.ListWatchOptions{
		// We don't care about meta updates and we don't want to get back our own status updates
		FieldChangeSelector: []string{"Spec"},
	}
	techSupportRequestWatcher, err := apicl.MonitoringV1().TechSupportRequest().Watch(ctx1, &tsrOpts)
	if err != nil {
		log.Errorf("Failed to start techSupport request watch (%s)\n", err)
		return
	}
	defer techSupportRequestWatcher.Stop()

	nodeOpts := api.ListWatchOptions{
		FieldChangeSelector: []string{"ObjectMeta.Labels"},
	}
	controllerNodeWatcher, err := apicl.ClusterV1().Node().Watch(ctx1, &nodeOpts)
	if err != nil {
		log.Errorf("Failed to start Controller nodes watch (%s)\n", err)
		return
	}
	defer controllerNodeWatcher.Stop()

	smartNICOpts := api.ListWatchOptions{
		FieldChangeSelector: []string{"Spec.ID", "ObjectMeta.Labels", "Status.AdmissionPhase"},
	}

	// Do not watch for changes in Status.Phase, as NICs that are
	// not admitted should not have any active connection to Venice.
	smartNICNodeWatcher, err := apicl.ClusterV1().DistributedServiceCard().Watch(ctx1, &smartNICOpts)
	if err != nil {
		log.Errorf("Failed to start DistributedServiceCard nodes watch (%s)\n", err)
		return
	}
	defer smartNICNodeWatcher.Stop()

	moduleOpts := api.ListWatchOptions{
		// We don't care about meta updates and we don't want to get back our own status updates
		FieldChangeSelector: []string{"Spec"},
	}
	moduleWatcher, err := apicl.DiagnosticsV1().Module().Watch(ctx1, &moduleOpts)
	if err != nil {
		log.Errorf("Failed to start module watch (%s)\n", err)
		return
	}
	defer moduleWatcher.Stop()

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
			log.Infof("apiwatcher: Got Mirror session  watch event: %v", evt.Type)
			w.statemgr.MirrorSessionWatcher <- *evt

		// TechSupport Events -- receive notifications on 3 separate ApiServer channels
		// (1 per kind) and forward all of them to statemgr using a single channel
		case evt, ok := <-techSupportRequestWatcher.EventChan():
			if !ok {
				log.Errorf("Error receiving from apisrv TechSupportRequest watcher")
				return
			}
			w.statemgr.TechSupportWatcher <- *evt

		case evt, ok := <-controllerNodeWatcher.EventChan():
			if !ok {
				log.Errorf("Error receiving from apisrv ControllerNode watcher")
				return
			}
			w.statemgr.TechSupportWatcher <- *evt

		case evt, ok := <-smartNICNodeWatcher.EventChan():
			if !ok {
				log.Errorf("Error receiving from apisrv DistributedServiceCard watcher")
				return
			}
			w.statemgr.TechSupportWatcher <- *evt

		case evt, ok := <-moduleWatcher.EventChan(): // receive notifications for module events
			if !ok {
				log.Errorf("Error receiving from apisrv Module watcher")
				return
			}
			w.statemgr.DiagnosticsModuleWatcher <- *evt

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

			// purge deleted mirror sessions and techsupport requests if this is reconnect to apiserver.
			msList, err := apicl.MonitoringV1().MirrorSession().List(ctx, &api.ListWatchOptions{})
			if err == nil {
				w.statemgr.PurgeDeletedMirrorSessions(msList)
			}

			// purge deleted nodes and TechSupportRequests if this is reconnect to apiserver.
			tsrList, err := apicl.MonitoringV1().TechSupportRequest().List(ctx, &api.ListWatchOptions{})
			if err == nil {
				w.statemgr.PurgeDeletedTechSupportObjects(tsrList)
			}
			vnList, err := apicl.ClusterV1().Node().List(ctx, &api.ListWatchOptions{})
			if err == nil {
				w.statemgr.PurgeDeletedTechSupportObjects(vnList)
			}
			nnList, err := apicl.ClusterV1().DistributedServiceCard().List(ctx, &api.ListWatchOptions{})
			if err == nil {
				w.statemgr.PurgeDeletedTechSupportObjects(nnList)
			}
			// purge module objects
			moduleList, err := apicl.DiagnosticsV1().Module().List(ctx, &api.ListWatchOptions{})
			if err == nil {
				w.statemgr.PurgeDeletedModuleObjects(moduleList)
			}

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
