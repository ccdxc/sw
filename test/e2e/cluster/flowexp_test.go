package cluster

import (
	"context"
	"encoding/json"
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/protos/tpmprotos"
	"github.com/pensando/sw/venice/ctrler/tpm"
	"github.com/pensando/sw/venice/globals"
)

var _ = Describe("flow export policy tests", func() {
	Context("flow export policy CRUD tests", func() {
		var flowExpClient monitoring.MonitoringV1FlowExportPolicyInterface

		AfterEach(func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.NewLoggedInContext(pctx)

			By("cleanup flow export policy")
			if testFlowExpSpecList, err := flowExpClient.List(ctx, &api.ListWatchOptions{}); err == nil {
				for i := range testFlowExpSpecList {
					By(fmt.Sprintf("delete %v", testFlowExpSpecList[i].ObjectMeta))
					flowExpClient.Delete(ctx, &testFlowExpSpecList[i].ObjectMeta)
				}
			}
		})

		BeforeEach(func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.NewLoggedInContext(pctx)

			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			restSvc, err := apiclient.NewRestAPIClient(apiGwAddr)
			Expect(err).ShouldNot(HaveOccurred())
			flowExpClient = restSvc.MonitoringV1().FlowExportPolicy()

			// delete all
			By("cleanup flow exp policy")
			if testFlowExpSpecList, err := flowExpClient.List(ctx, &api.ListWatchOptions{}); err == nil {
				for i := range testFlowExpSpecList {
					By(fmt.Sprintf("delete %v", testFlowExpSpecList[i].ObjectMeta))
					flowExpClient.Delete(ctx, &testFlowExpSpecList[i].ObjectMeta)
				}
			}
		})

		It("Should create/update/delete flow export policy", func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.NewLoggedInContext(pctx)

			testFwSpecList := make([]monitoring.FlowExportPolicySpec, tpm.MaxPolicyPerVrf)

			for i := 0; i < tpm.MaxPolicyPerVrf; i++ {
				testFwSpecList[i] = monitoring.FlowExportPolicySpec{
					VrfName:  globals.DefaultVrf,
					Interval: "10s",
					Format:   monitoring.FlowExportPolicySpec_Ipfix.String(),
					MatchRules: []monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"any"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"any"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"tcp/5500"},
							},
						},
					},
					Exports: []monitoring.ExportConfig{
						{
							// todo: configure only 1 collector for now till agent is fixed
							//Destination: fmt.Sprintf("192.168.100.%d", i+1),
							Destination: fmt.Sprintf("192.168.100.11"),
							Transport:   "TCP/5545",
						},
						//{
						//	Destination: fmt.Sprintf("192.168.100.%d", i+1),
						//	Transport:   "UDP/5545",
						//},
					},
				}
			}

			for i := 0; i < tpm.MaxPolicyPerVrf; i++ {
				flowPolicy := &monitoring.FlowExportPolicy{
					TypeMeta: api.TypeMeta{
						Kind: "FlowExportPolicy",
					},
					ObjectMeta: api.ObjectMeta{
						Name:      fmt.Sprintf("flowexp-%d", i),
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
					},
					Spec: testFwSpecList[i],
				}
				_, err := flowExpClient.Create(ctx, flowPolicy)
				Expect(err).ShouldNot(HaveOccurred())
				By(fmt.Sprintf("create flow export Policy %v", flowPolicy.Name))
			}

			Eventually(func() error {
				By("verify flow export policy in Venice")
				pl, err := flowExpClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				if len(pl) != len(testFwSpecList) {
					By(fmt.Sprintf("received flow export policy from venice %+v", pl))
					return fmt.Errorf("invalid number of policy in Venice, got %v expected %+v", len(pl), len(testFwSpecList))
				}

				for _, naples := range ts.tu.NaplesNodes {
					By(fmt.Sprintf("verify flow export policy in %v", naples))
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, "curl -s localhost:8888/api/telemetry/flowexports/"))
					var naplesPol []tpmprotos.FlowExportPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						By(fmt.Sprintf("received flow export policy from naples: %v, %+v", naples, st))
						return err
					}

					if len(naplesPol) != len(testFwSpecList) {
						By(fmt.Sprintf("received flow export policy from naples: %v, %v", naples, naplesPol))
						return fmt.Errorf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), len(testFwSpecList))
					}

				}
				return nil
			}, 120, 2).Should(BeNil(), "failed to find flow export policy")

			By("Update flow export Policy")
			for i := range testFwSpecList {
				fwPolicy := &monitoring.FlowExportPolicy{
					TypeMeta: api.TypeMeta{
						Kind: "FlowExportPolicy",
					},
					ObjectMeta: api.ObjectMeta{
						Name:      fmt.Sprintf("flowexp-%d", i),
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
					},
					Spec: testFwSpecList[len(testFwSpecList)-i-1],
				}
				_, err := flowExpClient.Update(ctx, fwPolicy)
				Expect(err).Should(BeNil())
			}

			Eventually(func() error {
				By("Verify flow export policy from Venice")
				pl, err := flowExpClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				if len(pl) != len(testFwSpecList) {
					By(fmt.Sprintf("received flow export policy from venice %+v", pl))
					return fmt.Errorf("invalid number of policy in Venice, got %v expected %+v", len(pl), len(testFwSpecList))
				}

				for _, naples := range ts.tu.NaplesNodes {
					By(fmt.Sprintf("verify flow export policy in %v", naples))
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, "curl -s localhost:8888/api/telemetry/flowexports/"))
					var naplesPol []tpmprotos.FlowExportPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						By(fmt.Sprintf("received flow export policy from naples: %v, %+v", naples, st))
						return err
					}

					if len(naplesPol) != len(testFwSpecList) {
						By(fmt.Sprintf("received flow export policy from naples: %v, %v", naples, naplesPol))
						return fmt.Errorf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), len(testFwSpecList))
					}
				}
				return nil
			}, 120, 2).Should(BeNil(), "failed to find flow export policy")

			By("Delete flow export policy")
			for i := range testFwSpecList {

				objMeta := &api.ObjectMeta{
					Name:      fmt.Sprintf("flowexp-%d", i),
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
				}

				_, err := flowExpClient.Delete(ctx, objMeta)
				Expect(err).ShouldNot(HaveOccurred())
			}

			Eventually(func() error {
				By("Verify flow export policy from Venice")
				pl, err := flowExpClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				if len(pl) != 0 {
					By(fmt.Sprintf("policy exists after delete, %+v", pl))
					return fmt.Errorf("policy exists after delete, %+v", pl)
				}

				for _, naples := range ts.tu.NaplesNodes {
					By(fmt.Sprintf("verify flow export policy in %v", naples))

					st := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, "curl -s localhost:8888/api/telemetry/flowexports/"))
					var naplesPol []tpmprotos.FlowExportPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						By(fmt.Sprintf("received flow export policy from naples:%v,  %+v", naples, st))
						return err
					}

					if len(naplesPol) != 0 {
						By(fmt.Sprintf("received flow export policy from naples:%v,  %+v", naples, naplesPol))
						return fmt.Errorf("invalid number of policy in %v, got %d, expected 0", naples, len(naplesPol))
					}
				}
				return nil
			}, 120, 2).Should(BeNil(), "failed to verify flow export policy")
		})

		It("validate flow export policy", func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.NewLoggedInContext(pctx)

			testFlowExpSpecList := []struct {
				name   string
				policy monitoring.FlowExportPolicy
				fail   bool
			}{
				{
					name: "empty collector",
					fail: true,
					policy: monitoring.FlowExportPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "flowExportPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},

						Spec: monitoring.FlowExportPolicySpec{
							MatchRules: []monitoring.MatchRule{
								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.1"},
									},

									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"TCP/1000"},
									},
								},

								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.2.1"},
									},
									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.2.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"TCP/1010"},
									},
								},
							},

							Interval: "15s",
							Format:   "IPFIX",
							Exports:  []monitoring.ExportConfig{},
						},
					},
				},
				{

					name: "large interval",
					fail: true,
					policy: monitoring.FlowExportPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "flowExportPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},

						Spec: monitoring.FlowExportPolicySpec{
							MatchRules: []monitoring.MatchRule{
								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.1"},
									},

									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"TCP/1000"},
									},
								},

								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.2.1"},
									},
									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.2.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"TCP/1010"},
									},
								},
							},

							Interval: "25h",
							Format:   "IPFIX",
							Exports: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.1",
									Transport:   "TCP/5055",
								},
							},
						},
					},
				},
				{

					name: "small interval",
					fail: true,
					policy: monitoring.FlowExportPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "flowExportPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},

						Spec: monitoring.FlowExportPolicySpec{
							MatchRules: []monitoring.MatchRule{
								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.1"},
									},

									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"TCP/1000"},
									},
								},

								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.2.1"},
									},
									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.2.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"TCP/1010"},
									},
								},
							},

							Interval: "10ms",
							Format:   "IPFIX",
							Exports: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.1",
									Transport:   "TCP/5055",
								},
							},
						},
					},
				},

				{

					name: "invalid interval",
					fail: true,
					policy: monitoring.FlowExportPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "flowExportPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},

						Spec: monitoring.FlowExportPolicySpec{
							MatchRules: []monitoring.MatchRule{
								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.1"},
									},

									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"TCP/1000"},
									},
								},

								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.2.1"},
									},
									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.2.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"TCP/1010"},
									},
								},
							},

							Interval: "1000",
							Format:   "IPFIX",
							Exports: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.1",
									Transport:   "TCP/5055",
								},
							},
						},
					},
				},

				{
					name: "invalid format",
					fail: true,
					policy: monitoring.FlowExportPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "flowExportPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},

						Spec: monitoring.FlowExportPolicySpec{
							MatchRules: []monitoring.MatchRule{
								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.1"},
									},

									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"TCP/1000"},
									},
								},

								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.2.1"},
									},
									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.2.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"TCP/1010"},
									},
								},
							},

							Interval: "10s",
							Format:   "NETFLOW",
							Exports: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.1",
									Transport:   "TCP/5055",
								},
							},
						},
					},
				},

				{
					name: "no match-rule",
					fail: true,
					policy: monitoring.FlowExportPolicy{
						TypeMeta: api.TypeMeta{
							Kind: "flowExportPolicy",
						},
						ObjectMeta: api.ObjectMeta{
							Namespace: globals.DefaultNamespace,
							Name:      globals.DefaultTenant,
							Tenant:    globals.DefaultTenant,
						},

						Spec: monitoring.FlowExportPolicySpec{
							Interval: "10s",
							Format:   "NETFLOW",
							Exports: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.1",
									Transport:   "TCP/5055",
								},
							},
						},
					},
				},
			}

			for i := range testFlowExpSpecList {
				_, err := flowExpClient.Create(ctx, &testFlowExpSpecList[i].policy)
				if testFlowExpSpecList[i].fail == true {
					By(fmt.Sprintf("test [%v] returned %v", testFlowExpSpecList[i].name, err))
					Expect(err).ShouldNot(BeNil())
				} else {
					By(fmt.Sprintf("test [%v] returned %v", testFlowExpSpecList[i].name, err))
					Expect(err).Should(BeNil())
				}
			}

		})
	})
})
