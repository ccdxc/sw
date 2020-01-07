// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
// watch and process telemetry policies

package tpm

import (
	"context"
	"expvar"
	"fmt"
	"net/http"
	"net/http/pprof"
	"reflect"
	"time"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/httputils"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	telemetry "github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/ctrler/tpm/rpcserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/debug/stats"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
	"github.com/pensando/sw/venice/utils/kvstore"
	vLog "github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Option fills the optional params for PolicyManager
type Option func(*PolicyManager)

// PolicyManager policy manager global info
type PolicyManager struct {
	// api client
	apiClient apiclient.Services
	// metric client
	metricClient metric.MetricApiClient
	// cancel watch operation
	cancel context.CancelFunc
	// name server
	nsClient resolver.Interface
	// debug stats
	debugStats *debugStats.Stats
	// policyDB
	policyDb *memdb.Memdb
	// tpm rpc server
	rpcServer *rpcserver.PolicyRPCServer
	// rest server
	restServer    *http.Server
	diagSvc       diagnostics.Service
	moduleWatcher module.Watcher
}

// WithDiagnosticsService passes a custom diagnostics service
func WithDiagnosticsService(diagSvc diagnostics.Service) Option {
	return func(pm *PolicyManager) {
		pm.diagSvc = diagSvc
	}
}

// WithModuleWatcher passes a module watcher
func WithModuleWatcher(moduleWatcher module.Watcher) Option {
	return func(pm *PolicyManager) {
		pm.moduleWatcher = moduleWatcher
	}
}

const pkgName = "tpm"
const maxRetry = 15

// stats collection interval
const statsCollectionInterval = "30s"

var pmLog vLog.Logger

// NewPolicyManager creates a policy manager instance
func NewPolicyManager(listenURL string, nsClient resolver.Interface, restURL string, opts ...Option) (*PolicyManager, error) {

	pmLog = vLog.WithContext("pkg", pkgName)
	pm := &PolicyManager{nsClient: nsClient,
		policyDb: memdb.NewMemdb()}
	for _, opt := range opts {
		if opt != nil {
			opt(pm)
		}
	}

	go pm.HandleEvents()
	server, err := rpcserver.NewRPCServer(listenURL, pm.policyDb, statsCollectionInterval, pm.diagSvc)
	if err != nil {
		pmLog.Fatalf("failed to create rpc server, %s", err)
	}

	pm.rpcServer = server

	router := mux.NewRouter()
	router.Methods("GET").Subrouter().Handle("/debug/state", httputils.MakeHTTPHandler(pm.Debug))
	router.Methods("GET").Subrouter().Handle("/debug/vars", expvar.Handler())

	// pprof
	router.Methods("GET").Subrouter().Handle("/debug/vars", expvar.Handler())
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/", pprof.Index)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/cmdline", pprof.Cmdline)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/profile", pprof.Profile)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/symbol", pprof.Symbol)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/trace", pprof.Trace)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/allocs", pprof.Handler("allocs").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/block", pprof.Handler("block").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/heap", pprof.Handler("heap").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/mutex", pprof.Handler("mutex").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/goroutine", pprof.Handler("goroutine").ServeHTTP)
	router.Methods("GET").Subrouter().HandleFunc("/debug/pprof/threadcreate", pprof.Handler("threadcreate").ServeHTTP)

	pm.restServer = &http.Server{Addr: restURL, Handler: router}
	go pm.restServer.ListenAndServe()
	return pm, nil
}

// Stop shutdown policy watch
func (pm *PolicyManager) Stop() {
	if pm.restServer != nil {
		pm.restServer.Close()
	}

	if pm.moduleWatcher != nil {
		pm.moduleWatcher.Stop()
	}

	if pm.diagSvc != nil {
		pm.diagSvc.Stop()
	}
	if pm.rpcServer != nil {
		pm.rpcServer.Stop()
	}
	pm.cancel()
}

// HandleEvents handles policy events
func (pm *PolicyManager) HandleEvents() error {
	ctx, cancel := context.WithCancel(context.Background())
	pm.cancel = cancel

	defer pm.cancel()

	for {
		pm.apiClient = pm.initAPIGrpcClient(globals.APIServer)
		pmLog.Infof("connected to {%s}", globals.APIServer)

		pm.metricClient = pm.initMetricGrpcClient(globals.Collector)
		pmLog.Infof("connected to {%s}", globals.Collector)

		pm.processEvents(ctx)

		// close rpc service
		pm.apiClient.Close()

		// context canceled, return
		if err := ctx.Err(); err != nil {
			pmLog.Warnf("policy watcher context error: %v, exit", err)
			return nil
		}
		time.Sleep(2 * time.Second)
	}
}

