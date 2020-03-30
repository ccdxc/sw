// +build iris
// +build linux

package iris

import (
	"fmt"
	"net"
	"reflect"
	"strings"
	"testing"
	"time"

	"github.com/mdlayher/arp"
	"github.com/vishvananda/netlink"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var dummyInb *netlink.Dummy

const (
	destIPOutSideSubnet  = "42.42.42.42"
	destIPOutSideSubnet1 = "42.42.42.03"

	dMac     = "42.42:42:42:42:42"
	NodeUUID = "luke"
	//mgmtIntf = "bond0"
	//inbandIP = "10.10.10.1/24"
)

func getKey(meta api.ObjectMeta) string {
	return meta.Tenant + "/" + meta.Namespace + "/" + meta.Name
}

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

func TestMirrorSessionCreateVeniceKnownCollector(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

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
			NodeUUID:      NodeUUID,
			UsegVlan:      42,
			IPv4Addresses: []string{destIPOutSideSubnet},
		},
	}

	err = HandleL2Segment(infraAPI, l2SegClient, types.Create, *knownNet, 65, []uint64{120, 121, 122})
	AssertOk(t, err, "Network Create failed. Err: %v", err)
	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Create, *knownEP, 65, 42)
	AssertOk(t, err, "Endpoint Create failed. Err: %v", err)

	listNw, _ := infraAPI.List("Network")
	oldNwCount := len(listNw)
	listEp, _ := infraAPI.List("Endpoint")
	oldEpCount := len(listEp)
	listTun, _ := infraAPI.List("Tunnel")
	oldTunCount := len(listTun)

	ms := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet},
				},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)
	if err != nil {
		t.Fatal("Failed to create lateral objects in netagent")
	}
	tunnel := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "Tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
		},
	}

	// Ensure only tunnel
	//tunnel, err := ag.FindTunnel(lateralObjMeta)
	var obj netproto.Tunnel
	dat, err := infraAPI.Read(tunnel.Kind, tunnel.GetKey())
	if err != nil {
		t.Fatal(err)
	}
	err = obj.Unmarshal(dat)
	if err != nil {
		t.Fatal(err)
	}
	if !reflect.DeepEqual(knownEP.Spec.IPv4Addresses, []string{obj.Spec.Dst}) {
		t.Fatalf("Mismatch in ipv4 address %v and %v", knownEP.Spec.IPv4Addresses, []string{obj.Spec.Dst})
	}

	// Assert Nw and EP counts are the same before and after lateral obj creates
	listNw, _ = infraAPI.List("Network")
	newNwCount := len(listNw)
	listEp, _ = infraAPI.List("Endpoint")
	newEpCount := len(listEp)
	listTun, _ = infraAPI.List("Tunnel")
	newTunCount := len(listTun)

	if oldNwCount != newNwCount {
		t.Fatalf("Network count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	}
	if oldEpCount != newEpCount {
		t.Fatalf("Endpoint count must remain unchaged. Before: %v | After: %v", oldEpCount, newEpCount)
	}
	if oldTunCount+1 != newTunCount {
		t.Fatalf("Tunnel count must increase by 1. Before: %v | After: %v", oldTunCount, newTunCount)
	}

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	var objNetwork netproto.Network
	dat, err = infraAPI.Read(knownNet.Kind, knownNet.GetKey())
	if err != nil {
		t.Fatal(err)
	}
	err = objNetwork.Unmarshal(dat)
	AssertOk(t, err, "Venice created network must not be deleted")

	var objEndpoint netproto.Endpoint
	dat, err = infraAPI.Read(knownEP.Kind, knownEP.GetKey())
	if err != nil {
		t.Fatal(err)
	}
	err = objEndpoint.Unmarshal(dat)
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	var objTunnel netproto.Tunnel
	dat, err = infraAPI.Read(tunnel.Kind, tunnel.GetKey())
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", objTunnel)

	// Cleanup
	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Delete, *knownEP, 65, 42)
	AssertOk(t, err, "Endpoint Delete failed. Err: %v", err)
	err = HandleL2Segment(infraAPI, l2SegClient, types.Delete, *knownNet, 65, []uint64{120, 121, 122})
	AssertOk(t, err, "Network Delete failed. Err: %v", err)
}

