package DataSwitch

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/greenpau/go-cisco-nx-api/pkg/client"
	"github.com/pensando/sw/venice/utils/log"
)

// define Rest API instance of Nexus 3k
type nexus3kRest struct {
	username string
	password string
	ip       string
	clt      *client.Client
}

func newNexus3kRest(ip, username, password string) Switch {
	clt := client.NewClient()

	err := clt.SetHost(ip)
	if err != nil {
		return nil
	}

	err = clt.SetUsername(username)
	if err != nil {
		return nil
	}

	err = clt.SetPassword(password)
	if err != nil {
		return nil
	}

	_, err = clt.GetSystemInfo()
	if err != nil {
		return nil
	}

	n3kInst := &nexus3kRest{username: username, password: password, ip: ip, clt: clt}

	return n3kInst
}

func (sw *nexus3kRest) runConfigCommands(cmds []string) error {
	log.Infof("Configuring commands: %v\n", cmds)
	resp, err := sw.clt.Configure(cmds)
	if err != nil {
		return err
	}

	for i, r := range resp {
		if r.Error != nil {
			return fmt.Errorf("ERROR: switch returned failure for command %s: %v", cmds[i], r.Error.Message)
		}
	}
	return nil
}

func (sw *nexus3kRest) Disconnect() {
	// do nothing
}

