// +build linux

package state

import (
	"fmt"
	"net"
	"strings"
	"testing"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/tpmprotos"

	"github.com/pensando/sw/nic/agent/protos/tsproto"

	"github.com/pensando/sw/nic/agent/protos/netproto"

	"github.com/vishvananda/netlink"

	"github.com/pensando/sw/api"

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

func TestMirrorSessionCreateVeniceKnownCollector(t *testing.T) {
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
			Labels:    map[string]string{"CreatedBy": "Venice"},
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   "Network-VLAN-42",
			MacAddress:    "42:42:42:42:42:42",
			NodeUUID:      ag.NodeUUID,
			UsegVlan:      42,
			IPv4Addresses: []string{fmt.Sprintf("%s/32", destIPOutSideSubnet)},
		},
	}

	err = ag.CreateNetwork(knownNet)
	AssertOk(t, err, "Network Create failed. Err: %v", err)
	err = ag.CreateEndpoint(knownEP)
	AssertOk(t, err, "Endpoint Create failed. Err: %v", err)

	oldNwCount := len(ag.ListNetwork())
	oldEpCount := len(ag.ListEndpoint())
	oldTunCount := len(ag.ListTunnel())

	ms := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    256, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	err = ag.CreateLateralNetAgentObjects(ms.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure only tunnel
	tunnel, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err == nil, "Venice known collectors must create a lateral tunnel. Tunnel : %v", tunnel)
	AssertEquals(t, []string{fmt.Sprintf("%s/32", tunnel.Spec.Dst)}, knownEP.Spec.IPv4Addresses, "Lateral Tunnel must point to the EP")

	// Assert Nw and EP counts are the same before and after lateral obj creates
	newNwCount := len(ag.ListNetwork())
	newEpCount := len(ag.ListEndpoint())
	newTunCount := len(ag.ListTunnel())

	Assert(t, oldNwCount == newNwCount, "Network count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldTunCount+1 == newTunCount, "Tunnel count must increase by 1. Before: %v | After: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(ms.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	_, err = ag.FindNetwork(knownNet.ObjectMeta)
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = ag.FindEndpoint(knownEP.ObjectMeta)
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)

}

func TestNetflowSessionCreateVeniceKnownCollector(t *testing.T) {
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
			Labels:    map[string]string{"CreatedBy": "Venice"},
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   "Network-VLAN-42",
			MacAddress:    "42:42:42:42:42:42",
			NodeUUID:      ag.NodeUUID,
			UsegVlan:      42,
			IPv4Addresses: []string{fmt.Sprintf("%s/32", destIPOutSideSubnet)},
		},
	}

	err = ag.CreateNetwork(knownNet)
	AssertOk(t, err, "Network Create failed. Err: %v", err)
	err = ag.CreateEndpoint(knownEP)
	AssertOk(t, err, "Endpoint Create failed. Err: %v", err)

	oldNwCount := len(ag.ListNetwork())
	oldEpCount := len(ag.ListEndpoint())
	oldTunCount := len(ag.ListTunnel())

	fePolicy := &tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []tsproto.MatchRule{
				{
					Src: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.1"},
					},
					Dst: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.2"},
					},
					AppProtoSel: &tsproto.AppProtoSelector{
						Ports: []string{"TCP/1000"},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport:   "UDP/2055",
				},
			},
		},
	}

	err = ag.CreateLateralNetAgentObjects(fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	// Assert Nw and EP counts are the same before and after lateral obj creates
	newNwCount := len(ag.ListNetwork())
	newEpCount := len(ag.ListEndpoint())
	newTunCount := len(ag.ListTunnel())

	Assert(t, oldNwCount == newNwCount, "Network count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldTunCount == newTunCount, "Tunnel count must increase by 1. Before: %v | After: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	_, err = ag.FindNetwork(knownNet.ObjectMeta)
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = ag.FindEndpoint(knownEP.ObjectMeta)
	AssertOk(t, err, "Venice created endpoint must not be deleted")
}

func TestMirrorSessionCreateUnknownCollector(t *testing.T) {
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
	ms := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    256, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	err = ag.CreateLateralNetAgentObjects(ms.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	defaultNwMeta := api.ObjectMeta{
		Name:      "_internal_untagged_nw",
		Tenant:    "default",
		Namespace: "default",
	}

	// Ensure all lateral objects are created
	nw, err := ag.FindNetwork(defaultNwMeta)
	AssertOk(t, err, "Lateral network obj not found")
	Assert(t, nw.Spec.VlanID == types.UntaggedCollVLAN, "VLAN ID did not match")

	ep, err := ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	tun, err := ag.FindTunnel(lateralObjMeta)
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Ensure TEP EP and Tunnel are associated.
	AssertEquals(t, []string{fmt.Sprintf("%s/32", tun.Spec.Dst)}, ep.Spec.IPv4Addresses, "Tunnel and EP objects did not correspond to each other.\nTun: %v\nEP: %v", tun, ep)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(ms.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the objects are really gone
	foundNw, err := ag.FindNetwork(lateralObjMeta)
	Assert(t, err != nil, "Lateral network obj found, when it is not expected to be created. Found: %v", foundNw)

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)
}

func TestNetflowSessionCreateUnknownCollector(t *testing.T) {
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

	fePolicy := &tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []tsproto.MatchRule{
				{
					Src: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.1"},
					},
					Dst: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.2"},
					},
					AppProtoSel: &tsproto.AppProtoSelector{
						Ports: []string{"TCP/1000"},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport:   "UDP/2055",
				},
			},
		},
	}

	err = ag.CreateLateralNetAgentObjects(fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	defaultNwMeta := api.ObjectMeta{
		Name:      "_internal_untagged_nw",
		Tenant:    "default",
		Namespace: "default",
	}

	// Ensure all lateral objects are created
	nw, err := ag.FindNetwork(defaultNwMeta)
	AssertOk(t, err, "Lateral network obj not found")
	Assert(t, nw.Spec.VlanID == types.UntaggedCollVLAN, "VLAN ID did not match")

	_, err = ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	_, err = ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel must not be created")

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)
}

