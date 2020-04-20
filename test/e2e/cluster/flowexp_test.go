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

	tpmprotos "github.com/pensando/sw/nic/agent/protos/netproto"

	"github.com/calmh/ipfix"

	vflow "github.com/pensando/sw/venice/utils/ipfix"
	"github.com/pensando/sw/venice/utils/ipfix/server"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/ctrler/tpm"
	"github.com/pensando/sw/venice/globals"
)

var _ = Describe("flow export policy tests", func() {
	Context("flow export policy CRUD tests", func() {
		var flowExpClient monitoring.MonitoringV1FlowExportPolicyInterface
		var apiGwAddr string

		AfterEach(func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.MustGetLoggedInContext(pctx)

			By("cleanup flow export policy")
			if testFlowExpSpecList, err := flowExpClient.List(ctx, &api.ListWatchOptions{}); err == nil {
				for i := range testFlowExpSpecList {
					fmt.Printf("delete %v\n", testFlowExpSpecList[i].ObjectMeta)
					flowExpClient.Delete(ctx, &testFlowExpSpecList[i].ObjectMeta)
				}
			}
		})

		BeforeEach(func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.MustGetLoggedInContext(pctx)

			apiGwAddr = ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			restSvc, err := apiclient.NewRestAPIClient(apiGwAddr)
			Expect(err).ShouldNot(HaveOccurred())
			flowExpClient = restSvc.MonitoringV1().FlowExportPolicy()

			// delete all
			By("cleanup flow exp policy")
			if testFlowExpSpecList, err := flowExpClient.List(ctx, &api.ListWatchOptions{}); err == nil {
				for i := range testFlowExpSpecList {
					fmt.Printf("delete %v \n", testFlowExpSpecList[i].ObjectMeta)
					flowExpClient.Delete(ctx, &testFlowExpSpecList[i].ObjectMeta)
				}
			}
		})

		It("Should create/update/delete flow export policy", func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.MustGetLoggedInContext(pctx)

			testFwSpecList := make([]monitoring.FlowExportPolicySpec, tpm.MaxNumExportPolicy)

			for i := 0; i < tpm.MaxNumExportPolicy; i++ {
				testFwSpecList[i] = monitoring.FlowExportPolicySpec{
					VrfName:          globals.DefaultVrf,
					Interval:         "10s",
					TemplateInterval: "5m",
					Format:           monitoring.FlowExportPolicySpec_Ipfix.String(),
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"any"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"any"},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"tcp/5500", "tcp/4400-4402"},
							},
						},
					},
					Exports: []monitoring.ExportConfig{
						{
							Destination: fmt.Sprintf("192.168.%d.1", i%tpm.MaxUniqueNumCollectors),
							Transport:   "UDP/5545",
						},
						{
							Destination: fmt.Sprintf("192.168.%d.1", (i+1)%tpm.MaxUniqueNumCollectors),
							Transport:   "UDP/5545",
						},
					},
				}
			}

			venicePolicy := []*monitoring.FlowExportPolicy{}
			for i := 0; i < tpm.MaxNumExportPolicy; i++ {
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
				fmt.Printf("create flow export Policy %v \n", flowPolicy.Name)
				venicePolicy = append(venicePolicy, flowPolicy)
			}

			// use token api to get NAPLES access credentials
			nodeAuthFile, err := utils.GetNodeAuthTokenTempFile(ctx, apiGwAddr, []string{"*"})
			Expect(err).ShouldNot(HaveOccurred())
			defer os.Remove(nodeAuthFile)

			Eventually(func() error {
				By("verify flow export policy in Venice")
				pl, err := flowExpClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				if len(pl) != len(testFwSpecList) {
					fmt.Printf("received flow export policy from venice %+v \n", pl)
					return fmt.Errorf("invalid number of policy in Venice, got %v expected %+v", len(pl), len(testFwSpecList))
				}

				for _, naples := range ts.tu.NaplesNodes {
					By(fmt.Sprintf("verify flow export policy in %v", naples))
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/flowexports/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					fmt.Printf("naples-%v: policy  %+v\n", naples, st)

					var naplesPol []*tpmprotos.FlowExportPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						fmt.Printf("received flow export policy from naples: %v, %+v \n", naples, st)
						return err
					}

					if len(naplesPol) != len(testFwSpecList) {
						fmt.Printf("received flow export policy from naples: %v, %v \n", naples, naplesPol)
						return fmt.Errorf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), len(testFwSpecList))
					}

					if err := cmpExportPolicy(naples, venicePolicy, naplesPol); err != nil {
						return err
					}
				}
				return nil
			}, 180, 2).Should(BeNil(), "failed to find flow export policy")

			By("Update flow export Policy")
			venicePolicy = []*monitoring.FlowExportPolicy{}
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
				venicePolicy = append(venicePolicy, fwPolicy)
			}

			Eventually(func() error {
				By("Verify flow export policy from Venice")
				pl, err := flowExpClient.List(ctx, &api.ListWatchOptions{})
				if err != nil {
					return err
				}

				if len(pl) != len(testFwSpecList) {
					fmt.Printf("received flow export policy from venice %+v \n", pl)
					return fmt.Errorf("invalid number of policy in Venice, got %v expected %+v", len(pl), len(testFwSpecList))
				}

				for _, naples := range ts.tu.NaplesNodes {
					By(fmt.Sprintf("verify flow export policy in %v", naples))
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/flowexports/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					fmt.Printf("naples-%v: policy  %+v\n", naples, st)

					var naplesPol []*tpmprotos.FlowExportPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						fmt.Printf("received flow export policy from naples: %v, %+v", naples, st)
						return err
					}

					if len(naplesPol) != len(testFwSpecList) {
						fmt.Printf("received flow export policy from naples: %v, %v \n", naples, naplesPol)
						return fmt.Errorf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), len(testFwSpecList))
					}

					if err := cmpExportPolicy(naples, venicePolicy, naplesPol); err != nil {
						return err
					}
				}
				return nil
			}, 180, 2).Should(BeNil(), "failed to find flow export policy")

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
					fmt.Printf("policy exists after delete, %+v \n", pl)
					return fmt.Errorf("policy exists after delete, %+v", pl)
				}

				for _, naples := range ts.tu.NaplesNodes {
					By(fmt.Sprintf("verify flow export policy in %v", naples))

					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/flowexports/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					fmt.Printf("naples-%v: policy  %+v\n", naples, st)

					var naplesPol []tpmprotos.FlowExportPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						fmt.Printf("received flow export policy from naples:%v, %+v \n", naples, st)
						return err
					}

					if len(naplesPol) != 0 {
						fmt.Printf("received flow export policy from naples:%v, %+v \n", naples, naplesPol)
						return fmt.Errorf("invalid number of policy in %v, got %d, expected 0", naples, len(naplesPol))
					}
				}
				return nil
			}, 180, 2).Should(BeNil(), "failed to verify flow export policy")
		})

		It("Should create/delete multiple flow export policy with the same collector", func() {
			Skip("Test skipped till agents move to to agg watch and all the refcounting is done on the controllers. Since Agent will not have any state")
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.MustGetLoggedInContext(pctx)

			testFwSpecList := make([]monitoring.FlowExportPolicySpec, tpm.MaxNumCollectorsPerPolicy)
			var expFlowRules int

			for i := 0; i < tpm.MaxNumCollectorsPerPolicy; i++ {
				testFwSpecList[i] = monitoring.FlowExportPolicySpec{
					VrfName:  globals.DefaultVrf,
					Interval: "10s",
					Format:   monitoring.FlowExportPolicySpec_Ipfix.String(),

					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{fmt.Sprintf("192.168.100.%d", i+1)},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{fmt.Sprintf("192.168.200.%d", i+1)},
							},
							AppProtoSel: &monitoring.AppProtoSelector{
								ProtoPorts: []string{"tcp/5500-5503"},
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
				// expected match flows
				expFlowRules += 4
			}

			// use token api to get NAPLES access credentials
			nodeAuthFile, err := utils.GetNodeAuthTokenTempFile(ctx, apiGwAddr, []string{"*"})
			Expect(err).ShouldNot(HaveOccurred())
			defer os.Remove(nodeAuthFile)

			for i := 0; i < tpm.MaxNumCollectorsPerPolicy; i++ {
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
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/flowexports/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					fmt.Printf("naples-%v: policy  %+v\n", naples, st)

					var naplesPol []*tpmprotos.FlowExportPolicy
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

					fmt.Printf("received debug info %+v \n", string(st))
					if err := json.Unmarshal([]byte(st), &naplesDbg); err != nil {
						fmt.Printf("received flow export debug from naples: %v, %+v \n", naples, st)
						return err
					}

					if len(naplesDbg.FlowRuleTable) != expFlowRules {
						err := fmt.Errorf("received %d(%+v) rules, expected %v", len(naplesDbg.FlowRuleTable), naplesDbg.FlowRuleTable, tpm.MaxNumCollectorsPerPolicy)
						fmt.Print(err)
						return err
					}

					if len(naplesDbg.CollectorTable) != tpm.MaxNumCollectorsPerPolicy {
						err := fmt.Errorf("received %d collectors, expected 2", len(naplesDbg.CollectorTable))
						fmt.Print(err)
						return err
					}

					for i := 0; i < tpm.MaxNumCollectorsPerPolicy; i++ {
						if len(naplesDbg.CollectorTable[i].PolicyNames) != tpm.MaxNumCollectorsPerPolicy {
							err := fmt.Errorf("received %d policynames in collector, expected 2", len(naplesDbg.CollectorTable[i].PolicyNames))
							fmt.Print(err)
							return err
						}
						if len(naplesDbg.FlowRuleTable[i].PolicyNames) != 1 {
							err := fmt.Errorf("received %d policynames in flow-rule, expected 1", len(naplesDbg.FlowRuleTable))
							fmt.Print(err)
							return err
						}
					}

				}
				return nil
			}, 180, 2).Should(BeNil(), "failed to find flow export policy")

			for i := 0; i < tpm.MaxNumCollectorsPerPolicy; i++ {
				expRules := tpm.MaxNumCollectorsPerPolicy - i - 1
				expCollectors := tpm.MaxNumCollectorsPerPolicy - i*2
				expFlowRules -= 4

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
						st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/flowexports/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
						fmt.Printf("naples-%v: policy  %+v\n", naples, st)

						var naplesPol []tpmprotos.FlowExportPolicy
						if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
							err := fmt.Errorf("received flow export policy from naples: %v, %+v", naples, st)
							fmt.Print(err)
							return err
						}

						if len(naplesPol) != expRules {
							fmt.Printf("received flow export policy from naples: %v, %v\n", naples, naplesPol)
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
							err := fmt.Errorf("received flow export debug from naples: %v, %+v", naples, st)
							fmt.Print(err)
							return err
						}

						if len(naplesDbg.FlowRuleTable) != expFlowRules {
							err := fmt.Errorf("received %d rules, expected %v", len(naplesDbg.FlowRuleTable), expRules)
							fmt.Print(err)
							return err
						}

						for j := 0; j < expFlowRules; j++ {
							if len(naplesDbg.FlowRuleTable[j].PolicyNames) != 1 {
								err := fmt.Errorf("received %d policynames, expected 1", len(naplesDbg.FlowRuleTable))
								fmt.Print(err)
								return err
							}
						}

						if len(naplesDbg.CollectorTable) != expCollectors {
							err := fmt.Errorf("received %d collectors, expected %d", len(naplesDbg.CollectorTable), expCollectors)
							fmt.Print(err)
							return err
						}

						for j := 0; j < expCollectors; j++ {
							if len(naplesDbg.CollectorTable[j].PolicyNames) != 1 {
								err := fmt.Errorf("received %d policynames in collector, expected 2", len(naplesDbg.CollectorTable[j].PolicyNames))
								fmt.Print(err)

								return err
							}
						}
					}
					return nil
				}, 180, 2).Should(BeNil(), "failed to find flow export policy")
			}

		})

		It("Should create/delete multiple flow export policy with the same match-rule", func() {
			Skip("Test skipped till agents move to to agg watch and all the refcounting is done on the controllers. Since Agent will not have any state")
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.MustGetLoggedInContext(pctx)

			testFwSpecList := make([]monitoring.FlowExportPolicySpec, tpm.MaxNumCollectorsPerPolicy)

			for i := 0; i < tpm.MaxNumCollectorsPerPolicy; i++ {
				testFwSpecList[i] = monitoring.FlowExportPolicySpec{
					VrfName:  globals.DefaultVrf,
					Interval: "10s",
					Format:   monitoring.FlowExportPolicySpec_Ipfix.String(),
					MatchRules: []*monitoring.MatchRule{
						{
							Src: &monitoring.MatchSelector{
								IPAddresses: []string{"192.168.20.11"},
							},
							Dst: &monitoring.MatchSelector{
								IPAddresses: []string{"192.168.200.11"},
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

			for i := 0; i < tpm.MaxNumCollectorsPerPolicy; i++ {
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

			// use token api to get NAPLES access credentials
			nodeAuthFile, err := utils.GetNodeAuthTokenTempFile(ctx, apiGwAddr, []string{"*"})
			Expect(err).ShouldNot(HaveOccurred())
			defer os.Remove(nodeAuthFile)

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
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/flowexports/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					fmt.Printf("naples-%v: policy  %+v\n", naples, st)

					var naplesPol []tpmprotos.FlowExportPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						By(fmt.Sprintf("received flow export policy from naples: %v, %+v", naples, st))
						return err
					}

					fmt.Printf("naples-%v: policy %+v\n", naples, naplesPol)

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

					if len(naplesDbg.FlowRuleTable) != 1 {
						err := fmt.Errorf("received %d rules, expected 1", len(naplesDbg.FlowRuleTable))
						fmt.Print(err)
						return err
					}

					fmt.Printf("received  info %+v \n", naplesDbg)

					if len(naplesDbg.CollectorTable) != tpm.MaxNumCollectorsPerPolicy {
						err := fmt.Errorf("received %d collectors, expected %d", len(naplesDbg.CollectorTable), tpm.MaxNumCollectorsPerPolicy)
						fmt.Print(err)
						return err
					}

					for i := 0; i < tpm.MaxNumCollectorsPerPolicy; i++ {
						if len(naplesDbg.CollectorTable[i].PolicyNames) != 1 {
							err := fmt.Errorf("received %d policynames in collector, expected 1", len(naplesDbg.CollectorTable[i].PolicyNames))
							fmt.Print(err)
							return err
						}
					}

					if len(naplesDbg.FlowRuleTable) != 1 {
						err := fmt.Errorf("received %+v flows, expected 1", naplesDbg.FlowRuleTable)
						fmt.Print(err)
						return err
					}

					if len(naplesDbg.FlowRuleTable[0].PolicyNames) != tpm.MaxNumCollectorsPerPolicy {
						err := fmt.Errorf("received %+v policynames in flow-rule, expected 1", naplesDbg.FlowRuleTable[0].PolicyNames)
						fmt.Print(err)
						return err
					}

				}
				return nil
			}, 180, 2).Should(BeNil(), "failed to find flow export policy")

			for i := 0; i < tpm.MaxNumCollectorsPerPolicy; i++ {
				expRules := 1 - i
				expCollectors := tpm.MaxNumCollectorsPerPolicy - 1 - i

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
						st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/flowexports/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
						fmt.Printf("naples-%v: policy  %+v\n", naples, st)

						var naplesPol []tpmprotos.FlowExportPolicy
						if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
							By(fmt.Sprintf("received flow export policy from naples: %v, %+v", naples, st))
							return err
						}

						fmt.Printf("naples-%v: policy %+v \n", naples, naplesPol)

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
							err := fmt.Errorf("received %d rules, expected %v", len(naplesDbg.FlowRuleTable), expRules)
							fmt.Print(err)
							return err
						}

						for j := 0; j < expRules; j++ {
							if len(naplesDbg.FlowRuleTable[j].PolicyNames) != 1 {
								err := fmt.Errorf("received %d policynames, expected 1", len(naplesDbg.FlowRuleTable))
								fmt.Print(err)
								return err
							}
						}

						if len(naplesDbg.CollectorTable) != expCollectors {
							err := fmt.Errorf("received %d collectors, expected %d", len(naplesDbg.CollectorTable), expCollectors)
							fmt.Print(err)
							return err
						}

						for j := 0; j < expCollectors; j++ {
							if len(naplesDbg.CollectorTable[j].PolicyNames) != 1 {
								err := fmt.Errorf("received %d policynames in collector, expected 1", len(naplesDbg.CollectorTable[j].PolicyNames))
								fmt.Print(err)
								return err
							}
						}

					}
					return nil
				}, 180, 2).Should(BeNil(), "failed to find flow export policy")
			}

		})

		It("Should receive ipfix templates in collector", func() {
			Skip("Test skipped temporarily till agent sends ipfix template packtets.")

			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.MustGetLoggedInContext(pctx)
			testFwSpecList := make([]monitoring.FlowExportPolicySpec, tpm.MaxNumCollectorsPerPolicy)
			collectors := make([]struct {
				addr string
				port string
				ch   chan ipfix.Message
			}, tpm.MaxNumCollectorsPerPolicy)

			conn, err := net.Dial("udp", "8.8.8.8:80")
			Expect(err).ShouldNot(HaveOccurred())
			defer conn.Close()
			localAddr := conn.LocalAddr().(*net.UDPAddr)

			for i := 0; i < tpm.MaxNumCollectorsPerPolicy; i++ {
				addr, ch, err := server.NewServer(pctx, localAddr.IP.String()+":0")
				Expect(err).ShouldNot(HaveOccurred())
				s := strings.Split(addr, ":")
				Expect(s).Should(HaveLen(2), "invalid addr/port")
				collectors[i].addr = s[0]
				collectors[i].port = s[1]
				collectors[i].ch = ch
			}

			for i := 0; i < tpm.MaxNumCollectorsPerPolicy; i++ {
				testFwSpecList[i] = monitoring.FlowExportPolicySpec{
					VrfName:          globals.DefaultVrf,
					Interval:         "10s",
					TemplateInterval: "1m",
					Format:           monitoring.FlowExportPolicySpec_Ipfix.String(),
					Exports: []monitoring.ExportConfig{
						{
							Destination: collectors[i].addr,
							Transport:   fmt.Sprintf("UDP/%v", collectors[i].port),
						},
					},
				}

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

			nodeAuthFile, err := utils.GetNodeAuthTokenTempFile(ctx, apiGwAddr, []string{"*"})
			Expect(err).ShouldNot(HaveOccurred())
			defer os.Remove(nodeAuthFile)

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
					st := ts.tu.LocalCommandOutput(fmt.Sprintf("curl -s -k --key %s --cert %s https://%s:8888/api/telemetry/flowexports/", nodeAuthFile, nodeAuthFile, ts.tu.NameToIPMap[naples]))
					fmt.Printf("naples-%v: policy  %+v\n", naples, st)

					var naplesPol []tpmprotos.FlowExportPolicy
					if err := json.Unmarshal([]byte(st), &naplesPol); err != nil {
						By(fmt.Sprintf("failed to unmarshal %v, %v", string(st), err))
						return err
					}

					fmt.Printf("naples-%v: policy %+v\n", naples, naplesPol)

					if len(naplesPol) != len(testFwSpecList) {
						By(fmt.Sprintf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), len(testFwSpecList)))
						return fmt.Errorf("invalid number of policy in %v, got %d, expected %d", naples, len(naplesPol), len(testFwSpecList))
					}

				}
				return nil
			}, 180, 2).Should(BeNil(), "failed to find flow export policy")

			for i := 0; i < tpm.MaxNumCollectorsPerPolicy; i++ {
				for _ = range ts.tu.NaplesNodes {
					select {
					case m, ok := <-collectors[i].ch:
						hdr := m.Header
						Expect(ok).Should(BeTrue())
						Expect(m.TemplateRecords).Should(HaveLen(3), "expected 3 templates, got %v", len(m.TemplateRecords))
						Expect(int(hdr.Version)).Should(Equal(0x0a), "invalid version %v", hdr.Version)
						Expect(int(hdr.SequenceNumber)).Should(Equal(0), "invalid sequence number %v", hdr.SequenceNumber)
						Expect(int(hdr.DomainID)).Should(Equal(0), "invalid domain id %v", hdr.DomainID)
						t, err := vflow.CreateTemplateMsg()
						Expect(err).Should(BeNil())
						Expect(int(hdr.Length)).Should(Equal(len(t)), "invalid length %v", hdr.Length)

					case <-time.After(time.Second * 30):
						Expect(false).Should(BeTrue(), "timed-out to receive template")
					}
				}
			}

		})

		It("validate flow export policy", func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.MustGetLoggedInContext(pctx)

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
							MatchRules: []*monitoring.MatchRule{
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
							MatchRules: []*monitoring.MatchRule{
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
							MatchRules: []*monitoring.MatchRule{
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
							MatchRules: []*monitoring.MatchRule{
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
							MatchRules: []*monitoring.MatchRule{
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
					name: "invalid template interval",
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
							Interval:         "10s",
							TemplateInterval: "1h",
							Format:           "IPFIX",
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
					name: "invalid template interval",
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
							Interval:         "10s",
							TemplateInterval: "1s",
							Format:           "IPFIX",
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
				{
					name: "port range",
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
							MatchRules: []*monitoring.MatchRule{
								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.1"},
									},

									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"TCP/1000-1005"},
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
										ProtoPorts: []string{"TCP/1010-1012,1013"},
									},
								},
							},
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
					name: "duplicate export with diff proro-port",
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
							MatchRules: []*monitoring.MatchRule{
								{
									Src: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.1"},
									},

									Dst: &monitoring.MatchSelector{
										IPAddresses: []string{"1.1.1.2"},
									},
									AppProtoSel: &monitoring.AppProtoSelector{
										ProtoPorts: []string{"udp/1000"},
									},
								},
							},
							Exports: []monitoring.ExportConfig{
								{
									Destination: "192.168.100.1",
									Transport:   "udp/5055",
								},
								{
									Destination: "192.168.100.1",
									Transport:   "udp/5065",
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

		It("validate max. collectors", func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.MustGetLoggedInContext(pctx)

			policy := &monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					Interval:         "10s",
					TemplateInterval: "5m",
					Format:           "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.1",
							Transport:   "UDP/5055",
						},
						{
							Destination: "192.168.100.1",
							Transport:   "UDP/6055",
						},
						{
							Destination: "192.168.100.1",
							Transport:   "UDP/7055",
						},
					},
				},
			}

			// create should fail for more than 2 collectors
			policy.Name = "test-max-collector"
			_, err := flowExpClient.Create(ctx, policy)
			Expect(err).ShouldNot(BeNil(), "policy create didn't fail for invalid number of collectors")
		})

		It("Check max unique session and update of transport should succeed", func() {
			ctx := ts.tu.MustGetLoggedInContext(context.Background())
			initFlowExportMap := make(map[string]monitoring.FlowExportPolicy)
			fe := monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					Interval:         "10s",
					TemplateInterval: "5m",
					Format:           "IPFIX",
				},
			}

			for i := 0; i < tpm.MaxUniqueNumCollectors+1; i++ {
				fe.Name = fmt.Sprintf("unique-flowexport-%d", i+1)
				fe.Spec.Exports = []monitoring.ExportConfig{
					{
						Destination: fmt.Sprintf("192.168.%d.1", i),
						Transport:   "UDP/5545",
					},
				}

				By(fmt.Sprintf("Creating FlowExport %v", fe.Name))
				_, err := flowExpClient.Create(ctx, &fe)
				if i < tpm.MaxUniqueNumCollectors {
					Expect(err).ShouldNot(HaveOccurred())
					initFlowExportMap[fe.GetName()] = fe
				} else {
					Expect(err).Should(HaveOccurred())
				}
			}
			for i := 0; i < tpm.MaxUniqueNumCollectors; i++ {
				key := fmt.Sprintf("unique-flowexport-%d", i+1)
				By(fmt.Sprintf("Update flowexport proto port for %v", key))
				if tfe, ok := initFlowExportMap[key]; ok {
					tfe.Spec.Exports[0].Transport = "udp/5533"
					_, err := flowExpClient.Update(ctx, &tfe)
					Expect(err).ShouldNot(HaveOccurred())
				}
			}
		})

		It("validate max. flow export policy", func() {
			pctx, cancel := context.WithTimeout(context.Background(), 5*time.Minute)
			defer cancel()
			ctx := ts.tu.MustGetLoggedInContext(pctx)

			policy := &monitoring.FlowExportPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "flowExportPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Tenant:    globals.DefaultTenant,
				},

				Spec: monitoring.FlowExportPolicySpec{
					Interval:         "10s",
					TemplateInterval: "5m",
					Format:           "IPFIX",
					Exports: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.1",
							Transport:   "UDP/5055",
						},
						{
							Destination: "192.168.100.2",
							Transport:   "UDP/6055",
						},
					},
				},
			}

			for i := 0; i < tpm.MaxNumExportPolicy; i++ {
				policy.Name = fmt.Sprintf("test-policy-%d", i)
				_, err := flowExpClient.Create(ctx, policy)
				Expect(err).Should(BeNil(), fmt.Sprintf("failed to create  %v, %v", policy.Name, err))
			}

			// new policy create should fail
			policy.Name = "test-policy-fail"
			_, err := flowExpClient.Create(ctx, policy)
			Expect(err).ShouldNot(BeNil(), fmt.Sprintf("policy create didn't fail, %v", err))

		})
	})
})

