// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

// Package tsm contains troubleshooting manager code
/************ Architecture of TsCtrler ******************
 *
 *    +-----------------------+
 *    |   API Server          |
 *    +-----------------------+
 *        |              ^
 *        V              |
 *    +---------+    +--------+
 *    | Watcher |    | Writer |
 *  +----------------------------------------+
 *  |    Trouble Shooting State Manager      |
 *  | +--------------- + +---------------+   |
 *  | | MirrorSessions | | Other Objects |   |
 *  | +----------------+ +---------------+   |
 *  +----------------------------------------+
 *  |               RPC Server               |
 *  +----------------------------------------+
 *       ^           ^            ^
 *       |           |            |
 *       V           V            V
 *  +--------+   +--------+   +--------+
 *  | Agent  |   | Agent  |   | Agent  |
 *  | Node 1 |   | Node 2 |   | Node 3 |
 *  +--------+   +--------+   +--------+
 *
 *********************************************************/
package tsm

import (
	"net/http"

	"github.com/pensando/sw/venice/ctrler/tsm/rpcserver"
	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"
	"github.com/pensando/sw/venice/ctrler/tsm/watcher"
	"github.com/pensando/sw/venice/ctrler/tsm/writer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// TsCtrler - Trouble shooting controller object
type TsCtrler struct {
	StateMgr  *statemgr.Statemgr // state manager
	Watchr    *watcher.Watcher
	RPCServer *rpcserver.RPCServer // grpc server for agents
	writer    writer.Writer
}

// NewTsCtrler returns a controller instance
func NewTsCtrler(serverURL, restURL, apisrvURL string, resolver resolver.Interface) (*TsCtrler, error) {

	// create writer
	wr, err := writer.NewAPISrvWriter(apisrvURL, resolver)
	if err != nil {
		log.Errorf("Error creating api server writer. Err: %v", err)
		return nil, err
	}

	// create trouble shooting state
	stateMgr, err := statemgr.NewStatemgr(wr)
	if err != nil {
		log.Errorf("Could not create trouble shooting manager. Err: %v", err)
		return nil, err
	}

	// create watcher on api server
	watcher, err := watcher.NewWatcher(stateMgr, apisrvURL, resolver)
	if err != nil {
		log.Errorf("Error creating api server watcher. Err: %v", err)
		return nil, err
	}

	log.Infof("API server watcher %v is running", watcher)

	// create RPC server for communication with Agents
	rpcServer, err := rpcserver.NewRPCServer(serverURL, stateMgr)
	if err != nil {
		log.Errorf("Error creating RPC server. Err: %v", err)
		return nil, err
	}

	log.Infof("RPC server is running at %v", serverURL)

	go http.ListenAndServe(restURL, nil)

	// create the controller instance
	ctrler := TsCtrler{
		StateMgr:  stateMgr,
		Watchr:    watcher,
		RPCServer: rpcServer,
		writer:    wr,
	}

	return &ctrler, err
}

// Stop trouble shooting controller and release resources
func (c *TsCtrler) Stop() error {
	if c.Watchr != nil {
		c.Watchr.Stop()
		c.Watchr = nil
	}
	if c.RPCServer != nil {
		c.RPCServer.Stop()
		c.RPCServer = nil
	}
	if c.StateMgr != nil {
		c.StateMgr.Stop()
		c.StateMgr = nil
	}
	if c.writer != nil {
		c.writer.Close()
	}
	return nil
}
