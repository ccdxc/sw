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
							Destination: fmt.Sprintf("192.168.100.%d", i+1),
							Transport:   "UDP/5545",
						},
						{
							Destination: fmt.Sprintf("192.168.100.%d", i+1),
							Transport:   "UDP/5565",
						},
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

		It("Should create/delete multiple flow export policy with the same collector", func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.NewLoggedInContext(pctx)

			testFwSpecList := make([]monitoring.FlowExportPolicySpec, tpm.MaxCollectorPerPolicy)

			for i := 0; i < tpm.MaxCollectorPerPolicy; i++ {
				testFwSpecList[i] = monitoring.FlowExportPolicySpec{
					VrfName:  globals.DefaultVrf,
					Interval: "10s",
					Format:   monitoring.FlowExportPolicySpec_Ipfix.String(),
					MatchRules: []monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{fmt.Sprintf("192.168.100.%d", i+1)},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{fmt.Sprintf("192.168.200.%d", i+1)},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"tcp/5500"},
							},
						},
					},
					Exports: []monitoring.ExportConfig{
						{
							Destination: "192.168.10.1",
							Transport:   "UDP/5545",
						},
						{
							Destination: "192.168.10.2",
							Transport:   "UDP/5565",
						},
					},
				}
			}

			for i := 0; i < tpm.MaxCollectorPerPolicy; i++ {
				flowPolicy := &monitoring.FlowExportPolicy{
					TypeMeta: api.TypeMeta{
						Kind: "FlowExportPolicy",
					},
					ObjectMeta: api.ObjectMeta{
						Name:      fmt.Sprintf("e2e-collector-%d", i),
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

					st = ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, "curl -s localhost:9007/debug/tpa"))
					naplesDbg := struct {
						FlowRuleTable []struct {
							PolicyNames []string
						}
						CollectorTable []struct {
							PolicyNames []string
						}
					}{}

					By(fmt.Sprintf("received debug info %+v", string(st)))
					if err := json.Unmarshal([]byte(st), &naplesDbg); err != nil {
						By(fmt.Sprintf("received flow export debug from naples: %v, %+v", naples, st))
						return err
					}

					if len(naplesDbg.FlowRuleTable) != tpm.MaxCollectorPerPolicy {
						By(fmt.Sprintf("received %d rules, expected %v", len(naplesDbg.FlowRuleTable), tpm.MaxCollectorPerPolicy))
						return err
					}

					if len(naplesDbg.CollectorTable) != tpm.MaxCollectorPerPolicy {
						By(fmt.Sprintf("received %d collectors, expected 2", len(naplesDbg.CollectorTable)))
						return err
					}

					for i := 0; i < tpm.MaxCollectorPerPolicy; i++ {
						if len(naplesDbg.CollectorTable[i].PolicyNames) != tpm.MaxCollectorPerPolicy {
							By(fmt.Sprintf("received %d policynames in collector, expected 2", len(naplesDbg.CollectorTable[i].PolicyNames)))
							return err
						}
						if len(naplesDbg.FlowRuleTable[i].PolicyNames) != 1 {
							By(fmt.Sprintf("received %d policynames in flow-rule, expected 1", len(naplesDbg.FlowRuleTable)))
							return err
						}
					}

				}
				return nil
			}, 120, 2).Should(BeNil(), "failed to find flow export policy")

			for i := 0; i < tpm.MaxCollectorPerPolicy; i++ {
				expRules := tpm.MaxCollectorPerPolicy - i - 1
				expCollectors := tpm.MaxCollectorPerPolicy - i*2

				flowPolicy := &monitoring.FlowExportPolicy{
					TypeMeta: api.TypeMeta{
						Kind: "FlowExportPolicy",
					},
					ObjectMeta: api.ObjectMeta{
						Name:      fmt.Sprintf("e2e-collector-%d", i),
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
					},
				}

				By(fmt.Sprintf("delete policy %v", flowPolicy.Name))
				_, err := flowExpClient.Delete(ctx, &flowPolicy.ObjectMeta)
				Expect(err).ShouldNot(HaveOccurred())

				By("verify flow export policy in Venice")
				pl, err := flowExpClient.List(ctx, &api.ListWatchOptions{})
				Expect(err).ShouldNot(HaveOccurred())
				testFwSpecList[0] = testFwSpecList[len(testFwSpecList)-1]
				testFwSpecList = testFwSpecList[0 : len(testFwSpecList)-1]
				Expect(len(pl)).To(Equal(len(testFwSpecList)))

				Eventually(func() error {
					for _, naples := range ts.tu.NaplesNodes {
						By(fmt.Sprintf("verify flow export policy in %v", naples))
						st := ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, "curl -s localhost:8888/api/telemetry/flowexports/"))
						var naplesPol []tpmprotos.FlowExportPolicy
						if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
							By(fmt.Sprintf("received flow export policy from naples: %v, %+v", naples, st))
							return err
						}

						if len(naplesPol) != expRules {
							By(fmt.Sprintf("received flow export policy from naples: %v, %v", naples, naplesPol))
							return fmt.Errorf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), expRules)
						}

						st = ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, "curl -s localhost:9007/debug/tpa"))
						naplesDbg := struct {
							FlowRuleTable []struct {
								PolicyNames []string
							}
							CollectorTable []struct {
								PolicyNames []string
							}
						}{}

						By(fmt.Sprintf("received debug info %+v", string(st)))
						if err := json.Unmarshal([]byte(st), &naplesDbg); err != nil {
							By(fmt.Sprintf("received flow export debug from naples: %v, %+v", naples, st))
							return err
						}

						if len(naplesDbg.FlowRuleTable) != expRules {
							By(fmt.Sprintf("received %d rules, expected %v", len(naplesDbg.FlowRuleTable), expRules))
							return err
						}

						for j := 0; j < expRules; j++ {
							if len(naplesDbg.FlowRuleTable[j].PolicyNames) != 1 {
								By(fmt.Sprintf("received %d policynames, expected 1", len(naplesDbg.FlowRuleTable)))
								return err
							}
						}

						if len(naplesDbg.CollectorTable) != expCollectors {
							By(fmt.Sprintf("received %d collectors, expected %d", len(naplesDbg.CollectorTable), expCollectors))
							return err
						}

						for j := 0; j < expCollectors; j++ {
							if len(naplesDbg.CollectorTable[j].PolicyNames) != 1 {
								By(fmt.Sprintf("received %d policynames in collector, expected 2", len(naplesDbg.CollectorTable[j].PolicyNames)))
								return err
							}
						}
					}
					return nil
				}, 120, 2).Should(BeNil(), "failed to find flow export policy")
			}

		})

		It("Should create/delete multiple flow export policy with the same match-rule", func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.NewLoggedInContext(pctx)

			testFwSpecList := make([]monitoring.FlowExportPolicySpec, tpm.MaxCollectorPerPolicy)

			for i := 0; i < tpm.MaxCollectorPerPolicy; i++ {
				testFwSpecList[i] = monitoring.FlowExportPolicySpec{
					VrfName:  globals.DefaultVrf,
					Interval: "10s",
					Format:   monitoring.FlowExportPolicySpec_Ipfix.String(),
					MatchRules: []monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{fmt.Sprintf("192.168.20.%d", i+1)},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{fmt.Sprintf("192.168.20.%d", i+1)},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"tcp/5500"},
							},
						},
					},
					Exports: []monitoring.ExportConfig{
						{
							Destination: fmt.Sprintf("192.168.10.%d", i+1),
							Transport:   "UDP/5545",
						},
					},
				}
			}

			for i := 0; i < tpm.MaxCollectorPerPolicy; i++ {
				flowPolicy := &monitoring.FlowExportPolicy{
					TypeMeta: api.TypeMeta{
						Kind: "FlowExportPolicy",
					},
					ObjectMeta: api.ObjectMeta{
						Name:      fmt.Sprintf("e2e-matchrule-%d", i),
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

					st = ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, "curl -s localhost:9007/debug/tpa"))
					naplesDbg := struct {
						FlowRuleTable []struct {
							PolicyNames []string
						}
						CollectorTable []struct {
							PolicyNames []string
						}
					}{}

					By(fmt.Sprintf("received debug info %+v", string(st)))
					if err := json.Unmarshal([]byte(st), &naplesDbg); err != nil {
						By(fmt.Sprintf("received flow export debug from naples: %v, %+v", naples, st))
						return err
					}

					if len(naplesDbg.FlowRuleTable) != tpm.MaxCollectorPerPolicy {
						By(fmt.Sprintf("received %d rules, expected %v", len(naplesDbg.FlowRuleTable), tpm.MaxCollectorPerPolicy))
						return err
					}

					By(fmt.Sprintf("received  info %+v", naplesDbg))

					if len(naplesDbg.CollectorTable) != tpm.MaxCollectorPerPolicy {
						By(fmt.Sprintf("received %d collectors, expected 2", len(naplesDbg.CollectorTable)))
						return err
					}

					for i := 0; i < tpm.MaxCollectorPerPolicy; i++ {
						if len(naplesDbg.CollectorTable[i].PolicyNames) != tpm.MaxCollectorPerPolicy {
							By(fmt.Sprintf("received %d policynames in collector, expected 2", len(naplesDbg.CollectorTable[i].PolicyNames)))
							return err
						}
						if len(naplesDbg.FlowRuleTable[i].PolicyNames) != 1 {
							By(fmt.Sprintf("received %d policynames in flow-rule, expected 1", len(naplesDbg.FlowRuleTable)))
							return err
						}
					}

				}
				return nil
			}, 120, 2).Should(BeNil(), "failed to find flow export policy")

			for i := 0; i < tpm.MaxCollectorPerPolicy; i++ {
				expRules := tpm.MaxCollectorPerPolicy - 2*i
				expCollectors := tpm.MaxCollectorPerPolicy - 1 - i

				flowPolicy := &monitoring.FlowExportPolicy{
					TypeMeta: api.TypeMeta{
						Kind: "FlowExportPolicy",
					},
					ObjectMeta: api.ObjectMeta{
						Name:      fmt.Sprintf("e2e-matchrule-%d", i),
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
					},
				}

				By(fmt.Sprintf("delete policy %v", flowPolicy.Name))
				_, err := flowExpClient.Delete(ctx, &flowPolicy.ObjectMeta)
				Expect(err).ShouldNot(HaveOccurred())

				By("verify flow export policy in Venice")
				pl, err := flowExpClient.List(ctx, &api.ListWatchOptions{})
				Expect(err).ShouldNot(HaveOccurred())
				testFwSpecList[0] = testFwSpecList[len(testFwSpecList)-1]
				testFwSpecList = testFwSpecList[0 : len(testFwSpecList)-1]
				Expect(len(pl)).To(Equal(len(testFwSpecList)))

				Eventually(func() error {
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

						st = ts.tu.LocalCommandOutput(fmt.Sprintf("docker exec %s %s", naples, "curl -s localhost:9007/debug/tpa"))
						naplesDbg := struct {
							FlowRuleTable []struct {
								PolicyNames []string
							}
							CollectorTable []struct {
								PolicyNames []string
							}
						}{}

						By(fmt.Sprintf("received debug info %+v", string(st)))
						if err := json.Unmarshal([]byte(st), &naplesDbg); err != nil {
							By(fmt.Sprintf("received flow export debug from naples: %v, %+v", naples, st))
							return err
						}

						if len(naplesDbg.FlowRuleTable) != expRules {
							By(fmt.Sprintf("received %d rules, expected %v", len(naplesDbg.FlowRuleTable), expRules))
							return err
						}

						for j := 0; j < expRules; j++ {
							if len(naplesDbg.FlowRuleTable[j].PolicyNames) != 1 {
								By(fmt.Sprintf("received %d policynames, expected 1", len(naplesDbg.FlowRuleTable)))
								return err
							}
						}

						if len(naplesDbg.CollectorTable) != expCollectors {
							By(fmt.Sprintf("received %d collectors, expected %d", len(naplesDbg.CollectorTable), expCollectors))
							return err
						}

						for j := 0; j < expCollectors; j++ {
							if len(naplesDbg.CollectorTable[j].PolicyNames) != 1 {
								By(fmt.Sprintf("received %d policynames in collector, expected 2", len(naplesDbg.CollectorTable[j].PolicyNames)))
								return err
							}
						}

					}
					return nil
				}, 120, 2).Should(BeNil(), "failed to find flow export policy")
			}

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
