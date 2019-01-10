package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
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
	_, err = ag.FindNatPolicy(app.ObjectMeta)
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
			ProtoPorts: []string{"udp/53"},
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

func TestAppUpdate(t *testing.T) {
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

	// create app
	err := ag.UpdateApp(&app)
	Assert(t, err != nil, "app udpate with non existent app should fail")
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
			ProtoPorts: []string{"udp/53"},
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
			ProtoPorts: []string{"udp/53"},
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
						ProgramID:        42,
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
			ProtoPorts: []string{"udp/53"},
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
			ProtoPorts: []string{"udp/53"},
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
			ProtoPorts: []string{"udp/53"},
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
			ProtoPorts: []string{"udp/53"},
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
			ProtoPorts: []string{"udp/53"},
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
			ProtoPorts: []string{"udp/53"},
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
			ProtoPorts: []string{"udp/53"},
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
