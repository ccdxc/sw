package equinix_test

import (
	"encoding/json"
	"fmt"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"strconv"
	"strings"
	"time"
)

var _ = Describe("Interface tests", func() {
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
	})
	AfterEach(func() {
		ts.tb.AfterTestCommon()
	})

	Context("Interfaces", func() {

		It("Update Loopback IP", func() {
			Skip("Disable test for sanity")
			fakeNaples := !ts.tb.HasNaplesHW()

			lbc := objects.GetLoopbacks(ts.model.ConfigClient(), ts.model.Testbed())
			Expect(lbc.Error()).ShouldNot(HaveOccurred())

			old := objects.GetLoopbacks(ts.model.ConfigClient(), ts.model.Testbed()) //For restoring
			Expect(old.Error()).ShouldNot(HaveOccurred())

			for i, intf := range lbc.Interfaces {
				oldIP := strings.Split(intf.Spec.IPConfig.IPAddress, "/")[0]
				ipBytes := strings.Split(oldIP, ".")
				lastByte, _ := strconv.Atoi(ipBytes[3])
				newLoopbackIP := fmt.Sprintf("%s.%s.%s.%v/32", ipBytes[0], ipBytes[1], ipBytes[2],
					(lastByte+1)%256)
				if fakeNaples {
					newLoopbackIP = fmt.Sprintf("10.8.250.%v/32", i+1)
				}
				intf.Spec.IPAllocType = "static"
				log.Infof("New Loopback ip : %s", newLoopbackIP)
				intf.Spec.IPConfig.IPAddress = newLoopbackIP
			}

			Expect(lbc.Commit()).Should(Succeed())

			//Verify in Venice
			verifyVeniceIntf(lbc)
			// Wait for Naples to finish configuring
			time.Sleep(10 * time.Second)
			Eventually(func() error {
				//verify api/interfaces/ in netagent
				return verifyNetAgentIntf(lbc)
			}).Should(Succeed())

			Eventually(func() error {
				//verify routerID has changed in pdsctl
				return verifyPDSAgentIntf(lbc)
			}).Should(Succeed())

			//reset back to original loopbackIP
			Expect(old.Commit()).Should(Succeed())
		})
	})
})

func verifyVeniceIntf(intf *objects.NetworkInterfaceCollection) error {
	//Get updated venice info
	updc := objects.GetLoopbacks(ts.model.ConfigClient(), ts.model.Testbed())
	Expect(updc.Error()).ShouldNot(HaveOccurred())

	for _, i := range intf.Interfaces {
		for _, d := range updc.Interfaces {
			//TODO what if order is different ?
			if i.Spec.IPConfig.GetIPAddress() != d.Spec.IPConfig.GetIPAddress() {
				return fmt.Errorf("loopback IPs don't match : Expected %s Found %s",
					i.Spec.IPConfig.GetIPAddress(), d.Spec.IPConfig.GetIPAddress())
			}
		}
	}
	return nil
}

func verifyNetAgentIntf(intf *objects.NetworkInterfaceCollection) error {

	if !ts.tb.HasNaplesHW() {
		return ts.model.ForEachFakeNaples(func(nc *objects.NaplesCollection) error {
			cmdOut, err := ts.model.RunFakeNaplesBackgroundCommand(nc,
				"curl localhost:9007/api/interfaces/")
			if err != nil {
				return err
			}

			cmdResp, _ := cmdOut.([]*iota.Command)
			for _, cmdLine := range cmdResp {
				intfData := []netproto.Interface{}
				err := json.Unmarshal([]byte(cmdLine.Stdout), &intfData)
				if err != nil {
					return err
				}

				for _, i := range intf.Interfaces {
					for _, datum := range intfData {
						if datum.Spec.Type == "LOOPBACK" &&
							datum.Spec.IPAddress == i.Spec.IPConfig.IPAddress {
							return nil
						}
					}
				}
			}
			return fmt.Errorf("Node IP not found")
		})
	}

	var lb_netagent_fn = func(cmdout *string) error {
		intfData := []netproto.Interface{}
		err := json.Unmarshal([]byte(*cmdout), &intfData)
		if err != nil {
			return err
		}

		for _, i := range intf.Interfaces {
			for _, datum := range intfData {
				if datum.Spec.Type == "LOOPBACK" &&
					datum.Spec.IPAddress == i.Spec.IPConfig.IPAddress {
					return nil
				}
			}
		}
		return fmt.Errorf("NetAgent : No loopback IP found")
	}

	return ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
		cmdOut, err := ts.model.RunNaplesCommand(nc, "curl localhost:9007/api/interfaces/")
		if err != nil {
			return err
		}

		return lb_netagent_fn(&cmdOut[0])
	})
}

func verifyPDSAgentIntf(intf *objects.NetworkInterfaceCollection) error {

	if !ts.tb.HasNaplesHW() {
		return ts.model.ForEachFakeNaples(func(nc *objects.NaplesCollection) error {
			cmdOut, err := ts.model.RunFakeNaplesBackgroundCommand(nc,
				"/naples/nic/bin/pdsctl show bgp --detail")
			if err != nil {
				return err
			}

			cmdResp, _ := cmdOut.([]*iota.Command)

			for _, cmdLine := range cmdResp {
				for _, i := range intf.Interfaces {
					ip := strings.Split(i.Spec.IPConfig.IPAddress, "/")[0]
					log.Infof("**** IP : %s", ip)
					if strings.Contains(cmdLine.Stdout, ip) {
						return nil
					}
				}
			}
			return fmt.Errorf("Loopback IP not found\n")
		})
	}

	return ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
		cmdout, err := ts.model.RunNaplesCommand(nc, "/nic/bin/pdsctl show bgp --detail")
		if err != nil {
			return err
		}

		for _, i := range intf.Interfaces {
			ip := strings.Split(i.Spec.IPConfig.GetIPAddress(), "/")[0]
			if strings.Contains(cmdout[0], ip) {
				return nil
			}
		}
		return fmt.Errorf("Loopback IP not found\n")
	})
}
