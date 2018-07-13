package main

import (
	"fmt"
	"os/exec"
	"strings"

	log "github.com/sirupsen/logrus"
)

var dockerPath string

// RunContainer creates and runs an alpine container. Returns containerID.
func RunContainer() (string, string, error) {
	out, err := exec.Command(dockerPath, "run", "-d", "alpine", "sleep", "360000").CombinedOutput()
	if err != nil {
		log.Errorf("Error %v creating container", err)
		return "", "", err
	}

	id := strings.Split(string(out), "\n")[0]
	out, err = exec.Command(dockerPath, "inspect", "--format", "{{.State.Pid}}", id).CombinedOutput()
	if err != nil {
		log.Errorf("Error %v inspecting container, out: %s", err, out)
		return "", "", err
	}
	pid := strings.Split(string(out), "\n")[0]
	log.Infof("Pid is %s -- from %s", pid, out)

	return id, pid, nil
}

// GetNetNS returns the network name space of the container
func GetNetNS(id string) (string, error) {
	out, err := exec.Command(dockerPath, "inspect", "--format", "'{{.State.Pid}}'", id).CombinedOutput()
	if err != nil {
		log.Errorf("Error %v inspecting container", err)
		return "", err
	}
	pid := fmt.Sprintf("%s", out)
	return pid, nil
}

// RemoveContainer removes the specified container
func RemoveContainer(containerID string) error {
	_, err := exec.Command(dockerPath, "rm", "-f", containerID).CombinedOutput()
	return err
}

// GetAllContainers gets all containers from docker
func GetAllContainers() map[string]bool {
	res := make(map[string]bool)
	cmd := dockerPath + " ps -a | grep alpine | awk '{print $1}'"
	out, err := exec.Command("bash", "-c", cmd).CombinedOutput()
	if err == nil {
		ids := strings.Split(string(out), "\n")
		for _, id := range ids {
			if id != "" {
				res[id] = true
			}
		}
	}

	return res
}

// ExecContainer executes the specified command in the container and returns the result
func ExecContainer(containerID, cmdline string) (string, error) {
	return "", nil
}

// InitDocker verifies docker path
func InitDocker() {
	p, err := exec.LookPath("docker")
	if err != nil {
		log.Fatalf("Failed to find docker -- %v", err)
	}

	dockerPath = p

	_, err = exec.Command(dockerPath, "ps").CombinedOutput()
	if err != nil {
		log.Fatalf("Unable to access docker -- %v", err)
	}
}
