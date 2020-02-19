package orchhub

import (
	"fmt"

	"github.com/pensando/sw/venice/utils/k8s"

	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	instanceManager "github.com/pensando/sw/venice/ctrler/orchhub/instancemanager"
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
	// Comma separated list of vc urls
	VcList string
	// URL to listen on
	ListenURL string
	// Logger to be used for logging.
	Logger log.Logger
	// DebugMode enables verbose logging and stack trace dump support.
	DebugMode bool
	// Resolver to use
	Resolver resolver.Interface
}

// OrchCtrler specifies the structure
type OrchCtrler struct {
	StateMgr    *statemgr.Statemgr // state manager
	rpcServer   *rpcserver.OrchServer
	instanceMgr *instanceManager.InstanceManager
}

// NewOrchCtrler creates an OrchCtrler
func NewOrchCtrler(opts Opts) (*OrchCtrler, error) {
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
	stateMgr, err := statemgr.NewStatemgr(globals.APIServer, opts.Resolver, opts.Logger, instanceMgrCh)
	if err != nil {
		opts.Logger.Errorf("Failed to create state manager. Err: %v", err)
		return nil, err
	}

	instance, err := instanceManager.NewInstanceManager(stateMgr, opts.VcList, opts.Logger, instanceMgrCh)
	if instance == nil || err != nil {
		opts.Logger.Errorf("Failed to create instance manager. Err : %v", err)
	}

	instance.Start()
	opts.Logger.Infof("Instance manager is running")
	ctrler := &OrchCtrler{
		StateMgr:    stateMgr,
		rpcServer:   server,
		instanceMgr: instance,
	}
	return ctrler, nil
}

// Stop stops orch hub
func (o *OrchCtrler) Stop() {
	o.rpcServer.StopServer()
	o.StateMgr.Controller().Stop()
	o.instanceMgr.Stop()
}
