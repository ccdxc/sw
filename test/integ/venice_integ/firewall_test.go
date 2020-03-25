// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"fmt"
	"strings"
	"time"

	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestNpmFirewallProfile tests firewall profile create/update/delete operations
func (it *veniceIntegSuite) TestFirewallProfile(c *C) {
	if it.config.DatapathKind == "hal" {
		c.Skip("Uncomment when deletes go through to HAL.")
	}
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// firewall profile
	fwp := security.FirewallProfile{
		TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "default",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.FirewallProfileSpec{
			SessionIdleTimeout:        "90s",
			TCPConnectionSetupTimeout: "30s",
			TCPCloseTimeout:           "15s",
			TCPHalfClosedTimeout:      "120s",
			TCPDropTimeout:            "90s",
			UDPDropTimeout:            "60s",
			ICMPDropTimeout:           "60s",
			DropTimeout:               "60s",
			TcpTimeout:                "3m",
			UdpTimeout:                "3m",
			IcmpTimeout:               "3m",
			TcpHalfOpenSessionLimit:   20000,
			UdpActiveSessionLimit:     20000,
			IcmpActiveSessionLimit:    20000,
			OtherActiveSessionLimit:   20000,
		},
	}

	fwpDefault := security.FirewallProfile{}
	fwpDefault.Defaults("all")
	// There should already be a firewall profile created by default.
	// verify policy gets created in agent
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			profileMeta := netproto.SecurityProfile{
				TypeMeta:   api.TypeMeta{Kind: "SecurityProfile"},
				ObjectMeta: fwp.ObjectMeta,
			}
			secp, cerr := sn.agent.PipelineAPI.HandleSecurityProfile(agentTypes.Get, profileMeta)

			if (cerr != nil) || (secp[0].Name != fwp.Name) {
				return false, secp
			}
		}
		return true, nil
	}, "Firewall profile not found in agent", "100ms", it.pollTimeout())

	// verify all the parameters
	for _, sn := range it.snics {
		profileMeta := netproto.SecurityProfile{
			TypeMeta:   api.TypeMeta{Kind: "SecurityProfile"},
			ObjectMeta: fwp.ObjectMeta,
		}
		secp, cerr := sn.agent.PipelineAPI.HandleSecurityProfile(agentTypes.Get, profileMeta)

		AssertOk(c, cerr, "Security profile not found in agent")
		AssertEquals(c, secp[0].Spec.Timeouts.SessionIdle, fwpDefault.Spec.SessionIdleTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.TCP, fwpDefault.Spec.TcpTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.TCPDrop, fwpDefault.Spec.TCPDropTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.TCPConnectionSetup, fwpDefault.Spec.TCPConnectionSetupTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.TCPClose, fwpDefault.Spec.TCPCloseTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.TCPHalfClose, fwpDefault.Spec.TCPHalfClosedTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.Drop, fwpDefault.Spec.DropTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.UDP, fwpDefault.Spec.UdpTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.UDPDrop, fwpDefault.Spec.UDPDropTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.ICMP, fwpDefault.Spec.IcmpTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.ICMPDrop, fwpDefault.Spec.ICMPDropTimeout, "incorrect params")
	}

	// change conn track and session timeout
	fwp.Spec.SessionIdleTimeout = "5m"
	_, err = it.restClient.SecurityV1().FirewallProfile().Update(ctx, &fwp)
	AssertOk(c, err, "Error updating firewall profile")

	// verify params got updated in agent
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			profileMeta := netproto.SecurityProfile{
				TypeMeta:   api.TypeMeta{Kind: "SecurityProfile"},
				ObjectMeta: fwp.ObjectMeta,
			}
			secp, cerr := sn.agent.PipelineAPI.HandleSecurityProfile(agentTypes.Get, profileMeta)
			if (cerr != nil) || (secp[0].Spec.Timeouts.SessionIdle != fwp.Spec.SessionIdleTimeout) {
				return false, secp
			}
		}
		return true, nil
	}, "Firewall profile params incorrect in agent", "100ms", it.pollTimeout())
	// verify all the parameters
	for _, sn := range it.snics {
		profileMeta := netproto.SecurityProfile{
			TypeMeta:   api.TypeMeta{Kind: "SecurityProfile"},
			ObjectMeta: fwp.ObjectMeta,
		}
		secp, cerr := sn.agent.PipelineAPI.HandleSecurityProfile(agentTypes.Get, profileMeta)
		AssertOk(c, cerr, "Security profile not found in agent")
		AssertEquals(c, secp[0].Spec.Timeouts.SessionIdle, fwp.Spec.SessionIdleTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.TCP, fwp.Spec.TcpTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.TCPDrop, fwp.Spec.TCPDropTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.TCPConnectionSetup, fwp.Spec.TCPConnectionSetupTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.TCPClose, fwp.Spec.TCPCloseTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.TCPHalfClose, fwp.Spec.TCPHalfClosedTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.Drop, fwp.Spec.DropTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.UDP, fwp.Spec.UdpTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.UDPDrop, fwp.Spec.UDPDropTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.ICMP, fwp.Spec.IcmpTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.Timeouts.ICMPDrop, fwp.Spec.ICMPDropTimeout, "incorrect params")
		AssertEquals(c, secp[0].Spec.RateLimits.TcpHalfOpenSessionLimit, fwp.Spec.TcpHalfOpenSessionLimit, "incorrect params")
		AssertEquals(c, secp[0].Spec.RateLimits.UdpActiveSessionLimit, fwp.Spec.UdpActiveSessionLimit, "incorrect params")
		AssertEquals(c, secp[0].Spec.RateLimits.IcmpActiveSessionLimit, fwp.Spec.IcmpActiveSessionLimit, "incorrect params")
		AssertEquals(c, secp[0].Spec.RateLimits.OtherActiveSessionLimit, fwp.Spec.OtherActiveSessionLimit, "incorrect params")
	}

	// verify FirewallProfile status reflects propagation status
	AssertEventually(c, func() (bool, interface{}) {
		fwp, gerr := it.restClient.SecurityV1().FirewallProfile().Get(ctx, &fwp.ObjectMeta)
		if err != nil {
			return false, gerr
		}
		if fwp.Status.PropagationStatus.GenerationID != fwp.ObjectMeta.GenerationID {
			return false, fwp
		}
		if (fwp.Status.PropagationStatus.Updated != int32(it.config.NumHosts)) || (fwp.Status.PropagationStatus.Pending != 0) ||
			(fwp.Status.PropagationStatus.MinVersion != "") {
			return false, fwp
		}
		return true, nil
	}, "FirewallProfile status was not updated", "100ms", it.pollTimeout())

	// delete firewall profile is not allowed.
	_, err = it.restClient.SecurityV1().FirewallProfile().Delete(ctx, &fwp.ObjectMeta)
}

