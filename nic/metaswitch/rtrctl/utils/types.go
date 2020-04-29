package utils

import (
	"encoding/binary"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/satori/go.uuid"

	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	type2MinLen = 8 + 10 + 4 + 1 + 6 + 1 + 3
	type2MaxLen = 8 + 10 + 4 + 1 + 6 + 1 + 16 + 3 + 3
	type3MinLen = 8 + 4 + 1 + 4
	type3MaxLen = 8 + 4 + 1 + 16
	type5MinLen = 8 + 10 + 4 + 1 + 4 + 4 + 3
	type5MaxLen = 8 + 10 + 4 + 1 + 16 + 16 + 3
)

// NLRIPrefix is a representation of the BGP Route
type NLRIPrefix struct {
	Afi    int
	Safi   int
	Type   int
	Length int
	Prefix UserPrefix
}

func dumpBytes(in []byte) string {
	out := ""
	for _, b := range in {
		out = fmt.Sprintf("%s0x%02x ", out, b)
	}
	return out
}

func dumpNonHexBytes(in []byte) string {
	out := ""
	for _, b := range in {
		out = fmt.Sprintf("%s%02x ", out, b)
	}
	return out
}

func printRD(in []byte) string {
	switch in[1] {
	case 0:
		return fmt.Sprintf("%d:%d", binary.BigEndian.Uint16(in[2:4]), binary.BigEndian.Uint32(in[4:8]))
	case 1:
		return fmt.Sprintf("%s:%d", net.IP(in[2:6]).String(), binary.BigEndian.Uint16(in[6:8]))
	case 2:
		return fmt.Sprintf("%d:%d", binary.BigEndian.Uint16(in[2:6]), binary.BigEndian.Uint16(in[6:8]))
	default:
		return "failed to parse"

	}
}

func label2int(in []byte) uint32 {
	if len(in) != 3 {
		return 0
	}
	return (((uint32(in[0]) * 256) + uint32(in[1])) * 256) + uint32(in[2])
}

// String returns a user friendly string
func (n *NLRIPrefix) String() string {
	if n == nil {
		return fmt.Sprintf("0.0.0.0")
	}
	return fmt.Sprintf("%v", n.Prefix)
}

// String returns a user friendly string
func (n *NLRIPrefix) AttrString() string {
	if n == nil || n.Prefix == nil {
		return fmt.Sprintf("")
	}
	return fmt.Sprintf("%v", n.Prefix.attrString())
}

func NewNLRIPrefix(afi int, safi int, in []byte) *NLRIPrefix {
	if len(in) < 3 {
		return nil
	}
	ret := &NLRIPrefix{
		Afi:    afi,
		Safi:   safi,
		Type:   int(in[0]),
		Length: int(in[1]),
	}

	if afi == 1 {
		ret.Prefix = newIPv4Route(in[0:])
	} else {
		switch ret.Type {
		case 2:
			p := &EVPNType2Route{}
			p.parseBytes(in[2:])
			ret.Prefix = newEVPNType2Route(p)

		case 3:
			p := &EVPNType3Route{}
			p.parseBytes(in[2:])
			ret.Prefix = newEVPNType3Route(p)

		case 5:
			p := &EVPNType5Route{}
			p.parseBytes(in[2:])
			ret.Prefix = newEVPNType5Route(p)
		}
	}
	return ret
}

type EVPNType2Route struct {
	RD           []byte
	ESI          []byte
	EthTagID     []byte
	MACAddrLen   int
	MACAddress   []byte
	IPAddressLen int
	IPAddress    []byte
	MPLSLabel1   []byte
	MPLSLabel2   []byte
}

type ShadowEVPNType2Route struct {
	RD         string
	ESI        string
	EthTagID   uint32
	MACAddress string
	IPAddress  string
	MPLSLabel1 uint32
	MPLSLabel2 uint32
	*EVPNType2Route
}

const type2Fmt = `[%d][%v][%v][%d][%v][%d][%v]`

// String returns a user friendly string
func (s *ShadowEVPNType2Route) String() string {
	var type2 int = 2
	var macsize int = 48
	if s.IPAddress == "<nil>" {
		s.IPAddress = "0.0.0.0"
	}
	return fmt.Sprintf(type2Fmt, type2, s.RD, s.EthTagID, macsize, s.MACAddress, s.IPAddressLen, s.IPAddress)
}

// String returns a user friendly string
func (s *ShadowEVPNType2Route) attrString() string {
	return fmt.Sprintf("      ESI %v L2VNI %v L3VNI %v", s.ESI, s.MPLSLabel1, s.MPLSLabel2)
}
func (a *EVPNType2Route) parseBytes(in []byte) {
	if len(in) < type2MinLen {
		log.Errorf("invalid length [%d] for evpn type2", len(in))
	}
	cur := 0
	a.RD = make([]byte, 8)
	copy(a.RD, in[cur:cur+8])
	cur += 8
	a.ESI = make([]byte, 10)
	copy(a.ESI, in[cur:cur+10])
	cur += 10
	a.EthTagID = make([]byte, 4)
	copy(a.EthTagID, in[cur:cur+4])
	cur += 4
	a.MACAddrLen = int(in[cur])
	cur += 1
	a.MACAddress = make([]byte, 6)
	copy(a.MACAddress, in[cur:cur+6])
	cur += 6
	a.IPAddressLen = int(in[cur])
	cur += 1
	switch a.IPAddressLen {
	case 32:
		a.IPAddress = make([]byte, 4)
		copy(a.IPAddress, in[cur:cur+4])
		cur += 4
	case 128:
		a.IPAddress = make([]byte, 16)
		copy(a.IPAddress, in[cur:cur+16])
		cur += 16
	}
	a.MPLSLabel1 = make([]byte, 3)
	copy(a.MPLSLabel1, in[cur:cur+3])
	cur += 3
	if len(in) > cur {
		copy(a.MPLSLabel2, in[cur:cur+3])
	}
}

func newEVPNType2Route(in *EVPNType2Route) *ShadowEVPNType2Route {
	return &ShadowEVPNType2Route{
		EVPNType2Route: in,
		RD:             printRD(in.RD),
		ESI:            dumpBytes(in.ESI),
		EthTagID:       binary.BigEndian.Uint32(in.EthTagID),
		MACAddress:     net.HardwareAddr(in.MACAddress).String(),
		IPAddress:      net.IP(in.IPAddress).String(),
		MPLSLabel1:     label2int(in.MPLSLabel1),
		MPLSLabel2:     label2int(in.MPLSLabel2),
	}
}

type EVPNType3Route struct {
	RD           []byte
	EthTagID     []byte
	IPAddressLen int
	IPAddress    []byte
}

type ShadowEVPNType3Route struct {
	RD        string
	EthTagID  uint32
	IPAddress string
	*EVPNType3Route
}

const type3Fmt = `[%d][%v][%v][%d][%v]`

// String returns a user friendly string
func (s *ShadowEVPNType3Route) String() string {
	var type3 int = 3
	return fmt.Sprintf(type3Fmt, type3, s.RD, s.EthTagID, s.IPAddressLen, s.IPAddress)
}

