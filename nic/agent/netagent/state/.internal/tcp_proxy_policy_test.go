// +build ignore

package state

import (
	"testing"

	"github.com/pensando/sw/nic/agent/netagent/state"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestTCPProxyPolicyCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Decrypt rules
	tcpProxyPolicy := netproto.TCPProxyPolicy{
		TypeMeta: api.TypeMeta{Kind: "TCPProxyPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testTCPProxyPolicy",
		},
		Spec: netproto.TCPProxyPolicySpec{
			Rules: []netproto.TCPProxyRule{
				{
					Action: "ENABLE",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "172.17.0.0/24", "4.4.4.4"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0", "8.8.8.8"},
					},
				},
			},
		},
	}
	err := ag.CreateTCPProxyPolicy(&tcpProxyPolicy)
	AssertOk(t, err, "Error creating TCPProxy Policy")

	// create TCPProxy policy
	err = ag.CreateTCPProxyPolicy(&tcpProxyPolicy)
	AssertOk(t, err, "Error creating TCPProxy policy")
	foundTCPProxyPolicy, err := ag.FindTCPProxyPolicy(tcpProxyPolicy.ObjectMeta)
	AssertOk(t, err, "TCPProxy Policy was not found in DB")
	Assert(t, foundTCPProxyPolicy.Name == "testTCPProxyPolicy", "TCPProxyPolicy names did not match", foundTCPProxyPolicy)

	// verify duplicate tenant creations succeed
	err = ag.CreateTCPProxyPolicy(&tcpProxyPolicy)
	AssertOk(t, err, "Error creating duplicate TCPProxy policy")

	// verify list api works.
	npList := ag.ListTCPProxyPolicy()
	Assert(t, len(npList) == 1, "Incorrect number of TCPProxy policies")

	// delete the tcp proxy policy policy and verify its gone from db
	err = ag.DeleteTCPProxyPolicy(tcpProxyPolicy.Tenant, tcpProxyPolicy.Namespace, tcpProxyPolicy.Name)
	AssertOk(t, err, "Error deleting nat policy")
	_, err = ag.FindTCPProxyPolicy(tcpProxyPolicy.ObjectMeta)
	Assert(t, err != nil, "TCPProxy Pool was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteTCPProxyPolicy(tcpProxyPolicy.Tenant, tcpProxyPolicy.Namespace, tcpProxyPolicy.Name)
	Assert(t, err != nil, "deleting non-existing nat policy succeeded", ag)
}

func TestTCPProxyPolicyUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	tcpProxyPolicy := netproto.TCPProxyPolicy{
		TypeMeta: api.TypeMeta{Kind: "TCPProxyPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testTCPProxyPolicy",
		},
		Spec: netproto.TCPProxyPolicySpec{
			Rules: []netproto.TCPProxyRule{
				{
					Action: "ENABLE",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "172.17.0.0/24", "4.4.4.4"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0", "8.8.8.8"},
					},
				},
			},
		},
	}
	err := ag.CreateTCPProxyPolicy(&tcpProxyPolicy)
	AssertOk(t, err, "Error creating TCPProxy SA Encrypt rule")

	tcpProxyPolicyUpdate := netproto.TCPProxyPolicy{
		TypeMeta: api.TypeMeta{Kind: "TCPProxyPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testTCPProxyPolicy",
		},
		Spec: netproto.TCPProxyPolicySpec{
			Rules: []netproto.TCPProxyRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"20.0.0.0 -  20.1.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					Action: "DISABLE",
				},
			},
		},
	}
	err = ag.CreateTCPProxyPolicy(&tcpProxyPolicy)
	AssertOk(t, err, "TCPProxy Policy creation failed")

	err = ag.UpdateTCPProxyPolicy(&tcpProxyPolicyUpdate)
	AssertOk(t, err, "TCPProxy Policy update failed")

}

//--------------------- Corner Case Tests ---------------------//

func TestTCPProxyPolicyDatapathFailure(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Decrypt rules
	tcpProxyPolicy := netproto.TCPProxyPolicy{
		TypeMeta: api.TypeMeta{Kind: "TCPProxyPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testTCPProxyPolicy",
		},
		Spec: netproto.TCPProxyPolicySpec{
			Rules: []netproto.TCPProxyRule{
				{
					Action: "ENABLE",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "172.17.0.0/24", "4.4.4.4"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0", "8.8.8.8"},
					},
				},
			},
		},
	}
	err := ag.CreateTCPProxyPolicy(&tcpProxyPolicy)
	AssertOk(t, err, "Error creating TCPProxy Policy")

	tcpProxyPolicy = netproto.TCPProxyPolicy{
		TypeMeta: api.TypeMeta{Kind: "TCPProxyPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "badTCPProxyPolicy",
		},
		Spec: netproto.TCPProxyPolicySpec{
			Rules: []netproto.TCPProxyRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - Bad IP"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					Action: "ENABLE",
				},
			},
		},
	}
	err = ag.CreateTCPProxyPolicy(&tcpProxyPolicy)
	Assert(t, err != nil, "TCPProxy Policy with invalid address range should fail")

}
