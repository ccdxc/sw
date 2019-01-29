// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"fmt"
	"time"

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
			Name:      "testProfile",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.FirewallProfileSpec{
			SessionIdleTimeout:        "3m",
			TCPConnectionSetupTimeout: "3m",
			TCPCloseTimeout:           "3m",
			TCPHalfClosedTimeout:      "3m",
			TCPDropTimeout:            "3m",
			UDPDropTimeout:            "3m",
			ICMPDropTimeout:           "3m",
			DropTimeout:               "3m",
			TcpTimeout:                "3m",
			UdpTimeout:                "3m",
			IcmpTimeout:               "3m",
		},
	}

	// create firewall profile
	_, err = it.restClient.SecurityV1().FirewallProfile().Create(ctx, &fwp)
	AssertOk(c, err, "Error creating firewall profile")

	// verify policy gets created in agent
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			secp, cerr := ag.NetworkAgent.FindSecurityProfile(fwp.ObjectMeta)
			if (cerr != nil) || (secp.Name != fwp.Name) {
				return false, secp
			}
		}
		return true, nil
	}, "Firewall profile not found in agent", "100ms", it.pollTimeout())

	// verify all the parameters
	for _, ag := range it.agents {
		secp, cerr := ag.NetworkAgent.FindSecurityProfile(fwp.ObjectMeta)
		AssertOk(c, cerr, "Security profile not found in agent")
		AssertEquals(c, secp.Spec.Timeouts.SessionIdle, fwp.Spec.SessionIdleTimeout, "incorrect params")
		AssertEquals(c, secp.Spec.Timeouts.TCP, fwp.Spec.TcpTimeout, "incorrect params")
		AssertEquals(c, secp.Spec.Timeouts.TCPDrop, fwp.Spec.TCPDropTimeout, "incorrect params")
		AssertEquals(c, secp.Spec.Timeouts.TCPConnectionSetup, fwp.Spec.TCPConnectionSetupTimeout, "incorrect params")
		AssertEquals(c, secp.Spec.Timeouts.TCPClose, fwp.Spec.TCPCloseTimeout, "incorrect params")
		AssertEquals(c, secp.Spec.Timeouts.TCPHalfClose, fwp.Spec.TCPHalfClosedTimeout, "incorrect params")
		AssertEquals(c, secp.Spec.Timeouts.Drop, fwp.Spec.DropTimeout, "incorrect params")
		AssertEquals(c, secp.Spec.Timeouts.UDP, fwp.Spec.UdpTimeout, "incorrect params")
		AssertEquals(c, secp.Spec.Timeouts.UDPDrop, fwp.Spec.UDPDropTimeout, "incorrect params")
		AssertEquals(c, secp.Spec.Timeouts.ICMP, fwp.Spec.IcmpTimeout, "incorrect params")
		AssertEquals(c, secp.Spec.Timeouts.ICMPDrop, fwp.Spec.ICMPDropTimeout, "incorrect params")
	}

	// change conn track and session timeout
	fwp.Spec.SessionIdleTimeout = "5m"
	_, err = it.restClient.SecurityV1().FirewallProfile().Update(ctx, &fwp)
	AssertOk(c, err, "Error updating firewall profile")

	// verify params got updated in agent
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			secp, cerr := ag.NetworkAgent.FindSecurityProfile(fwp.ObjectMeta)
			if (cerr != nil) || (secp.Spec.Timeouts.SessionIdle != fwp.Spec.SessionIdleTimeout) {
				return false, secp
			}
		}
		return true, nil
	}, "Firewall profile params incorrect in agent", "100ms", it.pollTimeout())

	// delete firewall profile
	_, err = it.restClient.SecurityV1().FirewallProfile().Delete(ctx, &fwp.ObjectMeta)
	AssertOk(c, err, "Error deleting firewall profile")

	// verify firewall profile is deleted from agent
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			secp, cerr := ag.NetworkAgent.FindSecurityProfile(fwp.ObjectMeta)
			if cerr == nil {
				return false, secp
			}
		}
		return true, nil
	}, "Firewall profile still found in agent after deleting", "100ms", it.pollTimeout())
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
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: "ICMP",
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
		for _, ag := range it.agents {
			napp, cerr := ag.NetworkAgent.FindApp(icmpApp.ObjectMeta)
			if (cerr != nil) || (napp.Name != icmpApp.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// verify ALG params
	for _, ag := range it.agents {
		napp, cerr := ag.NetworkAgent.FindApp(icmpApp.ObjectMeta)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp.Spec.AppIdleTimeout, icmpApp.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp.Spec.ALGType, icmpApp.Spec.ALG.Type, "invalid alg params")
		AssertEquals(c, fmt.Sprintf("%d", napp.Spec.ALG.ICMP.Type), icmpApp.Spec.ALG.Icmp.Type, "invalid alg params")
		AssertEquals(c, fmt.Sprintf("%d", napp.Spec.ALG.ICMP.Code), icmpApp.Spec.ALG.Icmp.Code, "invalid alg params")

	}

	// create an sg policy using the app
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "test-sgpolicy",
		},
		Spec: security.SGPolicySpec{
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
	_, err = it.restClient.SecurityV1().SGPolicy().Create(ctx, &sgp)
	AssertOk(c, err, "Error creating sg policy")

	// verify agents have this policy and params are correct
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			nsgp, cerr := ag.NetworkAgent.FindSGPolicy(sgp.ObjectMeta)
			if cerr != nil {
				return false, cerr
			}
			if len(nsgp.Spec.Rules) != len(sgp.Spec.Rules) {
				return false, nsgp
			}
		}
		return true, nil
	}, "Sg policy not found in agent or params didnt match", "100ms", it.pollTimeout())

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

	// verify creating agpolicy with unknown app fails
	sgpInv := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "invalid-sgpolicy",
		},
		Spec: security.SGPolicySpec{
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
	_, err = it.restClient.SecurityV1().SGPolicy().Create(ctx, &sgpInv)
	// FIXME: this check should be reversed once we have named ref validation
	Assert(c, (err == nil), "sg policy create with unknown-app failed")

	// verify agents dont have this policy
	time.Sleep(time.Millisecond * 100)
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			nsgp, cerr := ag.NetworkAgent.FindSGPolicy(sgpInv.ObjectMeta)
			if cerr == nil {
				return false, nsgp
			}
		}
		return true, nil
	}, "Invalid Sg policy found in agent", "100ms", it.pollTimeout())

	// delete sg policy
	_, err = it.restClient.SecurityV1().SGPolicy().Delete(ctx, &sgp.ObjectMeta)
	AssertOk(c, err, "Error creating sg policy")

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

	// verify sg policy is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			nsgp, cerr := ag.NetworkAgent.FindSGPolicy(sgp.ObjectMeta)
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
		for _, ag := range it.agents {
			napp, cerr := ag.NetworkAgent.FindApp(icmpApp.ObjectMeta)
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

	// ICMP app
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
				Type: "DNS",
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
		for _, ag := range it.agents {
			napp, cerr := ag.NetworkAgent.FindApp(app.ObjectMeta)
			if (cerr != nil) || (napp.Name != app.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// verify ALG params
	for _, ag := range it.agents {
		napp, cerr := ag.NetworkAgent.FindApp(app.ObjectMeta)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp.Spec.AppIdleTimeout, app.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp.Spec.ALGType, app.Spec.ALG.Type, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.DNS.DropMultiQuestionPackets, app.Spec.ALG.Dns.DropMultiQuestionPackets, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.DNS.DropLargeDomainPackets, app.Spec.ALG.Dns.DropLargeDomainNamePackets, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.DNS.DropLongLabelPackets, app.Spec.ALG.Dns.DropLongLabelPackets, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.DNS.MaxMessageLength, app.Spec.ALG.Dns.MaxMessageLength, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.DNS.QueryResponseTimeout, app.Spec.ALG.Dns.QueryResponseTimeout, "invalid alg params")
	}

	// delete app
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &app.ObjectMeta)
	AssertOk(c, err, "Error deleting app")

	// verify app is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			napp, cerr := ag.NetworkAgent.FindApp(app.ObjectMeta)
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

	// ICMP app
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
				Type: "FTP",
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
		for _, ag := range it.agents {
			napp, cerr := ag.NetworkAgent.FindApp(app.ObjectMeta)
			if (cerr != nil) || (napp.Name != app.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// verify ALG params
	for _, ag := range it.agents {
		napp, cerr := ag.NetworkAgent.FindApp(app.ObjectMeta)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp.Spec.AppIdleTimeout, app.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp.Spec.ALGType, app.Spec.ALG.Type, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.FTP.AllowMismatchIPAddresses, app.Spec.ALG.Ftp.AllowMismatchIPAddress, "invalid alg params")
	}

	// delete app
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &app.ObjectMeta)
	AssertOk(c, err, "Error deleting app")

	// verify app is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			napp, cerr := ag.NetworkAgent.FindApp(app.ObjectMeta)
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

	// ICMP app
	pgmID := 8000
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
				Type: "SunRPC",
				Sunrpc: []*security.Sunrpc{
					&security.Sunrpc{
						ProgramID: fmt.Sprintf("%d", pgmID),
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
				Type: "MSRPC",
				Msrpc: []*security.Msrpc{
					&security.Msrpc{
						ProgramUUID: fmt.Sprintf("%d", pgmID),
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
		for _, ag := range it.agents {
			napp, cerr := ag.NetworkAgent.FindApp(sunapp.ObjectMeta)
			if (cerr != nil) || (napp.Name != sunapp.Name) {
				return false, napp
			}
			napp, cerr = ag.NetworkAgent.FindApp(msapp.ObjectMeta)
			if (cerr != nil) || (napp.Name != msapp.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// verify ALG params
	for _, ag := range it.agents {
		napp, cerr := ag.NetworkAgent.FindApp(sunapp.ObjectMeta)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp.Spec.AppIdleTimeout, sunapp.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp.Spec.ALGType, sunapp.Spec.ALG.Type, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.SUNRPC[0].ProgramID, uint32(pgmID), "invalid alg params")

		napp, cerr = ag.NetworkAgent.FindApp(msapp.ObjectMeta)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp.Spec.AppIdleTimeout, msapp.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp.Spec.ALGType, msapp.Spec.ALG.Type, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.MSRPC[0].ProgramID, uint32(pgmID), "invalid alg params")
	}

	// delete app
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &sunapp.ObjectMeta)
	AssertOk(c, err, "Error deleting app")
	_, err = it.restClient.SecurityV1().App().Delete(ctx, &msapp.ObjectMeta)
	AssertOk(c, err, "Error deleting app")

	// verify app is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			napp, cerr := ag.NetworkAgent.FindApp(sunapp.ObjectMeta)
			if cerr == nil {
				return false, napp
			}
			napp, cerr = ag.NetworkAgent.FindApp(msapp.ObjectMeta)
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
				Type: "FTP",
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
		for _, ag := range it.agents {
			napp, cerr := ag.NetworkAgent.FindApp(app.ObjectMeta)
			if (cerr != nil) || (napp.Name != app.Name) {
				return false, napp
			}
		}
		return true, nil
	}, "App not found in agent", "100ms", it.pollTimeout())

	// create an sg policy using the app
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "test-sgpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					FromIPAddresses: []string{"10.1.1.0/24"},
					ToIPAddresses:   []string{"10.2.1.0/24"},
					Apps:            []string{"ftp"},
					Action:          "PERMIT",
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
					Action: "PERMIT",
				},
			},
		},
	}
	_, err = it.restClient.SecurityV1().SGPolicy().Create(ctx, &sgp)
	AssertOk(c, err, "Error creating sg policy")

	// verify all agents have the sg policy
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			nsgp, cerr := ag.NetworkAgent.FindSGPolicy(sgp.ObjectMeta)
			if (cerr != nil) || (nsgp.Name != sgp.Name) {
				return false, nsgp
			}
		}
		return true, nil
	}, "Sg policy not found in agent", "100ms", it.pollTimeout())

	// verify datapath has the sg policy
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			nsgp, cerr := ag.NetworkAgent.FindSGPolicy(sgp.ObjectMeta)
			if cerr != nil {
				return false, nil
			}
			if len(sgp.Spec.Rules) != len(nsgp.Spec.Rules) {
				return false, nsgp.Spec.Rules
			}

			for _, veniceRule := range sgp.Spec.Rules {
				for _, naplesRule := range nsgp.Spec.Rules {
					if veniceRule.Action != naplesRule.Action {
						// FIXME: we need to validate rules more add unit tests around conversion routines
						return false, naplesRule
					}
				}
			}
		}
		return true, nil
	}, "Sg policy incorrect in datapath", "100ms", it.pollTimeout())

	// delete sg policy
	_, err = it.restClient.SecurityV1().SGPolicy().Delete(ctx, &sgp.ObjectMeta)
	AssertOk(c, err, "Error creating sg policy")

	// verify sg policy is gone from agents
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			nsgp, cerr := ag.NetworkAgent.FindSGPolicy(sgp.ObjectMeta)
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
		for _, ag := range it.agents {
			napp, cerr := ag.NetworkAgent.FindApp(app.ObjectMeta)
			if cerr == nil {
				return false, napp
			}
		}
		return true, nil
	}, "App still found in agent after deleting", "100ms", it.pollTimeout())
}