func TestNetflowSessionCreateVeniceKnownCollector(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	fmt.Println("MGMT IP: ", mgmtIP)
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
			NodeUUID:      NodeUUID,
			UsegVlan:      42,
			IPv4Addresses: []string{destIPOutSideSubnet},
		},
	}

	err = HandleL2Segment(infraAPI, l2SegClient, types.Create, *knownNet, 65, []uint64{120, 121, 122})
	AssertOk(t, err, "Network Create failed. Err: %v", err)
	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Create, *knownEP, 65, 42)
	AssertOk(t, err, "Endpoint Create failed. Err: %v", err)

	listNw, _ := infraAPI.List("Network")
	oldNwCount := len(listNw)
	listEp, _ := infraAPI.List("Endpoint")
	oldEpCount := len(listEp)
	listTun, _ := infraAPI.List("Tunnel")
	oldTunCount := len(listTun)

	fePolicy := &netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy",
		},
		Spec: netproto.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []netproto.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	// Assert Nw and EP counts are the same before and after lateral obj creates
	listNw, _ = infraAPI.List("Network")
	newNwCount := len(listNw)
	listEp, _ = infraAPI.List("Endpoint")
	newEpCount := len(listEp)
	listTun, _ = infraAPI.List("Tunnel")
	newTunCount := len(listTun)

	Assert(t, oldNwCount == newNwCount, "Network count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldTunCount == newTunCount, "Tunnel count must increase by 1. Before: %v | After: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	var objNetwork netproto.Network
	dat, err := infraAPI.Read(knownNet.Kind, knownNet.GetKey())
	if err != nil {
		t.Fatal(err)
	}
	err = objNetwork.Unmarshal(dat)
	AssertOk(t, err, "Venice created network must not be deleted")

	var objEndpoint netproto.Endpoint
	dat, err = infraAPI.Read(knownEP.Kind, knownEP.GetKey())
	if err != nil {
		t.Fatal(err)
	}
	err = objEndpoint.Unmarshal(dat)
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	// Cleanup
	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Delete, *knownEP, 65, 42)
	AssertOk(t, err, "Endpoint Delete failed. Err: %v", err)
	err = HandleL2Segment(infraAPI, l2SegClient, types.Delete, *knownNet, 65, []uint64{120, 121, 122})
	AssertOk(t, err, "Network Delete failed. Err: %v", err)
}

func TestMirrorSessionCreateUnknownCollector(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	fmt.Println("MGMT IP: ", mgmtIP)
	ms := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet},
				},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	dmac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if !ok {
		Assert(t, ok, "Arp did not resolve ")
	}
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac),
	}
	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	var ep netproto.Endpoint
	dat, err := infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = ep.Unmarshal(dat)
	AssertOk(t, err, "Lateral endpoint obj not found")

	var tun netproto.Tunnel
	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = tun.Unmarshal(dat)
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Ensure TEP EP and Tunnel are associated.
	AssertEquals(t, []string{tun.Spec.Dst}, ep.Spec.IPv4Addresses, "Tunnel and EP objects did not correspond to each other.\nTun: %v\nEP: %v", tun, ep)

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the objects are really gone
	dat, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}

func TestNetflowSessionCreateUnknownCollector(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	fmt.Println("MGMT IP: ", mgmtIP)

	fePolicy := &netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy",
		},
		Spec: netproto.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{Protocol: "tcp",
								Port: "1000",
							},
						},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []netproto.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	dmac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if !ok {
		Assert(t, ok, "Arp Not resolved")
	}
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac),
	}
	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	var ep netproto.Endpoint
	dat, err := infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = ep.Unmarshal(dat)
	AssertOk(t, err, "Lateral endpoint obj not found")

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	dat, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}

