package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestSGPolicyCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
							{
								Port:     "443",
								Protocol: "tcp",
							},
							{
								Port:     "53",
								Protocol: "udp",
							},
						},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindSGPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testSGPolicy", "SGPolicy names did not match", sgp)

	// verify duplicate tenant creations succeed
	err = ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating duplicate sg policy")

	// verify list api works.
	sgpList := ag.ListSGPolicy()
	Assert(t, len(sgpList) == 1, "Incorrect number of sg policies")

	// delete the sg policy and verify its gone from db
	err = ag.DeleteSGPolicy(sgPolicy.Tenant, sgPolicy.Namespace, sgPolicy.Name)
	AssertOk(t, err, "Error deleting sg policy")
	_, err = ag.FindSGPolicy(sgPolicy.ObjectMeta)
	Assert(t, err != nil, "SG Policy was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteSGPolicy(sgPolicy.Tenant, sgPolicy.Namespace, sgPolicy.Name)
	Assert(t, err != nil, "deleting non-existing sg policy succeeded", ag)
}

func TestSGPolicyUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
							{
								Port:     "443",
								Protocol: "tcp",
							},
							{
								Port:     "53",
								Protocol: "udp",
							},
						},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindSGPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG policy was not found in DB")
	Assert(t, sgp.Name == "testSGPolicy", "Nat Pool names did not match", sgp)

	sgpSpec := netproto.SGPolicySpec{
		AttachGroup: []string{"preCreatedSecurityGroup"},
		Rules: []netproto.PolicyRule{
			{
				Action: "DENY",
			},
		},
	}

	sgPolicy.Spec = sgpSpec

	err = ag.UpdateSGPolicy(&sgPolicy)
	AssertOk(t, err, "Error updating sg policy")

	updSGPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachGroup:  []string{"preCreatedSecurityGroup"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
							{
								Port:     "443",
								Protocol: "tcp",
							},
							{
								Port:     "53",
								Protocol: "udp",
							},
						},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
				},
			},
		},
	}
	err = ag.UpdateSGPolicy(&updSGPolicy)
	AssertOk(t, err, "Error updating sg policy")

}

func TestSGPolicyMatchAllSrc(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Dst: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
							{
								Port:     "53",
								Protocol: "udp",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "sg policy creation matching on all source failed")
}

func TestSGPolicyMatchAllDst(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
							{
								Port:     "53",
								Protocol: "udp",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "sg policy matching on all dest failed")
}

func TestSGPolicyMatchAll(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "mordor",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "DENY",
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "sg policy matching on all failed")
}

//--------------------- Corner Case Tests ---------------------//

func TestSGPolicyOnMatchAllSrc(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "SG Policy creation with empty src should match all and pass validation")
}

func TestSGPolicyOnMatchAllDst(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
							{
								Port:     "443",
								Protocol: "tcp",
							},
							{
								Port:     "53",
								Protocol: "udp",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "SG Policy creation with empty dst should match all and pass validation")
}

func TestSGPolicyOnMatchAll(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "SG Policy creation with empty src and dst should match all and pass validation")
}

func TestSGPolicyUpdateOnNonExistentSGPolicy(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
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
	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachGroup:  []string{"preCreatedSecurityGroup"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
						},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
				},
			},
		},
	}

	// create sg policy
	err = ag.UpdateSGPolicy(&sgPolicy)
	Assert(t, err != nil, "Nat policy updates on non existing nat policies fail")
}

func TestSGPolicyOnNonExistentSG(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachGroup:  []string{"nonExistentSG"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	Assert(t, err != nil, "SG Policy creation with non existent security group attachment point should fail.")
}

func TestSGPolicyOnNonAttachmentPoints(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	Assert(t, err != nil, "SG Policy creation with non existent attachment points.")
}

func TestSGPolicyMatchAllPorts(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "Policies with empty port configs should not fail")
}

func TestSGPolicyBadPortRange(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Dst: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "foo",
								Protocol: "tcp",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	Assert(t, err != nil, "Policies with bad l4port config should fail")
}

func TestSGPolicyOutsidePortRange(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSGPolicy",
		},
		Spec: netproto.SGPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Dst: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "123456 - 123456",
								Protocol: "tcp",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateSGPolicy(&sgPolicy)
	Assert(t, err != nil, "Policies with ports > 64K should fail")
}
