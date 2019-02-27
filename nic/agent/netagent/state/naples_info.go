package state

import (
	"fmt"
	"time"

	"github.com/pensando/sw/nic/agent/netagent/state/types"
)

// GetNaplesInfo returns read only Naples information
func (na *Nagent) GetNaplesInfo() (*types.NaplesInfo, error) {
	var naplesInfo types.NaplesInfo
	naplesInfo.UUID = na.NodeUUID
	naplesInfo.ControllerIPs = na.ControllerIPs
	naplesInfo.Mode = na.Mode
	naplesInfo.IsNpmClientConnected = na.Ctrlerif.IsConnected()
	return &naplesInfo, nil
}

// GetNetagentUptime returns uptime of netagent
func (na *Nagent) GetNetagentUptime() (string, error) {
	dur := time.Since(na.NetAgentStartTime)
	uptime := fmt.Sprintf("Netagent up and running for: %s", dur.String())
	return uptime, nil
}
