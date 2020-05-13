// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package equinix_test

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"
	"text/tabwriter"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	fromIPcomb = []string{"workload-ip", "workload-subnet", "any"}
	toIPcomb   = []string{"workload-ip", "workload-subnet", "any"}
	// TODO: when PDS agent support any proto
	// protocomb  = []string{"tcp", "udp", "icmp", "any"}
	protocomb = []string{"tcp", "udp", "icmp"}
	portcomp  = []string{"8000", "8001,8010", "8005-8010", "any"}
)

func getNetworkCollection() (*objects.NetworkCollection, error) {

	// add permit rules for workload pairs
	ten, err := ts.model.ConfigClient().ListTenant()
	if err != nil {
		return nil, err
	}

	if len(ten) == 0 {
		return nil, fmt.Errorf("Not enough tenants to list networks")
	}

	nws, err := ts.model.ConfigClient().ListNetwork(ten[0].Name)

	nwc := objects.NewNetworkCollectionFromNetworks(ts.model.ConfigClient(), nws)

	return nwc, nil

}

type evaluator func() error

// checkEventually checks if a condition is met repeatedly
func checkEventually(eval evaluator) error {
	pollInterval := time.Second
	timeoutInterval := time.Second * 30

	timer := time.Now()
	timeout := time.After(timeoutInterval)

	// loop till we reach timeout interval
	for {
		select {
		case <-time.After(pollInterval):
			err := eval()
			if err == nil {
				return nil
			}
		case <-timeout:
			// eveluate one last time
			err := eval()
			if err != nil {
				_, file, line, _ := runtime.Caller(1)

				log.Errorf("%s:%d: Evaluator timed out after %v. Err: %v", filepath.Base(file), line, time.Since(timer), err)
				return err
			}

			return nil
		}
	}
}

// testPolicy tests policy for a combination
func testWhitelistPolicy(fromIP, toIP, proto, port string) error {
	//workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
	nwc, err := getNetworkCollection()
	Expect(err).Should(Succeed())

	selNetwork := nwc.Any(1)
	workloadPairs := ts.model.WorkloadPairs().OnNetwork(selNetwork.Subnets()[0]).Any(1)
	spc := ts.model.NetworkSecurityPolicy("test-policy").DeleteAllRules()

	// add allow all rule for workload followed by deny all rule
	spc = spc.AddRuleForWorkloadCombo(workloadPairs, fromIP, toIP, proto, port, "PERMIT")
	// TODO: when PDS agent support any proto
	// spc = spc.AddRule("any", "any", "any", "DENY")
	spc.SetTenant(selNetwork.GetTenant())
	Expect(spc.Commit()).Should(Succeed())
	//Now attach ingress and egress
	selNetwork.SetIngressSecurityPolicy(spc)
	selNetwork.SetEgressSecurityPolicy(spc)

	// verify policy was propagated correctly
	err = checkEventually(func() error {
		return ts.model.VerifyPolicyStatus(spc)
	})
	if err != nil {
		return err
	}

	err = checkEventually(func() error {
		switch proto {
		case "any":
			fallthrough // fallthrough to ICMP test
		case "icmp":
			aerr := ts.model.PingPairs(workloadPairs)
			if aerr != nil {
				return aerr
			}
		case "udp":
			fallthrough // TCP and UDP have common tests
		case "tcp":
			portRanges := strings.Split(port, ",")
			for _, portRange := range portRanges {
				if portRange == "any" {
					portRange = "8000"
				}
				vals := strings.Split(portRange, "-")
				portMin, _ := strconv.Atoi(vals[0])
				portMax := portMin
				if len(vals) > 1 {
					portMax, _ = strconv.Atoi(vals[1])
				}

				if proto == "udp" {
					aerr := ts.model.UDPSession(workloadPairs, portMin)
					if aerr != nil {
						return err
					}
					aerr = ts.model.UDPSession(workloadPairs, portMax)
					if aerr != nil {
						return aerr
					}
					// TODO: Enable when pdsagent supports any proto
					if port != "any" && false {
						aerr = ts.model.UDPSessionFails(workloadPairs, portMin-1)
						if aerr != nil {
							return aerr
						}
						aerr = ts.model.UDPSessionFails(workloadPairs, portMax+1)
						if aerr != nil {
							return aerr
						}
					}
				} else {
					aerr := ts.model.TCPSession(workloadPairs, portMin)
					if aerr != nil {
						return aerr
					}
					aerr = ts.model.TCPSession(workloadPairs, portMax)
					if aerr != nil {
						return aerr
					}
					// TODO: Enable when pdsagent supports any proto
					if port != "any" && false {
						aerr = ts.model.TCPSessionFails(workloadPairs, portMin-1)
						if aerr != nil {
							return aerr
						}
						aerr = ts.model.TCPSessionFails(workloadPairs, portMax+1)
						if aerr != nil {
							return aerr
						}
					}
				}
			}
		}
		return nil
	})
	if err != nil {
		return err
	}

	return nil
}