func (it *veniceIntegSuite) TestIcmpApp(c *C) {
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// ICMP app
	icmpApp := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "icmp-app",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			Timeout: "5m",
			ALG: &security.ALG{
				Type: security.ALG_ICMP.String(),
				Icmp: &security.Icmp{
					Type: "1",
					Code: "2",
				},
			},
		},
	}

	// create app
	_, err = it.restClient.SecurityV1().App().Create(ctx, &icmpApp)
	AssertOk(c, err, "Error creating icmp app")

	// verify all agents have the app
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: icmpApp.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if (cerr != nil) || (napp[0].Name != icmpApp.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// verify ALG params
	for _, sn := range it.snics {
		sapp := netproto.App{
			TypeMeta:   api.TypeMeta{Kind: "App"},
			ObjectMeta: icmpApp.ObjectMeta,
		}
		napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp[0].Spec.AppIdleTimeout, icmpApp.Spec.Timeout, "invalid alg params")
		AssertEquals(c, fmt.Sprintf("%d", napp[0].Spec.ALG.ICMP.Type), icmpApp.Spec.ALG.Icmp.Type, "invalid alg params")
		AssertEquals(c, fmt.Sprintf("%d", napp[0].Spec.ALG.ICMP.Code), icmpApp.Spec.ALG.Icmp.Code, "invalid alg params")

	}

	// create an sg policy using the app
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "test-sgpolicy",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					FromIPAddresses: []string{"10.1.1.0/24"},
					ToIPAddresses:   []string{"10.2.1.0/24"},
					Apps:            []string{"icmp-app"},
					Action:          "PERMIT",
				},
			},
		},
	}
	_, err = it.restClient.SecurityV1().NetworkSecurityPolicy().Create(ctx, &sgp)
	AssertOk(c, err, "Error creating sg policy")

	// verify agents have this policy and params are correct
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sgpMeta := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: sgp.ObjectMeta,
			}
			nsgp, cerr := sn.agent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, sgpMeta)
			if cerr != nil {
				return false, cerr
			}
			if len(nsgp[0].Spec.Rules) != len(sgp.Spec.Rules) {
				return false, nsgp
			}
		}
		return true, nil
	}, "Sg policy not found in agent or params didnt match", "100ms", it.pollTimeout())

	/*
		// verify app has sgpolicy in status
		AssertEventually(c, func() (bool, interface{}) {
			gapp, gerr := it.restClient.SecurityV1().App().Get(ctx, &icmpApp.ObjectMeta)
			if gerr != nil {
				return false, gerr
			}
			if len(gapp.Status.AttachedPolicies) == 0 {
				return false, gapp
			}
			found := false
			for _, tp := range gapp.Status.AttachedPolicies {
				if tp == sgp.Name {
					found = true
				}
			}
			if !found {
				return false, gapp
			}
			return true, nil
		}, "App did not have sgpolicy in status", "100ms", it.pollTimeout())
	*/

	// verify creating agpolicy with unknown app fails
	sgpInv := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "invalid-sgpolicy",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					FromIPAddresses: []string{"10.1.1.0/24"},
					ToIPAddresses:   []string{"10.2.1.0/24"},
					Apps:            []string{"unknown-app"},
					Action:          "PERMIT",
				},
			},
		},
	}
	_, err = it.restClient.SecurityV1().NetworkSecurityPolicy().Create(ctx, &sgpInv)
	Assert(c, (err != nil), "sg policy create with unknown-app did not fail")

	// verify agents dont have this policy
	time.Sleep(time.Millisecond * 100)
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sgpMeta := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: sgpInv.ObjectMeta,
			}
			nsgp, cerr := sn.agent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, sgpMeta)
			if cerr == nil {
				return false, nsgp
			}
		}
		return true, nil
	}, "Invalid Sg policy found in agent", "100ms", it.pollTimeout())

	// delete sg policy
	_, err = it.restClient.SecurityV1().NetworkSecurityPolicy().Delete(ctx, &sgp.ObjectMeta)
	AssertOk(c, err, "Error creating sg policy")

	/*
		// verify sgpolicy is removed from app
		AssertEventually(c, func() (bool, interface{}) {
			gapp, gerr := it.restClient.SecurityV1().App().Get(ctx, &icmpApp.ObjectMeta)
			if gerr != nil {
				return false, gerr
			}
			if len(gapp.Status.AttachedPolicies) != 0 {
				return false, gapp
			}

			return true, nil
		}, "App still has sgpolicy in status", "100ms", it.pollTimeout())

	*/

	// verify sg policy is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sgpMeta := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: sgp.ObjectMeta,
			}
			nsgp, cerr := sn.agent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, sgpMeta)
			if cerr == nil {
				return false, nsgp
			}
		}
		return true, nil
	}, "Sg policy still found in agent after deleting", "100ms", it.pollTimeout())

	// delete app
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &icmpApp.ObjectMeta)
	AssertOk(c, err, "Error creating ftp app")

	// verify app is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: icmpApp.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if cerr == nil {
				return false, napp
			}
		}
		return true, nil
	}, "App still found in agent after deleting", "100ms", it.pollTimeout())
}

