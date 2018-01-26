package scale

import (
	"os"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"

	"testing"
)

func TestScaleTest(t *testing.T) {
	if os.Getenv("SCALE_TEST") == "" {
		return
	}
	RegisterFailHandler(Fail)
	RunSpecs(t, "SCALE test Suite")
}

// All the test config, state and any helper caches for running this test
type TestSuite struct {
	tu *testutils.TestUtils
}

var ts *TestSuite

var _ = BeforeSuite(func() {
	ts = &TestSuite{
		tu: testutils.New(&testutils.TestBedConfig{NumVeniceNodes: 3, ClusterVIP: "10.100.0.10"}, "tb_config.json"),
	}
	ts.tu.Init()
})

var _ = AfterSuite(func() {
	ts.tu.Close()
})

type NpmDebugStats struct {
	DebugStats map[string]int `json:"debugStats__9006,omitempty"`
}

func npmDebugStats() *NpmDebugStats {
	resp := &NpmDebugStats{}
	ts.tu.DebugStats("pen-npm", globals.NpmRESTPort, resp)
	return resp
}
