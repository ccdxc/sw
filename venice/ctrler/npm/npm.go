// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// Package npm contains network policy manager code
/************ Architecture of NetCtrler ******************
 *
 *    +-----------------------+
 *    |   API Server          |
 *    +-----------------------+
 *        |             ^
 *        V             |
 *    +-----------------------+
 *    |     Controller kit    |
 *  +-+-----------------------+-------------+
 *  |        Network State Manager          |
 *  | +---------+ +----------+ +----------+ |
 *  | | Network | | Endpoint | | SecGroup | |
 *  | +---------+ +----------+ +----------+ |
 *  +---------------------------------------+
 *  |            Nimbus Server              |
 *  +---------------------------------------+
 *       ^           ^            ^
 *       |           |            |
 *       V           V            V
 *  +--------+   +--------+   +--------+
 *  | Agent  |   | Agent  |   | Agent  |
 *  | Node 1 |   | Node 2 |   | Node 3 |
 *  +--------+   +--------+   +--------+
 *
 *********************************************************/
package npm

import (
	"context"
	"encoding/json"
	"expvar"
	"fmt"
	"net/http"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"

	"github.com/gorilla/mux"

	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/nic/agent/protos/generated/nimbus"
	"github.com/pensando/sw/venice/ctrler/npm/statemgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	debugStats "github.com/pensando/sw/venice/utils/debug/stats"
	diagproto "github.com/pensando/sw/venice/utils/diagnostics/protos"
	diagsvc "github.com/pensando/sw/venice/utils/diagnostics/service"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// Netctrler is a netctrler instance
type Netctrler struct {
	StateMgr   *statemgr.Statemgr // state manager
	mserver    *nimbus.MbusServer // nimbus server
	debugStats *debugStats.Stats
	rpcServer  *rpckit.RPCServer
}

// NewNetctrler returns a controller instance
func NewNetctrler(serverURL, restURL, apisrvURL string, resolver resolver.Interface, logger log.Logger, enableDiagnostics bool) (*Netctrler, error) {

	// init tsdb client
	tsdbOpts := &tsdb.Opts{
		ClientName:              globals.Npm,
		ResolverClient:          resolver,
		Collector:               globals.Collector,
		DBName:                  "default",
		SendInterval:            time.Duration(30) * time.Second,
		ConnectionRetryInterval: 100 * time.Millisecond,
		StartLocalServer:        true,
	}
	tsdb.Init(context.Background(), tsdbOpts)

	// create an RPC server
	rpcServer, err := rpckit.NewRPCServer(globals.Npm, serverURL)
	if err != nil {
		logger.Fatalf("Error creating rpc server. Err; %v", err)
	}
	// create nimbus server
	msrv := nimbus.NewMbusServer(globals.Npm, rpcServer)
	var diagSvc *diagHandler
	var options []statemgr.Option
	if enableDiagnostics {
		diagSvc = &diagHandler{}
		options = append(options,
			statemgr.WithDiagnosticsHandler("Debug", diagapi.DiagnosticsRequest_Log.String(), diagsvc.NewElasticLogsHandler(globals.Npm, utils.GetHostname(), diagapi.ModuleStatus_Venice, resolver, logger)),
			statemgr.WithDiagnosticsHandler("Debug", diagapi.DiagnosticsRequest_Stats.String(), diagsvc.NewExpVarHandler(globals.Npm, utils.GetHostname(), diagapi.ModuleStatus_Venice, logger)),
			statemgr.WithDiagnosticsHandler("Debug", diagapi.DiagnosticsRequest_Action.String(), diagSvc))

	}
	// create network state manager
	stateMgr := statemgr.MustGetStatemgr()

	// TODO: Update once featuresflag api is implemented
	err = stateMgr.Run(rpcServer, apisrvURL, resolver, msrv, logger, options...)
	if err != nil {
		logger.Errorf("Could not create network manager. Err: %v", err)
		return nil, err
	}

	logger.Infof("RPC server is running at %v", serverURL)

	// create the controller instance
	ctrler := Netctrler{
		StateMgr:  stateMgr,
		mserver:   msrv,
		rpcServer: rpcServer,
	}
	if diagSvc != nil {
		diagSvc.stateMgr = stateMgr
		diagSvc.mserver = msrv
	}
	// start the RPC server
	rpcServer.Start()

	// start debug REST server
	if restURL != "" {
		ctrler.runDebugRESTServer(restURL)
	}

	return &ctrler, nil
}

// Stop server and release resources
func (c *Netctrler) Stop() error {
	log.Infof("========= NPM Stop called")
	if c.rpcServer != nil {
		log.Infof("========== RPC Server Stop called")
		c.rpcServer.Stop()
	}

	log.Infof("StateManager stop called")
	c.StateMgr.Stop()
	return nil
}

// StopAppWatch stop app watch
func (c *Netctrler) StopAppWatch() error {
	c.StateMgr.StopAppWatch()
	return nil
}

// StopNetworkSecurityPolicyWatch stop  security policy watch
func (c *Netctrler) StopNetworkSecurityPolicyWatch() error {
	c.StateMgr.StopNetworkSecurityPolicyWatch()
	return nil
}

// StartAppWatch stop app watch
func (c *Netctrler) StartAppWatch() error {
	c.StateMgr.StartAppWatch()
	return nil
}

// StartNetworkSecurityPolicyWatch stop  security policy watch
func (c *Netctrler) StartNetworkSecurityPolicyWatch() error {
	c.StateMgr.StartNetworkSecurityPolicyWatch()
	return nil
}

// EnableSelectivePushForKind enable selective push for kind
func (c *Netctrler) EnableSelectivePushForKind(kind string) error {
	return c.StateMgr.EnableSelectivePushForKind(kind)
}

// DisableSelectivePushForKind disable selective push for kind
func (c *Netctrler) DisableSelectivePushForKind(kind string) error {
	return c.StateMgr.DisableSelectivePushForKind(kind)
}

// runDebugRESTServer starts REST server for debug purposes
func (c *Netctrler) runDebugRESTServer(restURL string) error {
	// start a debug http server
	router := mux.NewRouter()
	router.Methods("GET").Subrouter().Handle("/debug/vars", expvar.Handler())
	router.Methods("GET").Subrouter().Handle("/nimbus/db/", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		jData, err := c.mserver.DumpDatabase()
		if err != nil {
			log.Errorf("Error dumping nimbus server database. Err: %v", err)
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}
		w.Header().Set("Content-Type", "application/json")
		w.Write(jData)
	}))
	router.Methods("GET").Subrouter().Handle("/ctkit/db/{kind}/", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		vars := mux.Vars(r)
		kind, ok := vars["kind"]
		if !ok {
			http.Error(w, fmt.Sprintf("kind not specified"), http.StatusInternalServerError)
			return
		}
		objlist := c.StateMgr.ListObjects(kind)
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusOK)
		json.NewEncoder(w).Encode(objlist)
	}))

	// start the server
	go http.ListenAndServe(restURL, router)

	return nil
}

