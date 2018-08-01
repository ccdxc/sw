package InfraApi

import (
	Common "github.com/pensando/sw/nic/e2etests/go/common"
)

//EndpointAPI interface
type EndpointAPI interface {
	RunCommand(cmd []string, timeout int, background bool) (Common.CmdHandle, error)
	GetIP() string
}
