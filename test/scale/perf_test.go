package scale

import (
	"context"
	"fmt"
	"strconv"
	"strings"
	"sync"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api/generated/apiclient"
)

var _ = Describe("baseline cluster health", func() {
	Context("baseline cluster health is good", func() {
		It("pen-base should be running on all nodes", func() {
			for _, ip := range ts.tu.VeniceNodeIPs {
				out := ts.tu.CommandOutput(ip, "docker ps -q -f Name=pen-cmd")
				Expect(out).ShouldNot(BeEmpty(), "pen-cmd container should be running on %s", ip)
			}
		})
		It("kubernetes indicated all pods to be Running", func() {
			out := strings.Split(ts.tu.VIPCommandOutput("kubectl get pods --no-headers"), "\n")
			for _, line := range out {
				Expect(line).Should(ContainSubstring("Running"), "pod should be in Running state")
			}
		})
		// TODO check that all the expected pods are running.
		// For this, the test needs the expected topology as input
	})
})

var _ = Describe("Network object create delete throughput", func() {
	var (
		numNetObj                 = 1000
		numConcurrentRESTSessions = 100
		createNetObjects          func()
		deleteNetObjects          func()
	)
	BeforeEach(func() {
		apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
		apiClient, err := apiclient.NewRestAPIClient(apiGwAddr)
		Expect(err).ShouldNot(HaveOccurred())
		netIf := apiClient.NetworkV1().Network()

		createNetObjects = func() {
			wg := sync.WaitGroup{}
			concorrencyCh := make(chan bool, numConcurrentRESTSessions)
			for i := 1; i <= numNetObj; i++ {
				NetName := "testN" + strconv.Itoa(i)
				nw := network.Network{
					TypeMeta: api.TypeMeta{Kind: "Network"},
					ObjectMeta: api.ObjectMeta{
						Name:      NetName,
						Namespace: "",
						Tenant:    "defaultTenant",
					},
					Spec: network.NetworkSpec{
						Type:        network.NetworkType_Bridged.String(),
						IPv4Subnet:  "100.100.100.0/24",
						IPv4Gateway: "100.100.100.1",
					},
					Status: network.NetworkStatus{},
				}
				wg.Add(1)
				go func() {
					defer func() { <-concorrencyCh }()
					defer wg.Done()
					defer GinkgoRecover()
					concorrencyCh <- true
					_, err := netIf.Create(context.Background(), &nw)
					Expect(err).ShouldNot(HaveOccurred(), nw.String(), err)
				}()
			}
			wg.Wait()
		}

		deleteNetObjects = func() {
			wg := sync.WaitGroup{}
			concorrencyCh := make(chan bool, numConcurrentRESTSessions)
			for i := 1; i <= numNetObj; i++ {
				NetName := "testN" + strconv.Itoa(i)
				obj := api.ObjectMeta{
					Name:      NetName,
					Namespace: "",
					Tenant:    "defaultTenant"}
				wg.Add(1)
				go func() {
					defer func() { <-concorrencyCh }()
					defer wg.Done()
					defer GinkgoRecover()
					concorrencyCh <- true
					_, err := netIf.Delete(context.Background(), &obj)
					Expect(err).ShouldNot(HaveOccurred(), obj.Name, err)
				}()
			}
			wg.Wait()
		}

	})
	Measure("Network Object create and delete in APIGW-APIServer", func(b Benchmarker) {
		b.Time("creation of objects", func() {
			createNetObjects()
		})
		b.Time("deletion of network objects", func() {
			deleteNetObjects()
		})
	}, 3)

	BeforeEach(func() {
		// Wait for NPM to stabilize from previous events - else it might be processing stuff from previous tests
		Eventually(func() int {
			npmStats1 := npmDebugStats()
			time.Sleep(100 * time.Millisecond)
			npmStats2 := npmDebugStats()
			return npmStats2.DebugStats["DeleteNetwork"] - npmStats1.DebugStats["DeleteNetwork"] + npmStats2.DebugStats["CreateNetwork"] - npmStats1.DebugStats["CreateNetwork"]
		}, "15s", "500ms").Should(BeZero())
	})

	Measure("Network Object create and delete in APIGW-APIServer-NPM", func(b Benchmarker) {
		npmStatsBefore := npmDebugStats()
		By(fmt.Sprintf("debugStatus before:%#v", npmStatsBefore))

		b.Time("creation of objects", func() {
			createNetObjects()

			var numCreateEventsInNpm int
			timeout := time.After(10 * time.Second)
			for {
				select {
				case <-time.After(100 * time.Millisecond):
					npmStatsAfter := npmDebugStats()
					numCreateEventsInNpm = npmStatsAfter.DebugStats["CreateNetwork"] - npmStatsBefore.DebugStats["CreateNetwork"]
					if numCreateEventsInNpm == numNetObj {
						return
					}
				case <-timeout:
					Expect(numCreateEventsInNpm).Should(BeNumerically("==", numNetObj))
				}
			}
		})
		b.Time("deletion of network objects", func() {
			deleteNetObjects()

			var numDeleteEventsInNpm int
			timeout := time.After(10 * time.Second)
			for {
				select {
				case <-time.After(100 * time.Millisecond):
					npmStatsAfterDel := npmDebugStats()
					numDeleteEventsInNpm = npmStatsAfterDel.DebugStats["DeleteNetwork"] - npmStatsBefore.DebugStats["DeleteNetwork"]
					if numDeleteEventsInNpm == numNetObj {
						return
					}
				case <-timeout:
					Expect(numDeleteEventsInNpm).Should(BeNumerically("==", numNetObj))
				}
			}
		})
	}, 3)
})
