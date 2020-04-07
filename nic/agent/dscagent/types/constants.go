package types

import (
	"math"
	"regexp"
	"time"
)

// TODO Uncomment when we support full range of IANA Protocol numbers
//// Protocol captures IANA Number based protocol
//type Protocol int

// Operation is the type of the CRUD Operation
type Operation int

// CloudPipelineKinds captures all the objects that agent will watch from Venice
var CloudPipelineKinds = []string{"Interface", "Collector", "IPAMPolicy", "App", "NetworkSecurityPolicy", "Vrf", "Network", "Endpoint", "SecurityProfile", "RouteTable", "RoutingConfig"}

// BaseNetKinds captures all the objects needed for Transparent Basenet Profile
var BaseNetKinds = []string{"Profile", "IPAMPolicy", "Interface", "Collector"}

// FlowAwareKinds catpures all the objects needed for FlowAware Profile
var FlowAwareKinds = []string{"Profile", "IPAMPolicy", "Interface", "Collector", "MirrorSession", "FlowExportPolicy"}

var EnforcedKinds = []string{"Profile", "IPAMPolicy", "Interface", "Collector", "MirrorSession", "FlowExportPolicy", "App", "NetworkSecurityPolicy", "SecurityProfile"}

// InsertionKinds captures all the objects needed for Insertion USeg Enforced Profile
var InsertionKinds = []string{"Profile", "IPAMPolicy", "Interface", "Collector", "MirrorSession", "FlowExportPolicy", "App", "NetworkSecurityPolicy", "Network", "Endpoint", "SecurityProfile"}

// IPAddressKind captures the type of IPAddress options, Singleton, Hyphen separated range or CIDR
type IPAddressKind int

// Const Opers
const (
	Get = iota
	List
	Create
	Update
	Delete
)

// ALG Bitmap Constants
const (
	AlgDNS = iota
	AlgFTP
	AlgICMP
	AlgMSRPC
	AlgRTSP
	AlgSIP
	AlgSUNRPC
	AlgTFTP
)

// IPAddress type
const (
	// IPAddressSingleton specifies a single IP Address
	IPAddressSingleton = iota

	// IPAddressCIDR specifies IPAddress in CIDR Format
	IPAddressCIDR

	// IPAddressRange specifies hyphen separated IP Address range
	IPAddressRange

	// IPAddressAny captures * for IPAddresses
	IPAddressAny
)

// Port Type
const (
	// PortSingleton specifies a single port
	PortSingleton = iota

	// PortRange specifies hyphen separated port range
	PortRange
)

