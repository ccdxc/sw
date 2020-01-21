package cluster

import (
	"context"
	"encoding/json"
	"fmt"
	"net"
	"os"
	"reflect"
	"strings"
	"time"

	"github.com/jeromer/syslogparser"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/agent/tmagent/state/fwgen/fwevent"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/ctrler/tpm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/syslog"
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

		testFwSpecList := make([]monitoring.FwlogPolicySpec, tpm.MaxNumExportPolicy)
		syslogCollectors := []testCollector{}

		AfterEach(func() {
			By("cleanup fwlog policy")
			ctx := ts.tu.MustGetLoggedInContext(context.Background())
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

			for i := 0; i < tpm.MaxNumExportPolicy; i++ {
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

			for i := 0; i < tpm.MaxNumExportPolicy; i++ {
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
			ctx := ts.tu.MustGetLoggedInContext(context.Background())
			if testFwSpecList, err := fwlogClient.List(ctx, &api.ListWatchOptions{}); err == nil {
				for i := range testFwSpecList {
					By(fmt.Sprintf("delete %v", testFwSpecList[i].ObjectMeta))
					fwlogClient.Delete(ctx, &testFwSpecList[i].ObjectMeta)
				}
			}
		})

		It("Should create/update/delete fwlog policy", func() {
			ctx := ts.tu.MustGetLoggedInContext(context.Background())
			fwlogMap := make(map[string]monitoring.FwlogPolicySpec)

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
				fwlogMap[fwPolicy.GetKey()] = testFwSpecList[i]
				_, err := fwlogClient.Create(ctx, fwPolicy)
				Expect(err).ShouldNot(HaveOccurred())
			}

			// use token api to get NAPLES access credentials
			nodeAuthFile, err := testutils.GetNodeAuthTokenTempFile(ctx, apiGwAddr, []string{"*"})
			Expect(err).ShouldNot(HaveOccurred())
			defer os.Remove(nodeAuthFile)

			Eventually(func() error {
				By("verify fwlog policy in Venice")
				pl, err := fwlogClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				// check length
				if len(pl) != len(testFwSpecList) {
					By(fmt.Sprintf("received fwlog policy from venice %+v", pl))
					return fmt.Errorf("invalid number of policy in Venice, got %v expected %+v", len(pl), len(testFwSpecList))
				}

				// deep check
				listFwlogMap := make(map[string]monitoring.FwlogPolicySpec)
				for _, policy := range pl {
					listFwlogMap[policy.GetKey()] = policy.Spec
				}
				Expect(reflect.DeepEqual(fwlogMap, listFwlogMap)).To(Equal(true))

				for _, naples := range ts.tu.NaplesNodes {
					napleFwlogMap := make(map[string]monitoring.FwlogPolicySpec)
					By(fmt.Sprintf("verify fwlog policy in %v", naples))
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/fwlog/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					var naplesPol []monitoring.FwlogPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						By(fmt.Sprintf("received fwlog policy from naples: %v, %+v", naples, st))
						return err
					}

					// check length
					if len(naplesPol) != len(testFwSpecList) {
						By(fmt.Sprintf("received fwlog policy from naples: %v, %v, %v", naples, string([]byte(st)), naplesPol))
						return fmt.Errorf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), len(testFwSpecList))
					}

					// deep check
					for _, policy := range naplesPol {
						napleFwlogMap[policy.GetKey()] = policy.Spec
					}
					Expect(reflect.DeepEqual(fwlogMap, napleFwlogMap)).To(Equal(true))
				}
				return nil
			}, 300, 2).Should(BeNil(), "failed to find fwlog policy")

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
				fwlogMap[fwPolicy.GetKey()] = fwPolicy.Spec
				_, err := fwlogClient.Update(ctx, fwPolicy)
				Expect(err).Should(BeNil())
			}

			Eventually(func() error {
				By("Verify fwlog policy from Venice")
				pl, err := fwlogClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				// check length
				if len(pl) != len(testFwSpecList) {
					By(fmt.Sprintf("received fwlog policy from venice %+v", pl))
					return fmt.Errorf("invalid number of policy in Venice, got %v expected %+v", len(pl), len(testFwSpecList))
				}

				// deep check
				listFwlogMap := make(map[string]monitoring.FwlogPolicySpec)
				for _, policy := range pl {
					listFwlogMap[policy.GetKey()] = policy.Spec
				}
				Expect(reflect.DeepEqual(fwlogMap, listFwlogMap)).To(Equal(true))

				for _, naples := range ts.tu.NaplesNodes {
					napleFwlogMap := make(map[string]monitoring.FwlogPolicySpec)
					By(fmt.Sprintf("verify fwlog policy in %v", naples))
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/fwlog/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					if !utils.IsEmpty(st) {
						var naplesPol []monitoring.FwlogPolicy
						if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
							By(fmt.Sprintf("received fwlog policy from naples: %v, %+v", naples, st))
							return err
						}

						// check length
						if len(naplesPol) != len(testFwSpecList) {
							By(fmt.Sprintf("received fwlog policy from naples: %v, %v", naples, naplesPol))
							return fmt.Errorf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), len(testFwSpecList))
						}

						// deep check
						for _, policy := range naplesPol {
							napleFwlogMap[policy.GetKey()] = policy.Spec
						}
						Expect(reflect.DeepEqual(fwlogMap, napleFwlogMap)).To(Equal(true))

					} else {
						return fmt.Errorf("failed to get fwlog policy from naples %s . got (%s)", naples, st)
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
			ctx := ts.tu.MustGetLoggedInContext(context.Background())

			// use token api to get NAPLES access credentials
			nodeAuthFile, err := testutils.GetNodeAuthTokenTempFile(ctx, apiGwAddr, []string{"*"})
			Expect(err).ShouldNot(HaveOccurred())
			defer os.Remove(nodeAuthFile)

			By(fmt.Sprintf("create %v fwlog Policy", len(testFwSpecList)))
			fwlogMap := make(map[string]monitoring.FwlogPolicySpec)
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
				fwlogMap[fwPolicy.GetKey()] = testFwSpecList[i]
				_, err := fwlogClient.Create(ctx, fwPolicy)
				Expect(err).ShouldNot(HaveOccurred())
			}

			Eventually(func() error {
				By("verify fwlog policy in Venice")
				pl, err := fwlogClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				// check length
				if len(pl) != len(testFwSpecList) {
					By(fmt.Sprintf("received fwlog policy from venice %+v", pl))
					return fmt.Errorf("invalid number of policy in Venice, got %v expected %v, %+v", len(pl), len(testFwSpecList), testFwSpecList)
				}

				// deep check
				veniceFwlogMap := make(map[string]monitoring.FwlogPolicySpec)
				for _, policy := range pl {
					veniceFwlogMap[policy.GetKey()] = policy.Spec
				}
				Expect(reflect.DeepEqual(fwlogMap, veniceFwlogMap)).To(Equal(true))

				return nil
			}, "20s", "2s").Should(BeNil(), "failed to find fwlog policy")

			for _, naples := range ts.tu.NaplesNodes {
				By(fmt.Sprintf("verify fwlog policy in %v", naples))
				Eventually(func() error {
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

					// check syslog server status
					cmd := "curl -s http://localhost:9013/debug/state"
					o := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, cmd))
					dbg := struct {
						FwlogCollectors []string `json:"fwlog-collectors"`
					}{}

					if err := json.Unmarshal([]byte(o), &dbg); err != nil {
						return err
					}

					for _, d := range dbg.FwlogCollectors {
						if !strings.Contains(d, "fd:true") {
							return fmt.Errorf("socket is not ready %v", d)
						}
					}

					return nil
				}, "20s", "2s").Should(BeNil(), "failed to find fwlog policy")
			}

			for _, naples := range ts.tu.NaplesNodes {
				Eventually(func() error {
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/vrfs/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					var vrfs []netproto.Vrf
					err := json.Unmarshal([]byte(st), &vrfs)
					Expect(err).Should(BeNil())
					// pick the vrf-id
					vrf := vrfs[0].Status.VrfID
					cmd := fwevent.Cmd(10, int(vrf))
					By(fmt.Sprintf("generate fwlogs in %v, %s", naples, cmd))
					st = ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, cmd))
					Expect(st == "null").Should(BeTrue())

					// check tx
					cmd = "curl -s http://localhost:9013/debug/state"
					o := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, cmd))
					dbg := struct {
						FwlogCollectors []string `json:"fwlog-collectors"`
					}{}

					if err := json.Unmarshal([]byte(o), &dbg); err != nil {
						return err
					}

					for _, d := range dbg.FwlogCollectors {
						if !strings.Contains(d, "fd:true") {
							return fmt.Errorf("socket is not ready %v", d)
						}

						if !strings.Contains(d, "txCount:10") {
							return fmt.Errorf("invalid tx count %v", d)
						}
					}
					return nil
				}, "20s", "2s").Should(BeNil(), "failed to send syslog")

				// check collectors
				for _, col := range syslogCollectors {
					By(fmt.Sprintf("verify syslog from %v in collector :%+v", naples, col))
					Eventually(func() error {
						select {
						case <-col.ch:
							return nil

						case <-time.After(time.Second):
							return fmt.Errorf("timed out")

						}
					}, "60s", "1s").Should(BeNil(), "failed to recv syslog")
				}
			}

			// delete all
			ctx = ts.tu.MustGetLoggedInContext(context.Background())
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
							Name:      "invalid-dns",
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
					name: "too many targets",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      "too-many-targets",
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
					name: "duplicate targets",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      "duplicate-targets",
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
							Name:   "invalid-destination",
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
							Name:      "invalid-target",
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
					name: "no port in transport",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      "no-port-in-transport",
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
							Name:      "invalid-proto",
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
							Name:      "invalid-port-abcd",
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
							Name:      "invalid-port-65536",
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
					name: "duplicate targets",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      "duplicate-targets",
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
					name: "no targets",
					fail: true,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      "no-targets",
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
							Name:      "invalid-override",
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
					name: "create policy (good)",
					fail: false,
					policy: &monitoring.FwlogPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "fwLogPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      "create-good-policy",
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

			ctx := ts.tu.MustGetLoggedInContext(context.Background())
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
		It("validate max. collectors", func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.MustGetLoggedInContext(pctx)

			policy := &monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					VrfName: globals.DefaultVrf,
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "udp/10001",
						},
						{
							Destination: "192.168.100.12",
							Transport:   "udp/11001",
						},
						{
							Destination: "192.168.100.13",
							Transport:   "tcp/12001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			}

			// create should fail for more than 2 collectors
			policy.Name = "test-max-collector"
			_, err := fwlogClient.Create(ctx, policy)
			Expect(err).ShouldNot(BeNil(), "policy create didn't fail for invalid number of collectors")
		})

		It("validate max. fwlog export policy", func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.MustGetLoggedInContext(pctx)

			policy := &monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					VrfName: globals.DefaultVrf,
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "udp/10001",
						},
						{
							Destination: "192.168.100.12",
							Transport:   "udp/11001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			}

			for i := 0; i < tpm.MaxNumExportPolicy; i++ {
				policy.Name = fmt.Sprintf("test-policy-%d", i)
				_, err := fwlogClient.Create(ctx, policy)
				Expect(err).Should(BeNil(), fmt.Sprintf("failed to create  %v, %v", policy.Name, err))
			}

			// new policy create should fail
			policy.Name = "test-policy-fail"
			_, err := fwlogClient.Create(ctx, policy)
			Expect(err).ShouldNot(BeNil(), fmt.Sprintf("policy create didn't fail, %v", err))

		})
	})
})
