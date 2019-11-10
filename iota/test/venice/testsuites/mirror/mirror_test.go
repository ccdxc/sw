// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package mirror_test

import (
	"context"
	"strings"
	"time"
	"fmt"
	"math/rand"
    "github.com/pensando/sw/iota/test/venice/iotakit"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

type trafficMap struct {
    name string
    proto string
    port int
    wln int
    pStr string
    msc *iotakit.MirrorSessionCollection
}

func GenerateSimpleTrafficMap(proto string, workloadNum int) trafficMap {
    var entry trafficMap
    entry.wln = workloadNum
    switch p := proto; p {
    case "icmp": 
        entry.proto = "icmp"
        entry.port = 0
        entry.pStr = "icmp/0/0"
    case "tcp": 
        entry.port = rand.Intn(32000)+1024
        entry.proto = "tcp"
        entry.pStr = fmt.Sprintf("tcp/%d",entry.port)
    case "udp": 
        entry.port = rand.Intn(32000)+1024
        entry.proto = "udp"
        entry.pStr = fmt.Sprintf("udp/%d",entry.port)
    default:
        log.Errorf("no match for protocol: %s",proto)
    }
    return entry
}

func GenerateSimpleTrafficMaps(pattern string, tmCount int, wlCount int) []trafficMap {
    var tMaps []trafficMap
    switch p := pattern; p {
    case "tcp":
        for i := 0; i<tmCount; i++ {
            tMaps = append(tMaps, GenerateSimpleTrafficMap("tcp",i % wlCount))
        }
    case "udp":
        for i := 0; i<tmCount; i++ {
            tMaps = append(tMaps, GenerateSimpleTrafficMap("udp",i % wlCount))
        }
    case "icmp":
        for i := 0; i<tmCount; i++ {
            tMaps = append(tMaps, GenerateSimpleTrafficMap("icmp",i % wlCount))
        }
    default:
        //mixed case
        ptypes := [3] string {"icmp","tcp","udp"}
        for i := 0; i<tmCount; i++ {
            tMaps = append(tMaps, GenerateSimpleTrafficMap(ptypes[i % len(ptypes)],i % wlCount))
        }
    }
    return tMaps
}

func VerifyMirrorSessionTraffic(veniceCollector *iotakit.VeniceNodeCollection, wlPair *iotakit.WorkloadPairCollection, entry trafficMap) () {
    ctx, cancel := context.WithCancel(context.Background())
    tcpdumpDone := make(chan error)
    var tcpdumpOut string

    go func() {
        var err error
        tcpdumpOut, err = veniceCollector.CaptureGRETCPDump(ctx)
        tcpdumpDone <- err
    }()

    //Sleep for a while so that tcpdump starts
    time.Sleep(2 * time.Second)
    switch wlp := entry.proto; wlp {
    case "icmp":
            ts.model.Action().PingPairs(wlPair)
    case "tcp":
            ts.model.Action().TCPSession(wlPair, entry.port)
    case "udp":
            ts.model.Action().UDPSession(wlPair, entry.port)
    default:
        log.Errorf("no traffic match for protocol: %s",entry.proto)
    }
    time.Sleep(1 * time.Second)

    cancel()
    <-tcpdumpDone
    Expect(strings.Contains(tcpdumpOut, "GREv0, length")).Should(BeTrue())
    return
}


var _ = Describe("mirror tests", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()
		Eventually(func() error {
			return ts.model.Action().VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
	})
	Context("Mirror tests", func() {
		It("tags:sanity=true tags:type=basic;datapath=true;duration=short Mirror packets to collector and check TCPDUMP", func() {
			if ts.tb.HasNaplesSim() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			veniceCollector := ts.model.VeniceNodes().Leader()
			// add permit rules for workload pairs
			workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			msc := ts.model.NewMirrorSession("test-mirror").AddRulesForWorkloadPairs(workloadPairs, "icmp/0/0")
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
			ts.model.Action().PingPairs(workloadPairs)
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
			ts.model.Action().PingPairs(workloadPairs)
			cancel()
			<-tcpdumpDone

			Expect(strings.Contains(output, "GREv0, length")).Should(BeTrue())

			// Delete the Mirror session
			Expect(msc.Delete()).Should(Succeed())

		})
	})
	Context("Scale mirror tests", func() {
        	It("tags:sanity=true tags:type=basic;datapath=true;duration=short Mirror packets to collector and check TCPDUMP at scale", func() {
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

			for i := 0; i < len(tMaps); i++ {
                log.Infof("updating mirror session %s",tMaps[i].name)
                tMaps[i].msc.ClearCollectors()
                tMaps[i].msc.AddRulesForWorkloadPairs(workloadPairs, tMaps[i].pStr)
                nextTMap := tMaps[(i+1) % len(tMaps)]
                log.Infof("adding collector with workflow %d",nextTMap.wln)
                tMaps[i].msc.AddVeniceCollector(veniceCollector, "udp/4545", nextTMap.wln)
                Expect(tMaps[i].msc.Commit()).Should(Succeed())
                log.Infof("running %s traffic for new workflow %d",nextTMap.pStr,nextTMap.wln)
                VerifyMirrorSessionTraffic(veniceCollector, workloadPairs, nextTMap)
            }

			for i := 0; i < len(tMaps); i++ {
                log.Infof("deleting mirror session %s",tMaps[i].name)
	    		Expect(tMaps[i].msc.Delete()).Should(Succeed())
            }

		})
	})
})
