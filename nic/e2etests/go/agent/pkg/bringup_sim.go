package pkg

import (
	"fmt"
	"os"
	"strconv"
	"strings"

	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/libs"
	"github.com/pkg/errors"
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

func MoveInterfacesOutOfSim() error {

	fmt.Println("Moving interfaces out of naples sim")
	intfs := getNaplesInterfaces()
	cmd := []string{"docker", "inspect", "--format", "'{{.State.Pid}}'", "naples-v1"}
	out, err := libs.CaptureCommandStdout(cmd[0], cmd[1:]...)
	if err != nil {
		fmt.Println(string(out))
		return errors.Wrap(err, "Error in inspecting docker naples sim")
	}
	pid := strings.TrimSuffix(string(out[:]), "\n")
	pid = strings.Trim(pid, `'"`)
	for _, intf := range intfs {
		cmd := []string{"nsenter", "-t", pid, "-n", "ip", "link", "set", intf, "netns", "1"}
		//Ignore error as they could have been moved before.
		libs.RunCommand(cmd[0], cmd[1:]...)
		cmd = []string{"ifconfig", intf, "up"}
		libs.RunCommand(cmd[0], cmd[1:]...)
	}

	return nil

}

func getNaplesInterfaces() []string {

	var intfs []string
	for i := LIF_START; i < LIF_START+16; i++ {
		intfs = append(intfs, "lif"+strconv.Itoa(i))
	}
	intfs = append(intfs, "pen-intf1")
	intfs = append(intfs, "pen-intf2")
	return intfs
}