func TestNetflowSessionAndMirrorSessionPointingToSameCollector(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	fmt.Println("MGMT IP: ", mgmtIP)

	fePolicy := &netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy",
		},
		Spec: netproto.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []netproto.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
			},
		},
	}

	ms := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet},
				},
			},
		},
	}

	// Create and validate fe policies
	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	dmac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if !ok {
		Assert(t, ok, "Arp Not resolved")
	}
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac),
	}

	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	dat, err := infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	AssertOk(t, err, "Lateral endpoint obj not found, when it is expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)

	// Create and validate mirror session
	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	// Ensure all lateral objects are created
	var ep netproto.Endpoint
	dat, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = ep.Unmarshal(dat)
	AssertOk(t, err, "Lateral endpoint obj not found")

	var tun netproto.Tunnel
	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = tun.Unmarshal(dat)
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Ensure TEP EP and Tunnel are associated.
	AssertEquals(t, []string{tun.Spec.Dst}, ep.Spec.IPv4Addresses, "Tunnel and EP objects did not correspond to each other.\nTun: %v\nEP: %v", tun, ep)

	//Call delete for mirror
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	AssertOk(t, err, "Shared collector must not be deleted when it has pending references")

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)

	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	dat, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}

func TestTwoMirrorSessionCreateVeniceKnownCollector(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	ms1 := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet},
				},
			},
		},
	}

	ms2 := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw_1",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
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
			NodeUUID:      NodeUUID,
			UsegVlan:      42,
			IPv4Addresses: []string{destIPOutSideSubnet},
		},
	}

	err = HandleL2Segment(infraAPI, l2SegClient, types.Create, *knownNet, 65, []uint64{120, 121, 122})
	AssertOk(t, err, "Network Create failed. Err: %v", err)
	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Create, *knownEP, 65, 42)
	AssertOk(t, err, "Endpoint Create failed. Err: %v", err)

	listNw, _ := infraAPI.List("Network")
	oldNwCount := len(listNw)
	listEp, _ := infraAPI.List("Endpoint")
	oldEpCount := len(listEp)
	listTun, _ := infraAPI.List("Tunnel")
	oldTunCount := len(listTun)

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms1.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms2.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure only tunnel
	var tunnel netproto.Tunnel
	dat, err := infraAPI.Read("Tunnel", getKey(lateralObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = tunnel.Unmarshal(dat)
	Assert(t, err == nil, "Venice known collectors must create a lateral tunnel. Tunnel : %v", tunnel)
	AssertEquals(t, []string{tunnel.Spec.Dst}, knownEP.Spec.IPv4Addresses, "Lateral Tunnel must point to the EP")

	// Assert Nw and EP counts are the same before and after lateral obj creates
	listNw, _ = infraAPI.List("Network")
	newNwCount := len(listNw)
	listEp, _ = infraAPI.List("Endpoint")
	newEpCount := len(listEp)
	listTun, _ = infraAPI.List("Tunnel")
	newTunCount := len(listTun)

	Assert(t, oldNwCount == newNwCount, "Network count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint count must remain unchaged. Before: %v | After: %v", oldEpCount, newEpCount)
	Assert(t, oldTunCount+1 == newTunCount, "Tunnel count must increase by 1. Before: %v | After: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms1.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	_, err = infraAPI.Read("Network", knownNet.GetKey())
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = infraAPI.Read("Endpoint", knownEP.GetKey())
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	_, err = infraAPI.Read("Tunnel", getKey(lateralObjMeta))
	AssertOk(t, err, "Tunnel has a mirror session referring to it. It should not be deleted")

	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms2.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	_, err = infraAPI.Read("Network", knownNet.GetKey())
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = infraAPI.Read("Endpoint", knownEP.GetKey())
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	dat, err = infraAPI.Read("Tunnel", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	// Cleanup
	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Delete, *knownEP, 65, 42)
	AssertOk(t, err, "Endpoint Delete failed. Err: %v", err)
	err = HandleL2Segment(infraAPI, l2SegClient, types.Delete, *knownNet, 65, []uint64{120, 121, 122})
	AssertOk(t, err, "Network Delete failed. Err: %v", err)
	GwCache = map[string]string{}
}

func TestTwoNetflowSessionCreateVeniceKnownCollector(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	fePolicy1 := &netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy1",
		},
		Spec: netproto.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []netproto.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
			},
		},
	}

	fePolicy2 := &netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy2",
		},
		Spec: netproto.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []netproto.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
			},
		},
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

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
			NodeUUID:      NodeUUID,
			UsegVlan:      42,
			IPv4Addresses: []string{destIPOutSideSubnet},
		},
	}

	err = HandleL2Segment(infraAPI, l2SegClient, types.Create, *knownNet, 65, []uint64{120, 121, 122})
	AssertOk(t, err, "Network Create failed. Err: %v", err)
	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Create, *knownEP, 65, 42)
	AssertOk(t, err, "Endpoint Create failed. Err: %v", err)

	listNw, _ := infraAPI.List("Network")
	oldNwCount := len(listNw)
	listEp, _ := infraAPI.List("Endpoint")
	oldEpCount := len(listEp)
	listTun, _ := infraAPI.List("Tunnel")
	oldTunCount := len(listTun)

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy1.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy2.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Assert Nw and EP counts are the same before and after lateral obj creates
	listNw, _ = infraAPI.List("Network")
	newNwCount := len(listNw)
	listEp, _ = infraAPI.List("Endpoint")
	newEpCount := len(listEp)
	listTun, _ = infraAPI.List("Tunnel")
	newTunCount := len(listTun)

	Assert(t, oldNwCount == newNwCount, "Network count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldTunCount == newTunCount, "Tunnel count must increase by 1. Before: %v | After: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy1.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	_, err = infraAPI.Read("Network", knownNet.GetKey())
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = infraAPI.Read("Endpoint", knownEP.GetKey())
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	dat, err := infraAPI.Read("Tunnel", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)

	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy2.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	_, err = infraAPI.Read("Network", knownNet.GetKey())
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = infraAPI.Read("Endpoint", knownEP.GetKey())
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	dat, err = infraAPI.Read("Tunnel", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	// Cleanup
	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Delete, *knownEP, 65, 42)
	AssertOk(t, err, "Endpoint Delete failed. Err: %v", err)
	err = HandleL2Segment(infraAPI, l2SegClient, types.Delete, *knownNet, 65, []uint64{120, 121, 122})
	AssertOk(t, err, "Network Delete failed. Err: %v", err)
	GwCache = map[string]string{}
}

