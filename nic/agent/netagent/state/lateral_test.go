// +build linux

package state

import (
	"fmt"
	"net"
	"strings"
	"testing"

	"github.com/pensando/sw/nic/agent/protos/netproto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"

	"github.com/vishvananda/netlink"

	. "github.com/pensando/sw/venice/utils/testutils"
)

var dummyInb *netlink.Dummy

const (
	destIPOutSideSubnet = "42.42.42.42"
	//mgmtIntf = "bond0"
	//inbandIP = "10.10.10.1/24"
)

func shouldSkip() bool {
	mac, _ := net.ParseMAC("42:42:42:42:42:42")

	dummyInb = &netlink.Dummy{
		LinkAttrs: netlink.LinkAttrs{
			Name:         "dummy",
			TxQLen:       1000,
			HardwareAddr: mac,
		},
	}

	err := netlink.LinkAdd(dummyInb)
	if err != nil && (err == netlink.ErrNotImplemented || strings.Contains(err.Error(), "operation not permitted")) {
		return true
	}

	return false

}

func findMgmtIP(destIP string) (mgmtIP string, err error) {
	rts, err := netlink.RouteGet(net.ParseIP(destIP))
	if err != nil {
		return
	}
	// Associate mgmt intf to the default route
	mgmtInft, err := netlink.LinkByIndex(rts[0].LinkIndex)
	addrs, err := netlink.AddrList(mgmtInft, netlink.FAMILY_V4)
	if err != nil {
		return
	}
	mgmtIP = addrs[0].IP.String()
	return
}

func TestCreateDeleteLateralObjUnknownCollectorWithTunnel(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	fmt.Println("MGMT IP: ", mgmtIP)

	err = ag.CreateLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	nw, err := ag.FindNetwork(lateralObjMeta)
	AssertOk(t, err, "Lateral network obj not found")
	Assert(t, nw.Spec.VlanID == types.UntaggedVLAN, "VLAN ID did not match")
	Assert(t, len(nw.Spec.RouterMAC) != 0, "Nw creates must be done with a RMAC. Found: %v", nw)

	ep, err := ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	tun, err := ag.FindTunnel(lateralObjMeta)
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Ensure TEP EP and Tunnel are associated.
	Assert(t, ep.Spec.IPv4Address == fmt.Sprintf("%s/32", tun.Spec.Dst), "Tunnel and EP objects did not correspond to each other.\nTun: %v\nEP: %v", tun, ep)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the objects are really gone
	foundNw, err := ag.FindNetwork(lateralObjMeta)
	Assert(t, err != nil, "Lateral network obj found, when it is not expected to be created. Found: %v", foundNw)

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)

}

func TestCreateDeleteLateralObjUnknownCollectorWithoutTunnel(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	fmt.Println("MGMT IP: ", mgmtIP)

	err = ag.CreateLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	nw, err := ag.FindNetwork(lateralObjMeta)
	AssertOk(t, err, "Lateral network obj not found")
	Assert(t, nw.Spec.VlanID == types.UntaggedVLAN, "VLAN ID did not match")
	Assert(t, len(nw.Spec.RouterMAC) != 0, "Nw creates must be done with a RMAC. Found: %v", nw)

	_, err = ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	tun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", tun)

	err = ag.DeleteLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the objects are really gone
	foundNw, err := ag.FindNetwork(lateralObjMeta)
	Assert(t, err != nil, "Lateral network obj found, when it is not expected to be created. Found: %v", foundNw)

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)
}

func TestCreateDeleteLateralObjKnownCollectorWithTunnel(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	fmt.Println("MGMT IP: ", mgmtIP)

	err = ag.CreateLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	nw, err := ag.FindNetwork(lateralObjMeta)
	AssertOk(t, err, "Lateral network obj not found")
	Assert(t, nw.Spec.VlanID == types.UntaggedVLAN, "VLAN ID did not match")
	Assert(t, len(nw.Spec.RouterMAC) != 0, "Nw creates must be done with a RMAC. Found: %v", nw)

	ep, err := ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	tun, err := ag.FindTunnel(lateralObjMeta)
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Ensure TEP EP and Tunnel are associated.
	Assert(t, ep.Spec.IPv4Address == fmt.Sprintf("%s/32", tun.Spec.Dst), "Tunnel and EP objects did not correspond to each other.\nTun: %v\nEP: %v", tun, ep)
	oldNwCount := len(ag.ListNetwork())
	oldEpCount := len(ag.ListEndpoint())
	oldTunCount := len(ag.ListTunnel())

	err = ag.CreateLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent the second time")

	newNwCount := len(ag.ListNetwork())
	newEpCount := len(ag.ListEndpoint())
	newTunCount := len(ag.ListTunnel())

	Assert(t, oldNwCount == newNwCount, "Network counts must match after two calls to lateral object creates. Old: %v | New: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint counts must match after two calls to lateral object creates. Old: %v | New: %v", oldEpCount, newEpCount)
	Assert(t, oldTunCount == newTunCount, "Tunnel counts must match after two calls to lateral object creates. Old: %v | New: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the objects are really gone
	foundNw, err := ag.FindNetwork(lateralObjMeta)
	Assert(t, err != nil, "Lateral network obj found, when it is not expected to be created. Found: %v", foundNw)

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)

}

