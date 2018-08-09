package pkg

import (
	"fmt"
	"os"

	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/libs"
)

func BringUpSim() error {

	_, err := os.Stat(NIC_SIM_RELEASE_TARGET)
	if err != nil {
		return fmt.Errorf("naples-release-v1.tgz file not found. Run make release. Err: %v", err)
	}

	// Ignore error during pre init clean up
	libs.RunCommand("bash", SIM_STOP_SCRIPT)

	err = libs.RunCommand("bash", SIM_START_UP_SCRIPT, "--lifs", fmt.Sprintf("%d", LIF_COUNT))
	if err != nil {
		return fmt.Errorf("NAPLES Bring up failed. Err: %v", err)
	}

	// check for health and return error
	err = libs.RunCommand("bash", HEALTH_CHECK_SCRIPT)
	if err != nil {
		return fmt.Errorf("NAPLES container health check failed")
	}
	return nil
}