func TestTwoMirrorSessionCreatesWithSameUnknownCollectorIP(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	ms1 := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	ms2 := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw_1",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms1.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms2.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	dmac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if !ok {
		Assert(t, ok, "Arp Not resolved")
	}

	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac),
	}

	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	var ep netproto.Endpoint
	dat, err := infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = ep.Unmarshal(dat)
	AssertOk(t, err, "Lateral endpoint obj not found")

	var tun netproto.Tunnel
	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = tun.Unmarshal(dat)
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Ensure TEP EP and Tunnel are associated.
	AssertEquals(t, []string{tun.Spec.Dst}, ep.Spec.IPv4Addresses, "Tunnel and EP objects did not correspond to each other.\nTun: %v\nEP: %v", tun, ep)

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms1.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	AssertOk(t, err, "Endpoints having pending references must not be deleted.")

	_, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	AssertOk(t, err, "Tunnels having pending references must not be deleted")

	// Trigger the delete of the second mirror session
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms2.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	dat, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}

func TestTwoNetflowSessionCreatesWithSameUnknownCollector(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	fmt.Println("MGMT IP: ", mgmtIP)
	fePolicy1 := &netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy1",
		},
		Spec: netproto.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []netproto.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
			},
		},
	}

	fePolicy2 := &netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy2",
		},
		Spec: netproto.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []netproto.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy1.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy2.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	dmac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if !ok {
		Assert(t, ok, "Arp Not resolved")
	}
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac),
	}

	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure lateral tunnel is not found
	_, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Tunnels must not be created for netflow collectors")
	// Ensure lateral EP is found
	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	AssertOk(t, err, "Lateral endpoint obj not found")

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy1.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure objects with pending references are not delted
	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	AssertOk(t, err, "EPs having pending references by flow export policies must not be deleted")

	// Delete the second flowexport policy
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy2.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	dat, err := infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}

