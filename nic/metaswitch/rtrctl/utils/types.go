package utils

import (
	"encoding/binary"
	"fmt"
	"net"
	"strings"

	"github.com/satori/go.uuid"

	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	type2MinLen = 8 + 10 + 4 + 1 + 6 + 1 + 3
	type2MaxLen = 8 + 10 + 4 + 1 + 6 + 1 + 16 + 3 + 3
	type5MinLen = 8 + 10 + 4 + 1 + 4 + 4 + 3
	type5MaxLen = 8 + 10 + 4 + 1 + 16 + 16 + 3
)

// NLRIPrefix is a representation of the BGP Route
type NLRIPrefix struct {
	Type   int
	Length int
	Prefix EVPNPrefix
}

func dumpBytes(in []byte) string {
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
	return fmt.Sprintf("Type : %d  %v", n.Type, n.Prefix)
}

func NewNLRIPrefix(in []byte) *NLRIPrefix {
	if len(in) < 3 {
		return nil
	}
	ret := &NLRIPrefix{
		Type:   int(in[0]),
		Length: int(in[1]),
	}
	switch ret.Type {
	case 2:
		p := &EVPNType2Route{}
		p.parseBytes(in[2:])
		ret.Prefix = newEVPNType2Route(p)

	case 5:
		p := &EVPNType5Route{}
		p.parseBytes(in[2:])
		ret.Prefix = newEVPNType5Route(p)
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

const type2Fmt = `
                   ----------------
                   RD             : %v
                   ESI            : %v
                   Ethernet Tag   : %v
                   MAC Address    : %v
                   IPAddress      : %v
                   Label1         : %v
                   Label2         : %v`

// String returns a user friendly string
func (s *ShadowEVPNType2Route) String() string {
	return fmt.Sprintf(type2Fmt, s.RD, s.ESI, s.EthTagID, s.MACAddress, s.IPAddress, s.MPLSLabel1, s.MPLSLabel2)
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
	case 4:
		a.IPAddress = make([]byte, 4)
		copy(a.IPAddress, in[cur:cur+4])
		cur += 4
	case 16:
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
	*EVPNType5Route
}

const type5Fmt = `
                   ----------------
                   RD             : %v
                   ESI            : %v
                   Ethernet Tag   : %v
                   IPPrefix       : %v
                   Prefix Length  : %v
                   Gateway IP     : %v
                   Label1         : %v`

// String returns a user friendly string
func (s *ShadowEVPNType5Route) String() string {
	return fmt.Sprintf(type5Fmt, s.RD, s.ESI, s.EthTagID, s.IPPrefix, s.IPPrefixLen, s.GWIPAddress, dumpBytes(s.MPLSLabel1))
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
	}
}

type EVPNPrefix interface {
	parseBytes(in []byte)
}

// Uint32ToIPv4Address returns an IP Address string given an integer
func Uint32ToIPv4Address(in uint32) string {
	ip := make(net.IP, 4)
	binary.LittleEndian.PutUint32(ip, in)
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

// ShadowBgpSpec shadows the BGPSpec for CLI purposes
type ShadowBgpSpec struct {
	*pds.BGPSpec
	Id       string
	RouterId string
}

// NewBGPSpec creates a new shadow of the BGPSpec
func NewBGPSpec(in *pds.BGPSpec) *ShadowBgpSpec {
	uid, err := uuid.FromBytes(in.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}

	return &ShadowBgpSpec{
		BGPSpec:  in,
		Id:       uidstr,
		RouterId: Uint32ToIPv4Address(in.RouterId),
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
	Id            string
	LastErrorRcvd string
	LastErrorSent string
	Status        string
	PrevStatus    string
	*pds.BGPPeerStatus
}

func newBGPPeerStatus(in *pds.BGPPeerStatus) ShadowBGPPeerStatus {
	return ShadowBGPPeerStatus{
		Id:            "",
		LastErrorRcvd: string(in.LastErrorRcvd),
		LastErrorSent: string(in.LastErrorSent),
		Status:        strings.TrimPrefix(in.Status.String(), "BGP_PEER_STATE_"),
		PrevStatus:    in.PrevStatus.String(),
		BGPPeerStatus: in,
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

// ShadowBGPPeerAf shadows the BGPPeerAf for CLI purposes
type ShadowBGPPeerAFSpec struct {
	*pds.BGPPeerAfSpec
	Id        string
	LocalAddr string
	PeerAddr  string
	Afi       string
	Safi      string
}

// NewBGPPeerAfSpec creates a shadow of BGPPeerAF
func NewBGPPeerAfSpec(in *pds.BGPPeerAfSpec) *ShadowBGPPeerAFSpec {
	uid, err := uuid.FromBytes(in.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}
	return &ShadowBGPPeerAFSpec{
		BGPPeerAfSpec: in,
		Id:            uidstr,
		LocalAddr:     PdsIPToString(in.LocalAddr),
		PeerAddr:      PdsIPToString(in.PeerAddr),
		Afi:           strings.TrimPrefix(in.Afi.String(), "BGP_AFI_"),
		Safi:          strings.TrimPrefix(in.Safi.String(), "BGP_SAFI_"),
	}
}

// ShadowBGPNLRIPrefixStatus is a shadow of the BGPNLRIPrefixStatus
type ShadowBGPNLRIPrefixStatus struct {
	Prefix      *NLRIPrefix
	ASPathStr   string
	PathOrigId  string
	NextHopAddr string
	*pds.BGPNLRIPrefixStatus
}

func NewBGPNLRIPrefixStatus(in *pds.BGPNLRIPrefixStatus) *ShadowBGPNLRIPrefixStatus {
	return &ShadowBGPNLRIPrefixStatus{
		ASPathStr:           fmt.Sprintf("%v", in.ASPathStr),
		PathOrigId:          net.IP(in.PathOrigId).String(),
		NextHopAddr:         net.IP(in.NextHopAddr).String(),
		Prefix:              NewNLRIPrefix(in.Prefix),
		BGPNLRIPrefixStatus: in,
	}
}

//
// ShadowEvpnIpVrf
//

// ShadowEvpnIpVrfSpec shadows the EvpnIpVrfSpec for CLI purposes
type ShadowEvpnIpVrfSpec struct {
	Id        string
	VPCId     string
	RD        string
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

//
// ShadowEvpnIpVrfRt
//

// ShadowEvpnIpVrfRtSpec shadows the EvpnIpVrfRtSpec for CLI purposes
type ShadowEvpnIpVrfRtSpec struct {
	Id        string
	VPCId     string
	RT        string
    RTType    string
	*pds.EvpnIpVrfRtSpec
}

func NewEvpnIpVrfRtSpec(in *pds.EvpnIpVrfRtSpec) ShadowEvpnIpVrfRtSpec {
	uid, err := uuid.FromBytes(in.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}
	return ShadowEvpnIpVrfRtSpec{
		Id:            uidstr,
		VPCId:         string(in.VPCId),
		RT:            dumpBytes(in.RT),
		RTType:        strings.TrimPrefix(in.RTType.String(), "EVPN_RT_"),
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
	Id         string
	SubnetId   string
	RD         string
    RTType     string
	*pds.EvpnEviSpec
}

func NewEvpnEviSpec(in *pds.EvpnEviSpec) ShadowEvpnEviSpec {
	uid, err := uuid.FromBytes(in.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}
	return ShadowEvpnEviSpec{
		Id:             uidstr,
		SubnetId:       string(in.SubnetId),
		RD:             printRD(in.RD),
		RTType:         strings.TrimPrefix(in.RTType.String(), "EVPN_RT_"),
		EvpnEviSpec:    in,
	}
}

// ShadowEvpnEviStatus shadows the EvpnEviStatus for CLI purposes
type ShadowEvpnEviStatus struct {
    RD         string
    Status     string
	*pds.EvpnEviStatus
}

func NewEvpnEviStatus(in *pds.EvpnEviStatus) ShadowEvpnEviStatus {
	return ShadowEvpnEviStatus{
		RD:               printRD(in.RD),
		Status:           strings.TrimPrefix(in.Status.String(), "EVPN_OPER_STATUS_"),
		EvpnEviStatus:    in,
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
	Id         string
	SubnetId   string
	RT         string
    RTType     string
	*pds.EvpnEviRtSpec
}

func NewEvpnEviRtSpec(in *pds.EvpnEviRtSpec) ShadowEvpnEviRtSpec {
	uid, err := uuid.FromBytes(in.Id)
	uidstr := ""
	if err == nil {
		uidstr = uid.String()
	}
	return ShadowEvpnEviRtSpec{
		Id:               uidstr,
		SubnetId:         string(in.SubnetId),
		RT:               dumpBytes(in.RT),
		RTType:           strings.TrimPrefix(in.RTType.String(), "EVPN_RT_"),
		EvpnEviRtSpec:    in,
	}
}

// ShadowEvpnEviRtStatus shadows the EvpnEviRtStatus for CLI purposes
type ShadowEvpnEviRtStatus struct {
	*pds.EvpnEviRtStatus
}

func NewEvpnEviRtStatus(in *pds.EvpnEviRtStatus) ShadowEvpnEviRtStatus {
	return ShadowEvpnEviRtStatus{
		EvpnEviRtStatus:    in,
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