// Constants
const (
	// NetagentLogFile is the default log file location for netagent
	NetagentLogFile = "/var/log/pensando/pen-netagent.log"

	// NetagentPrimaryDBPath is the primary DB for netagent to store configs
	NetagentPrimaryDBPath = "/data/pen-netagent.db"

	// NetagentBackupDBPath is the backup DB for netagent to store configs
	NetagentBackupDBPath = "/data/pen-netagent.db.bak"

	// NPM is the resolver name for NPM
	Npm = "pen-npm"

	// Tsm is the resolver name for TSM
	Tsm = "pen-tsm"

	// Tpm is the resolver name for Tpm
	Tpm = "pen-tpm"

	// Netagent is the default name for Netagent module
	Netagent = "pen-netagent"

	//Collector is the default name for debug stats collector
	Collector = "pen-collector"

	// StatsSendInterval captures how frequently netagent updates stats
	StatsSendInterval = time.Second * 30

	// SyncInterval captures how frequently TSM Controller resyncs for Mirror objects
	SyncInterval = time.Minute * 5

	// StatsRetryInterval captures retry interval for connections
	StatsRetryInterval = time.Millisecond * 100

	// DefaultAgentRestURL is netagent's default REST Endpoint
	DefaultAgentRestURL = "127.0.0.1:9007"

	// ControllerWaitDelay is the duration during for which netagent waits to reconnect to the controllers
	ControllerWaitDelay = time.Minute * 1

	// DefaultVrf is the name of the default vrf
	DefaultVrf = "default"

	// DefaulUnderlaytVrf is the name of the default underlay vrf
	DefaulUnderlaytVrf = "underlay-vpc"

	// DefaultNamespace is the name of the default namespace
	DefaultNamespace = "default"

	// DefaultTenant is the name of the default tenant
	DefaultTenant = "default"

	// HalGRPCDefaultBaseURL is the default endpoint for HAL Svc
	HalGRPCDefaultBaseURL = "127.0.0.1"

	// HalGRPCDefaultPort is the default port for HAL
	HalGRPCDefaultPort = "50054"

	// HalGRPCDefaultURL is the default HAL gRPC URL
	HalGRPCDefaultURL = "127.0.0.1:50054"

	// HalGRPCWaitTimeout is the timeout for which agent will wait for HAL to be up
	HalGRPCWaitTimeout = time.Minute * 10

	// HalGRPCTickerDuration is the retry duration trying to connect to HAL
	HalGRPCTickerDuration = time.Millisecond * 500

	// ARPResolutionTimeout is the maximum time agent will wait for ARP to be resolved.
	ARPResolutionTimeout = time.Duration(time.Second * 3)

	// UntaggedCollVLAN is the VLAN ID for untagged network. This is used as L2Seg for Collectors
	UntaggedCollVLAN = 8191

	// InternalDefaultUntaggedNetwork is the name of untagged l2seg that agent creates on init. This is used by collector EPs
	InternalDefaultUntaggedNetwork = "_internal_untagged_nw"

	// TunnelMTU is the default MTU for tunnels
	TunnelMTU = 9192

	// TunnelTTL is the default TTL value for tunnels
	TunnelTTL = 64

	// DefaultTimeout for non user specified timeouts
	DefaultTimeout = math.MaxUint32

	// LifPrefix is the name prefix for the LIFs. TODO Change this once NetworkInterface canonical naming gets formalized
	LifPrefix = "lif-"

	// UplinkPrefix is the name prefix for the Uplink interfaces.  TODO Change this once NetworkInterface canonical naming gets formalized
	UplinkPrefix = "uplink-"

	// EthPrefix is the name prefix for the physical uplink ports. TODO Change this once NetworkInterface canonical naming gets formalized
	EthPrefix = "eth-"

	// DefaultGCDuration captures the frequency of garbage collection of marked go-routine's heap space
	DefaultGCDuration = time.Minute

	// DefaultTemplateDuration captures the frequency of sending template packets to the netflow collector
	DefaultTemplateDuration = time.Minute * 5

	// DefaultConnectionSetUpTimeout is the default connection set up timeout
	DefaultConnectionSetUpTimeout = 30

	// NaplesOOBInterface is the inteface name of OOB Management Interface
	NaplesOOBInterface = "oob_mnic0"

	//--------------------------- Default ALG Option ---------------------------

	// DefaultDNSMaxMessageLength is set to 8192 by default
	DefaultDNSMaxMessageLength = 8192

	// IPFIXSrcPort is the default src port for ipfix template packets
	IPFIXSrcPort = 32007

	// DefaultNetflowExportPort is the default src port for collector
	DefaultNetflowExportPort = 2055

	// MaxMirrorSessions is the maximum number of mirror sessions allowed.
	MaxMirrorSessions = 8

	// MaxCollectorsPerFlow is the maximum number of collectors per flow export policy.
	MaxCollectorsPerFlow = 4

	// MaxCollectors is the maximum number of collectors
	MaxCollectors = 16
)