// String returns a user friendly string
func (s *ShadowEVPNType3Route) attrString() string {
	return fmt.Sprintf("")
}
func (a *EVPNType3Route) parseBytes(in []byte) {
	if len(in) < type3MinLen {
		log.Errorf("invalid length [%d] for evpn type3", len(in))
	}
	cur := 0
	a.RD = make([]byte, 8)
	copy(a.RD, in[cur:cur+8])
	cur += 8
	a.EthTagID = make([]byte, 4)
	copy(a.EthTagID, in[cur:cur+4])
	cur += 4
	a.IPAddressLen = int(in[cur])
	cur += 1
	switch a.IPAddressLen {
	case 32:
		a.IPAddress = make([]byte, 4)
		copy(a.IPAddress, in[cur:cur+4])
		cur += 4
	case 128:
		a.IPAddress = make([]byte, 16)
		copy(a.IPAddress, in[cur:cur+16])
		cur += 16
	}
}

func newEVPNType3Route(in *EVPNType3Route) *ShadowEVPNType3Route {
	return &ShadowEVPNType3Route{
		EVPNType3Route: in,
		RD:             printRD(in.RD),
		EthTagID:       binary.BigEndian.Uint32(in.EthTagID),
		IPAddress:      net.IP(in.IPAddress).String(),
	}
}

type EVPNType5Route struct {
	RD          []byte
	ESI         []byte
	EthTagID    []byte
	IPPrefixLen int
	IPPrefix    []byte
	GWIPAddress []byte
	MPLSLabel1  []byte
}

type ShadowEVPNType5Route struct {
	RD          string
	ESI         string
	EthTagID    uint32
	IPPrefix    string
	GWIPAddress string
	MPLSLabel1  uint32
	*EVPNType5Route
}

const type5Fmt = `[%d][%v][%v][%v][%v]`

// String returns a user friendly string
func (s *ShadowEVPNType5Route) String() string {
	var type5 int = 5
	return fmt.Sprintf(type5Fmt, type5, s.RD, s.EthTagID, s.IPPrefixLen, s.IPPrefix)
}

// String returns a user friendly string
func (s *ShadowEVPNType5Route) attrString() string {
	return fmt.Sprintf("      ESI %v GW-IP %v L3VNI %v", s.ESI, s.GWIPAddress, s.MPLSLabel1)
}
func (a *EVPNType5Route) parseBytes(in []byte) {
	if len(in) < type5MinLen {
		log.Errorf("invalid length [%d] for evpn type2", len(in))
	}
	cur := 0
	a.RD = make([]byte, 8)
	copy(a.RD, in[cur:cur+8])
	cur += 8
	a.ESI = make([]byte, 10)
	copy(a.ESI, in[cur:cur+10])
	cur += 10
	a.EthTagID = make([]byte, 4)
	copy(a.EthTagID, in[cur:cur+4])
	cur += 4
	a.IPPrefixLen = int(in[cur])
	cur += 1
	if len(in) == type5MinLen {
		a.IPPrefix = make([]byte, 4)
		copy(a.IPPrefix, in[cur:cur+4])
		cur += 4
		a.GWIPAddress = make([]byte, 4)
		copy(a.GWIPAddress, in[cur:cur+4])
		cur += 4
	} else {
		a.IPPrefix = make([]byte, 16)
		copy(a.IPPrefix, in[cur:cur+16])
		cur += 16
		a.GWIPAddress = make([]byte, 16)
		copy(a.GWIPAddress, in[cur:cur+16])
		cur += 16
	}
	a.MPLSLabel1 = make([]byte, 3)
	copy(a.MPLSLabel1, in[cur:cur+3])
}

func newEVPNType5Route(in *EVPNType5Route) *ShadowEVPNType5Route {
	return &ShadowEVPNType5Route{
		EVPNType5Route: in,
		RD:             printRD(in.RD),
		ESI:            dumpBytes(in.ESI),
		EthTagID:       binary.BigEndian.Uint32(in.EthTagID),
		IPPrefix:       net.IP(in.IPPrefix).String(),
		GWIPAddress:    net.IP(in.GWIPAddress).String(),
		MPLSLabel1:     label2int(in.MPLSLabel1),
	}
}

type ShadowIPv4Route struct {
	IPPrefix string
}

// String returns a user friendly string
func (s *ShadowIPv4Route) attrString() string {
	return fmt.Sprintf("")
}

const ipv4Fmt = `%v`

func (a *ShadowIPv4Route) parseBytes(in []byte) {
}

// String returns a user friendly string
func (s *ShadowIPv4Route) String() string {
	return fmt.Sprintf(ipv4Fmt, s.IPPrefix)
}

func newIPv4Route(in []byte) *ShadowIPv4Route {
	if in == nil || len(net.IP(in)) == 0 {
		return &ShadowIPv4Route{
			IPPrefix: "0.0.0.0",
		}
	} else {
		return &ShadowIPv4Route{
			IPPrefix: net.IP(in).String(),
		}
	}
}

type UserPrefix interface {
	parseBytes(in []byte)
	attrString() string
}

// Uint32ToIPv4Address returns an IP Address string given an integer
func Uint32ToIPv4Address(in uint32) string {
	ip := make(net.IP, 4)
	binary.BigEndian.PutUint32(ip, in)
	return ip.String()
}

// PdsIPToString convverts a PDS IPAddress type to a string. Only IPv4 is supported
func PdsIPToString(in *pds.IPAddress) string {
	if in == nil {
		return ""
	}
	if in.Af == pds.IPAF_IP_AF_INET {
		return Uint32ToIPv4Address(in.GetV4Addr())
	}
	return ""
}

// IPAddrStrtoUint32 converts string IP address to uint32
func IPAddrStrtoUint32(ip string) uint32 {
	var addr [4]uint32
	fmt.Sscanf(ip, "%d.%d.%d.%d", &addr[0], &addr[1], &addr[2], &addr[3])
	return ((addr[0] << 24) + (addr[1] << 16) + (addr[2] << 8) + (addr[3]))
}

// IPAddrStrToPdsIPAddr converts string ip address to pds native IPAddress Type
func IPAddrStrToPdsIPAddr(ip string) *pds.IPAddress {
	netIP := net.ParseIP(ip)
	isV6 := (netIP.To4() == nil)

	var ipAddr *pds.IPAddress
	if isV6 {
		ipAddr = &pds.IPAddress{
			Af: pds.IPAF_IP_AF_INET6,
			V4OrV6: &pds.IPAddress_V6Addr{
				V6Addr: netIP,
			},
		}
	} else {
		ipAddr = &pds.IPAddress{
			Af: pds.IPAF_IP_AF_INET,
			V4OrV6: &pds.IPAddress_V4Addr{
				V4Addr: IPAddrStrtoUint32(ip),
			},
		}
	}
	return ipAddr
}

// ShadowBgpSpec shadows the BGPSpec for CLI purposes
type ShadowBgpSpec struct {
	*pds.BGPSpec
	Id                     string
	RouterId               string
	ClusterId              string
	Status                 string
	NumAdjRibOutRoutes     uint32
	PeakNumAdjRibOutRoutes uint32
	RemDelayTime           uint32
	TableVer               uint32
}

// NewBGPGetResp creates a new shadow of the BGPSpec
func NewBGPGetResp(spec *pds.BGPSpec, status *pds.BGPStatus) *ShadowBgpSpec {
	uid, err := uuid.FromBytes(spec.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}

	return &ShadowBgpSpec{
		BGPSpec:                spec,
		Id:                     uidstr,
		RouterId:               Uint32ToIPv4Address(spec.RouterId),
		ClusterId:              Uint32ToIPv4Address(spec.ClusterId),
		Status:                 strings.TrimPrefix(status.Status.String(), "BGP_OPER_STATUS_"),
		NumAdjRibOutRoutes:     status.NumAdjRibOutRoutes,
		PeakNumAdjRibOutRoutes: status.PeakNumAdjRibOutRoutes,
		RemDelayTime:           status.RemDelayTime,
		TableVer:               status.TableVer,
	}
}