func (it *veniceIntegSuite) TestDnsApp(c *C) {
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// DNS app
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "dns-app",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "udp",
					Ports:    "68",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: security.ALG_DNS.String(),
				Dns: &security.Dns{
					DropMultiQuestionPackets:   true,
					DropLargeDomainNamePackets: true,
					DropLongLabelPackets:       true,
					MaxMessageLength:           100,
					QueryResponseTimeout:       "60s",
				},
			},
		},
	}

	// create app
	_, err = it.restClient.SecurityV1().App().Create(ctx, &app)
	AssertOk(c, err, "Error creating app")

	// verify all agents have the app
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: app.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if (cerr != nil) || (napp[0].Name != app.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// verify ALG params
	for _, sn := range it.snics {
		sapp := netproto.App{
			TypeMeta:   api.TypeMeta{Kind: "App"},
			ObjectMeta: app.ObjectMeta,
		}
		napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp[0].Spec.AppIdleTimeout, app.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp[0].Spec.ALG.DNS.DropMultiQuestionPackets, app.Spec.ALG.Dns.DropMultiQuestionPackets, "invalid alg params")
		AssertEquals(c, napp[0].Spec.ALG.DNS.DropLargeDomainPackets, app.Spec.ALG.Dns.DropLargeDomainNamePackets, "invalid alg params")
		AssertEquals(c, napp[0].Spec.ALG.DNS.DropLongLabelPackets, app.Spec.ALG.Dns.DropLongLabelPackets, "invalid alg params")
		AssertEquals(c, napp[0].Spec.ALG.DNS.MaxMessageLength, app.Spec.ALG.Dns.MaxMessageLength, "invalid alg params")
		AssertEquals(c, napp[0].Spec.ALG.DNS.QueryResponseTimeout, app.Spec.ALG.Dns.QueryResponseTimeout, "invalid alg params")
	}

	// delete app
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &app.ObjectMeta)
	AssertOk(c, err, "Error deleting app")

	// verify app is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: app.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if cerr == nil {
				return false, napp
			}
		}
		return true, nil
	}, "App still found in agent after deleting", "100ms", it.pollTimeout())
}

