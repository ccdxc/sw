package orchhub

import (
	"expvar"
	"fmt"
	"net/http"
	"net/http/pprof"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/venice/utils/k8s"

	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	instanceManager "github.com/pensando/sw/venice/ctrler/orchhub/instancemanager"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub"
	"github.com/pensando/sw/venice/ctrler/orchhub/rpcserver"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
	diagsvc "github.com/pensando/sw/venice/utils/diagnostics/service"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const configWatcherQueueLen = 32

// Opts specifies the config for OrchHub
type Opts struct {
	// URL to listen on
	ListenURL string
	// Logger to be used for logging.
	Logger log.Logger
	// DebugMode enables verbose logging and stack trace dump support.
	DebugMode bool
	// Resolver to use
	Resolver  resolver.Interface
	VCHubOpts []vchub.Option
}

// OrchCtrler specifies the structure
type OrchCtrler struct {
	StateMgr    *statemgr.Statemgr // state manager
	RPCServer   *rpcserver.OrchServer
	InstanceMgr *instanceManager.InstanceManager
	restServer  *http.Server
}

// NewOrchCtrler creates an OrchCtrler
func NewOrchCtrler(opts Opts) (*OrchCtrler, error) {
	opts.Logger.Infof("OrchCtrler starting...")
	// start module watcher
	moduleChangeCb := func(diagmod *diagapi.Module) {
		opts.Logger.ResetFilter(diagnostics.GetLogFilter(diagmod.Spec.LogLevel))
		opts.Logger.InfoLog("method", "moduleChangeCb", "msg", "setting log level", "moduleLogLevel", diagmod.Spec.LogLevel)
	}
	watcherOption := rpcserver.WithModuleWatcher(module.GetWatcher(fmt.Sprintf("%s-%s", k8s.GetNodeName(), globals.OrchHub), globals.APIServer, opts.Resolver, opts.Logger, moduleChangeCb))

	// add diagnostics service
	diagOption := rpcserver.WithDiagnosticsService(diagsvc.GetDiagnosticsServiceWithDefaults(globals.OrchHub, k8s.GetNodeName(), diagapi.ModuleStatus_Venice, opts.Resolver, opts.Logger))

	// Start grpc server
	server, err := rpcserver.NewOrchServer(opts.ListenURL, diagOption, watcherOption)
	if err != nil {
		opts.Logger.Errorf("OrchServer start failed %v", err)
		return nil, err
	}

	instanceMgrCh := make(chan *kvstore.WatchEvent, configWatcherQueueLen)
	ctkitReconnectCh := make(chan string, configWatcherQueueLen)
	stateMgr, err := statemgr.NewStatemgr(globals.APIServer, opts.Resolver, opts.Logger, instanceMgrCh, ctkitReconnectCh)
	if err != nil {
		opts.Logger.Errorf("Failed to create state manager. Err: %v", err)
		return nil, err
	}

	instance, err := instanceManager.NewInstanceManager(stateMgr, opts.Logger, instanceMgrCh, ctkitReconnectCh, opts.VCHubOpts)
	if instance == nil || err != nil {
		opts.Logger.Errorf("Failed to create instance manager. Err : %v", err)
	}

	instance.Start()
	opts.Logger.Infof("Instance manager is running")
	ctrler := &OrchCtrler{
		StateMgr:    stateMgr,
		RPCServer:   server,
		InstanceMgr: instance,
	}

	router := mux.NewRouter()
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
	ctrler.restServer = &http.Server{Addr: fmt.Sprintf("127.0.0.1:%s", globals.OrchHubRESTPort), Handler: router}

	go ctrler.restServer.ListenAndServe()
	return ctrler, nil
}

// Stop stops orch hub
func (o *OrchCtrler) Stop() {
	o.RPCServer.StopServer()
	o.StateMgr.Controller().Stop()
	o.InstanceMgr.Stop()
}