// ShadowBGPPeerSpec shadows the BGPPeerSpec for CLI purposes
type ShadowBGPPeerSpec struct {
	Id        string
	LocalAddr string
	PeerAddr  string
	Password  bool
	State     string
	*pds.BGPPeerSpec
}

func newBGPPeerSpec(in *pds.BGPPeerSpec) ShadowBGPPeerSpec {
	uid, err := uuid.FromBytes(in.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}
	return ShadowBGPPeerSpec{
		Id:          uidstr,
		LocalAddr:   PdsIPToString(in.LocalAddr),
		PeerAddr:    PdsIPToString(in.PeerAddr),
		Password:    len(in.Password) != 0,
		State:       strings.TrimPrefix(in.State.String(), "ADMIN_STATE_"),
		BGPPeerSpec: in,
	}
}

// ShadowBGPPeerStatus shadows the BGPPeerStatus for CLI purposes
type ShadowBGPPeerStatus struct {
	Id               string
	LastErrorRcvd    string
	LastErrorSent    string
	Status           string
	PrevStatus       string
	LocalAddr        string
	CapsSent         string
	CapsRcvd         string
	CapsNeg          string
	SelLocalAddrType string
	*pds.BGPPeerStatus
}

type BgpCapabilities uint32

const (
	BGP_CAP_MP_IPV4_UNI         BgpCapabilities = 0x00000001
	BGP_CAP_MP_IPV4_MULT        BgpCapabilities = 0x00000002
	BGP_CAP_MP_IPV4_VPN         BgpCapabilities = 0x00000004
	BGP_CAP_MP_IPV4_LABEL       BgpCapabilities = 0x00000008
	BGP_CAP_MP_IPV6_UNI         BgpCapabilities = 0x00000010
	BGP_CAP_MP_IPV6_MULT        BgpCapabilities = 0x00000020
	BGP_CAP_MP_IPV6_VPN         BgpCapabilities = 0x00000040
	BGP_CAP_MP_IPV6_LABEL       BgpCapabilities = 0x00000080
	BGP_CAP_ROUTE_REFRESH       BgpCapabilities = 0x00000100
	BGP_CAP_GRACEFUL_RESTART    BgpCapabilities = 0x00000200
	BGP_CAP_ROUTE_REFRESH_CISCO BgpCapabilities = 0x00000400
	BGP_CAP_ORF                 BgpCapabilities = 0x00000800
	BGP_CAP_ORF_CISCO           BgpCapabilities = 0x00001000
	BGP_CAP_4_OCTET_AS          BgpCapabilities = 0x00002000
	BGP_CAP_MP_L2VPN_VPLS       BgpCapabilities = 0x00004000
	BGP_CAP_ADD_PATH            BgpCapabilities = 0x00008000
	BGP_CAP_MP_L2VPN_EVPN       BgpCapabilities = 0x00010000
	BGP_CAP_MP_IPV4_PRIV        BgpCapabilities = 0x00020000
	BGP_CAP_ENH_RT_REFRESH      BgpCapabilities = 0x00040000
	BGP_CAP_ENHE_IPV4_UNI       BgpCapabilities = 0x00080000
	BGP_CAP_ENHE_IPV4_MULTI     BgpCapabilities = 0x00100000
	BGP_CAP_ENHE_IPV4_LABEL     BgpCapabilities = 0x00200000
	BGP_CAP_ENHE_IPV4_VPN       BgpCapabilities = 0x00400000
)

func BgpCapsStr(in BgpCapabilities) string {
	ret := ""
	if in&BGP_CAP_MP_IPV4_UNI != 0 {
		ret += "BGP_CAP_MP_IPV4_UNI, "
	}
	if in&BGP_CAP_MP_IPV4_MULT != 0 {
		ret += "BGP_CAP_MP_IPV4_MULT, "
	}
	if in&BGP_CAP_MP_IPV4_VPN != 0 {
		ret += "BGP_CAP_MP_IPV4_VPN, "
	}
	if in&BGP_CAP_MP_IPV4_LABEL != 0 {
		ret += "BGP_CAP_MP_IPV4_LABEL, "
	}
	if in&BGP_CAP_MP_IPV6_UNI != 0 {
		ret += "BGP_CAP_MP_IPV6_UNI, "
	}
	if in&BGP_CAP_MP_IPV6_MULT != 0 {
		ret += "BGP_CAP_MP_IPV6_MULT, "
	}
	if in&BGP_CAP_MP_IPV6_VPN != 0 {
		ret += "BGP_CAP_MP_IPV6_VPN, "
	}
	if in&BGP_CAP_MP_IPV6_LABEL != 0 {
		ret += "BGP_CAP_MP_IPV6_LABEL, "
	}
	if in&BGP_CAP_ROUTE_REFRESH != 0 {
		ret += "BGP_CAP_ROUTE_REFRESH, "
	}
	if in&BGP_CAP_GRACEFUL_RESTART != 0 {
		ret += "BGP_CAP_GRACEFUL_RESTART, "
	}
	if in&BGP_CAP_ROUTE_REFRESH_CISCO != 0 {
		ret += "BGP_CAP_ROUTE_REFRESH_CISCO, "
	}
	if in&BGP_CAP_ORF != 0 {
		ret += "BGP_CAP_ORF, "
	}
	if in&BGP_CAP_ORF_CISCO != 0 {
		ret += "BGP_CAP_ORF_CISCO, "
	}
	if in&BGP_CAP_4_OCTET_AS != 0 {
		ret += "BGP_CAP_4_OCTET_AS, "
	}
	if in&BGP_CAP_MP_L2VPN_VPLS != 0 {
		ret += "BGP_CAP_MP_L2VPN_VPLS, "
	}
	if in&BGP_CAP_ADD_PATH != 0 {
		ret += "BGP_CAP_ADD_PATH, "
	}
	if in&BGP_CAP_MP_L2VPN_EVPN != 0 {
		ret += "BGP_CAP_MP_L2VPN_EVPN, "
	}
	if in&BGP_CAP_MP_IPV4_PRIV != 0 {
		ret += "BGP_CAP_MP_IPV4_PRIV, "
	}
	if in&BGP_CAP_ENH_RT_REFRESH != 0 {
		ret += "BGP_CAP_ENH_RT_REFRESH, "
	}
	if in&BGP_CAP_ENHE_IPV4_UNI != 0 {
		ret += "BGP_CAP_ENHE_IPV4_UNI, "
	}
	if in&BGP_CAP_ENHE_IPV4_MULTI != 0 {
		ret += "BGP_CAP_ENHE_IPV4_MULTI, "
	}
	if in&BGP_CAP_ENHE_IPV4_LABEL != 0 {
		ret += "BGP_CAP_ENHE_IPV4_LABEL, "
	}
	if in&BGP_CAP_ENHE_IPV4_VPN != 0 {
		ret += "BGP_CAP_ENHE_IPV4_VPN, "
	}
	if ret == "" {
		ret = "BGP_CAP_NONE"
	} else {
		ret = strings.TrimSuffix(ret, ", ")
	}

	return ret
}

