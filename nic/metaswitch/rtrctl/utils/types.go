package utils

import (
	"encoding/binary"
	"fmt"
	"net"

	"github.com/satori/go.uuid"

	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
	mstypes "github.com/pensando/sw/nic/metaswitch/gen/agent"
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

func NewNLRIPrefix(in []byte) *NLRIPrefix {
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
	EthTagID   string
	MACAddress string
	IPAddress  string
	*EVPNType2Route
}

func (a *EVPNType2Route) parseBytes(in []byte) {
	if len(in) < type2MinLen {
		log.Errorf("invalid length [%d] for evpn type2", len(in))
	}
	cur := 0
	copy(a.RD, in[cur:cur+8])
	cur += 8
	copy(a.ESI, in[cur:cur+10])
	cur += 10
	copy(a.EthTagID, in[cur:cur+4])
	cur += 4
	a.MACAddrLen = int(in[cur])
	cur += 1
	copy(a.MACAddress, in[cur:cur+6])
	cur += 6
	a.IPAddressLen = int(in[cur])
	switch a.IPAddressLen {
	case 4:
		copy(a.IPAddress, in[cur:cur+4])
		cur += 4
	case 16:
		copy(a.IPAddress, in[cur:cur+16])
		cur += 16
	}
	copy(a.MPLSLabel1, in[cur:cur+3])
	cur += 3
	if len(in) > cur {
		copy(a.MPLSLabel2, in[cur:cur+3])
	}
}

func newEVPNType2Route(in *EVPNType2Route) *ShadowEVPNType2Route {
	return &ShadowEVPNType2Route{
		EVPNType2Route: in,
		RD:             fmt.Sprintf("%v", in.RD),
		ESI:            fmt.Sprintf("%v", in.ESI),
		EthTagID:       fmt.Sprintf("%v", in.EthTagID),
		MACAddress:     net.HardwareAddr(in.MACAddress).String(),
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
	EthTagID    string
	IPPrefix    string
	GWIPAddress string
	*EVPNType5Route
}

func (a *EVPNType5Route) parseBytes(in []byte) {
	if len(in) < type5MinLen {
		log.Errorf("invalid length [%d] for evpn type2", len(in))
	}
	cur := 0
	copy(a.RD, in[cur:cur+8])
	cur += 8
	copy(a.ESI, in[cur:cur+10])
	cur += 10
	copy(a.EthTagID, in[cur:cur+4])
	cur += 4
	a.IPPrefixLen = int(in[cur])
	if len(in) == type5MinLen {
		copy(a.IPPrefix, in[cur:cur+4])
		cur += 4
		copy(a.GWIPAddress, in[cur:cur+4])
		cur += 4
	} else {
		copy(a.IPPrefix, in[cur:cur+16])
		cur += 16
		copy(a.GWIPAddress, in[cur:cur+16])
		cur += 16
	}
	copy(a.MPLSLabel1, in[cur:cur+3])
}

func newEVPNType5Route(in *EVPNType5Route) *ShadowEVPNType5Route {
	return &ShadowEVPNType5Route{
		EVPNType5Route: in,
		RD:             fmt.Sprintf("%v", in.RD),
		ESI:            fmt.Sprintf("%v", in.ESI),
		EthTagID:       fmt.Sprintf("%v", in.EthTagID),
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
	*mstypes.BGPSpec
	Id       string
	RouterId string
}

// NewBGPSpec creates a new shadow of the BGPSpec
func NewBGPSpec(in *mstypes.BGPSpec) *ShadowBgpSpec {
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
	*mstypes.BGPPeerSpec
}

func newBGPPeerSpec(in *mstypes.BGPPeerSpec) ShadowBGPPeerSpec {
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
		BGPPeerSpec: in,
	}
}

// ShadowBGPPeerStatus shadows the BGPPeerStatus for CLI purposes
type ShadowBGPPeerStatus struct {
	Id            string
	LastErrorRcvd string
	LastErrorSent string
	LocalAddr     string
	PeerAddr      string
	Status        string
	PrevStatus    string
	*mstypes.BGPPeerStatus
}

func newBGPPeerStatus(in *mstypes.BGPPeerStatus) ShadowBGPPeerStatus {
	return ShadowBGPPeerStatus{
		Id:            "",
		LastErrorRcvd: string(in.LastErrorRcvd),
		LastErrorSent: string(in.LastErrorSent),
		LocalAddr:     PdsIPToString(in.LocalAddr),
		PeerAddr:      PdsIPToString(in.PeerAddr),
		Status:        in.Status.String(),
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
func NewBGPPeer(in *mstypes.BGPPeer) *ShadowBGPPeer {
	return &ShadowBGPPeer{
		Spec:   newBGPPeerSpec(in.Spec),
		Status: newBGPPeerStatus(in.Status),
	}
}

// ShadowBGPNLRIPrefixStatus is a shadow of the BGPNLRIPrefixStatus
type ShadowBGPNLRIPrefixStatus struct {
	Prefix      *NLRIPrefix
	ASPathStr   string
	PathOrigId  string
	NextHopAddr string
	*mstypes.BGPNLRIPrefixStatus
}

func NewBGPNLRIPrefixStatus(in *mstypes.BGPNLRIPrefixStatus) *ShadowBGPNLRIPrefixStatus {
	return &ShadowBGPNLRIPrefixStatus{
		ASPathStr:   string(in.ASPathStr),
		PathOrigId:  string(in.PathOrigId),
		NextHopAddr: string(in.NextHopAddr),
		Prefix:      NewNLRIPrefix(in.Prefix),
	}
}