func TestCreateDeleteLateralObjKnownCollectorWithoutTunnel(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	fmt.Println("MGMT IP: ", mgmtIP)

	err = ag.CreateLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	nw, err := ag.FindNetwork(lateralObjMeta)
	AssertOk(t, err, "Lateral network obj not found")
	Assert(t, nw.Spec.VlanID == types.UntaggedVLAN, "VLAN ID did not match")
	Assert(t, len(nw.Spec.RouterMAC) != 0, "Nw creates must be done with a RMAC. Found: %v", nw)

	_, err = ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	_, err = ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it should not have been created")

	oldNwCount := len(ag.ListNetwork())
	oldEpCount := len(ag.ListEndpoint())
	oldTunCount := len(ag.ListTunnel())

	err = ag.CreateLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent the second time")

	newNwCount := len(ag.ListNetwork())
	newEpCount := len(ag.ListEndpoint())
	newTunCount := len(ag.ListTunnel())

	Assert(t, oldNwCount == newNwCount, "Network counts must match after two calls to lateral object creates. Old: %v | New: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint counts must match after two calls to lateral object creates. Old: %v | New: %v", oldEpCount, newEpCount)
	Assert(t, oldTunCount == newTunCount, "Tunnel counts must match after two calls to lateral object creates. Old: %v | New: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the objects are really gone
	foundNw, err := ag.FindNetwork(lateralObjMeta)
	Assert(t, err != nil, "Lateral network obj found, when it is not expected to be created. Found: %v", foundNw)

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)
}

func TestCreateDeleteLateralObjVeniceKnownCollectorWithTunnel(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, err := findMgmtIP(destIPOutSideSubnet)
	fmt.Println("MGMT IP: ", mgmtIP)
	AssertOk(t, err, "failed to find the mgmt IP.")

	// Simulate a venice known collector EP by creating corresponding nw and ep obj
	knownNet := &netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "Network-VLAN-42",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 42,
		},
	}
	knownEP := &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "CollectorEP",
		},
		Spec: netproto.EndpointSpec{
			NetworkName: "Network-VLAN-42",
			MacAddress:  "42:42:42:42:42:42",
			NodeUUID:    ag.NodeUUID,
			UsegVlan:    42,
			IPv4Address: fmt.Sprintf("%s/32", destIPOutSideSubnet),
		},
	}

	err = ag.CreateNetwork(knownNet)
	AssertOk(t, err, "Network Create failed. Err: %v", err)
	_, err = ag.CreateEndpoint(knownEP)
	AssertOk(t, err, "Endpoint Create failed. Err: %v", err)

	oldNwCount := len(ag.ListNetwork())
	oldEpCount := len(ag.ListEndpoint())
	oldTunCount := len(ag.ListTunnel())

	err = ag.CreateLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure only tunnel and updated network is created
	_, err = ag.FindNetwork(lateralObjMeta)
	Assert(t, err != nil, "Venice known collectors must not create new networks")
	foundNet, err := ag.FindNetwork(knownNet.ObjectMeta)
	AssertOk(t, err, "Venice known network lookup failed. Existing NW: %v", ag.ListNetwork())
	// Ensure that the updated Nw has not changed vlan ID
	Assert(t, knownNet.Spec.VlanID == foundNet.Spec.VlanID, "Venice known collectos must not update the nw vlan id. Actual: %v | Found: %v ", knownNet, foundNet)
	// Ensure that the updated nw has a router mac set
	Assert(t, len(foundNet.Spec.RouterMAC) != 0, "Venice known collector, must trigger a network update with populated router mac. Found: %v", foundNet)

	// Assert Nw and EP counts are the same before and after lateral obj creates
	newNwCount := len(ag.ListNetwork())
	newEpCount := len(ag.ListEndpoint())
	newTunCount := len(ag.ListTunnel())

	Assert(t, oldNwCount == newNwCount, "Network count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldTunCount+1 == newTunCount, "Tunnel count must increase by 1. Before: %v | After: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	_, err = ag.FindNetwork(knownNet.ObjectMeta)
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = ag.FindEndpoint(knownEP.ObjectMeta)
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)

}
