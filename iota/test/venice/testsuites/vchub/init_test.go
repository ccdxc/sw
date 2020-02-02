// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package vchub_test

import (
	"flag"
	"os"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"testing"

	"github.com/pensando/sw/iota/test/venice/iotakit/model"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

var testbedParams = flag.String("testbed", "/warmd.json", "testbed params file (i.e warmd.json)")
var topoName = flag.String("topo", "3Venice_Nodes", "topology name")
var debugFlag = flag.Bool("debug", false, "set log level to debug")
var scaleFlag = flag.Bool("scale", false, "enable scale configuration")
var scaleDataFlag = flag.Bool("scale-data", false, "enable datapath scale")

// TestSuite : cluster test suite - all venice only test cases go here
type TestSuite struct {
	tb        *testbed.TestBed        // testbed
	model     model.SysModelInterface // system model
	scaleData bool                    // configuration if connections would need to scale
}

var ts *TestSuite

func TestIotaVcHub(t *testing.T) {
	// set log level
	cfg := log.GetDefaultConfig("venice-e2e")
	if *debugFlag {
		cfg.Debug = true
		cfg.Filter = log.AllowDebugFilter
	}
	log.SetConfig(cfg)

	if os.Getenv("JOB_ID") == "" {
		log.Warnf("Skipping Iota tests outside warmd environment")
		return
	}
	RegisterFailHandler(Fail)
	RunSpecs(t, "Iota VcHub E2E Suite")
}

// BeforeSuite runs before the test suite and sets up the testbed
var _ = BeforeSuite(func() {
	tb, model, err := model.InitSuite(*topoName, *testbedParams, *scaleFlag, *scaleDataFlag)
	Expect(err).ShouldNot(HaveOccurred())

	//Make sure this suite is run only with vcenter in it
	//Expect(model.() == iotakit.VcenterModel).Should(BeTrue())

	//TODO: Create VCHub configuration here
	orchestrator, err := model.GetOrchestrator()
	Expect(err).ShouldNot(HaveOccurred())
	log.Infof("Vcenter orchestrator IP : %v, Usename : %v, Password : %v",
		orchestrator.IP, orchestrator.Username, orchestrator.Password)
	//TODO create orchestrator config on vcenter

	// verify cluster, workload are in good health
	Eventually(func() error {
		return model.VerifySystemHealth(true)
	}).Should(Succeed())

	// test suite
	ts = &TestSuite{
		tb:        tb,
		model:     model,
		scaleData: *scaleDataFlag,
	}
})

// AfterSuite handles cleanup after test suite completes
var _ = AfterSuite(func() {
	if ts != nil && ts.tb != nil {
		ts.model.Cleanup()
		ts.tb.PrintResult()
	}
})