// testBlacklistPolicy tests if black list policies work
func testBlacklistPolicy(fromIP, toIP, proto, port string) error {
	//workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
	nwc, err := getNetworkCollection()
	Expect(err).Should(Succeed())

	selNetwork := nwc.Any(1)
	workloadPairs := ts.model.WorkloadPairs().OnNetwork(selNetwork.Subnets()[0]).Any(1)
	spc := ts.model.NetworkSecurityPolicy("test-policy").DeleteAllRules()

	// add deny rule for workload followed by allow all rule
	spc = spc.AddRuleForWorkloadCombo(workloadPairs, fromIP, toIP, proto, port, "DENY")
	// TODO: when PDS agent support any proto
	// spc = spc.AddRule("any", "any", "any", "DENY")
	spc.SetTenant(selNetwork.GetTenant())
	Expect(spc.Commit()).Should(Succeed())
	//Now attach ingress and egress
	selNetwork.SetIngressSecurityPolicy(spc)
	selNetwork.SetEgressSecurityPolicy(spc)

	// verify policy was propagated correctly
	err = checkEventually(func() error {
		return ts.model.VerifyPolicyStatus(spc)
	})
	if err != nil {
		return err
	}

	err = checkEventually(func() error {
		switch proto {
		case "any":
			fallthrough // fallthrough to ICMP test
		case "icmp":
			aerr := ts.model.PingFails(workloadPairs)
			if aerr != nil {
				return aerr
			}
		case "udp":
			fallthrough // TCP and UDP have common tests
		case "tcp":
			portRanges := strings.Split(port, ",")
			for _, portRange := range portRanges {
				if portRange == "any" {
					portRange = "8000"
				}
				vals := strings.Split(portRange, "-")
				portMin, _ := strconv.Atoi(vals[0])
				portMax := portMin
				if len(vals) > 1 {
					portMax, _ = strconv.Atoi(vals[1])
				}

				if proto == "udp" {
					aerr := ts.model.UDPSessionFails(workloadPairs, portMin)
					if aerr != nil {
						return err
					}
					aerr = ts.model.UDPSessionFails(workloadPairs, portMax)
					if aerr != nil {
						return aerr
					}
					// TODO: Enable when pdsagent supports any proto
					if port != "any" && false {
						aerr = ts.model.UDPSession(workloadPairs, portMin-1)
						if aerr != nil {
							return aerr
						}
						aerr = ts.model.UDPSession(workloadPairs, portMax+1)
						if aerr != nil {
							return aerr
						}
					}
				} else {
					aerr := ts.model.TCPSessionFails(workloadPairs, portMin)
					if aerr != nil {
						return aerr
					}
					aerr = ts.model.TCPSessionFails(workloadPairs, portMax)
					if aerr != nil {
						return aerr
					}
					// TODO: Enable when pdsagent supports any proto
					if port != "any" && false {
						aerr = ts.model.TCPSession(workloadPairs, portMin-1)
						if aerr != nil {
							return aerr
						}
						aerr = ts.model.TCPSession(workloadPairs, portMax+1)
						if aerr != nil {
							return aerr
						}
					}
				}
			}
		}
		return nil
	})
	if err != nil {
		return err
	}

	return nil
}

func getNetworkCollections() ([]*objects.NetworkCollection, error) {

	// add permit rules for workload pairs
	ten, err := ts.model.ConfigClient().ListTenant()
	if err != nil {
		return nil, err
	}

	if len(ten) == 0 {
		return nil, fmt.Errorf("Not enough tenants to list networks")
	}

	nws, err := ts.model.ConfigClient().ListNetwork(ten[0].Name)

	nwc := objects.NewNetworkCollectionsFromNetworks(ts.model.ConfigClient(), nws)

	return nwc, nil
}

