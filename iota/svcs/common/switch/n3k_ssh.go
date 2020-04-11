package DataSwitch

import (
	"bytes"
	"fmt"
	"io"
	"regexp"
	"strconv"
	"strings"
	"time"

	expect "github.com/pensando/goexpect"
	"github.com/pkg/errors"
	"golang.org/x/crypto/ssh"

	"github.com/pensando/sw/venice/utils/log"
)

var (
	exitTimeout        = 500 * time.Millisecond
	promptRegex        = regexp.MustCompile("# ")
	configRegex        = regexp.MustCompile(`\(config\)\# `)
	configIfRegex      = regexp.MustCompile(`\(config-if\)\# `)
	configVlanCfgRegex = regexp.MustCompile(`\(config-vlan-config\)\# `)
	configCmapNwQos    = regexp.MustCompile(`\(config-cmap-nqos\)\# `)
	configPmapNwQos    = regexp.MustCompile(`\(config-pmap-nqos\)\# `)
	configNwQosClass   = regexp.MustCompile(`\(config-pmap-nqos-c\)\# `)
	configDscpCmap     = regexp.MustCompile(`\(config-cmap-qos\)\# `)
	configDscpQos      = regexp.MustCompile(`\(config-pmap-qos\)\# `)
	configDscpClass    = regexp.MustCompile(`\(config-pmap-c-qos\)\# `)
	configQueueQos     = regexp.MustCompile(`\(config-pmap-que\)\# `)
	configQueueClass   = regexp.MustCompile(`\(config-pmap-c-que\)\# `)
	configSystemQos    = regexp.MustCompile(`\(config-sys-qos\)\# `)
)

type writerProxy struct {
	w io.Writer
}

// connection context
type connectCtx struct {
	sshClt *ssh.Client
}

// Write writes data to proxy
func (wp *writerProxy) Write(p []byte) (n int, err error) {
	return wp.w.Write(p)
}

// Close close proxy
func (wp *writerProxy) Close() error {
	return nil
}

