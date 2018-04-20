package standalone

import (
	"fmt"
	"os"
	"os/exec"

	. "github.com/onsi/ginkgo"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/netutils"
)

var _ = Describe("Agent standalone tests", func() {
	Context("When a network is created", func() {
		var (
			lis        netutils.TestListenAddr
			lisErr     = lis.GetAvailablePort()
			resp       error
			agentArgs  []string
			networkURL = fmt.Sprintf("http://%s/api/networks/", lis.ListenURL.String())
			tenantURL  = fmt.Sprintf("http://%s/api/tenants/", lis.ListenURL.String())
		)
		// Verify if NetAgent is up
		BeforeEach(func() {
			if lisErr != nil {
				Fail(fmt.Sprintf("could not assign an available port, %v", lisErr))
			}

			// Allow for testing with HAL in jobd
			if os.Getenv("E2E_AGENT_DATAPATH") == "HAL" {
				agentArgs = []string{"-hostif", "lo", "-logtofile", "/tmp/agent.log", "-datapath", "hal", "-rest-url", lis.ListenURL.String()}
			} else {
				agentArgs = []string{"-hostif", "lo", "-logtofile", "/tmp/agent.log", "-rest-url", lis.ListenURL.String()}
			}

			fmt.Println("Agent CLI: ", agentArgs)
			// start as the agent binary needs to run in the background
			err := exec.Command("netagent", agentArgs...).Start()
			if err != nil {
				Fail(fmt.Sprintf("could not start netagent, Err: %v", err))
			}

			ok := checkAgentIsUp(tenantURL)
			if !ok {
				Fail(fmt.Sprintf("netagent bringup took too long, %v", err))
			}

			postData := netproto.Network{
				TypeMeta: api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "default",
					Namespace: "default",
					Name:      "skynet",
				},
				Spec: netproto.NetworkSpec{
					IPv4Subnet:  "10.1.1.0/24",
					IPv4Gateway: "10.1.1.254",
					VlanID:      42,
				},
			}

			err = netutils.HTTPPost(networkURL, &postData, &resp)
			if err != nil {
				Fail(fmt.Sprintf("could not post network object, %v", err))
			}
		})

		It("agent gets for tenants and networks should work", func() {

			var tnList []*netproto.Tenant
			err := netutils.HTTPGet(tenantURL, &tnList)
			if err != nil {
				Fail(fmt.Sprintf("could not get default tenants, %v", err))
			}

		})

		AfterEach(func() {
			exec.Command("pkill", "netagent").Run()
			os.Remove("/tmp/naples-netagent.db")
		})
	})
})