var _ = Describe("scale policy", func() {
	var startTime time.Time
	BeforeEach(func() {
		//verify cluster is in good health
		startTime = time.Now().UTC()

		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		// delete the default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).ShouldNot(HaveOccurred())
	})
	AfterEach(func() {
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
		nwcs, err := getNetworkCollections()
		Expect(err).Should(BeNil())

		// Delete the policies. Reset the network references
		for index := 0; index < len(nwcs) ; index++ {
			nwcs[index].SetIngressSecurityPolicy(nil)
			nwcs[index].SetEgressSecurityPolicy(nil)
			ts.model.NetworkSecurityPolicy(fmt.Sprintf("ingress%v%v", index, index)).Delete()
			ts.model.NetworkSecurityPolicy(fmt.Sprintf("ingress%v%v", index, index+1)).Delete()
			ts.model.NetworkSecurityPolicy(fmt.Sprintf("egress%v%v", index, index)).Delete()
			ts.model.NetworkSecurityPolicy(fmt.Sprintf("egress%v%v", index, index+1)).Delete()
		}
		// recreate default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())
	})
	Context("Scale Policy tests", func() {
		var (
			protoList = []string{"tcp", "udp", "icmp"}
			portList = []string{"80", "53", "0"}
		)
		It("scale policy", func() {
			Skip("Skip scale policy tests until datapath issues are addressed")
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}
			//Get all the networks as individual collection
			nwcs, err := getNetworkCollections()
			Expect(err).Should(BeNil())
			var networkPrefixList []string
			for _, nwc := range nwcs {
				networkPrefixList = append(networkPrefixList, nwc.Subnets()[0].IPPrefix())
			}
			log.Infof("PREFIX::::: %v", networkPrefixList)

			// Add Policy for all networks (source:'x' destination:all excluding 'x')
			for index := 0; index < len(nwcs) ; index++ {
				nwc := nwcs[index]
				sourcePrefixList := []string{networkPrefixList[index]}
				var targetPrefixList []string
				for _, prefix := range networkPrefixList {
					if  prefix != sourcePrefixList[0] {
						targetPrefixList = append(targetPrefixList, prefix)
					}
				}
				//Create user provider Ingress policy
				ingress_policy := ts.model.NewNetworkSecurityPolicy(fmt.Sprintf("ingress%v%v",index,index)).AddRuleForSubnets(targetPrefixList, sourcePrefixList, "any", "any", "PERMIT")
				ingress_policy.Add(ts.model.NewNetworkSecurityPolicy(fmt.Sprintf("ingress%v%v",index,index+1)).AddRuleForSubnets(
					targetPrefixList, sourcePrefixList,portList[index % len(portList)], protoList[index % len(protoList)], "PERMIT"))
				//Create user provider Egress policy
				egress_policy := ts.model.NewNetworkSecurityPolicy(fmt.Sprintf("egress%v%v",index,index)).AddRuleForSubnets(sourcePrefixList, targetPrefixList, "any", "any", "PERMIT")
				egress_policy.Add(ts.model.NewNetworkSecurityPolicy(fmt.Sprintf("egress%v%v",index,index+1)).AddRuleForSubnets(
					sourcePrefixList, targetPrefixList,portList[index % len(portList)], protoList[index % len(protoList)], "PERMIT"))

				//Set tenant for both policies
				ingress_policy.SetTenant(nwc.GetTenant())
				egress_policy.SetTenant(nwc.GetTenant())
				//Commit the ingress and egress policy
				Expect(ingress_policy.Commit()).Should(Succeed())
				Expect(egress_policy.Commit()).Should(Succeed())
				//Apply the ingress and egress policies
				Expect(nwc.SetIngressSecurityPolicy(ingress_policy)).Should(BeNil())
				Expect(nwc.SetEgressSecurityPolicy(egress_policy)).Should(BeNil())
				//Verify if the policies are available in datapath
				Eventually(func() error { return ts.model.VerifyPolicyStatus(ingress_policy) }).Should(Succeed())
				Eventually(func() error { return ts.model.VerifyPolicyStatus(egress_policy) }).Should(Succeed())

			}

			//Data Traffic Tests. From network 'x' to all other networks as per the installed policy
			for index := 0; index < len(nwcs) ; index++ {
				nwc := nwcs[index]
				sourceNetwork := nwc.Subnets()[0]
				var targetNetwork []*objects.Network
				for _, network := range nwcs {
					//Collect the target networks
					if network.Subnets()[0] != sourceNetwork {
						targetNetwork = append(targetNetwork, network.Subnets()[0])
					}
				}
				//Create workload pairs in same network
				wp1 := ts.model.WorkloadPairs().OnNetwork(sourceNetwork)
				//Create workload pair in different network and in same naples
				wps2 := ts.model.Workloads().LocalPairsAcrossNetwork(wp1.Pairs[0].First).Any(1)
				//Create workload pair in different network and in different naples
				wps3 := ts.model.Workloads().RemotePairsAcrossNetwork(wp1.Pairs[0].First).Any(1)

				//Verify the traffic does not flow within same subnet within and across hosts
				//Uncomment the below once the datapath for dual policy is fixed
				Expect(ts.model.PingFails(wp1)).Should(BeNil())

				//Verify the traffic across networks according to policies
				port, err := strconv.Atoi(portList[index % len(portList)])
				Expect(err).Should(BeNil())
				proto := protoList[index % len(protoList)]
				switch proto {
				case "tcp":
					//Verify TCP traffic flow
					Expect(ts.model.TCPSession(wps2, port)).Should(BeNil())
					Expect(ts.model.TCPSession(wps2.ReversePairs(), port)).Should(BeNil())
					Expect(ts.model.TCPSession(wps3, port)).Should(BeNil())
					Expect(ts.model.TCPSession(wps3.ReversePairs(), port)).Should(BeNil())

					//Verify other traffic fails
					Expect(ts.model.PingFails(wps2)).Should(BeNil())
					Expect(ts.model.PingFails(wps3)).Should(BeNil())
					Expect(ts.model.UDPSessionFails(wps2, port)).Should(BeNil())
					Expect(ts.model.UDPSessionFails(wps3, port)).Should(BeNil())
				case "udp":
					//Verify UDP traffic flow
					Expect(ts.model.UDPSession(wps2, port)).Should(BeNil())
					Expect(ts.model.UDPSession(wps2.ReversePairs(), port)).Should(BeNil())
					Expect(ts.model.UDPSession(wps3, port)).Should(BeNil())
					Expect(ts.model.UDPSession(wps3.ReversePairs(), port)).Should(BeNil())

					//Verify other traffic fails
					Expect(ts.model.PingFails(wps2)).Should(BeNil())
					Expect(ts.model.PingFails(wps3)).Should(BeNil())
					Expect(ts.model.TCPSessionFails(wps2, port)).Should(BeNil())
					Expect(ts.model.TCPSessionFails(wps3, port)).Should(BeNil())
				case "icmp":
					//Verify ICMP traffic flow
					Expect(ts.model.PingPairs(wps2)).Should(BeNil())
					Expect(ts.model.PingPairs(wps3)).Should(BeNil())

					//Verify other traffic fails
					Expect(ts.model.UDPSessionFails(wps2, port)).Should(BeNil())
					Expect(ts.model.UDPSessionFails(wps2.ReversePairs(), port)).Should(BeNil())
					Expect(ts.model.TCPSessionFails(wps2, port)).Should(BeNil())
					Expect(ts.model.TCPSessionFails(wps2.ReversePairs(), port)).Should(BeNil())
					Expect(ts.model.UDPSessionFails(wps3, port)).Should(BeNil())
					Expect(ts.model.UDPSessionFails(wps3.ReversePairs(), port)).Should(BeNil())
					Expect(ts.model.TCPSessionFails(wps3, port)).Should(BeNil())
					Expect(ts.model.TCPSessionFails(wps3.ReversePairs(), port)).Should(BeNil())
				default:
				}
			}
		})
	})
})

