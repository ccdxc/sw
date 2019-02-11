package n3k

import (
	"bytes"
	"fmt"
	"io"
	"regexp"
	"strings"
	"time"

	"github.com/pensando/goexpect"
	"github.com/pkg/errors"
	"golang.org/x/crypto/ssh"
)

var (
	exitTimeout   = 500 * time.Millisecond
	promptRegex   = regexp.MustCompile("# ")
	configRegex   = regexp.MustCompile(`\(config\)\# `)
	configIfRegex = regexp.MustCompile(`\(config-if\)\# `)
)

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

// Configure connects to switch, enter config mode,
func Configure(n3k *ConnectCtx, port string, commands []string, timeout time.Duration) (string, error) {
	buf := &bytes.Buffer{}
	exp, err := spawnExp(n3k, buf)
	if err != nil {
		return "", errors.Wrapf(err, "while spawn goexpect")
	}

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
	sendStr := fmt.Sprintf("show interface %s brief | grep %s", port, status)

	port = strings.Replace(port, "e", "Eth", -1)
	matchInterfaceRegex := regexp.MustCompile(fmt.Sprintf("(.*)%s(.*)%s(.*)%s(.*)%s(.*)", port, mode, status, speed))

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

	_, _, err = exp.Expect(promptRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	err = interfaceConfigured(exp, buf, port, mode, status, speed, timeout)

	return buf.String(), err

}
func confIfCmd(exp expect.Expecter, cmd string, timeout time.Duration) error {
	if err := exp.Send(cmd); err != nil {
		return err
	}
	_, _, err := exp.Expect(configIfRegex, timeout)
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

func disconnect(n3k *ConnectCtx) {
	n3k.sshClt.Close()
}
