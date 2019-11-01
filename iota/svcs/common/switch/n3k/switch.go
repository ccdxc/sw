package n3k

import (
	"bytes"
	"fmt"
	"io"
	"regexp"
	"strings"
	"time"

	expect "github.com/pensando/goexpect"
	"github.com/pkg/errors"
	"golang.org/x/crypto/ssh"
)

var (
	exitTimeout     = 500 * time.Millisecond
	promptRegex     = regexp.MustCompile("# ")
	configRegex     = regexp.MustCompile(`\(config\)\# `)
	configIfRegex   = regexp.MustCompile(`\(config-if\)\# `)
	configVlanRegex = regexp.MustCompile(`\(config-vlan\)\# `)
	configVlanCfgRegex = regexp.MustCompile(`\(config-vlan-config\)\# `)
	configPmapQos   = regexp.MustCompile(`\(config-pmap-nqos\)\# `)
	configClassQos  = regexp.MustCompile(`\(config-pmap-nqos-c\)\# `)
	configSystemQos = regexp.MustCompile(`\(config-sys-qos\)\# `)
)

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

type writerProxy struct {
	w io.Writer
}

//ConnectCtx connection context
type ConnectCtx struct {
	sshClt *ssh.Client
}

// Write writes data to proxy
func (wp *writerProxy) Write(p []byte) (n int, err error) {
	return wp.w.Write(p)
}

// Close clodes proxy
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

// ConfigInterface connects to switch, enter config mode, and program interface
func ConfigInterface(n3k *ConnectCtx, port string, commands []string, timeout time.Duration) (string, error) {
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

//ConfigureQos configure qos on switch
func ConfigureQos(n3k *ConnectCtx, qosCfg *QosConfig, timeout time.Duration) (string, error) {

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

	nwQos := fmt.Sprintf("policy-map type network-qos %v\n", qosCfg.Name)
	//Set spanning tree mode to mst to support more vlans
	if err = exp.Send(nwQos); err != nil {
		return buf.String(), err
	}
	_, _, err = exp.Expect(configPmapQos, timeout)
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
			_, _, err = exp.Expect(configClassQos, timeout)
			if err != nil {
				return buf.String(), err
			}

			if qosClass.Mtu != 0 {
				mtuCmd := fmt.Sprintf("mtu %v\n", qosClass.Mtu)
				if err = exp.Send(mtuCmd); err != nil {
					return buf.String(), err
				}
			}

			pfcCmd := fmt.Sprintf("pause pfc-cos %v\n", qosClass.PfsCos)
			if err = exp.Send(pfcCmd); err != nil {
				return buf.String(), err
			}

			exp.Send("exit\n") //exit configClassQos
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

// ConfigVlan configures vlan on the interface
func ConfigVlan(n3k *ConnectCtx, vlanRange string, igmpEnabled bool, timeout time.Duration) (string, error) {
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

// Configure connects to switch, enter config mode,
func Configure(n3k *ConnectCtx, port string, commands []string, timeout time.Duration) (string, error) {
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

func interfaceConfigured(exp expect.Expecter, buf *bytes.Buffer, port, mode, status, speed string, timeout time.Duration) error {
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

func spawnExp(n3k *ConnectCtx, buf *bytes.Buffer) (expect.Expecter, error) {
	exp, _, err := expect.SpawnSSH(n3k.sshClt, 5*time.Second,
		expect.Tee(&writerProxy{w: buf}),
		expect.CheckDuration(50*time.Millisecond),
		expect.Verbose(true),
		expect.VerboseWriter(buf))
	return exp, err
}

// CheckInterfaceConigured Checks if interface is configured with specified parameters.
func CheckInterfaceConigured(n3k *ConnectCtx, port, mode, status, speed string, timeout time.Duration) (string, error) {
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

	//try twice
	for i := 0; i < 5; i++ {
		err = interfaceConfigured(exp, buf, port, mode, status, speed, timeout)
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

func confVlanCmd(exp expect.Expecter, cmd string, timeout time.Duration) error {
	if err := exp.Send(cmd); err != nil {
		return err
	}
	_, _, err := exp.Expect(configVlanRegex, timeout)
	return err
}

func confVlanCfgCmd(exp expect.Expecter, cmd string, timeout time.Duration) error {
	if err := exp.Send(cmd); err != nil {
		return err
	}
	_, _, err := exp.Expect(configVlanCfgRegex, timeout)
	return err
}

// Connect connects to switch, enter config mode, and program interface
func Connect(ip, user, password string) (*ConnectCtx, error) {
	sshClt, err := spawnExpectSSH(ip, user, password, 10*time.Second)
	if err != nil {
		return nil, err
	}

	return &ConnectCtx{sshClt: sshClt}, nil
}

func Disconnect(n3k *ConnectCtx) {
	n3k.sshClt.Close()
}
