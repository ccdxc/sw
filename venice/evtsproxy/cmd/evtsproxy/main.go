// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"os/signal"
	"path/filepath"
	"strings"
	"sync"
	"syscall"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/exporters"
	"github.com/pensando/sw/venice/utils/events/policy"
	"github.com/pensando/sw/venice/utils/k8s"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var retryDelay = time.Second // delay between retries

// configs required to start the services
type config struct {
	grpcListenURL string
	evtsStoreDir  string
	evtsMgrURL    string
	dedupInterval time.Duration
	batchInterval time.Duration
}

// represents the list of services (policy watcher, evtsproxy, etc)
type evtServices struct {
	sync.RWMutex
	config          *config                // evtsproxy config
	resolverClient  resolver.Interface     // resolver client
	eps             *evtsproxy.EventsProxy // events proxy server
	policyWatcher   *policy.Watcher        // policy watcher responsible for watching event policies from evtsmgr; will be nil in host mode
	policyMgr       *policy.Manager        // responsible for creating/deleting syslog exporters for the incoming event policies
	maintenanceMode *bool                  // flag set during
	logger          log.Logger             // logger
	wg              sync.WaitGroup         // for closing rollout watcher
	ctx             context.Context
	cancel          context.CancelFunc
}

// starts all the services required to process events, alerts and policies
func (e *evtServices) start() {
	var err error

	e.wg.Add(1)
	go e.watchRolloutActionObject()

	waitTime := 0
	for { // wait for maintenance mode
		e.RLock()
		if e.maintenanceMode != nil {
			if !(*e.maintenanceMode) {
				e.logger.Info("no upgrade in process, starting fresh")
			}
			e.RUnlock()
			break
		}
		e.RUnlock()

		waitTime++
		if waitTime%10 == 0 {
			e.logger.Info("waiting (%ds) for maintenance mode updates from watcher", waitTime)
		}
		time.Sleep(time.Second)
	}

	nodeName := k8s.GetNodeName()

	// create events proxy
	e.RLock()
	e.logger.Infof("starting evtsproxy with maintenance mode: %v", *e.maintenanceMode)
	e.eps, err = evtsproxy.NewEventsProxy(nodeName, globals.EvtsProxy, e.config.grpcListenURL, nil,
		e.config.dedupInterval, e.config.batchInterval, &events.StoreConfig{Dir: e.config.evtsStoreDir}, e.logger,
		evtsproxy.WithDefaultObjectRef(getDefaultObjectRef(nodeName)), evtsproxy.WithMaintenanceMode(*e.maintenanceMode))
	e.RUnlock()
	if err != nil {
		e.logger.Fatalf("error creating events proxy instance: %v", err)
	}

	if _, err := e.eps.RegisterEventsExporter(exporters.Venice,
		&exporters.VeniceExporterConfig{EvtsMgrURL: e.config.evtsMgrURL}); err != nil {
		log.Fatalf("failed to register venice events exporter with events proxy, err: %v", err)
	}

	// start events policy manager
	e.policyMgr, err = policy.NewManager(utils.GetHostname(), e.eps.GetEventsDispatcher(), e.logger)
	if err != nil {
		log.Fatalf("failed to create event policy manager, err: %v", err)
	}

	// start events policy watcher
	e.policyWatcher, err = policy.NewWatcher(e.policyMgr, e.logger, policy.WithEventsMgrURL(e.config.evtsMgrURL))
	if err != nil {
		log.Fatalf("failed to create events policy watcher, err: %v", err)
	}

	e.eps.StartDispatch()

	e.logger.Infof("%s is running {%+v}", globals.EvtsProxy, e.eps)
}

// stops the services that were started during start()
func (e *evtServices) stop() {
	// stop policy watcher
	e.policyWatcher.Stop()

	// stop policy manager
	e.policyMgr.Stop()

	// stop evts proxy
	if err := e.eps.RPCServer.Stop(); err != nil {
		e.logger.Errorf("failed to stop RPC server, err %v", err)
	}

	e.cancel()
	e.wg.Wait()
}

// helper function that watches rollout action object and sets the maintenance mode accordingly
func (e *evtServices) watchRolloutActionObject() {
	defer e.wg.Done()
	var cl apiclient.Services
	var err error

	for {
		select {
		case <-e.ctx.Done():
			e.logger.Errorf("context closed; rollout action watcher")
			return
		default:
			cl, err = apiclient.NewGrpcAPIClient(globals.EvtsProxy, globals.APIServer, e.logger, rpckit.WithBalancer(balancer.New(e.resolverClient)), rpckit.WithLogger(e.logger))
			if err != nil {
				e.logger.Errorf("failed to create API client {API server URLs from resolver: %v}, err: %v", e.resolverClient.GetURLs(globals.APIServer), err)
				time.Sleep(retryDelay)
				continue
			}

			e.startWatcher(cl)
			cl.Close()
			if e.ctx.Err() != nil {
				return
			}
			time.Sleep(retryDelay)
		}
	}
}