func spawnExpectSSH(ip, user, password string, timeout time.Duration) (*ssh.Client, error) {
	sshClt, err := ssh.Dial("tcp", ip, &ssh.ClientConfig{
		User: user,
		Auth: []ssh.AuthMethod{
			ssh.Password(password),
			ssh.KeyboardInteractive(func(user, instruction string, questions []string, echos []bool) (answers []string, err error) {
				answers = make([]string, len(questions))
				for n := range questions {
					answers[n] = password
				}

				return answers, nil
			}),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	})
	if err != nil {
		return nil, errors.Wrapf(err, "while ssh dial %s", ip)
	}

	return sshClt, nil
}

// connects to switch, enter config mode, and program interface
func configInterface(n3k *connectCtx, port string, commands []string, timeout time.Duration) (string, error) {
	buf := &bytes.Buffer{}
	exp, err := spawnExp(n3k, buf)
	if err != nil {
		return "", errors.Wrapf(err, "while spawn goexpect")
	}

	defer exp.Close()
	_, _, err = exp.Expect(promptRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	if err = exp.Send("conf\n"); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configRegex, timeout)
	if err != nil {
		return buf.String(), err
	}
	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	if err := confIfCmd(exp, fmt.Sprintf("int %s\n", port), timeout); err != nil {
		return buf.String(), err
	}
	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	for _, cmd := range commands {
		if err := confIfCmd(exp, cmd+"\n", timeout); err != nil {
			return buf.String(), err
		}
	}

	return buf.String(), nil
}

// configure qos on switch
func configureQos(n3k *connectCtx, qosCfg *QosConfig, timeout time.Duration) (string, error) {

	buf := &bytes.Buffer{}
	exp, err := spawnExp(n3k, buf)
	if err != nil {
		return "", errors.Wrapf(err, "while spawn goexpect")
	}
	defer exp.Close()

	_, _, err = exp.Expect(promptRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	if err = exp.Send("conf\n"); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	if len(qosCfg.Classes) != 0 {
		for _, qosClass := range qosCfg.Classes {
			nwQosClassCmd := fmt.Sprintf("class-map type network-qos %v\n", qosClass.Name)

			if err = exp.Send(nwQosClassCmd); err != nil {
				return buf.String(), err
			}
			_, _, err = exp.Expect(configCmapNwQos, timeout)
			if err != nil {
				return buf.String(), err
			}

			pfcCmd := fmt.Sprintf("match qos-group %v\n", qosClass.PfsCos)
			if err = exp.Send(pfcCmd); err != nil {
				return buf.String(), err
			}

			exp.Send("exit\n") //exit configCmapNwQos
			time.Sleep(exitTimeout)
		}
	}

	nwQos := fmt.Sprintf("policy-map type network-qos %v\n", qosCfg.Name)
	//Set spanning tree mode to mst to support more vlans
	if err = exp.Send(nwQos); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configPmapNwQos, timeout)
	if err != nil {
		return buf.String(), err
	}
	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	if len(qosCfg.Classes) != 0 {
		for _, qosClass := range qosCfg.Classes {
			qosCmd := fmt.Sprintf("class type network-qos %v\n", qosClass.Name)

			if err = exp.Send(qosCmd); err != nil {
				return buf.String(), err
			}
			_, _, err = exp.Expect(configNwQosClass, timeout)
			if err != nil {
				return buf.String(), err
			}

			if qosClass.Mtu != 0 {
				mtuCmd := fmt.Sprintf("mtu %v\n", qosClass.Mtu)
				if err = exp.Send(mtuCmd); err != nil {
					return buf.String(), err
				}
			}

			if qosClass.PfsCos <= 7 {
				// 7 is maximum allowed CoS value so far
				pfcCmd := fmt.Sprintf("pause pfc-cos %v\n", qosClass.PfsCos)
				if err = exp.Send(pfcCmd); err != nil {
					return buf.String(), err
				}
			}

			exp.Send("exit\n") //exit configNwQosClass 
			time.Sleep(exitTimeout)
		}
	}

	if err = exp.Send("system qos\n"); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configSystemQos, timeout)
	if err != nil {
		return buf.String(), err
	}

	systemQosCmd := fmt.Sprintf("service-policy type network-qos %v\n", qosCfg.Name)

	if err = exp.Send(systemQosCmd); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configSystemQos, timeout)
	if err != nil {
		return buf.String(), err
	}

	exp.Send("exit\n") // exit system qos
	time.Sleep(exitTimeout)

	return "", nil
}

func (sw *nexus3k) DoDscpConfig(dscpCfg *DscpConfig) error {
	timeout := 5 * time.Second

	buf := &bytes.Buffer{}
	exp, err := spawnExp(sw.ctx, buf)
	if err != nil {
		return errors.Wrapf(err, "while spawn goexpect")
	}
	defer exp.Close()

	_, _, err = exp.Expect(promptRegex, timeout)
	if err != nil {
		return err
	}

	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	if err = exp.Send("conf\n"); err != nil {
		return err
	}
	_, _, err = exp.Expect(configRegex, timeout)
	if err != nil {
		return err
	}

	if len(dscpCfg.Classes) != 0 {
		for _, dscpQosClass := range dscpCfg.Classes {
			cmd := fmt.Sprintf("class-map type qos match-any %v\n", dscpQosClass.Name)

			if err = exp.Send(cmd); err != nil {
				return err
			}
			_, _, err = exp.Expect(configDscpCmap, timeout)
			if err != nil {
				return err
			}

			cmd = fmt.Sprintf("match cos %v\n", dscpQosClass.Cos)
			if err = exp.Send(cmd); err != nil {
				return err
			}

			cmd = fmt.Sprintf("match dscp %v\n", dscpQosClass.Dscp)
			if err = exp.Send(cmd); err != nil {
				return err
			}

			exp.Send("exit\n") //exit configDscpClass
			time.Sleep(exitTimeout)
		}
	}

	nwQos := fmt.Sprintf("policy-map type qos %v\n", dscpCfg.Name)
	if err = exp.Send(nwQos); err != nil {
		return err
	}
	_, _, err = exp.Expect(configDscpQos, timeout)
	if err != nil {
		return err
	}
	if len(dscpCfg.Classes) != 0 {
		for _, dscpQosClass := range dscpCfg.Classes {
			cmd := fmt.Sprintf("class %v\n", dscpQosClass.Name)

			if err = exp.Send(cmd); err != nil {
				return err
			}
			_, _, err = exp.Expect(configDscpClass, timeout)
			if err != nil {
				return err
			}

			cmd = fmt.Sprintf("set qos-group %v\n", dscpQosClass.Cos)
			if err = exp.Send(cmd); err != nil {
				return err
			}

			exp.Send("exit\n") //exit configDscpClass
			time.Sleep(exitTimeout)
		}
	}

	exp.Send("exit\n") // exit system qos
	time.Sleep(exitTimeout)

	return nil
}

