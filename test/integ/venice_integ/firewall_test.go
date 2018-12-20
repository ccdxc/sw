// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"context"
	"fmt"
	"time"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestNpmFirewallProfile tests firewall profile create/update/delete operations
func (it *veniceIntegSuite) TestFirewallProfile(c *C) {
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), integTestAPIGWURL, it.userCred)
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

			IPNormalizationEnable:   true,
			TCPNormalizationEnable:  true,
			ICMPNormalizationEnable: true,
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
	}, "Firewall profile not found in agent", "100ms", "10s")

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
		AssertEquals(c, secp.Spec.EnableConnectionTracking, true, "incorrect params")
		AssertEquals(c, secp.Spec.EnableTCPNormalization, fwp.Spec.TCPNormalizationEnable, "incorrect params")
		AssertEquals(c, secp.Spec.EnableIPNormalization, fwp.Spec.IPNormalizationEnable, "incorrect params")
		AssertEquals(c, secp.Spec.EnableICMPNormalization, fwp.Spec.ICMPNormalizationEnable, "incorrect params")
	}

	// change conn track and session timeout
	fwp.Spec.DisableConnTrack = true
	fwp.Spec.SessionIdleTimeout = "5m"
	_, err = it.restClient.SecurityV1().FirewallProfile().Update(ctx, &fwp)
	AssertOk(c, err, "Error updating firewall profile")

	// verify params got updated in agent
	AssertEventually(c, func() (bool, interface{}) {
		for _, ag := range it.agents {
			secp, cerr := ag.NetworkAgent.FindSecurityProfile(fwp.ObjectMeta)
			if (cerr != nil) || (secp.Spec.EnableConnectionTracking != false) ||
				(secp.Spec.Timeouts.SessionIdle != fwp.Spec.SessionIdleTimeout) {
				return false, secp
			}
		}
		return true, nil
	}, "Firewall profile params incorrect in agent", "100ms", "10s")

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
	}, "Firewall profile still found in agent after deleting", "100ms", "10s")
}

func (it *veniceIntegSuite) TestIcmpApp(c *C) {
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), integTestAPIGWURL, it.userCred)
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
				IcmpAlg: &security.IcmpAlg{
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
	}, "App not found in agent", "100ms", "10s")

	// verify ALG params
	for _, ag := range it.agents {
		napp, cerr := ag.NetworkAgent.FindApp(icmpApp.ObjectMeta)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp.Spec.AppTimeout, icmpApp.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp.Spec.ALGType, icmpApp.Spec.ALG.Type, "invalid alg params")
		AssertEquals(c, fmt.Sprintf("%d", napp.Spec.ALG.ICMP.Type), icmpApp.Spec.ALG.IcmpAlg.Type, "invalid alg params")
		AssertEquals(c, fmt.Sprintf("%d", napp.Spec.ALG.ICMP.Code), icmpApp.Spec.ALG.IcmpAlg.Code, "invalid alg params")

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
	}, "Invalid Sg policy found in agent", "100ms", "10s")

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
	}, "Sg policy still found in agent after deleting", "100ms", "10s")

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
	}, "App still found in agent after deleting", "100ms", "10s")
}

func (it *veniceIntegSuite) TestDnsApp(c *C) {
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), integTestAPIGWURL, it.userCred)
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
				DnsAlg: &security.DnsAlg{
					DropMultiQuestionPackets:   true,
					DropLargeDomainNamePackets: true,
					DropLongLabelPackets:       true,
					DropMultiZonePackets:       true,
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
	}, "App not found in agent", "100ms", "10s")

	// verify ALG params
	for _, ag := range it.agents {
		napp, cerr := ag.NetworkAgent.FindApp(app.ObjectMeta)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp.Spec.AppTimeout, app.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp.Spec.ALGType, app.Spec.ALG.Type, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.DNS.DropMultiQuestionPackets, app.Spec.ALG.DnsAlg.DropMultiQuestionPackets, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.DNS.DropLargeDomainPackets, app.Spec.ALG.DnsAlg.DropLargeDomainNamePackets, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.DNS.DropLongLabelPackets, app.Spec.ALG.DnsAlg.DropLongLabelPackets, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.DNS.DropMultiZonePackets, app.Spec.ALG.DnsAlg.DropMultiZonePackets, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.DNS.MaxMessageLength, app.Spec.ALG.DnsAlg.MaxMessageLength, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.DNS.QueryResponseTimeout, app.Spec.ALG.DnsAlg.QueryResponseTimeout, "invalid alg params")
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
	}, "App still found in agent after deleting", "100ms", "10s")
}

