package cluster

import (
	"os"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"testing"

	testutils "github.com/pensando/sw/test/utils"
)

func TestE2ETest(t *testing.T) {
	if os.Getenv("E2E_TEST") == "" {
		return
	}
	RegisterFailHandler(Fail)
	RunSpecs(t, "E2e cmd Suite")
}

// All the test config, state and any helper caches for running this test
type TestSuite struct {
	tu *testutils.TestUtils
}

var ts *TestSuite

var _ = BeforeSuite(func() {
	ts = &TestSuite{
		tu: testutils.New(nil),
	}
	ts.tu.Init()
})

var _ = AfterSuite(func() {
	ts.tu.Close()
})
