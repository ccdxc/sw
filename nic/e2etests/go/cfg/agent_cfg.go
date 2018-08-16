package cfg

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
	"strconv"
	"time"

	yaml "gopkg.in/yaml.v2"

	Common "github.com/pensando/sw/nic/e2etests/go/common"
)

//PostMethod Method used for posting
var PostMethod = http.Post

//AgentURL URL agent endpoint
const AgentURL = "http://127.0.0.1:9007/"

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
		interfaceSlice[i] = ns._GetAgentAPI() + "default" + "/" + d.NSMeta.Name
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

func _GenerateNameSpaces(e2eCfg *E2eCfg, scaleConfig *E2ETScaleTestCfg) {
	e2eCfg.NamespacesInfo.API = scaleConfig.Namespaces.API
	for i := 0; i < scaleConfig.Namespaces.Count; i++ {
		ns := Namespace{Kind: "Namespace",
			NSMeta: NSMeta{Name: "SCALE_NS_" + strconv.Itoa(i), Tenant: "default"}}
		_GenerateNetworks(e2eCfg, &ns, scaleConfig)
		e2eCfg.NamespacesInfo.Namespaces = append(e2eCfg.NamespacesInfo.Namespaces, ns)
	}

}

var _CurNwAddr = "64.0.0.1/24"

func _NextNwAddress() string {
	nwAddr, _ := Common.IncrementCidr(_CurNwAddr)
	_CurNwAddr = nwAddr
	return nwAddr
}
func _GenerateNetworks(e2eCfg *E2eCfg, nsInfo *Namespace,
	scaleConfig *E2ETScaleTestCfg) {
	e2eCfg.NetworksInfo.API = scaleConfig.Networks.API
	for i := 0; i < scaleConfig.Networks.Count; i++ {
		nw := Network{Kind: "Network",
			NetworkMeta: NetworkMeta{Name: "NW_" + nsInfo.NSMeta.Name + strconv.Itoa(i),
				Tenant:    "default",
				Namespace: nsInfo.NSMeta.Name}}
		nw.NetworkSpec.Ipv4Subnet = _NextNwAddress()
		ip, _, _ := net.ParseCIDR(nw.NetworkSpec.Ipv4Subnet)
		ip[3] = 1
		nw.NetworkSpec.Ipv4Gateway = ip.String()
		nw.NetworkSpec.VlanID = _NextVlan()
		_GenerateEndpoints(e2eCfg, &nw, scaleConfig)
		e2eCfg.NetworksInfo.Networks = append(e2eCfg.NetworksInfo.Networks, nw)
	}
}

var curHwAddress = "00:22:0A:00:03:14"

func _NextMacAddress() string {
	addr, _ := Common.IncrementMacAddress(curHwAddress)
	curHwAddress = addr
	return addr
}

var curUsegVlan = 0

func _NextUsegVlan() int {
	curUsegVlan++
	return curUsegVlan
}

var curVlan = 0

func _NextVlan() int {
	curVlan++
	return curVlan
}

var curEP = 0

func _NextEpID() int {
	curEP++
	return curEP
}

var startLifID int
var maxLifs int
var curLifID int

func _NextLif() string {
	if (maxLifs + startLifID - 1)  <= curLifID {
		curLifID = startLifID
	} else {
		curLifID = (curLifID + 1)
	}
	return "lif" + strconv.Itoa(curLifID)
}

func _GenerateEndpoints(e2eCfg *E2eCfg, nwInfo *Network,
	scaleConfig *E2ETScaleTestCfg) {
	e2eCfg.EndpointsInfo.API = scaleConfig.Endpoints.API
	ipAddr, _, _ := net.ParseCIDR(nwInfo.NetworkSpec.Ipv4Subnet)
	curIPAddr := ipAddr.String()
	nextIPAddr := func() string {
		nwAddr, _ := Common.IncrementIP(curIPAddr,
			nwInfo.NetworkSpec.Ipv4Subnet)
		curIPAddr = nwAddr
		return nwAddr
	}

	startUplinkid := scaleConfig.Interfaces.Remote.UplinkStart
	_nextUplink := func() string {
	//	startUplinkid = (startUplinkid + 1) % scaleConfig.Interfaces.Remote.Uplinks
		return "uplink" + strconv.Itoa(startUplinkid)
	}
	initEp := func(i int) Endpoint {
		ep := Endpoint{Kind: "Endpoint",
			EndpointMeta: EndpointMeta{Name: "EP_" + strconv.Itoa(_NextEpID()),
				Tenant:    "default",
				Namespace: nwInfo.NetworkMeta.Namespace}}
		ep.EndpointSpec.NetworkName = nwInfo.NetworkMeta.Name
		ep.EndpointSpec.MacAddresss = _NextMacAddress()
		ep.EndpointSpec.Ipv4Address = nextIPAddr() + "/32"
		return ep
	}
	for i := 0; i < scaleConfig.Endpoints.Useg; i++ {
		ep := initEp(i)
		ep.EndpointSpec.Interface = _NextLif()
		ep.EndpointSpec.InterfaceType = "lif"
		ep.EndpointSpec.UsegVlan = _NextUsegVlan()
		e2eCfg.EndpointsInfo.Endpoints =
			append(e2eCfg.EndpointsInfo.Endpoints, ep)
	}
	for i := 0; i < scaleConfig.Endpoints.Remote; i++ {
		ep := initEp(i + scaleConfig.Endpoints.Useg)
		ep.EndpointSpec.Interface = _nextUplink()
		ep.EndpointSpec.InterfaceType = "uplink"
		ep.EndpointSpec.NodeUUID = "GWUUID"
		e2eCfg.EndpointsInfo.Endpoints =
			append(e2eCfg.EndpointsInfo.Endpoints, ep)
	}
}

func _GenerateConfig(e2eCfg *E2eCfg, scaleConfig *E2ETScaleTestCfg) {
	startLifID = scaleConfig.Interfaces.Host.LifStart
	maxLifs = scaleConfig.Interfaces.Host.Lifs
	curLifID = startLifID
	_GenerateNameSpaces(e2eCfg, scaleConfig)
}

//GetScaleAgentConfig read test specs from given directory
func GetScaleAgentConfig(file string) *E2eCfg {
	yamlFile, err := os.Open(file)
	if err != nil {
		fmt.Println(err)
	}
	defer yamlFile.Close()
	byteValue, _ := ioutil.ReadAll(yamlFile)
	var scaleConfig E2ETScaleTestCfg
	yaml.Unmarshal(byteValue, &scaleConfig)

	/* Now generate the config */
	var e2eCfg E2eCfg
	_GenerateConfig(&e2eCfg, &scaleConfig)
	return &e2eCfg
}

// GetNumOfLifs get number of lifs to create
func GetNumOfLifs(file string) int {
	yamlFile, err := os.Open(file)
	if err != nil {
		fmt.Println(err)
	}
	defer yamlFile.Close()
	byteValue, _ := ioutil.ReadAll(yamlFile)
	var scaleConfig E2ETScaleTestCfg
	yaml.Unmarshal(byteValue, &scaleConfig)
	return scaleConfig.Interfaces.Host.Lifs
}
