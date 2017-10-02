// Code generated by protoc-gen-go. DO NOT EDIT.
// source: nic/proto/types.proto

package halproto

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// IP protocol numbers
type IPProtocol int32

const (
	IPProtocol_IPPROTO_NONE   IPProtocol = 0
	IPProtocol_IPPROTO_ICMP   IPProtocol = 1
	IPProtocol_IPPROTO_IPV4   IPProtocol = 4
	IPProtocol_IPPROTO_TCP    IPProtocol = 6
	IPProtocol_IPPROTO_UDP    IPProtocol = 17
	IPProtocol_IPPROTO_GRE    IPProtocol = 47
	IPProtocol_IPPROTO_ESP    IPProtocol = 50
	IPProtocol_IPPROTO_AH     IPProtocol = 51
	IPProtocol_IPPROTO_ICMPV6 IPProtocol = 58
)

var IPProtocol_name = map[int32]string{
	0:  "IPPROTO_NONE",
	1:  "IPPROTO_ICMP",
	4:  "IPPROTO_IPV4",
	6:  "IPPROTO_TCP",
	17: "IPPROTO_UDP",
	47: "IPPROTO_GRE",
	50: "IPPROTO_ESP",
	51: "IPPROTO_AH",
	58: "IPPROTO_ICMPV6",
}
var IPProtocol_value = map[string]int32{
	"IPPROTO_NONE":   0,
	"IPPROTO_ICMP":   1,
	"IPPROTO_IPV4":   4,
	"IPPROTO_TCP":    6,
	"IPPROTO_UDP":    17,
	"IPPROTO_GRE":    47,
	"IPPROTO_ESP":    50,
	"IPPROTO_AH":     51,
	"IPPROTO_ICMPV6": 58,
}

func (x IPProtocol) String() string {
	return proto.EnumName(IPProtocol_name, int32(x))
}
func (IPProtocol) EnumDescriptor() ([]byte, []int) { return fileDescriptor24, []int{0} }

// ICMP message types
// TBD - add more here !!
type ICMPMsgType int32

const (
	ICMPMsgType_ICMP_MSG_TYPE_NONE ICMPMsgType = 0
	ICMPMsgType_ICMP_MSG_TYPE_ECHO ICMPMsgType = 1
)

var ICMPMsgType_name = map[int32]string{
	0: "ICMP_MSG_TYPE_NONE",
	1: "ICMP_MSG_TYPE_ECHO",
}
var ICMPMsgType_value = map[string]int32{
	"ICMP_MSG_TYPE_NONE": 0,
	"ICMP_MSG_TYPE_ECHO": 1,
}

func (x ICMPMsgType) String() string {
	return proto.EnumName(ICMPMsgType_name, int32(x))
}
func (ICMPMsgType) EnumDescriptor() ([]byte, []int) { return fileDescriptor24, []int{1} }

// API return codes (more descriptive than protobuf enums)
type ApiStatus int32

const (
	ApiStatus_API_STATUS_OK                           ApiStatus = 0
	ApiStatus_API_STATUS_ERR                          ApiStatus = 1
	ApiStatus_API_STATUS_INVALID_ARG                  ApiStatus = 2
	ApiStatus_API_STATUS_EXISTS_ALREADY               ApiStatus = 3
	ApiStatus_API_STATUS_OUT_OF_MEM                   ApiStatus = 4
	ApiStatus_API_STATUS_NOT_FOUND                    ApiStatus = 5
	ApiStatus_API_STATUS_OUT_OF_RESOURCE              ApiStatus = 6
	ApiStatus_API_STATUS_ENCAP_INVALID                ApiStatus = 7
	ApiStatus_API_STATUS_TENANT_ID_INVALID            ApiStatus = 8
	ApiStatus_API_STATUS_TENANT_NOT_FOUND             ApiStatus = 9
	ApiStatus_API_STATUS_L2_SEGMENT_ID_INVALID        ApiStatus = 10
	ApiStatus_API_STATUS_L2_SEGMENT_NOT_FOUND         ApiStatus = 11
	ApiStatus_API_STATUS_INTERFACE_ID_INVALID         ApiStatus = 12
	ApiStatus_API_STATUS_INTERFACE_NOT_FOUND          ApiStatus = 13
	ApiStatus_API_STATUS_IF_TYPE_INVALID              ApiStatus = 14
	ApiStatus_API_STATUS_IF_INFO_INVALID              ApiStatus = 15
	ApiStatus_API_STATUS_IF_L2SEGMENT_INVALID         ApiStatus = 16
	ApiStatus_API_STATUS_IF_ENIC_INFO_INVALID         ApiStatus = 17
	ApiStatus_API_STATUS_IF_ENIC_TYPE_INVALID         ApiStatus = 18
	ApiStatus_API_STATUS_IF_LIF_INFO_NOT_INVALID      ApiStatus = 19
	ApiStatus_API_STATUS_IF_LIFQ_INFO_NOT_INVALID     ApiStatus = 20
	ApiStatus_API_STATUS_ENDPOINT_NOT_FOUND           ApiStatus = 21
	ApiStatus_API_STATUS_FLOW_KEY_INVALID             ApiStatus = 22
	ApiStatus_API_STATUS_FLOW_INFO_INVALID            ApiStatus = 23
	ApiStatus_API_STATUS_FLOW_NOT_FOUND               ApiStatus = 24
	ApiStatus_API_STATUS_SESSION_NOT_FOUND            ApiStatus = 25
	ApiStatus_API_STATUS_HANDLE_INVALID               ApiStatus = 26
	ApiStatus_API_STATUS_HW_PROG_ERR                  ApiStatus = 27
	ApiStatus_API_STATUS_LIF_ID_INVALID               ApiStatus = 28
	ApiStatus_API_STATUS_LIF_NOT_FOUND                ApiStatus = 29
	ApiStatus_API_STATUS_NWSEC_PROFILE_ID_INVALID     ApiStatus = 30
	ApiStatus_API_STATUS_NWSEC_PROFILE_NOT_FOUND      ApiStatus = 31
	ApiStatus_API_STATUS_TLS_CB_ID_INVALID            ApiStatus = 32
	ApiStatus_API_STATUS_TLS_CB_NOT_FOUND             ApiStatus = 33
	ApiStatus_API_STATUS_TCP_CB_ID_INVALID            ApiStatus = 34
	ApiStatus_API_STATUS_TCP_CB_NOT_FOUND             ApiStatus = 35
	ApiStatus_API_STATUS_BUF_POOL_ID_INVALID          ApiStatus = 36
	ApiStatus_API_STATUS_BUF_POOL_NOT_FOUND           ApiStatus = 37
	ApiStatus_API_STATUS_BUF_POOL_COS_MAP_EXISTS      ApiStatus = 38
	ApiStatus_API_STATUS_QUEUE_ID_INVALID             ApiStatus = 39
	ApiStatus_API_STATUS_QUEUE_COUNT_INVALID          ApiStatus = 40
	ApiStatus_API_STATUS_QUEUE_NOT_FOUND              ApiStatus = 41
	ApiStatus_API_STATUS_POLICER_ID_INVALID           ApiStatus = 42
	ApiStatus_API_STATUS_POLICER_NOT_FOUND            ApiStatus = 43
	ApiStatus_API_STATUS_NETWORK_NOT_FOUND            ApiStatus = 44
	ApiStatus_API_STATUS_ACL_ID_INVALID               ApiStatus = 45
	ApiStatus_API_STATUS_ACL_NOT_FOUND                ApiStatus = 46
	ApiStatus_API_STATUS_WRING_ID_INVALID             ApiStatus = 47
	ApiStatus_API_STATUS_WRING_NOT_FOUND              ApiStatus = 48
	ApiStatus_API_STATUS_WRING_TYPE_INVALID           ApiStatus = 49
	ApiStatus_API_STATUS_PROXY_TYPE_INVALID           ApiStatus = 50
	ApiStatus_API_STATUS_IPSEC_CB_ID_INVALID          ApiStatus = 51
	ApiStatus_API_STATUS_IPSEC_CB_NOT_FOUND           ApiStatus = 52
	ApiStatus_API_STATUS_L4LB_KEY_INVALID             ApiStatus = 53
	ApiStatus_API_STATUS_CPU_CB_ID_INVALID            ApiStatus = 54
	ApiStatus_API_STATUS_CPU_CB_NOT_FOUND             ApiStatus = 55
	ApiStatus_API_STATUS_PROXY_NOT_ENABLED            ApiStatus = 56
	ApiStatus_API_STATUS_PROXY_FLOW_NOT_FOUND         ApiStatus = 57
	ApiStatus_API_STATUS_CFG_DB_ERR                   ApiStatus = 58
	ApiStatus_API_STATUS_ENDPOINT_UPD_KEY_HDL_INVALID ApiStatus = 59
)

