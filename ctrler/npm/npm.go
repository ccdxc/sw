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
	"github.com/pensando/sw/ctrler/npm/rpcserver"
	"github.com/pensando/sw/ctrler/npm/statemgr"
	"github.com/pensando/sw/ctrler/npm/watcher"
	"github.com/pensando/sw/utils/log"
)

// Netctrler is a netctrler instance
type Netctrler struct {
	StateMgr  *statemgr.Statemgr   // state manager
	Watchr    *watcher.Watcher     // watcher
	RPCServer *rpcserver.RPCServer // rpc server
}

// NewNetctrler returns a controller instance
func NewNetctrler(serverURL, apisrvURL, vmmURL string) (*Netctrler, error) {
	// create network state manager
	stateMgr, err := statemgr.NewStatemgr()
	if err != nil {
		log.Errorf("Could not create network manager. Err: %v", err)
		return nil, err
	}

	// create watcher on api server
	watcher, err := watcher.NewWatcher(stateMgr, apisrvURL, vmmURL)
	if err != nil {
		log.Errorf("Error creating api server watcher. Err: %v", err)
		return nil, err
	}

	log.Infof("API server watcher %v is running", watcher)

	// create RPC server
	rpcServer, err := rpcserver.NewRPCServer(serverURL, stateMgr)
	if err != nil {
		log.Errorf("Error creating RPC server. Err: %v", err)
		return nil, err
	}

	log.Infof("RPC server {%v} is running at %v", rpcServer, serverURL)

	// create the controller instance
	ctrler := Netctrler{
		StateMgr:  stateMgr,
		Watchr:    watcher,
		RPCServer: rpcServer,
	}

	return &ctrler, err
}