// ID Allocation constants and offsets
const (
	// AppID captures kind for ID allocation for App
	AppID = "appID"

	// EnicID captures kind for ID allocation for Enic
	EnicID = "enicID"

	// FlowMonitorRuleID captures kind for ID allocation for HAL Flow Monitor Rule used in telemetry CRUDs
	FlowMonitorRuleID = "flowMonitorRuleID"

	// CollectorID captures kind for ID allocation for HAL collector used in FlowExportPolicy CRUDs
	CollectorID = "collectorID"

	// InterfaceID captures kind for ID allocation for Interface
	InterfaceID = "interfaceID"

	// MirrorSessionID captures kind for ID allocation for MirrorSession
	MirrorSessionID = "mirrorSessionID"

	// FlowExportPolicyID captures kind for ID allocation for FlowExportPolicy
	FlowExportPolicyID = "flowExportPolicyID"

	// NetworkID captures kind for ID allocation for Network
	NetworkID = "networkID"

	// NetworkSecurityPolicyID captures kind for ID allocation for NetworkSecurityPolicy
	NetworkSecurityPolicyID = "networkSecurityPolicyID"

	// SecurityProfileID captures kind for ID allocation for SecurityProfile
	SecurityProfileID = "securityProfileID"

	// SecurityRulePriority captures the priority of a security rule
	SecurityRulePriority = "securityRulePriority"
	// DSCProfileID captures kind for ID allocation for DSCProfile
	DSCProfileID = "dscProfileID"

	// TunnelID captures kind for ID allocation for Tunnel
	TunnelID = "tunnelID"

	// VrfID captures kind for ID allocation for Vrf
	VrfID = "vrfID"

	// IPAMPolicyID captures kind for ID allocation for IPAMPolicy
	IPAMPolicyID = "ipamPolicyID"

	// EnicOffset captures ID Alloc space for Enic
	EnicOffset = UplinkOffset + 10000

	// NetworkOffSet captures ID Alloc space for Network
	NetworkOffSet = 100

	// SecurityProfileOffSet captures ID Alloc space for SecurityProfile
	SecurityProfileOffSet = 15

	// DSCProfileOffSet captures ID Alloc space for DSCProfile
	DSCProfileOffset = 0 // check with abhi if we need this for DSCProfile

	// TunnelOffset captures ID Alloc space for Tunnel
	TunnelOffset = UplinkOffset + 20000

	// UplinkOffset captures ID Alloc space for Uplink
	UplinkOffset = 127

	// VrfOffSet captures ID Alloc space for Vrf
	VrfOffSet = 65
)

const (
	// DefaultTCPIdleTimeout captures idle TCP timeout
	DefaultTCPIdleTimeout = 60

	// DefaultUDPIdleTimeout captures idle UDP timeout
	DefaultUDPIdleTimeout = 120

	// DefaultICMPIdleTimeout captures idle ICMP timeout
	DefaultICMPIdleTimeout = 15

	// DefaultOtherIdleTimeout captures idle timeout for other protocol
	DefaultOtherIdleTimeout = 90

	// DefaultTCPCnxnSetupTimeout captures TCP connection setup timeout
	DefaultTCPCnxnSetupTimeout = 10

	// DefaultTCPHalfCloseTimeout captures TCP half close timeout
	DefaultTCPHalfCloseTimeout = 120

	// DefaultTCPCloseTimeout captures TCP close timeout
	DefaultTCPCloseTimeout = 15

	// DefaultTCPDropTimeout captures TCP drop timeout
	DefaultTCPDropTimeout = 90

	// DefaultUDPDropTimeout captures UDP drop timeout
	DefaultUDPDropTimeout = 60

	// DefaultICMPDropTimeout captures ICMP drop timeout
	DefaultICMPDropTimeout = 30

	// DefaultOtherDropTimeout captures drop timeout for other timeout
	DefaultOtherDropTimeout = 60
)