func (it *veniceIntegSuite) TestFtpApp(c *C) {
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// FTP app
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "ftp-app",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "21",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: security.ALG_FTP.String(),
				Ftp: &security.Ftp{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	// create app
	_, err = it.restClient.SecurityV1().App().Create(ctx, &app)
	AssertOk(c, err, "Error creating app")

	// verify all agents have the app
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: app.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if (cerr != nil) || (napp[0].Name != app.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// verify ALG params
	for _, sn := range it.snics {
		sapp := netproto.App{
			TypeMeta:   api.TypeMeta{Kind: "App"},
			ObjectMeta: app.ObjectMeta,
		}
		napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp[0].Spec.AppIdleTimeout, app.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp[0].Spec.ALG.FTP.AllowMismatchIPAddresses, app.Spec.ALG.Ftp.AllowMismatchIPAddress, "invalid alg params")
	}

	// delete app
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &app.ObjectMeta)
	AssertOk(c, err, "Error deleting app")

	// verify app is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: app.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if cerr == nil {
				return false, napp
			}
		}
		return true, nil
	}, "App still found in agent after deleting", "100ms", it.pollTimeout())
}

func (it *veniceIntegSuite) TestTftpApp(c *C) {
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// TFTP app
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "tftp-app",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "udp",
					Ports:    "69",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: security.ALG_TFTP.String(),
			},
		},
	}

	// create app
	_, err = it.restClient.SecurityV1().App().Create(ctx, &app)
	AssertOk(c, err, "Error creating app")

	// verify all agents have the app
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: app.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if (cerr != nil) || (napp[0].Name != app.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// verify ALG params
	for _, sn := range it.snics {
		sapp := netproto.App{
			TypeMeta:   api.TypeMeta{Kind: "App"},
			ObjectMeta: app.ObjectMeta,
		}
		napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp[0].Spec.AppIdleTimeout, app.Spec.Timeout, "invalid alg params")
		Assert(c, napp[0].Spec.ALG.TFTP != nil, "invalid tftp alg params")
	}

	// delete app
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &app.ObjectMeta)
	AssertOk(c, err, "Error deleting app")

	// verify app is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: app.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if cerr == nil {
				return false, napp
			}
		}
		return true, nil
	}, "App still found in agent after deleting", "100ms", it.pollTimeout())
}

