package standalone

import (
	"fmt"
	"testing"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/venice/utils/netutils"
)

const retryCount = 3

func TestAgentStandalone(t *testing.T) {
	RegisterFailHandler(Fail)
	RunSpecs(t, "Agent standalone tests")
}

// Waits till Agent's REST Endpoint is ready to accept connections.
func checkAgentIsUp(agentURL string) bool {
	for i := retryCount; i > 0; i-- {
		_, err := netutils.HTTPGetRaw(agentURL)
		if err != nil {
			fmt.Println(err)
			time.Sleep(time.Duration(i) * time.Second)
			continue
		}
		return true
	}
	return false
}