type diagHandler struct {
	stateMgr *statemgr.Statemgr
	mserver  *nimbus.MbusServer
}

// HandleRequest processes diagnostic query specified in the request
func (d *diagHandler) HandleRequest(ctx context.Context, req *diagapi.DiagnosticsRequest) (*api.Any, error) {
	params := req.Parameters
	action, ok := params["action"]
	if !ok {
		return nil, fmt.Errorf("action not specified")
	}
	ret := &diagproto.String{}
	switch action {
	case "list-objects":
		kind, ok := params["kind"]
		if !ok {
			ret.Content = "kind was not specified"
		} else {
			objs := d.stateMgr.ListObjects(kind)
			str, err := json.Marshal(objs)
			if err != nil {
				ret.Content = fmt.Sprintf("marshall returned error (%s)", err)
			} else {
				ret.Content = strings.Replace(string(str), "\\\"", "\"", -1)
			}
		}
	case "dump-nimbus-db":
		ddb, err := d.mserver.DumpDatabase()
		if err == nil {
			ret.Content = strings.Replace(string(ddb), "\\\"", "\"", -1)
		} else {
			ret.Content = fmt.Sprintf("dump db returned error (%s)", err)
		}
	case "config-status":
		cfgPushStatus := d.stateMgr.GetConfigPushStatus()
		str, err := json.Marshal(cfgPushStatus)
		if err != nil {
			ret.Content = fmt.Sprintf("marshall returned error (%s)", err)
		} else {
			ret.Content = strings.Replace(string(str), "\\\"", "\"", -1)
		}

	case "watch-db":
		kind, ok := params["kind"]
		if !ok {
			ret.Content = "kind was not specified"
		} else {
			objs := d.stateMgr.GetDBWatchStatus(kind)
			str, err := json.Marshal(objs)
			if err != nil {
				ret.Content = fmt.Sprintf("marshall returned error (%s)", err)
			} else {
				ret.Content = strings.Replace(string(str), "\\\"", "\"", -1)
			}
		}

	case "config-stats":
		cfgPushStats := d.stateMgr.GetConfigPushStats()
		str, err := json.Marshal(cfgPushStats)
		if err != nil {
			ret.Content = fmt.Sprintf("marshall returned error (%s)", err)
		} else {
			ret.Content = strings.Replace(string(str), "\\\"", "\"", -1)
		}

	case "reset-stats":
		d.stateMgr.ResetConfigPushStats()
		ret.Content = fmt.Sprintf("Reset complete")

	default:
		ret.Content = fmt.Sprintf("Unknown action [%v]. valid actions (list-objects, dump-nimbus-db)", action)
	}
	anyObj, err := types.MarshalAny(ret)
	if err != nil {
		return nil, err
	}
	return &api.Any{Any: *anyObj}, nil
}

// Start initializes the handler. If the handler is already started it should be a no-op
func (d *diagHandler) Start() error {
	return nil
}

// Stop stops the handler. If the handler is already stopped it should be a no-op
func (d *diagHandler) Stop() {}
