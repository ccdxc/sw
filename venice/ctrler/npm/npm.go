// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// Package npm contains network policy manager code
/************ Architecture of NetCtrler ******************
 *
 *    +-----------------------+
 *    |   API Server          |
 *    +-----------------------+
 *        |             ^
 *        V             |
 *    +---------+    +--------+
 *    | Watcher |    | Writer |
 *  +---------------------------------------+
 *  |        Network State Manager          |
 *  | +---------+ +----------+ +----------+ |
 *  | | Network | | Endpoint | | SecGroup | |
 *  | +---------+ +----------+ +----------+ |
 *  +---------------------------------------+
 *  |               RPC Server              |
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
	"net/http"

	"github.com/pensando/sw/venice/ctrler/npm/rpcserver"
	"github.com/pensando/sw/venice/ctrler/npm/statemgr"
	"github.com/pensando/sw/venice/ctrler/npm/watcher"
	"github.com/pensando/sw/venice/ctrler/npm/writer"
	"github.com/pensando/sw/venice/utils/debug"
	"github.com/pensando/sw/venice/utils/log"
)

// Netctrler is a netctrler instance
type Netctrler struct {
	StateMgr   *statemgr.Statemgr   // state manager
	Watchr     *watcher.Watcher     // watcher
	RPCServer  *rpcserver.RPCServer // rpc server
	debugStats *debug.Stats
}

// NewNetctrler returns a controller instance
func NewNetctrler(serverURL, restURL, apisrvURL, vmmURL, resolverURLs string) (*Netctrler, error) {

	debugStats := debug.New(restURL)

	wr, err := writer.NewAPISrvWriter(apisrvURL, resolverURLs)
	if err != nil {
		log.Errorf("Error creating api server writer. Err: %v", err)
		return nil, err
	}

	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(wr)
	if err != nil {
		log.Errorf("Could not create network manager. Err: %v", err)
		return nil, err
	}

	// create watcher on api server
	watcher, err := watcher.NewWatcher(stateMgr, apisrvURL, vmmURL, resolverURLs, debugStats)
	if err != nil {
		log.Errorf("Error creating api server watcher. Err: %v", err)
		return nil, err
	}

	log.Infof("API server watcher %v is running", watcher)

	// create RPC server
	rpcServer, err := rpcserver.NewRPCServer(serverURL, stateMgr, debugStats)
	if err != nil {
		log.Errorf("Error creating RPC server. Err: %v", err)
		return nil, err
	}

	log.Infof("RPC server {%v} is running at %v", rpcServer, serverURL)

	go http.ListenAndServe(restURL, nil)

	// create the controller instance
	ctrler := Netctrler{
		StateMgr:   stateMgr,
		Watchr:     watcher,
		RPCServer:  rpcServer,
		debugStats: debugStats,
	}

	return &ctrler, err
}