var ApiStatus_name = map[int32]string{
	0:  "API_STATUS_OK",
	1:  "API_STATUS_ERR",
	2:  "API_STATUS_INVALID_ARG",
	3:  "API_STATUS_EXISTS_ALREADY",
	4:  "API_STATUS_OUT_OF_MEM",
	5:  "API_STATUS_NOT_FOUND",
	6:  "API_STATUS_OUT_OF_RESOURCE",
	7:  "API_STATUS_ENCAP_INVALID",
	8:  "API_STATUS_TENANT_ID_INVALID",
	9:  "API_STATUS_TENANT_NOT_FOUND",
	10: "API_STATUS_L2_SEGMENT_ID_INVALID",
	11: "API_STATUS_L2_SEGMENT_NOT_FOUND",
	12: "API_STATUS_INTERFACE_ID_INVALID",
	13: "API_STATUS_INTERFACE_NOT_FOUND",
	14: "API_STATUS_IF_TYPE_INVALID",
	15: "API_STATUS_IF_INFO_INVALID",
	16: "API_STATUS_IF_L2SEGMENT_INVALID",
	17: "API_STATUS_IF_ENIC_INFO_INVALID",
	18: "API_STATUS_IF_ENIC_TYPE_INVALID",
	19: "API_STATUS_IF_LIF_INFO_NOT_INVALID",
	20: "API_STATUS_IF_LIFQ_INFO_NOT_INVALID",
	21: "API_STATUS_ENDPOINT_NOT_FOUND",
	22: "API_STATUS_FLOW_KEY_INVALID",
	23: "API_STATUS_FLOW_INFO_INVALID",
	24: "API_STATUS_FLOW_NOT_FOUND",
	25: "API_STATUS_SESSION_NOT_FOUND",
	26: "API_STATUS_HANDLE_INVALID",
	27: "API_STATUS_HW_PROG_ERR",
	28: "API_STATUS_LIF_ID_INVALID",
	29: "API_STATUS_LIF_NOT_FOUND",
	30: "API_STATUS_NWSEC_PROFILE_ID_INVALID",
	31: "API_STATUS_NWSEC_PROFILE_NOT_FOUND",
	32: "API_STATUS_TLS_CB_ID_INVALID",
	33: "API_STATUS_TLS_CB_NOT_FOUND",
	34: "API_STATUS_TCP_CB_ID_INVALID",
	35: "API_STATUS_TCP_CB_NOT_FOUND",
	36: "API_STATUS_BUF_POOL_ID_INVALID",
	37: "API_STATUS_BUF_POOL_NOT_FOUND",
	38: "API_STATUS_BUF_POOL_COS_MAP_EXISTS",
	39: "API_STATUS_QUEUE_ID_INVALID",
	40: "API_STATUS_QUEUE_COUNT_INVALID",
	41: "API_STATUS_QUEUE_NOT_FOUND",
	42: "API_STATUS_POLICER_ID_INVALID",
	43: "API_STATUS_POLICER_NOT_FOUND",
	44: "API_STATUS_NETWORK_NOT_FOUND",
	45: "API_STATUS_ACL_ID_INVALID",
	46: "API_STATUS_ACL_NOT_FOUND",
	47: "API_STATUS_WRING_ID_INVALID",
	48: "API_STATUS_WRING_NOT_FOUND",
	49: "API_STATUS_WRING_TYPE_INVALID",
	50: "API_STATUS_PROXY_TYPE_INVALID",
	51: "API_STATUS_IPSEC_CB_ID_INVALID",
	52: "API_STATUS_IPSEC_CB_NOT_FOUND",
	53: "API_STATUS_L4LB_KEY_INVALID",
	54: "API_STATUS_CPU_CB_ID_INVALID",
	55: "API_STATUS_CPU_CB_NOT_FOUND",
	56: "API_STATUS_PROXY_NOT_ENABLED",
	57: "API_STATUS_PROXY_FLOW_NOT_FOUND",
	58: "API_STATUS_CFG_DB_ERR",
	59: "API_STATUS_ENDPOINT_UPD_KEY_HDL_INVALID",
}
var ApiStatus_value = map[string]int32{
	"API_STATUS_OK":                           0,
	"API_STATUS_ERR":                          1,
	"API_STATUS_INVALID_ARG":                  2,
	"API_STATUS_EXISTS_ALREADY":               3,
	"API_STATUS_OUT_OF_MEM":                   4,
	"API_STATUS_NOT_FOUND":                    5,
	"API_STATUS_OUT_OF_RESOURCE":              6,
	"API_STATUS_ENCAP_INVALID":                7,
	"API_STATUS_TENANT_ID_INVALID":            8,
	"API_STATUS_TENANT_NOT_FOUND":             9,
	"API_STATUS_L2_SEGMENT_ID_INVALID":        10,
	"API_STATUS_L2_SEGMENT_NOT_FOUND":         11,
	"API_STATUS_INTERFACE_ID_INVALID":         12,
	"API_STATUS_INTERFACE_NOT_FOUND":          13,
	"API_STATUS_IF_TYPE_INVALID":              14,
	"API_STATUS_IF_INFO_INVALID":              15,
	"API_STATUS_IF_L2SEGMENT_INVALID":         16,
	"API_STATUS_IF_ENIC_INFO_INVALID":         17,
	"API_STATUS_IF_ENIC_TYPE_INVALID":         18,
	"API_STATUS_IF_LIF_INFO_NOT_INVALID":      19,
	"API_STATUS_IF_LIFQ_INFO_NOT_INVALID":     20,
	"API_STATUS_ENDPOINT_NOT_FOUND":           21,
	"API_STATUS_FLOW_KEY_INVALID":             22,
	"API_STATUS_FLOW_INFO_INVALID":            23,
	"API_STATUS_FLOW_NOT_FOUND":               24,
	"API_STATUS_SESSION_NOT_FOUND":            25,
	"API_STATUS_HANDLE_INVALID":               26,
	"API_STATUS_HW_PROG_ERR":                  27,
	"API_STATUS_LIF_ID_INVALID":               28,
	"API_STATUS_LIF_NOT_FOUND":                29,
	"API_STATUS_NWSEC_PROFILE_ID_INVALID":     30,
	"API_STATUS_NWSEC_PROFILE_NOT_FOUND":      31,
	"API_STATUS_TLS_CB_ID_INVALID":            32,
	"API_STATUS_TLS_CB_NOT_FOUND":             33,
	"API_STATUS_TCP_CB_ID_INVALID":            34,
	"API_STATUS_TCP_CB_NOT_FOUND":             35,
	"API_STATUS_BUF_POOL_ID_INVALID":          36,
	"API_STATUS_BUF_POOL_NOT_FOUND":           37,
	"API_STATUS_BUF_POOL_COS_MAP_EXISTS":      38,
	"API_STATUS_QUEUE_ID_INVALID":             39,
	"API_STATUS_QUEUE_COUNT_INVALID":          40,
	"API_STATUS_QUEUE_NOT_FOUND":              41,
	"API_STATUS_POLICER_ID_INVALID":           42,
	"API_STATUS_POLICER_NOT_FOUND":            43,
	"API_STATUS_NETWORK_NOT_FOUND":            44,
	"API_STATUS_ACL_ID_INVALID":               45,
	"API_STATUS_ACL_NOT_FOUND":                46,
	"API_STATUS_WRING_ID_INVALID":             47,
	"API_STATUS_WRING_NOT_FOUND":              48,
	"API_STATUS_WRING_TYPE_INVALID":           49,
	"API_STATUS_PROXY_TYPE_INVALID":           50,
	"API_STATUS_IPSEC_CB_ID_INVALID":          51,
	"API_STATUS_IPSEC_CB_NOT_FOUND":           52,
	"API_STATUS_L4LB_KEY_INVALID":             53,
	"API_STATUS_CPU_CB_ID_INVALID":            54,
	"API_STATUS_CPU_CB_NOT_FOUND":             55,
	"API_STATUS_PROXY_NOT_ENABLED":            56,
	"API_STATUS_PROXY_FLOW_NOT_FOUND":         57,
	"API_STATUS_CFG_DB_ERR":                   58,
	"API_STATUS_ENDPOINT_UPD_KEY_HDL_INVALID": 59,
}

