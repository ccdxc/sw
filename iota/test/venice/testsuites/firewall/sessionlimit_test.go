// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package firewall_test

import (
	"fmt"
	"strings"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

func VerifyFirewallPropagation(fwp *objects.FirewallProfileCollection) error {
	naplesCol := ts.model.Naples()

	if len(naplesCol.Nodes) == 0 {
		return nil
	}

	secp, err := fwp.GetDefaultFirewallProfile()
	if err != nil {
		log.Errorf("Error getting FirewallProfile Err: %v", err)
		return err
	}

	if secp.Status.PropagationStatus.GenerationID != secp.ObjectMeta.GenerationID {
		log.Warnf("Propagation generation id did not match: Meta: %+v, PropagationStatus: %+v", secp.ObjectMeta, secp.Status.PropagationStatus)
		return fmt.Errorf("Propagation generation id did not match")
	}
	if (secp.Status.PropagationStatus.Updated != (int32(len(naplesCol.Nodes)) + int32(len(naplesCol.FakeNodes)))) || (secp.Status.PropagationStatus.Pending != 0) {
		log.Warnf("Propagation status incorrect: Meta: %+v, PropagationStatus: %+v", secp.ObjectMeta, secp.Status.PropagationStatus)
		return fmt.Errorf("Propagation status was incorrect")
	}

	return nil
}

func VerifyFirewallHalConfig(fwp *objects.FirewallProfileCollection, sessType string, limit int) error {
	var fieldString string
	naplesCol := ts.model.Naples()

	if len(naplesCol.Nodes) == 0 {
		return nil
	}

	switch sessType {
	case "TCP":
		fieldString = fmt.Sprintf("tcphalfopensessionlimit: %d", limit)
	case "UDP":
		fieldString = fmt.Sprintf("udpactivesessionlimit: %d", limit)
	case "ICMP":
		fieldString = fmt.Sprintf("icmpactivesessionlimit: %d", limit)
	case "OTHER":
		fieldString = fmt.Sprintf("otheractivesessionlimit: %d", limit)
	default:
		return fmt.Errorf("Unsuported sessType :%v ", sessType)
	}

	// command example : halctl show nwsec profile | grep 'tcphalfopensessionlimit: 10'
	halCommand := fmt.Sprintf("/nic/bin/halctl show nwsec profile | /bin/grep '%s'", fieldString)
	return ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
		if out, err := ts.model.RunNaplesCommand(nc, halCommand); err == nil {
			log.Infof(strings.Join(out, ","))
			if len(out) == 0 {
				return fmt.Errorf("hal programming failed for %v", fieldString)
			}
		} else {
			return fmt.Errorf("failed to run halctl, %v", err)
		}
		return nil
	})
}

func VerifyDetectAppHalConfig(fwp *objects.FirewallProfileCollection, enable bool) error {
	var fieldString string
	naplesCol := ts.model.Naples()

	if len(naplesCol.Nodes) == 0 {
		return nil
	}
	fieldString = fmt.Sprintf("appdetectionen: %v", enable)

	halCommand := fmt.Sprintf("/nic/bin/halctl show nwsec profile | /bin/grep '%s'", fieldString)
	return ts.model.ForEachNaples(func(nc *objects.NaplesCollection) error {
		if out, err := ts.model.RunNaplesCommand(nc, halCommand); err == nil {
			log.Infof(strings.Join(out, ","))
			if len(out) == 0 {
				return fmt.Errorf("hal programming failed for %v", fieldString)
			}
		} else {
			return fmt.Errorf("failed to run halctl, %v", err)
		}
		return nil
	})

}

func SaveSettings() *objects.FirewallProfileCollection {
    fwp := ts.model.NewFirewallProfile("default")
	if fwp.Commit() != nil {
		log.Errorf("Firewall programming failed for default")
		return nil
	}
	return fwp
}

func RestoreSettings(fwp *objects.FirewallProfileCollection) error {
	if len(fwp.Profiles) < 1 {
        return fmt.Errorf("Invalid FirwallProfile config len : %d", len(fwp.Profiles))
    }
	if fwp.Commit() != nil {
        return fmt.Errorf("Firewall programming failed to restore settings")
	}
	return nil
}

