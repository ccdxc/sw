package command

import (
	"bytes"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"syscall"
	"time"

	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	"github.com/pkg/errors"
)

var (
	execOutputDir = "/tmp"
	cmdCnt        = 0
)

func getOutFiles() (string, string) {
	cmdCnt += 1
	return execOutputDir + "/stdout_" + strconv.Itoa(cmdCnt), execOutputDir + "/stderr_" + strconv.Itoa(cmdCnt)
}

//ExecCmd run shell command
func execCmd(cmdArgs []string, runDir string, TimedOut int, background bool, shell bool,
	env []string) (*CommandInfo, error) {

	var process *exec.Cmd
	var stdoutBuf, stderrBuf bytes.Buffer
	var shellStdout, shellStderr string

	cmdInfo := &CommandInfo{Ctx: &CommandCtx{}}
	if shell {
		fullCmd := strings.Join(cmdArgs, " ")
		if background {
			shellStdout, shellStderr = getOutFiles()
			if strings.ContainsAny(fullCmd, ">") {
				fullCmd = fullCmd + " | tee" + shellStdout
			} else {
				fullCmd = fullCmd + " 2> " + shellStderr + " 1>" + shellStdout
			}
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
	if runDir != "" {
		process.Dir = runDir
	}
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

	cmdInfo.Ctx.status = make(chan error)

	if background {
		fmt.Println("Starting background command :", strings.Join(cmdArgs, " "))
		//For now don't send signal to background process.
		//process.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
		if err := process.Start(); err != nil {
			cmdInfo.Ctx.ExitCode = 1
			cmdInfo.Ctx.Done = true
			cmdInfo.Ctx.Stdout = errors.Wrapf(err, "Background process start failed!").Error()
			cmdInfo.Ctx.status <- nil
			return cmdInfo, err
		}
		cmdInfo.Handle = process
		go func(cmdInfo *CommandInfo) {
			process.Wait()
			cmdInfo.Ctx.Done = true
			cmdInfo.Ctx.ExitCode = 1
			if shell {
				//Sleep for couple of second for file dump complete.
				time.Sleep(2 * time.Second)
				if b, err := ioutil.ReadFile(shellStdout); err == nil {
					cmdInfo.Ctx.Stdout = string(b)
				}
				if b, err := ioutil.ReadFile(shellStderr); err == nil {
					cmdInfo.Ctx.Stderr = string(b)
				}
			} else {
				cmdInfo.Ctx.Stdout = stdoutBuf.String()
				cmdInfo.Ctx.Stderr = stderrBuf.String()
			}
			cmdInfo.Ctx.status <- nil
		}(cmdInfo)
		return cmdInfo, nil
	}

	go func(cmdInfo *CommandInfo) {
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
		cmdInfo.Ctx.status <- nil
		return
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
		case err := <-cmdInfo.Ctx.status:
			return cmdInfo, err
		}
	}

	err := <-cmdInfo.Ctx.status
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
			for _, pid := range pids {
				if pid != 0 {
					killCmd := []string{"sudo", "kill", "-SIGKILL", strconv.Itoa(pid)}
					Utils.Run(killCmd, 0, false, true, nil)
				}
			}
		}
		cmdInfo.Handle.(*exec.Cmd).Process.Signal(syscall.SIGKILL)

		//For bg command, don't return success until command terminates
		cmdInfo.Ctx.ExitCode = 255

		//Try multiple times to kill the same process
		for i := 0; i < 3; i++ {
			//Wait for 5 seconds before forcefully killing
			timeoutEvent := time.After(time.Duration(5) * time.Second)
			select {
			case <-timeoutEvent:
				killCmd := []string{"sudo", "kill", "-SIGKILL", strconv.Itoa(cmdInfo.Handle.(*exec.Cmd).Process.Pid)}
				Utils.Run(killCmd, 0, false, true, nil)
			case <-cmdInfo.Ctx.status:
				//Command successfully terminated.
				cmdInfo.Ctx.ExitCode = 0
				break
			}
		}

	}

	if cmdInfo.Ctx.ExitCode != 0 {
		cmdInfo.Ctx.Stderr = "Command failed to terminate."
	}
	cmdInfo.Ctx.Done = true
	cmdInfo.Handle = nil
	return nil
}

func SetOutputDirectory(outDirectory string) {
	execOutputDir = outDirectory
}
