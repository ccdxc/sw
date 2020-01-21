// +build iris

package types

import (
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
)

// Constants
const (
	//-------------------------- Default Action Allow --------------------------

	// IPDFAction is set to allow for IP Do not fragment
	IPDFAction = halapi.NormalizationAction_NORM_ACTION_ALLOW

	// IPOptionsAction is set to allow by defaukt
	IPOptionsAction = halapi.NormalizationAction_NORM_ACTION_ALLOW

	//-------------------------- Default Action Edit --------------------------

	// IPInvalidLenAction is set to edit by default
	IPInvalidLenAction = halapi.NormalizationAction_NORM_ACTION_EDIT

	// TCPUnexpectedMSSAction is set to edit by default
	TCPUnexpectedMSSAction = halapi.NormalizationAction_NORM_ACTION_EDIT

	// TCPUnexpectedWinScaleAction is set to edit by default
	TCPUnexpectedWinScaleAction = halapi.NormalizationAction_NORM_ACTION_EDIT

	// TCPUnexpectedSACKPermAction is set to edit by default
	TCPUnexpectedSACKPermAction = halapi.NormalizationAction_NORM_ACTION_EDIT

	// TCPUrgentPtrNotSetAction is set to edit by default
	TCPUrgentPtrNotSetAction = halapi.NormalizationAction_NORM_ACTION_EDIT

	// TCPUrgentFlagNotSetAction is set to edit by default
	TCPUrgentFlagNotSetAction = halapi.NormalizationAction_NORM_ACTION_EDIT

	// TCPUrgentPayloadMissingAction is set to edit by default
	TCPUrgentPayloadMissingAction = halapi.NormalizationAction_NORM_ACTION_EDIT

	// TCPDataLenGreaterThanMSSAction is set to edit by default
	TCPDataLenGreaterThanMSSAction = halapi.NormalizationAction_NORM_ACTION_EDIT

	// TCPDataLenGreaterThanWinSizeAction is set to edit by default
	TCPDataLenGreaterThanWinSizeAction = halapi.NormalizationAction_NORM_ACTION_EDIT

	// TCPUnexpectedTSOptionAction is set to edit by default
	TCPUnexpectedTSOptionAction = halapi.NormalizationAction_NORM_ACTION_EDIT

	//-------------------------- Default Action Drop --------------------------

	// TCPUnexpectedEchoTSAction is set to drop by default
	TCPUnexpectedEchoTSAction = halapi.NormalizationAction_NORM_ACTION_DROP

	// ICMPInvalidCodeAction is set to allow by default
	ICMPInvalidCodeAction = halapi.NormalizationAction_NORM_ACTION_DROP

	// TCPUnexpectedSACKOptionAction is set to drop by default
	TCPUnexpectedSACKOptionAction = halapi.NormalizationAction_NORM_ACTION_DROP

	// TCPReservedFlagsAction is set to drop by default
	TCPReservedFlagsAction = halapi.NormalizationAction_NORM_ACTION_DROP

	// TCPRSTWithDataAction is set to none by default
	TCPRSTWithDataAction = halapi.NormalizationAction_NORM_ACTION_DROP
)

var (
	// HalIPAddressAny captures hal proto object for any
	HalIPAddressAny = &halapi.IPAddressObj{
		Formats: &halapi.IPAddressObj_Address{
			Address: &halapi.Address{
				Address: &halapi.Address_Prefix{
					Prefix: &halapi.IPSubnet{
						Subnet: &halapi.IPSubnet_Ipv4Subnet{
							Ipv4Subnet: &halapi.IPPrefix{
								Address: &halapi.IPAddress{
									IpAf:   halapi.IPAddressFamily_IP_AF_INET,
									V4OrV6: &halapi.IPAddress_V4Addr{},
								},
							},
						},
					},
				},
			},
		},
	}
)
