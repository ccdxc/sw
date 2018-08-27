package config

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net"
	"net/http"
	"os"
	"strconv"
	"time"
)

//PostMethod Method used for posting
var PostMethod = http.Post

//AgentURL URL agent endpoint
var AgentURL = "http://127.0.0.1:9007/"

//AgentObject Agent object
type AgentObject interface {
	_GetAgentObjects() []interface{}
	_GetAgentDelObjects() []interface{}
	_GetAgentAPI() string
}

//E2ETScaleTestCfg e2e scale topo parse
type E2ETScaleTestCfg struct {
	Interfaces struct {
		Host struct {
			Lifs     int `yaml:"lifs"`
			LifStart int `yaml:"lif_start"`
		} `yaml:"host"`
		Remote struct {
			Uplinks     int `yaml:"uplinks"`
			UplinkStart int `yaml:"uplink_start"`
		} `yaml:"remote"`
	} `yaml:"interfaces"`
	Namespaces struct {
		Count int    `yaml:"count"`
		API   string `yaml:"api"`
	} `yaml:"namespaces"`
	Networks struct {
		Count int    `yaml:"count"`
		API   string `yaml:"api"`
	} `yaml:"networks"`
	Endpoints struct {
		Useg   int    `yaml:"useg"`
		Remote int    `yaml:"remote"`
		API    string `yaml:"api"`
	} `yaml:"endpoints"`
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

func (ns NamespacesInfo) _GetAgentDelObjects() []interface{} {
	var interfaceSlice = make([]interface{}, len(ns.Namespaces))
	for i, d := range ns.Namespaces {
		interfaceSlice[i] = ns._GetAgentAPI() + d.NSMeta.Name
	}
	return interfaceSlice
}

func (ep EndpointsInfo) _GetAgentDelObjects() []interface{} {
	var interfaceSlice = make([]interface{}, len(ep.Endpoints))
	for i, d := range ep.Endpoints {
		interfaceSlice[i] = ep._GetAgentAPI() +
			d.EndpointMeta.Tenant + "/" + d.EndpointMeta.Namespace + "/" + d.EndpointMeta.Name
	}
	return interfaceSlice
}

func (nw *NetworksInfo) _GetAgentDelObjects() []interface{} {
	var interfaceSlice = make([]interface{}, len(nw.Networks))
	for i, d := range nw.Networks {
		interfaceSlice[i] = nw._GetAgentAPI() +
			d.NetworkMeta.Tenant + "/" + d.NetworkMeta.Namespace + "/" + d.NetworkMeta.Name
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

var _transport = &http.Transport{
	Proxy: http.ProxyFromEnvironment,
	Dial: (&net.Dialer{
		Timeout:   0,
		KeepAlive: 0,
	}).Dial,
	TLSHandshakeTimeout: 10 * time.Second,
}

var _httpClient = &http.Client{Transport: _transport}

//DoConfig default points to http request
var DoConfig = func(req *http.Request) (*http.Response, error) {
	return _httpClient.Do(req)
}

func _DoConfigWork(method string, url string, data *[]byte) {

	var buffer io.Reader
	if data != nil {
		buffer = bytes.NewBuffer(*data)
	} else {
		buffer = (io.Reader)(nil)
	}
	req, reqErr := http.NewRequest(method, url, buffer)
	if reqErr != nil {
		panic(reqErr)
	}
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Connection", "close")
	req.Close = true

	resp, err := DoConfig(req)
	if err != nil {
		panic(err)
	}
	resp.Body.Close()
}

func _PushAgentConfig(agentObject AgentObject) {

	url := agentObject._GetAgentAPI()
	for _, object := range agentObject._GetAgentObjects() {
		bytesRepresentation, err := json.Marshal(object)
		if err != nil {
			log.Fatalln(err)
		}
		fmt.Println("Post", url, string(bytesRepresentation))
		_DoConfigWork("POST", url, &bytesRepresentation)
	}
}

func _DeleteAgentConfig(agentObject AgentObject) {
	for _, object := range agentObject._GetAgentDelObjects() {
		if url, ok := object.(string); ok {
			fmt.Println("Delete", url)
			_DoConfigWork("DELETE", url, nil)
		} else {
			panic("Invalid object type.")
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

//ClearNaplesContainer Delete Agent with config.
func ClearNaplesContainer(e2ecfg *E2eCfg) {
	fmt.Println("Number of configs", len(e2ecfg.EndpointsInfo.Endpoints))
	fmt.Println("Deleting Endpoints")
	_DeleteAgentConfig(&e2ecfg.EndpointsInfo)
	fmt.Println("Deleting Networks")
	_DeleteAgentConfig(&e2ecfg.NetworksInfo)
	fmt.Println("Deleting Namespaces")
	_DeleteAgentConfig(&e2ecfg.NamespacesInfo)
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

// PushConfiguration push agent configuration.
func PushConfiguration(ip string, port int, configFile string) error {

	agentCfg := GetAgentConfig(configFile)

	if agentCfg == nil {
		return errors.New("Unable to read agent config :" + configFile)
	}
	AgentURL = "http://" + ip + ":" + strconv.Itoa(port) + "/"
	ConfigureNaplesContainer(agentCfg)
	return nil
}
