// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package techsupport_test

import (
	"flag"
	"os"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"testing"

	"github.com/pensando/sw/iota/test/venice/iotakit/model"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var testbedParams = flag.String("testbed", "/warmd.json", "testbed params file (i.e warmd.json)")
var topoName = flag.String("topo", "3Venice_3NaplesSim", "topology name")
var debugFlag = flag.Bool("debug", false, "set log level to debug")
var scaleFlag = flag.Bool("scale", false, "enable scale configuration")
var scaleDataFlag = flag.Bool("scale-data", false, "enable datapath scale")

// TestSuite : techsupport test suite
type TestSuite struct {
	tb        *testbed.TestBed        // testbed
	model     model.SysModelInterface // system model
	scaleData bool                    // configuration if connections would need to scale
}

var ts *TestSuite

func TestIotaTechsupportTest(t *testing.T) {
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
	tb, model, err := model.InitSuite(*topoName, *testbedParams, *scaleFlag, *scaleDataFlag)

	Assert(t, err == nil, "Init suite failed")

	// test suite
	ts = &TestSuite{
		tb:        tb,
		model:     model,
		scaleData: *scaleDataFlag,
		//stress:    *stressFlag,
	}

	RegisterFailHandler(Fail)
	RunSpecsWithCustomReporters(t, "Iota Techsupport E2E Suite", []Reporter{model})
}

// BeforeSuite runs before the test suite and sets up the testbed
var _ = BeforeSuite(func() {

	// verify cluster, workload are in good health
	Eventually(func() error {
		return ts.model.VerifySystemHealth(true)
	}).Should(Succeed())

})

// AfterSuite handles cleanup after test suite completes
var _ = AfterSuite(func() {
	if ts != nil && ts.tb != nil {
		ts.model.Cleanup()
		ts.model.PrintResult()
	}
})
