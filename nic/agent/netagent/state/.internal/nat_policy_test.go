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
func TestNatPolicyCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.0 - 192.168.1.1"},
					},
					NatPool: "testNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Error creating nat policy")
	natPool, err := ag.FindNatPolicy(natPolicy.ObjectMeta)
	AssertOk(t, err, "Nat Policy was not found in DB")
	Assert(t, natPool.Name == "testNatPolicy", "NatPolicy names did not match", natPool)

	// verify duplicate tenant creations succeed
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Error creating duplicate nat policy")

	// verify list api works.
	npList := ag.ListNatPolicy()
	Assert(t, len(npList) == 1, "Incorrect number of nat policies")

	// delete the nat policy and verify its gone from db
	err = ag.DeleteNatPolicy(natPolicy.Tenant, natPolicy.Namespace, natPolicy.Name)
	AssertOk(t, err, "Error deleting nat policy")
	_, err = ag.FindNatPolicy(np.ObjectMeta)
	Assert(t, err != nil, "Nat Pool was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteNatPolicy(natPolicy.Tenant, natPolicy.Namespace, natPolicy.Name)
	Assert(t, err != nil, "deleting non-existing nat policy succeeded", ag)
}

func TestNatPolicyUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing nat pool
	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")
	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.0 - 192.168.1.1"},
					},
					NatPool: "testNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Error creating nat policy")
	natPool, err := ag.FindNatPolicy(natPolicy.ObjectMeta)
	AssertOk(t, err, "Nat policy was not found in DB")
	Assert(t, natPool.Name == "testNatPolicy", "Nat Pool names did not match", natPool)

	npSpec := netproto.NatPolicySpec{
		Rules: []netproto.NatRule{
			{
				Src: &netproto.MatchSelector{
					Addresses: []string{"172.17.0.0 - 172.17.1.0"},
				},
				Dst: &netproto.MatchSelector{
					Addresses: []string{"192.168.0.0 - 192.168.1.1"},
				},
				NatPool: "testNatPool",
				Action:  "SNAT",
			},
		},
	}

	natPolicy.Spec = npSpec

	err = ag.UpdateNatPolicy(&natPolicy)
	AssertOk(t, err, "Error updating nat policy")

	updNatPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"172.168.0.0 - 172.168.1.0"},
					},
					NatPool: "testNatPool",
					Action:  "SNAT",
				},
			},
		},
	}
	err = ag.UpdateNatPolicy(&updNatPolicy)
	AssertOk(t, err, "Error updating nat policy")

}

//--------------------- Corner Case Tests ---------------------//

func TestNatPolicyCreateOnRemoteNatPool(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create the backing namespace and nat pool
	remoteNS := &netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "remoteNS",
		},
	}

	err := ag.CreateNamespace(remoteNS)
	AssertOk(t, err, "could not create remote namespace")

	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "remoteNS",
			Name:      "remoteNSNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err = ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.0 - 192.168.1.1"},
					},
					NatPool: "remoteNS/remoteNSNatPool",
					Action:  "DNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Error creating nat policy")
	natPool, err := ag.FindNatPolicy(natPolicy.ObjectMeta)
	AssertOk(t, err, "Nat Policy was not found in DB")
	Assert(t, natPool.Name == "testNatPolicy", "NatPolicy names did not match", natPool)
}

func TestNatPolicyOnNonExistentLocalNatPool(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.0 - 192.168.1.1"},
					},
					NatPool: "localNonExistentNatPool",
					Action:  "DNAT",
				},
			},
		},
	}

	// create nat policy
	err := ag.CreateNatPolicy(&natPolicy)
	Assert(t, err != nil, "Nat Policy create with a non existent local nat rule must fail validation. It passed instead")
}

func TestNatPolicyOnMatchAllDst(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create backing nat pool
	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					NatPool: "testNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Nat Policy create with missing Dst Selector should pass")
}

func TestNatPolicyOnMatchAllSrc(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create backing nat pool
	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.0 - 192.168.1.1"},
					},
					NatPool: "testNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Nat Policy create with missing Src Selector should pass")
}

func TestNatPolicyOnMatchAll(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create backing nat pool
	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					NatPool: "testNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	AssertOk(t, err, "Nat Policy create with missing Src Selector should pass")
}

func TestNatPolicyOnNonExistentRemoteNatPool(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create the backing namespace and nat pool
	remoteNS := &netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "remoteNS",
		},
	}

	err := ag.CreateNamespace(remoteNS)
	AssertOk(t, err, "could not create remote namespace")

	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.0 - 192.168.1.1"},
					},
					NatPool: "remoteNS/nonExistentRemoteNatPool",
					Action:  "SNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	Assert(t, err != nil, "Nat Policy creation on non existent remote nat pool should fail validation. It passed instead.")
}

func TestNatPolicyDatapathFailure(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")
	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - Bad IP Address"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.0 - 192.168.1.1"},
					},
					NatPool: "testNatPool",
					Action:  "DNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.CreateNatPolicy(&natPolicy)
	Assert(t, err != nil, "Nat policy with bad selector IP range formats should fail")
}

func TestNatPolicyUpdateOnNonExistentNatPolicy(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create the backing nat pool
	np := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPool",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.1.2.1-10.1.2.200",
		},
	}

	// create nat pool
	err := ag.CreateNatPool(&np)
	AssertOk(t, err, "Error creating nat pool")
	// nat policy
	natPolicy := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNatPolicy",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.0 - 192.168.1.1"},
					},
					NatPool: "testNatPool",
					Action:  "DNAT",
				},
			},
		},
	}

	// create nat policy
	err = ag.UpdateNatPolicy(&natPolicy)
	Assert(t, err != nil, "Nat policy updates on non existing nat policies fail")
}
