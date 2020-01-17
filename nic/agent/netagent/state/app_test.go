// +build ignore

package state

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestAppCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
				},
			},
		},
	}

	preExistingAppCount := len(ag.ListApp())

	// create app
	err := ag.CreateApp(&app)
	AssertOk(t, err, "Error creating app")
	p, err := ag.FindApp(app.ObjectMeta)
	AssertOk(t, err, "App was not found in DB")
	Assert(t, p.Name == "testApp", "App names did not match", app)

	// verify duplicate app creations succeed
	err = ag.CreateApp(&app)
	AssertOk(t, err, "Error creating duplicate app")

	// verify list api works.
	rtList := ag.ListApp()
	Assert(t, len(rtList) == preExistingAppCount+1, "Incorrect number of apps")

	// delete the app and verify its gone from db
	err = ag.DeleteApp(app.Tenant, app.Namespace, app.Name)
	AssertOk(t, err, "Error deleting app")
	_, err = ag.FindApp(app.ObjectMeta)
	Assert(t, err != nil, "App was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteApp(app.Tenant, app.Namespace, app.Name)
	Assert(t, err != nil, "deleting non-existing app succeeded", ag)
}

func TestAppCreateDNS(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	dnsApp := netproto.App{
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
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets:   true,
					DropMultiZonePackets:     true,
					DropLongLabelPackets:     true,
					DropMultiQuestionPackets: true,
					QueryResponseTimeout:     "30s",
				},
			},
		},
	}

	// create app
	err := ag.CreateApp(&dnsApp)
	AssertOk(t, err, "Error creating dns app")
	p, err := ag.FindApp(dnsApp.ObjectMeta)
	AssertOk(t, err, "DNS App was not found in DB")
	Assert(t, p.Name == "dns", "App names did not match", dnsApp)
}

func TestAppCreateICMP(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	icmpApp := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "ping",
		},
		Spec: netproto.AppSpec{
			ALG: &netproto.ALG{
				ICMP: &netproto.ICMP{},
			},
		},
	}

	// create app
	err := ag.CreateApp(&icmpApp)
	AssertOk(t, err, "Error creating icmp app")
	p, err := ag.FindApp(icmpApp.ObjectMeta)
	AssertOk(t, err, "ICMP was not found in DB")
	Assert(t, p.Name == "ping", "App names did not match", icmpApp)
}

func TestAppCreateICMPWithProtocolSpecified(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	icmpApp := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "ping",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &netproto.ALG{
				ICMP: &netproto.ICMP{},
			},
		},
	}

	// create app
	err := ag.CreateApp(&icmpApp)
	AssertOk(t, err, "Error creating icmp app")
	p, err := ag.FindApp(icmpApp.ObjectMeta)
	AssertOk(t, err, "ICMP was not found in DB")
	Assert(t, p.Name == "ping", "App names did not match", icmpApp)
}

func TestAppUpdateDNS(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// app
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
				},
			},
		},
	}

	// create app
	err := ag.CreateApp(&app)
	AssertOk(t, err, "Error creating app")
	p, err := ag.FindApp(app.ObjectMeta)
	AssertOk(t, err, "App not found in DB")
	Assert(t, p.Name == "testApp", "App names did not match", app)

	appSpec := netproto.AppSpec{
		AppIdleTimeout: "1m",
	}

	app.Spec = appSpec

	err = ag.UpdateApp(&app)
	AssertOk(t, err, "Error updating app")

}

