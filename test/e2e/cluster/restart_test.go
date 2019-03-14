package cluster

import (
	"math/rand"
	"strconv"
	"strings"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
)

var _ = Describe("node tests", func() {
	Context("When CMD on a node is restarted", func() {
		var (
			selNode string // the node where the cmd is restarted
		)
		getMainPid := func(str string) string {
			var pidString string
			for _, v := range strings.Split(str, "\n") {
				if strings.Contains(v, "Main PID:") {
					pidString = v
				}
			}
			return pidString
		}

		BeforeEach(func() {
			selNode = "node" + strconv.Itoa(1+rand.Intn(ts.tu.NumQuorumNodes))

			validateCluster()
		})

		It("it should come back fine", func() {
			out := ts.tu.CommandOutput(ts.tu.NameToIPMap[selNode], "systemctl status pen-cmd --plain")
			Expect(out).Should(ContainSubstring("Active: active (running)"))

			pidString := getMainPid(out)
			By("reloading cmd on " + selNode)
			By("pid of CMD is " + pidString)
			out = ts.tu.CommandOutput(ts.tu.NameToIPMap[selNode], "systemctl restart pen-cmd")

			Eventually(func() bool {
				out = ts.tu.CommandOutput(ts.tu.NameToIPMap[selNode], "systemctl status pen-cmd --plain")
				if !strings.Contains(out, "Active: active (running)") {
					return false
				}
				if getMainPid(out) != pidString {
					return true
				}
				return false
			}, 95, 1).Should(BeTrue(), "cmd should come up fine on node %s after restart", selNode)

		})

		AfterEach(func() {
			// Since we just restarted pen-cmd, the resolver entries on the cmd that just came up
			// take time to sync-up. Lets leave this test in sane state for other tests to continue after this
			validateCluster()
		})

	})
})