//ConfigureQueue configure qos on switch
func ConfigureQueue(n3k *connectCtx, queueCfg *QueueConfig, timeout time.Duration) (string, error) {

	buf := &bytes.Buffer{}
	exp, err := spawnExp(n3k, buf)
	if err != nil {
		return "", errors.Wrapf(err, "while spawn goexpect")
	}
	defer exp.Close()

	_, _, err = exp.Expect(promptRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	if err = exp.Send("conf\n"); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	nwQos := fmt.Sprintf("policy-map type queuing %v\n", queueCfg.Name)
	if err = exp.Send(nwQos); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configQueueQos, timeout)
	if err != nil {
		return buf.String(), err
	}
	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	if len(queueCfg.Classes) != 0 {
		for _, queueQosClass := range queueCfg.Classes {
			cmd := fmt.Sprintf("class type queuing %v\n", queueQosClass.Name)

			if err = exp.Send(cmd); err != nil {
				return buf.String(), err
			}
			_, _, err = exp.Expect(configQueueClass, timeout)
			if err != nil {
				return buf.String(), err
			}

			cmd = fmt.Sprintf("priority level %v\n", queueQosClass.Priority)
			if err = exp.Send(cmd); err != nil {
				return buf.String(), err
			}

			cmd = fmt.Sprintf("bandwidth remaining percent %v\n", queueQosClass.Percent)
			if err = exp.Send(cmd); err != nil {
				return buf.String(), err
			}

			cmd = fmt.Sprintf("random-detect minimum-threshold 50 kbytes maximum-threshold 500 kbytes drop-probability 80 weight 15 ecn")
			if err = exp.Send(cmd); err != nil {
				return buf.String(), err
			}

			cmd = fmt.Sprintf("shape min 20 gbps max 20 gbps")
			if err = exp.Send(cmd); err != nil {
				return buf.String(), err
			}

			exp.Send("exit\n") //exit configQueueClass
			time.Sleep(exitTimeout)
		}
	}

	if err = exp.Send("system qos\n"); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configSystemQos, timeout)
	if err != nil {
		return buf.String(), err
	}

	systemQosCmd := fmt.Sprintf("service-policy type network-qos %v\n", queueCfg.Name)

	if err = exp.Send(systemQosCmd); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configSystemQos, timeout)
	if err != nil {
		return buf.String(), err
	}

	exp.Send("exit\n") // exit system qos
	time.Sleep(exitTimeout)

	return "", nil
}

