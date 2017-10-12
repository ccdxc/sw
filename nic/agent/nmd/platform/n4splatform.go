// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package platform

import (
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/nic/agent/nmd/state"
)

// NaplesPlatformAgent is the mock platform for testing purposes
type NaplesPlatformAgent struct {
	nmd state.NmdPlatformAPI // NMD server instance
}

// NewNaplesPlatformAgent returns a new mock platform agent
func NewNaplesPlatformAgent() (*NaplesPlatformAgent, error) {
	// create new naples platform
	np := NaplesPlatformAgent{}
	return &np, nil
}

// RegisterNMD sets the agent for this platform
func (np *NaplesPlatformAgent) RegisterNMD(nmd state.NmdPlatformAPI) error {
	// store the nmd instance
	np.nmd = nmd
	return nil
}

// CreateSmartNIC creates a SmartNIC in platform
func (np *NaplesPlatformAgent) CreateSmartNIC(nw *cmd.SmartNIC) error {
	return nil
}

// UpdateSmartNIC updates a SmartNIC in platform
func (np *NaplesPlatformAgent) UpdateSmartNIC(nw *cmd.SmartNIC) error {
	return nil
}

// DeleteSmartNIC deletes a SmartNIC from platform
func (np *NaplesPlatformAgent) DeleteSmartNIC(nw *cmd.SmartNIC) error {
	return nil
}