// helper function that starts the rollout action watcher
func (e *evtServices) startWatcher(cl apiclient.Services) {
	var watcher kvstore.Watcher

	ii := 0
rolloutActionWatcher:
	for {
		select {
		case <-e.ctx.Done():
			return
		default:
			rolloutActions, err := cl.RolloutV1().RolloutAction().List(e.ctx, &api.ListWatchOptions{})
			if err != nil {
				e.logger.Errorf("failed to list rollout action, err: %v", err)
				time.Sleep(time.Second)
				continue
			}

			if len(rolloutActions) == 0 { // no rollout action object found
				e.setMaintenanceMode(false)
			}

			if watcher, err = cl.RolloutV1().RolloutAction().Watch(e.ctx, &api.ListWatchOptions{}); err == nil {
				break rolloutActionWatcher
			}

			ii++
			if ii%10 == 0 {
				e.logger.Errorf("waiting for rollout action watch to succeed for %v seconds", ii)

			}
			time.Sleep(time.Second)
		}
	}
	e.logger.Info("rollout action watcher established")

	// handle rollout watcher events
	for {
		select {
		case <-e.ctx.Done():
			return
		case event, ok := <-watcher.EventChan():
			if !ok { // restart
				e.logger.Errorf("error receiving from rollout action watch channel, exiting watcher")
				return
			}

			rolloutAcObj, ok := event.Object.(*rollout.RolloutAction)
			if !ok {
				e.logger.Info("watcher failed to get rollout action object")
				return
			}

			e.logger.Infof("received rollout action watch event: %v, %v", event.Type, rolloutAcObj)
			switch event.Type {
			case kvstore.Created, kvstore.Updated:
				switch rollout.RolloutStatus_RolloutOperationalState(rollout.RolloutStatus_RolloutOperationalState_vvalue[rolloutAcObj.Status.OperationalState]) {
				case rollout.RolloutStatus_PROGRESSING, rollout.RolloutStatus_SUSPEND_IN_PROGRESS:
					e.setMaintenanceMode(true)
				case rollout.RolloutStatus_SUCCESS, rollout.RolloutStatus_SUSPENDED, rollout.RolloutStatus_FAILURE, rollout.RolloutStatus_DEADLINE_EXCEEDED:
					e.setMaintenanceMode(false)
				case rollout.RolloutStatus_SCHEDULED, rollout.RolloutStatus_PRECHECK_IN_PROGRESS:
					e.setMaintenanceMode(false)
				}
			case kvstore.Deleted:
				e.setMaintenanceMode(false)
			}
		}
	}
}

// setMaintenanceMode sets the maintenance mode flag
func (e *evtServices) setMaintenanceMode(flag bool) {
	e.Lock()
	defer e.Unlock()
	if flag {
		if e.maintenanceMode == nil || (e.maintenanceMode != nil && !*e.maintenanceMode) {
			e.logger.Info("upgrade in process, entering maintenance mode")
			if e.eps != nil {
				e.eps.SetMaintenanceMode(flag)
			}
		}
	} else {
		if e.maintenanceMode != nil && *e.maintenanceMode {
			e.logger.Info("upgrade finished, leaving maintenance mode")
			if e.eps != nil {
				e.eps.SetMaintenanceMode(flag)
			}
		}
	}

	e.maintenanceMode = &flag
}

// main (command source) for events proxy
func main() {

	var (
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs of the form 'ip:port'")
		listenURL       = flag.String("listen-url", fmt.Sprintf(":%s", globals.EvtsProxyRPCPort), "RPC listen URL")
		evtsStoreDir    = flag.String("evts-store-dir", globals.EventsDir, "Local events store directory")
		dedupInterval   = flag.Duration("dedup-interval", 10*time.Second, "Events de-duplication interval") // default 24hrs
		batchInterval   = flag.Duration("batch-interval", 10*time.Second, "Events batching interval")       // default 10s
		evtsMgrURL      = flag.String("evts-mgr-url", fmt.Sprintf(":%s", globals.EvtsMgrRPCPort), "RPC listen URL of events manager")
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("log-to-file", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.EvtsProxy)), "Path of the log file")
		logToStdoutFlag = flag.Bool("log-to-stdout", false, "Enable logging to stdout")
	)

	flag.Parse()

	// Fill logger config params
	lConfig := &log.Config{
		Module:      globals.EvtsProxy,
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       *debugflag,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	logger := log.SetConfig(lConfig)
	defer logger.Close()

	ctx, cancel := context.WithCancel(context.Background())

	// start event services
	es := &evtServices{
		config: &config{
			grpcListenURL: *listenURL,
			evtsStoreDir:  *evtsStoreDir,
			dedupInterval: *dedupInterval,
			batchInterval: *batchInterval,
			evtsMgrURL:    *evtsMgrURL,
		},
		resolverClient: resolver.New(&resolver.Config{
			Name:    globals.EvtsMgr,
			Servers: strings.Split(*resolverURLs, ",")}),
		ctx:    ctx,
		cancel: cancel,
		logger: logger,
	}
	es.start()

	// channel to receive signal
	gracefulStop := make(chan os.Signal, 1)
	signal.Notify(gracefulStop, syscall.SIGTERM, syscall.SIGINT)

	select {
	case <-gracefulStop:
		logger.Debug("got signal, exiting")
		es.stop()
	case <-es.eps.RPCServer.Done():
		logger.Debug("server stopped serving, exiting")
		es.stop()
	}
}

// returns the default object ref for events
func getDefaultObjectRef(nodeName string) *api.ObjectRef {
	node := &cluster.Node{}
	node.Defaults("all")
	node.Name = nodeName
	return &api.ObjectRef{
		Kind:      node.GetKind(),
		Name:      node.GetName(),
		Tenant:    node.GetTenant(),
		Namespace: node.GetNamespace(),
		URI:       node.GetSelfLink(),
	}
}