// cmpExportPolicy compare policy in Naples to that in Venice
func cmpExportPolicy(naples string, vp []*monitoring.FlowExportPolicy, np []*tpmprotos.FlowExportPolicy) error {
	naplesPolicyMap := map[string]*tpmprotos.FlowExportPolicy{}
	for _, p := range np {
		naplesPolicyMap[p.GetKey()] = p
	}
	venicePolicyMap := map[string]*monitoring.FlowExportPolicy{}
	for _, p := range vp {
		venicePolicyMap[p.GetKey()] = p
	}

	if len(naplesPolicyMap) != len(venicePolicyMap) {
		err := fmt.Errorf("policy didn't match in %v, got %v expected %v", naples, len(naplesPolicyMap),
			len(venicePolicyMap))
		fmt.Print(err)
		return err
	}

	for k, v := range venicePolicyMap {
		n, ok := naplesPolicyMap[k]
		if !ok {
			err := fmt.Errorf("failed to find %v in naples-%v", k, naples)
			fmt.Print(err)
			return err
		}
		vspec := v.Spec
		nspec := n.Spec

		// empty match-rule
		if vspec.MatchRules == nil && nspec.MatchRules == nil {
			return nil
		}

		// compare fields
		if len(vspec.MatchRules) != len(nspec.MatchRules) {
			err := fmt.Errorf("match-rules didnt match in %v, got %v expected %v",
				naples, len(nspec.MatchRules), len(vspec.MatchRules))
			fmt.Print(err)
			return err
		}

		for i, v := range vspec.MatchRules {
			n := nspec.MatchRules[i]

			if len(v.Dst.IPAddresses) != len(n.Dst.Addresses) {
				err := fmt.Errorf("matchrule-dst[%d] length didn't match in %v, got %v expected %v", i, naples,
					n.Dst.String(), v.Dst.String())
				fmt.Print(err)
				return err
			}

			if !reflect.DeepEqual(v.Dst.IPAddresses, n.Dst.Addresses) {
				err := fmt.Errorf("matchrule-dst[%d] didn't match in %v, got %v expected %v", i, naples,
					n.Dst.String(), v.Dst.String())
				fmt.Print(err)
				return err
			}

			if len(v.AppProtoSel.ProtoPorts) != len(n.Dst.ProtoPorts) {
				err := fmt.Errorf("matchrule-proto-port[%d] length didn't match in %v, got %v expected %v", i, naples,
					n.Dst.String(), v.AppProtoSel.String())
				fmt.Print(err)
				return err
			}

			for i, vp := range v.AppProtoSel.ProtoPorts {
				if vp != fmt.Sprintf("%v/%v", n.Dst.ProtoPorts[i].Protocol, n.Dst.ProtoPorts[i].Port) {
					err := fmt.Errorf("matchrule-proto-port[%d]  didn't match in %v, got %v expected %v", i, naples,
						vp, n.Dst.ProtoPorts[i].String())
					fmt.Print(err)
					return err
				}
			}

			if len(v.Src.IPAddresses) != len(n.Src.Addresses) {
				err := fmt.Errorf("matchrule-src[%d] count didn't match in %v, got %v expected %v", i, naples,
					n.Src.String(), v.Src.String())
				fmt.Print(err)
				return err
			}

			if !reflect.DeepEqual(v.Src.IPAddresses, n.Src.Addresses) {
				err := fmt.Errorf("matchrule-src[%d] didn't match in %v, got %v expected %v", i, naples,
					n.Src.String(), v.Src.String())
				fmt.Print(err)
				return err
			}
		}

		if vspec.Interval != nspec.Interval {
			err := fmt.Errorf("interval didn't match in %v, got %v expected %v", naples,
				nspec.Interval, vspec.Interval)
			fmt.Print(err)
			return err
		}

		if vspec.Format != nspec.Format {
			err := fmt.Errorf("format didn't match in %v, got %v expected %v", naples,
				nspec.Format, vspec.Format)
			fmt.Print(err)
			return err
		}

		if vspec.TemplateInterval != nspec.TemplateInterval {
			err := fmt.Errorf("template interval didn't match in %v, got %v expected %v", naples,
				nspec.TemplateInterval, vspec.TemplateInterval)
			fmt.Print(err)
			return err
		}

		if len(vspec.Exports) != len(nspec.Exports) {
			err := fmt.Errorf("exports didn't match in %v, got %v expected %v", naples,
				len(nspec.Exports), len(vspec.Exports))
			fmt.Print(err)
			return err
		}

		for i, v := range vspec.Exports {
			n := nspec.Exports[i]
			components := strings.Split(v.Transport, "/")

			if components[0] != n.Transport.Protocol {
				err := fmt.Errorf("exports[%d] protocol didn't match in %v, got %v expected %v", i, naples,
					components[0], n.Transport.Protocol)
				fmt.Print(err)
				return err
			}
			if components[1] != n.Transport.Port {
				err := fmt.Errorf("exports[%d] port didn't match in %v, got %v expected %v", i, naples,
					components[1], n.Transport.Port)
				fmt.Print(err)
				return err
			}
		}
	}

	return nil
}
