package cluster

import (
	"context"
	"fmt"
	"net"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/globals"
)

type ntpTest struct {
	externalNtpServers []string
	ntpLeaderIP        string
	oldLeaderIP        string
}

var _ = Describe("ntp tests", func() {
	var (
		obj       api.ObjectMeta
		cl        *cmd.Cluster
		clusterIf cmd.ClusterV1ClusterInterface
		err       error
		ntpT      ntpTest
	)

	BeforeEach(func() {
		if ts.tu.NumQuorumNodes < 2 {
			Skip(fmt.Sprintf("Skipping failover test: %d quorum nodes found, need >= 2", ts.tu.NumQuorumNodes))
		}

		apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
		apiClient, err := apiclient.NewRestAPIClient(apiGwAddr)
		Expect(err).ShouldNot(HaveOccurred())
		clusterIf = apiClient.ClusterV1().Cluster()
		ctx := ts.tu.MustGetLoggedInContext(context.Background())
		obj = api.ObjectMeta{Name: "testCluster"}
		cl, err = clusterIf.Get(ctx, &obj)
		Expect(err).ShouldNot(HaveOccurred())
		ntpT = getValidatedNtpObject(cl.Spec.NTPServers, cl.Status.Leader, "")
	})

	It("ntp leader should be re-elected when CMD master is paused", func() {
		Expect(err).ShouldNot(HaveOccurred())
		ntpT.oldLeaderIP = ntpT.ntpLeaderIP
		ts.tu.CommandOutput(ntpT.oldLeaderIP, "docker pause pen-cmd")
		Eventually(func() bool {
			cl, err = clusterIf.Get(ts.tu.MustGetLoggedInContext(context.Background()), &obj)
			if err != nil {
				return false
			}
			newLeaderIP := ts.tu.NameToIPMap[cl.Status.Leader]
			if newLeaderIP == ntpT.oldLeaderIP {
				return false
			}
			By(fmt.Sprintf("Found new leader %v", newLeaderIP))
			return true
		}, 30, 1).Should(BeTrue(), "Did not find a new leader")
		getValidatedNtpObject(cl.Spec.NTPServers, cl.Status.Leader, ntpT.oldLeaderIP)
	})

	It("ntp leader config file should be updated when external NTP servers change", func() {
		ntpServersBak := cl.Spec.NTPServers
		// use a server name that is not resolvable, otherwise the config file will contain the IP
		cl.Spec.NTPServers = []string{"e2e-test-ntp-ext-server-new"}
		ctx := ts.tu.MustGetLoggedInContext(context.Background())
		cl, err = clusterIf.Update(ctx, cl)
		Expect(err).ShouldNot(HaveOccurred())
		getValidatedNtpObject(cl.Spec.NTPServers, cl.Status.Leader, "")

		// now check that if a resolvable pool name is given, we resolve it and insert multiple IP addresses in leader config
		cl.Spec.NTPServers = []string{"pool.ntp.org"}
		// we expect ntp.pool.org to return at least 4 valid IPv4 addresses
		minIPAddresses := 4
		cl, err = clusterIf.Update(ctx, cl)
		Expect(err).ShouldNot(HaveOccurred())
		Eventually(func() bool {
			leaderIP := ts.tu.NameToIPMap[cl.Status.Leader]
			ntpConf := ts.tu.CommandOutput(leaderIP, "bash -c 'if [ -f /etc/pensando/pen-ntp/chrony.conf ] ; then  cat /etc/pensando/pen-ntp/chrony.conf; fi' ")
			numIPv4Addrs := 0
			for _, line := range strings.Split(ntpConf, "\n") {
				var addr string
				if n, err := fmt.Sscanf(line, "server %s iburst", &addr); n == 0 || err != nil {
					continue
				}
				a := net.ParseIP(addr)
				if a != nil && a.IsGlobalUnicast() && a.To4() != nil {
					numIPv4Addrs++
				}
			}
			if numIPv4Addrs < minIPAddresses {
				By(fmt.Sprintf("Expected at least %d IP addresses in leader config, found %d. Config: %s", minIPAddresses, numIPv4Addrs, ntpConf))
				return false
			}
			return true
		}, 75, 5).Should(BeTrue(), "NTP config for leader node should contain IP addresses of NTP servers")

		cl.Spec.NTPServers = ntpServersBak
		cl, err = clusterIf.Update(ctx, cl)
		Expect(err).ShouldNot(HaveOccurred())
		getValidatedNtpObject(cl.Spec.NTPServers, cl.Status.Leader, "")
	})

	AfterEach(func() {
		if ntpT.oldLeaderIP != "" {
			By(fmt.Sprintf("Resuming cmd on %v", ntpT.oldLeaderIP))
			ts.tu.CommandOutput(ntpT.oldLeaderIP, "docker unpause pen-cmd")
			time.Sleep(60 * time.Second)
			ntpT.oldLeaderIP = ""
		}
	})
})

// getValidatedNtpObject generates NTP object for test and calls validation function
func getValidatedNtpObject(ntpServers []string, ntpLeader, oldLeaderIP string) ntpTest {
	var ntpObj ntpTest
	ntpObj.externalNtpServers = ntpServers
	ntpObj.ntpLeaderIP = ts.tu.NameToIPMap[ntpLeader]
	ntpObj.oldLeaderIP = oldLeaderIP
	validateNtpOnCluster(ntpObj)
	return ntpObj
}

// validateNtpOnCluster performs NTP validation over cluster
func validateNtpOnCluster(ntpObj ntpTest) {
	By(fmt.Sprintf("ts:%s Validating Cluster", time.Now().String()))

	By(fmt.Sprintf("Validates NTP config file on Quorum Nodes"))
	for _, qnode := range ts.tu.QuorumNodes {
		ip := ts.tu.NameToIPMap[qnode]
		if ip == ntpObj.oldLeaderIP {
			continue // skip validation as cmd is paused on that node
		}
		var ntpServers []string
		if ip == ntpObj.ntpLeaderIP {
			ntpServers = ntpObj.externalNtpServers
		} else {
			ntpServers = []string{ntpObj.ntpLeaderIP}
		}

		Eventually(func() bool {
			ntpConf := ts.tu.CommandOutput(ip, "bash -c 'if [ -f /etc/pensando/pen-ntp/chrony.conf ] ; then  cat /etc/pensando/pen-ntp/chrony.conf; fi' ")
			if strings.Count(ntpConf, "server ") == len(ntpServers) {
				for _, ntpServer := range ntpServers {
					if strings.Index(ntpConf, "server "+ntpServer+" iburst") == -1 {
						By(fmt.Sprintf("%v not present in config. found %v", ntpServer, ntpConf))
						return false
					}
				}
				return true
			}
			By(fmt.Sprintf("ntpserver: %v ntpconf: %v", ntpServers, ntpConf))
			return false
		}, 75, 5).Should(BeTrue(), "NTP servers for %v quorum node should be %v", qnode, ntpServers)
	}
}