func (x ApiStatus) String() string {
	return proto.EnumName(ApiStatus_name, int32(x))
}
func (ApiStatus) EnumDescriptor() ([]byte, []int) { return fileDescriptor24, []int{2} }

// Types of L2 segments
type L2SegmentType int32

const (
	L2SegmentType_L2_SEGMENT_TYPE_NONE   L2SegmentType = 0
	L2SegmentType_L2_SEGMENT_TYPE_TENANT L2SegmentType = 1
	L2SegmentType_L2_SEGMENT_TYPE_MGMT   L2SegmentType = 2
	L2SegmentType_L2_SEGMENT_TYPE_INFRA  L2SegmentType = 3
	L2SegmentType_L2_SEGMENT_PRIMARY     L2SegmentType = 4
	L2SegmentType_L2_SEGMENT_ISOLATED    L2SegmentType = 5
)

var L2SegmentType_name = map[int32]string{
	0: "L2_SEGMENT_TYPE_NONE",
	1: "L2_SEGMENT_TYPE_TENANT",
	2: "L2_SEGMENT_TYPE_MGMT",
	3: "L2_SEGMENT_TYPE_INFRA",
	4: "L2_SEGMENT_PRIMARY",
	5: "L2_SEGMENT_ISOLATED",
}
var L2SegmentType_value = map[string]int32{
	"L2_SEGMENT_TYPE_NONE":   0,
	"L2_SEGMENT_TYPE_TENANT": 1,
	"L2_SEGMENT_TYPE_MGMT":   2,
	"L2_SEGMENT_TYPE_INFRA":  3,
	"L2_SEGMENT_PRIMARY":     4,
	"L2_SEGMENT_ISOLATED":    5,
}

func (x L2SegmentType) String() string {
	return proto.EnumName(L2SegmentType_name, int32(x))
}
func (L2SegmentType) EnumDescriptor() ([]byte, []int) { return fileDescriptor24, []int{3} }

// Supported wire encaps for the L2 segments
type EncapType int32

const (
	EncapType_ENCAP_TYPE_NONE     EncapType = 0
	EncapType_ENCAP_TYPE_DOT1Q    EncapType = 1
	EncapType_ENCAP_TYPE_VXLAN    EncapType = 2
	EncapType_ENCAP_TYPE_IP_IN_IP EncapType = 3
	EncapType_ENCAP_TYPE_GRE      EncapType = 4
	EncapType_ENCAP_TYPE_IPSEC    EncapType = 5
)

var EncapType_name = map[int32]string{
	0: "ENCAP_TYPE_NONE",
	1: "ENCAP_TYPE_DOT1Q",
	2: "ENCAP_TYPE_VXLAN",
	3: "ENCAP_TYPE_IP_IN_IP",
	4: "ENCAP_TYPE_GRE",
	5: "ENCAP_TYPE_IPSEC",
}
var EncapType_value = map[string]int32{
	"ENCAP_TYPE_NONE":     0,
	"ENCAP_TYPE_DOT1Q":    1,
	"ENCAP_TYPE_VXLAN":    2,
	"ENCAP_TYPE_IP_IN_IP": 3,
	"ENCAP_TYPE_GRE":      4,
	"ENCAP_TYPE_IPSEC":    5,
}

func (x EncapType) String() string {
	return proto.EnumName(EncapType_name, int32(x))
}
func (EncapType) EnumDescriptor() ([]byte, []int) { return fileDescriptor24, []int{4} }

// IP address families
type IPAddressFamily int32

const (
	IPAddressFamily_IP_AF_NONE  IPAddressFamily = 0
	IPAddressFamily_IP_AF_INET  IPAddressFamily = 1
	IPAddressFamily_IP_AF_INET6 IPAddressFamily = 2
)

var IPAddressFamily_name = map[int32]string{
	0: "IP_AF_NONE",
	1: "IP_AF_INET",
	2: "IP_AF_INET6",
}
var IPAddressFamily_value = map[string]int32{
	"IP_AF_NONE":  0,
	"IP_AF_INET":  1,
	"IP_AF_INET6": 2,
}

func (x IPAddressFamily) String() string {
	return proto.EnumName(IPAddressFamily_name, int32(x))
}
func (IPAddressFamily) EnumDescriptor() ([]byte, []int) { return fileDescriptor24, []int{5} }

// Types of Work Ring
type WRingType int32

