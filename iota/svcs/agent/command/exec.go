package command

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"syscall"
	"time"

	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	"github.com/pkg/errors"
)

//ExecCmd run shell command
func execCmd(cmdArgs []string, TimedOut int, background bool, shell bool,
	env []string) (*CommandInfo, error) {

	var process *exec.Cmd
	var stdoutBuf, stderrBuf bytes.Buffer

	cmdInfo := &CommandInfo{Ctx: &CommandCtx{}}
	if shell {
		fullCmd := strings.Join(cmdArgs, " ")
		if background {
			newCmdArgs := []string{"sh", "-c", fullCmd}
			process = exec.Command(newCmdArgs[0], newCmdArgs[1:]...)

		} else {
			newCmdArgs := []string{"nohup", "sh", "-c", fullCmd}
			process = exec.Command(newCmdArgs[0], newCmdArgs[1:]...)
		}
	} else {
		process = exec.Command(cmdArgs[0], cmdArgs[1:]...)
	}
	process.Env = os.Environ()
	for _, env := range env {
		process.Env = append(process.Env, env)
	}

	sshOut, _ := process.StdoutPipe()
	sshErr, _ := process.StderrPipe()

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
        

	done := make(chan error)

	go func(cmdInfo *CommandInfo) {
		if background {
			fmt.Println("Starting background command :", strings.Join(cmdArgs, " "))
			//For now don't send signal to background process.
			//process.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
			if err := process.Start(); err != nil {
				cmdInfo.Ctx.ExitCode = 1
				cmdInfo.Ctx.Done = true
				cmdInfo.Ctx.Stdout = errors.Wrapf(err, "Background process start failed!").Error()
				done <- nil
			}
			cmdInfo.Handle = process
			go func(cmdInfo *CommandInfo) {
				process.Wait()
				cmdInfo.Ctx.Done = true
				cmdInfo.Ctx.ExitCode = 1
				cmdInfo.Ctx.Stdout = stdoutBuf.String()
			}(cmdInfo)
			done <- nil
		} else {
			process.Start()
			cmdErr := process.Wait()
			if cmdErr != nil {
				if exitError, ok := cmdErr.(*exec.ExitError); ok {
					ws := exitError.Sys().(syscall.WaitStatus)
					cmdInfo.Ctx.ExitCode = (int32)(ws.ExitStatus())
				} else {
					cmdInfo.Ctx.ExitCode = 1
				}
			}
                        for i := 0; i < 2; i++ {
                            <-ioCopies
                        }
			cmdInfo.Ctx.Done = true
			cmdInfo.Ctx.Stdout = stdoutBuf.String()
			cmdInfo.Ctx.Stderr = stderrBuf.String()
			done <- nil
			return
		}
	}(cmdInfo)

	if TimedOut != 0 {
		TimedOutEvent := time.After(time.Duration(TimedOut) * time.Second)

		select {
		case <-TimedOutEvent:
			// TimedOut happened first, kill the process and print a message.
			process.Process.Signal(os.Kill)
			cmdInfo.Ctx.ExitCode = 1
			cmdInfo.Ctx.Stdout = stdoutBuf.String()
			cmdInfo.Ctx.Stderr = stderrBuf.String()
			cmdInfo.Ctx.TimedOut = true
			cmdInfo.Ctx.Done = true
			return cmdInfo, nil
		case err := <-done:
			return cmdInfo, err
		}
	}

	err := <-done
	return cmdInfo, err
}

//ExecCmd Run shell command
var ExecCmd = execCmd

func getChildPids(ppid int) []int {
	ret := []int{}
	cmd := []string{"pstree", "-p", strconv.Itoa(ppid), "|", "perl", "-ne", "'print \"$1\\n\" while /\\((\\d+)\\)/g'"}
	exitCode, stdoutStderr, err := Utils.Run(cmd, 0, false, true, nil)
	if err == nil && exitCode == 0 {
		pids := strings.Split(stdoutStderr, "\n")
		for _, pid := range pids {
			if ipid, err := strconv.Atoi(pid); err == nil {
				ret = append(ret, ipid)
			}
		}
	}

	return ret
}

//StopExecCmd Stop bg process running
func StopExecCmd(cmdInfo *CommandInfo) error {
	process := cmdInfo.Handle.(*exec.Cmd)
	if process != nil {
		pids := getChildPids(cmdInfo.Handle.(*exec.Cmd).Process.Pid)
		if len(pids) != 0 {
			pids = append(pids, cmdInfo.Handle.(*exec.Cmd).Process.Pid)
			for _, pid := range pids {
				if pid != 0 {
					killCmd := []string{"kill", "-9", strconv.Itoa(pid)}
					Utils.Run(killCmd, 0, false, true, nil)
				}
			}
		} else {
			cmdInfo.Handle.(*exec.Cmd).Process.Signal(syscall.SIGKILL)
		}

		time.Sleep(2 * time.Second)
	}

	cmdInfo.Ctx.Done = true
	cmdInfo.Handle = nil
	return nil
}