// TODO Uncomment when we support full range of IANA Protocol numbers
//// Protocol constants. This will enable support for IANA number based match criteria.
//const (
//	ProtocolIP             = 0   // IPv4 encapsulation, pseudo protocol number
//	ProtocolHOPOPT         = 0   // IPv6 Hop-by-Hop Option
//	ProtocolICMP           = 1   // Internet Control Message
//	ProtocolIGMP           = 2   // Internet Group Management
//	ProtocolGGP            = 3   // Gateway-to-Gateway
//	ProtocolIPv4           = 4   // IPv4 encapsulation
//	ProtocolST             = 5   // Stream
//	ProtocolTCP            = 6   // Transmission Control
//	ProtocolCBT            = 7   // CBT
//	ProtocolEGP            = 8   // Exterior Gateway Protocol
//	ProtocolIGP            = 9   // any private interior gateway (used by Cisco for their IGRP)
//	ProtocolBBNRCCMON      = 10  // BBN RCC Monitoring
//	ProtocolNVPII          = 11  // Network Voice Protocol
//	ProtocolPUP            = 12  // PUP
//	ProtocolEMCON          = 14  // EMCON
//	ProtocolXNET           = 15  // Cross Net Debugger
//	ProtocolCHAOS          = 16  // Chaos
//	ProtocolUDP            = 17  // User Datagram
//	ProtocolMUX            = 18  // Multiplexing
//	ProtocolDCNMEAS        = 19  // DCN Measurement Subsystems
//	ProtocolHMP            = 20  // Host Monitoring
//	ProtocolPRM            = 21  // Packet Radio Measurement
//	ProtocolXNSIDP         = 22  // XEROX NS IDP
//	ProtocolTRUNK1         = 23  // Trunk-1
//	ProtocolTRUNK2         = 24  // Trunk-2
//	ProtocolLEAF1          = 25  // Leaf-1
//	ProtocolLEAF2          = 26  // Leaf-2
//	ProtocolRDP            = 27  // Reliable Data Protocol
//	ProtocolIRTP           = 28  // Internet Reliable Transaction
//	ProtocolISOTP4         = 29  // ISO Transport Protocol Class 4
//	ProtocolNETBLT         = 30  // Bulk Data Transfer Protocol
//	ProtocolMFENSP         = 31  // MFE Network Services Protocol
//	ProtocolMERITINP       = 32  // MERIT Internodal Protocol
//	ProtocolDCCP           = 33  // Datagram Congestion Control Protocol
//	Protocol3PC            = 34  // Third Party Connect Protocol
//	ProtocolIDPR           = 35  // Inter-Domain Policy Routing Protocol
//	ProtocolXTP            = 36  // XTP
//	ProtocolDDP            = 37  // Datagram Delivery Protocol
//	ProtocolIDPRCMTP       = 38  // IDPR Control Message Transport Proto
//	ProtocolTPPP           = 39  // TP++ Transport Protocol
//	ProtocolIL             = 40  // IL Transport Protocol
//	ProtocolIPv6           = 41  // IPv6 encapsulation
//	ProtocolSDRP           = 42  // Source Demand Routing Protocol
//	ProtocolIPv6Route      = 43  // Routing Header for IPv6
//	ProtocolIPv6Frag       = 44  // Fragment Header for IPv6
//	ProtocolIDRP           = 45  // Inter-Domain Routing Protocol
//	ProtocolRSVP           = 46  // Reservation Protocol
//	ProtocolGRE            = 47  // Generic Routing Encapsulation
//	ProtocolDSR            = 48  // Dynamic Source Routing Protocol
//	ProtocolBNA            = 49  // BNA
//	ProtocolESP            = 50  // Encap Security Payload
//	ProtocolAH             = 51  // Authentication Header
//	ProtocolINLSP          = 52  // Integrated Net Layer Security  TUBA
//	ProtocolNARP           = 54  // NBMA Address Resolution Protocol
//	ProtocolMOBILE         = 55  // IP Mobility
//	ProtocolTLSP           = 56  // Transport Layer Security Protocol using Kryptonet key management
//	ProtocolSKIP           = 57  // SKIP
//	ProtocolIPv6ICMP       = 58  // ICMP for IPv6
//	ProtocolIPv6NoNxt      = 59  // No Next Header for IPv6
//	ProtocolIPv6Opts       = 60  // Destination Options for IPv6
//	ProtocolCFTP           = 62  // CFTP
//	ProtocolSATEXPAK       = 64  // SATNET and Backroom EXPAK
//	ProtocolKRYPTOLAN      = 65  // Kryptolan
//	ProtocolRVD            = 66  // MIT Remote Virtual Disk Protocol
//	ProtocolIPPC           = 67  // Internet Pluribus Packet Core
//	ProtocolSATMON         = 69  // SATNET Monitoring
//	ProtocolVISA           = 70  // VISA Protocol
//	ProtocolIPCV           = 71  // Internet Packet Core Utility
//	ProtocolCPNX           = 72  // Computer Protocol Network Executive
//	ProtocolCPHB           = 73  // Computer Protocol Heart Beat
//	ProtocolWSN            = 74  // Wang Span Network
//	ProtocolPVP            = 75  // Packet Video Protocol
//	ProtocolBRSATMON       = 76  // Backroom SATNET Monitoring
//	ProtocolSUNND          = 77  // SUN ND PROTOCOL-Temporary
//	ProtocolWBMON          = 78  // WIDEBAND Monitoring
//	ProtocolWBEXPAK        = 79  // WIDEBAND EXPAK
//	ProtocolISOIP          = 80  // ISO Internet Protocol
//	ProtocolVMTP           = 81  // VMTP
//	ProtocolSECUREVMTP     = 82  // SECURE-VMTP
//	ProtocolVINES          = 83  // VINES
//	ProtocolTTP            = 84  // Transaction Transport Protocol
//	ProtocolIPTM           = 84  // Internet Protocol Traffic Manager
//	ProtocolNSFNETIGP      = 85  // NSFNET-IGP
//	ProtocolDGP            = 86  // Dissimilar Gateway Protocol
//	ProtocolTCF            = 87  // TCF
//	ProtocolEIGRP          = 88  // EIGRP
//	ProtocolOSPFIGP        = 89  // OSPFIGP
//	ProtocolSpriteRPC      = 90  // Sprite RPC Protocol
//	ProtocolLARP           = 91  // Locus Address Resolution Protocol
//	ProtocolMTP            = 92  // Multicast Transport Protocol
//	ProtocolAX25           = 93  // AX.25 Frames
//	ProtocolIPIP           = 94  // IP-within-IP Encapsulation Protocol
//	ProtocolSCCSP          = 96  // Semaphore Communications Sec. Pro.
//	ProtocolETHERIP        = 97  // Ethernet-within-IP Encapsulation
//	ProtocolENCAP          = 98  // Encapsulation Header
//	ProtocolGMTP           = 100 // GMTP
//	ProtocolIFMP           = 101 // Ipsilon Flow Management Protocol
//	ProtocolPNNI           = 102 // PNNI over IP
//	ProtocolPIM            = 103 // Protocol Independent Multicast
//	ProtocolARIS           = 104 // ARIS
//	ProtocolSCPS           = 105 // SCPS
//	ProtocolQNX            = 106 // QNX
//	ProtocolAN             = 107 // Active Networks
//	ProtocolIPComp         = 108 // IP Payload Compression Protocol
//	ProtocolSNP            = 109 // Sitara Networks Protocol
//	ProtocolCompaqPeer     = 110 // Compaq Peer Protocol
//	ProtocolIPXinIP        = 111 // IPX in IP
//	ProtocolVRRP           = 112 // Virtual Router Redundancy Protocol
//	ProtocolPGM            = 113 // PGM Reliable Transport Protocol
//	ProtocolL2TP           = 115 // Layer Two Tunneling Protocol
//	ProtocolDDX            = 116 // D-II Data Exchange (DDX)
//	ProtocolIATP           = 117 // Interactive Agent Transfer Protocol
//	ProtocolSTP            = 118 // Schedule Transfer Protocol
//	ProtocolSRP            = 119 // SpectraLink Radio Protocol
//	ProtocolUTI            = 120 // UTI
//	ProtocolSMP            = 121 // Simple Message Protocol
//	ProtocolPTP            = 123 // Performance Transparency Protocol
//	ProtocolISIS           = 124 // ISIS over IPv4
//	ProtocolFIRE           = 125 // FIRE
//	ProtocolCRTP           = 126 // Combat Radio Transport Protocol
//	ProtocolCRUDP          = 127 // Combat Radio User Datagram
//	ProtocolSSCOPMCE       = 128 // SSCOPMCE
//	ProtocolIPLT           = 129 // IPLT
//	ProtocolSPS            = 130 // Secure Packet Shield
//	ProtocolPIPE           = 131 // Private IP Encapsulation within IP
//	ProtocolSCTP           = 132 // Stream Control Transmission Protocol
//	ProtocolFC             = 133 // Fibre Channel
//	ProtocolRSVPE2EIGNORE  = 134 // RSVP-E2E-IGNORE
//	ProtocolMobilityHeader = 135 // Mobility Header
//	ProtocolUDPLite        = 136 // UDPLite
//	ProtocolMPLSinIP       = 137 // MPLS-in-IP
//	ProtocolMANET          = 138 // MANET Protocols
//	ProtocolHIP            = 139 // Host Identity Protocol
//	ProtocolShim6          = 140 // Shim6 Protocol
//	ProtocolWESP           = 141 // Wrapped Encapsulating Security Payload
//	ProtocolROHC           = 142 // Robust Header Compression
//	ProtocolReserved       = 255 // Reserved
//)