func TestTwoNetflowSessionCreatesIdempotency(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	fmt.Println("MGMT IP: ", mgmtIP)
	fePolicy1 := &netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy1",
		},
		Spec: netproto.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []netproto.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
			},
		},
	}

	fePolicy2 := &netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy1",
		},
		Spec: netproto.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []netproto.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy1.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	listEp, _ := infraAPI.List("Endpoint")
	epCountAfter1stCreate := len(listEp)

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy2.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	listEp, _ = infraAPI.List("Endpoint")
	epCountAfter2ndCreate := len(listEp)

	AssertEquals(t, epCountAfter1stCreate, epCountAfter2ndCreate, "For idempotent netflow session creates, EP count must remain same")

	dmac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if !ok {
		Assert(t, ok, "Arp Not resolved")
	}
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac),
	}

	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure lateral tunnel is not found
	_, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Tunnels must not be created for netflow collectors")
	// Ensure lateral EP is found
	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	AssertOk(t, err, "Lateral endpoint obj not found")

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy1.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure objects with pending references are not delted
	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Idempotent deletes must delete the endpoint")

	// Delete the second flowexport policy
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy2.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	dat, err := infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}

func TestTwoMirrorSessionCreatesIdempotency(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	fmt.Println("MGMT IP: ", mgmtIP)
	ms1 := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	ms2 := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms1.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	listEp, _ := infraAPI.List("Endpoint")
	epCountAfter1stCreate := len(listEp)
	listTun, _ := infraAPI.List("Tunnel")
	tunnelCountAfter1stCreate := len(listTun)

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms2.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	listEp, _ = infraAPI.List("Endpoint")
	epCountAfter2ndCreate := len(listEp)
	listTun, _ = infraAPI.List("Tunnel")
	tunnelCountAfter2ndCreate := len(listTun)

	AssertEquals(t, epCountAfter1stCreate, epCountAfter2ndCreate, "For idempotent mirror session creates, EP count must remain same")
	AssertEquals(t, tunnelCountAfter1stCreate, tunnelCountAfter2ndCreate, "For idempotent mirror session creates, EP count must remain same")

	dmac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if !ok {
		Assert(t, ok, "Arp Not resolved")
	}
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac),
	}
	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure lateral tunnel is not found
	_, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Tunnels must not be created for netflow collectors")
	// Ensure lateral EP is found
	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	AssertOk(t, err, "Lateral endpoint obj not found")

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms1.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure objects with pending references are not delted
	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Idempotent deletes must delete the endpoint")

	// Delete the second flowexport policy
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms2.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	dat, err := infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}

func TestCreateDeleteLateralObjUnknownCollectorWithTunnel(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	fmt.Println("MGMT IP: ", mgmtIP)

	ms := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)

	dmac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if !ok {
		Assert(t, ok, "Arp Not resolved")
	}
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac),
	}

	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	var ep netproto.Endpoint
	dat, err := infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = ep.Unmarshal(dat)
	AssertOk(t, err, "Lateral endpoint obj not found")

	var tun netproto.Tunnel
	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = tun.Unmarshal(dat)
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Ensure TEP EP and Tunnel are associated.
	AssertEquals(t, []string{tun.Spec.Dst}, ep.Spec.IPv4Addresses, "Tunnel and EP objects did not correspond to each other.\nTun: %v\nEP: %v", tun, ep)

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the objects are really gone
	dat, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}

func TestCreateDeleteLateralObjUnknownCollectorWithoutTunnel(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	fmt.Println("MGMT IP: ", mgmtIP)
	fePolicy := &netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy",
		},
		Spec: netproto.FlowExportPolicySpec{
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "1000",
							},
						},
					},
				},
			},
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []netproto.ExportConfig{
				{
					Destination: destIPOutSideSubnet,
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2055",
					},
				},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy.GetKey(), mgmtIP, destIPOutSideSubnet, "", false)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	dmac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if !ok {
		Assert(t, ok, "Arp Not resolved")
	}
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac),
	}

	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	AssertOk(t, err, "Lateral endpoint obj not found")

	dat, err := infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)

	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, fePolicy.GetKey(), destIPOutSideSubnet, false)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the objects are really gone
	dat, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}

