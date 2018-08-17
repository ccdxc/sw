package infra

import (
	"fmt"
	"log"
	"regexp"
	"strconv"
	"strings"

	"github.com/pensando/sw/nic/e2etests/go/cfg"
	"github.com/pensando/sw/nic/e2etests/go/common"
)

//EpInterface EpInterface struct
type EpInterface struct {
	ID         int
	Name       string
	IPAddress  string
	PrefixLen  int
	MacAddress string
	EncapVlan  int
}

//Endpoint Endpoint struct
type Endpoint struct {
	Name      string
	Remote    bool
	Interface EpInterface
	AppEngine AppEngine
}

func getIDFromString(str string) (int, error) {
	re := regexp.MustCompile("[0-9]+")
	return strconv.Atoi((re.FindAllString(str, 1))[0])
}

func getNetworkFromConfig(nwName string, fullCfg *cfg.E2eCfg) *cfg.Network {
	for _, network := range fullCfg.NetworksInfo.Networks {
		if network.NetworkMeta.Name == nwName {
			return &network
		}
	}

	return nil
}

//Init Endpoint Init
func (ep *Endpoint) Init(container bool) {
	fmt.Println("Initialzing endpoint : ", ep.Name)
	if container {
		ep.AppEngine = NewContainer(ep.Name, "", "")
	} else {
		ep.AppEngine = NewNS(ep.Name)
		ep.AppEngine.BringUp()
	}
	ep.AppEngine.AttachInterface(ep.Interface.Name)
	ep.AppEngine.SetMacAddress(ep.Interface.Name, ep.Interface.MacAddress, 0)
	if ep.Interface.EncapVlan != 0 {
		ep.AppEngine.AddVlan(ep.Interface.Name, ep.Interface.EncapVlan)
		ep.AppEngine.SetMacAddress(ep.Interface.Name, ep.Interface.MacAddress, ep.Interface.EncapVlan)
	}
	if ep.Interface.IPAddress != "" {
		ep.AppEngine.SetIPAddress(ep.Interface.Name, ep.Interface.IPAddress, ep.Interface.PrefixLen,
			ep.Interface.EncapVlan)
	}
}

//Delete Endpoint Delete
func (ep *Endpoint) Delete() {
	fmt.Println("Cleaning up endpoint : ", ep.Name)
	ep.AppEngine.Stop()
}

//RunCommand Endpoint Run command
func (ep *Endpoint) RunCommand(cmd []string, timeout int, background bool) (common.CmdHandle, error) {
	return ep.AppEngine.RunCommand(cmd, 0, background)
}

//GetIP Endpoint Get IP
func (ep *Endpoint) GetIP() string {
	return ep.Interface.IPAddress
}

//GetIP Endpoint Get IP
func (ep *Endpoint) GetNetwork() string {
	return ep.Interface.IPAddress + "/" + strconv.Itoa(ep.Interface.PrefixLen)
}

//PrintInformation Print Endpoint Information
func (ep *Endpoint) PrintInformation() {
	var epType string
	if ep.Remote {
		epType = "Remote"
	} else {
		epType = "Local"
	}
	fmt.Println("\t Endpoint Type      : ", epType)
	ep.AppEngine.PrintAppInformation()
}

//NewEndpoint Instantiate new endpoint
func NewEndpoint(name string, data cfg.Endpoint, fullCfg *cfg.E2eCfg) *Endpoint {

	intfID, err := getIDFromString(data.EndpointSpec.Interface)
	network := getNetworkFromConfig(data.EndpointSpec.NetworkName,
		fullCfg)

	if network == nil {
		log.Fatalln("Network not found in config!", data.EndpointSpec.NetworkName)
	}

	remote := data.EndpointSpec.InterfaceType != "lif"

	if err != nil {
		log.Fatalln(err)
	}

	intf := EpInterface{
		ID:         intfID,
		Name:       name,
		MacAddress: data.EndpointSpec.MacAddresss,
		IPAddress:  strings.Split(data.EndpointSpec.Ipv4Address, "/")[0],
		PrefixLen:  24,
	}
	if remote {
		intf.EncapVlan = network.NetworkSpec.VlanID
	} else {
		intf.EncapVlan = data.EndpointSpec.UsegVlan
	}

	return &Endpoint{
		Name:      name,
		Remote:    remote,
		Interface: intf,
	}
}
