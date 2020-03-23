// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package mirror_test

import (
	//"errors"
	"context"
	"fmt"
	"math/rand"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

type trafficMap struct {
	name      string
	proto     string
	flowcount int
	port      int
	wln       int
	pStr      string
	msc       *objects.MirrorSessionCollection
}

func GenerateSimpleTrafficMap(proto string, workloadNum int) trafficMap {
	var entry trafficMap
	entry.proto = proto
	entry.wln = workloadNum
	switch proto {
	case "icmp":
		entry.port = 0
		entry.pStr = "icmp/0"
		entry.flowcount = 1
	case "tcp":
		entry.port = rand.Intn(32000) + 1024
		entry.pStr = fmt.Sprintf("tcp/%d", entry.port)
		entry.flowcount = 1
	case "udp":
		entry.port = rand.Intn(32000) + 1024
		entry.pStr = fmt.Sprintf("udp/%d", entry.port)
		entry.flowcount = 1
	case "maxflow-udp":
		entry.port = rand.Intn(32000) + 1024
		entry.pStr = fmt.Sprintf("udp/%d", entry.port)
		entry.flowcount = 10 //1000
	case "maxflow-tcp":
		entry.port = rand.Intn(32000) + 1024
		entry.pStr = fmt.Sprintf("tcp/%d", entry.port)
		entry.flowcount = 10 //1000
	default:
		log.Errorf("no match for pattern: %s", proto)
	}
	return entry
}

func GenerateSimpleTrafficMaps(pattern string, tmCount int, wlCount int) []trafficMap {
	var tMaps []trafficMap
	if pattern == "mixed" {
		ptypes := [5]string{"icmp", "tcp", "udp", "maxflow-udp", "maxflow-tcp"}
		for i := 0; i < tmCount; i++ {
			tMaps = append(tMaps, GenerateSimpleTrafficMap(ptypes[i%len(ptypes)], i%wlCount))
		}
	} else {
		for i := 0; i < tmCount; i++ {
			tMaps = append(tMaps, GenerateSimpleTrafficMap(pattern, i%wlCount))
		}
	}
	return tMaps
}

func VerifyMirrorSessionTraffic(veniceCollector *objects.VeniceNodeCollection, wlPair *objects.WorkloadPairCollection, entry trafficMap) {
	ctx, cancel := context.WithCancel(context.Background())
	tcpdumpDone := make(chan error)
	var tcpdumpOut string

	go func() {
		var err error
		tcpdumpOut, err = veniceCollector.CaptureGRETCPDump(ctx)
		tcpdumpDone <- err
	}()

	//Sleep for a while so that tcpdump starts
	time.Sleep(3 * time.Second)
	switch entry.proto {
	case "icmp":
		ts.model.PingPairs(wlPair)
	case "tcp":
		ts.model.TCPSession(wlPair, entry.port)
	case "udp":
		ts.model.UDPSession(wlPair, entry.port)
	case "maxflow-udp":
		cmd := fmt.Sprintf("-u --sh-exec 'echo test' -m %d", entry.flowcount)
		ts.model.NetcatWrapper(wlPair, cmd, "-u", entry.port, false, 0, "")
	case "maxflow-tcp":
		cmd := fmt.Sprintf("-m %d", entry.flowcount)
		ts.model.NetcatWrapper(wlPair, cmd, "", entry.port, false, 0, "")
	default:
		log.Errorf("no traffic match for: %s", entry.proto)
	}
	time.Sleep(3 * time.Second)

	cancel()
	<-tcpdumpDone
	//log.Infof("tcpdump output: %s",tcpdumpOut)
	Expect(strings.Contains(tcpdumpOut, "GREv0, length")).Should(BeTrue())
	return
}

var _ = Describe("mirror tests", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
	})
	Context("Flow Mirror tests", func() {
		It("tags:sanity=true tags:type=basic;datapath=true;duration=short Mirror packets to collector and check TCPDUMP", func() {
			if ts.tb.HasNaplesSim() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			veniceCollector := ts.model.VeniceNodes().Leader()
			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			msc := ts.model.NewMirrorSession("test-mirror").AddRulesForWorkloadPairs(workloadPairs, "icmp")
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 0)
			Expect(msc.Commit()).Should(Succeed())

			ctx, cancel := context.WithCancel(context.Background())
			tcpdumpDone := make(chan error)
			var output string
			go func() {
				var err error
				output, err = veniceCollector.CaptureGRETCPDump(ctx)
				tcpdumpDone <- err

			}()

			//Sleep for a while so that tcpdump starts
			time.Sleep(2 * time.Second)
			ts.model.PingPairs(workloadPairs)
			cancel()
			<-tcpdumpDone

			Expect(strings.Contains(output, "GREv0, length")).Should(BeTrue())

			// Clear collectors
			msc.ClearCollectors()

			// Update the collector
			msc.AddVeniceCollector(veniceCollector, "udp/4545", 1)
			Expect(msc.Commit()).Should(Succeed())

			ctx, cancel = context.WithCancel(context.Background())
			tcpdumpDone = make(chan error)
			go func() {
				var err error
				output, err = veniceCollector.CaptureGRETCPDump(ctx)
				tcpdumpDone <- err

			}()

			//Sleep for a while so that tcpdump starts
			time.Sleep(2 * time.Second)
			ts.model.PingPairs(workloadPairs)
			cancel()
			<-tcpdumpDone

			Expect(strings.Contains(output, "GREv0, length")).Should(BeTrue())

			// Delete the Mirror session
			Expect(msc.Delete()).Should(Succeed())

		})
	})
	Context("Flow Mirror tests : scale", func() {
		It("tags:sanity=false tags:type=basic;datapath=true;duration=short Mirror packets to collector and check TCPDUMP at scale", func() {
			if ts.tb.HasNaplesSim() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			veniceCollector := ts.model.VeniceNodes().Leader()
			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

			var tMaps []trafficMap
			//tMaps holds a mapping of protocols to workload ids.
			sessionCount := 8
			tMaps = GenerateSimpleTrafficMaps("mixed", sessionCount, len(workloadPairs.ListIPAddr()))

			//run initial test. adds new mirror session and collector. mix of icmp, udp, tcp.
			for i := 0; i < len(tMaps); i++ {
				tMaps[i].name = fmt.Sprintf("test-mirror-%d-wl-%d-%s", i, tMaps[i].wln, tMaps[i].proto)
				log.Infof("creating new mirror session %s", tMaps[i].name)
				tMaps[i].msc = ts.model.NewMirrorSession(tMaps[i].name).AddRulesForWorkloadPairs(workloadPairs.GetSingleWorkloadPair(tMaps[i].wln), tMaps[i].pStr)
				tMaps[i].msc.AddVeniceCollector(veniceCollector, "udp/4545", tMaps[i].wln)
				Expect(tMaps[i].msc.Commit()).Should(Succeed())
				VerifyMirrorSessionTraffic(veniceCollector, workloadPairs.GetSingleWorkloadPair(tMaps[i].wln), tMaps[i])
			}

			//modify existing session, use different traffic map for each modified session.
			for i := 0; i < len(tMaps); i++ {
				log.Infof("updating mirror session %s", tMaps[i].name)
				tMaps[i].msc.ClearCollectors()
				tMaps[i].msc.AddRulesForWorkloadPairs(workloadPairs, tMaps[i].pStr)
				nextTMap := tMaps[(i+1)%len(tMaps)]
				log.Infof("adding collector with workflow %d", nextTMap.wln)
				tMaps[i].msc.AddVeniceCollector(veniceCollector, "udp/4545", nextTMap.wln)
				Expect(tMaps[i].msc.Commit()).Should(Succeed())
				log.Infof("running %s traffic for new workflow %d", nextTMap.pStr, nextTMap.wln)
				VerifyMirrorSessionTraffic(veniceCollector, workloadPairs, nextTMap)
			}

			//delete sessions.
			for i := 0; i < len(tMaps); i++ {
				log.Infof("deleting mirror session %s", tMaps[i].name)
				Expect(tMaps[i].msc.Delete()).Should(Succeed())
			}
		})
	})
	/*
		Context("Scale mirror test with upgrade", func() {
			It("tags:sanity=true tags:type=basic;datapath=true;duration=short Mirror packets to collector and check TCPDUMP at scale with upgrade", func() {
				if ts.tb.HasNaplesSim() {
					Skip("Disabling on naples sim till traffic issue is debugged")
				}

				veniceCollector := ts.model.VeniceNodes().Leader()
				// add permit rules for workload pairs
				workloadPairs := ts.model.WorkloadPairs().WithinNetwork()

				var tMaps []trafficMap
				//tMaps holds a mapping of protocols to workload ids.
				//protocol can be tcp, udp, icmp, or mixed.
				sessionCount := 8
				tMaps = GenerateSimpleTrafficMaps("mixed",sessionCount,len(workloadPairs.ListIPAddr()))

				for i := 0; i < len(tMaps); i++ {
					tMaps[i].name = fmt.Sprintf("test-mirror-%d-wl-%d-%s",i,tMaps[i].wln,tMaps[i].proto)
					log.Infof("creating new mirror session %s",tMaps[i].name)
					tMaps[i].msc = ts.model.NewMirrorSession(tMaps[i].name).AddRulesForWorkloadPairs(workloadPairs.GetSingleWorkloadPair(tMaps[i].wln), tMaps[i].pStr)
					tMaps[i].msc.AddVeniceCollector(veniceCollector, "udp/4545", tMaps[i].wln)
					Expect(tMaps[i].msc.Commit()).Should(Succeed())
					VerifyMirrorSessionTraffic(veniceCollector, workloadPairs.GetSingleWorkloadPair(tMaps[i].wln), tMaps[i])
				}

				//upgrade naples
				rollout, err := ts.model.GetRolloutObject(ts.scaleData)
				Expect(err).ShouldNot(HaveOccurred())

				workloadPairs = ts.model.WorkloadPairs().WithinNetwork().Any(40)
				log.Infof(" Length workloadPairs %v", len(workloadPairs.ListIPAddr()))
				Expect(len(workloadPairs.ListIPAddr()) != 0).Should(BeTrue())

				err = ts.model.PerformRollout(rollout)
				Expect(err).ShouldNot(HaveOccurred())
				rerr := make(chan bool)
				go func() {
					_ = ts.model.TCPSessionWithOptions(workloadPairs, 8000, "180s", 100)
					log.Infof("TCP SESSION TEST COMPLETE")
					rerr <- true
					return
				}()

				// verify rollout is successful
				Eventually(func() error {
					return ts.model.VerifyRolloutStatus(rollout.Name)
				}).Should(Succeed())
				log.Infof("Rollout Completed. Waiting for Fuz tests to complete..")
				errWaitingForFuz := func() error {
					select {
						case <-rerr:
							log.Infof("Verified DataPlane using fuz connections")
							return nil
						case <-time.After(time.Duration(900) * time.Second):
							log.Infof("Timeout while waiting for fuz api to return")
							return errors.New("Timeout while waiting for fuz api to return")
					}
				}
				Expect(errWaitingForFuz()).ShouldNot(HaveOccurred())

				for i := 0; i < len(tMaps); i++ {
					VerifyMirrorSessionTraffic(veniceCollector, workloadPairs.GetSingleWorkloadPair(tMaps[i].wln), tMaps[i])
				}

				for i := 0; i < len(tMaps); i++ {
					log.Infof("deleting mirror session %s",tMaps[i].name)
					Expect(tMaps[i].msc.Delete()).Should(Succeed())
				}
			})
		})
	*/
})
