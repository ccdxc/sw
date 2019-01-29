package n3k

import (
	"bytes"
	"fmt"
	"io"
	"regexp"
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

// Write writes data to proxy
func (wp *writerProxy) Write(p []byte) (n int, err error) {
	return wp.w.Write(p)
}

// Close clodes proxy
func (wp *writerProxy) Close() error {
	return nil
}

func spawnExpectSSH(ip, user, password, port string, buf *bytes.Buffer, timeout time.Duration) (expect.Expecter, *ssh.Client, error) {
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
		return nil, nil, errors.Wrapf(err, "while ssh dial %s", ip)
	}

	exp, _, err := expect.SpawnSSH(sshClt, timeout,
		expect.Tee(&writerProxy{w: buf}),
		expect.CheckDuration(50*time.Millisecond),
		expect.Verbose(true),
		expect.VerboseWriter(buf))
	if err != nil {
		return nil, nil, errors.Wrapf(err, "while spawn goexpect for %s", ip)
	}

	return exp, sshClt, nil
}

// ConfigInterface connects to switch, enter config mode, and program interface
func ConfigInterface(ip, user, password, port string, commands []string, timeout time.Duration) (string, error) {
	buf := &bytes.Buffer{}
	exp, sshClt, err := spawnExpectSSH(ip, user, password, port, buf, timeout)

	if err != nil {
		return "", errors.New("Failed to spawn remote session")
	}

	defer sshClt.Close()
	defer exp.Close()

	_, _, err = exp.Expect(promptRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	if err := exp.Send("conf\n"); err != nil {
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
func Configure(ip, user, password, port string, commands []string, timeout time.Duration) (string, error) {
	buf := &bytes.Buffer{}
	exp, sshClt, err := spawnExpectSSH(ip, user, password, port, buf, timeout)

	if err != nil {
		return "", errors.New("Failed to spawn remote session")
	}

	defer sshClt.Close()
	defer exp.Close()

	_, _, err = exp.Expect(promptRegex, timeout)
	if err != nil {
		return buf.String(), err
	}

	defer func() {
		exp.Send("exit\n")
		time.Sleep(exitTimeout)
	}()

	if err := exp.Send("conf\n"); err != nil {
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

func confIfCmd(exp expect.Expecter, cmd string, timeout time.Duration) error {
	if err := exp.Send(cmd); err != nil {
		return err
	}
	_, _, err := exp.Expect(configIfRegex, timeout)
	return err
}