// configures vlan on the interface
func configVlan(n3k *connectCtx, vlanRange string, igmpEnabled bool, timeout time.Duration) (string, error) {
	buf := &bytes.Buffer{}
	exp, err := spawnExp(n3k, buf)
	if err != nil {
		return "", errors.Wrapf(err, "while spawn goexpect")
	}

	defer exp.Close()
	_, _, err = exp.Expect(promptRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	if err = exp.Send("conf\n"); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configRegex, timeout)
	if err != nil {
		return buf.String(), err
	}
	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	//Set spanning tree mode to mst to support more vlans
	if err = exp.Send("spanning-tree mode mst\n"); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	if err := confVlanCfgCmd(exp, fmt.Sprintf("vlan config %s\n", vlanRange), timeout); err != nil {
		return buf.String(), err
	}

	var cmd string
	if igmpEnabled {
		cmd = "ip igmp snooping\n"
	} else {
		cmd = "no ip igmp snooping\n"
	}

	if err = exp.Send(cmd); err != nil {
		return buf.String(), err
	}

	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	return buf.String(), nil
}

// connects to switch, enter config mode,
func configure(n3k *connectCtx, commands []string, timeout time.Duration) (string, error) {
	buf := &bytes.Buffer{}
	exp, err := spawnExp(n3k, buf)
	if err != nil {
		return "", errors.Wrapf(err, "while spawn goexpect")
	}

	defer exp.Close()
	_, _, err = exp.Expect(promptRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	if err = exp.Send("conf\n"); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configRegex, timeout)
	if err != nil {
		return buf.String(), err
	}
	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	for _, cmd := range commands {
		if err := confCmd(exp, cmd+"\n", timeout); err != nil {
			return buf.String(), err
		}
	}

	return buf.String(), nil
}

func confCmd(exp expect.Expecter, cmd string, timeout time.Duration) error {
	if err := exp.Send(cmd); err != nil {
		return err
	}

	exp.Send("exit\n")
	time.Sleep(exitTimeout)

	_, _, err := exp.Expect(configRegex, timeout)
	return err
}

func interfaceConfigured(exp expect.Expecter, port, mode, status, speed string, timeout time.Duration) error {
	sendStr := fmt.Sprintf("show interface %s brief | grep Eth", port)

	port = strings.Replace(port, "e", "Eth", -1)

	matchInterfaceRegex := &regexp.Regexp{}
	if speed != "" {
		matchInterfaceRegex = regexp.MustCompile(fmt.Sprintf("(.*)%s(.*)%s(.*)%s(.*)%s(.*)", port, mode, status, speed))
	} else {
		matchInterfaceRegex = regexp.MustCompile(fmt.Sprintf("(.*)%s(.*)%s(.*)%s(.*)", port, mode, status))
	}

	if err := exp.Send("show clock" + "\n"); err != nil {
		return err
	}
	if err := exp.Send(sendStr + "\n"); err != nil {
		return err
	}
	_, _, err := exp.Expect(matchInterfaceRegex, timeout)
	if err != nil {
		return err
	}
	return err

}

func spawnExp(n3k *connectCtx, buf *bytes.Buffer) (expect.Expecter, error) {
	exp, _, err := expect.SpawnSSH(n3k.sshClt, 5*time.Second,
		expect.Tee(&writerProxy{w: buf}),
		expect.CheckDuration(50*time.Millisecond),
		expect.Verbose(true),
		expect.VerboseWriter(buf))
	return exp, err
}