var _ = Describe("single policy", func() {
	var startTime time.Time
	BeforeEach(func() {
		//verify cluster is in good health
		startTime = time.Now().UTC()

		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		// delete the default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).ShouldNot(HaveOccurred())
	})
	AfterEach(func() {
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))

		nwc, err := getNetworkCollection()
		Expect(err).Should(Succeed())

		//Reset ingress and egress policies
		nwc.SetIngressSecurityPolicy(nil)
		nwc.SetEgressSecurityPolicy(nil)

		// Delete the policies. we can ignore the error here
		for index := 1; index <= 12; index++ {
			ts.model.NetworkSecurityPolicy(fmt.Sprintf("p%v", index)).Delete()
		}
		ts.model.DefaultNetworkSecurityPolicy().Delete()
		// recreate default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())
	})
	Context("Single Policy tests", func() {
		It("Multiple rule combinations", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}
			selNetwork, err := getNetworkCollection()
			Expect(err).Should(Succeed())
			workloadPairWithinNetwork := ts.model.WorkloadPairs().WithinNetwork().Any(1)
			workloadPairAcrossNetwork := ts.model.WorkloadPairs().AcrossNetwork(workloadPairWithinNetwork.Pairs[0].First).Any(1)

			//Case 1: No rule Specified
			policy1 := ts.model.NewNetworkSecurityPolicy("p1")
			policy2 := ts.model.NewNetworkSecurityPolicy("p2")
			policy1.SetTenant(selNetwork.GetTenant())
			policy2.SetTenant(selNetwork.GetTenant())
			Expect(policy1.Commit()).Should(Succeed())
			Expect(policy2.Commit()).Should(Succeed())
			selNetwork.SetIngressSecurityPolicy(policy1)
			selNetwork.SetEgressSecurityPolicy(policy2)
			// Verify if policies are successfully applied
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy1) }).Should(Succeed())
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy2) }).Should(Succeed())
			// Verify traffic does not flow
			Expect(ts.model.PingFails(workloadPairWithinNetwork)).Should(BeNil())
			Expect(ts.model.PingFails(workloadPairAcrossNetwork)).Should(BeNil())
			// Reset policies
			selNetwork.SetIngressSecurityPolicy(nil)
			selNetwork.SetEgressSecurityPolicy(nil)

			//Case 2: One rule with Permit Action Specified
			policy1 = ts.model.NewNetworkSecurityPolicy("p3").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "TCP", "80", "PERMIT")
			policy2 = ts.model.NewNetworkSecurityPolicy("p4").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "TCP", "80", "PERMIT")
			policy1.SetTenant(selNetwork.GetTenant())
			policy2.SetTenant(selNetwork.GetTenant())
			Expect(policy1.Commit()).Should(Succeed())
			Expect(policy2.Commit()).Should(Succeed())
			selNetwork.SetIngressSecurityPolicy(policy1)
			selNetwork.SetEgressSecurityPolicy(policy2)
			// Verify if policies are successfully applied
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy1) }).Should(Succeed())
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy2) }).Should(Succeed())
			// Verify only TCP traffic flows
			Expect(ts.model.TCPSession(workloadPairWithinNetwork, 80)).Should(BeNil())
			Expect(ts.model.PingFails(workloadPairWithinNetwork)).Should(BeNil())
			// Reset policies
			selNetwork.SetIngressSecurityPolicy(nil)
			selNetwork.SetEgressSecurityPolicy(nil)

			//Case 3: One rule with Deny Action Specified
			policy1 = ts.model.NewNetworkSecurityPolicy("p5").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "TCP", "80", "DENY")
			policy2 = ts.model.NewNetworkSecurityPolicy("p6").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "TCP", "80", "DENY")
			policy1.SetTenant(selNetwork.GetTenant())
			policy2.SetTenant(selNetwork.GetTenant())
			Expect(policy1.Commit()).Should(Succeed())
			Expect(policy2.Commit()).Should(Succeed())
			selNetwork.SetIngressSecurityPolicy(policy1)
			selNetwork.SetEgressSecurityPolicy(policy2)
			// Verify if policies are successfully applied
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy1) }).Should(Succeed())
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy2) }).Should(Succeed())
			// Verify only TCP traffic flows
			Expect(ts.model.TCPSessionFails(workloadPairWithinNetwork, 80)).Should(BeNil())
			Expect(ts.model.PingFails(workloadPairWithinNetwork)).Should(BeNil())
			// Reset policies
			selNetwork.SetIngressSecurityPolicy(nil)
			selNetwork.SetEgressSecurityPolicy(nil)

			//Case 4: Two rules with with selective permit
			policy1 = ts.model.NewNetworkSecurityPolicy("p7").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "udp", "9100", "PERMIT")
			policy1.AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork, "workload-subnet", "workload-subnet", "icmp", "", "PERMIT")
			policy1.AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork.ReversePairs(), "workload-subnet", "workload-subnet", "icmp", "", "PERMIT")
			policy2 = ts.model.NewNetworkSecurityPolicy("p8").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "udp", "9100", "PERMIT")
			policy2.AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork, "workload-subnet", "workload-subnet", "icmp", "", "PERMIT")
			policy2.AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork.ReversePairs(), "workload-subnet", "workload-subnet", "icmp", "", "PERMIT")
			policy1.SetTenant(selNetwork.GetTenant())
			policy2.SetTenant(selNetwork.GetTenant())
			Expect(policy1.Commit()).Should(Succeed())
			Expect(policy2.Commit()).Should(Succeed())
			selNetwork.SetIngressSecurityPolicy(policy1)
			selNetwork.SetEgressSecurityPolicy(policy2)
			// Verify if policies are successfully applied
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy1) }).Should(Succeed())
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy2) }).Should(Succeed())
			// Verify only UDP 9100 and ICMP flows
			Expect(ts.model.PingPairs(workloadPairAcrossNetwork)).Should(BeNil())
			Expect(ts.model.UDPSession(workloadPairWithinNetwork, 9100)).Should(BeNil())
			Expect(ts.model.TCPSessionFails(workloadPairWithinNetwork, 80)).Should(BeNil())
			// Reset policies
			selNetwork.SetIngressSecurityPolicy(nil)
			selNetwork.SetEgressSecurityPolicy(nil)

			//Case 5: Two rules with conflicting rules allowing traffic
			policy1 = ts.model.NewNetworkSecurityPolicy("p9").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "any", "any", "PERMIT")
			policy1.AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "any", "any", "DENY")
			policy2 = ts.model.NewNetworkSecurityPolicy("p10").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "any", "any", "PERMIT")
			policy2.AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "any", "any", "DENY")
			policy1.SetTenant(selNetwork.GetTenant())
			policy2.SetTenant(selNetwork.GetTenant())
			Expect(policy1.Commit()).Should(Succeed())
			Expect(policy2.Commit()).Should(Succeed())
			selNetwork.SetIngressSecurityPolicy(policy1)
			selNetwork.SetEgressSecurityPolicy(policy2)
			// Verify if policies are successfully applied
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy1) }).Should(Succeed())
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy2) }).Should(Succeed())
			// Verify only UDP 9100 and ICMP flows
			Expect(ts.model.UDPSession(workloadPairWithinNetwork, 9100)).Should(BeNil())
			Expect(ts.model.TCPSession(workloadPairWithinNetwork, 80)).Should(BeNil())
			Expect(ts.model.PingPairs(workloadPairWithinNetwork)).Should(BeNil())
			// Reset policies
			selNetwork.SetIngressSecurityPolicy(nil)
			selNetwork.SetEgressSecurityPolicy(nil)

			//Case 6: Two rules with conflicting information Deny Traffic
			policy1 = ts.model.NewNetworkSecurityPolicy("p11").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "any", "any", "DENY")
			policy1.AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "any", "any", "PERMIT")
			policy2 = ts.model.NewNetworkSecurityPolicy("p12").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "any", "any", "DENY")
			policy2.AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "any", "any", "PERMIT")
			policy1.SetTenant(selNetwork.GetTenant())
			policy2.SetTenant(selNetwork.GetTenant())
			Expect(policy1.Commit()).Should(Succeed())
			Expect(policy2.Commit()).Should(Succeed())
			selNetwork.SetIngressSecurityPolicy(policy1)
			selNetwork.SetEgressSecurityPolicy(policy2)
			// Verify if policies are successfully applied
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy1) }).Should(Succeed())
			Eventually(func() error { return ts.model.VerifyPolicyStatus(policy2) }).Should(Succeed())
			// Verify only UDP 9100 and ICMP flows
			Expect(ts.model.UDPSessionFails(workloadPairWithinNetwork, 9100)).Should(BeNil())
			Expect(ts.model.TCPSessionFails(workloadPairWithinNetwork, 80)).Should(BeNil())
			Expect(ts.model.PingFails(workloadPairWithinNetwork)).Should(BeNil())
			// Reset policies
			selNetwork.SetIngressSecurityPolicy(nil)
			selNetwork.SetEgressSecurityPolicy(nil)
		})
	})
})

