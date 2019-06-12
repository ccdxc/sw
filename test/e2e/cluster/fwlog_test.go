package cluster

import (
	"context"
	"encoding/json"
	"fmt"
	"net"
	"os"
	"strings"
	"time"

	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/tmagent/state/fwgen/fwevent"

	"github.com/jeromer/syslogparser"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/venice/ctrler/tpm"
	"github.com/pensando/sw/venice/utils/syslog"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
)

var _ = Describe("fwlog policy tests", func() {
	Context("fwlog policy CRUD tests", func() {
		var fwlogClient monitoring.MonitoringV1FwlogPolicyInterface
		var cancel context.CancelFunc
		var localAddr net.IP
		var apiGwAddr string
		type testCollector struct {
			proto  string
			addr   string
			port   string
			format string
			ch     chan syslogparser.LogParts
		}

		testFwSpecList := make([]monitoring.FwlogPolicySpec, tpm.MaxPolicyPerVrf)
		syslogCollectors := []testCollector{}

		AfterEach(func() {
			By("cleanup fwlog policy")
			ctx := ts.tu.NewLoggedInContext(context.Background())
			if testFwSpecList, err := fwlogClient.List(ctx, &api.ListWatchOptions{}); err == nil {
				for i := range testFwSpecList {
					By(fmt.Sprintf("delete %v", testFwSpecList[i].ObjectMeta))
					fwlogClient.Delete(ctx, &testFwSpecList[i].ObjectMeta)
				}
			}
			cancel()
			syslogCollectors = []testCollector{}
		})

		BeforeEach(func() {
			pctx, cfunc := context.WithCancel(context.Background())
			cancel = cfunc
			// pick the local ip from the route to naples
			conn, err := net.Dial("udp", ts.tu.NaplesNodeIPs[0]+":80")
			Expect(err).Should(BeNil())

			localAddr = conn.LocalAddr().(*net.UDPAddr).IP
			By(fmt.Sprintf("using local ip addr %v", localAddr.String()))

			protos := []string{"udp", "tcp"}
			formats := []string{monitoring.MonitoringExportFormat_SYSLOG_BSD.String(), monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String()}

			syslogCollectors = []testCollector{}

			for i := 0; i < tpm.MaxPolicyPerVrf; i++ {
				proto := protos[i%len(protos)]
				format := formats[i/4]
				addr, ch, err := syslog.Server(pctx, localAddr.String()+":0", format, proto)
				Expect(err).Should(BeNil())
				s := strings.Split(addr, ":")
				Expect(len(s)).To(Equal(2))

				syslogCollectors = append(syslogCollectors, testCollector{
					proto:  proto,
					format: format,
					addr:   s[0],
					port:   s[1],
					ch:     ch,
				})
				By(fmt.Sprintf("[%d] syslog server %v://%v %v created", i, proto, addr, format))
			}

			for i := 0; i < tpm.MaxPolicyPerVrf; i++ {
				testFwSpecList[i] = monitoring.FwlogPolicySpec{
					VrfName: globals.DefaultVrf,
					Targets: []monitoring.ExportConfig{
						{
							Destination: syslogCollectors[i].addr,
							Transport:   syslogCollectors[i].proto + "/" + syslogCollectors[i].port,
						},
					},
					Format: syslogCollectors[i].format,
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
				}
			}

			apiGwAddr = ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			restSvc, err := apiclient.NewRestAPIClient(apiGwAddr)
			Expect(err).ShouldNot(HaveOccurred())
			fwlogClient = restSvc.MonitoringV1().FwlogPolicy()

			// delete all
			By("cleanup fwlog policy")
			ctx := ts.tu.NewLoggedInContext(context.Background())
			if testFwSpecList, err := fwlogClient.List(ctx, &api.ListWatchOptions{}); err == nil {
				for i := range testFwSpecList {
					By(fmt.Sprintf("delete %v", testFwSpecList[i].ObjectMeta))
					fwlogClient.Delete(ctx, &testFwSpecList[i].ObjectMeta)
				}
			}
		})

		It("Should create/update/delete fwlog policy", func() {
			ctx := ts.tu.NewLoggedInContext(context.Background())

			By("create fwlog Policy")
			for i := range testFwSpecList {
				fwPolicy := &monitoring.FwlogPolicy{
					TypeMeta: api.TypeMeta{
						Kind: "fwLogPolicy",
					},
					ObjectMeta: api.ObjectMeta{
						Name:      fmt.Sprintf("fwpolicy-%d", i),
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
					},
					Spec: testFwSpecList[i],
				}
				_, err := fwlogClient.Create(ctx, fwPolicy)
				Expect(err).ShouldNot(HaveOccurred())
			}

			// use token api to get NAPLES access credentials
			nodeAuthFile, err := utils.GetNodeAuthTokenTempFile(ctx, apiGwAddr, []string{"*"})
			Expect(err).ShouldNot(HaveOccurred())
			defer os.Remove(nodeAuthFile)

			Eventually(func() error {
				By("verify fwlog policy in Venice")
				pl, err := fwlogClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				if len(pl) != len(testFwSpecList) {
					By(fmt.Sprintf("received fwlog policy from venice %+v", pl))
					return fmt.Errorf("invalid number of policy in Venice, got %v expected %+v", len(pl), len(testFwSpecList))
				}

				for _, naples := range ts.tu.NaplesNodes {
					By(fmt.Sprintf("verify fwlog policy in %v", naples))
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/fwlog/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					var naplesPol []monitoring.FwlogPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						By(fmt.Sprintf("received fwlog policy from naples: %v, %+v", naples, st))
						return err
					}

					if len(naplesPol) != len(testFwSpecList) {
						By(fmt.Sprintf("received fwlog policy from naples: %v, %v", naples, naplesPol))
						return fmt.Errorf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), len(testFwSpecList))
					}
				}
				return nil
			}, 120, 2).Should(BeNil(), "failed to find fwlog policy")

			By("Update fwlog Policy")
			for i := range testFwSpecList {
				fwPolicy := &monitoring.FwlogPolicy{
					TypeMeta: api.TypeMeta{
						Kind: "fwLogPolicy",
					},
					ObjectMeta: api.ObjectMeta{
						Name:      fmt.Sprintf("fwpolicy-%d", i),
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
					},
					Spec: testFwSpecList[len(testFwSpecList)-i-1],
				}
				_, err := fwlogClient.Update(ctx, fwPolicy)
				Expect(err).Should(BeNil())
			}

			Eventually(func() error {
				By("Verify fwlog policy from Venice")
				pl, err := fwlogClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				if len(pl) != len(testFwSpecList) {
					By(fmt.Sprintf("received fwlog policy from venice %+v", pl))
					return fmt.Errorf("invalid number of policy in Venice, got %v expected %+v", len(pl), len(testFwSpecList))
				}

				for _, naples := range ts.tu.NaplesNodes {
					By(fmt.Sprintf("verify fwlog policy in %v", naples))
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/fwlog/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					var naplesPol []monitoring.FwlogPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						By(fmt.Sprintf("received fwlog policy from naples: %v, %+v", naples, st))
						return err
					}

					if len(naplesPol) != len(testFwSpecList) {
						By(fmt.Sprintf("received fwlog policy from naples: %v, %v", naples, naplesPol))
						return fmt.Errorf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), len(testFwSpecList))
					}
				}
				return nil
			}, 120, 2).Should(BeNil(), "failed to find fwlog policy")

			By("Delete fwlog policy")
			for i := range testFwSpecList {

				objMeta := &api.ObjectMeta{
					Name:      fmt.Sprintf("fwpolicy-%d", i),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}

				_, err := fwlogClient.Delete(ctx, objMeta)
				Expect(err).ShouldNot(HaveOccurred())
			}

			Eventually(func() error {
				By("Verify fwlog policy from Venice")
				pl, err := fwlogClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				if len(pl) != 0 {
					By(fmt.Sprintf("policy exists after delete, %+v", pl))
					return fmt.Errorf("policy exists after delete, %+v", pl)
				}

				for _, naples := range ts.tu.NaplesNodes {
					By(fmt.Sprintf("verify fwlog policy in %v", naples))

					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/fwlog/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					var naplesPol []monitoring.FwlogPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						By(fmt.Sprintf("received fwlog policy from naples:%v,  %+v", naples, st))
						return err
					}

					if len(naplesPol) != 0 {
						By(fmt.Sprintf("received fwlog policy from naples:%v,  %+v", naples, naplesPol))
						return fmt.Errorf("invalid number of policy in %v, got %d, expected 0", naples, len(naplesPol))
					}
				}
				return nil
			}, 120, 2).Should(BeNil(), "failed to verify fwlog policy")
		})

		// check collectors
		It("Should receive syslog in Collector", func() {
			Skip("skip to debug CI failures")
			ctx := ts.tu.NewLoggedInContext(context.Background())

			// use token api to get NAPLES access credentials
			nodeAuthFile, err := utils.GetNodeAuthTokenTempFile(ctx, apiGwAddr, []string{"*"})
			Expect(err).ShouldNot(HaveOccurred())
			defer os.Remove(nodeAuthFile)

			By(fmt.Sprintf("create %v fwlog Policy", len(testFwSpecList)))
			for i := range testFwSpecList {
				fwPolicy := &monitoring.FwlogPolicy{
					TypeMeta: api.TypeMeta{
						Kind: "fwLogPolicy",
					},
					ObjectMeta: api.ObjectMeta{
						Name:      fmt.Sprintf("fwpolicy-%d", i),
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
					},
					Spec: testFwSpecList[i],
				}
				_, err := fwlogClient.Create(ctx, fwPolicy)
				Expect(err).ShouldNot(HaveOccurred())
			}

			Eventually(func() error {
				By("verify fwlog policy in Venice")
				pl, err := fwlogClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				if len(pl) != len(testFwSpecList) {
					By(fmt.Sprintf("received fwlog policy from venice %+v", pl))
					return fmt.Errorf("invalid number of policy in Venice, got %v expected %v, %+v", len(pl), len(testFwSpecList), testFwSpecList)
				}

				for _, naples := range ts.tu.NaplesNodes {
					By(fmt.Sprintf("verify fwlog policy in %v", naples))
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/fwlog/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					var naplesPol []monitoring.FwlogPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						By(fmt.Sprintf("received fwlog policy from naples: %v, %+v", naples, st))
						return err
					}

					if len(naplesPol) != len(testFwSpecList) {
						By(fmt.Sprintf("received fwlog policy from naples: %v, %v", naples, naplesPol))
						return fmt.Errorf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), len(testFwSpecList))
					}
				}
				return nil
			}, 120, 2).Should(BeNil(), "failed to find fwlog policy")

			By(fmt.Sprintf("configured fwlog policy %+v", testFwSpecList))

			// wait to connect to collectors
			time.Sleep(time.Second * 60)
			for _, naples := range ts.tu.NaplesNodes {
				st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/vrfs/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
				var vrfs []netproto.Vrf
				err := json.Unmarshal([]byte(st), &vrfs)
				Expect(err).Should(BeNil())
				// pick the vrf-id
				vrf := vrfs[0].Status.VrfID
				cmd := fwevent.Cmd(100, int(vrf))
				By(fmt.Sprintf("generate fwlogs in %v, %s", naples, cmd))
				st = ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, cmd))
				Expect(st == "null").Should(BeTrue())

				// check collectors
				for _, col := range syslogCollectors {
					By(fmt.Sprintf("verify syslog from %v in collector :%+v", naples, col))
					Eventually(func() error {
						select {
						case l := <-col.ch:
							By(fmt.Sprintf("received syslog: %+v", l))
							return nil

						case <-time.After(time.Second):
							return fmt.Errorf("timed out")

						}
					}, "180s", "1s").Should(BeNil(), "failed to recv syslog")
				}
			}

			// delete all
			By("cleanup fwlog policy")
			ctx = ts.tu.NewLoggedInContext(context.Background())
			for i := range testFwSpecList {
				objMeta := &api.ObjectMeta{
					Name:      fmt.Sprintf("fwpolicy-%d", i),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}
				fwlogClient.Delete(ctx, objMeta)
			}
		})

		// validate policy
		It("validate fwlog policy", func() {
			testFwPolicy := []struct {
				name   string
				fail   bool
				policy *monitoring.FwlogPolicy
			}{
				{
					name: "no name",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/10001",
								},
								{
									Destination: "test.pensando.iox",
									Transport:   "tcp/15001",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},
				{
					name: "invalid dns",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/10001",
								},
								{
									Destination: "test.pensando.iox",
									Transport:   "tcp/15001",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},
				{
					name: "too many collectors",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/10001",
								},
								{
									Destination: "localhost",
									Transport:   "tcp/15001",
								},
								{
									Destination: "192.168.100.1",
									Transport:   "udp/10001",
								},
								{
									Destination: "192.168.100.2",
									Transport:   "udp/10002",
								},
								{
									Destination: "192.168.100.1",
									Transport:   "tcp/10001",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},

				{
					name: "duplicate collectors",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/10001",
								},
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/10001",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},
				{
					name: "invalid destination",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Name:   "fwlog-invalid-dest",
							Tenant: globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/10001",
								},
								{
									Destination: "",
									Transport:   "tcp/10001",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},
				{
					name: "invalid target",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},

				{
					name: "no port in collector",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/10001",
								},
								{
									Destination: "192.168.100.11",
									Transport:   "tcp",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},

				{
					name: "invalid proto",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.11",
									Transport:   "ip/10001",
								},
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/10001",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},

				{
					name: "invalid port",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/10001",
								},
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/abcd",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},

				{
					name: "invalid port",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/10001",
								},
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/65536",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},
				{
					name: "duplicate collector",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.12",
									Transport:   "tcp/10001",
								},
								{
									Destination: "192.168.100.12",
									Transport:   "tcp/10001",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},
				{
					name: "no collectors",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},
				{
					name: "invalid override",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.12",
									Transport:   "tcp/10001",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: "test-override",
							},
						},
					},
				},
				{
					name: "create policy",
					fail: false,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      "fwlog" + globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},
						Spec: monitoring.FwlogPolicySpec{
							VrfName: globals.DefaultVrf,
							Targets: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.11",
									Transport:   "tcp/10001",
								},
								{
									Destination: "localhost",
									Transport:   "tcp/15001",
								},
							},
							Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
							Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
							Config: &monitoring.SyslogExportConfig{
								FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
							},
						},
					},
				},
			}

			ctx := ts.tu.NewLoggedInContext(context.Background())
			for i := range testFwPolicy {
				_, err := fwlogClient.Create(ctx, testFwPolicy[i].policy)
				if testFwPolicy[i].fail == true {
					By(fmt.Sprintf("test [%v] returned %v", testFwPolicy[i].name, err))
					Expect(err).ShouldNot(BeNil())
				} else {
					By(fmt.Sprintf("test [%v] returned %v", testFwPolicy[i].name, err))
					Expect(err).Should(BeNil())
				}
			}
		})
	})
})