func BgpErrStr(bs []byte) string {
	if bs[0] == 0 {
		return "NONE"
	}
	if bs[0] > 7 {
		return "Unknown Error " + strconv.Itoa(int(bs[0]))
	}
	type BGPErrCodeInfo struct {
		Str       string
		SubCodeSz int
	}
	ErrCodeStr := [...]BGPErrCodeInfo{
		{"NONE", 0},
		{"Message Header Error", 3},
		{"OPEN Message Error", 8},
		{"UPDATE Message Error", 11},
		{"Hold Timer Expired", 0},
		{"Finite State Machine Error", 3},
		{"Cease", 7},
		{"ROUTE-REFRESH Message Error", 1}}

	SubErrCodeStr := [...][12]string{
		{},
		{"Unspecific", "Connection Not Synchronized", "Bad Message Length", "Bad Message Type"},
		{"Unspecific", "Unsupported Version Number", "Bad Peer AS", "Bad BGP Identifier", "Unsupported Optional Parameter", "", "Unacceptable Hold Time", "Unsupported Capability", "Role Mismatch"},
		{"Unspecific", "Malformed Attribute List", "Unrecognized Well-known Attribute", "Missing Well-known Attribute", "Attribute Flags Error", "Attribute Length Error", "Invalid ORIGIN Attribute", "", "Invalid NEXT_HOP Attribute", "Optional Attribute Error", "Invalid Network Field", "Malformed AS_PATH"},
		{},
		{"Unspecific", "Receive Unexpected Message in OpenSent State", "Receive Unexpected Message in OpenConfirm State", "Receive Unexpected Message in Established State"},
		{"Unspecific", "Maximum Number of Prefixes Reached", "Administrative Shutdown", "Connection Rejected", "Other Configuration Change", "Connection Collision Resolution", "Out of Resources", "Hard Reset"},
		{"Reserved", "Invalid Message Length"}}

	ErrStr := ErrCodeStr[bs[0]].Str

	if int(bs[1]) > ErrCodeStr[bs[0]].SubCodeSz {
		ErrStr += " : Unknown sub error " + strconv.Itoa(int(bs[1]))
	} else if ErrCodeStr[bs[0]].SubCodeSz > 0 {
		ErrStr += " : " + SubErrCodeStr[bs[0]][bs[1]]
	}
	return ErrStr
}

func newBGPPeerStatus(in *pds.BGPPeerStatus) ShadowBGPPeerStatus {
	return ShadowBGPPeerStatus{
		Id:               "",
		LastErrorRcvd:    BgpErrStr(in.LastErrorRcvd),
		LastErrorSent:    BgpErrStr(in.LastErrorSent),
		Status:           strings.TrimPrefix(in.Status.String(), "BGP_PEER_STATE_"),
		PrevStatus:       strings.TrimPrefix(in.PrevStatus.String(), "BGP_PEER_STATE_"),
		LocalAddr:        PdsIPToString(in.LocalAddr),
		CapsSent:         BgpCapsStr(BgpCapabilities(in.CapsSent)),
		CapsRcvd:         BgpCapsStr(BgpCapabilities(in.CapsRcvd)),
		CapsNeg:          BgpCapsStr(BgpCapabilities(in.CapsNeg)),
		SelLocalAddrType: strings.TrimPrefix(in.SelLocalAddrType.String(), "BGP_ADDR_TYPE_"),
		BGPPeerStatus:    in,
	}
}

// ShadowBGPPeer shadows the BGPPeer for CLI purposes
type ShadowBGPPeer struct {
	Spec   ShadowBGPPeerSpec
	Status ShadowBGPPeerStatus
}

// NewBGPPeer creates a shadow of BGPPeer
func NewBGPPeer(in *pds.BGPPeer) *ShadowBGPPeer {
	return &ShadowBGPPeer{
		Spec:   newBGPPeerSpec(in.Spec),
		Status: newBGPPeerStatus(in.Status),
	}
}

// ShadowBGPPeerAfSpec shadows the BGPPeerAf for CLI purposes
type ShadowBGPPeerAFSpec struct {
	*pds.BGPPeerAfSpec
	Id        string
	LocalAddr string
	PeerAddr  string
	Afi       string
	Safi      string
}

// NewBGPPeerAfSpec creates a shadow of BGPPeerAFSpec
func NewBGPPeerAfSpec(in *pds.BGPPeerAfSpec) ShadowBGPPeerAFSpec {
	uid, err := uuid.FromBytes(in.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}
	return ShadowBGPPeerAFSpec{
		BGPPeerAfSpec: in,
		Id:            uidstr,
		LocalAddr:     PdsIPToString(in.LocalAddr),
		PeerAddr:      PdsIPToString(in.PeerAddr),
		Afi:           strings.TrimPrefix(in.Afi.String(), "BGP_AFI_"),
		Safi:          strings.TrimPrefix(in.Safi.String(), "BGP_SAFI_"),
	}
}

// ShadowBGPPeerAfStatus shadows the BGPPeerAfStatus for CLI purposes
type ShadowBGPPeerAFStatus struct {
	*pds.BGPPeerAfStatus
	UpdGrp uint32
}

// NewBGPPeerAfStatus creates a shadow of BGPPeerAF
func NewBGPPeerAfStatus(in *pds.BGPPeerAfStatus) ShadowBGPPeerAFStatus {
	return ShadowBGPPeerAFStatus{
		BGPPeerAfStatus: in,
		UpdGrp:          in.UpdateGroup,
	}
}

// ShadowBGPPeerAF shadows the BGPPeerAf for CLI purposes
type ShadowBGPPeerAF struct {
	Spec   ShadowBGPPeerAFSpec
	Status ShadowBGPPeerAFStatus
}

// NewBGPPeerAf creates a shadow of BGPPeerAF
func NewBGPPeerAf(in *pds.BGPPeerAf) *ShadowBGPPeerAF {
	return &ShadowBGPPeerAF{
		Spec:   NewBGPPeerAfSpec(in.Spec),
		Status: NewBGPPeerAfStatus(in.Status),
	}
}

// ShadowBGPNLRIPrefixStatus is a shadow of the BGPNLRIPrefixStatus
type ShadowBGPNLRIPrefixStatus struct {
	Prefix           *NLRIPrefix
	ASPathStr        string
	PathOrigId       string
	NextHopAddr      string
	RouteSource      string
	FlapStatsFlapcnt uint32
	FlapStatsSupprsd bool
	IsActive         string
	Stale            bool
	FlapStartTime    uint32
	ReasonNotBest    string
	EcmpRoute        bool
	PeerAddr         string
	ExtComm          []string
	*pds.BGPNLRIPrefixStatus
}

func BGPASPath(ASSize int, ASPath []byte) string {
	// Flag Type Total-Len {ASSegmentType NumAS {AS}*}*
	TotalLen := int(ASPath[2])
	if TotalLen == 0 {
		return "NONE"
	}
	FirstASSeg := true
	var ASStr string
	for ASSegStart := 3; TotalLen > 0; {
		if !FirstASSeg {
			ASStr += " { "
		}
		NumAS := int(ASPath[ASSegStart+1])
		ASSegLen := 2 + NumAS*ASSize
		asseq := ASPath[ASSegStart+2 : ASSegStart+ASSegLen]
		FirstAS := true
		for i := 0; i < NumAS; i++ {
			asint := binary.BigEndian.Uint32(asseq[i*ASSize : (i+1)*ASSize])
			if !FirstAS {
				ASStr += " "
			} else {
				FirstAS = false
			}
			ASStr += strconv.FormatUint(uint64(asint), 10)
		}
		if !FirstASSeg {
			ASStr += " } "
		} else {
			FirstASSeg = false
		}
		ASSegStart += ASSegLen
		TotalLen -= ASSegLen
	}
	return ASStr
}