func (it *veniceIntegSuite) TestRtspApp(c *C) {
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// TFTP app
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "rtsp-app",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "554",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: security.ALG_RTSP.String(),
			},
		},
	}

	// create app
	_, err = it.restClient.SecurityV1().App().Create(ctx, &app)
	AssertOk(c, err, "Error creating app")

	// verify all agents have the app
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: app.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if (cerr != nil) || (napp[0].Name != app.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// verify ALG params
	for _, sn := range it.snics {
		sapp := netproto.App{
			TypeMeta:   api.TypeMeta{Kind: "App"},
			ObjectMeta: app.ObjectMeta,
		}
		napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp[0].Spec.AppIdleTimeout, app.Spec.Timeout, "invalid alg params")
		Assert(c, napp[0].Spec.ALG.RTSP != nil, "invalid rtsp alg params")
	}

	// delete app
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &app.ObjectMeta)
	AssertOk(c, err, "Error deleting app")

	// verify app is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: app.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if cerr == nil {
				return false, napp
			}
		}
		return true, nil
	}, "App still found in agent after deleting", "100ms", it.pollTimeout())
}

func (it *veniceIntegSuite) TestRPCApp(c *C) {
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// SunRPC app
	pgmID := "8000"
	sunapp := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "sunrpc-app",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "5000",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: security.ALG_SunRPC.String(),
				Sunrpc: []*security.Sunrpc{
					{
						ProgramID: pgmID,
						Timeout:   "2s",
					},
				},
			},
		},
	}
	msapp := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "msrpc-app",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "5000",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: security.ALG_MSRPC.String(),
				Msrpc: []*security.Msrpc{
					{
						ProgramUUID: pgmID,
						Timeout:     "2s",
					},
				},
			},
		},
	}

	// create app
	_, err = it.restClient.SecurityV1().App().Create(ctx, &sunapp)
	AssertOk(c, err, "Error creating app")
	_, err = it.restClient.SecurityV1().App().Create(ctx, &msapp)
	AssertOk(c, err, "Error creating app")

	// verify all agents have the app
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: sunapp.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if (cerr != nil) || (napp[0].Name != sunapp.Name) {
				return false, napp
			}

			sapp = netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: msapp.ObjectMeta,
			}
			napp, cerr = sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if (cerr != nil) || (napp[0].Name != msapp.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// verify ALG params
	for _, sn := range it.snics {
		sapp := netproto.App{
			TypeMeta:   api.TypeMeta{Kind: "App"},
			ObjectMeta: sunapp.ObjectMeta,
		}
		napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp[0].Spec.AppIdleTimeout, sunapp.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp[0].Spec.ALG.SUNRPC[0].ProgramID, pgmID, "invalid alg params")

		sapp = netproto.App{
			TypeMeta:   api.TypeMeta{Kind: "App"},
			ObjectMeta: msapp.ObjectMeta,
		}
		napp, cerr = sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)

		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp[0].Spec.AppIdleTimeout, msapp.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp[0].Spec.ALG.MSRPC[0].ProgramID, pgmID, "invalid alg params")
	}

	// delete app
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &sunapp.ObjectMeta)
	AssertOk(c, err, "Error deleting app")
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &msapp.ObjectMeta)
	AssertOk(c, err, "Error deleting app")

	// verify app is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: sunapp.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if cerr == nil {
				return false, napp
			}

			sapp = netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: msapp.ObjectMeta,
			}
			napp, cerr = sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if cerr == nil {
				return false, napp
			}
		}
		return true, nil
	}, "App still found in agent after deleting", "100ms", it.pollTimeout())
}

