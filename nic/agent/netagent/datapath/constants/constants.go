package constants

import (
	"math"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
)

const (
	// UntaggedCollVLAN identifies an untagged vlan
	UntaggedCollVLAN = 8191
	// DefaultTimeout for non user specified timeouts
	DefaultTimeout = math.MaxUint32
	// DefaultConnectionSetUpTimeout is the default connection set up timeout
	DefaultConnectionSetUpTimeout = 30

	//-------------------------- Default Action Allow --------------------------

	// IPDFAction is set to allow for IP Do not fragment
	IPDFAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	// IPOptionsAction is set to allow by defaukt
	IPOptionsAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	//-------------------------- Default Action Edit --------------------------

	// IPInvalidLenAction is set to edit by default
	IPInvalidLenAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	// TCPUnexpectedMSSAction is set to edit by default
    TCPUnexpectedMSSAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	// TCPUnexpectedWinScaleAction is set to edit by default
	TCPUnexpectedWinScaleAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	// TCPUnexpectedSACKPermAction is set to edit by default
	TCPUnexpectedSACKPermAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	// TCPUrgentPtrNotSetAction is set to edit by default
	TCPUrgentPtrNotSetAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	// TCPUrgentFlagNotSetAction is set to edit by default
	TCPUrgentFlagNotSetAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	// TCPUrgentPayloadMissingAction is set to edit by default
	TCPUrgentPayloadMissingAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	// TCPDataLenGreaterThanMSSAction is set to edit by default
	TCPDataLenGreaterThanMSSAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	// TCPDataLenGreaterThanWinSizeAction is set to edit by default
	TCPDataLenGreaterThanWinSizeAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	// TCPUnexpectedTSOptionAction is set to edit by default
	TCPUnexpectedTSOptionAction = halproto.NormalizationAction_NORM_ACTION_ALLOW

	//-------------------------- Default Action Drop --------------------------

	// TCPUnexpectedEchoTSAction is set to drop by default
	TCPUnexpectedEchoTSAction = halproto.NormalizationAction_NORM_ACTION_DROP

	// ICMPInvalidCodeAction is set to allow by default
	ICMPInvalidCodeAction = halproto.NormalizationAction_NORM_ACTION_DROP

	// TCPUnexpectedSACKOptionAction is set to drop by default
	TCPUnexpectedSACKOptionAction = halproto.NormalizationAction_NORM_ACTION_DROP

	// TCPReservedFlagsAction is set to drop by default
	TCPReservedFlagsAction = halproto.NormalizationAction_NORM_ACTION_DROP

	// TCPRSTWithDataAction is set to none by default
	TCPRSTWithDataAction = halproto.NormalizationAction_NORM_ACTION_DROP

	//--------------------------- Default ALG Option ---------------------------

	// DefaultDNSMaxMessageLength is set to 8192 by default
	DefaultDNSMaxMessageLength = 8192
)
