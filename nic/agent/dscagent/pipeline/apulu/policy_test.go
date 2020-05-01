// +build apulu

package apulu

import (
	"sync"
	"testing"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestRuleL3MatchExpansion(t *testing.T) {
	var mappings sync.Map
	nsp := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.1.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"10.1.1.0/24"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Port:     "1-65535",
								Protocol: "tcp",
							},
							{
								Port:     "1-65535",
								Protocol: "udp",
							},
							{
								Protocol: "icmp",
							},
						},
					},
				},
			},
		},
	}
	err := HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Create, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}
func TestRuleLimit(t *testing.T) {
	var mappings sync.Map
	t.Parallel()
	nsp := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.1.1.0/24", "10.1.2.0/24", "10.1.3.0/24", "10.1.4.0/24", "10.1.5.0/24", "10.1.6.0/24"},
						ProtoPorts: []*netproto.ProtoPort{
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
					Dst: &netproto.MatchSelector{
						Addresses: []string{"10.1.1.0/24", "10.1.2.0/24", "10.1.3.0/24", "10.1.4.0/24", "10.1.5.0/24", "10.1.6.0/24"},
						ProtoPorts: []*netproto.ProtoPort{
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
	err := HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Create, nsp, &mappings)
	if err == nil {
		t.Fatalf("Expected failure. Got %v", err)
	}
	nsp.Spec.Rules[0].Src.Addresses = []string{"10.1.1.0/24", "10.1.2.0/24", "10.1.3.0/24", "10.1.4.0/24", "10.1.5.0/24"}
	nsp.Spec.Rules[0].Dst.Addresses = []string{"10.1.1.0/24", "10.1.2.0/24", "10.1.3.0/24", "10.1.4.0/24", "10.1.5.0/24"}
	err = HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Create, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestMultipleIPRule(t *testing.T) {
	var mappings sync.Map
	t.Parallel()
	nsp := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.1.1.0/24", "10.1.2.0/24", "10.1.3.0/24", "10.1.4.0/24", "10.1.5.0/24", "10.1.6.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"10.1.1.0/24"},
						ProtoPorts: []*netproto.ProtoPort{
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
	err := HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Create, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicy(t *testing.T) {
	var mappings sync.Map
	t.Parallel()
	nsp := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "192.168.1.1", "172.16.0.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
						ProtoPorts: []*netproto.ProtoPort{
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
	err := HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Create, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Update, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Delete, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, 42, nsp, &mappings)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleNetworkSecurityPolicyInlineRulesDeny(t *testing.T) {
	var mappings sync.Map

	t.Parallel()
	nsp := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "DENY",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "192.168.1.1", "172.16.0.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
						ProtoPorts: []*netproto.ProtoPort{
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

	err := HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Create, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Update, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Delete, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicyInlineRulesReject(t *testing.T) {
	t.Parallel()
	var mappings sync.Map
	nsp := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "REJECT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "192.168.1.1", "172.16.0.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
						ProtoPorts: []*netproto.ProtoPort{
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
					ID: 42,
				},
			},
		},
	}
	err := HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Create, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	nsp.Spec.Rules[0].ID = 44

	err = HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Update, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Delete, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicyInlineRulesLog(t *testing.T) {
	t.Parallel()
	var mappings sync.Map
	nsp := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "LOG",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "192.168.1.1", "172.16.0.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
						ProtoPorts: []*netproto.ProtoPort{
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
	err := HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Create, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Update, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Delete, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicyICMPApp(t *testing.T) {
	t.Parallel()
	var mappings sync.Map

	icmp := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ALG: &netproto.ALG{
				ICMP: &netproto.ICMP{
					Type: 13,
					Code: 42,
				},
			},
		},
	}

	nsp := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					ID:     1,
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					AppName: "icmp",
				},
			},
		},
	}
	mappings.LoadOrStore(nsp.Spec.Rules[0].ID, icmp)

	err := HandleNetworkSecurityPolicy(infraAPI, secPolicyClient, types.Create, nsp, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicyInfraFailures(t *testing.T) {
	t.Parallel()
	var mappings sync.Map
	nsp := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
			UUID:      uuid.NewV4().String(),
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "192.168.1.1", "172.16.0.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
						ProtoPorts: []*netproto.ProtoPort{
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
	i := newBadInfraAPI()
	err := HandleNetworkSecurityPolicy(i, secPolicyClient, types.Create, nsp, &mappings)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleNetworkSecurityPolicy(i, secPolicyClient, types.Update, nsp, &mappings)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleNetworkSecurityPolicy(i, secPolicyClient, types.Delete, nsp, &mappings)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