const (
	WRingType_WRING_TYPE_NONE          WRingType = 0
	WRingType_WRING_TYPE_SERQ          WRingType = 1
	WRingType_WRING_TYPE_NMDR_TX       WRingType = 2
	WRingType_WRING_TYPE_NMDR_RX       WRingType = 3
	WRingType_WRING_TYPE_NMPR_SMALL_TX WRingType = 4
	WRingType_WRING_TYPE_NMPR_SMALL_RX WRingType = 5
	WRingType_WRING_TYPE_NMPR_BIG_TX   WRingType = 6
	WRingType_WRING_TYPE_NMPR_BIG_RX   WRingType = 7
	WRingType_WRING_TYPE_BSQ           WRingType = 8
	WRingType_WRING_TYPE_BRQ           WRingType = 9
	WRingType_WRING_TYPE_SESQ          WRingType = 10
	WRingType_WRING_TYPE_IPSECCBQ      WRingType = 11
	WRingType_WRING_TYPE_ARQRX         WRingType = 12
	WRingType_WRING_TYPE_ASQ           WRingType = 13
	WRingType_WRING_TYPE_ASESQ         WRingType = 14
)

var WRingType_name = map[int32]string{
	0:  "WRING_TYPE_NONE",
	1:  "WRING_TYPE_SERQ",
	2:  "WRING_TYPE_NMDR_TX",
	3:  "WRING_TYPE_NMDR_RX",
	4:  "WRING_TYPE_NMPR_SMALL_TX",
	5:  "WRING_TYPE_NMPR_SMALL_RX",
	6:  "WRING_TYPE_NMPR_BIG_TX",
	7:  "WRING_TYPE_NMPR_BIG_RX",
	8:  "WRING_TYPE_BSQ",
	9:  "WRING_TYPE_BRQ",
	10: "WRING_TYPE_SESQ",
	11: "WRING_TYPE_IPSECCBQ",
	12: "WRING_TYPE_ARQRX",
	13: "WRING_TYPE_ASQ",
	14: "WRING_TYPE_ASESQ",
}
var WRingType_value = map[string]int32{
	"WRING_TYPE_NONE":          0,
	"WRING_TYPE_SERQ":          1,
	"WRING_TYPE_NMDR_TX":       2,
	"WRING_TYPE_NMDR_RX":       3,
	"WRING_TYPE_NMPR_SMALL_TX": 4,
	"WRING_TYPE_NMPR_SMALL_RX": 5,
	"WRING_TYPE_NMPR_BIG_TX":   6,
	"WRING_TYPE_NMPR_BIG_RX":   7,
	"WRING_TYPE_BSQ":           8,
	"WRING_TYPE_BRQ":           9,
	"WRING_TYPE_SESQ":          10,
	"WRING_TYPE_IPSECCBQ":      11,
	"WRING_TYPE_ARQRX":         12,
	"WRING_TYPE_ASQ":           13,
	"WRING_TYPE_ASESQ":         14,
}

func (x WRingType) String() string {
	return proto.EnumName(WRingType_name, int32(x))
}
func (WRingType) EnumDescriptor() ([]byte, []int) { return fileDescriptor24, []int{6} }

// Types of Proxy Service
type ProxyType int32

const (
	ProxyType_PROXY_TYPE_NONE  ProxyType = 0
	ProxyType_PROXY_TYPE_TCP   ProxyType = 1
	ProxyType_PROXY_TYPE_TLS   ProxyType = 2
	ProxyType_PROXY_TYPE_IPSEC ProxyType = 3
	ProxyType_PROXY_TYPE_CPU   ProxyType = 4
	ProxyType_PROXY_TYPE_IPFIX ProxyType = 5
)

var ProxyType_name = map[int32]string{
	0: "PROXY_TYPE_NONE",
	1: "PROXY_TYPE_TCP",
	2: "PROXY_TYPE_TLS",
	3: "PROXY_TYPE_IPSEC",
	4: "PROXY_TYPE_CPU",
	5: "PROXY_TYPE_IPFIX",
}
var ProxyType_value = map[string]int32{
	"PROXY_TYPE_NONE":  0,
	"PROXY_TYPE_TCP":   1,
	"PROXY_TYPE_TLS":   2,
	"PROXY_TYPE_IPSEC": 3,
	"PROXY_TYPE_CPU":   4,
	"PROXY_TYPE_IPFIX": 5,
}

func (x ProxyType) String() string {
	return proto.EnumName(ProxyType_name, int32(x))
}
func (ProxyType) EnumDescriptor() ([]byte, []int) { return fileDescriptor24, []int{7} }

// Types of Cryptographic keys
type CryptoKeyType int32

const (
	CryptoKeyType_CRYPTO_KEY_TYPE_AES128   CryptoKeyType = 0
	CryptoKeyType_CRYPTO_KEY_TYPE_AES192   CryptoKeyType = 1
	CryptoKeyType_CRYPTO_KEY_TYPE_AES256   CryptoKeyType = 2
	CryptoKeyType_CRYPTO_KEY_TYPE_DES      CryptoKeyType = 3
	CryptoKeyType_CRYPTO_KEY_TYPE_CHACHA20 CryptoKeyType = 4
	CryptoKeyType_CRYPTO_KEY_TYPE_POLY1305 CryptoKeyType = 5
	CryptoKeyType_CRYPTO_KEY_TYPE_HMAC     CryptoKeyType = 6
)

var CryptoKeyType_name = map[int32]string{
	0: "CRYPTO_KEY_TYPE_AES128",
	1: "CRYPTO_KEY_TYPE_AES192",
	2: "CRYPTO_KEY_TYPE_AES256",
	3: "CRYPTO_KEY_TYPE_DES",
	4: "CRYPTO_KEY_TYPE_CHACHA20",
	5: "CRYPTO_KEY_TYPE_POLY1305",
	6: "CRYPTO_KEY_TYPE_HMAC",
}
var CryptoKeyType_value = map[string]int32{
	"CRYPTO_KEY_TYPE_AES128":   0,
	"CRYPTO_KEY_TYPE_AES192":   1,
	"CRYPTO_KEY_TYPE_AES256":   2,
	"CRYPTO_KEY_TYPE_DES":      3,
	"CRYPTO_KEY_TYPE_CHACHA20": 4,
	"CRYPTO_KEY_TYPE_POLY1305": 5,
	"CRYPTO_KEY_TYPE_HMAC":     6,
}

func (x CryptoKeyType) String() string {
	return proto.EnumName(CryptoKeyType_name, int32(x))
}
func (CryptoKeyType) EnumDescriptor() ([]byte, []int) { return fileDescriptor24, []int{8} }

// common meta object that is part of all other top level objects
// TODO: should we add last updated timestamp, revision etc. here ?
type ObjectMeta struct {
	TenantId uint32 `protobuf:"fixed32,1,opt,name=tenant_id,json=tenantId" json:"tenant_id,omitempty"`
}

func (m *ObjectMeta) Reset()                    { *m = ObjectMeta{} }
func (m *ObjectMeta) String() string            { return proto.CompactTextString(m) }
func (*ObjectMeta) ProtoMessage()               {}
func (*ObjectMeta) Descriptor() ([]byte, []int) { return fileDescriptor24, []int{0} }

func (m *ObjectMeta) GetTenantId() uint32 {
	if m != nil {
		return m.TenantId
	}
	return 0
}