func TestCreateDeleteLateralObjVeniceKnownCollectorWithTunnel(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	ms := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
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
			NodeUUID:      NodeUUID,
			UsegVlan:      42,
			IPv4Addresses: []string{destIPOutSideSubnet},
		},
	}

	err = HandleL2Segment(infraAPI, l2SegClient, types.Create, *knownNet, 65, []uint64{120, 121, 122})
	AssertOk(t, err, "Network Create failed. Err: %v", err)
	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Create, *knownEP, 65, 42)
	AssertOk(t, err, "Endpoint Create failed. Err: %v", err)

	listNw, _ := infraAPI.List("Network")
	oldNwCount := len(listNw)
	listEp, _ := infraAPI.List("Endpoint")
	oldEpCount := len(listEp)
	listTun, _ := infraAPI.List("Tunnel")
	oldTunCount := len(listTun)

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure only tunnel and updated network is created
	_, err = infraAPI.Read("Network", getKey(lateralObjMeta))
	Assert(t, err != nil, "Venice known collectors must not create new networks")
	var foundNet netproto.Network
	dat, err := infraAPI.Read(knownNet.Kind, knownNet.GetKey())
	if err != nil {
		t.Fatal(err)
	}
	err = foundNet.Unmarshal(dat)
	AssertOk(t, err, "Venice created network must not be deleted")
	// Ensure that the updated Nw has not changed vlan ID
	Assert(t, knownNet.Spec.VlanID == foundNet.Spec.VlanID, "Venice known collectos must not update the nw vlan id. Actual: %v | Found: %v ", knownNet, foundNet)

	// Assert Nw and EP counts are the same before and after lateral obj creates
	listNw, _ = infraAPI.List("Network")
	newNwCount := len(listNw)
	listEp, _ = infraAPI.List("Endpoint")
	newEpCount := len(listEp)
	listTun, _ = infraAPI.List("Tunnel")
	newTunCount := len(listTun)

	Assert(t, oldNwCount == newNwCount, "Network count must remain unchaged. Before: %v | After: %v", oldNwCount, newNwCount)
	Assert(t, oldEpCount == newEpCount, "Endpoint count must remain unchaged. Before: %v | After: %v", oldEpCount, newEpCount)
	Assert(t, oldTunCount+1 == newTunCount, "Tunnel count must increase by 1. Before: %v | After: %v", oldTunCount, newTunCount)

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	// Ensure the venice created objects are not deleted
	_, err = infraAPI.Read("Network", knownNet.GetKey())
	AssertOk(t, err, "Venice created network must not be deleted")

	_, err = infraAPI.Read("Endpoint", knownEP.GetKey())
	AssertOk(t, err, "Venice created endpoint must not be deleted")

	dat, err = infraAPI.Read("Tunnel", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	// Cleanup
	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Delete, *knownEP, 65, 42)
	AssertOk(t, err, "Endpoint Delete failed. Err: %v", err)
	err = HandleL2Segment(infraAPI, l2SegClient, types.Delete, *knownNet, 65, []uint64{120, 121, 122})
	AssertOk(t, err, "Network Delete failed. Err: %v", err)
	GwCache = map[string]string{}
}

func TestFailedARPResolutionRetry(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}
	ms := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP("127.0.0.1")
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)
	AssertOk(t, err, "Creating lateral objects must succeed.")
	mac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if ok {
		Assert(t, mac == "", "Lateral object creates must fail on failed arp resolutions %v", mac)
	}
	// Make corrections so that ARP resolution goes through
	ArpClient.Close()

	mgmtIP, mgmtLink, mgmtIntf, err = findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err = arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	// Wait for next loop to run
	time.Sleep(time.Second * 70)
	dmac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if ok {
		Assert(t, dmac != "", "Arp Not resolved ")
	}
	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac),
	}

	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	// Ensure all lateral objects are created
	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	AssertOk(t, err, "Lateral endpoint obj not found")

	_, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	AssertOk(t, err, "Lateral endpoint obj not found")

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	dat, err := infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}

func TestFailedARPResolution(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}
	ms := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP("127.0.0.1")
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)
	AssertOk(t, err, "Creating lateral objects must succeed.")
	mac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if ok {
		Assert(t, mac == "", "Lateral object creates must fail on failed arp resolutions %v", mac)
	}
	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")
	GwCache = map[string]string{}
}