func TestNetflowSessionAndMirrorSessionPointingToSameCollector(t *testing.T) {
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

	fePolicy := &tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []tsproto.MatchRule{
				{
					Src: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.1"},
					},
					Dst: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.2"},
					},
					AppProtoSel: &tsproto.AppProtoSelector{
						Ports: []string{"TCP/1000"},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport:   "UDP/2055",
				},
			},
		},
	}

	ms := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    256, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	// Create and validate fe policies
	err = ag.CreateLateralNetAgentObjects(fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	defaultNwMeta := api.ObjectMeta{
		Name:      "_internal_untagged_nw",
		Tenant:    "default",
		Namespace: "default",
	}

	// Ensure all lateral objects are created
	nw, err := ag.FindNetwork(defaultNwMeta)
	AssertOk(t, err, "Lateral network obj not found")
	Assert(t, nw.Spec.VlanID == types.UntaggedCollVLAN, "VLAN ID did not match")

	_, err = ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	_, err = ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel must not be created")

	// Create and validate mirror session
	err = ag.CreateLateralNetAgentObjects(ms.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	// Ensure all lateral objects are created
	nw, err = ag.FindNetwork(defaultNwMeta)
	AssertOk(t, err, "Lateral network obj not found")
	Assert(t, nw.Spec.VlanID == types.UntaggedCollVLAN, "VLAN ID did not match")

	ep, err := ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	tun, err := ag.FindTunnel(lateralObjMeta)
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Ensure TEP EP and Tunnel are associated.
	AssertEquals(t, []string{fmt.Sprintf("%s/32", tun.Spec.Dst)}, ep.Spec.IPv4Addresses, "Tunnel and EP objects did not correspond to each other.\nTun: %v\nEP: %v", tun, ep)

	//Call delete for mirror
	err = ag.DeleteLateralNetAgentObjects(ms.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	_, err = ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Shared collector must not be deleted when it has pending references")

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)

	err = ag.DeleteLateralNetAgentObjects(fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)

	foundTun, err = ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)

}