// will be used when functionality is in naples
/*
func VerifyActiveSessions(fwp *objects.FirewallProfileCollection, timestr string, sessType string, limit int) error {

	testFields := map[string]int{}
	naplesCol := ts.model.Naples()

	if len(naplesCol.Nodes) == 0 {
		return nil
	}
	switch sessType {
	case "TCP":
		testFields["NumTcpSessions"] =  limit
	case "UDP":
		testFields["NumUdpSessions"] =  limit
	case "ICMP":
		testFields["NumIcmpSessions"] =  limit
	default:
		return fmt.Errorf("Unsuported sessType :%v ", sessType)
	}

	res, err := ts.model.QueryMetricsFields("SessionSummaryMetrics", timestr)
	if err != nil {
		return err
	}
	log.Infof("Done with query for max results : %+v", res)
	log.Infof("TestFields : %+v", testFields)

	for _, rslt := range res.Results {
		log.Infof("Results %d", len(rslt.Series))
		for _, series := range rslt.Series {
			log.Infof("series")
			cIndex := map[string]int{}
			log.Infof("Columns : %+v", series.Columns)
			log.Infof("Values: %+v", series.Values)
			for i, c:= range series.Columns {
				if _, ok := testFields[c]; ok {
					cIndex[c] = i
				}
			}
			log.Infof("cIndex : %+v", cIndex)
			for _, t := range series.Values{
				for k,v := range cIndex {
					temp := int(t[v].(float64))
					if temp != testFields[k] {
						return fmt.Errorf("received %v : %v expected: %v", k, temp, testFields[k])
					}
				}
			}
		}
	}
	return nil
}*/


var _ = Describe("session limit tests", func() {
	var (
		fwp *objects.FirewallProfileCollection
		defaultFwp *objects.FirewallProfileCollection
	)
	BeforeEach(func() {
		// verify cluster is in good health
		Eventually(func() error {
			return ts.model.VerifyClusterStatus()
		}).Should(Succeed())
		// disable the session limits to start afresh
		fwp = ts.model.NewFirewallProfile("default")
		defaultFwp = SaveSettings()
		Expect(defaultFwp != nil).To(Equal(true))
		Eventually(func() error {
			return VerifyFirewallPropagation(defaultFwp)
		}).Should(Succeed())
	})
	AfterEach(func() {
		// disable the session limits to cleanup
		Expect(RestoreSettings(defaultFwp)).Should(Succeed())
		Eventually(func() error {
			return VerifyFirewallPropagation(defaultFwp)
		}).Should(Succeed())
		ts.tb.AfterTestCommon()
	})

	Context("tags:type=basic;datapath=true;duration=short enable disable session limit config test", func() {
		It("tags:sanity=true Should be able to enable and disable session limits", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}
			By("Enabling Firewall Session Limits")
			Expect(fwp.EnableFirewallLimit(500)).Should(Succeed())
			Expect(fwp.Commit()).Should(Succeed())
			Eventually(func() error {
				return VerifyFirewallPropagation(fwp)
			}).Should(Succeed())

			By("Disabling Firewall Session Limits")
			Expect(fwp.DisableFirewallLimit()).Should(Succeed())
			Expect(fwp.Commit()).Should(Succeed())
			Eventually(func() error {
				return VerifyFirewallPropagation(fwp)
			}).Should(Succeed())
		})

		It("tags:sanity=true Should enable all session limit and verify on hal", func() {
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}
			var conn_limit int = 1000

			By(fmt.Sprintf("Enabling TCP, UDP, ICMP, OTHER Firewall Session Limits to %d", conn_limit))
			Expect(fwp.SetFirewallLimit("ALL", conn_limit)).Should(Succeed())
			Expect(fwp.Commit()).Should(Succeed())

			Eventually(func() error {
				return VerifyFirewallPropagation(fwp)
			}).Should(Succeed())

			By(fmt.Sprintf("Verify Hal Programming config on naples"))
			Expect(VerifyFirewallHalConfig(fwp, "TCP", conn_limit)).ShouldNot(HaveOccurred())
			Expect(VerifyFirewallHalConfig(fwp, "UDP", conn_limit)).ShouldNot(HaveOccurred())
			Expect(VerifyFirewallHalConfig(fwp, "ICMP", conn_limit)).ShouldNot(HaveOccurred())
			Expect(VerifyFirewallHalConfig(fwp, "OTHER", conn_limit)).ShouldNot(HaveOccurred())

		})

		It("tags:sanity=true Verify default firewallprofile have detect-app to false", func(){
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}
			testMode := false
			By(fmt.Sprintf("Verify Hal Programming of default DetectApp on naples"))
			Expect(VerifyDetectAppHalConfig(fwp, testMode)).ShouldNot(HaveOccurred())
		})

		It("tags:sanity=true Should enable detect-app and verify on hal", func(){
			Skip("Disable test until hal functionality is in place")
			if !ts.tb.HasNaplesHW() {
				Skip("Disabling on naples sim till traffic issue is debugged")
			}
			testMode := true
			By(fmt.Sprintf("Set DetectApp[%v] on naples", testMode))
			Expect(fwp.SetDetectApp(testMode)).Should(Succeed())
			Expect(fwp.Commit()).Should(Succeed())

			Eventually(func() error {
				return VerifyFirewallPropagation(fwp)
			}).Should(Succeed())

			By(fmt.Sprintf("Verify Hal Programming of DetectApp on naples"))
			Expect(VerifyDetectAppHalConfig(fwp, testMode)).ShouldNot(HaveOccurred())
		})
	})
})
