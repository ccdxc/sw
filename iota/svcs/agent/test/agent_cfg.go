package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net"
	"net/http"
	"os"
	"time"
)

//PostMethod Method used for posting
var PostMethod = http.Post

//AgentURL URL agent endpoint
var AgentURL = "http://127.0.0.1:9007/"

//AgentObject Agent object
type AgentObject interface {
	_GetAgentObjects() []interface{}
	_GetAgentAPI() string
}

//AgentTestCfg e2e scale topo parse
type AgentTestCfg struct {
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
	SgPoliciesInfo SgPoliciesInfo `json:"SgPolicies"`
}

//NamespacesInfo Namespaces info
type NamespacesInfo struct {
	ContentType string        `json:"content-type"`
	API         string        `json:"api"`
	Namespaces  []interface{} `json:"namespaces"`
}

//NetworksInfo Networks info
type NetworksInfo struct {
	ContentType string        `json:"content-type"`
	API         string        `json:"api"`
	Networks    []interface{} `json:"networks"`
}

//EndpointsInfo EndpointsInfo
type EndpointsInfo struct {
	ContentType string        `json:"content-type"`
	API         string        `json:"api"`
	Endpoints   []interface{} `json:"endpoints"`
}

//SgPoliciesInfo EndpointsInfo
type SgPoliciesInfo struct {
	ContentType string        `json:"content-type"`
	API         string        `json:"api"`
	SgPolicies  []interface{} `json:"sgpolicies"`
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
func (sg SgPoliciesInfo) _GetAgentObjects() []interface{} {
	var interfaceSlice = make([]interface{}, len(sg.SgPolicies))
	for i, d := range sg.SgPolicies {
		interfaceSlice[i] = d
	}
	return interfaceSlice
}

func (sg *SgPoliciesInfo) _GetAgentDelObjects() []interface{} {
	var interfaceSlice = make([]interface{}, len(sg.SgPolicies))
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

func (sg *SgPoliciesInfo) _GetAgentAPI() string {
	return AgentURL + sg.API
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

	resp, err := _httpClient.Do(req)
	if err != nil || resp.StatusCode != 200 {
		fmt.Println("Agent response :", resp)
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

//ConfigureNaples Configure Agent with config.
func ConfigureNaples(e2ecfg *E2eCfg) {
	fmt.Println("Number of configs", len(e2ecfg.EndpointsInfo.Endpoints))
	fmt.Println("Configuring Namespaces")
	_PushAgentConfig(&e2ecfg.NamespacesInfo)
	fmt.Println("Configuring Networks")
	_PushAgentConfig(&e2ecfg.NetworksInfo)
	fmt.Println("Configuring Endpoints")
	_PushAgentConfig(&e2ecfg.EndpointsInfo)
	fmt.Println("Configuring SgPolicies")
	_PushAgentConfig(&e2ecfg.SgPoliciesInfo)
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
