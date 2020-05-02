// +build apulu

package validator

import (
	"testing"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestRuleLimit(t *testing.T) {
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
	err := ValidateNetworkSecurityPolicy(infraAPI, nsp)
	if err == nil {
		t.Fatalf("Expected failure. Got %v", err)
	}
	nsp.Spec.Rules[0].Src.Addresses = []string{"10.1.1.0/24", "10.1.2.0/24", "10.1.3.0/24", "10.1.4.0/24", "10.1.5.0/24"}
	nsp.Spec.Rules[0].Dst.Addresses = []string{"10.1.1.0/24", "10.1.2.0/24", "10.1.3.0/24", "10.1.4.0/24", "10.1.5.0/24"}
	err = ValidateNetworkSecurityPolicy(infraAPI, nsp)
	if err != nil {
		t.Fatal(err)
	}
}