func BGPRouteSource(routeSrc pds.NLRISrc, peerip *pds.IPAddress) string {
	if routeSrc != pds.NLRISrc_NLRI_PEER {
		return "LOCAL"
	}
	return PdsIPToString(peerip)
}

func NewBGPNLRIPrefixStatus(in *pds.BGPNLRIPrefixStatus) *ShadowBGPNLRIPrefixStatus {
	var pathOrigId string

	if net.IP(in.PathOrigId).String() == "0.0.0.0" {
		pathOrigId = "<not set>"
	} else {
		pathOrigId = net.IP(in.PathOrigId).String()
	}

	var ASSize int

	switch in.ASSize {
	case pds.BGPASSize_BGP_AS_SIZE_TWO_OCTET:
		ASSize = 2
	case pds.BGPASSize_BGP_AS_SIZE_FOUR_OCTET:
		ASSize = 4
	default:
		log.Errorf("Invalid AS Size")
	}
	ret := ShadowBGPNLRIPrefixStatus{
		ASPathStr:           BGPASPath(ASSize, in.ASPathStr),
		PathOrigId:          pathOrigId,
		NextHopAddr:         net.IP(in.NextHopAddr).String(),
		Prefix:              NewNLRIPrefix(int(in.Afi), int(in.Safi), in.Prefix),
		RouteSource:         BGPRouteSource(in.RouteSource, in.PeerAddr),
		FlapStatsFlapcnt:    in.FlapStatsFlapcnt,
		FlapStatsSupprsd:    in.FlapStatsSupprsd,
		IsActive:            strings.TrimPrefix(in.IsActive.String(), "BGP_NLRI_ISA_"),
		Stale:               in.Stale,
		FlapStartTime:       in.FlapStartTime,
		ReasonNotBest:       strings.TrimPrefix(in.ReasonNotBest.String(), "BGP_REASON_"),
		EcmpRoute:           in.EcmpRoute,
		PeerAddr:            PdsIPToString(in.PeerAddr),
		BGPNLRIPrefixStatus: in,
	}
	ret.ExtComm = dumpExtComm(in.ExtComm)
	return &ret
}

func dumpExtComm(in [][]byte) []string {
	var r []string
	for _, b := range in {
		v := dumpRt(b)
		if v != "" {
			r = append(r, v)
		}
	}
	return r
}

//
// ShadowEvpnIpVrf
//

// ShadowEvpnIpVrfSpec shadows the EvpnIpVrfSpec for CLI purposes
type ShadowEvpnIpVrfSpec struct {
	Id    string
	VPCId string
	RD    string
	*pds.EvpnIpVrfSpec
}

func NewEvpnIpVrfSpec(in *pds.EvpnIpVrfSpec) ShadowEvpnIpVrfSpec {
	uid, err := uuid.FromBytes(in.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}
	return ShadowEvpnIpVrfSpec{
		Id:            uidstr,
		VPCId:         string(in.VPCId),
		RD:            printRD(in.RD),
		EvpnIpVrfSpec: in,
	}
}

// ShadowEvpnIpVrfStatus shadows the EvpnIpVrfStatus for CLI purposes
type ShadowEvpnIpVrfStatus struct {
	*pds.EvpnIpVrfStatus
}

func NewEvpnIpVrfStatus(in *pds.EvpnIpVrfStatus) ShadowEvpnIpVrfStatus {
	return ShadowEvpnIpVrfStatus{
		EvpnIpVrfStatus: in,
	}
}

// ShadowEvpnIpVrf shadows the EvpnIpVrf for CLI purposes
type ShadowEvpnIpVrf struct {
	Spec   ShadowEvpnIpVrfSpec
	Status ShadowEvpnIpVrfStatus
}

func NewEvpnIpVrf(in *pds.EvpnIpVrf) *ShadowEvpnIpVrf {
	return &ShadowEvpnIpVrf{
		Spec:   NewEvpnIpVrfSpec(in.Spec),
		Status: NewEvpnIpVrfStatus(in.Status),
	}
}

// ShadowEvpnMacIpStatus shadows the EvpnMacIpStatus for CLI purposes
type ShadowEvpnMacIpStatus struct {
	EVIId      uint32
	EthTagID   uint32
	MACAddress string
	IPAddress  string
	PathID     uint32
	Source     string
	NHAddress  string
	LocalIfId  uint32
	Label      uint32
	InUse      bool
	Esi        string
	SeqNum     uint32
	Sticky     bool
	*pds.EvpnMacIpStatus
}

func NewEvpnMacIpStatus(in *pds.EvpnMacIpStatus) ShadowEvpnMacIpStatus {
	return ShadowEvpnMacIpStatus{
		EVIId:           in.EVIId,
		EthTagID:        in.EthTagID,
		MACAddress:      dumpBytes([]byte(in.MACAddress)),
		IPAddress:       PdsIPToString(in.IPAddress),
		PathID:          in.PathID,
		Source:          strings.TrimPrefix(in.Source.String(), "EVPN_SOURCE_"),
		NHAddress:       PdsIPToString(in.NHAddress),
		LocalIfId:       in.LocalIfId,
		Label:           in.Label,
		InUse:           in.InUse,
		Esi:             in.Esi,
		SeqNum:          in.SeqNum,
		Sticky:          in.Sticky,
		EvpnMacIpStatus: in,
	}
}

// ShadowEvpnMacIp shadows the EvpnMacIp for CLI purposes
type ShadowEvpnMacIp struct {
	Status ShadowEvpnMacIpStatus
}

func NewEvpnMacIp(in *pds.EvpnMacIp) *ShadowEvpnMacIp {
	return &ShadowEvpnMacIp{
		Status: NewEvpnMacIpStatus(in.Status),
	}
}

//
// ShadowEvpnIpVrfRt
//

// ShadowEvpnIpVrfRtSpec shadows the EvpnIpVrfRtSpec for CLI purposes
type ShadowEvpnIpVrfRtSpec struct {
	Id     string
	VPCId  string
	RT     string
	RTType string
	*pds.EvpnIpVrfRtSpec
}

func dumpRt(in []byte) string {
	rt := ""
	if len(in) != 8 || int(in[1]) != 2 {
		return rt
	}
	inStr := dumpNonHexBytes(in)
	inStrSlice := strings.Split(inStr, " ")
	str := strings.Join(inStrSlice[:1], "")
	var rttype int64
	var err error
	if rttype, err = strconv.ParseInt(str, 16, 64); err == nil {
		rt += fmt.Sprintf("Type: %v, ", rttype)
	}
	str = strings.Join(inStrSlice[1:2], "")
	if s, err := strconv.ParseInt(str, 16, 64); err == nil {
		rt += fmt.Sprintf("Sub-type: %v, ", s)
	}
	if (rttype == 1) || (rttype == 2) {
		str = strings.Join(inStrSlice[2:6], "")
		if s, err := strconv.ParseInt(str, 16, 64); err == nil {
			if rttype == 1 {
				rt += fmt.Sprintf("IP: %v", Uint32ToIPv4Address(uint32(s)))
			} else {
				rt += fmt.Sprintf("AS: %v, ", s)
			}
		}
		str = strings.Join(inStrSlice[6:], "")
		if s, err := strconv.ParseInt(str, 16, 64); err == nil {
			rt += fmt.Sprintf("AN: %v", s)
		}
	} else if rttype == 0 {
		str = strings.Join(inStrSlice[2:4], "")
		if s, err := strconv.ParseInt(str, 16, 64); err == nil {
			rt += fmt.Sprintf("AS: %v, ", s)
		}
		str = strings.Join(inStrSlice[4:], "")
		if s, err := strconv.ParseInt(str, 16, 64); err == nil {
			rt += fmt.Sprintf("AN: %v", s)
		}
	} else if rttype == 3 {
		str = strings.Join(inStrSlice[2:], "")
		if s, err := strconv.ParseInt(str, 16, 64); err == nil {
			rt += fmt.Sprintf("AS: %v, ", s)
		}
	}
	return rt
}

