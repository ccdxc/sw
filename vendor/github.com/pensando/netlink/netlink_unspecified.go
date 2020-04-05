// +build !linux

package netlink

import "net"

// Proto is an enum representing an ipsec protocol.
type Proto uint8

// Dir is an enum representing an ipsec template direction.
type Dir uint8

// XfrmMark represents the mark associated to the state or policy
type XfrmMark struct {
	Value uint32
	Mask  uint32
}

// Mode is an enum representing an ipsec transport.
type Mode uint8

// XfrmPolicy represents an ipsec policy. It represents the overlay network
// and has a list of XfrmPolicyTmpls representing the base addresses of
// the policy.
type XfrmPolicy struct {
	Dst      *net.IPNet
	Src      *net.IPNet
	Proto    Proto
	DstPort  int
	SrcPort  int
	Dir      Dir
	Priority int
	Index    int
	Mark     *XfrmMark
	Tmpls    []XfrmPolicyTmpl
}

// XfrmPolicyTmpl encapsulates a rule for the base addresses of an ipsec
// policy. These rules are matched with XfrmState to determine encryption
// and authentication algorithms.
type XfrmPolicyTmpl struct {
	Dst   net.IP
	Src   net.IP
	Proto Proto
	Mode  Mode
	Spi   int
	Reqid int
}

func LinkSetUp(link Link) error {
	return ErrNotImplemented
}

func LinkSetDown(link Link) error {
	return ErrNotImplemented
}

func LinkSetMTU(link Link, mtu int) error {
	return ErrNotImplemented
}

func LinkSetMaster(link Link, master *Bridge) error {
	return ErrNotImplemented
}

func LinkSetNsPid(link Link, nspid int) error {
	return ErrNotImplemented
}

func LinkSetNsFd(link Link, fd int) error {
	return ErrNotImplemented
}

func LinkSetName(link Link, name string) error {
	return ErrNotImplemented
}

func LinkSetAlias(link Link, name string) error {
	return ErrNotImplemented
}

func LinkSetHardwareAddr(link Link, hwaddr net.HardwareAddr) error {
	return ErrNotImplemented
}

func LinkSetVfHardwareAddr(link Link, vf int, hwaddr net.HardwareAddr) error {
	return ErrNotImplemented
}

func LinkSetVfVlan(link Link, vf, vlan int) error {
	return ErrNotImplemented
}

func LinkSetVfTxRate(link Link, vf, rate int) error {
	return ErrNotImplemented
}

func LinkSetNoMaster(link Link) error {
	return ErrNotImplemented
}

func LinkSetMasterByIndex(link Link, masterIndex int) error {
	return ErrNotImplemented
}

func LinkSetXdpFd(link Link, fd int) error {
	return ErrNotImplemented
}

func LinkSetARPOff(link Link) error {
	return ErrNotImplemented
}

func LinkSetARPOn(link Link) error {
	return ErrNotImplemented
}

func LinkByName(name string) (Link, error) {
	return nil, ErrNotImplemented
}

func LinkByAlias(alias string) (Link, error) {
	return nil, ErrNotImplemented
}

func LinkByIndex(index int) (Link, error) {
	return nil, ErrNotImplemented
}

func LinkSetHairpin(link Link, mode bool) error {
	return ErrNotImplemented
}

func LinkSetGuard(link Link, mode bool) error {
	return ErrNotImplemented
}

func LinkSetFastLeave(link Link, mode bool) error {
	return ErrNotImplemented
}

func LinkSetLearning(link Link, mode bool) error {
	return ErrNotImplemented
}

func LinkSetRootBlock(link Link, mode bool) error {
	return ErrNotImplemented
}

func LinkSetFlood(link Link, mode bool) error {
	return ErrNotImplemented
}

func LinkSetTxQLen(link Link, qlen int) error {
	return ErrNotImplemented
}

func LinkAdd(link Link) error {
	return ErrNotImplemented
}

func LinkDel(link Link) error {
	return ErrNotImplemented
}

func SetHairpin(link Link, mode bool) error {
	return ErrNotImplemented
}

func SetGuard(link Link, mode bool) error {
	return ErrNotImplemented
}

func SetFastLeave(link Link, mode bool) error {
	return ErrNotImplemented
}

func SetLearning(link Link, mode bool) error {
	return ErrNotImplemented
}

func SetRootBlock(link Link, mode bool) error {
	return ErrNotImplemented
}

func SetFlood(link Link, mode bool) error {
	return ErrNotImplemented
}

func LinkList() ([]Link, error) {
	return nil, ErrNotImplemented
}

func AddrAdd(link Link, addr *Addr) error {
	return ErrNotImplemented
}

func AddrDel(link Link, addr *Addr) error {
	return ErrNotImplemented
}

func AddrList(link Link, family int) ([]Addr, error) {
	return nil, ErrNotImplemented
}

func RouteAdd(route *Route) error {
	return ErrNotImplemented
}

func RouteDel(route *Route) error {
	return ErrNotImplemented
}

func RouteList(link Link, family int) ([]Route, error) {
	return nil, ErrNotImplemented
}

func XfrmPolicyAdd(policy *XfrmPolicy) error {
	return ErrNotImplemented
}

func XfrmPolicyDel(policy *XfrmPolicy) error {
	return ErrNotImplemented
}

func XfrmPolicyList(family int) ([]XfrmPolicy, error) {
	return nil, ErrNotImplemented
}

func XfrmStateAdd(policy *XfrmState) error {
	return ErrNotImplemented
}

func XfrmStateDel(policy *XfrmState) error {
	return ErrNotImplemented
}

func XfrmStateList(family int) ([]XfrmState, error) {
	return nil, ErrNotImplemented
}

func NeighAdd(neigh *Neigh) error {
	return ErrNotImplemented
}

func NeighSet(neigh *Neigh) error {
	return ErrNotImplemented
}

func NeighAppend(neigh *Neigh) error {
	return ErrNotImplemented
}

func NeighDel(neigh *Neigh) error {
	return ErrNotImplemented
}

func NeighList(linkIndex, family int) ([]Neigh, error) {
	return nil, ErrNotImplemented
}

func NeighDeserialize(m []byte) (*Neigh, error) {
	return nil, ErrNotImplemented
}

func SocketGet(local, remote net.Addr) (*Socket, error) {
	return nil, ErrNotImplemented
}