func (sw *nexus3kRest) SetNativeVlan(port string, vlan int) error {
	//first create vlan
	err := sw.ConfigureVlans(strconv.Itoa(vlan), true)
	if err != nil {
		return err
	}

	cmds := []string{
		"interface " + port,
		"switchport trunk native vlan " + strconv.Itoa(vlan),
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) UnsetNativeVlan(port string, vlan int) error {
	cmds := []string{
		"interface " + port,
		"no switchport trunk native vlan " + strconv.Itoa(vlan),
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) LinkOp(port string, shutdown bool) error {
	cmds := []string{
		"interface " + port,
	}

	if shutdown {
		cmds = append(cmds, "shutdown")
	} else {
		cmds = append(cmds, "no shutdown")
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) ConfigureVlans(vlans string, igmpEnabled bool) error {
	cmds := []string{
		"spanning-tree mode mst",
		"vlan " + vlans,
		fmt.Sprintf("vlan config %s", vlans),
	}

	if igmpEnabled {
		cmds = append(cmds, "ip igmp snooping")
	} else {
		cmds = append(cmds, "no ip igmp snooping")
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) SetSpeed(port string, speed PortSpeed) error {

	cmds := []string{
		"interface " + port,
		"speed " + (portSpeedValue(speed)).String(),
	}

	if speed == SpeedAuto {
		cmds = append(cmds, "negotiate auto")
	} else {
		cmds = append(cmds, "no negotiate auto")
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) SetFlowControlReceive(port string, enable bool) error {
	cmds := []string{
		"interface " + port,
	}

	if enable {
		cmds = append(cmds, "flowcontrol receive on")
	} else {
		cmds = append(cmds, "flowcontrol receive off")
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) SetFlowControlSend(port string, enable bool) error {
	cmds := []string{
		"interface " + port,
	}

	if enable {
		cmds = append(cmds, "flowcontrol send on")
	} else {
		cmds = append(cmds, "flowcontrol send off")
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) SetMtu(port string, mtu uint32) error {
	cmds := []string{
		"interface " + port,
		fmt.Sprintf("mtu %v", mtu),
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) DisableIGMP(vlanRange string) error {
	//make sure vlans created
	err := sw.ConfigureVlans(vlanRange, false)
	if err != nil {
		return err
	}

	return nil
}

func (sw *nexus3kRest) SetTrunkVlanRange(port string, vlanRange string) error {

	//first create vlans
	err := sw.ConfigureVlans(vlanRange, true)
	if err != nil {
		return err
	}
	cmds := []string{
		"interface " + port,
		"switchport trunk allowed vlan " + vlanRange,
		//for faster convergence
		"spanning-tree port type edge trunk",
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) UnsetTrunkVlanRange(port string, vlanRange string) error {
	cmds := []string{
		"interface " + port,
		"no switchport trunk allowed vlan " + vlanRange,
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) SetTrunkMode(port string) error {
	cmds := []string{
		"interface " + port,
		"switchport mode trunk",
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) UnsetTrunkMode(port string) error {
	cmds := []string{
		"interface " + port,
		"no switchport mode trunk",
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) CheckSwitchConfiguration(port string, mode PortMode, status PortStatus, speed PortSpeed) (string, error) {
	// skipped
	return "", nil
}

func (sw *nexus3kRest) DoQosConfig(qosCfg *QosConfig) error {
	sysInfo, err := sw.clt.GetSystemInfo()
	if err != nil {
		return err
	}

	pfcSupport := true
	if !strings.Contains(sysInfo.ChassisID, "Nexus3000") {
		// Nexus9000 doesn't fully support PFC COS. Skip it.
		pfcSupport = false
	}

	cmds := []string{
		"policy-map type network-qos " + qosCfg.Name,
	}

	if len(qosCfg.Classes) != 0 {
		for _, qosClass := range qosCfg.Classes {
			cmds = append(cmds, "class type network-qos "+qosClass.Name)
			if qosClass.Mtu != 0 {
				cmds = append(cmds, fmt.Sprintf("mtu %v", qosClass.Mtu))
			}

			if qosClass.PfsCos <= 7 && pfcSupport {
				cmds = append(cmds, fmt.Sprintf("pause pfc-cos %v", qosClass.PfsCos))
			}

			cmds = append(cmds, "exit")
		}
	}

	cmds = append(cmds, "system qos")
	cmds = append(cmds, "service-policy type network-qos "+qosCfg.Name)
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) SetPortQueuing(port string, enable bool, params string) error {

	cmds := []string{
		"interface " + port,
	}
	if enable {
		cmds = append(cmds, fmt.Sprintf("service-policy type queuing output %s", params))
	} else {
		cmds = append(cmds, fmt.Sprintf("no service-policy type queuing output %s", params))
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) SetPortQos(port string, enable bool, params string) error {
	cmds := []string{
		"interface " + port,
	}
	if enable {
		cmds = append(cmds, fmt.Sprintf("service-policy type qos input %s", params))
	} else {
		cmds = append(cmds, fmt.Sprintf("no service-policy type qos input %s", params))
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) SetPortPause(port string, enable bool) error {
	cmds := []string{
		"interface " + port,
	}
	if enable {
		cmds = append(cmds, "flowcontrol send on", "flowcontrol receive on")
	} else {
		cmds = append(cmds, "no flowcontrol send on", "no flowcontrol receive on")
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) SetPortPfc(port string, enable bool) error {
	cmds := []string{
		"interface " + port,
	}
	if enable {
		cmds = append(cmds, "no flowcontrol send on", "no flowcontrol receive on", "priority-flow-control mode on")
	} else {
		cmds = append(cmds, "no priority-flow-control mode on")
	}
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) DoDscpConfig(dscpConfig *DscpConfig) error {

	cmds := []string{
		"class-map type network-qos " + dscpConfig.Name,
	}
	if len(dscpConfig.Classes) != 0 {
		for _, dscpQosClass := range dscpConfig.Classes {
			cmds = append(cmds, "class-map type qos match-any "+dscpQosClass.Name)
			cmds = append(cmds, fmt.Sprintf("match cos %v", dscpQosClass.Cos))
			cmds = append(cmds, fmt.Sprintf("match dscp %v", dscpQosClass.Dscp))
			cmds = append(cmds, "exit")
		}
	}

	cmds = append(cmds, "system qos")
	cmds = append(cmds, "service-policy type network-qos "+dscpConfig.Name)
	return sw.runConfigCommands(cmds)
}

func (sw *nexus3kRest) DoQueueConfig(queueConfig *QueueConfig) error {
	cmds := []string{
		"policy-map type queuing ", queueConfig.Name,
	}
	if len(queueConfig.Classes) != 0 {
		for _, queueQosClass := range queueConfig.Classes {
			cmds = append(cmds, "class type queuing "+queueQosClass.Name)
			cmds = append(cmds, fmt.Sprintf("priority level %v", queueQosClass.Priority))
			cmds = append(cmds, fmt.Sprintf("bandwidth remaining percent %v", queueQosClass.Percent))
			cmds = append(cmds, "random-detect minimum-threshold 50 kbytes maximum-threshold 500 kbytes drop-probability 80 weight 15 ecn")
			cmds = append(cmds, "shape min 20 gbps max 20 gbps")
			cmds = append(cmds, "exit")
		}
	}
	cmds = append(cmds, "system qos")
	cmds = append(cmds, "service-policy type network-qos "+queueConfig.Name)
	return sw.runConfigCommands(cmds)
}