func NewEvpnIpVrfRtSpec(in *pds.EvpnIpVrfRtSpec) ShadowEvpnIpVrfRtSpec {
	uid, err := uuid.FromBytes(in.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}
	return ShadowEvpnIpVrfRtSpec{
		Id:              uidstr,
		VPCId:           string(in.VPCId),
		RT:              dumpRt(in.RT),
		RTType:          strings.TrimPrefix(in.RTType.String(), "EVPN_RT_"),
		EvpnIpVrfRtSpec: in,
	}
}

// ShadowEvpnIpVrfRtStatus shadows the EvpnIpVrfRtStatus for CLI purposes
type ShadowEvpnIpVrfRtStatus struct {
	*pds.EvpnIpVrfRtStatus
}

func NewEvpnIpVrfRtStatus(in *pds.EvpnIpVrfRtStatus) ShadowEvpnIpVrfRtStatus {
	return ShadowEvpnIpVrfRtStatus{
		EvpnIpVrfRtStatus: in,
	}
}

// ShadowEvpnIpVrfRt shadows the EvpnIpVrfRt for CLI purposes
type ShadowEvpnIpVrfRt struct {
	Spec   ShadowEvpnIpVrfRtSpec
	Status ShadowEvpnIpVrfRtStatus
}

func NewEvpnIpVrfRt(in *pds.EvpnIpVrfRt) *ShadowEvpnIpVrfRt {
	return &ShadowEvpnIpVrfRt{
		Spec:   NewEvpnIpVrfRtSpec(in.Spec),
		Status: NewEvpnIpVrfRtStatus(in.Status),
	}
}

//
// ShadowEvpnEvi
//

// ShadowEvpnEviSpec shadows the EvpnEviSpec for CLI purposes
type ShadowEvpnEviSpec struct {
	Id       string
	SubnetId string
	RD       string
	RTType   string
	*pds.EvpnEviSpec
}

func NewEvpnEviSpec(in *pds.EvpnEviSpec) ShadowEvpnEviSpec {
	uid, err := uuid.FromBytes(in.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}
	return ShadowEvpnEviSpec{
		Id:          uidstr,
		SubnetId:    string(in.SubnetId),
		RD:          printRD(in.RD),
		RTType:      strings.TrimPrefix(in.RTType.String(), "EVPN_RT_"),
		EvpnEviSpec: in,
	}
}

// ShadowEvpnEviStatus shadows the EvpnEviStatus for CLI purposes
type ShadowEvpnEviStatus struct {
	RD     string
	Status string
	*pds.EvpnEviStatus
}

func NewEvpnEviStatus(in *pds.EvpnEviStatus) ShadowEvpnEviStatus {
	return ShadowEvpnEviStatus{
		RD:            printRD(in.RD),
		Status:        strings.TrimPrefix(in.Status.String(), "EVPN_OPER_STATUS_"),
		EvpnEviStatus: in,
	}
}

// ShadowEvpnEvi shadows the EvpnEvi for CLI purposes
type ShadowEvpnEvi struct {
	Spec   ShadowEvpnEviSpec
	Status ShadowEvpnEviStatus
}

func NewEvpnEvi(in *pds.EvpnEvi) *ShadowEvpnEvi {
	return &ShadowEvpnEvi{
		Spec:   NewEvpnEviSpec(in.Spec),
		Status: NewEvpnEviStatus(in.Status),
	}
}

//
// ShadowEvpnEviRt
//

// ShadowEvpnEviRtSpec shadows the EvpnEviRtSpec for CLI purposes
type ShadowEvpnEviRtSpec struct {
	Id       string
	SubnetId string
	RT       string
	RTType   string
	*pds.EvpnEviRtSpec
}

func NewEvpnEviRtSpec(in *pds.EvpnEviRtSpec) ShadowEvpnEviRtSpec {
	uid, err := uuid.FromBytes(in.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}
	return ShadowEvpnEviRtSpec{
		Id:            uidstr,
		SubnetId:      string(in.SubnetId),
		RT:            dumpRt(in.RT),
		RTType:        strings.TrimPrefix(in.RTType.String(), "EVPN_RT_"),
		EvpnEviRtSpec: in,
	}
}

// ShadowEvpnEviRtStatus shadows the EvpnEviRtStatus for CLI purposes
type ShadowEvpnEviRtStatus struct {
	*pds.EvpnEviRtStatus
}

func NewEvpnEviRtStatus(in *pds.EvpnEviRtStatus) ShadowEvpnEviRtStatus {
	return ShadowEvpnEviRtStatus{
		EvpnEviRtStatus: in,
	}
}

// ShadowEvpnEviRt shadows the EvpnEviRt for CLI purposes
type ShadowEvpnEviRt struct {
	Spec   ShadowEvpnEviRtSpec
	Status ShadowEvpnEviRtStatus
}

func NewEvpnEviRt(in *pds.EvpnEviRt) *ShadowEvpnEviRt {
	return &ShadowEvpnEviRt{
		Spec:   NewEvpnEviRtSpec(in.Spec),
		Status: NewEvpnEviRtStatus(in.Status),
	}
}

// ShadowLimIfStatus shadows the LimIfStatus for CLI purposes
type ShadowLimIfStatus struct {
	*pds.LimIfStatus
	OperStatus   string
	Name         string
	Description  string
	Type         string
	MacAddr      string
	Mtu          uint32
	LoopBackMode string
	FlapCount    uint32
	OperReason   string
}

// NewLimIfStatusGetResp creates a new shadow of the LimIfStatus
func NewLimIfStatusGetResp(status *pds.LimIfStatus) *ShadowLimIfStatus {
	return &ShadowLimIfStatus{
		LimIfStatus:  status,
		OperStatus:   strings.TrimPrefix(status.OperStatus.String(), "OPER_"),
		Name:         status.Name,
		Description:  status.Description,
		Type:         strings.TrimPrefix(status.Type.String(), "IFTYP_"),
		MacAddr:      dumpBytes([]byte(status.MacAddr)),
		Mtu:          status.Mtu,
		LoopBackMode: status.LoopBackMode.String(),
		FlapCount:    status.FlapCount,
		OperReason:   strings.TrimPrefix(status.OperReason.String(), "OPR_RSN_"),
	}
}

// ShadowCPStaticRouteSpec shadows the CPStaticRouteSpec for CLI purposes
type ShadowCPStaticRouteSpec struct {
	DestAddr    string
	PrefixLen   uint32
	NextHopAddr string
	State       string
	Override    bool
	AdminDist   uint32
	*pds.CPStaticRouteSpec
}

func newCPStaticRouteSpec(in *pds.CPStaticRouteSpec) ShadowCPStaticRouteSpec {
	return ShadowCPStaticRouteSpec{
		DestAddr:          PdsIPToString(in.DestAddr),
		PrefixLen:         in.PrefixLen,
		NextHopAddr:       PdsIPToString(in.NextHopAddr),
		State:             strings.TrimPrefix(in.State.String(), "ADMIN_STATE_"),
		Override:          in.Override,
		AdminDist:         in.AdminDist,
		CPStaticRouteSpec: in,
	}
}

