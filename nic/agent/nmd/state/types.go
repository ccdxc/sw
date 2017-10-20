// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"sync"
	"time"

	"net"
	"net/http"

	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/nic/agent/nmd/protos"
	"github.com/pensando/sw/venice/utils/emstore"
)

// NMD is the Naples management daemon instance object
type NMD struct {
	sync.Mutex     // Lock for NMD object
	sync.WaitGroup // Wait group

	store    emstore.Emstore // Embedded DB
	nodeUUID string          // Node's UUID
	cmd      CmdAPI          // CMD API object
	platform PlatformAPI     // Platform Agent object

	config nmd.Naples    // Naples config received via REST
	nic    *cmd.SmartNIC // SmartNIC object

	stopNICReg     chan bool     // channel to stop NIC registration
	nicRegInterval time.Duration // time interval between nic registration in milliseconds
	isRegOngoing   bool          // status of ongoing nic registration task

	stopNICUpd     chan bool     // channel to stop NIC update
	nicUpdInterval time.Duration // time interval between nic updates in seconds
	isUpdOngoing   bool          // status of ongoing nic update task

	listenURL        string       // URL where http server is listening
	listener         net.Listener // socket listener
	httpServer       *http.Server // HTTP server
	isRestSrvRunning bool         // status of the REST server

}

// NaplesConfigResp is response to NaplesConfig request nmd.Naples
type NaplesConfigResp struct {
	ErrorMsg string
}

// Setters and Getters for NMD attributes
// Note: Locks are in place for these setter and getters
// to prevent data races reported by go race detector.

func (n *NMD) getRegStatus() bool {
	n.Lock()
	defer n.Unlock()
	return n.isRegOngoing
}

func (n *NMD) setRegStatus(value bool) {
	n.Lock()
	defer n.Unlock()
	n.isRegOngoing = value
}

func (n *NMD) getUpdStatus() bool {
	n.Lock()
	defer n.Unlock()
	return n.isUpdOngoing
}

func (n *NMD) setUpdStatus(value bool) {
	n.Lock()
	defer n.Unlock()
	n.isUpdOngoing = value
}

func (n *NMD) getRestServerStatus() bool {
	n.Lock()
	defer n.Unlock()
	return n.isRestSrvRunning
}

func (n *NMD) setRestServerStatus(value bool) {
	n.Lock()
	defer n.Unlock()
	n.isRestSrvRunning = value
}

func (n *NMD) getConfigMode() nmd.NaplesMode {
	n.Lock()
	defer n.Unlock()
	return n.config.Spec.Mode
}

func (n *NMD) setNaplesConfig(cfg nmd.Naples) {
	n.Lock()
	defer n.Unlock()

	n.config = cfg
}

func (n *NMD) getNaplesConfig() nmd.Naples {
	n.Lock()
	defer n.Unlock()

	return n.config
}

// SetSmartNIC intializes the smartNIC object
func (n *NMD) SetSmartNIC(nic *cmd.SmartNIC) error {
	n.Lock()
	defer n.Unlock()

	n.nic = nic
	return nil
}

// GetSmartNIC returns the smartNIC object
func (n *NMD) GetSmartNIC() (*cmd.SmartNIC, error) {
	n.Lock()
	defer n.Unlock()

	return n.nic, nil
}

// getListenURL returns the listen URL of the http server
func (n *NMD) getListenURL() string {
	n.Lock()
	defer n.Unlock()

	if n.listener != nil {
		return n.listener.Addr().String()
	}
	return ""
}
