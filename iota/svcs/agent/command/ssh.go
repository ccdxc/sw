package command

import (
	"bytes"
	"io"
	"time"

	"github.com/pkg/errors"
	log "github.com/sirupsen/logrus"

	"golang.org/x/crypto/ssh"
)

const (
	SSH_CREATION_FAILED_EXIT_CODE = 127
)

//SudoCmd sudo cmd constructor
var SudoCmd = func(cmd string) string {
	return "sudo " + cmd
}

//CreateSSHSession create session so that caller will run on his own
func createSSHSession(SSHHandle *ssh.Client) (*ssh.Session, io.Reader, io.Reader, error) {
	sshSession, err := SSHHandle.NewSession()
	if err != nil {
		return nil, nil, nil, err
	}

	modes := ssh.TerminalModes{
		ssh.ECHO:          0,     // disable echoing
		ssh.TTY_OP_ISPEED: 14400, // input speed = 14.4kbaud
		ssh.TTY_OP_OSPEED: 14400, // output speed = 14.4kbaud
	}

	if err = sshSession.RequestPty("xterm", 80, 40, modes); err != nil {
		return nil, nil, nil, err
	}

	sshOut, err := sshSession.StdoutPipe()
	if err != nil {
		return nil, nil, nil, err
	}
	sshErr, err := sshSession.StderrPipe()
	if err != nil {
		return nil, nil, nil, err
	}

	return sshSession, sshOut, sshErr, nil
}

//RunSSHCommand run command over SSH
func RunSSHCommand(SSHHandle *ssh.Client, cmd string, TimedOut uint32, sudo bool, bg bool, logger *log.Logger) (*CommandInfo, error) {
	logger.Println("Running cmd " + cmd)
	var stdoutBuf, stderrBuf bytes.Buffer

	cmdInfo := &CommandInfo{Ctx: &CommandCtx{}}
	sshSession, sshOut, sshErr, err := createSSHSession(SSHHandle)
	defer sshSession.Close()
	if err != nil {
		logger.Println("SSH session creation failed!")
		cmdInfo.Ctx.ExitCode = 127
		return cmdInfo, errors.New("Ssh creation failed")
	}

	shout := io.MultiWriter(&stdoutBuf)
	ssherr := io.MultiWriter(&stderrBuf)

	ioCopies := make(chan int)
	go func() {
		io.Copy(shout, sshOut)
		ioCopies <- 1
	}()
	go func() {
		io.Copy(ssherr, sshErr)
		ioCopies <- 2
	}()

	if bg {
		cmd = "nohup sh -c  \"" + cmd + " 2>&1 >/dev/null </dev/null & \""
	} else {
		cmd = "sh -c \"" + cmd + "\""
	}

	if sudo {
		cmd = SudoCmd(cmd)
	}

	logger.Println("Running command : " + cmd)

	runCmd := func() {
		if err = sshSession.Run(cmd); err != nil {
			logger.Println("failed command : " + cmd)
			switch v := err.(type) {
			case *ssh.ExitError:
				cmdInfo.Ctx.ExitCode = (int32)(v.Waitmsg.ExitStatus())
			default:
				cmdInfo.Ctx.ExitCode = -1
			}
		} else {
			logger.Println("sucess command : " + cmd)
		}
		for i := 0; i < 2; i++ {
			<-ioCopies
		}
		cmdInfo.Ctx.Stdout = stdoutBuf.String()
		cmdInfo.Ctx.Stderr = stderrBuf.String()
		cmdInfo.Ctx.Done = true
	}

	if TimedOut != 0 {
		cTimedOut := time.After(time.Second * time.Duration(TimedOut))
		go runCmd()
		for true {
			select {
			case <-cTimedOut:
				sshSession.Close()
				time.Sleep(1 * time.Second)
				cmdInfo.Ctx.Stdout = stdoutBuf.String()
				cmdInfo.Ctx.Stderr = stderrBuf.String()
				cmdInfo.Ctx.TimedOut = true
				return cmdInfo, nil
			default:
				if cmdInfo.Ctx.Done {
					return cmdInfo, nil
				}
				time.Sleep(100 * time.Millisecond)
			}
		}

	} else {
		runCmd()
	}

	return cmdInfo, nil
}

//StartSSHBgCommand start bg ssh Command
func StartSSHBgCommand(SSHHandle *ssh.Client, cmd string, sudo bool) (*CommandInfo, error) {
	var stdoutBuf, stderrBuf bytes.Buffer
	cmdInfo := &CommandInfo{Ctx: &CommandCtx{}}

	sshSession, sshOut, sshErr, err := createSSHSession(SSHHandle)
	if err != nil {
		return nil, errors.Wrap(err, "Error in sss session creation")
	}
	shout := io.MultiWriter(&stdoutBuf)
	ssherr := io.MultiWriter(&stderrBuf)

	go func(ctx *CommandCtx) {
		go func() {
			io.Copy(shout, sshOut)
		}()
		go func() {

			io.Copy(ssherr, sshErr)
		}()

		fullCmd := "sh -c \"" + cmd + "\""

		if sudo {
			fullCmd = SudoCmd(fullCmd)
		}
		err := sshSession.Start(fullCmd)
		if err == nil {
			err := sshSession.Wait()
			defer sshSession.Close()
			switch v := err.(type) {
			case *ssh.ExitError:
				ctx.ExitCode = (int32)(v.Waitmsg.ExitStatus())
			default:
				ctx.ExitCode = -1
			}
			ctx.Stderr = stderrBuf.String()
			ctx.Stdout = stdoutBuf.String()
		} else {
			ctx.ExitCode = 1
			ctx.Stderr = "Failed to start cmd : " + cmd
		}
		ctx.Done = true
	}(cmdInfo.Ctx)

	cmdInfo.Handle = sshSession
	/* Give it couple of seconds to make sure command has started */
	time.Sleep(2 * time.Second)
	return cmdInfo, nil
}

//StopSSHCmd Stop bg process running
func StopSSHCmd(cmdInfo *CommandInfo) error {
	session := cmdInfo.Handle.(*ssh.Session)
	if session != nil {
		session.Close()
		time.Sleep(2 * time.Second)
	}
	cmdInfo.Handle = nil

	return nil
}
