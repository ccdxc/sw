package state

import (
	"github.com/pensando/sw/nic/agent/netagent/state/types"
)

// GetNaplesInfo returns read only Naples information
func (na *Nagent) GetNaplesInfo() (*types.NaplesInfo, error) {
	var naplesInfo types.NaplesInfo
	naplesInfo.UUID = na.NodeUUID
	naplesInfo.ControllerIPs = na.ControllerIPs
	naplesInfo.Mode = na.Mode
	return &naplesInfo, nil
}