var _ = Describe("multiple policies", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()

		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		// delete the default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).ShouldNot(HaveOccurred())
	})
	AfterEach(func() {
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
		nwc, err := getNetworkCollection()
		Expect(err).Should(Succeed())

		//Now attach ingress and egres
		nwc.SetIngressSecurityPolicy(nil)
		nwc.SetEgressSecurityPolicy(nil)

		// Delete the policies. we can ignore the error here
		ts.model.NetworkSecurityPolicy("p1").Delete()
		ts.model.NetworkSecurityPolicy("p2").Delete()
		ts.model.NetworkSecurityPolicy("p3").Delete()
		ts.model.NetworkSecurityPolicy("p4").Delete()
		ts.model.DefaultNetworkSecurityPolicy().Delete()

		// recreate default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())
	})
	Context("user-provider policy tests", func() {
		It("Allow TCP, Deny UDP, Same subnet", func() {
			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			selNetwork, err := getNetworkCollection()
			Expect(err).Should(Succeed())
			networkA := selNetwork.Subnets()[0]

			//Workloadpair in SubnetA
			workloadPairWithinNetwork := ts.model.WorkloadPairs().OnNetwork(networkA).Any(1)

			//Add Rules to deny UDP and allow all other traffic
			ingress_policy := ts.model.NewNetworkSecurityPolicy("p1").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			ingress_policy.Add(ts.model.NewNetworkSecurityPolicy("p2").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "17", "9100", "DENY"))

			egress_policy := ts.model.NewNetworkSecurityPolicy("p3").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork.ReversePairs(), "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			egress_policy.Add(ts.model.NewNetworkSecurityPolicy("p4").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "17", "9100", "DENY"))

			ingress_policy.SetTenant(selNetwork.GetTenant())
			egress_policy.SetTenant(selNetwork.GetTenant())
			Expect(ingress_policy.Commit()).Should(Succeed())
			Expect(egress_policy.Commit()).Should(Succeed())
			selNetwork.SetIngressSecurityPolicy(ingress_policy)
			selNetwork.SetEgressSecurityPolicy(egress_policy)

			// Verify if policies are successfully applied
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ingress_policy)
			}).Should(Succeed())
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(egress_policy)
			}).Should(Succeed())

			//Skipping data path checks till validation of wildcard fix
			return

			//Verify UDP session fails
			Eventually(func() error {
				return ts.model.UDPSessionFails(workloadPairWithinNetwork, 9100)
			}).Should(Succeed())

			// verify TCP connections
			Eventually(func() error {
				return ts.model.TCPSession(workloadPairWithinNetwork, 80)
			}).Should(Succeed())

			// verify TCP connections in reverse direction
			Eventually(func() error {
				return ts.model.TCPSession(workloadPairWithinNetwork.ReversePairs(), 80)
			}).Should(Succeed())
		})
		It("Allow TCP, Deny UDP, Across subnets", func() {
			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			selNetwork, err := getNetworkCollection()
			Expect(err).Should(Succeed())
			networkA := selNetwork.Subnets()[0]

			//Workloadpair in SubnetA
			workloadPairWithinNetwork := ts.model.WorkloadPairs().OnNetwork(networkA).Any(1)

			//Workloadpair across SubnetA, SubnetB
			workloadPairAcrossNetwork := ts.model.WorkloadPairs().AcrossNetwork(
				workloadPairWithinNetwork.Pairs[0].First).Any(1)

			//Add Rules between SubnetA and SubnetB (Deny UDP 9100)
			ingress_policy := ts.model.NewNetworkSecurityPolicy("p1").AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork, "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			ingress_policy.Add(ts.model.NewNetworkSecurityPolicy("p2").AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork, "workload-subnet", "workload-subnet", "17", "9100", "DENY"))

			egress_policy := ts.model.NewNetworkSecurityPolicy("p3").AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork.ReversePairs(), "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			egress_policy.Add(ts.model.NewNetworkSecurityPolicy("p4").AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork.ReversePairs(), "workload-subnet", "workload-subnet", "17", "9100", "DENY"))

			ingress_policy.SetTenant(selNetwork.GetTenant())
			egress_policy.SetTenant(selNetwork.GetTenant())
			Expect(ingress_policy.Commit()).Should(Succeed())
			Expect(egress_policy.Commit()).Should(Succeed())
			selNetwork.SetIngressSecurityPolicy(ingress_policy)
			selNetwork.SetEgressSecurityPolicy(egress_policy)

			// Verify if policies are successfully applied
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ingress_policy)
			}).Should(Succeed())
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(egress_policy)
			}).Should(Succeed())

			//Skipping data path checks till validation of wildcard fix
			return

			//Verify UDP 9100 session fails
			Eventually(func() error {
				return ts.model.UDPSessionFails(workloadPairAcrossNetwork, 9100)
			}).Should(Succeed())

			//Verify UDP 9200 session succeeds
			Eventually(func() error {
				return ts.model.UDPSession(workloadPairAcrossNetwork, 9200)
			}).Should(Succeed())
		})
		It("Provider AllowAll, User DenyAll, Samesubnet", func() {
			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			selNetwork, err := getNetworkCollection()
			Expect(err).Should(Succeed())
			networkA := selNetwork.Subnets()[0]

			//Workloadpair in SubnetA
			workloadPairWithinNetwork := ts.model.WorkloadPairs().OnNetwork(networkA).Any(1)

			//Add Provider (allow-all) rule and User (deny-all) rule
			ingress_policy := ts.model.NewNetworkSecurityPolicy("p1").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			ingress_policy.Add(ts.model.NewNetworkSecurityPolicy("p2").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "any", "any", "DENY"))

			egress_policy := ts.model.NewNetworkSecurityPolicy("p3").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork.ReversePairs(), "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			egress_policy.Add(ts.model.NewNetworkSecurityPolicy("p4").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "any", "any", "DENY"))

			ingress_policy.SetTenant(selNetwork.GetTenant())
			egress_policy.SetTenant(selNetwork.GetTenant())
			Expect(ingress_policy.Commit()).Should(Succeed())
			Expect(egress_policy.Commit()).Should(Succeed())
			selNetwork.SetIngressSecurityPolicy(ingress_policy)
			selNetwork.SetEgressSecurityPolicy(egress_policy)

			// Verify if policy successfully applied
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ingress_policy)
			}).Should(Succeed())
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(egress_policy)
			}).Should(Succeed())

			//Skipping data path checks till validation of wildcard fix
			return

			//Verify Ping fails
			Eventually(func() error {
				return ts.model.PingFails(workloadPairWithinNetwork)
			}).Should(Succeed())

			// verify TCP connections in forward direction fail
			Eventually(func() error {
				return ts.model.TCPSessionFails(workloadPairWithinNetwork, 80)
			}).Should(Succeed())

			// verify TCP connections in reverse direction fail
			Eventually(func() error {
				return ts.model.TCPSessionFails(workloadPairWithinNetwork.ReversePairs(), 80)
			}).Should(Succeed())
		})
		It("Provider AllowAll, User DenyAll, across Subnets", func() {
			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			selNetwork, err := getNetworkCollection()
			Expect(err).Should(Succeed())
			networkA := selNetwork.Subnets()[0]

			//Workloadpair in SubnetA
			workloadPairWithinNetwork := ts.model.WorkloadPairs().OnNetwork(networkA).Any(1)
			//Workloadpair across SubnetA, SubnetB
			workloadPairAcrossNetwork := ts.model.WorkloadPairs().AcrossNetwork(
				workloadPairWithinNetwork.Pairs[0].First).Any(1)

			//Add Provider (allow-all) rule and User (deny-all) rule
			ingress_policy := ts.model.NewNetworkSecurityPolicy("p1").AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork, "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			ingress_policy.Add(ts.model.NewNetworkSecurityPolicy("p2").AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork, "any", "any", "any", "any", "DENY"))

			egress_policy := ts.model.NewNetworkSecurityPolicy("p3").AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork.ReversePairs(), "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			egress_policy.Add(ts.model.NewNetworkSecurityPolicy("p4").AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork, "any", "any", "any", "any", "DENY"))

			ingress_policy.SetTenant(selNetwork.GetTenant())
			egress_policy.SetTenant(selNetwork.GetTenant())
			Expect(ingress_policy.Commit()).Should(Succeed())
			Expect(egress_policy.Commit()).Should(Succeed())
			selNetwork.SetIngressSecurityPolicy(ingress_policy)
			selNetwork.SetEgressSecurityPolicy(egress_policy)

			// Verify if policy successfully applied
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ingress_policy)
			}).Should(Succeed())
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(egress_policy)
			}).Should(Succeed())

			//Skipping data path checks till validation of wildcard fix
			return

			//Verify Ping fails for across subnet
			Eventually(func() error {
				return ts.model.PingFails(workloadPairAcrossNetwork)
			}).Should(Succeed())
			log.Infof("")

			// verify TCP connection fails between workload pairs in different network
			Eventually(func() error {
				return ts.model.TCPSessionFails(workloadPairAcrossNetwork, 80)
			}).Should(Succeed())

			// verify TCP connection (reverse) fails between workload pairs in different network
			Eventually(func() error {
				return ts.model.TCPSessionFails(workloadPairAcrossNetwork.ReversePairs(), 80)
			}).Should(Succeed())
		})
		It("User allow TCP/SSH ingress, Allowall egress", func() {
			Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).Should(Succeed())
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}

			selNetwork, err := getNetworkCollection()
			Expect(err).Should(Succeed())
			networkA := selNetwork.Subnets()[0]

			//Workloadpair in SubnetA
			workloadPairWithinNetwork := ts.model.WorkloadPairs().OnNetwork(networkA).Any(1)
			//Workloadpair across SubnetA, SubnetB
			workloadPairAcrossNetwork := ts.model.WorkloadPairs().AcrossNetwork(
				workloadPairWithinNetwork.Pairs[0].First).Any(1)

			// Ingress. Provider Allow B->A, A->A, User Allow all TCP (80/443)
			ingress_policy := ts.model.NewNetworkSecurityPolicy("p1").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			ingress_policy.AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork, "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			pol1 := ts.model.NewNetworkSecurityPolicy("p2").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "6", "80", "PERMIT")
			pol1.AddRuleForWorkloadCombo(workloadPairAcrossNetwork, "any", "any", "6", "443", "PERMIT")
			ingress_policy.Add(pol1)

			// Egress. Provider Allow A->B A<->A, User Allow all
			egress_policy := ts.model.NewNetworkSecurityPolicy("p3").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			egress_policy.AddRuleForWorkloadCombo(
				workloadPairAcrossNetwork.ReversePairs(), "workload-subnet", "workload-subnet", "any", "any", "PERMIT")
			pol2 := ts.model.NewNetworkSecurityPolicy("p4").AddRuleForWorkloadCombo(
				workloadPairWithinNetwork, "any", "any", "any", "any", "PERMIT")
			egress_policy.Add(pol2)

			ingress_policy.SetTenant(selNetwork.GetTenant())
			egress_policy.SetTenant(selNetwork.GetTenant())
			Expect(ingress_policy.Commit()).Should(Succeed())
			Expect(egress_policy.Commit()).Should(Succeed())
			selNetwork.SetIngressSecurityPolicy(ingress_policy)
			selNetwork.SetEgressSecurityPolicy(egress_policy)

			// Verify if policies are successfully applied
			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(ingress_policy)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.VerifyPolicyStatus(egress_policy)
			}).Should(Succeed())

			//Skipping data path checks till validation of wildcard fix
			return

			// verify TCP connection succeeds A<->A
			Eventually(func() error {
				return ts.model.TCPSession(workloadPairWithinNetwork, 80)
			}).Should(Succeed())

			// verify TCP connection (reverse) succeeds A<->A
			Eventually(func() error {
				return ts.model.TCPSession(workloadPairWithinNetwork.ReversePairs(), 80)
			}).Should(Succeed())

			// verify TCP connection succeeds from A->B
			Eventually(func() error {
				return ts.model.TCPSession(workloadPairAcrossNetwork, 80)
			}).Should(Succeed())

			// verify TCP connection succeeds from B->A
			Eventually(func() error {
				return ts.model.TCPSession(workloadPairAcrossNetwork.ReversePairs(), 80)
			}).Should(Succeed())

			//Verify Ping fails for both within subnet and across subnet
			Eventually(func() error {
				return ts.model.PingFails(workloadPairWithinNetwork)
			}).Should(Succeed())

			Eventually(func() error {
				return ts.model.PingFails(workloadPairAcrossNetwork)
			}).Should(Succeed())
		})
	})
})