// init api grpc client
func (pm *PolicyManager) initAPIGrpcClient(serviceName string) apiclient.Services {
	for {
		for i := 0; i < maxRetry; i++ {
			// create a grpc client
			client, apiErr := apiclient.NewGrpcAPIClient(globals.Tpm, serviceName, vLog.WithContext("pkg", "TPM-GRPC-API"),
				rpckit.WithBalancer(balancer.New(pm.nsClient)))
			if apiErr == nil {
				return client
			}
			pmLog.Warnf("failed to connect to {%s}, error: %s, retry", serviceName, apiErr)
			time.Sleep(2 * time.Second)
		}
	}
}

// init metric grpc client
func (pm *PolicyManager) initMetricGrpcClient(serviceName string) metric.MetricApiClient {
	for {
		for i := 0; i < maxRetry; i++ {
			// create a grpc client
			client, err := rpckit.NewRPCClient(globals.Tpm, serviceName,
				rpckit.WithBalancer(balancer.New(pm.nsClient)))
			if err == nil {
				return metric.NewMetricApiClient(client.ClientConn)
			}

			pmLog.Warnf("failed to connect to {%s}, error: %s, retry", serviceName, err)
			time.Sleep(2 * time.Second)
		}
	}
}

// processEvents watch & process telemetry policy events
func (pm *PolicyManager) processEvents(parentCtx context.Context) error {

	ctx, cancelWatch := context.WithCancel(parentCtx)
	// stop all watch channels
	defer cancelWatch()

	watchList := map[int]string{}
	opts := api.ListWatchOptions{FieldChangeSelector: []string{"Spec"}}
	selCases := []reflect.SelectCase{}

	// stats
	watcher, err := pm.apiClient.MonitoringV1().StatsPolicy().Watch(ctx, &opts)
	if err != nil {
		pmLog.Errorf("failed to watch stats policy, error: {%s}", err)
		return err
	}

	watchList[len(selCases)] = "stats"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(watcher.EventChan())})

	// fwlog
	watcher, err = pm.apiClient.MonitoringV1().FwlogPolicy().Watch(ctx, &opts)
	if err != nil {
		pmLog.Errorf("failed to watch fwlog policy, error: {%s}", err)
		return err
	}

	watchList[len(selCases)] = "fwlog"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(watcher.EventChan())})

	// export
	watcher, err = pm.apiClient.MonitoringV1().FlowExportPolicy().Watch(ctx, &opts)
	if err != nil {
		pmLog.Errorf("failed to watch export policy, error: {%s}", err)
		return err
	}

	watchList[len(selCases)] = "export"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(watcher.EventChan())})

	// watch tenants
	watcher, err = pm.apiClient.ClusterV1().Tenant().Watch(ctx, &opts)
	if err != nil {
		pmLog.Errorf("failed to watch tenant, error: {%s}", err)
		return err
	}

	watchList[len(selCases)] = "tenant"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(watcher.EventChan())})

	// ctx done
	watchList[len(selCases)] = "ctx-canceled"
	selCases = append(selCases, reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(ctx.Done())})

	// event loop
	for {
		id, recVal, ok := reflect.Select(selCases)
		if !ok {
			pmLog.Errorf("{%s} channel closed", watchList[id])
			return fmt.Errorf("channel closed")
		}

		event, ok := recVal.Interface().(*kvstore.WatchEvent)
		if !ok {
			pmLog.Errorf("unknon policy object received from {%s}: %+v", watchList[id], recVal.Interface())
			return fmt.Errorf("unknown policy")
		}

		pmLog.Infof("received event %#v", event)

		switch polObj := event.Object.(type) {
		case *telemetry.StatsPolicy:
			pm.processStatsPolicy(event.Type, polObj)

		case *telemetry.FlowExportPolicy:
			if err := pm.processExportPolicy(event.Type, polObj); err != nil {
				pmLog.Errorf("failed to process flow export policy, %v", err)
			}

		case *telemetry.FwlogPolicy:
			if err := pm.processFwlogPolicy(event.Type, polObj); err != nil {
				pmLog.Errorf("failed to process fwlog policy, %v", err)
			}

		case *cluster.Tenant:
			if err := pm.processTenants(ctx, event.Type, polObj); err != nil {
				pmLog.Errorf("failed to process tenant, %v", err)
			}

		default:
			pmLog.Errorf("invalid event type received from {%s}, %+v", watchList[id], event)
			return fmt.Errorf("invalid event type")
		}
	}
}

