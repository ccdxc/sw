// +build apulu

package apulu

import (
	"context"
	"encoding/binary"
	"fmt"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleSecurityProfile handles crud operations on profile
func HandleSecurityProfile(infraAPI types.InfraAPI, client halapi.SecurityPolicySvcClient, oper types.Operation, profile netproto.SecurityProfile) error {
	switch oper {
	case types.Create:
		return createSecurityProfileHandler(infraAPI, client, profile)
	case types.Update:
		return updateSecurityProfileHandler(infraAPI, client, profile)
	case types.Delete:
		return deleteSecurityProfileHandler(infraAPI, client, profile)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createSecurityProfileHandler(infraAPI types.InfraAPI, client halapi.SecurityPolicySvcClient, profile netproto.SecurityProfile) error {
	profileReqMsg := convertSecurityProfile(profile)
	resp, err := client.SecurityProfileCreate(context.Background(), profileReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", profile.GetKind(), profile.GetKey())); err != nil {
			return err
		}
	}

	dat, _ := profile.Marshal()

	if err := infraAPI.Store(profile.Kind, profile.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "SecurityProfile: %s | Err: %v", profile.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "SecurityProfile: %s | Err: %v", profile.GetKey(), err)
	}
	return nil
}

func updateSecurityProfileHandler(infraAPI types.InfraAPI, client halapi.SecurityPolicySvcClient, profile netproto.SecurityProfile) error {
	profileReqMsg := convertSecurityProfile(profile)
	resp, err := client.SecurityProfileUpdate(context.Background(), profileReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", profile.GetKind(), profile.GetKey())); err != nil {
			return err
		}
	}

	dat, _ := profile.Marshal()

	if err := infraAPI.Store(profile.Kind, profile.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "SecurityProfile: %s | Err: %v", profile.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "SecurityProfile: %s | Err: %v", profile.GetKey(), err)
	}
	return nil
}

func deleteSecurityProfileHandler(infraAPI types.InfraAPI, client halapi.SecurityPolicySvcClient, profile netproto.SecurityProfile) error {
	profileID := make([]byte, 8)
	binary.LittleEndian.PutUint64(profileID, profile.Status.SecurityProfileID)
	securityProfileDeleteRequest := &halapi.SecurityProfileDeleteRequest{
		Id: [][]byte{
			profileID,
		},
	}

	resp, err := client.SecurityProfileDelete(context.Background(), securityProfileDeleteRequest)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus[0], err, fmt.Sprintf("Create failed for SecurityProfile")); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(profile.Kind, profile.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "SecurityProfile: %s | Err: %v", profile.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "SecurityProfile: %s | Err: %v", profile.GetKey(), err)
	}
	return nil
}

func convertSecurityProfile(profile netproto.SecurityProfile) *halapi.SecurityProfileRequest {
	var (
		tcpIdleTimeout,
		udpIdleTimeout,
		icmpIdleTimeout,
		tcpConnectionSetupTimeout,
		tcpHalfCloseTimeout,
		tcpCloseTimeout,
		tcpDropTimeout,
		udpDropTimeout,
		icmpDropTimeout uint32
	)

	timeouts := profile.Spec.Timeouts

	if (timeouts != nil) && len(timeouts.TCP) > 0 {
		dur, err := time.ParseDuration(timeouts.TCP)
		if err != nil {
			tcpIdleTimeout = types.DefaultTCPIdleTimeout
		} else {
			tcpIdleTimeout = uint32(dur.Seconds())
		}
	} else {
		tcpIdleTimeout = types.DefaultTCPIdleTimeout
	}

	if (timeouts != nil) && len(timeouts.UDP) > 0 {
		dur, err := time.ParseDuration(timeouts.UDP)
		if err != nil {
			udpIdleTimeout = types.DefaultUDPIdleTimeout
		} else {
			udpIdleTimeout = uint32(dur.Seconds())
		}
	} else {
		udpIdleTimeout = types.DefaultUDPIdleTimeout
	}

	if (timeouts != nil) && len(timeouts.ICMP) > 0 {
		dur, err := time.ParseDuration(timeouts.ICMP)
		if err != nil {
			icmpIdleTimeout = types.DefaultICMPIdleTimeout
		} else {
			icmpIdleTimeout = uint32(dur.Seconds())
		}
	} else {
		icmpIdleTimeout = types.DefaultICMPIdleTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPConnectionSetup) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPConnectionSetup)
		if err != nil {
			tcpConnectionSetupTimeout = types.DefaultTCPCnxnSetupTimeout
		} else {
			tcpConnectionSetupTimeout = uint32(dur.Seconds())
		}
	} else {
		tcpConnectionSetupTimeout = types.DefaultTCPCnxnSetupTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPHalfClose) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPHalfClose)
		if err != nil {
			tcpHalfCloseTimeout = types.DefaultTCPHalfCloseTimeout
		} else {
			tcpHalfCloseTimeout = uint32(dur.Seconds())
		}
	} else {
		tcpHalfCloseTimeout = types.DefaultTCPHalfCloseTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPClose) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPClose)
		if err != nil {
			tcpCloseTimeout = types.DefaultTCPCloseTimeout
		} else {
			tcpCloseTimeout = uint32(dur.Seconds())
		}
	} else {
		tcpCloseTimeout = types.DefaultTCPCloseTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPDrop)
		if err != nil {
			tcpDropTimeout = types.DefaultTCPDropTimeout
		} else {
			tcpDropTimeout = uint32(dur.Seconds())
		}
	} else {
		tcpDropTimeout = types.DefaultTCPDropTimeout
	}

	if (timeouts != nil) && len(timeouts.UDPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.UDPDrop)
		if err != nil {
			udpDropTimeout = types.DefaultUDPDropTimeout
		} else {
			udpDropTimeout = uint32(dur.Seconds())
		}
	} else {
		udpDropTimeout = types.DefaultUDPDropTimeout
	}

	if (timeouts != nil) && len(timeouts.ICMPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.ICMPDrop)
		if err != nil {
			icmpDropTimeout = types.DefaultICMPDropTimeout
		} else {
			icmpDropTimeout = uint32(dur.Seconds())
		}
	} else {
		icmpDropTimeout = types.DefaultICMPDropTimeout
	}

	return &halapi.SecurityProfileRequest{
		Request: []*halapi.SecurityProfileSpec{
			{
				Id:                  []byte{1},
				DefaultFWAction:     halapi.SecurityRuleAction_SECURITY_RULE_ACTION_DENY,
				TCPIdleTimeout:      tcpIdleTimeout,
				UDPIdleTimeout:      udpIdleTimeout,
				ICMPIdleTimeout:     icmpIdleTimeout,
				OtherIdleTimeout:    types.DefaultOtherIdleTimeout,
				TCPCnxnSetupTimeout: tcpConnectionSetupTimeout,
				TCPHalfCloseTimeout: tcpHalfCloseTimeout,
				TCPCloseTimeout:     tcpCloseTimeout,
				TCPDropTimeout:      tcpDropTimeout,
				UDPDropTimeout:      udpDropTimeout,
				ICMPDropTimeout:     icmpDropTimeout,
				OtherDropTimeout:    types.DefaultOtherDropTimeout,
			},
		},
	}
}