func TestTwoMirrorSessionCreateVeniceKnownCollector(t *testing.T) {
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
	ms1 := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    256, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	ms2 := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw_1",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    128, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

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
			Labels:    map[string]string{"CreatedBy": "Venice"},
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   "Network-VLAN-42",
			MacAddress:    "42:42:42:42:42:42",
			NodeUUID:      ag.NodeUUID,
			UsegVlan:      42,
			IPv4Addresses: []string{fmt.Sprintf("%s/32", destIPOutSideSubnet)},
		},
	}

	err = ag.CreateNetwork(knownNet)
	AssertOk(t, err, "Network Create failed. Err: %v", err)
	err = ag.CreateEndpoint(knownEP)
	AssertOk(t, err, "Endpoint Create failed. Err: %v", err)

	oldNwCount := len(ag.ListNetwork())
	oldEpCount := len(ag.ListEndpoint())
	oldTunCount := len(ag.ListTunnel())

	err = ag.CreateLateralNetAgentObjects(ms1.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	err = ag.CreateLateralNetAgentObjects(ms2.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure only tunnel
	tunnel, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err == nil, "Venice known collectors must create a lateral tunnel. Tunnel : %v", tunnel)
	AssertEquals(t, []string{fmt.Sprintf("%s/32", tunnel.Spec.Dst)}, knownEP.Spec.IPv4Addresses, "Lateral Tunnel must point to the EP")

	// Assert Nw and EP counts are the same before and after lateral obj creates
	newNwCount := len(ag.ListNetwork())
	newEpCount := len(ag.ListEndpoint())
	newTunCount := len(ag.ListTunnel())

	Assert(t, oldNwCount == newNwCount, "Network count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint count must remain unchaged. Before: %v | After: %v", oldEpCount, newEpCount)
	Assert(t, oldTunCount+1 == newTunCount, "Tunnel count must increase by 1. Before: %v | After: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(ms1.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	_, err = ag.FindNetwork(knownNet.ObjectMeta)
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = ag.FindEndpoint(knownEP.ObjectMeta)
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	_, err = ag.FindTunnel(lateralObjMeta)
	AssertOk(t, err, "Tunnel has a mirror session referring to it. It should not be deleted")

	err = ag.DeleteLateralNetAgentObjects(ms2.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	_, err = ag.FindNetwork(knownNet.ObjectMeta)
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = ag.FindEndpoint(knownEP.ObjectMeta)
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)

}

func TestTwoNetflowSessionCreateVeniceKnownCollector(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	fePolicy1 := &tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy1",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []tsproto.MatchRule{
				{
					Src: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.1"},
					},
					Dst: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.2"},
					},
					AppProtoSel: &tsproto.AppProtoSelector{
						Ports: []string{"TCP/1000"},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport:   "UDP/2055",
				},
			},
		},
	}

	fePolicy2 := &tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy2",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []tsproto.MatchRule{
				{
					Src: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.1"},
					},
					Dst: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.2"},
					},
					AppProtoSel: &tsproto.AppProtoSelector{
						Ports: []string{"TCP/1000"},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport:   "UDP/2055",
				},
			},
		},
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
			Labels:    map[string]string{"CreatedBy": "Venice"},
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   "Network-VLAN-42",
			MacAddress:    "42:42:42:42:42:42",
			NodeUUID:      ag.NodeUUID,
			UsegVlan:      42,
			IPv4Addresses: []string{fmt.Sprintf("%s/32", destIPOutSideSubnet)},
		},
	}

	err = ag.CreateNetwork(knownNet)
	AssertOk(t, err, "Network Create failed. Err: %v", err)
	err = ag.CreateEndpoint(knownEP)
	AssertOk(t, err, "Endpoint Create failed. Err: %v", err)

	oldNwCount := len(ag.ListNetwork())
	oldEpCount := len(ag.ListEndpoint())
	oldTunCount := len(ag.ListTunnel())

	err = ag.CreateLateralNetAgentObjects(fePolicy1.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	err = ag.CreateLateralNetAgentObjects(fePolicy2.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Assert Nw and EP counts are the same before and after lateral obj creates
	newNwCount := len(ag.ListNetwork())
	newEpCount := len(ag.ListEndpoint())
	newTunCount := len(ag.ListTunnel())

	Assert(t, oldNwCount == newNwCount, "Network count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldTunCount == newTunCount, "Tunnel count must increase by 1. Before: %v | After: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(fePolicy1.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	_, err = ag.FindNetwork(knownNet.ObjectMeta)
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = ag.FindEndpoint(knownEP.ObjectMeta)
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)

}

func TestTwoMirrorSessionCreatesWithSameUnknownCollector(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	ms1 := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    256, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	ms2 := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw_1",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    128, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	err = ag.CreateLateralNetAgentObjects(ms1.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	err = ag.CreateLateralNetAgentObjects(ms2.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	ep, err := ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	tun, err := ag.FindTunnel(lateralObjMeta)
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Ensure TEP EP and Tunnel are associated.
	AssertEquals(t, []string{fmt.Sprintf("%s/32", tun.Spec.Dst)}, ep.Spec.IPv4Addresses, "Tunnel and EP objects did not correspond to each other.\nTun: %v\nEP: %v", tun, ep)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(ms1.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	_, err = ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Endpoints having pending references must not be deleted.")

	_, err = ag.FindTunnel(lateralObjMeta)
	AssertOk(t, err, "Tunnels having pending references must not be deleted")

	// Trigger the delete of the second mirror session
	err = ag.DeleteLateralNetAgentObjects(ms2.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)
}

func TestTwoNetflowSessionCreatesWithSameUnknownCollector(t *testing.T) {
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
	fePolicy1 := &tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy1",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []tsproto.MatchRule{
				{
					Src: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.1"},
					},
					Dst: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.2"},
					},
					AppProtoSel: &tsproto.AppProtoSelector{
						Ports: []string{"TCP/1000"},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport:   "UDP/2055",
				},
			},
		},
	}

	fePolicy2 := &tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy2",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []tsproto.MatchRule{
				{
					Src: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.1"},
					},
					Dst: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.2"},
					},
					AppProtoSel: &tsproto.AppProtoSelector{
						Ports: []string{"TCP/1000"},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport:   "UDP/2055",
				},
			},
		},
	}

	err = ag.CreateLateralNetAgentObjects(fePolicy1.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	err = ag.CreateLateralNetAgentObjects(fePolicy2.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure lateral tunnel is not found
	_, err = ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Tunnels must not be created for netflow collectors")
	// Ensure lateral EP is found
	_, err = ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(fePolicy1.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure objects with pending references are not delted
	_, err = ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "EPs having pending references by flow export policies must not be deleted")

	// Delete the second flowexport policy
	err = ag.DeleteLateralNetAgentObjects(fePolicy2.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)
}

func TestTwoNetflowSessionCreatesIdempotency(t *testing.T) {
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
	fePolicy1 := &tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy1",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []tsproto.MatchRule{
				{
					Src: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.1"},
					},
					Dst: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.2"},
					},
					AppProtoSel: &tsproto.AppProtoSelector{
						Ports: []string{"TCP/1000"},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport:   "UDP/2055",
				},
			},
		},
	}

	fePolicy2 := &tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy1",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []tsproto.MatchRule{
				{
					Src: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.1"},
					},
					Dst: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.2"},
					},
					AppProtoSel: &tsproto.AppProtoSelector{
						Ports: []string{"TCP/1000"},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport:   "UDP/2055",
				},
			},
		},
	}

	err = ag.CreateLateralNetAgentObjects(fePolicy1.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	epCountAfter1stCreate := len(ag.ListEndpoint())

	err = ag.CreateLateralNetAgentObjects(fePolicy2.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	epCountAfter2ndCreate := len(ag.ListEndpoint())

	AssertEquals(t, epCountAfter1stCreate, epCountAfter2ndCreate, "For idempotent netflow session creates, EP count must remain same")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure lateral tunnel is not found
	_, err = ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Tunnels must not be created for netflow collectors")
	// Ensure lateral EP is found
	_, err = ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(fePolicy1.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure objects with pending references are not delted
	_, err = ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Idempotent deletes must delete the endpoint")

	// Delete the second flowexport policy
	err = ag.DeleteLateralNetAgentObjects(fePolicy2.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)
}

func TestTwoMirrorSessionCreatesIdempotency(t *testing.T) {
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
	ms1 := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    256, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	ms2 := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    128, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	err = ag.CreateLateralNetAgentObjects(ms1.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	epCountAfter1stCreate := len(ag.ListEndpoint())
	tunnelCountAfter1stCreate := len(ag.ListTunnel())

	err = ag.CreateLateralNetAgentObjects(ms2.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	epCountAfter2ndCreate := len(ag.ListEndpoint())
	tunnelCountAfter2ndCreate := len(ag.ListTunnel())

	AssertEquals(t, epCountAfter1stCreate, epCountAfter2ndCreate, "For idempotent mirror session creates, EP count must remain same")
	AssertEquals(t, tunnelCountAfter1stCreate, tunnelCountAfter2ndCreate, "For idempotent mirror session creates, EP count must remain same")

	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure lateral tunnel is not found
	_, err = ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Tunnels must not be created for netflow collectors")
	// Ensure lateral EP is found
	_, err = ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(ms1.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure objects with pending references are not delted
	_, err = ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Idempotent deletes must delete the endpoint")

	// Delete the second flowexport policy
	err = ag.DeleteLateralNetAgentObjects(ms2.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)
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

	ms := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    256, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	err = ag.CreateLateralNetAgentObjects(ms.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	ep, err := ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	tun, err := ag.FindTunnel(lateralObjMeta)
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Ensure TEP EP and Tunnel are associated.
	AssertEquals(t, []string{fmt.Sprintf("%s/32", tun.Spec.Dst)}, ep.Spec.IPv4Addresses, "Tunnel and EP objects did not correspond to each other.\nTun: %v\nEP: %v", tun, ep)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(ms.GetKey(), mgmtIP, destIPOutSideSubnet, true)
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
	fePolicy := &tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			MatchRules: []tsproto.MatchRule{
				{
					Src: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.1"},
					},
					Dst: &tsproto.MatchSelector{
						IPAddresses: []string{"1.1.1.2"},
					},
					AppProtoSel: &tsproto.AppProtoSelector{
						Ports: []string{"TCP/1000"},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport:   "UDP/2055",
				},
			},
		},
	}

	err = ag.CreateLateralNetAgentObjects(fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	_, err = ag.FindEndpoint(lateralObjMeta)
	AssertOk(t, err, "Lateral endpoint obj not found")

	tun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", tun)

	err = ag.DeleteLateralNetAgentObjects(fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the objects are really gone
	foundNw, err := ag.FindNetwork(lateralObjMeta)
	Assert(t, err != nil, "Lateral network obj found, when it is not expected to be created. Found: %v", foundNw)

	foundEp, err := ag.FindEndpoint(lateralObjMeta)
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", foundEp)
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
	ms := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    256, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

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
			Labels:    map[string]string{"CreatedBy": "Venice"},
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   "Network-VLAN-42",
			MacAddress:    "42:42:42:42:42:42",
			NodeUUID:      ag.NodeUUID,
			UsegVlan:      42,
			IPv4Addresses: []string{fmt.Sprintf("%s/32", destIPOutSideSubnet)},
		},
	}

	err = ag.CreateNetwork(knownNet)
	AssertOk(t, err, "Network Create failed. Err: %v", err)
	err = ag.CreateEndpoint(knownEP)
	AssertOk(t, err, "Endpoint Create failed. Err: %v", err)

	oldNwCount := len(ag.ListNetwork())
	oldEpCount := len(ag.ListEndpoint())
	oldTunCount := len(ag.ListTunnel())

	err = ag.CreateLateralNetAgentObjects(ms.GetKey(), mgmtIP, destIPOutSideSubnet, true)
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

	// Assert Nw and EP counts are the same before and after lateral obj creates
	newNwCount := len(ag.ListNetwork())
	newEpCount := len(ag.ListEndpoint())
	newTunCount := len(ag.ListTunnel())

	Assert(t, oldNwCount == newNwCount, "Network count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint count must remain unchaged. Before: %v | After: %v", oldEpCount, newEpCount)
	Assert(t, oldTunCount+1 == newTunCount, "Tunnel count must increase by 1. Before: %v | After: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = ag.DeleteLateralNetAgentObjects(ms.GetKey(), mgmtIP, destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	_, err = ag.FindNetwork(knownNet.ObjectMeta)
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = ag.FindEndpoint(knownEP.ObjectMeta)
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	foundTun, err := ag.FindTunnel(lateralObjMeta)
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", foundTun)

}

func TestFailedARPResolution(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}
	ms := &tsproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: tsproto.MirrorSessionSpec{
			PacketSize:    256, //Modified
			Enable:        true,
			PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			CaptureAt:     1, // Modified
			Collectors: []tsproto.MirrorCollector{
				{Type: "erspan", ExportCfg: tsproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	mgmtIP, err := findMgmtIP(globals.Localhost)
	AssertOk(t, err, "failed to find the mgmt IP.")

	err = ag.CreateLateralNetAgentObjects(ms.GetKey(), mgmtIP, globals.Localhost, true)
	Assert(t, err != nil, "Lateral object creates must fail on failed arp resolutions")
}