func TestTwoMirrorSessionCreatesWithSameUnknownCollectorMac(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	ms1 := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet}},
			},
		},
	}

	ms2 := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw_1",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet1}},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms1.GetKey(), mgmtIP, destIPOutSideSubnet, "", true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms2.GetKey(), mgmtIP, destIPOutSideSubnet1, "", true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	dmac, ok := destIPToMAC.Load(destIPOutSideSubnet)
	if !ok {
		Assert(t, ok, "Arp Not resolved")
	}

	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac),
	}

	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet),
	}

	dmac1, ok := destIPToMAC.Load(destIPOutSideSubnet1)
	if !ok {
		Assert(t, ok, "Arp Not resolved")
	}

	lateralObjMeta1 := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac1),
	}

	tunnelObjMeta1 := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet1),
	}

	// Ensure all lateral objects are created
	var ep netproto.Endpoint
	dat, err := infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = ep.Unmarshal(dat)
	AssertOk(t, err, "Lateral endpoint obj not found")
	AssertEquals(t, len(ep.Spec.IPv4Addresses), 2, "Number of Ips not equal")

	_, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	AssertOk(t, err, "Lateral tunnel obj not found")

	_, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta1))
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms1.GetKey(), destIPOutSideSubnet, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	AssertOk(t, err, "Endpoints having pending references must not be deleted.")

	_, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Tunnels should have been deleted")

	// Trigger the delete of the second mirror session
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms2.GetKey(), destIPOutSideSubnet1, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")
	time.Sleep(time.Second * 10)

	dat, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta1))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta1))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}

func TestTwoMirrorSessionCreatesWithSameIP(t *testing.T) {
	// Usage of netlink requires CAP_SYSADMIN. If this is not set, there is no point running the tests. Test for this here
	if shouldSkip() {
		t.Skip("Needs CAP_SYSADMIN set for netlink. Either unsupported platform or don't have enough privs to run.")
	}

	mgmtIP, mgmtLink, mgmtIntf, err := findMgmtIP(destIPOutSideSubnet1)
	AssertOk(t, err, "failed to find the mgmt IP.")

	client, err := arp.Dial(mgmtIntf)
	AssertOk(t, err, "Failed to initiate an ARP client")
	ArpClient = client
	MgmtLink = mgmtLink
	defer ArpClient.Close()

	ms1 := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet1}},
			},
		},
	}

	ms2 := &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw_1",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: destIPOutSideSubnet1}},
			},
		},
	}

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms1.GetKey(), mgmtIP, destIPOutSideSubnet1, "", true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")

	err = CreateLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms2.GetKey(), mgmtIP, destIPOutSideSubnet1, "", true)
	AssertOk(t, err, "Failed to create lateral objects in netagent")
	dmac1, ok := destIPToMAC.Load(destIPOutSideSubnet1)
	if !ok {
		Assert(t, ok, "Arp Not resolved")
	}

	lateralObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", dmac1),
	}

	tunnelObjMeta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      fmt.Sprintf("_internal-%s", destIPOutSideSubnet1),
	}

	// Ensure all lateral objects are created
	var ep netproto.Endpoint
	dat, err := infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	if err != nil {
		t.Fatal(err)
	}
	err = ep.Unmarshal(dat)
	AssertOk(t, err, "Lateral endpoint obj not found")
	AssertEquals(t, len(ep.Spec.IPv4Addresses), 1, "Number of Ips not equal")

	_, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	AssertOk(t, err, "Lateral tunnel obj not found")

	// Call delete objs
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms1.GetKey(), destIPOutSideSubnet1, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	_, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	AssertOk(t, err, "Endpoints having pending references must not be deleted.")

	_, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	AssertOk(t, err, "Tunnels should not have been deleted")

	// Trigger the delete of the second mirror session
	err = DeleteLateralNetAgentObjects(infraAPI, intfClient, epClient, 65, ms2.GetKey(), destIPOutSideSubnet1, true)
	AssertOk(t, err, "Deleting lateral objects must succeed.")

	dat, err = infraAPI.Read("Endpoint", getKey(lateralObjMeta))
	Assert(t, err != nil, "Lateral endpoint obj found, when it is not expected to be created. Found: %v", dat)

	dat, err = infraAPI.Read("Tunnel", getKey(tunnelObjMeta))
	Assert(t, err != nil, "Lateral tunnel obj found, when it is not expected to be created. Found: %v", dat)
	GwCache = map[string]string{}
}