// process stats policy
func (pm *PolicyManager) processStatsPolicy(eventType kvstore.WatchEventType, policy *telemetry.StatsPolicy) error {
	pmLog.Infof("process stats policy event:{%s} {%#v} ", eventType, policy)

	switch eventType {
	case kvstore.Created:
		return pm.policyDb.AddObject(policy)

	case kvstore.Updated:
		return pm.policyDb.UpdateObject(policy)

	case kvstore.Deleted:
		return pm.policyDb.DeleteObject(policy)

	default:
		pmLog.Errorf("invalid stats event, type %s policy %+v", eventType, policy)
		return fmt.Errorf("invalid event")
	}
}

// process fwlog policy
func (pm *PolicyManager) processFwlogPolicy(eventType kvstore.WatchEventType, policy *telemetry.FwlogPolicy) error {
	pmLog.Infof("process fwlog policy event:{%s} {%#v} ", eventType, policy)

	switch eventType {
	case kvstore.Created:
		return pm.policyDb.AddObject(policy)

	case kvstore.Updated:
		return pm.policyDb.UpdateObject(policy)

	case kvstore.Deleted:
		return pm.policyDb.DeleteObject(policy)

	default:
		pmLog.Errorf("invalid fwlog event, type %s policy %+v", eventType, policy)
		return fmt.Errorf("invalid fwlog event")
	}
}

// process export policy
func (pm *PolicyManager) processExportPolicy(eventType kvstore.WatchEventType, policy *telemetry.FlowExportPolicy) error {
	pmLog.Infof("process export policy event:{%v} {%#v} ", eventType, policy)

	switch eventType {
	case kvstore.Created:
		return pm.policyDb.AddObject(policy)

	case kvstore.Updated:
		return pm.policyDb.UpdateObject(policy)

	case kvstore.Deleted:
		return pm.policyDb.DeleteObject(policy)

	default:
		pmLog.Errorf("invalid export event, type %s policy %+v", eventType, policy)
		return fmt.Errorf("invalid export event")
	}
}

// process tenants
func (pm *PolicyManager) processTenants(ctx context.Context, eventType kvstore.WatchEventType, tenant *cluster.Tenant) error {
	pmLog.Infof("process tenant event:{%v} {%#v} ", eventType, tenant)

	switch eventType {
	case kvstore.Created:
		// create database request
		databaseReq := &metric.DatabaseReq{
			DatabaseName: tenant.GetName(),
		}

		if _, err := pm.metricClient.CreateDatabase(ctx, databaseReq); err != nil {
			pmLog.Errorf("failed to create database for tenant %s, error: %s", tenant.GetName(), err)
			return err
		}

	case kvstore.Updated: // no-op

	case kvstore.Deleted:
	default:
		return fmt.Errorf("invalid tenant event type %s", eventType)
	}

	return nil
}

// create/update database in Citadel
func (pm *PolicyManager) updateDatabase(tenantName string, dbName string,
	retentionTime string, downSampleRetention string) error {
	pmLog.Infof("update db tenant: %s, db: %s, retention: %s, dretention: %s",
		tenantName, dbName, retentionTime, downSampleRetention)
	return nil
}

// delete database in Citadel
func (pm *PolicyManager) deleteDatabase(tenantName string, dbName string) error {
	pmLog.Infof("delete db tenant: %s, db: %s, retention: %s, dretention: %s",
		tenantName, dbName)
	return nil
}

// Debug function dumps all policy cache for debug
func (pm *PolicyManager) Debug(r *http.Request) (interface{}, error) {
	kind := []string{"StatsPolicy", "FwlogPolicy", "FlowExportPolicy"}
	dbgInfo := struct {
		Policy  map[string]map[string]memdb.Object
		Clients map[string]map[string]string
	}{
		Policy:  map[string]map[string]memdb.Object{},
		Clients: map[string]map[string]string{},
	}

	for _, key := range kind {
		dbgInfo.Policy[key] = map[string]memdb.Object{}

		pl := pm.policyDb.ListObjects(key, nil)
		for _, p := range pl {
			dbgInfo.Policy[key][p.GetObjectMeta().GetName()] = p
		}
	}
	dbgInfo.Clients = pm.rpcServer.Debug()
	return dbgInfo, nil
}
