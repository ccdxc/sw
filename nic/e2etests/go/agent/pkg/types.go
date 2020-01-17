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
	DEFAULT_TEMPLATE_FILE  = fmt.Sprintf("%s/src/github.com/pensando/sw/nic/e2etests/go/agent/configs/scale/heimdall.tmpl.json", os.Getenv("GOPATH"))
	LIF_COUNT              = 256 // TODO get this from the manifest file
	LIF_START              = 100 // TODO don't hard code this. Currently hntap assumes the lifs will start from id 100.
	UPLINK_COUNT           = 2   // TODO get this from the manifest file
	UPLINK_START           = 128 // TODO don't hard code this.
	INFRA_EP_COUNT         = 8
	AGENT_URL              = "http://localhost:9007"
)

type Object struct {
	Kind         string `yaml:"kind,omitempty"`
	Name         string `yaml:"name,omitempty"`
	Count        int    `yaml:"count,omitempty"`
	RestEndpoint string `yaml:"rest-endpoint,omitempty"`
	SpecFile     string `yaml:"spec-file,omitempty"`
	TemplateFile string `yaml:"tmpl-file,omitempty"`
}

// Response captures the HTTP Response sent by Agent REST Server
type Response struct {
	StatusCode int      `json:"status-code,omitempty"`
	Error      string   `json:"error,omitempty"`
	References []string `json:"references,omitempty"`
}

type Config struct {
	Objects []Object `yaml:"objects"`
}

type IOTAConfig struct {
	Type         string      `json:"type,omitempty"`
	RestEndpoint string      `json:"rest-endpoint,omitempty"`
	ObjectKey    string      `json:"object-key,omitempty"`
	Objects      interface{} `json:"objects,omitempty"`
}

type ConfigTemplate struct {
	ParentCIDR          string   `json:"network-start,omitempty"`
	VlanOffset          uint32   `json:"vlan-offset,omitempty"`
	USegVlanOffset      uint32   `json:"useg-vlan-offset,omitempty"`
	FirewallPolicyRules []string `json:"firewall-policy-rules,omitempty"`
	ALGInfo             []string `json:"alg-info,omitempty"`
	FlowMonInterval     string   `json:"flowmon-interval,omitempty"`
	L4MatchPerRule      int      `json:"l4match-per-rule,omitempty"`
	IPAddressesPerRule  int      `json:"ip-addresses-per-rule,omitempty"`
}

func init() {
	agentPort := os.Getenv("AGENT_PORT")
	if len(agentPort) != 0 {
		AGENT_URL = "http://localhost:" + agentPort
	}
}