// EncapInfo captures wire encap information
type EncapInfo struct {
	EncapType  EncapType `protobuf:"varint,1,opt,name=encap_type,json=encapType,enum=types.EncapType" json:"encap_type,omitempty"`
	EncapValue uint32    `protobuf:"varint,2,opt,name=encap_value,json=encapValue" json:"encap_value,omitempty"`
}

func (m *EncapInfo) Reset()                    { *m = EncapInfo{} }
func (m *EncapInfo) String() string            { return proto.CompactTextString(m) }
func (*EncapInfo) ProtoMessage()               {}
func (*EncapInfo) Descriptor() ([]byte, []int) { return fileDescriptor24, []int{1} }

func (m *EncapInfo) GetEncapType() EncapType {
	if m != nil {
		return m.EncapType
	}
	return EncapType_ENCAP_TYPE_NONE
}

func (m *EncapInfo) GetEncapValue() uint32 {
	if m != nil {
		return m.EncapValue
	}
	return 0
}

// IP address object
type IPAddress struct {
	IpAf IPAddressFamily `protobuf:"varint,1,opt,name=ip_af,json=ipAf,enum=types.IPAddressFamily" json:"ip_af,omitempty"`
	// Types that are valid to be assigned to V4OrV6:
	//	*IPAddress_V4Addr
	//	*IPAddress_V6Addr
	V4OrV6 isIPAddress_V4OrV6 `protobuf_oneof:"v4_or_v6"`
}

func (m *IPAddress) Reset()                    { *m = IPAddress{} }
func (m *IPAddress) String() string            { return proto.CompactTextString(m) }
func (*IPAddress) ProtoMessage()               {}
func (*IPAddress) Descriptor() ([]byte, []int) { return fileDescriptor24, []int{2} }

type isIPAddress_V4OrV6 interface {
	isIPAddress_V4OrV6()
}

type IPAddress_V4Addr struct {
	V4Addr uint32 `protobuf:"fixed32,2,opt,name=v4_addr,json=v4Addr,oneof"`
}
type IPAddress_V6Addr struct {
	V6Addr []byte `protobuf:"bytes,3,opt,name=v6_addr,json=v6Addr,proto3,oneof"`
}

func (*IPAddress_V4Addr) isIPAddress_V4OrV6() {}
func (*IPAddress_V6Addr) isIPAddress_V4OrV6() {}

func (m *IPAddress) GetV4OrV6() isIPAddress_V4OrV6 {
	if m != nil {
		return m.V4OrV6
	}
	return nil
}

func (m *IPAddress) GetIpAf() IPAddressFamily {
	if m != nil {
		return m.IpAf
	}
	return IPAddressFamily_IP_AF_NONE
}

func (m *IPAddress) GetV4Addr() uint32 {
	if x, ok := m.GetV4OrV6().(*IPAddress_V4Addr); ok {
		return x.V4Addr
	}
	return 0
}

func (m *IPAddress) GetV6Addr() []byte {
	if x, ok := m.GetV4OrV6().(*IPAddress_V6Addr); ok {
		return x.V6Addr
	}
	return nil
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*IPAddress) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _IPAddress_OneofMarshaler, _IPAddress_OneofUnmarshaler, _IPAddress_OneofSizer, []interface{}{
		(*IPAddress_V4Addr)(nil),
		(*IPAddress_V6Addr)(nil),
	}
}

func _IPAddress_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*IPAddress)
	// v4_or_v6
	switch x := m.V4OrV6.(type) {
	case *IPAddress_V4Addr:
		b.EncodeVarint(2<<3 | proto.WireFixed32)
		b.EncodeFixed32(uint64(x.V4Addr))
	case *IPAddress_V6Addr:
		b.EncodeVarint(3<<3 | proto.WireBytes)
		b.EncodeRawBytes(x.V6Addr)
	case nil:
	default:
		return fmt.Errorf("IPAddress.V4OrV6 has unexpected type %T", x)
	}
	return nil
}

func _IPAddress_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*IPAddress)
	switch tag {
	case 2: // v4_or_v6.v4_addr
		if wire != proto.WireFixed32 {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeFixed32()
		m.V4OrV6 = &IPAddress_V4Addr{uint32(x)}
		return true, err
	case 3: // v4_or_v6.v6_addr
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeRawBytes(true)
		m.V4OrV6 = &IPAddress_V6Addr{x}
		return true, err
	default:
		return false, nil
	}
}

