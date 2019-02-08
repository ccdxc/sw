// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package firewall_test

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
var topoName = flag.String("topo", "3Venice_3NaplesSim", "topology name")
var debugFlag = flag.Bool("debug", false, "set log level to debug")

// TestSuite : firewall test suite
type TestSuite struct {
	tb    *iotakit.TestBed  // testbed
	model *iotakit.SysModel // system model
}

var ts *TestSuite

func TestIotaFirewallTest(t *testing.T) {
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
	RunSpecs(t, "Iota Firewall E2E Suite")
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

	err = model.SetupDefaultConfig()
	Expect(err).ShouldNot(HaveOccurred())

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
	}
})