func (it *veniceIntegSuite) TestFtpApp(c *C) {
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), integTestAPIGWURL, it.userCred)
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
				FtpAlg: &security.FtpAlg{
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
	}, "App not found in agent", "100ms", "10s")

	// verify ALG params
	for _, ag := range it.agents {
		napp, cerr := ag.NetworkAgent.FindApp(app.ObjectMeta)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp.Spec.AppTimeout, app.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp.Spec.ALGType, app.Spec.ALG.Type, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.FTP.AllowMismatchIPAddresses, app.Spec.ALG.FtpAlg.AllowMismatchIPAddress, "invalid alg params")
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
	}, "App still found in agent after deleting", "100ms", "10s")
}

func (it *veniceIntegSuite) TestRPCApp(c *C) {
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), integTestAPIGWURL, it.userCred)
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
				SunrpcAlg: &security.SunrpcAlg{
					ProgramID: fmt.Sprintf("%d", pgmID),
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
				MsrpcAlg: &security.MsrpcAlg{
					ProgramUUID: fmt.Sprintf("%d", pgmID),
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
	}, "App not found in agent", "100ms", "10s")

	// verify ALG params
	for _, ag := range it.agents {
		napp, cerr := ag.NetworkAgent.FindApp(sunapp.ObjectMeta)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp.Spec.AppTimeout, sunapp.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp.Spec.ALGType, sunapp.Spec.ALG.Type, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.SUNRPC.ProgramID, uint32(pgmID), "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.SUNRPC.MapEntryTimeout, sunapp.Spec.Timeout, "invalid alg params")

		napp, cerr = ag.NetworkAgent.FindApp(msapp.ObjectMeta)
		AssertOk(c, cerr, "App not found in agent")
		AssertEquals(c, napp.Spec.AppTimeout, msapp.Spec.Timeout, "invalid alg params")
		AssertEquals(c, napp.Spec.ALGType, msapp.Spec.ALG.Type, "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.MSRPC.ProgramID, uint32(pgmID), "invalid alg params")
		AssertEquals(c, napp.Spec.ALG.MSRPC.MapEntryTimeout, msapp.Spec.Timeout, "invalid alg params")
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
	}, "App still found in agent after deleting", "100ms", "10s")
}

// TestFirewallFtpAlg validates FTP alg operations
// FIXME: revisit this once netagent datapath can fully handle app object
func (it *veniceIntegSuite) TestFirewallFtpAlg(c *C) {
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), integTestAPIGWURL, it.userCred)
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
				FtpAlg: &security.FtpAlg{
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
	}, "App not found in agent", "100ms", "10s")

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
	}, "Sg policy not found in agent", "100ms", "10s")

	// verify datapath has the sg policy
	AssertEventually(c, func() (bool, interface{}) {
		for _, dp := range it.datapaths {
			nsgp, cerr := dp.FindSGPolicy(sgp.ObjectMeta)
			if cerr != nil {
				return false, nil
			}
			for _, req := range nsgp.Request {
				if len(req.Rule) != len(sgp.Spec.Rules) {
					return false, req
				}
				found := false
				for _, rule := range req.Rule {
					// FIXME: we need to validate rules more
					if rule.Match.Protocol == halproto.IPProtocol_IPPROTO_TCP {
						found = true
					}
				}
				if !found {
					return false, req
				}
			}
		}
		return true, nil
	}, "Sg policy incorrect in datapath", "100ms", "10s")

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
	}, "Sg policy still found in agent after deleting", "100ms", "10s")

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
	}, "App still found in agent after deleting", "100ms", "10s")
}
