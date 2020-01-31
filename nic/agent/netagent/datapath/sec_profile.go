package datapath

import (
	"context"
	"fmt"
	"strings"
	"time"

	"github.com/pensando/sw/nic/agent/netagent/datapath/constants"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateSecurityProfile creates a security profile in the datapath
func (hd *Datapath) CreateSecurityProfile(profile *netproto.SecurityProfile, attachmentVrfs []*netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var vrfSpec []*halproto.VrfSpec
	var vrfType halproto.VrfType
	for _, vrf := range attachmentVrfs {
		if strings.ToLower(vrf.Spec.VrfType) == "infra" {
			vrfType = halproto.VrfType_VRF_TYPE_INFRA
		} else {
			vrfType = halproto.VrfType_VRF_TYPE_CUSTOMER
		}

		// Build vrfUpdateMessage
		spec := halproto.VrfSpec{
			KeyOrHandle: &halproto.VrfKeyHandle{
				KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
					VrfId: vrf.Status.VrfID,
				},
			},
			SecurityKeyHandle: &halproto.SecurityProfileKeyHandle{
				KeyOrHandle: &halproto.SecurityProfileKeyHandle_ProfileId{
					ProfileId: uint32(profile.Status.SecurityProfileID),
				},
			},
			VrfType: vrfType,
		}
		vrfSpec = append(vrfSpec, &spec)
	}

	vrfUpdateMsg := halproto.VrfRequestMsg{
		Request: vrfSpec,
	}

	var sessionIdleTimeout, tcpTimeout, tcpDropTimeout, tcpConnectionSetupTimeout, tcpHalfCloseTimeout, tcpCloseTimeout, dropTimeout, udpTimeout, udpDropTimeout, icmpTimeout, icmpDropTimeout uint32
	timeouts := profile.Spec.Timeouts

	if (timeouts != nil) && (len(timeouts.SessionIdle)) > 0 {
		dur, err := time.ParseDuration(timeouts.SessionIdle)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.SessionIdle)
		}
		sessionIdleTimeout = uint32(dur.Seconds())
	} else {
		sessionIdleTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.TCP) > 0 {
		dur, err := time.ParseDuration(timeouts.TCP)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.TCP)
		}
		tcpTimeout = uint32(dur.Seconds())
	} else {
		tcpTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPDrop)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.TCPDrop)
		}
		tcpDropTimeout = uint32(dur.Seconds())
	} else {
		tcpDropTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPConnectionSetup) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPConnectionSetup)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.TCPConnectionSetup)
		}
		tcpConnectionSetupTimeout = uint32(dur.Seconds())
	} else {
		tcpConnectionSetupTimeout = constants.DefaultConnectionSetUpTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPHalfClose) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPHalfClose)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.TCPHalfClose)
		}
		tcpHalfCloseTimeout = uint32(dur.Seconds())
	} else {
		tcpHalfCloseTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPClose) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPClose)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.TCPClose)
		}
		tcpCloseTimeout = uint32(dur.Seconds())
	} else {
		tcpCloseTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.Drop) > 0 {
		dur, err := time.ParseDuration(timeouts.Drop)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.Drop)
		}
		dropTimeout = uint32(dur.Seconds())
	} else {
		dropTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.UDP) > 0 {
		dur, err := time.ParseDuration(timeouts.UDP)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.UDP)
		}
		udpTimeout = uint32(dur.Seconds())
	} else {
		udpTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.UDPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.UDPDrop)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.UDP)
		}
		udpDropTimeout = uint32(dur.Seconds())
	} else {
		udpDropTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.ICMP) > 0 {
		dur, err := time.ParseDuration(timeouts.ICMP)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.ICMP)
		}
		icmpTimeout = uint32(dur.Seconds())
	} else {
		icmpTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.ICMPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.ICMPDrop)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.ICMP)
		}
		icmpDropTimeout = uint32(dur.Seconds())
	} else {
		icmpDropTimeout = constants.DefaultTimeout
	}

	secProfileReqMsg := &halproto.SecurityProfileRequestMsg{
		Request: []*halproto.SecurityProfileSpec{
			{
				KeyOrHandle: &halproto.SecurityProfileKeyHandle{
					KeyOrHandle: &halproto.SecurityProfileKeyHandle_ProfileId{
						ProfileId: uint32(profile.Status.SecurityProfileID),
					},
				},
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
				PolicyEnforceEn:        true,

				// Default Normalization action allow
				IpDfAction:            constants.IPDFAction,
				IpOptionsAction:       constants.IPOptionsAction,
				IcmpInvalidCodeAction: constants.ICMPInvalidCodeAction,

				// Default Normalization actino edit
				IpInvalidLenAction:          constants.IPInvalidLenAction,
				TcpUnexpectedMssAction:      constants.TCPUnexpectedMSSAction,
				TcpUnexpectedWinScaleAction: constants.TCPUnexpectedWinScaleAction,
				TcpUnexpectedSackPermAction: constants.TCPUnexpectedSACKPermAction,
				TcpUrgPtrNotSetAction:       constants.TCPUrgentPtrNotSetAction,
				TcpUrgFlagNotSetAction:      constants.TCPUrgentFlagNotSetAction,
				TcpUrgPayloadMissingAction:  constants.TCPUrgentPayloadMissingAction,
				TcpDataLenGtMssAction:       constants.TCPDataLenGreaterThanMSSAction,
				TcpDataLenGtWinSizeAction:   constants.TCPDataLenGreaterThanWinSizeAction,
				TcpUnexpectedTsOptionAction: constants.TCPUnexpectedTSOptionAction,

				// Default Normalization action drop
				TcpUnexpectedEchoTsAction:     constants.TCPUnexpectedEchoTSAction,
				TcpUnexpectedSackOptionAction: constants.TCPUnexpectedSACKOptionAction,
				TcpRsvdFlagsAction:            constants.TCPReservedFlagsAction,

				// Default Normalization action none
				TcpRstWithDataAction: constants.TCPRSTWithDataAction,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Sgclient.SecurityProfileCreate(context.Background(), secProfileReqMsg)
		if err != nil {
			log.Errorf("Error creating security profile. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Sgclient.SecurityProfileCreate(context.Background(), secProfileReqMsg)
		if err != nil {
			log.Errorf("Error creating security profile. Err: %v", err)
			return err
		}
	}

	// Update the vrf spec in hal to point to the security profile that was created
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Tnclient.VrfUpdate(context.Background(), &vrfUpdateMsg)
		if err != nil {
			log.Errorf("Error updating vrf after security profile create. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Tnclient.VrfUpdate(context.Background(), &vrfUpdateMsg)
		if err != nil {
			log.Errorf("Error updating vrf after security profile create. Err: %v", err)
			return err
		}
	}
	return nil
}

// UpdateSecurityProfile updates a security profile in the datapath
func (hd *Datapath) UpdateSecurityProfile(profile *netproto.SecurityProfile, attachmentVrfs []*netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()

	var vrfSpec []*halproto.VrfSpec
	var vrfType halproto.VrfType
	for _, vrf := range attachmentVrfs {
		if strings.ToLower(vrf.Spec.VrfType) == "infra" {
			vrfType = halproto.VrfType_VRF_TYPE_INFRA
		} else {
			vrfType = halproto.VrfType_VRF_TYPE_CUSTOMER
		}

		// Build vrfUpdateMessage
		spec := halproto.VrfSpec{
			KeyOrHandle: &halproto.VrfKeyHandle{
				KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
					VrfId: vrf.Status.VrfID,
				},
			},
			SecurityKeyHandle: &halproto.SecurityProfileKeyHandle{
				KeyOrHandle: &halproto.SecurityProfileKeyHandle_ProfileId{
					ProfileId: uint32(profile.Status.SecurityProfileID),
				},
			},
			VrfType: vrfType,
		}
		vrfSpec = append(vrfSpec, &spec)
	}

	vrfUpdateMsg := halproto.VrfRequestMsg{
		Request: vrfSpec,
	}

	var sessionIdleTimeout, tcpTimeout, tcpDropTimeout, tcpConnectionSetupTimeout, tcpHalfCloseTimeout, tcpCloseTimeout, dropTimeout, udpTimeout, udpDropTimeout, icmpTimeout, icmpDropTimeout uint32
	timeouts := profile.Spec.Timeouts

	if (timeouts != nil) && (len(timeouts.SessionIdle)) > 0 {
		dur, err := time.ParseDuration(timeouts.SessionIdle)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.SessionIdle)
		}
		sessionIdleTimeout = uint32(dur.Seconds())
	} else {
		sessionIdleTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.TCP) > 0 {
		dur, err := time.ParseDuration(timeouts.TCP)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.TCP)
		}
		tcpTimeout = uint32(dur.Seconds())
	} else {
		tcpTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPDrop)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.TCPDrop)
		}
		tcpDropTimeout = uint32(dur.Seconds())
	} else {
		tcpDropTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPConnectionSetup) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPConnectionSetup)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.TCPConnectionSetup)
		}
		tcpConnectionSetupTimeout = uint32(dur.Seconds())
	} else {
		tcpConnectionSetupTimeout = constants.DefaultConnectionSetUpTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPHalfClose) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPHalfClose)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.TCPHalfClose)
		}
		tcpHalfCloseTimeout = uint32(dur.Seconds())
	} else {
		tcpHalfCloseTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.TCPClose) > 0 {
		dur, err := time.ParseDuration(timeouts.TCPClose)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.TCPClose)
		}
		tcpCloseTimeout = uint32(dur.Seconds())
	} else {
		tcpCloseTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.Drop) > 0 {
		dur, err := time.ParseDuration(timeouts.Drop)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.Drop)
		}
		dropTimeout = uint32(dur.Seconds())
	} else {
		dropTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.UDP) > 0 {
		dur, err := time.ParseDuration(timeouts.UDP)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.UDP)
		}
		udpTimeout = uint32(dur.Seconds())
	} else {
		udpTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.UDPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.UDPDrop)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.UDP)
		}
		udpDropTimeout = uint32(dur.Seconds())
	} else {
		udpDropTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.ICMP) > 0 {
		dur, err := time.ParseDuration(timeouts.ICMP)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.ICMP)
		}
		icmpTimeout = uint32(dur.Seconds())
	} else {
		icmpTimeout = constants.DefaultTimeout
	}

	if (timeouts != nil) && len(timeouts.ICMPDrop) > 0 {
		dur, err := time.ParseDuration(timeouts.ICMPDrop)
		if err != nil {
			return fmt.Errorf("invalid time duration %s", timeouts.ICMP)
		}
		icmpDropTimeout = uint32(dur.Seconds())
	} else {
		icmpDropTimeout = constants.DefaultTimeout
	}

	secProfileReqMsg := &halproto.SecurityProfileRequestMsg{
		Request: []*halproto.SecurityProfileSpec{
			{
				KeyOrHandle: &halproto.SecurityProfileKeyHandle{
					KeyOrHandle: &halproto.SecurityProfileKeyHandle_ProfileId{
						ProfileId: uint32(profile.Status.SecurityProfileID),
					},
				},

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
				PolicyEnforceEn:        true,

				// Default Normalization action allow
				IpDfAction:            constants.IPDFAction,
				IpOptionsAction:       constants.IPOptionsAction,
				IcmpInvalidCodeAction: constants.ICMPInvalidCodeAction,

				// Default Normalization actino edit
				IpInvalidLenAction:          constants.IPInvalidLenAction,
				TcpUnexpectedMssAction:      constants.TCPUnexpectedMSSAction,
				TcpUnexpectedWinScaleAction: constants.TCPUnexpectedWinScaleAction,
				TcpUnexpectedSackPermAction: constants.TCPUnexpectedSACKPermAction,
				TcpUrgPtrNotSetAction:       constants.TCPUrgentPtrNotSetAction,
				TcpUrgFlagNotSetAction:      constants.TCPUrgentFlagNotSetAction,
				TcpUrgPayloadMissingAction:  constants.TCPUrgentPayloadMissingAction,
				TcpDataLenGtMssAction:       constants.TCPDataLenGreaterThanMSSAction,
				TcpDataLenGtWinSizeAction:   constants.TCPDataLenGreaterThanWinSizeAction,
				TcpUnexpectedTsOptionAction: constants.TCPUnexpectedTSOptionAction,

				// Default Normalization action drop
				TcpUnexpectedEchoTsAction:     constants.TCPUnexpectedEchoTSAction,
				TcpUnexpectedSackOptionAction: constants.TCPUnexpectedSACKOptionAction,
				TcpRsvdFlagsAction:            constants.TCPReservedFlagsAction,

				// Default Normalization action none
				TcpRstWithDataAction: constants.TCPRSTWithDataAction,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Sgclient.SecurityProfileUpdate(context.Background(), secProfileReqMsg)
		if err != nil {
			log.Errorf("Error updating security profile. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Sgclient.SecurityProfileUpdate(context.Background(), secProfileReqMsg)
		if err != nil {
			log.Errorf("Error updating security profile. Err: %v", err)
			return err
		}
	}

	// Update the vrf spec in hal to point to the security profile that was created
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Tnclient.VrfUpdate(context.Background(), &vrfUpdateMsg)
		if err != nil {
			log.Errorf("Error updating vrf after security profile create. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Tnclient.VrfUpdate(context.Background(), &vrfUpdateMsg)
		if err != nil {
			log.Errorf("Error updating vrf after security profile create. Err: %v", err)
			return err
		}
	}

	return nil
}

// DeleteSecurityProfile deletes a security profile in the datapath
func (hd *Datapath) DeleteSecurityProfile(profile *netproto.SecurityProfile, attachmentVrfs []*netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	var vrfSpec []*halproto.VrfSpec

	for _, vrf := range attachmentVrfs {
		var vrfType halproto.VrfType
		if strings.ToLower(vrf.Spec.VrfType) == "infra" {
			vrfType = halproto.VrfType_VRF_TYPE_INFRA
		} else {
			vrfType = halproto.VrfType_VRF_TYPE_CUSTOMER
		}

		// Build vrfUpdateMessage
		spec := halproto.VrfSpec{
			KeyOrHandle: &halproto.VrfKeyHandle{
				KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
					VrfId: vrf.Status.VrfID,
				},
			},
			// All tenant creates are currently customer type as we don't intend to expose infra vrf creates to the user.
			VrfType: vrfType,
		}
		vrfSpec = append(vrfSpec, &spec)
	}

	vrfUpdateMsg := halproto.VrfRequestMsg{
		Request: vrfSpec,
	}

	sgProfileDelReq := &halproto.SecurityProfileDeleteRequestMsg{
		Request: []*halproto.SecurityProfileDeleteRequest{
			{
				KeyOrHandle: &halproto.SecurityProfileKeyHandle{
					KeyOrHandle: &halproto.SecurityProfileKeyHandle_ProfileId{
						ProfileId: uint32(profile.Status.SecurityProfileID),
					},
				},
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Sgclient.SecurityProfileDelete(context.Background(), sgProfileDelReq)
		if err != nil {
			log.Errorf("Error deleting security profile. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Sgclient.SecurityProfileDelete(context.Background(), sgProfileDelReq)
		if err != nil {
			log.Errorf("Error deleting security profile. Err: %v", err)
			return err
		}
	}

	// Update the vrf spec in hal to point to the security profile that was created
	if hd.Kind == "hal" {
		resp, err := hd.Hal.Tnclient.VrfUpdate(context.Background(), &vrfUpdateMsg)
		if err != nil {
			log.Errorf("Error updating vrf after security profile create. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Tnclient.VrfUpdate(context.Background(), &vrfUpdateMsg)
		if err != nil {
			log.Errorf("Error updating vrf after security profile create. Err: %v", err)
			return err
		}
	}

	return nil
}
