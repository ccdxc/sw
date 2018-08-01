package cfg

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
)

//PostMethod Method used for posting
var PostMethod = http.Post

//AgentURL URL agent endpoint
const AgentURL = "http://localhost:9007/"

//AgentObject Agent object
type AgentObject interface {
	_GetAgentObjects() []interface{}
	_GetAgentAPI() string
}

//E2eCfg E2e config file
type E2eCfg struct {
	NamespacesInfo NamespacesInfo `json:"Namespaces"`
	NetworksInfo   NetworksInfo   `json:"Networks"`
	EndpointsInfo  EndpointsInfo  `json:"Endpoints"`
}

//NamespacesInfo Namespaces info
type NamespacesInfo struct {
	ContentType string      `json:"content-type"`
	API         string      `json:"api"`
	Namespaces  []Namespace `json:"namespaces"`
}

//Namespace namespace info
type Namespace struct {
	Kind   string `json:"kind"`
	NSMeta NSMeta `json:"meta"`
}

//NSMeta Namespace meta
type NSMeta struct {
	Name   string `json:"name"`
	Tenant string `json:"tenant"`
}

//NetworksInfo Networks info
type NetworksInfo struct {
	ContentType string    `json:"content-type"`
	API         string    `json:"api"`
	Networks    []Network `json:"networks"`
}

//Network Network info
type Network struct {
	Kind        string      `json:"kind"`
	NetworkMeta NetworkMeta `json:"meta"`
	NetworkSpec NetworkSpec `json:"spec"`
}

//NetworkMeta Network meta
type NetworkMeta struct {
	Name      string `json:"name"`
	Tenant    string `json:"tenant"`
	Namespace string `json:"namespace"`
}

//NetworkSpec Network spec
type NetworkSpec struct {
	Ipv4Subnet  string `json:"ipv4-subnet"`
	Ipv4Gateway string `json:"ipv4-gateway"`
	VlanID      int    `json:"vlan-id"`
}

//EndpointsInfo EndpointsInfo
type EndpointsInfo struct {
	ContentType string     `json:"content-type"`
	API         string     `json:"api"`
	Endpoints   []Endpoint `json:"endpoints"`
}

//Endpoint Endpoint Info
type Endpoint struct {
	Kind         string       `json:"kind"`
	EndpointMeta EndpointMeta `json:"meta"`
	EndpointSpec EndpointSpec `json:"spec"`
}

//EndpointMeta Endpoint Meta
type EndpointMeta struct {
	Name      string `json:"name"`
	Tenant    string `json:"tenant"`
	Namespace string `json:"namespace"`
}

//EndpointSpec Endpoint Spec
type EndpointSpec struct {
	NetworkName   string `json:"network-name"`
	InterfaceType string `json:"interface-type"`
	Interface     string `json:"interface"`
	UsegVlan      int    `json:"useg-vlan,omitempty"`
	Ipv4Address   string `json:"ipv4-address"`
	MacAddresss   string `json:"mac-address"`
	NodeUUID      string `json:"node-uuid,omitempty"`
}

func (ns NamespacesInfo) _GetAgentObjects() []interface{} {
	var interfaceSlice = make([]interface{}, len(ns.Namespaces))
	for i, d := range ns.Namespaces {
		interfaceSlice[i] = d
	}
	return interfaceSlice
}

func (ep EndpointsInfo) _GetAgentObjects() []interface{} {
	var interfaceSlice = make([]interface{}, len(ep.Endpoints))
	for i, d := range ep.Endpoints {
		interfaceSlice[i] = d
	}
	return interfaceSlice
}

func (nw *NetworksInfo) _GetAgentObjects() []interface{} {
	var interfaceSlice = make([]interface{}, len(nw.Networks))
	for i, d := range nw.Networks {
		interfaceSlice[i] = d
	}
	return interfaceSlice
}

func (ns *NamespacesInfo) _GetAgentAPI() string {
	return AgentURL + ns.API
}

func (ep *EndpointsInfo) _GetAgentAPI() string {
	return AgentURL + ep.API
}

func (nw *NetworksInfo) _GetAgentAPI() string {
	return AgentURL + nw.API
}

func _PushAgentConfig(agentObject AgentObject) {

	url := agentObject._GetAgentAPI()
	for _, object := range agentObject._GetAgentObjects() {
		bytesRepresentation, err := json.Marshal(object)
		if err != nil {
			log.Fatalln(err)
		}
		fmt.Println(url, string(bytesRepresentation))
		resp, err := PostMethod(url,
			"application/json", bytes.NewBuffer(bytesRepresentation))
		if err != nil {
			log.Fatalln(err)
		}
		if resp.StatusCode != 200 {
			log.Fatalln("Config failed ", object)
		}
	}
}

//ConfigureNaplesContainer Configure Agent with config.
func ConfigureNaplesContainer(e2ecfg *E2eCfg) {
	fmt.Println("Number of configs", len(e2ecfg.EndpointsInfo.Endpoints))
	fmt.Println("Configuring Namespaces")
	_PushAgentConfig(&e2ecfg.NamespacesInfo)
	fmt.Println("Configuring Networks")
	_PushAgentConfig(&e2ecfg.NetworksInfo)
	fmt.Println("Configuring Endpoints")
	_PushAgentConfig(&e2ecfg.EndpointsInfo)
}

//GetAgentConfig get agent configuration from file
func GetAgentConfig(cfgFile string) *E2eCfg {
	jsonFile, err := os.Open(cfgFile)
	if err != nil {
		fmt.Println(err)
		return nil
	}
	// defer the closing of our jsonFile so that we can parse it later on
	defer jsonFile.Close()

	byteValue, _ := ioutil.ReadAll(jsonFile)

	var e2ecfg E2eCfg

	json.Unmarshal(byteValue, &e2ecfg)
	return &e2ecfg
}
