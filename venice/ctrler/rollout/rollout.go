// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rollout

import (
	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver"
	"github.com/pensando/sw/venice/ctrler/rollout/statemgr"
	"github.com/pensando/sw/venice/ctrler/rollout/watcher"
	"github.com/pensando/sw/venice/ctrler/rollout/writer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// Ctrler - Rollout controller object
type Ctrler struct {
	StateMgr  *statemgr.Statemgr // state manager
	Watchr    *watcher.Watcher
	RPCServer *rpcserver.RPCServer // grpc server for agents
	writer    writer.Writer
}

// NewCtrler returns a controller instance
func NewCtrler(serverURL, apisrvURL string, resolver resolver.Interface) (*Ctrler, error) {

	// create writer
	wr, err := writer.NewAPISrvWriter(apisrvURL, resolver)
	if err != nil {
		log.Errorf("Error creating api server writer. Err: %v", err)
		return nil, err
	}

	// create rollout state
	stateMgr, err := statemgr.NewStatemgr(wr)
	if err != nil {
		log.Errorf("Could not create rollout statemanager. Err: %v", err)
		return nil, err
	}

	// create watcher on api server
	watcher, err := watcher.NewWatcher(stateMgr, apisrvURL, resolver, nil)
	if err != nil {
		log.Errorf("Error creating api server watcher. Err: %v", err)
		return nil, err
	}

	log.Infof("API server watcher is running")

	// create RPC server for communication with Agents
	rpcServer, err := rpcserver.NewRPCServer(serverURL, stateMgr)
	if err != nil {
		log.Errorf("Error creating RPC server. Err: %v", err)
		return nil, err
	}

	log.Infof("RPC server is running at %v", serverURL)

	// create the controller instance
	ctrler := Ctrler{
		StateMgr:  stateMgr,
		Watchr:    watcher,
		RPCServer: rpcServer,
		writer:    wr,
	}

	return &ctrler, err
}

// Stop rollout controller and release resources
func (c *Ctrler) Stop() error {
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
