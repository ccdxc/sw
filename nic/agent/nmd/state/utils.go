package state

import (
	"errors"
	"fmt"
	"net"

	nmdProto "github.com/pensando/sw/nic/agent/protos/nmd"
	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
)

func isHostModeValid(spec nmdProto.DistributedServiceCardSpec) (err error) {
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

func isNetworkModeValid(spec nmdProto.DistributedServiceCardSpec) (err error) {
	switch {
	case len(spec.NetworkMode) == 0:
		return errors.New("network mode must specify management network. either inband or oob")
	case len(spec.ID) == 0:
		return errors.New("id cannot be empty for network managed mode")
	case !(spec.NetworkMode == nmdProto.NetworkMode_INBAND.String() || spec.NetworkMode == nmdProto.NetworkMode_OOB.String()):
		return fmt.Errorf("network mode must specify management network. either inband or oob. Found: %v", spec.NetworkMode)
		// TODO : Bring this back once decision on various profiles when in network managed mode is made
		//case len(spec.DSCProfile) != 0 && spec.DSCProfile != "default":
		//	return fmt.Errorf("naples profile is not applicable when naples is in network managed mode. Found: %v", spec.DSCProfile)
	}

	checkSubnet := false
	var mgmtNet *net.IPNet

	// IPAddress will be non-empty if management IP is passed for statically configuring management port on Naples
	if spec.IPConfig != nil && len(spec.IPConfig.IPAddress) != 0 {
		if len(spec.Controllers) == 0 {
			return fmt.Errorf("controllers must be passed when statically configuring management IP. Use --controllers option")
		}

		if vldtor.CIDR(spec.IPConfig.IPAddress) == nil {
			if len(spec.IPConfig.DefaultGW) == 0 {
				checkSubnet = true
				_, mgmtNet, _ = net.ParseCIDR(spec.IPConfig.IPAddress)
			}
		} else {
			return fmt.Errorf("invalid management IP %v specified. Must be in CIDR Format", spec.IPConfig.IPAddress)
		}

		if spec.InbandIPConfig != nil && (len(spec.InbandIPConfig.IPAddress) == 0 || vldtor.CIDR(spec.InbandIPConfig.IPAddress) != nil) {
			return fmt.Errorf("invalid inband IP %v specified. Must be in CIDR Format", spec.InbandIPConfig.IPAddress)
		}

		if spec.InbandIPConfig != nil && spec.NetworkMode == nmdProto.NetworkMode_INBAND.String() {
			return fmt.Errorf("cannot configure inband ip on inb when it is also mgmt network")
		}
	}

	for _, c := range spec.Controllers {
		if vldtor.HostAddr(c) != nil {
			return fmt.Errorf("invalid controller %v specified. Must be either IP Addresses or FQDNs", c)
		} else if checkSubnet {
			controllerIP := net.ParseIP(c)
			if !mgmtNet.Contains(controllerIP) {
				return fmt.Errorf("controller %v is not in the same subnet as the Management IP %v. Add default gateway using --default-gw option", c, spec.IPConfig.IPAddress)
			}
		}
	}

	return nil
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