var _ = Describe("firewall policy model tests", func() {
	var startTime time.Time
	BeforeEach(func() {
		// verify cluster is in good health
		startTime = time.Now().UTC()
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		// delete the default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).ShouldNot(HaveOccurred())
	})
	AfterEach(func() {
		//Expect No Service is stopped
		Expect(ts.model.ServiceStoppedEvents(startTime, ts.model.Naples()).Len(0))
		nwc, err := getNetworkCollection()
		Expect(err).Should(Succeed())

		//Now attach ingress and egres
		nwc.SetIngressSecurityPolicy(nil)
		nwc.SetEgressSecurityPolicy(nil)

		// delete test policy if its left over. we can ignore the error here
		ts.model.NetworkSecurityPolicy("test-policy").Delete()
		ts.model.DefaultNetworkSecurityPolicy().Delete()

		// recreate default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())
	})
	Context("policy model tests", func() {
		It("Should be able to verify whitelist policies", func() {
			Skip("Skipping whitelist policy model tests")
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}
			Expect(ts.model.NewNetworkSecurityPolicy("test-policy").Commit()).Should(Succeed())

			// whitelist tests
			whitelistResult := make(map[string]error)
			failed := false
			for _, fromIP := range fromIPcomb {
				for _, toIP := range toIPcomb {
					for _, proto := range protocomb {
						if proto != "icmp" && proto != "any" {
							for _, port := range portcomp {
								err := testWhitelistPolicy(fromIP, toIP, proto, port)
								if err != nil {
									failed = true
									log.Errorf("Error during whitelist policy test for %s. Err: %v", fmt.Sprintf("%s\t%s\t%s\t%s", fromIP, toIP, proto, port), err)
									if os.Getenv("STOP_ON_ERROR") != "" {
										log.Errorf("Whitelist Test failed for: %s\t%s\t%s\t%s. Err: %v", fromIP, toIP, proto, port, err)
										os.Exit(1)
									}
								}
								whitelistResult[fmt.Sprintf("%s\t%s\t%s\t%s", fromIP, toIP, proto, port)] = err
							}
						} else {
							err := testWhitelistPolicy(fromIP, toIP, proto, "")
							if err != nil {
								failed = true
								log.Errorf("Error during whitelist policy test for %s. Err: %v", fmt.Sprintf("%s\t%s\t%s\t", fromIP, toIP, proto), err)
								if os.Getenv("STOP_ON_ERROR") != "" {
									log.Errorf("Whitelist Test failed for: %s\t%s\t%s. Err: %v", fromIP, toIP, proto, err)
									os.Exit(1)
								}
							}
							whitelistResult[fmt.Sprintf("%s\t%s\t%s\t", fromIP, toIP, proto)] = err

						}
					}
				}
			}

			// print the whitelist results
			fmt.Print("==================================================================\n")
			fmt.Printf("                Whitelist Test Results\n")
			fmt.Print("==================================================================\n")

			w := tabwriter.NewWriter(os.Stdout, 0, 0, 4, ' ', tabwriter.AlignRight|tabwriter.Debug)

			for key, err := range whitelistResult {

				if err == nil {
					fmt.Fprintf(w, "%s\t    PASS\n", key)
				} else {
					fmt.Fprintf(w, "%s\t    FAIL\n", key)
				}
			}
			w.Flush()
			Expect(failed).Should(Equal(false))

		})

		It("Should be able to verify blacklist policies", func() {
			Skip("Skipping blacklist policy model tests")
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}
			Expect(ts.model.NewNetworkSecurityPolicy("test-policy").Commit()).Should(Succeed())

			// blacklist tests
			blacklistResult := make(map[string]error)
			failed := false
			for _, fromIP := range fromIPcomb {
				for _, toIP := range toIPcomb {
					for _, proto := range protocomb {
						if proto != "icmp" && proto != "any" {
							for _, port := range portcomp {
								err := testBlacklistPolicy(fromIP, toIP, proto, port)
								if err != nil {
									failed = true
									log.Errorf("Error during blacklist policy test for %s. Err: %v", fmt.Sprintf("%s\t%s\t%s\t%s", fromIP, toIP, proto, port), err)
									if os.Getenv("STOP_ON_ERROR") != "" {
										log.Errorf("Blacklist Test failed for: %s\t%s\t%s\t%s. Err: %v", fromIP, toIP, proto, port, err)
										os.Exit(1)
									}
								}
								blacklistResult[fmt.Sprintf("%s\t%s\t%s\t%s", fromIP, toIP, proto, port)] = err
							}
						} else {
							err := testBlacklistPolicy(fromIP, toIP, proto, "")
							if err != nil {
								failed = true
								log.Errorf("Error during blacklist policy test for %s. Err: %v", fmt.Sprintf("%s\t%s\t%s\t", fromIP, toIP, proto), err)

								if os.Getenv("STOP_ON_ERROR") != "" {
									log.Errorf("Blacklist Test failed for: %s\t%s\t%s. Err: %v", fromIP, toIP, proto, err)
									os.Exit(1)
								}
							}
							blacklistResult[fmt.Sprintf("%s\t%s\t%s\t", fromIP, toIP, proto)] = err

						}
					}
				}
			}

			// print the blacklist results
			fmt.Print("==================================================================\n")
			fmt.Printf("                Blacklist Test Results\n")
			fmt.Print("==================================================================\n")

			w := tabwriter.NewWriter(os.Stdout, 0, 0, 4, ' ', tabwriter.AlignRight|tabwriter.Debug)
			for key, err := range blacklistResult {

				if err == nil {
					fmt.Fprintf(w, "%s\t    PASS\n", key)
				} else {
					fmt.Fprintf(w, "%s\t    FAIL\n", key)
				}
			}
			w.Flush()
			Expect(failed).Should(Equal(false))
		})
	})
})
