// +build apulu

package apulu

import (
	"testing"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleIPAMPolicy(t *testing.T) {
	// Create vrfuid
	uid := uuid.NewV4().String()
	vrfuid, err := uuid.FromString(uid)
	if err != nil {
		t.Fatal(err)
	}

	ipam := netproto.IPAMPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPAMPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPAM",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.IPAMPolicySpec{
			DHCPRelay: &netproto.DHCPRelayPolicy{
				Servers: []*netproto.DHCPServer{
					{
						IPAddress: "192.168.100.101",
					},
				},
			},
		},
	}

	ipam1 := netproto.IPAMPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPAMPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPAM1",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.IPAMPolicySpec{
			DHCPRelay: &netproto.DHCPRelayPolicy{
				Servers: []*netproto.DHCPServer{
					{
						IPAddress: "192.168.100.101",
					},
					{
						IPAddress: "192.168.100.102",
					},
				},
			},
		},
	}

	if err := HandleIPAMPolicy(infraAPI, ipamClient, subnetClient, types.Create, ipam, vrfuid.Bytes()); err != nil {
		t.Fatal(err)
	}
	if err := HandleIPAMPolicy(infraAPI, ipamClient, subnetClient, types.Create, ipam1, vrfuid.Bytes()); err != nil {
		t.Fatal(err)
	}

	if _, ok := DHCPServerIPToUUID["192.168.100.101"]; !ok {
		t.Fatalf("Expected 192.168.100.101 to be in the map. %v", DHCPServerIPToUUID)
	}
	if _, ok := DHCPServerIPToUUID["192.168.100.102"]; !ok {
		t.Fatalf("Expected 192.168.100.102 to be in the map. %v", DHCPServerIPToUUID)
	}

	if len(IPAMPolicyIDToServerIDs[ipam.UUID]) != 1 {
		t.Fatalf("Expected 1 entry in the map for %s. %v", ipam.UUID, IPAMPolicyIDToServerIDs)
	}
	if len(IPAMPolicyIDToServerIDs[ipam1.UUID]) != 2 {
		t.Fatalf("Expected 2 entries in the map for %s. %v", ipam1.UUID, IPAMPolicyIDToServerIDs)
	}

	ipam.Spec.DHCPRelay.Servers[0].IPAddress = "192.168.100.103"
	if err := HandleIPAMPolicy(infraAPI, ipamClient, subnetClient, types.Update, ipam, vrfuid.Bytes()); err != nil {
		t.Fatal(err)
	}
	if _, ok := DHCPServerIPToUUID["192.168.100.101"]; !ok {
		t.Fatalf("Expected 192.168.100.101 to be in the map. %v", DHCPServerIPToUUID)
	}
	if _, ok := DHCPServerIPToUUID["192.168.100.103"]; !ok {
		t.Fatalf("Expected 192.168.100.103 to be in the map. %v", DHCPServerIPToUUID)
	}

	ipam1.Spec.DHCPRelay.Servers[0].IPAddress = "192.168.100.103"
	if err := HandleIPAMPolicy(infraAPI, ipamClient, subnetClient, types.Update, ipam1, vrfuid.Bytes()); err != nil {
		t.Fatal(err)
	}
	if _, ok := DHCPServerIPToUUID["192.168.100.101"]; ok {
		t.Fatalf("Expected 192.168.100.101 to not be in the map. %v", DHCPServerIPToUUID)
	}

	ipam1.Spec.DHCPRelay.Servers[0].IPAddress = "192.168.100.102"
	ipam1.Spec.DHCPRelay.Servers[1].IPAddress = "192.168.100.104"
	if err := HandleIPAMPolicy(infraAPI, ipamClient, subnetClient, types.Update, ipam1, vrfuid.Bytes()); err != nil {
		t.Fatal(err)
	}
	if _, ok := DHCPServerIPToUUID["192.168.100.103"]; !ok {
		t.Fatalf("Expected 192.168.100.103 to be in the map. %v", DHCPServerIPToUUID)
	}
	if _, ok := DHCPServerIPToUUID["192.168.100.102"]; !ok {
		t.Fatalf("Expected 192.168.100.102 to be in the map. %v", DHCPServerIPToUUID)
	}
	if _, ok := DHCPServerIPToUUID["192.168.100.104"]; !ok {
		t.Fatalf("Expected 192.168.100.104 to be in the map. %v", DHCPServerIPToUUID)
	}

	if err := HandleIPAMPolicy(infraAPI, ipamClient, subnetClient, types.Delete, ipam, vrfuid.Bytes()); err != nil {
		t.Fatal(err)
	}
	if _, ok := DHCPServerIPToUUID["192.168.100.103"]; ok {
		t.Fatalf("Expected 192.168.100.103 to not be in the map. %v", DHCPServerIPToUUID)
	}

	if err := HandleIPAMPolicy(infraAPI, ipamClient, subnetClient, types.Delete, ipam1, vrfuid.Bytes()); err != nil {
		t.Fatal(err)
	}
	if len(DHCPServerIPToUUID) != 0 {
		t.Fatalf("Expected DHCPServerIPToUUID to be empty %v", DHCPServerIPToUUID)
	}
	if len(IPAMPolicyIDToServerIDs) != 0 {
		t.Fatalf("Expected IPAMPolicyIDToServerIDs to be empty %v", IPAMPolicyIDToServerIDs)
	}
}