func TestAppUpdateICMP(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// app
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &netproto.ALG{
				ICMP: &netproto.ICMP{
					Type: 13,
					Code: 6,
				},
			},
		},
	}

	// create app
	err := ag.CreateApp(&app)
	AssertOk(t, err, "Error creating app")
	p, err := ag.FindApp(app.ObjectMeta)
	AssertOk(t, err, "App not found in DB")
	Assert(t, p.Name == "testApp", "App names did not match", app)

	appSpec := netproto.AppSpec{
		ProtoPorts: []*netproto.ProtoPort{
			{
				Protocol: "icmp",
			},
		},
		ALG: &netproto.ALG{
			ICMP: &netproto.ICMP{
				Type: 13,
				Code: 6,
			},
		},
		AppIdleTimeout: "1m",
	}

	app.Spec = appSpec

	err = ag.UpdateApp(&app)
	AssertOk(t, err, "Error updating app")

}
func TestAppUpdateLinkedNetworkSecurityPolicyUpdate(t *testing.T) {
	// create netagent
	ag, _, dp := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// app
	app := netproto.App{
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
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
				},
			},
		},
	}

	// app
	updApp := netproto.App{
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
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "1m",
				},
			},
		},
	}

	// create app
	err := ag.CreateApp(&app)
	AssertOk(t, err, "Error creating app")
	p, err := ag.FindApp(app.ObjectMeta)
	AssertOk(t, err, "App not found in DB")
	Assert(t, p.Name == "dns", "App names did not match", app)

	// create an NetworkSecurityPolicy referring to the App
	// sg policy
	sgPolicy := &netproto.NetworkSecurityPolicy{
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

	err = ag.CreateNetworkSecurityPolicy(sgPolicy)
	AssertOk(t, err, "Failed to create a linked SG Policy")

	actualNetworkSecurityPolicy, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "Failed to find find sg policy")

	// find the sgpolicy in datapath
	dpsgp, ok := dp.DB.SgPolicyDB[fmt.Sprintf("%s|%s", actualNetworkSecurityPolicy.ObjectMeta.Tenant, actualNetworkSecurityPolicy.ObjectMeta.Name)]
	Assert(t, ok, "Failed to find sgpolicy in datapath")
	Assert(t, len(dpsgp.Request[0].Rule) == 1, "sgpolicy in datapath is incorrect")
	Assert(t, dpsgp.Request[0].Rule[0].Action.AppData.Alg == halproto.ALGName_APP_SVC_DNS, "sgpolicy in datapath is incorrect")
	Assert(t, dpsgp.Request[0].Rule[0].Action.AppData.GetDnsOptionInfo().QueryResponseTimeout == 30, "sgpolicy in datapath is incorrect")

	err = ag.UpdateApp(&updApp)
	AssertOk(t, err, "Error updating app")
	// Ensure the updated app has correct fields.
	actualApp, err := ag.FindApp(updApp.ObjectMeta)
	AssertOk(t, err, "Failed to find the update app")
	Assert(t, actualApp.Spec.ALG.DNS.QueryResponseTimeout == updApp.Spec.ALG.DNS.QueryResponseTimeout, "Updated app didn't have expected fields updated")

	Assert(t, actualApp.Spec.ALG.DNS.QueryResponseTimeout == updApp.Spec.ALG.DNS.QueryResponseTimeout, "Updated app didn't have expected fields updated")

	updateNetworkSecurityPolicy, err := ag.FindNetworkSecurityPolicy(sgPolicy.ObjectMeta)
	AssertOk(t, err, "Failed to find find sg policy")

	// Ensure the rule ids have changed
	Assert(t, updateNetworkSecurityPolicy.Spec.Rules[0].ID == actualNetworkSecurityPolicy.Spec.Rules[0].ID, "Rule IDs must change on an update")

	// find the sgpolicy in datapath
	dpsgp, ok = dp.DB.SgPolicyDB[fmt.Sprintf("%s|%s", actualNetworkSecurityPolicy.ObjectMeta.Tenant, actualNetworkSecurityPolicy.ObjectMeta.Name)]
	Assert(t, ok, "Failed to find sgpolicy in datapath")
	Assert(t, len(dpsgp.Request[0].Rule) == 1, "sgpolicy in datapath is incorrect")
	Assert(t, dpsgp.Request[0].Rule[0].Action.AppData.GetDnsOptionInfo().QueryResponseTimeout == 60, "sgpolicy in datapath is incorrect")
}

//--------------------- Corner Case Tests ---------------------//
func TestAppCreateOnNonExistingNamespace(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "nonExistentNamespace",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
				},
			},
		},
	}

	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app create on non existent namespace should fail")
}

func TestAppUpdateOnNonExistentApp(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
				},
			},
		},
	}

	// create app
	err := ag.UpdateApp(&app)
	Assert(t, err != nil, "app udpate with non existent app should fail")
}

func TestAppRepeatedRPC(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				SUNRPC: []*netproto.RPC{
					{
						ProgramID:        "42",
						ProgramIDTimeout: "2s",
					},
					{
						ProgramID:        "84",
						ProgramIDTimeout: "10s",
					},
				},
			},
		},
	}
	// create app
	err := ag.CreateApp(&app)
	AssertOk(t, err, "rpc with repeated program IDs must pass")
}

