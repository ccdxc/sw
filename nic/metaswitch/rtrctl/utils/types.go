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
		ret.Prefix = &EVPNType2Route{}
		ret.Prefix.parseBytes(in[2:])
	case 5:
		ret.Prefix = &EVPNType5Route{}
		ret.Prefix.parseBytes(in[2:])
	}
	return ret
}

type EVPNType2Route struct {
	RD            []byte
	RDStr         string
	ESI           []byte
	ESIStr        string
	EthTagID      []byte
	EthTagIDstr   string
	MACAddrLen    int
	MACAddress    []byte
	MACAddressStr string
	IPAddressLen  int
	IPAddress     []byte
	IPAddressStr  string
	MPLSLabel1    []byte
	MPLSLabel2    []byte
}

type ShadowEVPNType2Route struct {
}

func (a *EVPNType2Route) parseBytes(in []byte) {
	if len(in) < type2MinLen {
		log.Errorf("invalid length [%d] for evpn type2", len(in))
	}
	cur := 0
	copy(a.RD, in[cur:cur+8])
	cur += 8
	a.RDStr = fmt.Sprintf("%v", a.RD)
	copy(a.ESI, in[cur:cur+10])
	cur += 10
	a.ESIStr = fmt.Sprintf("%v", a.ESI)
	copy(a.EthTagID, in[cur:cur+4])
	a.EthTagIDstr = fmt.Sprintf("%v", a.EthTagID)
	cur += 4
	a.MACAddrLen = int(in[cur])
	cur += 1
	copy(a.MACAddress, in[cur:cur+6])
	cur += 6
	mac := net.HardwareAddr(a.MACAddress)
	a.MACAddressStr = mac.String()
	a.IPAddressLen = int(in[cur])
	switch a.IPAddressLen {
	case 4:
		copy(a.IPAddress, in[cur:cur+4])
		ip := net.IP(a.IPAddress)
		a.IPAddressStr = ip.String()
		cur += 4
	case 16:
		copy(a.IPAddress, in[cur:cur+16])
		ip := net.IP(a.IPAddress)
		a.IPAddressStr = ip.String()
		cur += 16
	}
	copy(a.MPLSLabel1, in[cur:cur+3])
	cur += 3
	if len(in) > cur {
		copy(a.MPLSLabel2, in[cur:cur+3])
	}
}

type EVPNType5Route struct {
	RD             [8]byte
	RDStr          string
	ESI            [10]byte
	ESIStr         string
	EthTagID       [4]byte
	EthTagIDstr    string
	IPPrefixLen    int
	IPPrefix       []byte
	IPPrefixStr    string
	GWIPAddress    []byte
	GWIPAddressStr string
}

func (a *EVPNType5Route) parseBytes(in []byte) {

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
	Prefix      NLRIPrefix
	ASPathStr   string
	PathOrigId  string
	NextHopAddr string
	*mstypes.BGPNLRIPrefixStatus
}
