package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
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
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
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
							{
								Port:     "0-40",
								Protocol: "tcp",
							},
							{
								Port:     "60000-65535",
								Protocol: "tcp",
							},
							{
								Port:     "0-65535",
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
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindSGPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testSGPolicy", "SGPolicy names did not match", sgp)

	err = ag.DeleteSGPolicy(sgPolicy.Tenant, sgPolicy.Namespace, sgPolicy.Name)
	AssertOk(t, err, "Error deleting sg policy")

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

	dns := &netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []string{"udp/53"},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
				},
			},
		},
	}
	err := ag.CreateApp(dns)
	AssertOk(t, err, "Failed to create dns app")

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
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					AppName: "dns",
				},
			},
		},
	}

	// create sg policy
	err = ag.CreateSGPolicy(&sgPolicy)
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
						Addresses: []string{"10.0.0.0 - 10.10.10.10"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					AppName: "dns",
				},
			},
		},
	}
	err = ag.UpdateSGPolicy(&updSGPolicy)
	AssertOk(t, err, "Error updating sg policy")

}

func TestSGPolicyALGMatchMSRPC(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	alg := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "sunrpc_tcp",
			Tenant:    "default",
			Namespace: "default",
		},
		Spec: netproto.AppSpec{
			ProtoPorts:     []string{"tcp/111"},
			ALGType:        "sunrpc",
			AppIdleTimeout: "100s",
			ALG: &netproto.ALG{
				SUNRPC: []*netproto.RPC{
					{
						ProgramID:        "10000",
						ProgramIDTimeout: "100s",
					},
					{
						ProgramID:        "10024",
						ProgramIDTimeout: "60s",
					},
					{
						ProgramID:        "10023",
						ProgramIDTimeout: "90s",
					},
				},
			},
		},
	}

	err := ag.CreateApp(&alg)
	AssertOk(t, err, "App creates must succeed")

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
						Addresses: []string{"any"}},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					AppName: "sunrpc_tcp",
				},
			},
		},
	}

	// create sg policy
	err = ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindSGPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testSGPolicy", "SGPolicy names did not match", sgp)

}

func TestSGPolicyALGMatchICMP(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	alg := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "icmp-echo-req",
			Tenant:    "default",
			Namespace: "default",
		},
		Spec: netproto.AppSpec{
			ALGType: "ICMP",
			ALG: &netproto.ALG{
				ICMP: &netproto.ICMP{
					Type: 8,
				},
			},
		},
	}

	err := ag.CreateApp(&alg)
	AssertOk(t, err, "App creates must succeed")

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
						Addresses: []string{"any"}},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					AppName: "icmp-echo-req",
				},
			},
		},
	}

	// create sg policy
	err = ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindSGPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testSGPolicy", "SGPolicy names did not match", sgp)

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

func TestSGPolicyICMPProtoMatch(t *testing.T) {
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
								Protocol: "icmp",
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

}

//--------------------- Corner Case Tests ---------------------//

func TestInvalidSGPolicyICMPPortMatch(t *testing.T) {
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
								Port:     "42",
								Protocol: "icmp",
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
	Assert(t, err != nil, "SGPolicy with ICMP protocol with a port must fail")
}

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

func TestInvalidSGPolicyWithAppAndProtoPort(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	alg := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "icmp-echo-req",
			Tenant:    "default",
			Namespace: "default",
		},
		Spec: netproto.AppSpec{
			ALGType: "ICMP",
			ALG: &netproto.ALG{
				ICMP: &netproto.ICMP{
					Type: 8,
				},
			},
		},
	}

	err := ag.CreateApp(&alg)
	AssertOk(t, err, "App creates must succeed")

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
						Addresses: []string{"any"},
						AppConfigs: []*netproto.AppConfig{
							{
								Protocol: "icmp",
							},
						}},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					AppName: "icmp-echo-req",
				},
			},
		},
	}

	// create sg policy
	err = ag.CreateSGPolicy(&sgPolicy)
	Assert(t, err != nil, "SGPolicy referring to both ALG and match criteria in the same rule must fail")
}

func TestConsistentRuleHashes(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

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
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
						},
					},
				},
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
				},
			},
		},
	}

	err := ag.CreateSGPolicy(&sgPolicy)
	AssertOk(t, err, "SgPolicy Create failed")
	actualSGPolicy, err := ag.FindSGPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy not found")

	rule1HashOnCreate := actualSGPolicy.Spec.Rules[0].ID
	rule2HashOnCreate := actualSGPolicy.Spec.Rules[1].ID

	// Update SG Policy, with updates going only to rule 2
	updSGPolicy := netproto.SGPolicy{
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
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
						},
					},
				},
				{
					Action: "DENY",
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
				},
			},
		},
	}

	err = ag.UpdateSGPolicy(&updSGPolicy)
	AssertOk(t, err, "SgPolicy Update failed")
	updatedSGPolicy, err := ag.FindSGPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy not found")

	rule1HashOnUpdate := updatedSGPolicy.Spec.Rules[0].ID
	rule2HashOnUpdate := updatedSGPolicy.Spec.Rules[1].ID

	// Ensure rule 1 hash on create and on update are the same
	Assert(t, rule1HashOnCreate == rule1HashOnUpdate, "Hashes got changed on a rule that was not updated")

	// Ensure rule 2 hash on create and on update are not the same
	Assert(t, rule2HashOnCreate != rule2HashOnUpdate, "Hashes did not change on a rule that was updated")
}
