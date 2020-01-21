// +build iris

package iris

import (
	"context"
	"fmt"
	"time"

	"github.com/pkg/errors"

	iristypes "github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/types"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleSecurityProfile handles crud operations on profile
func HandleSecurityProfile(infraAPI types.InfraAPI, client halapi.NwSecurityClient, vrfClient halapi.VrfClient, oper types.Operation, profile netproto.SecurityProfile, vrf netproto.Vrf) error {
	switch oper {
	case types.Create:
		return createSecurityProfileHandler(infraAPI, client, vrfClient, profile, vrf)
	case types.Update:
		return updateSecurityProfileHandler(infraAPI, client, vrfClient, profile, vrf)
	case types.Delete:
		return deleteSecurityProfileHandler(infraAPI, client, vrfClient, profile, vrf)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createSecurityProfileHandler(infraAPI types.InfraAPI, client halapi.NwSecurityClient, vrfClient halapi.VrfClient, profile netproto.SecurityProfile, vrf netproto.Vrf) error {
	profileReqMsg := convertSecurityProfile(profile)
	resp, err := client.SecurityProfileCreate(context.Background(), profileReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", profile.GetKind(), profile.GetKey())); err != nil {
			return err
		}
	}

	// Update Vrf with attached security profile
	vrfUpdateMsg := convertVrf(vrf)
	vrfUpdateMsg.Request[0].SecurityKeyHandle = convertSecurityProfileKeyHandle(profile.Status.SecurityProfileID)

	vrfResp, err := vrfClient.VrfUpdate(context.Background(), vrfUpdateMsg)
	if vrfResp != nil {
		if err := utils.HandleErr(types.Update, vrfResp.Response[0].ApiStatus, err, fmt.Sprintf("Update VRF Failed for %s | %s", profile.GetKind(), profile.GetKey())); err != nil {
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

func updateSecurityProfileHandler(infraAPI types.InfraAPI, client halapi.NwSecurityClient, vrfClient halapi.VrfClient, profile netproto.SecurityProfile, vrf netproto.Vrf) error {
	profileReqMsg := convertSecurityProfile(profile)
	resp, err := client.SecurityProfileUpdate(context.Background(), profileReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.Response[0].ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", profile.GetKind(), profile.GetKey())); err != nil {
			return err
		}
	}

	// Update Vrf with attached security profile
	vrfUpdateMsg := convertVrf(vrf)
	vrfUpdateMsg.Request[0].SecurityKeyHandle = convertSecurityProfileKeyHandle(profile.Status.SecurityProfileID)

	vrfResp, err := vrfClient.VrfUpdate(context.Background(), vrfUpdateMsg)
	if vrfResp != nil {
		if err := utils.HandleErr(types.Update, vrfResp.Response[0].ApiStatus, err, fmt.Sprintf("Update VRF Failed for %s | %s", profile.GetKind(), profile.GetKey())); err != nil {
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

func deleteSecurityProfileHandler(infraAPI types.InfraAPI, client halapi.NwSecurityClient, vrfClient halapi.VrfClient, profile netproto.SecurityProfile, vrf netproto.Vrf) error {
	profileDelReq := &halapi.SecurityProfileDeleteRequestMsg{
		Request: []*halapi.SecurityProfileDeleteRequest{
			{
				KeyOrHandle: convertSecurityProfileKeyHandle(profile.Status.SecurityProfileID),
			},
		},
	}

	resp, err := client.SecurityProfileDelete(context.Background(), profileDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.Response[0].ApiStatus, err, fmt.Sprintf("SecurityProfile: %s", profile.GetKey())); err != nil {
			return err
		}
	}

	// Update Vrf with detached security profile
	vrfUpdateMsg := convertVrf(vrf)
	vrfResp, err := vrfClient.VrfUpdate(context.Background(), vrfUpdateMsg)
	if vrfResp != nil {
		if err := utils.HandleErr(types.Update, vrfResp.Response[0].ApiStatus, err, fmt.Sprintf("Update VRF Failed for %s | %s", profile.GetKind(), profile.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(profile.Kind, profile.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "VRF: %s | Err: %v", profile.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "VRF: %s | Err: %v", profile.GetKey(), err)
	}
	return nil
}

func convertSecurityProfile(profile netproto.SecurityProfile) *halapi.SecurityProfileRequestMsg {

	var (
		sessionIdleTimeout,
		tcpTimeout,
		tcpDropTimeout,
		tcpConnectionSetupTimeout,
		tcpHalfCloseTimeout,
		tcpCloseTimeout,
		dropTimeout,
		udpTimeout,
		udpDropTimeout,
		icmpTimeout,
		icmpDropTimeout uint32
	)

	timeouts := profile.Spec.Timeouts

	if (timeouts != nil) && (len(timeouts.SessionIdle)) > 0 {
		dur, err := time.ParseDuration(timeouts.SessionIdle)
		if err != nil {
			sessionIdleTimeout = types.DefaultTimeout
		} else {
			sessionIdleTimeout = uint32(dur.Seconds())
		}
	}

	if (timeouts != nil) && len(timeouts.TCP) > 0 {
		dur, err := time.ParseDuration(timeouts.TCP)
		if err != nil {
			tcpTimeout = types.DefaultTimeout
		} else {
			tcpTimeout = uint32(dur.Seconds())
		}
	}

	if (timeouts != nil) && len(timeouts.TCPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPDrop)
		if err != nil {
			tcpDropTimeout = types.DefaultTimeout
		} else {
			tcpDropTimeout = uint32(dur.Seconds())
		}
	}

	if (timeouts != nil) && len(timeouts.TCPConnectionSetup) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPConnectionSetup)
		if err != nil {
			tcpConnectionSetupTimeout = types.DefaultConnectionSetUpTimeout
		} else {
			tcpConnectionSetupTimeout = uint32(dur.Seconds())
		}
	}

	if (timeouts != nil) && len(timeouts.TCPHalfClose) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPHalfClose)
		if err != nil {
			tcpHalfCloseTimeout = types.DefaultTimeout
		} else {
			tcpHalfCloseTimeout = uint32(dur.Seconds())
		}
	}

	if (timeouts != nil) && len(timeouts.TCPClose) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPClose)
		if err != nil {
			tcpCloseTimeout = types.DefaultTimeout
		} else {
			tcpCloseTimeout = uint32(dur.Seconds())

		}
	}

	if (timeouts != nil) && len(timeouts.Drop) > 0 {
		dur, err := time.ParseDuration(timeouts.Drop)
		if err != nil {
			dropTimeout = types.DefaultTimeout

		} else {
			dropTimeout = uint32(dur.Seconds())
		}
	}

	if (timeouts != nil) && len(timeouts.UDP) > 0 {
		dur, err := time.ParseDuration(timeouts.UDP)
		if err != nil {
			udpTimeout = types.DefaultTimeout
		} else {
			udpTimeout = uint32(dur.Seconds())

		}
	}

	if (timeouts != nil) && len(timeouts.UDPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.UDPDrop)
		if err != nil {
			udpDropTimeout = types.DefaultTimeout
		} else {
			udpDropTimeout = uint32(dur.Seconds())
		}
	}

	if (timeouts != nil) && len(timeouts.ICMP) > 0 {
		dur, err := time.ParseDuration(timeouts.ICMP)
		if err != nil {
			icmpTimeout = types.DefaultTimeout
		} else {
			icmpTimeout = uint32(dur.Seconds())
		}
	}

	if (timeouts != nil) && len(timeouts.ICMPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.ICMPDrop)
		if err != nil {
			icmpDropTimeout = types.DefaultTimeout
		} else {
			icmpDropTimeout = uint32(dur.Seconds())
		}
	}

	return &halapi.SecurityProfileRequestMsg{
		Request: []*halapi.SecurityProfileSpec{
			{
				KeyOrHandle: convertSecurityProfileKeyHandle(profile.Status.SecurityProfileID),
				// User Specified timeout values
				SessionIdleTimeout:   sessionIdleTimeout,
				TcpTimeout:           tcpTimeout,
				TcpDropTimeout:       tcpDropTimeout,
				TcpCnxnSetupTimeout:  tcpConnectionSetupTimeout,
				TcpHalfClosedTimeout: tcpHalfCloseTimeout,
				TcpCloseTimeout:      tcpCloseTimeout,
				DropTimeout:          dropTimeout,
				UdpTimeout:           udpTimeout,
				UdpDropTimeout:       udpDropTimeout,
				IcmpTimeout:          icmpTimeout,
				IcmpDropTimeout:      icmpDropTimeout,

				// Defaults Enables
				TcpTsNotPresentDrop:    true,
				TcpNonSynFirstPktDrop:  true,
				TcpInvalidFlagsDrop:    true,
				TcpNonsynNoackDrop:     true,
				MulticastSrcDrop:       true,
				IcmpDeprecatedMsgsDrop: true,
				IcmpRedirectMsgDrop:    true,
				CnxnTrackingEn:         true,
				TcpNormalizationEn:     true,
				IpNormalizationEn:      true,

				// Default Normalization action allow
				IpDfAction:            iristypes.IPDFAction,
				IpOptionsAction:       iristypes.IPOptionsAction,
				IcmpInvalidCodeAction: iristypes.ICMPInvalidCodeAction,

				// Default Normalization actino edit
				IpInvalidLenAction:          iristypes.IPInvalidLenAction,
				TcpUnexpectedMssAction:      iristypes.TCPUnexpectedMSSAction,
				TcpUnexpectedWinScaleAction: iristypes.TCPUnexpectedWinScaleAction,
				TcpUnexpectedSackPermAction: iristypes.TCPUnexpectedSACKPermAction,
				TcpUrgPtrNotSetAction:       iristypes.TCPUrgentPtrNotSetAction,
				TcpUrgFlagNotSetAction:      iristypes.TCPUrgentFlagNotSetAction,
				TcpUrgPayloadMissingAction:  iristypes.TCPUrgentPayloadMissingAction,
				TcpDataLenGtMssAction:       iristypes.TCPDataLenGreaterThanMSSAction,
				TcpDataLenGtWinSizeAction:   iristypes.TCPDataLenGreaterThanWinSizeAction,
				TcpUnexpectedTsOptionAction: iristypes.TCPUnexpectedTSOptionAction,

				// Default Normalization action drop
				TcpUnexpectedEchoTsAction:     iristypes.TCPUnexpectedEchoTSAction,
				TcpUnexpectedSackOptionAction: iristypes.TCPUnexpectedSACKOptionAction,
				TcpRsvdFlagsAction:            iristypes.TCPReservedFlagsAction,

				// Default Normalization action none
				TcpRstWithDataAction: iristypes.TCPRSTWithDataAction,
			},
		},
	}
}

func convertSecurityProfileKeyHandle(profileID uint64) *halapi.SecurityProfileKeyHandle {
	return &halapi.SecurityProfileKeyHandle{
		KeyOrHandle: &halapi.SecurityProfileKeyHandle_ProfileId{
			ProfileId: uint32(profileID),
		},
	}
}