func _IPAddress_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*IPAddress)
	// v4_or_v6
	switch x := m.V4OrV6.(type) {
	case *IPAddress_V4Addr:
		n += proto.SizeVarint(2<<3 | proto.WireFixed32)
		n += 4
	case *IPAddress_V6Addr:
		n += proto.SizeVarint(3<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(len(x.V6Addr)))
		n += len(x.V6Addr)
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

// IP Prefix object
type IPPrefix struct {
	Address   *IPAddress `protobuf:"bytes,1,opt,name=address" json:"address,omitempty"`
	PrefixLen uint32     `protobuf:"varint,2,opt,name=prefix_len,json=prefixLen" json:"prefix_len,omitempty"`
}

func (m *IPPrefix) Reset()                    { *m = IPPrefix{} }
func (m *IPPrefix) String() string            { return proto.CompactTextString(m) }
func (*IPPrefix) ProtoMessage()               {}
func (*IPPrefix) Descriptor() ([]byte, []int) { return fileDescriptor24, []int{3} }

func (m *IPPrefix) GetAddress() *IPAddress {
	if m != nil {
		return m.Address
	}
	return nil
}

func (m *IPPrefix) GetPrefixLen() uint32 {
	if m != nil {
		return m.PrefixLen
	}
	return 0
}

// PortRange object has low and high end of the port ranges
type L4PortRange struct {
	PortLow  uint32 `protobuf:"varint,1,opt,name=port_low,json=portLow" json:"port_low,omitempty"`
	PortHigh uint32 `protobuf:"varint,2,opt,name=port_high,json=portHigh" json:"port_high,omitempty"`
}

func (m *L4PortRange) Reset()                    { *m = L4PortRange{} }
func (m *L4PortRange) String() string            { return proto.CompactTextString(m) }
func (*L4PortRange) ProtoMessage()               {}
func (*L4PortRange) Descriptor() ([]byte, []int) { return fileDescriptor24, []int{4} }

func (m *L4PortRange) GetPortLow() uint32 {
	if m != nil {
		return m.PortLow
	}
	return 0
}

func (m *L4PortRange) GetPortHigh() uint32 {
	if m != nil {
		return m.PortHigh
	}
	return 0
}

type Empty struct {
}

func (m *Empty) Reset()                    { *m = Empty{} }
func (m *Empty) String() string            { return proto.CompactTextString(m) }
func (*Empty) ProtoMessage()               {}
func (*Empty) Descriptor() ([]byte, []int) { return fileDescriptor24, []int{5} }

func init() {
	proto.RegisterType((*ObjectMeta)(nil), "types.ObjectMeta")
	proto.RegisterType((*EncapInfo)(nil), "types.EncapInfo")
	proto.RegisterType((*IPAddress)(nil), "types.IPAddress")
	proto.RegisterType((*IPPrefix)(nil), "types.IPPrefix")
	proto.RegisterType((*L4PortRange)(nil), "types.L4PortRange")
	proto.RegisterType((*Empty)(nil), "types.Empty")
	proto.RegisterEnum("types.IPProtocol", IPProtocol_name, IPProtocol_value)
	proto.RegisterEnum("types.ICMPMsgType", ICMPMsgType_name, ICMPMsgType_value)
	proto.RegisterEnum("types.ApiStatus", ApiStatus_name, ApiStatus_value)
	proto.RegisterEnum("types.L2SegmentType", L2SegmentType_name, L2SegmentType_value)
	proto.RegisterEnum("types.EncapType", EncapType_name, EncapType_value)
	proto.RegisterEnum("types.IPAddressFamily", IPAddressFamily_name, IPAddressFamily_value)
	proto.RegisterEnum("types.WRingType", WRingType_name, WRingType_value)
	proto.RegisterEnum("types.ProxyType", ProxyType_name, ProxyType_value)
	proto.RegisterEnum("types.CryptoKeyType", CryptoKeyType_name, CryptoKeyType_value)
}

func init() { proto.RegisterFile("nic/proto/types.proto", fileDescriptor24) }

var fileDescriptor24 = []byte{
	// 1475 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x74, 0x57, 0xdd, 0x52, 0x1b, 0x39,
	0x16, 0x8e, 0x0d, 0xc6, 0xf8, 0xf0, 0xa7, 0x28, 0x84, 0x00, 0x09, 0x81, 0x38, 0xd9, 0x84, 0x38,
	0xbb, 0x21, 0x18, 0xc2, 0x26, 0xd9, 0xda, 0x8b, 0x76, 0xb7, 0x6c, 0x77, 0xd1, 0xee, 0x96, 0xd5,
	0x6d, 0x30, 0x5b, 0xb5, 0xa5, 0x72, 0xa0, 0x01, 0x4f, 0x11, 0xdb, 0x65, 0x1c, 0x12, 0xe6, 0x09,
	0xe6, 0x6e, 0xde, 0x63, 0xe6, 0x6d, 0xe6, 0x21, 0xe6, 0x76, 0x5e, 0x61, 0x4a, 0x6a, 0xdb, 0xad,
	0x96, 0xc9, 0x15, 0xd6, 0xf9, 0x3e, 0x7d, 0xe7, 0x47, 0x47, 0xea, 0x03, 0x3c, 0xec, 0xb4, 0x4f,
	0x77, 0x7a, 0xfd, 0xee, 0xa0, 0xbb, 0x33, 0xb8, 0xed, 0x85, 0xd7, 0x6f, 0xe5, 0x6f, 0x9c, 0x91,
	0x8b, 0xfc, 0x6b, 0x00, 0xef, 0xf3, 0x4f, 0xe1, 0xe9, 0xa0, 0x16, 0x0e, 0x5a, 0xf8, 0x31, 0xe4,
	0x06, 0x61, 0xa7, 0xd5, 0x19, 0xf0, 0xf6, 0xd9, 0x6a, 0x6a, 0x2b, 0xb5, 0x9d, 0x65, 0xb3, 0x91,
	0xc1, 0x3e, 0xcb, 0xff, 0x1f, 0x72, 0xa4, 0x73, 0xda, 0xea, 0xd9, 0x9d, 0xf3, 0x2e, 0xde, 0x01,
	0x08, 0xc5, 0x82, 0x0b, 0x19, 0x49, 0x5d, 0x2c, 0xa2, 0xb7, 0x91, 0x03, 0x09, 0x04, 0xb7, 0xbd,
	0x90, 0xe5, 0xc6, 0x3f, 0xf1, 0x26, 0xcc, 0x45, 0x1b, 0x6e, 0x5a, 0x57, 0x5f, 0xc3, 0xd5, 0xf4,
	0x56, 0x6a, 0x7b, 0x81, 0x45, 0x1a, 0x47, 0xc2, 0x92, 0xff, 0x19, 0x72, 0x36, 0x35, 0xce, 0xce,
	0xfa, 0xe1, 0xf5, 0x35, 0x7e, 0x03, 0x99, 0x76, 0x8f, 0xb7, 0xce, 0x87, 0xca, 0x2b, 0x43, 0xe5,
	0x31, 0xa1, 0xdc, 0xfa, 0xd2, 0xbe, 0xba, 0x65, 0xd3, 0xed, 0x9e, 0x71, 0x8e, 0xd7, 0x20, 0x7b,
	0xb3, 0xcf, 0x5b, 0x67, 0x67, 0x7d, 0x29, 0x9b, 0xad, 0xde, 0x63, 0x33, 0x37, 0xfb, 0x82, 0x29,
	0xa1, 0x83, 0x08, 0x9a, 0xda, 0x4a, 0x6d, 0xcf, 0x4b, 0xe8, 0x40, 0x40, 0x25, 0x80, 0xd9, 0x9b,
	0x7d, 0xde, 0xed, 0xf3, 0x9b, 0x83, 0x7c, 0x03, 0x66, 0x6d, 0x4a, 0xfb, 0xe1, 0x79, 0xfb, 0x3b,
	0x2e, 0x40, 0xb6, 0x15, 0x39, 0x91, 0xce, 0xe7, 0xc6, 0x69, 0x8d, 0x9d, 0xb3, 0x11, 0x01, 0x6f,
	0x00, 0xf4, 0xe4, 0x2e, 0x7e, 0x15, 0x76, 0x86, 0x39, 0xe5, 0x22, 0x8b, 0x13, 0x76, 0xf2, 0x04,
	0xe6, 0x9c, 0x7d, 0xda, 0xed, 0x0f, 0x58, 0xab, 0x73, 0x11, 0xe2, 0x35, 0x98, 0xed, 0x75, 0xfb,
	0x03, 0x7e, 0xd5, 0xfd, 0x26, 0xa5, 0x17, 0x58, 0x56, 0xac, 0x9d, 0xee, 0x37, 0x51, 0x78, 0x09,
	0x5d, 0xb6, 0x2f, 0x2e, 0x87, 0x3a, 0x92, 0x5b, 0x6d, 0x5f, 0x5c, 0xe6, 0xb3, 0x90, 0x21, 0x5f,
	0x7a, 0x83, 0xdb, 0xc2, 0x6f, 0x29, 0x00, 0x11, 0x67, 0x77, 0xd0, 0x3d, 0xed, 0x5e, 0x61, 0x04,
	0xf3, 0x36, 0xa5, 0xcc, 0x0b, 0x3c, 0xee, 0x7a, 0x2e, 0x41, 0xf7, 0x54, 0x8b, 0x6d, 0xd6, 0x28,
	0x4a, 0x25, 0x2c, 0xf4, 0x68, 0x1f, 0x4d, 0xe3, 0x25, 0x98, 0x1b, 0x59, 0x02, 0x93, 0xa2, 0x19,
	0xd5, 0xd0, 0xb0, 0x28, 0xba, 0xaf, 0x1a, 0x2a, 0x8c, 0xa0, 0x1d, 0xd5, 0x40, 0x7c, 0x8a, 0x8a,
	0x78, 0x51, 0xc6, 0x21, 0x0d, 0x46, 0x15, 0xed, 0x61, 0x0c, 0x8b, 0xaa, 0xdf, 0xa3, 0x03, 0xf4,
	0xa9, 0xf0, 0x5f, 0x98, 0x13, 0xbf, 0x6b, 0xd7, 0x17, 0xf2, 0xfc, 0x57, 0x00, 0x8b, 0x25, 0xaf,
	0xf9, 0x15, 0x1e, 0x9c, 0x50, 0x32, 0x0a, 0x79, 0xc2, 0x4e, 0xcc, 0xaa, 0x87, 0x52, 0x85, 0x3f,
	0x97, 0x20, 0x67, 0xf4, 0xda, 0xfe, 0xa0, 0x35, 0xf8, 0x7a, 0x8d, 0xef, 0xc3, 0x82, 0x41, 0x6d,
	0xee, 0x07, 0x46, 0xd0, 0xf0, 0xb9, 0x77, 0x88, 0xee, 0x09, 0x9f, 0x8a, 0x89, 0x30, 0x86, 0x52,
	0x78, 0x1d, 0x56, 0x14, 0x9b, 0xed, 0x1e, 0x19, 0x8e, 0x6d, 0x71, 0x83, 0x55, 0x50, 0x1a, 0x6f,
	0xc0, 0x9a, 0xca, 0x6f, 0xda, 0x7e, 0xe0, 0x73, 0xc3, 0x61, 0xc4, 0xb0, 0x4e, 0xd0, 0x14, 0x5e,
	0x83, 0x87, 0xaa, 0x87, 0x46, 0xc0, 0xbd, 0x32, 0xaf, 0x91, 0x1a, 0x9a, 0xc6, 0xab, 0xb0, 0xac,
	0x40, 0xae, 0x17, 0xf0, 0xb2, 0xd7, 0x70, 0x2d, 0x94, 0xc1, 0x4f, 0x61, 0x7d, 0x72, 0x13, 0x23,
	0xbe, 0xd7, 0x60, 0x26, 0x41, 0x33, 0xf8, 0x09, 0xac, 0xaa, 0x3e, 0x5d, 0xd3, 0xa0, 0xa3, 0xa8,
	0x50, 0x16, 0x6f, 0xc1, 0x13, 0x05, 0x0d, 0x88, 0x6b, 0xb8, 0x01, 0xb7, 0xad, 0x31, 0x63, 0x16,
	0x6f, 0xc2, 0xe3, 0x49, 0x46, 0x1c, 0x40, 0x0e, 0xbf, 0x80, 0x2d, 0x85, 0xe0, 0x14, 0xb9, 0x4f,
	0x2a, 0x35, 0x92, 0x94, 0x01, 0xfc, 0x1c, 0x36, 0xef, 0x66, 0xc5, 0x52, 0x73, 0x1a, 0xc9, 0x76,
	0x03, 0xc2, 0xca, 0x86, 0x49, 0x54, 0xa5, 0x79, 0x9c, 0x87, 0xa7, 0x77, 0x92, 0x62, 0xa1, 0x05,
	0xad, 0x28, 0x76, 0x39, 0x3a, 0xd6, 0x91, 0xc6, 0xe2, 0x24, 0x6e, 0xbb, 0x65, 0x6f, 0x8c, 0x2f,
	0xe9, 0x81, 0x94, 0xb9, 0x53, 0x1c, 0x67, 0x35, 0x24, 0xa1, 0x49, 0x12, 0x71, 0x6d, 0x33, 0xa9,
	0x74, 0xff, 0x07, 0xa4, 0x44, 0x38, 0x18, 0xbf, 0x84, 0xbc, 0xe6, 0x6e, 0x14, 0x92, 0x48, 0x6a,
	0xc4, 0x7b, 0x80, 0x5f, 0xc1, 0xf3, 0x09, 0x5e, 0x7d, 0x92, 0xb8, 0x8c, 0x9f, 0xc1, 0x46, 0xe2,
	0xd0, 0x2d, 0xea, 0xd9, 0x89, 0x5a, 0x3f, 0xd4, 0xce, 0xb5, 0xec, 0x78, 0xc7, 0xfc, 0x90, 0x9c,
	0x8c, 0x35, 0x56, 0xb4, 0xd6, 0x90, 0x84, 0x44, 0x6e, 0x8f, 0xb4, 0x76, 0x96, 0x8c, 0xd8, 0xc3,
	0xaa, 0x26, 0xe0, 0x13, 0xdf, 0xb7, 0x3d, 0x57, 0x61, 0xac, 0x69, 0x02, 0x55, 0xc3, 0xb5, 0x9c,
	0xb8, 0x2c, 0xeb, 0xda, 0x55, 0xaa, 0x1e, 0x73, 0xca, 0xbc, 0x8a, 0xbc, 0x66, 0x8f, 0xb5, 0xad,
	0xb2, 0x5e, 0x71, 0x93, 0x3c, 0xd1, 0xba, 0x5e, 0xc0, 0xb1, 0xdf, 0x0d, 0xad, 0x8e, 0xee, 0xb1,
	0x4f, 0x4c, 0xa1, 0x5d, 0xb6, 0x9d, 0x44, 0xaf, 0x3d, 0xd5, 0x0e, 0x26, 0x49, 0x8c, 0x05, 0x37,
	0xf5, 0x6b, 0xe4, 0xf8, 0xdc, 0x2c, 0xa9, 0x4a, 0x5b, 0xfa, 0x35, 0x8a, 0x18, 0xb1, 0xc4, 0x33,
	0x5d, 0xc2, 0xa4, 0x9a, 0x44, 0x5e, 0x97, 0x88, 0x18, 0xb1, 0xc4, 0x73, 0xed, 0x66, 0x94, 0x1a,
	0x65, 0x4e, 0x3d, 0xcf, 0x51, 0x45, 0x5e, 0x68, 0x9d, 0x31, 0xe6, 0xc4, 0x32, 0xff, 0xd0, 0x92,
	0x1e, 0x53, 0x4c, 0xcf, 0xe7, 0x35, 0x83, 0x0e, 0x9f, 0x2d, 0xf4, 0x52, 0x8b, 0xa7, 0xde, 0x20,
	0x8d, 0x44, 0xf5, 0x5e, 0x69, 0xf1, 0x44, 0x04, 0xd3, 0x6b, 0x28, 0x97, 0x68, 0x5b, 0xbb, 0x89,
	0x11, 0x27, 0x0e, 0xe6, 0xb5, 0x16, 0x2f, 0xf5, 0x1c, 0xdb, 0x24, 0x4c, 0x75, 0x53, 0xd0, 0x2a,
	0x37, 0xa2, 0xc4, 0x22, 0x6f, 0x34, 0x86, 0x4b, 0x82, 0x63, 0x8f, 0x1d, 0x2a, 0x8c, 0x7f, 0x6a,
	0xed, 0x64, 0x98, 0x89, 0xaa, 0xfd, 0x4b, 0x6b, 0x27, 0x01, 0xc7, 0x9b, 0xdf, 0x6a, 0x85, 0x38,
	0x66, 0xb6, 0x5b, 0x51, 0xb7, 0xef, 0x68, 0x49, 0x46, 0x84, 0x58, 0xe0, 0x9d, 0x96, 0x64, 0x84,
	0x27, 0x9e, 0x88, 0x5d, 0xbd, 0x0e, 0xcc, 0x6b, 0x9e, 0x24, 0x29, 0x45, 0xfd, 0x61, 0xa4, 0xa2,
	0x59, 0x93, 0x3d, 0xb4, 0xa7, 0xc9, 0x8c, 0x39, 0x71, 0x30, 0xfb, 0x5a, 0x36, 0xce, 0xbe, 0x53,
	0x4a, 0x3c, 0x0c, 0xef, 0xb5, 0x6a, 0x9a, 0xb4, 0xa1, 0x79, 0x39, 0xd0, 0x24, 0x86, 0x8c, 0xd8,
	0xc7, 0xbf, 0xf5, 0x23, 0x93, 0xd9, 0x08, 0x9c, 0xb8, 0x46, 0xc9, 0x21, 0x16, 0xfa, 0xa0, 0xbd,
	0x9b, 0x11, 0x43, 0x7b, 0x61, 0x3e, 0x6a, 0x1f, 0x4c, 0xb3, 0x5c, 0xe1, 0x56, 0x49, 0xbe, 0x0f,
	0x9f, 0xf0, 0x1b, 0x78, 0x75, 0xd7, 0x0b, 0xd8, 0xa0, 0x96, 0xcc, 0xa6, 0x6a, 0x39, 0xe3, 0x78,
	0xff, 0x53, 0xf8, 0x3d, 0x05, 0x0b, 0x4e, 0xd1, 0x0f, 0x2f, 0xbe, 0x84, 0x9d, 0x81, 0x1c, 0x15,
	0x56, 0x61, 0x59, 0xf9, 0x46, 0xa9, 0xc3, 0xc2, 0x3a, 0xac, 0xe8, 0x48, 0xf4, 0x51, 0x44, 0xa9,
	0xbb, 0x76, 0xd5, 0x2a, 0xb5, 0x00, 0xa5, 0x45, 0xa4, 0x3a, 0x62, 0xbb, 0x65, 0x66, 0xa0, 0x29,
	0x31, 0x7d, 0x28, 0x10, 0x65, 0x76, 0xcd, 0x60, 0x27, 0x68, 0x1a, 0x3f, 0x82, 0x07, 0xea, 0xc7,
	0xd4, 0xf7, 0x1c, 0x23, 0x20, 0x16, 0xca, 0x14, 0x7e, 0x4d, 0x81, 0x32, 0xd4, 0x3e, 0x80, 0xa5,
	0xe8, 0xa3, 0xae, 0x06, 0xb9, 0x0c, 0x48, 0x31, 0x5a, 0x5e, 0xb0, 0x5b, 0x47, 0x29, 0xcd, 0x7a,
	0xd4, 0x74, 0x0c, 0x17, 0xa5, 0x85, 0x1f, 0xc5, 0x6a, 0x8b, 0xe9, 0x80, 0xdb, 0x14, 0x4d, 0x89,
	0xe9, 0x46, 0x01, 0xc4, 0x18, 0x36, 0xad, 0x49, 0xc8, 0xfe, 0x41, 0x99, 0x42, 0x09, 0x96, 0xb4,
	0xb1, 0x38, 0x1a, 0xcf, 0xb8, 0x51, 0x1e, 0x45, 0x34, 0x5e, 0xdb, 0x2e, 0x11, 0xa5, 0x92, 0xf3,
	0xdc, 0x68, 0x7d, 0x80, 0xd2, 0x85, 0xbf, 0xd2, 0x90, 0x3b, 0x66, 0xed, 0xce, 0xc5, 0x28, 0x2b,
	0xe5, 0x1a, 0x0c, 0x35, 0x92, 0x46, 0x9f, 0x30, 0x91, 0xd4, 0x0a, 0x60, 0x95, 0x59, 0xb3, 0x18,
	0x0f, 0x9a, 0x28, 0x7d, 0x97, 0x9d, 0x35, 0xd1, 0x94, 0xb8, 0xca, 0x09, 0x3b, 0x65, 0xdc, 0xaf,
	0x19, 0x8e, 0x23, 0x76, 0x4d, 0xff, 0x18, 0x65, 0x4d, 0x94, 0x11, 0x67, 0xaf, 0xa3, 0x25, 0xbb,
	0x22, 0x76, 0xce, 0xfc, 0x08, 0x63, 0x4d, 0x94, 0x15, 0x95, 0x54, 0xb0, 0x92, 0x5f, 0x47, 0xb3,
	0xba, 0x8d, 0xd5, 0x51, 0x6e, 0x22, 0x41, 0xbf, 0x8e, 0x40, 0x9c, 0x8f, 0xfa, 0x22, 0x88, 0x92,
	0x9b, 0xa5, 0x3a, 0x9a, 0x13, 0x67, 0xa1, 0x00, 0x06, 0xab, 0xb3, 0x26, 0x9a, 0xd7, 0x74, 0x0d,
	0xbf, 0x8e, 0x16, 0x74, 0xa6, 0x14, 0x5e, 0x2c, 0xfc, 0x92, 0x82, 0x1c, 0xed, 0x77, 0xbf, 0xdf,
	0x8e, 0x2a, 0xae, 0xbc, 0x2a, 0xc3, 0x8a, 0x63, 0x58, 0x54, 0x8c, 0x62, 0x56, 0x4f, 0xe9, 0x36,
	0xc7, 0x47, 0x69, 0xe1, 0x40, 0x7d, 0x92, 0x64, 0x5b, 0x4c, 0x69, 0x4c, 0x93, 0x36, 0xa2, 0x06,
	0x4a, 0x30, 0xcb, 0x76, 0x13, 0x65, 0x0a, 0x7f, 0xa4, 0x60, 0xc1, 0xec, 0xdf, 0xf6, 0x06, 0xdd,
	0xc3, 0x30, 0x0a, 0x67, 0x1d, 0x56, 0x4c, 0x76, 0x42, 0x03, 0x4f, 0x5e, 0xd7, 0x28, 0x6e, 0xe2,
	0xef, 0x16, 0x3f, 0x44, 0x57, 0xf0, 0x2e, 0xec, 0x63, 0x31, 0x1a, 0xbf, 0xef, 0xc0, 0x8a, 0xef,
	0x0f, 0xa2, 0x4e, 0xd7, 0x31, 0x8b, 0xf8, 0x51, 0x4f, 0xe8, 0x80, 0x59, 0x35, 0xcc, 0xaa, 0x51,
	0x7c, 0x17, 0xf5, 0x84, 0x8e, 0x52, 0xcf, 0x39, 0xd9, 0xdd, 0x7b, 0xf7, 0x1e, 0x65, 0xc4, 0x9d,
	0xd7, 0xd1, 0x6a, 0xcd, 0x30, 0xd1, 0x4c, 0x09, 0xfe, 0x37, 0x7b, 0xd9, 0xba, 0x92, 0xff, 0xea,
	0x7e, 0x9e, 0x91, 0x7f, 0xf6, 0xfe, 0x0e, 0x00, 0x00, 0xff, 0xff, 0xae, 0xc5, 0xf7, 0x34, 0x0a,
	0x0f, 0x00, 0x00,
}
