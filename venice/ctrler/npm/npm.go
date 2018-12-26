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

	"github.com/pensando/sw/nic/agent/netagent/protos/generated/nimbus"
	"github.com/pensando/sw/venice/ctrler/npm/statemgr"
	"github.com/pensando/sw/venice/globals"
	debugStats "github.com/pensando/sw/venice/utils/debug/stats"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// Netctrler is a netctrler instance
type Netctrler struct {
	StateMgr   *statemgr.Statemgr // state manager
	mserver    *nimbus.MbusServer // nimbu server
	debugStats *debugStats.Stats
}

// NewNetctrler returns a controller instance
func NewNetctrler(serverURL, restURL, apisrvURL, vmmURL string, resolver resolver.Interface) (*Netctrler, error) {

	debugStats := debugStats.New(restURL).Build()

	// create nimbus server
	msrv, err := nimbus.NewMbusServer(globals.Npm, serverURL)
	if err != nil {
		log.Fatalf("Could not start RPC server. Err: %v", err)
	}

	// create network state manager
	stateMgr, err := statemgr.NewStatemgr(apisrvURL, resolver, msrv)
	if err != nil {
		log.Errorf("Could not create network manager. Err: %v", err)
		return nil, err
	}

	log.Infof("RPC server is running at %v", serverURL)

	// start a REST server for debug endpoints
	go http.ListenAndServe(restURL, nil)

	// create the controller instance
	ctrler := Netctrler{
		StateMgr:   stateMgr,
		mserver:    msrv,
		debugStats: debugStats,
	}

	// start the RPC server
	msrv.Start()

	return &ctrler, nil
}

// Stop server and release resources
func (c *Netctrler) Stop() error {
	if c.mserver != nil {
		c.mserver.Stop()
	}

	c.StateMgr.Stop()
	return nil
}
