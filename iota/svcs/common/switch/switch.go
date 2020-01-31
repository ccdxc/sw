package DataSwitch

import (
	"fmt"
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

//QosClass qos classes
type QosClass struct {
	Name   string
	Mtu    uint32
	PfsCos uint32
}

//QosConfig qos config
type QosConfig struct {
	Name    string
	Classes []QosClass
}

//Switch interface
type Switch interface {
	Disconnect()
	SetNativeVlan(port string, vlan int) error
	UnsetNativeVlan(port string, vlan int) error
	LinkOp(port string, shutdown bool) error
	SetSpeed(port string, speed PortSpeed) error
	SetFlowControlReceive(port string, enable bool) error
	SetFlowControlSend(port string, enable bool) error
	SetMtu(port string, mtu uint32) error
	DisableIGMP(vlanRange string) error
	SetTrunkVlanRange(port string, vlanRange string) error
	UnsetTrunkVlanRange(port string, vlanRange string) error
	SetTrunkMode(port string) error
	UnsetTrunkMode(port string) error
	DoQosConfig(qosConfig *QosConfig) error
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

	//first create vlan
	err := sw.ConfigureVlans(strconv.Itoa(vlan), true)
	if err != nil {
		return err
	}

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

func (sw *nexus3k) ConfigureVlans(vlans string, igmpEnabled bool) error {

	out, err := n3k.ConfigVlan(sw.ctx, vlans, igmpEnabled, 10*time.Second)
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

func (sw *nexus3k) LinkOp(port string, shutdown bool) error {
	var cmds []string

	if shutdown {
		cmds = []string{"shutdown"}

	} else {
		cmds = []string{"no shutdown"}
	}

	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) SetFlowControlReceive(port string, enable bool) error {
	var cmds []string

	if enable {
		cmds = []string{"flowcontrol receive on"}

	} else {
		cmds = []string{"flowcontrol receive off"}
	}

	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) SetFlowControlSend(port string, enable bool) error {
	var cmds []string

	if enable {
		cmds = []string{"flowcontrol send on"}

	} else {
		cmds = []string{"flowcontrol send off"}
	}

	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) SetMtu(port string, mtu uint32) error {
	var cmds []string

	cmds = []string{fmt.Sprintf("mtu %v", mtu)}

	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) UnsetNativeVlan(port string, vlan int) error {
	cmds := []string{
		"no switchport trunk native vlan " + strconv.Itoa(vlan),
	}

	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) DisableIGMP(vlanRange string) error {
	//first create vlans
	err := sw.ConfigureVlans(vlanRange, false)
	if err != nil {
		return err
	}

	return nil
}

func (sw *nexus3k) SetTrunkVlanRange(port string, vlanRange string) error {

	//first create vlans
	err := sw.ConfigureVlans(vlanRange, true)
	if err != nil {
		return err
	}

	cmds := []string{
		"vlan " + vlanRange,
	}

	err = sw.runConfigCommands(port, cmds)
	if err != nil {
		return err
	}

	cmds = []string{
		"switchport trunk allowed vlan " + vlanRange,
		//for faster convergence
		"spanning-tree port type edge trunk",
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
	buf, err := n3k.CheckInterfaceConfigured(sw.ctx, port, mode.String(), status.String(),
		speedStr, 5*time.Second)

	return buf, err
}

func (sw *nexus3k) DoQosConfig(qosConfig *QosConfig) error {

	n3kQos := n3k.QosConfig{Name: qosConfig.Name}

	for _, qClass := range qosConfig.Classes {
		n3kQos.Classes = append(n3kQos.Classes, n3k.QosClass{Mtu: qClass.Mtu,
			Name: qClass.Name, PfsCos: qClass.PfsCos})
	}
	_, err := n3k.ConfigureQos(sw.ctx, &n3kQos, 5*time.Second)
	return err
}

func (sw *nexus3k) Disconnect() {
	n3k.Disconnect(sw.ctx)
}

//NewSwitch Create a new switch handler
func NewSwitch(swType, ip, username, password string) Switch {
	switch swType {
	case N3KSwitchType:
		return newnexus3k(ip+":22", username, password)
	}

	return nil
}
