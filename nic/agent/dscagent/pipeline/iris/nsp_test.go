// +build iris

package iris

import (
	"sync"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleNetworkSecurityPolicy(t *testing.T) {
	var mappings sync.Map
	t.Parallel()
	nsp := netproto.NetworkSecurityPolicy{
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
	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Update, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Delete, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, nwSecClient, 42, nsp, 65, &mappings)
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

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Update, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Delete, nsp, 65, &mappings)
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
	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	nsp.Spec.Rules[0].ID = 44

	err = HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Update, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	//err = HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Delete, nsp, 65, &mappings)
	//if err != nil {
	//	t.Fatal(err)
	//}
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
	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Update, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Delete, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicyDNSAppCustomTimeouts(t *testing.T) {
	t.Parallel()
	var mappings sync.Map

	dns := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
					MaxMessageLength:       42,
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
					AppName: "dns",
				},
			},
		},
	}
	mappings.LoadOrStore(nsp.Spec.Rules[0].ID, dns)

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicyDNSAppDefaultTimeouts(t *testing.T) {
	t.Parallel()
	var mappings sync.Map

	dns := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
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
					AppName: "dns",
				},
			},
		},
	}
	mappings.LoadOrStore(nsp.Spec.Rules[0].ID, dns)

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicyFTPApp(t *testing.T) {
	t.Parallel()
	var mappings sync.Map

	ftp := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				FTP: &netproto.FTP{
					AllowMismatchIPAddresses: true,
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
					AppName: "ftp",
				},
			},
		},
	}
	mappings.LoadOrStore(nsp.Spec.Rules[0].ID, ftp)

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicyMSRPCAppCustomTimeouts(t *testing.T) {
	t.Parallel()
	var mappings sync.Map

	msrpc := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				MSRPC: []*netproto.RPC{
					{
						ProgramID:        "42",
						ProgramIDTimeout: "10s",
					},
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
					AppName: "msrpc",
				},
			},
		},
	}
	mappings.LoadOrStore(nsp.Spec.Rules[0].ID, msrpc)

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicyMSRPCAppDefaultTimeouts(t *testing.T) {
	t.Parallel()
	var mappings sync.Map

	msrpc := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				MSRPC: []*netproto.RPC{
					{
						ProgramID: "42",
					},
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
					AppName: "msrpc",
				},
			},
		},
	}
	mappings.LoadOrStore(nsp.Spec.Rules[0].ID, msrpc)

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicySUNRPCAppCustomTimeouts(t *testing.T) {
	t.Parallel()
	var mappings sync.Map

	sunrpc := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				SUNRPC: []*netproto.RPC{
					{
						ProgramID: "42",
					},
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
					AppName: "sunrpc",
				},
			},
		},
	}
	mappings.LoadOrStore(nsp.Spec.Rules[0].ID, sunrpc)

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicyRTSPApp(t *testing.T) {
	t.Parallel()
	var mappings sync.Map

	rtsp := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				RTSP: &netproto.RTSP{},
			},
		},
	}

	nsp := netproto.NetworkSecurityPolicy{
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
					ID:     1,
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					AppName: "rtsp",
				},
			},
		},
	}
	mappings.LoadOrStore(nsp.Spec.Rules[0].ID, rtsp)

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicySIPAppCustomTimeouts(t *testing.T) {
	t.Parallel()
	var mappings sync.Map

	sip := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				SIP: &netproto.SIP{
					MediaInactivityTimeout: "30s",
					MaxCallDuration:        "1s",
					CTimeout:               "10us",
					T1Timeout:              "1h",
					T4Timeout:              "20m",
					DSCPCodePoint:          42,
				},
			},
			AppIdleTimeout: "10s",
		},
	}

	nsp := netproto.NetworkSecurityPolicy{
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
					ID:     1,
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					AppName: "sip",
				},
			},
		},
	}
	mappings.LoadOrStore(nsp.Spec.Rules[0].ID, sip)

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicySIPAppDefaultTimeouts(t *testing.T) {
	t.Parallel()
	var mappings sync.Map

	sip := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				SIP: &netproto.SIP{},
			},
			AppIdleTimeout: "10s",
		},
	}

	nsp := netproto.NetworkSecurityPolicy{
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
					ID:     1,
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					AppName: "sip",
				},
			},
		},
	}
	mappings.LoadOrStore(nsp.Spec.Rules[0].ID, sip)

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleNetworkSecurityPolicyTFTPApp(t *testing.T) {
	t.Parallel()
	var mappings sync.Map

	tftp := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "dns",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
				{
					Protocol: "tcp",
					Port:     "54",
				},
			},
			ALG: &netproto.ALG{
				TFTP: &netproto.TFTP{},
			},
		},
	}

	nsp := netproto.NetworkSecurityPolicy{
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
					ID:     1,
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					AppName: "tftp",
				},
			},
		},
	}
	mappings.LoadOrStore(nsp.Spec.Rules[0].ID, tftp)

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
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

	err := HandleNetworkSecurityPolicy(infraAPI, nwSecClient, types.Create, nsp, 65, &mappings)
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
	err := HandleNetworkSecurityPolicy(i, nwSecClient, types.Create, nsp, 65, &mappings)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleNetworkSecurityPolicy(i, nwSecClient, types.Update, nsp, 65, &mappings)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleNetworkSecurityPolicy(i, nwSecClient, types.Delete, nsp, 65, &mappings)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
