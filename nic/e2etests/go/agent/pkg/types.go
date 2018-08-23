package pkg

import (
	"fmt"
	"os"
)

var (
	NIC_SIM_RELEASE_TARGET = fmt.Sprintf("%s/src/github.com/pensando/sw/nic/obj/images/naples-release-v1.tgz", os.Getenv("GOPATH"))
	SIM_START_UP_SCRIPT    = fmt.Sprintf("%s/src/github.com/pensando/sw/nic/sim/naples/start-naples-docker.sh", os.Getenv("GOPATH"))
	SIM_STOP_SCRIPT        = fmt.Sprintf("%s/src/github.com/pensando/sw/nic/sim/naples/stop-naples-docker.sh", os.Getenv("GOPATH"))
	HEALTH_CHECK_SCRIPT    = fmt.Sprintf("%s/src/github.com/pensando/sw/nic/e2etests/go/agent/scripts/check-naples-health.sh", os.Getenv("GOPATH"))
	LIF_COUNT              = 256 // TODO get this from the manifest file
	LIF_START              = 100 // TODO don't hard code this. Currently hntap assumes the lifs will start from id 100.
	UPLINK_COUNT           = 2   // TODO get this from the manifest file
	UPLINK_START           = 128 // TODO don't hard code this.
	AGENT_URL              = "http://localhost:9007"
)

type Object struct {
	Kind         string `yaml:"kind,omitempty"`
	Name         string `yaml:"name,omitempty"`
	Count        int    `yaml:"count,omitempty"`
	RestEndpoint string `yaml:"rest-endpoint,omitempty"`
	SpecFile     string `yaml:"spec-file,omitempty"`
}

type Config struct {
	Objects []Object `yaml:"objects"`
}
