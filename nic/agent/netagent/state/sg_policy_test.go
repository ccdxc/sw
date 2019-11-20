package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestNetworkSecurityPolicyCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testNetworkSecurityPolicy", "NetworkSecurityPolicy names did not match", sgp)

	err = ag.DeleteNetworkSecurityPolicy(sgPolicy.Tenant, sgPolicy.Namespace, sgPolicy.Name)
	AssertOk(t, err, "Error deleting sg policy")

	// verify duplicate tenant creations succeed
	err = ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating duplicate sg policy")

	// verify list api works.
	sgpList := ag.ListNetworkSecurityPolicy()
	Assert(t, len(sgpList) == 1, "Incorrect number of sg policies")

	// delete the sg policy and verify its gone from db
	err = ag.DeleteNetworkSecurityPolicy(sgPolicy.Tenant, sgPolicy.Namespace, sgPolicy.Name)
	AssertOk(t, err, "Error deleting sg policy")
	_, err = ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	Assert(t, err != nil, "SG Policy was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteNetworkSecurityPolicy(sgPolicy.Tenant, sgPolicy.Namespace, sgPolicy.Name)
	Assert(t, err != nil, "deleting non-existing sg policy succeeded", ag)
}

func TestNetworkSecurityPolicyCreatePortas0(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
								Port:     "0",
								Protocol: "tcp",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testNetworkSecurityPolicy", "NetworkSecurityPolicy names did not match", sgp)
}

func TestNetworkSecurityPolicyUpdate(t *testing.T) {
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
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err = ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG policy was not found in DB")
	Assert(t, sgp.Name == "testNetworkSecurityPolicy", "Nat Pool names did not match", sgp)

	sgpSpec := netproto.NetworkSecurityPolicySpec{
		AttachGroup: []string{"preCreatedSecurityGroup"},
		Rules: []netproto.PolicyRule{
			{
				Action: "DENY",
			},
		},
	}

	sgPolicy.Spec = sgpSpec

	err = ag.UpdateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error updating sg policy")

	updNetworkSecurityPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err = ag.UpdateNetworkSecurityPolicy(&updNetworkSecurityPolicy)
	AssertOk(t, err, "Error updating sg policy")

}

func TestNetworkSecurityPolicyALGMatchMSRPC(t *testing.T) {
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
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err = ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testNetworkSecurityPolicy", "NetworkSecurityPolicy names did not match", sgp)

}

func TestNetworkSecurityPolicyALGMatchICMP(t *testing.T) {
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
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err = ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testNetworkSecurityPolicy", "NetworkSecurityPolicy names did not match", sgp)

}

func TestNetworkSecurityPolicyMatchAllSrc(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "sg policy creation matching on all source failed")
}

func TestNetworkSecurityPolicyMatchAllDst(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "sg policy matching on all dest failed")
}

func TestNetworkSecurityPolicyMatchAll(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "mordor",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "DENY",
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "sg policy matching on all failed")
}

func TestNetworkSecurityPolicyICMPProtoMatch(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testNetworkSecurityPolicy", "NetworkSecurityPolicy names did not match", sgp)

}

func TestNetworkSecurityPolicyCreateIANA6_TCP(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
								Protocol: "6",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testNetworkSecurityPolicy", "NetworkSecurityPolicy names did not match", sgp)

	err = ag.DeleteNetworkSecurityPolicy(sgPolicy.Tenant, sgPolicy.Namespace, sgPolicy.Name)
	AssertOk(t, err, "Error deleting sg policy")
}

func TestNetworkSecurityPolicyCreateIANA17_UDP(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
								Port:     "53",
								Protocol: "17",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testNetworkSecurityPolicy", "NetworkSecurityPolicy names did not match", sgp)

	err = ag.DeleteNetworkSecurityPolicy(sgPolicy.Tenant, sgPolicy.Namespace, sgPolicy.Name)
	AssertOk(t, err, "Error deleting sg policy")
}