// Checks if interface is configured with specified parameters.
func checkInterfaceConfigured(n3k *connectCtx, port, mode, status, speed string, timeout time.Duration) (string, error) {
	buf := &bytes.Buffer{}
	exp, err := spawnExp(n3k, buf)
	if err != nil {
		return "", errors.Wrapf(err, "while spawn goexpect")
	}

	defer exp.Close()

	_, _, err = exp.Expect(promptRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	buf.Reset()
	for i := 0; i < 5; i++ {
		err = interfaceConfigured(exp, port, mode, status, speed, timeout)
		if err == nil {
			break
		}
		time.Sleep(500 * time.Millisecond)
	}

	return buf.String(), err

}

func confIfCmd(exp expect.Expecter, cmd string, timeout time.Duration) error {
	if err := exp.Send(cmd); err != nil {
		return err
	}
	_, _, err := exp.Expect(configIfRegex, timeout)
	return err
}

func confVlanCfgCmd(exp expect.Expecter, cmd string, timeout time.Duration) error {
	if err := exp.Send(cmd); err != nil {
		return err
	}
	_, _, err := exp.Expect(configVlanCfgRegex, timeout)
	return err
}

// connects to switch, enter config mode, and program interface
func connect(ip, user, password string) (*connectCtx, error) {
	sshClt, err := spawnExpectSSH(ip, user, password, 10*time.Second)
	if err != nil {
		return nil, err
	}

	return &connectCtx{sshClt: sshClt}, nil
}

type nexus3k struct {
	username string
	password string
	ip       string
	ctx      *connectCtx
}

func newNexus3kSsh(ip, username, password string) Switch {
	n3kInst := &nexus3k{username: username, password: password, ip: ip}
	ctx, err := connect(ip, username, password)
	if err != nil {
		return nil
	}
	n3kInst.ctx = ctx
	return n3kInst
}

func (sw *nexus3k) runConfigIFCommands(port string, cmds []string) error {
	out, err := configInterface(sw.ctx, port, cmds, 30*time.Second)
	log.Println("-------------------output-------------------")
	log.Println(out)
	if err != nil {
		log.Println("-------------------ERROR-------------------")
	}

	return err
}

func (sw *nexus3k) runConfigCommands(cmds []string) error {
	out, err := configure(sw.ctx, cmds, 30*time.Second)
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

	out, err := configInterface(sw.ctx, port, cmds, 10*time.Second)
	log.Println("-------------------output-------------------")
	log.Println(out)
	if err != nil {
		log.Println("-------------------ERROR-------------------")
	}

	return err
}

func (sw *nexus3k) ConfigureVlans(vlans string, igmpEnabled bool) error {

	out, err := configVlan(sw.ctx, vlans, igmpEnabled, 10*time.Second)
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

	out, err := configInterface(sw.ctx, port, cmds, 10*time.Second)
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

	out, err = configInterface(sw.ctx, port, cmds, 10*time.Second)
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

	err = sw.runConfigCommands(cmds)
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
	buf, err := checkInterfaceConfigured(sw.ctx, port, mode.String(), status.String(),
		speedStr, 5*time.Second)

	return buf, err
}

func (sw *nexus3k) DoQosConfig(qosConfig *QosConfig) error {
	_, err := configureQos(sw.ctx, qosConfig, 5*time.Second)
	return err
}

func (sw *nexus3k) DoQueueConfig(queueConfig *QueueConfig) error {
	_, err := ConfigureQueue(sw.ctx, queueConfig, 5*time.Second)
	return err
}

func (sw *nexus3k) SetPortQos(port string, enable bool, params string) error {
	var cmds []string
	if enable {
		cmds = []string{fmt.Sprintf("service-policy type qos input %s", params)}
	} else {
		cmds = []string{fmt.Sprintf("no service-policy type qos input %s", params)}
	}
	log.Println(cmds)
	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) SetPortQueuing(port string, enable bool, params string) error {
	var cmds []string
	if enable {
		cmds = []string{fmt.Sprintf("service-policy type queuing output %s", params)}
	} else {
		cmds = []string{fmt.Sprintf("no service-policy type queuing output %s", params)}
	}
	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) SetPortPause(port string, enable bool) error {
	var cmds []string
	if enable {
		cmds = []string{"flowcontrol send on", "flowcontrol receive on"}

	} else {
		cmds = []string{"no flowcontrol send on", "no flowcontrol receive on"}
	}

	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) SetPortPfc(port string, enable bool) error {
	var cmds []string
	if enable {
		cmds = []string{"no flowcontrol send on", "no flowcontrol receive on", "priority-flow-control mode on"}
	} else {
		cmds = []string{"no priority-flow-control mode on"}
	}

	return sw.runConfigIFCommands(port, cmds)
}

func (sw *nexus3k) Disconnect() {
	sw.ctx.sshClt.Close()
}
