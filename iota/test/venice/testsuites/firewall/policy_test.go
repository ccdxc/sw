// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

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

	"github.com/pensando/sw/venice/utils/log"
)

var (
	fromIPcomb = []string{"workload-ip", "workload-subnet", "any"}
	toIPcomb   = []string{"workload-ip", "workload-subnet", "any"}
	protocomb  = []string{"tcp", "udp", "icmp", "any"}
	portcomp   = []string{"8000", "8001,8010", "8005-8010", "any"}
)

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
	workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
	spc := ts.model.NetworkSecurityPolicy("test-policy").DeleteAllRules()

	// add allow all rule for workload followed by deny all rule
	spc = spc.AddRuleForWorkloadCombo(workloadPairs, fromIP, toIP, proto, port, "PERMIT")
	spc = spc.AddRule("any", "any", "any", "DENY")
	err := spc.Commit()
	if err != nil {
		return err
	}

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
					if port != "any" {
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
					if port != "any" {
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
	workloadPairs := ts.model.WorkloadPairs().WithinNetwork().Any(1)
	spc := ts.model.NetworkSecurityPolicy("test-policy").DeleteAllRules()

	// add deny rule for workload followed by allow all rule
	spc = spc.AddRuleForWorkloadCombo(workloadPairs, fromIP, toIP, proto, port, "DENY")
	spc = spc.AddRule("any", "any", "any", "PERMIT")
	err := spc.Commit()
	if err != nil {
		return err
	}

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
					if port != "any" {
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
					if port != "any" {
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

var _ = Describe("firewall policy model tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())

		// delete the default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Delete()).ShouldNot(HaveOccurred())
	})
	AfterEach(func() {
		// delete test policy if its left over. we can ignore the error here
		ts.model.NetworkSecurityPolicy("test-policy").Delete()
		ts.model.DefaultNetworkSecurityPolicy().Delete()

		// recreate default allow policy
		Expect(ts.model.DefaultNetworkSecurityPolicy().Restore()).ShouldNot(HaveOccurred())
	})
	Context("policy model tests", func() {
		It("Should be able to verify whitelist policies", func() {
			if !ts.tb.IsMockMode() && os.Getenv("REGRESSION") == "" {
				Skip("Skipping policy model tests on PR tests")
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
			if !ts.tb.IsMockMode() && os.Getenv("REGRESSION") == "" {
				Skip("Skipping policy model tests on PR tests")
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
