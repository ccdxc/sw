package DataSwitch

import (
	"strconv"
	"time"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/iota/svcs/common/switch/n3k"
)

// N3KSwitchType represents n3k switch type
const N3KSwitchType = "n3k"

// PortMode is port mode
type PortMode int

// constants
const (
	Access PortMode = iota
	Trunk
)

func (s PortMode) String() string {
	return [...]string{"access", "trunk"}[s]
}

// PortStatus is port status
type PortStatus int

// enums
const (
	Up PortStatus = iota
	Down
)

func (s PortStatus) String() string {
	return [...]string{"up", "down"}[s]
}

// PortSpeed is port speed
type PortSpeed int

// enums
const (
	Speed100g PortSpeed = iota
	Speed10g
	SpeedAuto
)

type portSpeedValue int

const (
	speed100gVal portSpeedValue = iota
	speed10gVal
	speedAutoVal
)

func (s PortSpeed) String() string {
	return [...]string{"100G", "10G", "auto"}[s]
}

func (s portSpeedValue) String() string {
	return [...]string{"100000", "10000", "auto"}[s]
}

//Switch interface
type Switch interface {
	SetNativeVlan(port string, vlan int) error
	UnsetNativeVlan(port string) error
	SetSpeed(port string, speed PortSpeed) error
	SetTrunkVlanRange(port string, vlanRange string) error
	UnsetTrunkVlanRange(port string, vlanRange string) error
	SetTrunkMode(port string) error
	UnsetTrunkMode(port string) error
	CheckSwitchConfiguration(port string, mode PortMode, status PortStatus, speed PortSpeed) (string, error)
}

type nexus3k struct {
	username string
	password string
	ip       string
	ctx      *n3k.ConnectCtx
}

func newnexus3k(ip, username, password string) Switch {
	n3kInst := &nexus3k{username: username, password: password, ip: ip}
	ctx, err := n3k.Connect(ip, username, password)
	if err != nil {
		return nil
	}
	n3kInst.ctx = ctx
	return n3kInst
}

func (sw *nexus3k) runConfigIFCommands(port string, cmds []string) error {
	out, err := n3k.ConfigInterface(sw.ctx, port, cmds, 30*time.Second)
	log.Println("-------------------output-------------------")
	log.Println(out)
	if err != nil {
		log.Println("-------------------ERROR-------------------")
	}

	return err
}

func (sw *nexus3k) runConfigCommands(port string, cmds []string) error {
	out, err := n3k.Configure(sw.ctx, port, cmds, 30*time.Second)
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

	out, err := n3k.ConfigInterface(sw.ctx, port, cmds, 10*time.Second)
	log.Println("-------------------output-------------------")
	log.Println(out)
	if err != nil {
		log.Println("-------------------ERROR-------------------")
	}

	return err
}

func (sw *nexus3k) SetSpeed(port string, speed PortSpeed) error {

	cmds := []string{
		"speed " + (portSpeedValue(speed)).String(),
	}

	out, err := n3k.ConfigInterface(sw.ctx, port, cmds, 10*time.Second)
	log.Println("-------------------output-------------------")
	log.Println(out)
	if err != nil {
		log.Println("-------------------ERROR-------------------")
		return err
	}

	if speed == SpeedAuto {
		cmds = []string{
			"negotiate auto",
		}
	} else {
		cmds = []string{
			"no negotiate auto",
		}
	}

	out, err = n3k.ConfigInterface(sw.ctx, port, cmds, 10*time.Second)
	log.Println("-------------------output-------------------")
	log.Println(out)
	if err != nil {
		log.Println("-------------------ERROR-------------------")
		return err
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

func (sw *nexus3k) CheckSwitchConfiguration(port string, mode PortMode, status PortStatus, speed PortSpeed) (string, error) {

	speedStr := ""
	if speed == SpeedAuto {
		speedStr = ""
	} else {
		speedStr = speed.String()
	}
	buf, err := n3k.CheckInterfaceConigured(sw.ctx, port, mode.String(), status.String(),
		speedStr, 5*time.Second)

	return buf, err
}

//NewSwitch Create a new switch handler
func NewSwitch(swType, ip, username, password string) Switch {
	switch swType {
	case N3KSwitchType:
		return newnexus3k(ip+":22", username, password)
	}

	return nil
}
