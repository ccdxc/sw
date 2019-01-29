package DataSwitch

import (
	"strconv"
	"time"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/iota/svcs/common/switch/n3k"
)

const (
	N3KSwitchType = "n3k"
)

//Switch interface
type Switch interface {
	SetNativeVlan(port string, vlan int) error
	UnsetNativeVlan(port string) error
	SetTrunkVlanRange(port string, vlanRange string) error
	UnsetTrunkVlanRange(port string, vlanRange string) error
	SetTrunkMode(port string) error
	UnsetTrunkMode(port string) error
}

type nexus3k struct {
	username string
	password string
	ip       string
}

func newnexus3k(ip, username, password string) Switch {
	return &nexus3k{username: username, password: password, ip: ip}
}

func (sw *nexus3k) runConfigIFCommands(port string, cmds []string) error {
	out, err := n3k.ConfigInterface(sw.ip+":22", sw.username, sw.password, port, cmds, 30*time.Second)
	log.Println("-------------------output-------------------")
	log.Println(out)
	if err != nil {
		log.Println("-------------------ERROR-------------------")
	}

	return err
}

func (sw *nexus3k) runConfigCommands(port string, cmds []string) error {
	out, err := n3k.Configure(sw.ip+":22", sw.username, sw.password, port, cmds, 30*time.Second)
	log.Println("-------------------output-------------------")
	log.Println(out)
	if err != nil {
		log.Println("-------------------ERROR-------------------")
	}

	return err
}

func (sw *nexus3k) SetNativeVlan(port string, vlan int) error {

	cmds := []string{
		"switchport trunk native vlan " + strconv.Itoa(vlan),
	}

	out, err := n3k.ConfigInterface(sw.ip+":22", sw.username, sw.password, port, cmds, 10*time.Second)
	log.Println("-------------------output-------------------")
	log.Println(out)
	if err != nil {
		log.Println("-------------------ERROR-------------------")
	}

	return err
}

func (sw *nexus3k) UnsetNativeVlan(port string) error {
	cmds := []string{
		"no switchport trunk native vlan",
	}

	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) SetTrunkVlanRange(port string, vlanRange string) error {

	cmds := []string{
		"vlan " + vlanRange,
	}

	err := sw.runConfigCommands(port, cmds)
	if err != nil {
		return err
	}

	cmds = []string{
		"switchport trunk allowed vlan " + vlanRange,
	}
	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) UnsetTrunkVlanRange(port string, vlanRange string) error {
	cmds := []string{
		"no switchport trunk allowed vlan " + vlanRange,
	}
	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) SetTrunkMode(port string) error {
	cmds := []string{
		"switchport mode trunk",
	}

	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) UnsetTrunkMode(port string) error {
	cmds := []string{
		"no switchport mode trunk",
	}

	return sw.runConfigIFCommands(port, cmds)
}

//NewSwitch Create a new switch handler
func NewSwitch(swType, ip, username, password string) Switch {
	switch swType {
	case N3KSwitchType:
		return newnexus3k(ip, username, password)
	}

	return nil
}
