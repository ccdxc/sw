package state

import (
	"errors"
	"fmt"

	nmdProto "github.com/pensando/sw/nic/agent/protos/nmd"
)

func isHostModeValid(spec nmdProto.NaplesSpec) (err error) {
	switch {
	case len(spec.NetworkMode) != 0 && spec.NetworkMode != nmdProto.NetworkMode_NONE.String():
		return fmt.Errorf("network mode must not be specified when naples is in host managed mode. Found: %v", spec.NetworkMode)

	case len(spec.Controllers) != 0:
		return fmt.Errorf("controllers must not be specified when naples is in host managed mode. Found: %v", spec.Controllers)

	case spec.IPConfig != nil && (len(spec.IPConfig.IPAddress) != 0 || len(spec.IPConfig.DefaultGW) != 0 || len(spec.IPConfig.DNSServers) != 0):
		return fmt.Errorf("ip config must be empty when naples is in host managed mode. Found: %v", spec.IPConfig)

	default:
		return nil
	}
}

func isNetworkModeValid(spec nmdProto.NaplesSpec) (err error) {
	switch {
	case len(spec.NetworkMode) == 0:
		return errors.New("network mode must specify management network. either inband or oob")
	case len(spec.ID) == 0:
		return errors.New("id cannot be empty for network managed mode")
	case !(spec.NetworkMode == nmdProto.NetworkMode_INBAND.String() || spec.NetworkMode == nmdProto.NetworkMode_OOB.String()):
		return fmt.Errorf("network mode must specify management network. either inband or oob. Found: %v", spec.NetworkMode)

	case len(spec.NaplesProfile) != 0 && spec.NaplesProfile != "default":
		return fmt.Errorf("naples profile is not applicable when naples is in network managed mode. Found: %v", spec.NaplesProfile)
	default:
		return nil
	}
}

func errBadRequest(err error) *ErrBadRequest {
	if err != nil {
		errBadReq := ErrBadRequest{
			Message: err.Error(),
		}
		return &errBadReq
	}
	return nil
}

func errInternalServer(err error) *ErrInternalServer {
	if err != nil {
		errSrv := ErrInternalServer{
			Message: err.Error(),
		}
		return &errSrv
	}
	return nil
}
