package pkg

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"reflect"
	"time"

	"path/filepath"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/netutils"
)

func ConfigAgent(c *Config, manifestFile string) error {

	agentConfig, err := GetAgentConfig(c, manifestFile)
	if err != nil {
		return err
	}
	agentConfig.push()
	return nil
}

type AgentConfig struct {
	Namespaces         []netproto.Namespace
	Vrfs               []netproto.Vrf
	Networks           []netproto.Network
	Endpoints          []netproto.Endpoint
	SgPolicies         []netproto.NetworkSecurityPolicy
	MirrorSessions     []netproto.MirrorSession
	Tunnels            []netproto.Tunnel
	FlowExportPolicies []monitoring.FlowExportPolicy
	restApiMap         map[reflect.Type]string
}

//GetAgentConfig Get Configuration in agent Format to be consumed by traffic gen.
func GetAgentConfig(c *Config, manifestFile string) (*AgentConfig, error) {
	agentConfig := AgentConfig{}
	agentConfig.restApiMap = make(map[reflect.Type]string)
	for _, o := range c.Objects {
		err := o.populateAgentConfig(manifestFile, &agentConfig)
		if err != nil {
			fmt.Println("Stuff failed at getting cgfg: ", err)
			return nil, err
		}
	}
	return &agentConfig, nil
}

func (o *Object) populateAgentConfig(manifestFile string, agentCfg *AgentConfig) error {

	// Automatically interpret the the base dir of the manifest file as the config dir to dump all the generated files
	configDir, _ := filepath.Split(manifestFile)
	specFile := fmt.Sprintf("%s%s", configDir, o.SpecFile)

	dat, err := ioutil.ReadFile(specFile)
	if err != nil {
		return err
	}

	kindMap := map[string]interface{}{
		"Vrf":              &agentCfg.Vrfs,
		"Namespace":        &agentCfg.Namespaces,
		"Network":          &agentCfg.Networks,
		"Endpoint":         &agentCfg.Endpoints,
		"NetworkSecurityPolicy":         &agentCfg.SgPolicies,
		"MirrorSession":    &agentCfg.MirrorSessions,
		"Tunnel":           &agentCfg.Tunnels,
		"FlowExportPolicy": &agentCfg.FlowExportPolicies,
	}

	err = json.Unmarshal(dat, kindMap[o.Kind])
	if err != nil {
		return err
	}

	agentCfg.restApiMap[reflect.TypeOf(kindMap[o.Kind])] = o.RestEndpoint

	return nil
}

func (agentCfg *AgentConfig) push() error {
	doConfig := func(config interface{}, restURL string) error {
		var resp Response
		err := netutils.HTTPPost(restURL, config, &resp)
		if err != nil {
			agentCfg.printErr(err, resp)
			return err
		}
		time.Sleep(time.Millisecond)
		return nil
	}

	fmt.Printf("Creating %d Namespaces...\n", len(agentCfg.Namespaces))
	restURL := fmt.Sprintf("%s%s", AGENT_URL,
		agentCfg.restApiMap[reflect.TypeOf(&agentCfg.Namespaces)])
	for _, ns := range agentCfg.Namespaces {
		doConfig(ns, restURL)

	}

	fmt.Printf("Creating %d VRFs...\n", len(agentCfg.Vrfs))
	restURL = fmt.Sprintf("%s%s", AGENT_URL,
		agentCfg.restApiMap[reflect.TypeOf(&agentCfg.Vrfs)])
	for _, vrf := range agentCfg.Vrfs {
		doConfig(vrf, restURL)
	}

	fmt.Printf("Creating %d Networks...\n", len(agentCfg.Networks))
	restURL = fmt.Sprintf("%s%s", AGENT_URL,
		agentCfg.restApiMap[reflect.TypeOf(&agentCfg.Networks)])
	for _, nt := range agentCfg.Networks {
		doConfig(nt, restURL)
	}
	fmt.Printf("Creating %d Endpoints...\n", len(agentCfg.Endpoints))
	restURL = fmt.Sprintf("%s%s", AGENT_URL,
		agentCfg.restApiMap[reflect.TypeOf(&agentCfg.Endpoints)])
	for _, ep := range agentCfg.Endpoints {
		doConfig(ep, restURL)
	}

	fmt.Printf("Configuring %d Tunnels...\n", len(agentCfg.Tunnels))
	restURL = fmt.Sprintf("%s%s", AGENT_URL,
		agentCfg.restApiMap[reflect.TypeOf(&agentCfg.Tunnels)])
	for _, ms := range agentCfg.Tunnels {
		doConfig(ms, restURL)
	}

	fmt.Printf("Configuring %d SGPolicies...\n", len(agentCfg.SgPolicies))
	restURL = fmt.Sprintf("%s%s", AGENT_URL,
		agentCfg.restApiMap[reflect.TypeOf(&agentCfg.SgPolicies)])
	for _, ep := range agentCfg.SgPolicies {
		doConfig(ep, restURL)
	}

	fmt.Printf("Configuring %d Mirror Sessions...\n", len(agentCfg.MirrorSessions))
	restURL = fmt.Sprintf("%s%s", AGENT_URL,
		agentCfg.restApiMap[reflect.TypeOf(&agentCfg.MirrorSessions)])
	for _, ms := range agentCfg.MirrorSessions {
		doConfig(ms, restURL)
	}
	return nil
}

func (o *AgentConfig) printErr(err error, resp Response) {
	fmt.Printf("Agent configuration failed with. Err: %v\n", err)
	fmt.Println("######### RESPONSE #########")
	b, _ := json.MarshalIndent(resp, "", "   ")
	fmt.Println(string(b))
}