// TestFirewallFtp validates FTP alg operations
// FIXME: revisit this once netagent datapath can fully handle app object
func (it *veniceIntegSuite) TestFirewallFtp(c *C) {
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// app
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "ftp",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "21",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: security.ALG_FTP.String(),
				Ftp: &security.Ftp{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	// create app
	_, err = it.restClient.SecurityV1().App().Create(ctx, &app)
	AssertOk(c, err, "Error creating ftp app")

	// verify all agents have the app
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: app.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if (cerr != nil) || (napp[0].Name != app.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// create an sg policy using the app
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "test-sgpolicy",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					FromIPAddresses: []string{"10.1.1.0/24"},
					ToIPAddresses:   []string{"10.2.1.0/24"},
					Apps:            []string{"ftp"},
					Action:          security.SGRule_PERMIT.String(),
				},
				{
					FromIPAddresses: []string{"10.1.1.0/24"},
					ToIPAddresses:   []string{"10.2.1.0/24"},
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "tcp",
							Ports:    "8000",
						},
					},
					Action: security.SGRule_PERMIT.String(),
				},
			},
		},
	}
	_, err = it.restClient.SecurityV1().NetworkSecurityPolicy().Create(ctx, &sgp)
	AssertOk(c, err, "Error creating sg policy")

	// verify all agents have the sg policy
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sgpMeta := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: sgp.ObjectMeta,
			}
			nsgp, cerr := sn.agent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, sgpMeta)
			if (cerr != nil) || (nsgp[0].Name != sgp.Name) {
				return false, nsgp
			}
		}
		return true, nil
	}, "Sg policy not found in agent", "100ms", it.pollTimeout())

	// verify datapath has the sg policy
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sgpMeta := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: sgp.ObjectMeta,
			}
			nsgp, cerr := sn.agent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, sgpMeta)
			if cerr != nil {
				return false, nil
			}
			if len(sgp.Spec.Rules) != len(nsgp[0].Spec.Rules) {
				return false, nsgp[0].Spec.Rules
			}

			for _, veniceRule := range sgp.Spec.Rules {
				for _, naplesRule := range nsgp[0].Spec.Rules {
					if veniceRule.Action != strings.ToLower(naplesRule.Action) {
						// FIXME: we need to validate rules more add unit tests around conversion routines
						return false, naplesRule
					}
				}
			}
		}
		return true, nil
	}, "Sg policy incorrect in datapath", "100ms", it.pollTimeout())

	// delete sg policy
	_, err = it.restClient.SecurityV1().NetworkSecurityPolicy().Delete(ctx, &sgp.ObjectMeta)
	AssertOk(c, err, "Error creating sg policy")

	// verify sg policy is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sgpMeta := netproto.NetworkSecurityPolicy{
				TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
				ObjectMeta: sgp.ObjectMeta,
			}
			nsgp, cerr := sn.agent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, sgpMeta)
			if cerr == nil {
				return false, nsgp
			}
		}
		return true, nil
	}, "Sg policy still found in agent after deleting", "100ms", it.pollTimeout())

	// delete app
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &app.ObjectMeta)
	AssertOk(c, err, "Error creating ftp app")

	// verify app is removed from all agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, sn := range it.snics {
			sapp := netproto.App{
				TypeMeta:   api.TypeMeta{Kind: "App"},
				ObjectMeta: app.ObjectMeta,
			}
			napp, cerr := sn.agent.PipelineAPI.HandleApp(agentTypes.Get, sapp)
			if cerr == nil {
				return false, napp
			}
		}
		return true, nil
	}, "App still found in agent after deleting", "100ms", it.pollTimeout())
}