// ShadowCPStaticRoute shadows the CPStaticRoute for CLI purposes
type ShadowCPStaticRoute struct {
	Spec ShadowCPStaticRouteSpec
}

// NewCPStaticRoute creates a shadow of CPStaticRoute
func NewCPStaticRoute(in *pds.CPStaticRoute) *ShadowCPStaticRoute {
	return &ShadowCPStaticRoute{
		Spec: newCPStaticRouteSpec(in.Spec),
	}
}

// ShadowCPActiveRouteStatus shadows the CPActiveRouteStatus for CLI purposes
type ShadowCPActiveRouteStatus struct {
	RouteTableId  uint32
	DestAddr      string
	DestPrefixLen uint32
	NHAddr        string
	IfIndex       uint32
	Type          string
	Proto         string
	Age           int32
	Metric1       int32
	Connected     bool
	AdminDistance int32
	*pds.CPActiveRouteStatus
}

func newCPActiveRouteStatus(in *pds.CPActiveRouteStatus) ShadowCPActiveRouteStatus {
	return ShadowCPActiveRouteStatus{
		RouteTableId:        in.RouteTableId,
		DestAddr:            PdsIPToString(in.DestAddr),
		DestPrefixLen:       in.DestPrefixLen,
		NHAddr:              PdsIPToString(in.NHAddr),
		IfIndex:             in.IfIndex,
		Type:                strings.TrimPrefix(in.Type.String(), "ROUTE_TYPE_"),
		Proto:               strings.TrimPrefix(in.Proto.String(), "ROUTE_PROTO_"),
		Age:                 in.Age,
		Metric1:             in.Metric1,
		Connected:           in.Connected,
		AdminDistance:       in.AdminDistance,
		CPActiveRouteStatus: in,
	}
}

// ShadowCPActiveRoute shadows the CPActiveRoute for CLI purposes
type ShadowCPActiveRoute struct {
	Status ShadowCPActiveRouteStatus
}

// NewCPActiveRoute creates a shadow of CPActiveRoute
func NewCPActiveRoute(in *pds.CPActiveRoute) *ShadowCPActiveRoute {
	return &ShadowCPActiveRoute{
		Status: newCPActiveRouteStatus(in.Status),
	}
}

// ShadowLimIfAddrTableStatus shadows the LimIfAddrTableStatus for CLI purposes
type ShadowLimIfAddrTableStatus struct {
	*pds.LimIfAddrTableStatus
	IfIndex    uint32
	IPAddr     string
	OperStatus string
}

// NewLimIfAddrTableGetResponse creates a new shadow of the LimIfAddrTableStatus
func NewLimIfAddrTableGetResponse(status *pds.LimIfAddrTableStatus) *ShadowLimIfAddrTableStatus {
	return &ShadowLimIfAddrTableStatus{
		IfIndex:    status.IfIndex,
		IPAddr:     PdsIPToString(status.IPAddr),
		OperStatus: strings.TrimPrefix(status.OperStatus.String(), "OPER_STATUS_"),
	}
}

// ShadowCPRouteStatus shadows the CPRouteStatus for CLI purposes
type ShadowCPRouteStatus struct {
	RouteTableId  uint32
	DestAddr      string
	DestPrefixLen uint32
	NHAddr        string
	IfIndex       uint32
	Type          string
	Proto         string
	Age           int32
	Metric1       int32
	FibRoute      bool
	Connected     bool
	LooseNextHop  bool
	AdminDistance int32
	*pds.CPRouteStatus
}

func newCPRouteStatus(in *pds.CPRouteStatus) ShadowCPRouteStatus {
	return ShadowCPRouteStatus{
		RouteTableId:  in.RouteTableId,
		DestAddr:      PdsIPToString(in.DestAddr),
		DestPrefixLen: in.DestPrefixLen,
		NHAddr:        PdsIPToString(in.NHAddr),
		IfIndex:       in.IfIndex,
		Type:          strings.TrimPrefix(in.Type.String(), "ROUTE_TYPE_"),
		Proto:         strings.TrimPrefix(in.Proto.String(), "ROUTE_PROTO_"),
		Age:           in.Age,
		Metric1:       in.Metric1,
		FibRoute:      in.FibRoute,
		Connected:     in.Connected,
		LooseNextHop:  in.LooseNextHop,
		AdminDistance: in.AdminDistance,
		CPRouteStatus: in,
	}
}

// ShadowCPRoute shadows the CPRoute for CLI purposes
type ShadowCPRoute struct {
	Status ShadowCPRouteStatus
}

// NewCPRoute creates a shadow of CPRoute
func NewCPRoute(in *pds.CPRoute) *ShadowCPRoute {
	return &ShadowCPRoute{
		Status: newCPRouteStatus(in.Status),
	}
}

// ShadowBGPPrfxCntrsStatus is a shadow of the BGPRouteMapStatus
type ShadowBGPPrfxCntrsStatus struct {
	EntIndex                uint32
	PeerIndex               uint32
	Afi                     string
	Safi                    string
	InPrfxes                uint32
	InPrfxesAccepted        uint32
	InPrfxesRejected        uint32
	OutPrfxes               uint32
	OutPrfxesAdvertised     uint32
	UserData                string
	InPrfxesFlapped         uint32
	InPrfxesFlapSuppressed  uint32
	InPrfxesFlapHistory     uint32
	InPrfxesActive          uint32
	InPrfxesDeniedByPol     uint32
	NumLocRibRoutes         uint32
	NumLocRibBestRoutes     uint32
	InPrfxesDeniedMartian   uint32
	InPrfxesDeniedAsLoop    uint32
	InPrfxesDeniedNextHop   uint32
	InPrfxesDeniedAsLength  uint32
	InPrfxesDeniedCommunity uint32
	InPrfxesDeniedLocalOrig uint32
	InTotalPrfxes           uint32
	OutTotalPrfxes          uint32
	PeerState               string
	OutPrfxesDenied         uint32
	OutPrfxesImpWdr         uint32
	OutPrfxesExpWdr         uint32
	InPrfxesImpWdr          uint32
	InPrfxesExpWdr          uint32
	CurPrfxesDeniedByPol    uint32
	*pds.BGPPrfxCntrsStatus
}

func NewBGPPrfxCntrsStatus(in *pds.BGPPrfxCntrsStatus) *ShadowBGPPrfxCntrsStatus {
	return &ShadowBGPPrfxCntrsStatus{
		EntIndex:                in.EntIndex,
		PeerIndex:               in.PeerIndex,
		Afi:                     strings.TrimPrefix(in.Afi.String(), "BGP_AFI_"),
		Safi:                    strings.TrimPrefix(in.Safi.String(), "BGP_SAFI_"),
		InPrfxes:                in.InPrfxes,
		InPrfxesAccepted:        in.InPrfxesAccepted,
		InPrfxesRejected:        in.InPrfxesRejected,
		OutPrfxes:               in.OutPrfxes,
		OutPrfxesAdvertised:     in.OutPrfxesAdvertised,
		UserData:                dumpBytes([]byte(in.UserData)),
		InPrfxesFlapped:         in.InPrfxesFlapped,
		InPrfxesFlapSuppressed:  in.InPrfxesFlapSuppressed,
		InPrfxesFlapHistory:     in.InPrfxesFlapHistory,
		InPrfxesActive:          in.InPrfxesActive,
		InPrfxesDeniedByPol:     in.InPrfxesDeniedByPol,
		NumLocRibRoutes:         in.NumLocRibRoutes,
		NumLocRibBestRoutes:     in.NumLocRibBestRoutes,
		InPrfxesDeniedMartian:   in.InPrfxesDeniedMartian,
		InPrfxesDeniedAsLoop:    in.InPrfxesDeniedAsLoop,
		InPrfxesDeniedNextHop:   in.InPrfxesDeniedNextHop,
		InPrfxesDeniedAsLength:  in.InPrfxesDeniedAsLength,
		InPrfxesDeniedCommunity: in.InPrfxesDeniedCommunity,
		InPrfxesDeniedLocalOrig: in.InPrfxesDeniedLocalOrig,
		InTotalPrfxes:           in.InTotalPrfxes,
		OutTotalPrfxes:          in.OutTotalPrfxes,
		PeerState:               strings.TrimPrefix(in.PeerState.String(), "BGP_PEER_STATE_"),
		OutPrfxesDenied:         in.OutPrfxesDenied,
		OutPrfxesImpWdr:         in.OutPrfxesImpWdr,
		OutPrfxesExpWdr:         in.OutPrfxesExpWdr,
		InPrfxesImpWdr:          in.InPrfxesImpWdr,
		InPrfxesExpWdr:          in.InPrfxesExpWdr,
		CurPrfxesDeniedByPol:    in.CurPrfxesDeniedByPol,
		BGPPrfxCntrsStatus:      in,
	}
}

