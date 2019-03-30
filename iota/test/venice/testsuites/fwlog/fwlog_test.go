// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package fwlog_test

import (
	"context"
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/telemetryclient"
)

var _ = Describe("fwlog tests", func() {
	var tclient *telemetryclient.TelemetryClient
	var loggedInCtx context.Context

	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())

		// get telemetry client
		tmc, err := telemetryclient.NewTelemetryClient(ts.tb.GetVeniceURL()[0])
		Expect(err).Should(BeNil())
		tclient = tmc

		ctx, err := ts.tb.VeniceLoggedInCtx()
		Expect(err).Should(BeNil())
		loggedInCtx = ctx

	})

	AfterEach(func() {
		ts.tb.AfterTestCommon()

		// delete test policy if its left over. we can ignore thes error here
		ts.model.SGPolicy("test-policy").Delete()
		ts.model.SGPolicy("default-policy").Delete()

		// recreate default allow policy
		Expect(ts.model.NewSGPolicy("default-policy").AddRule("any", "any", "0-65535", "PERMIT").Commit()).ShouldNot(HaveOccurred())

		// verify policy was propagated correctly
		Eventually(func() error {
			return ts.model.Action().VerifyPolicyStatus(ts.model.SGPolicy("default-policy"))
		}).Should(Succeed())
	})

	Describe("on traffic", func() {
		Context("verify fwlog ", func() {
			It("should log ICMP allow in fwlog", func() {
				if ts.tb.HasNaplesSim() {
					Skip("Disabling on naples sim till shm flag is enabled")
				}

				workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

				Eventually(func() error {
					return ts.model.Action().PingPairs(workloadPairs)
				}).Should(Succeed())

				log.Infof("workload ip address %+v", workloadPairs.ListIpAddr())

				// ping is done during init, don't set time in query
				Eventually(func() error {
					res, err := tclient.Fwlogs(loggedInCtx, &telemetry_query.FwlogsQueryList{
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
						Queries: []*telemetry_query.FwlogsQuerySpec{
							{
								Protocols: []string{"ICMP"},
								Actions:   []string{telemetry_query.FwlogActions_allow.String()},
							},
						},
					})
					if err != nil {
						return err
					}

					for i, r := range res.Results {
						log.Infof("[%-3d] %v ", i+1, r.String())
					}

					for _, ips := range workloadPairs.ListIpAddr() {
						if tclient.CheckIPAddrInFwlog(ips, res.Results) != true {
							err := fmt.Errorf("did not find %v in fwlog", ips)
							log.Errorf("%v", err)
							return err
						}
					}
					return nil
				}).Should(Succeed())
			})

			It("should log TCP/8000 allow in fwlog", func() {
				if ts.tb.HasNaplesSim() {
					Skip("Disabling on naples sim till shm flag is enabled")
				}

				workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
				stime := &api.Timestamp{}
				stime.Parse(time.Now().String())

				Eventually(func() error {
					return ts.model.Action().TCPSession(workloadPairs, 8000)
				}).Should(Succeed())

				log.Infof("workload ip address %+v", workloadPairs.ListIpAddr())

				// check fwlog
				Eventually(func() error {
					res, err := tclient.Fwlogs(loggedInCtx, &telemetry_query.FwlogsQueryList{
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
						Queries: []*telemetry_query.FwlogsQuerySpec{
							{
								DestPorts: []uint32{8000},
								Protocols: []string{"TCP"},
								Actions:   []string{telemetry_query.FwlogActions_allow.String()},
								StartTime: stime,
							},
						},
					})
					if err != nil {
						return err
					}

					for i, r := range res.Results {
						log.Infof("[%-3d] %v ", i+1, r.String())
					}

					for _, ips := range workloadPairs.ListIpAddr() {
						if tclient.CheckIPAddrInFwlog(ips, res.Results) != true {
							err := fmt.Errorf("did not find %+v in fwlog", ips)
							log.Errorf("%v", err)
							return err
						}
					}
					return nil

				}).Should(Succeed())
			})

			It("should log UDP/9000 allow in fwlog", func() {
				if ts.tb.HasNaplesSim() {
					Skip("Disabling on naples sim till shm flag is enabled")
				}

				workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
				stime := &api.Timestamp{}
				stime.Parse(time.Now().String())

				log.Infof("workload ip address %+v", workloadPairs.ListIpAddr())

				Eventually(func() error {
					return ts.model.Action().UDPSession(workloadPairs, 9000)
				}).Should(Succeed())

				// check fwlog
				Eventually(func() error {
					res, err := tclient.Fwlogs(loggedInCtx, &telemetry_query.FwlogsQueryList{
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
						Queries: []*telemetry_query.FwlogsQuerySpec{
							{
								DestPorts: []uint32{9000},
								Protocols: []string{"UDP"},
								Actions:   []string{telemetry_query.FwlogActions_allow.String()},
								StartTime: stime,
							},
						},
					})
					if err != nil {
						return err
					}

					for i, r := range res.Results {
						log.Infof("[%-3d] %v ", i+1, r.String())
					}

					for _, ips := range workloadPairs.ListIpAddr() {
						if tclient.CheckIPAddrInFwlog(ips, res.Results) != true {
							err := fmt.Errorf("did not find %+v in fwlog", ips)
							log.Errorf("%v", err)
							return err
						}
					}
					return nil
				}).Should(Succeed())
			})

			It("should log ICMP deny in fwlog", func() {
				if ts.tb.HasNaplesSim() {
					Skip("Disabling on naples sim till shm flag is enabled")
				}

				Expect(ts.model.SGPolicy("default-policy").Delete()).Should(Succeed())
				stime := &api.Timestamp{}
				stime.Parse(time.Now().String())

				// deny policy
				denyPolicy := ts.model.NewSGPolicy("default-policy").AddRule("any", "any", "ICMP", "DENY")
				denyPolicy.AddRule("any", "any", "", "PERMIT")
				Expect(denyPolicy.Commit()).ShouldNot(HaveOccurred())

				// verify policy was propagated correctly
				Eventually(func() error {
					return ts.model.Action().VerifyPolicyStatus(ts.model.SGPolicy("default-policy"))
				}).Should(Succeed())

				workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
				log.Infof("workload ip address %+v", workloadPairs.ListIpAddr())

				Eventually(func() error {
					return ts.model.Action().PingFails(workloadPairs)
				}).Should(Succeed())

				// check fwlog
				Eventually(func() error {
					res, err := tclient.Fwlogs(loggedInCtx, &telemetry_query.FwlogsQueryList{
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
						Queries: []*telemetry_query.FwlogsQuerySpec{
							{
								Protocols: []string{"ICMP"},
								Actions:   []string{telemetry_query.FwlogActions_deny.String()},
								StartTime: stime,
							},
						},
					})
					if err != nil {
						return err
					}

					for i, r := range res.Results {
						log.Infof("[%-3d] %v ", i+1, r.String())
					}

					for _, ips := range workloadPairs.ListIpAddr() {
						if tclient.CheckIPAddrInFwlog([]string{ips[1], ips[0]}, res.Results) != true {
							err := fmt.Errorf("did not find %+v in fwlog", ips)
							log.Errorf("%v", err)
							return err
						}
					}
					return nil

				}).Should(Succeed())
			})

			It("should log ICMP reject in fwlog", func() {
				Skip("+++skip ")
				if ts.tb.HasNaplesSim() {
					Skip("Disabling on naples sim till shm flag is enabled")
				}

				Expect(ts.model.SGPolicy("default-policy").Delete()).Should(Succeed())
				stime := &api.Timestamp{}
				stime.Parse(time.Now().String())

				// reject policy
				debyPolicy := ts.model.NewSGPolicy("default-policy").AddRule("any", "any", "ICMP", "REJECT")
				debyPolicy.AddRule("any", "any", "", "PERMIT")
				Expect(debyPolicy.Commit()).ShouldNot(HaveOccurred())

				// verify policy was propagated correctly
				Eventually(func() error {
					return ts.model.Action().VerifyPolicyStatus(ts.model.SGPolicy("default-policy"))
				}).Should(Succeed())

				workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
				Eventually(func() error {
					return ts.model.Action().PingFails(workloadPairs)
				}).Should(Succeed())

				// check fwlog
				Eventually(func() error {
					res, err := tclient.Fwlogs(loggedInCtx, &telemetry_query.FwlogsQueryList{
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
						Queries: []*telemetry_query.FwlogsQuerySpec{
							{
								Protocols: []string{"ICMP"},
								Actions:   []string{telemetry_query.FwlogActions_reject.String()},
								StartTime: stime,
							},
						},
					})
					if err != nil {
						return err
					}

					for i, r := range res.Results {
						log.Infof("[%-3d] %v ", i+1, r.String())
					}

					for _, ips := range workloadPairs.ListIpAddr() {
						if tclient.CheckIPAddrInFwlog(ips, res.Results) != true {
							err := fmt.Errorf("did not find %+v in fwlog", ips)
							log.Errorf("%v", err)
							return err
						}
					}
					return nil
				}).Should(Succeed())
			})

			It("should log TCP/8100 deny in fwlog", func() {
				if ts.tb.HasNaplesSim() {
					Skip("Disabling on naples sim till shm flag is enabled")
				}

				Expect(ts.model.SGPolicy("default-policy").Delete()).Should(Succeed())
				stime := &api.Timestamp{}
				stime.Parse(time.Now().String())

				// deny policy
				debyPolicy := ts.model.NewSGPolicy("default-policy").AddRule("any", "any", "TCP", "DENY")
				debyPolicy.AddRule("any", "any", "", "PERMIT")
				Expect(debyPolicy.Commit()).ShouldNot(HaveOccurred())

				// verify policy was propagated correctly
				Eventually(func() error {
					return ts.model.Action().VerifyPolicyStatus(ts.model.SGPolicy("default-policy"))
				}).Should(Succeed())

				workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
				log.Infof("workload ip address %+v", workloadPairs.ListIpAddr())

				Eventually(func() error {
					return ts.model.Action().TCPSessionFails(workloadPairs, 8100)
				}).Should(Succeed())

				// check fwlog
				Eventually(func() error {
					res, err := tclient.Fwlogs(loggedInCtx, &telemetry_query.FwlogsQueryList{
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
						Queries: []*telemetry_query.FwlogsQuerySpec{
							{
								DestPorts: []uint32{8100},
								Protocols: []string{"TCP"},
								Actions:   []string{telemetry_query.FwlogActions_deny.String()},
								StartTime: stime,
							},
						},
					})
					if err != nil {
						return err
					}

					for i, r := range res.Results {
						log.Infof("[%-3d] %v ", i+1, r.String())
					}

					for _, ips := range workloadPairs.ListIpAddr() {
						if tclient.CheckIPAddrInFwlog(ips, res.Results) != true {
							err := fmt.Errorf("did not find %+v in fwlog", ips)
							log.Errorf("%v", err)
							return err
						}
					}
					return nil
				}).Should(Succeed())
			})

			It("should log UDP/9100 deny in fwlog", func() {
				if ts.tb.HasNaplesSim() {
					Skip("Disabling on naples sim till shm flag is enabled")
				}

				// deny policy
				debyPolicy := ts.model.NewSGPolicy("default-policy").AddRule("any", "any", "UDP", "DENY")
				debyPolicy.AddRule("any", "any", "", "PERMIT")
				Expect(debyPolicy.Commit()).ShouldNot(HaveOccurred())

				// verify policy was propagated correctly
				Eventually(func() error {
					return ts.model.Action().VerifyPolicyStatus(ts.model.SGPolicy("default-policy"))
				}).Should(Succeed())

				workloadPairs := ts.model.WorkloadPairs().WithinNetwork()
				stime := &api.Timestamp{}
				stime.Parse(time.Now().String())
				log.Infof("workload ip address %+v", workloadPairs.ListIpAddr())

				Eventually(func() error {
					return ts.model.Action().UDPSessionFails(workloadPairs, 9100)
				}).Should(Succeed())

				// check fwlog
				Eventually(func() error {
					res, err := tclient.Fwlogs(loggedInCtx, &telemetry_query.FwlogsQueryList{
						Tenant:    globals.DefaultTenant,
						Namespace: globals.DefaultNamespace,
						Queries: []*telemetry_query.FwlogsQuerySpec{
							{
								DestPorts: []uint32{9100},
								Protocols: []string{"UDP"},
								Actions:   []string{telemetry_query.FwlogActions_deny.String()},
								StartTime: stime,
							},
						},
					})
					if err != nil {
						return err
					}

					for i, r := range res.Results {
						log.Infof("[%-3d] %v ", i+1, r.String())
					}

					for _, ips := range workloadPairs.ListIpAddr() {
						if tclient.CheckIPAddrInFwlog(ips, res.Results) != true {
							err := fmt.Errorf("did not find %+v in fwlog", ips)
							log.Errorf("%v", err)
							return err
						}
					}
					return nil
				}).Should(Succeed())
			})
		})
	})
})