var (
	// MacStringRegex defines mac string regular expression
	MacStringRegex = regexp.MustCompile(`[^a-fA-F0-9]`)
)

// String implements stringer for Operation
func (o Operation) String() string {
	return [...]string{"Get", "List", "Create", "Update", "Delete"}[o]
}

// String implements stringer for IPAddressKind
func (i IPAddressKind) String() string {
	return [...]string{"IPAddressSingleton", "IPAddressCIDR", "IPAddressRange", "IPAddressAny"}[i]
}

// TODO Uncomment when we support full range of IANA Protocol numbers
//func (p Protocol) String() string {
//	return [...]string{
//		"IP",
//		"HOPOPT",
//		"ICMP",
//		"IGMP",
//		"GGP",
//		"IPv4",
//		"ST",
//		"TCP",
//		"CBT",
//		"EGP",
//		"IGP",
//		"BBNRCCMON",
//		"NVPII",
//		"PUP",
//		"EMCON",
//		"XNET",
//		"CHAOS",
//		"UDP",
//		"MUX",
//		"DCNMEAS",
//		"HMP",
//		"PRM",
//		"XNSIDP",
//		"TRUNK1",
//		"TRUNK2",
//		"LEAF1",
//		"LEAF2",
//		"RDP",
//		"IRTP",
//		"ISOTP4",
//		"NETBLT",
//		"MFENSP",
//		"MERITINP",
//		"DCCP",
//		"3PC",
//		"IDPR",
//		"XTP",
//		"DDP",
//		"IDPRCMTP",
//		"TPPP",
//		"IL",
//		"IPv6",
//		"SDRP",
//		"IPv6Route",
//		"IPv6Frag",
//		"IDRP",
//		"RSVP",
//		"GRE",
//		"DSR",
//		"BNA",
//		"ESP",
//		"AH",
//		"INLSP",
//		"NARP",
//		"MOBILE",
//		"TLSP",
//		"SKIP",
//		"IPv6ICMP",
//		"IPv6NoNxt",
//		"IPv6Opts",
//		"CFTP",
//		"SATEXPAK",
//		"KRYPTOLAN",
//		"RVD",
//		"IPPC",
//		"SATMON",
//		"VISA",
//		"IPCV",
//		"CPNX",
//		"CPHB",
//		"WSN",
//		"PVP",
//		"BRSATMON",
//		"SUNND",
//		"WBMON",
//		"WBEXPAK",
//		"ISOIP",
//		"VMTP",
//		"SECUREVMTP",
//		"VINES",
//		"TTP",
//		"IPTM",
//		"NSFNETIGP",
//		"DGP",
//		"TCF",
//		"EIGRP",
//		"OSPFIGP",
//		"SpriteRPC",
//		"LARP",
//		"MTP",
//		"AX25",
//		"IPIP",
//		"SCCSP",
//		"ETHERIP",
//		"ENCAP",
//		"GMTP",
//		"IFMP",
//		"PNNI",
//		"PIM",
//		"ARIS",
//		"SCPS",
//		"QNX",
//		"AN",
//		"IPComp",
//		"SNP",
//		"CompaqPeer",
//		"IPXinIP",
//		"VRRP",
//		"PGM",
//		"L2TP",
//		"DDX",
//		"IATP",
//		"STP",
//		"SRP",
//		"UTI",
//		"SMP",
//		"PTP",
//		"ISIS",
//		"FIRE",
//		"CRTP",
//		"CRUDP",
//		"SSCOPMCE",
//		"IPLT",
//		"SPS",
//		"PIPE",
//		"SCTP",
//		"FC",
//		"RSVPE2EIGNORE",
//		"MobilityHeader",
//		"UDPLite",
//		"MPLSinIP",
//		"MANET",
//		"HIP",
//		"Shim6",
//		"WESP",
//		"ROHC",
//		"Reserved",
//	}[p]
//}