// ShadowBGPRouteMapStatus is a shadow of the BGPRouteMapStatus
type ShadowBGPRouteMapStatus struct {
	EntIndex uint32
	Index    uint32
	Number   uint32
	Hitcnt   int32
	OrfAssoc string
	*pds.BGPRouteMapStatus
}

func NewBGPRouteMapStatus(in *pds.BGPRouteMapStatus) *ShadowBGPRouteMapStatus {
	return &ShadowBGPRouteMapStatus{
		EntIndex:          in.EntIndex,
		Index:             in.Index,
		Number:            in.Number,
		Hitcnt:            in.Hitcnt,
		OrfAssoc:          strings.TrimPrefix(in.OrfAssoc.String(), "BGP_ORF_ASSOC_"),
		BGPRouteMapStatus: in,
	}
}

// ShadowEvpnBdStatus shadows the EvpnBdStatus for CLI purposes
type ShadowEvpnBdStatus struct {
	EntityIndex uint32
	EviIndex    uint32
	OperStatus  string
	OperReason  string
	*pds.EvpnBdStatus
}

func NewEvpnBdStatus(in *pds.EvpnBdStatus) ShadowEvpnBdStatus {
	return ShadowEvpnBdStatus{
		EntityIndex:  in.EntityIndex,
		EviIndex:     in.EviIndex,
		OperStatus:   strings.TrimPrefix(in.OperStatus.String(), "EVPN_OPER_STATUS_"),
		OperReason:   strings.TrimPrefix(in.OperReason.String(), "EVPN_"),
		EvpnBdStatus: in,
	}
}

// ShadowEvpnBd shadows the EvpnBd for CLI purposes
type ShadowEvpnBd struct {
	Status ShadowEvpnBdStatus
}

func NewEvpnBd(in *pds.EvpnBd) *ShadowEvpnBd {
	return &ShadowEvpnBd{
		Status: NewEvpnBdStatus(in.Status),
	}
}

// ShadowEvpnBdIfStatus shadows the EvpnBdIfStatus for CLI purposes
type ShadowEvpnBdIfStatus struct {
	EntityIndex uint32
	EviIndex    uint32
	OperStatus  string
	OperReason  string
	IfId        uint32
	*pds.EvpnBdIfStatus
}

func NewEvpnBdIfStatus(in *pds.EvpnBdIfStatus) ShadowEvpnBdIfStatus {
	return ShadowEvpnBdIfStatus{
		EntityIndex:    in.EntityIndex,
		EviIndex:       in.EviIndex,
		IfId:           in.IfId,
		OperStatus:     strings.TrimPrefix(in.OperStatus.String(), "EVPN_OPER_STATUS_"),
		OperReason:     strings.TrimPrefix(in.OperReason.String(), "EVPN_"),
		EvpnBdIfStatus: in,
	}
}

// ShadowEvpnBdIf shadows the EvpnBdIf for CLI purposes
type ShadowEvpnBdIf struct {
	Status ShadowEvpnBdIfStatus
}

func NewEvpnBdIf(in *pds.EvpnBdIf) *ShadowEvpnBdIf {
	return &ShadowEvpnBdIf{
		Status: NewEvpnBdIfStatus(in.Status),
	}
}

// ShadowLimVrfStatus shadows the LimVrfStatus for CLI purposes
type ShadowLimVrfStatus struct {
	*pds.LimVrfStatus
	EntityIndex   uint32
	VrfName       string
	Desc          string
	Description   string
	OperStatus    string
	OperReason    string
	NumInterfaces uint32
}

// NewLimVrfGetResp creates a new shadow of the LimVrfStatus
func NewLimVrfGetResp(status *pds.LimVrfStatus) *ShadowLimVrfStatus {
	return &ShadowLimVrfStatus{
		LimVrfStatus:  status,
		EntityIndex:   status.EntityIndex,
		VrfName:       status.VrfName,
		Description:   status.Desc,
		OperReason:    strings.TrimPrefix(status.OperReason.String(), "OPR_RSN_"),
		OperStatus:    strings.TrimPrefix(status.OperStatus.String(), "OPER_STATUS_"),
		NumInterfaces: status.NumInterfaces,
	}
}

// ShadowCPRouteRedistStatus shadows the CPRouteRedistStatus for CLI purposes
type ShadowCPRouteRedistStatus struct {
	FteIndex       uint32
	EntryId        uint32
	RuleUsageCount uint32
	AddrFilter     string
	*pds.CPRouteRedistStatus
}

func newCPRouteRedistStatus(in *pds.CPRouteRedistStatus) ShadowCPRouteRedistStatus {
	return ShadowCPRouteRedistStatus{
		FteIndex:            in.FteIndex,
		EntryId:             in.EntryId,
		RuleUsageCount:      in.RuleUsageCount,
		AddrFilter:          PdsIPToString(in.AddrFilter),
		CPRouteRedistStatus: in,
	}
}

// ShadowCPRouteRedist shadows the CPRouteRedist for CLI purposes
type ShadowCPRouteRedist struct {
	Status ShadowCPRouteRedistStatus
}

// NewCPRouteRedist creates a shadow of CPRouteRedist
func NewCPRouteRedist(in *pds.CPRouteRedist) *ShadowCPRouteRedist {
	return &ShadowCPRouteRedist{
		Status: newCPRouteRedistStatus(in.Status),
	}
}

// ShadowLimSwIfStatus shadows the LimSwIfStatus for CLI purposes
type ShadowLimSwIfStatus struct {
	*pds.LimSwIfStatus
	EntityIndex int32
	Type        string
	Index       uint32
	OperStatus  string
	Name        string
	IfIndex     int32
}

// NewLimSwIfGetResponse creates a new shadow of the LimSwIfStatus
func NewLimSwIfGetResponse(status *pds.LimSwIfStatus) *ShadowLimSwIfStatus {
	return &ShadowLimSwIfStatus{
		EntityIndex: status.EntityIndex,
		Type:        strings.TrimPrefix(status.Type.String(), "LIM_SOFTWIF_"),
		Index:       status.Index,
		Name:        status.Name,
		IfIndex:     status.IfIndex,
		OperStatus:  strings.TrimPrefix(status.OperStatus.String(), "OPER_STATUS_"),
	}
}