func TestAppMultipleALGConfig_TwoALG(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
				},
				FTP: &netproto.FTP{
					AllowMismatchIPAddresses: true,
				},
			},
		},
	}
	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app with multiple alg configs should fail")
}

func TestAppMultipleALGConfig_ThreeALG(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
				},
				FTP: &netproto.FTP{
					AllowMismatchIPAddresses: true,
				},
				SUNRPC: []*netproto.RPC{
					&netproto.RPC{
						ProgramID:        "42",
						ProgramIDTimeout: "2s",
					},
				},
			},
		},
	}
	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app with multiple alg configs should fail")
}

func TestAppBadAppIdleTimeout(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "30s",
				},
			},
			AppIdleTimeout: "bad timeout format",
		},
	}
	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app with multiple alg configs should fail")
}

func TestAppBadDNSQueryResponseTimeout(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				DNS: &netproto.DNS{
					DropLargeDomainPackets: true,
					DropMultiZonePackets:   true,
					QueryResponseTimeout:   "bad timeout format",
				},
			},
			AppIdleTimeout: "bad timeout format",
		},
	}
	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app bad dns query response timeout must fail")
}

func TestAppBadSIPMediaInActivityTimeout(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				SIP: &netproto.SIP{
					MediaInactivityTimeout: "bad timeout format",
				},
			},
			AppIdleTimeout: "1m",
		},
	}
	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app bad sip media inactivity timeout must fail")
}

func TestAppBadSunRPCProgramIDTimeout(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "tcp",
					Port:     "13337",
				},
			},
			ALG: &netproto.ALG{
				SUNRPC: []*netproto.RPC{
					{
						ProgramID:        "42",
						ProgramIDTimeout: "2s",
					},
					{
						ProgramID:        "84",
						ProgramIDTimeout: "bad program id timeout",
					},
				},
			},
			AppIdleTimeout: "1m",
		},
	}
	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app bad sun rpc program id timeout must fail")
}

func TestAppBadMSRPCProgramIDTimeout(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "tcp",
					Port:     "13337",
				},
			},
			ALG: &netproto.ALG{
				MSRPC: []*netproto.RPC{
					{
						ProgramID:        "42",
						ProgramIDTimeout: "2s",
					},
					{
						ProgramID:        "84",
						ProgramIDTimeout: "bad program id timeout",
					},
				},
			},
			AppIdleTimeout: "1m",
		},
	}
	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app bad msrpc program id timeout must fail")
}

func TestAppBadSIPMaxCallDuration(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				SIP: &netproto.SIP{
					MaxCallDuration: "bad timeout format",
				},
			},
			AppIdleTimeout: "1m",
		},
	}
	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app with bad sip max call duration timeout must fail")
}

func TestAppBadSIPCTimeout(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				SIP: &netproto.SIP{
					CTimeout: "bad timeout format",
				},
			},
			AppIdleTimeout: "1m",
		},
	}
	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app with bad sip c timeout must fail")
}

func TestAppBadSIPT1Timeout(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				SIP: &netproto.SIP{
					T1Timeout: "bad timeout format",
				},
			},
			AppIdleTimeout: "1m",
		},
	}
	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app with bad sip t1 timeout must fail")
}

func TestAppBadSIPT4Timeout(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "udp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				SIP: &netproto.SIP{
					T4Timeout: "bad timeout format",
				},
			},
			AppIdleTimeout: "1m",
		},
	}
	// create app
	err := ag.CreateApp(&app)
	Assert(t, err != nil, "app with bad sip t4 timeout must fail")
}

func TestInvalidAppCreateICMPWithPortSpecified(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	icmpApp := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "ping",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "icmp",
					Port:     "53",
				},
			},
			ALG: &netproto.ALG{
				ICMP: &netproto.ICMP{},
			},
		},
	}

	// create app
	err := ag.CreateApp(&icmpApp)
	Assert(t, err != nil, "ICMP App with port must fail")
}

func TestInvalidAppCreateICMPWithPortocolSpecified(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	icmpApp := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "ping",
		},
		Spec: netproto.AppSpec{
			ProtoPorts: []*netproto.ProtoPort{
				{
					Protocol: "tcp",
				},
			},
			ALG: &netproto.ALG{
				ICMP: &netproto.ICMP{},
			},
		},
	}

	// create app
	err := ag.CreateApp(&icmpApp)
	Assert(t, err != nil, "ICMP App with non icmp proto port must fail")
}