func TestNetworkSecurityPolicyCreateIANA1_ICMP(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
								Protocol: "1",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testNetworkSecurityPolicy", "NetworkSecurityPolicy names did not match", sgp)

	err = ag.DeleteNetworkSecurityPolicy(sgPolicy.Tenant, sgPolicy.Namespace, sgPolicy.Name)
	AssertOk(t, err, "Error deleting sg policy")
}

func TestNetworkSecurityPolicyCreatePort0(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
								Protocol: "tcp",
								Port:     "0",
							},
						},
					},
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Error creating sg policy")
	sgp, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy was not found in DB")
	Assert(t, sgp.Name == "testNetworkSecurityPolicy", "NetworkSecurityPolicy names did not match", sgp)

	err = ag.DeleteNetworkSecurityPolicy(sgPolicy.Tenant, sgPolicy.Namespace, sgPolicy.Name)
	AssertOk(t, err, "Error deleting sg policy")
}

//--------------------- Corner Case Tests ---------------------//

func TestInvalidNetworkSecurityPolicyICMPPortMatch(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	Assert(t, err != nil, "NetworkSecurityPolicy with ICMP protocol with a port must fail")
}

func TestNetworkSecurityPolicyOnMatchAllSrc(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "SG Policy creation with empty src should match all and pass validation")
}

func TestNetworkSecurityPolicyOnMatchAllDst(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "SG Policy creation with empty dst should match all and pass validation")
}

func TestNetworkSecurityPolicyOnMatchAll(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
				},
			},
		},
	}

	// create sg policy
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "SG Policy creation with empty src and dst should match all and pass validation")
}

func TestNetworkSecurityPolicyUpdateOnNonExistentNetworkSecurityPolicy(t *testing.T) {
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
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err = ag.UpdateNetworkSecurityPolicy(&sgPolicy)
	Assert(t, err != nil, "Nat policy updates on non existing nat policies fail")
}

func TestNetworkSecurityPolicyOnNonExistentSG(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	Assert(t, err != nil, "SG Policy creation with non existent security group attachment point should fail.")
}

func TestNetworkSecurityPolicyOnNonAttachmentPoints(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	Assert(t, err != nil, "SG Policy creation with non existent attachment points.")
}

func TestNetworkSecurityPolicyMatchAllPorts(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "Policies with empty port configs should not fail")
}

func TestNetworkSecurityPolicyBadPortRange(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	Assert(t, err != nil, "Policies with bad l4port config should fail")
}

func TestNetworkSecurityPolicyOutsidePortRange(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	Assert(t, err != nil, "Policies with ports > 64K should fail")
}

func TestInvalidNetworkSecurityPolicyWithAppAndProtoPort(t *testing.T) {
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
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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
	err = ag.CreateNetworkSecurityPolicy(&sgPolicy)
	Assert(t, err != nil, "NetworkSecurityPolicy referring to both ALG and match criteria in the same rule must fail")
}

func TestConsistentRuleHashes(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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

	err := ag.CreateNetworkSecurityPolicy(&sgPolicy)
	AssertOk(t, err, "SgPolicy Create failed")
	actualNetworkSecurityPolicy, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy not found")

	rule1HashOnCreate := actualNetworkSecurityPolicy.Spec.Rules[0].ID
	rule2HashOnCreate := actualNetworkSecurityPolicy.Spec.Rules[1].ID

	// Update SG Policy, with updates going only to rule 2
	updNetworkSecurityPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
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

	err = ag.UpdateNetworkSecurityPolicy(&updNetworkSecurityPolicy)
	AssertOk(t, err, "SgPolicy Update failed")
	updatedNetworkSecurityPolicy, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "SG Policy not found")

	rule1HashOnUpdate := updatedNetworkSecurityPolicy.Spec.Rules[0].ID
	rule2HashOnUpdate := updatedNetworkSecurityPolicy.Spec.Rules[1].ID

	// Ensure rule 1 hash on create and on update are the same
	Assert(t, rule1HashOnCreate == rule1HashOnUpdate, "Hashes got changed on a rule that was not updated")

	// Ensure rule 2 hash on create and on update are not the same
	Assert(t, rule2HashOnCreate != rule2HashOnUpdate, "Hashes did not change on a rule that was updated")
}
