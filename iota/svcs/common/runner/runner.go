package runner

import (
	"bytes"
	"fmt"
	"io"

	"golang.org/x/crypto/ssh"

	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/venice/utils/log"
)

// Runner implements all remote to command execution functions
type Runner struct {
	SSHClientConfig *ssh.ClientConfig
}

// NewRunner returns a new copier instance
func NewRunner(c *ssh.ClientConfig) *Runner {
	runner := &Runner{
		SSHClientConfig: c,
	}
	return runner
}

// Run runs a command either in foreground on on background
func (r *Runner) Run(ipPort, command string, cmdMode int) error {
	client, err := ssh.Dial("tcp", ipPort, r.SSHClientConfig)
	if client == nil || err != nil {
		log.Errorf("Runner | Run on node %v failed, Err: %v", ipPort, err)
		return err
	}
	defer client.Close()

	session, err := client.NewSession()
	if session == nil || err != nil {
		log.Errorf("Runner | Run on node %v failed, Err: %v", ipPort, err)
		return err
	}
	defer session.Close()

	ptyModes := ssh.TerminalModes{
		ssh.TTY_OP_ISPEED: 14400,
		ssh.TTY_OP_OSPEED: 14400,
		ssh.ECHO:          0,
	}

	if err := session.RequestPty("xterm", 80, 40, ptyModes); err != nil {
		log.Errorf("Runner | Run on node %v failed to get a pseudo TTY, Err: %v", ipPort, err)
		return err
	}

	stdoutBuffer := bytes.Buffer{}
	stdoutWriter := io.Writer(&stdoutBuffer)

	//Pipe stdout and stderr
	stdout, err := session.StdoutPipe()
	if err != nil {
		log.Errorf("Runner | Run on node %v failed to get a stdout, Err: %v", ipPort, err)
		return fmt.Errorf("could not capture stdout. %v", err)
	}
	go io.Copy(stdoutWriter, stdout)

	if cmdMode == constants.RunCommandBackground {
		log.Infof("Runner | Running command %v in background...", command)
		command = "nohup sh -c  \"" + command + " 2>&1 >/dev/null </dev/null & \""
	} else {
		command = "sh -c \"" + command + "\""
	}

	if err := session.Run(command); err != nil {
		log.Errorf("Runner | Command %v failed on node %v Err: %v", command, ipPort, err)
		return err
	}
	return nil
}
