// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package fwlog_test

import (
	"flag"
	"os"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"testing"

	"github.com/pensando/sw/iota/test/venice/iotakit"
	"github.com/pensando/sw/venice/utils/log"
)

var testbedParams = flag.String("testbed", "/warmd.json", "testbed params file (i.e warmd.json)")
var topoName = flag.String("topo", "1Venice_1Naples", "topology name")
var debugFlag = flag.Bool("debug", false, "set log level to debug")
var scaleFlag = flag.Bool("scale", false, "enable scale configuration")
var scaleDataFlag = flag.Bool("scale-data", false, "enable datapath scale")

// TestSuite : fwlog test suite
type TestSuite struct {
	tb    *iotakit.TestBed  // testbed
	model *iotakit.SysModel // system model
}

var ts *TestSuite

func TestIotaFwlogTest(t *testing.T) {
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
	RunSpecs(t, "Iota fwlog E2E Suite")
}

// BeforeSuite runs before the test suite and sets up the testbed
var _ = BeforeSuite(func() {
	tb, err := iotakit.NewTestBed(*topoName, *testbedParams)
	Expect(err).ShouldNot(HaveOccurred())

	// make cluster & setup auth
	err = tb.SetupConfig()
	Expect(err).ShouldNot(HaveOccurred())

	model, err := iotakit.NewSysModel(tb)
	Expect(err).ShouldNot(HaveOccurred())
	Expect(model).ShouldNot(BeNil())

	model.NewSGPolicy("test-policy").Delete()

	err = model.SetupDefaultConfig(*scaleFlag, *scaleDataFlag)
	Expect(err).ShouldNot(HaveOccurred())

	// collect logs at the end of setup, in case setup fails
	defer tb.CollectLogs()

	// verify cluster is in good health
	Eventually(func() error {
		return model.Action().VerifyClusterStatus()
	}).Should(Succeed())

	// verify ping is successful across all workloads
	Eventually(func() error {
		return model.Action().PingPairs(model.WorkloadPairs().WithinNetwork())
	}).Should(Succeed())

	// test suite
	ts = &TestSuite{
		tb:    tb,
		model: model,
	}
})

// AfterSuite handles cleanup after test suite completes
var _ = AfterSuite(func() {
	if ts != nil && ts.tb != nil {
		ts.tb.Cleanup()
		ts.tb.PrintResult()
	}
})
