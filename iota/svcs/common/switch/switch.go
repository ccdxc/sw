package DataSwitch

import "github.com/pensando/sw/venice/utils/log"

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

//DscpClass qos classes
type DscpClass struct {
	Name string
	Dscp string
	Cos  uint32
}

//DscpConfig qos config
type DscpConfig struct {
	Name    string
	Classes []DscpClass
}

//QueueClass qos classes
type QueueClass struct {
	Name     string
	Priority uint32
	Percent  uint32
}

//QueueConfig qos config
type QueueConfig struct {
	Name    string
	Classes []QueueClass
}

//Switch interface
type Switch interface {
	Disconnect()
	SetNativeVlan(port string, vlan int) error
	UnsetNativeVlan(port string, vlan int) error
	LinkOp(port string, shutdown bool) error
	SetPortQueuing(port string, enable bool, params string) error
	SetPortQos(port string, enable bool, params string) error
	SetPortPause(port string, enable bool) error
	SetPortPfc(port string, enable bool) error
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
	DoDscpConfig(dscpConfig *DscpConfig) error
	DoQueueConfig(queueConfig *QueueConfig) error
	CheckSwitchConfiguration(port string, mode PortMode, status PortStatus, speed PortSpeed) (string, error)
}

//NewSwitch Create a new switch handler
func NewSwitch(swType, ip, username, password string) Switch {
	switch swType {
	case N3KSwitchType:
		sw := newNexus3kRest(ip, username, password)
		if sw != nil {
			log.Info("Switch Rest API mode enabled\n")
			return sw
		}
		return newNexus3kSsh(ip+":22", username, password)
	}
	return nil
}
