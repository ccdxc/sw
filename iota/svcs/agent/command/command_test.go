package command

import (
	"io"
	"os"
	"testing"
	"time"

	log "github.com/sirupsen/logrus"
	"golang.org/x/crypto/ssh"

	Utils "github.com/pensando/sw/iota/svcs/agent/utils"

	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func TestMain(m *testing.M) {
	startCmd := []string{"docker", "run", "--rm", "-d", "--publish=22:22", "--name", "test_copy_node", "sickp/alpine-sshd:7.5-r2"}
	if retCode, stdout, err := Utils.RunCmd(startCmd, 0, false, false, nil); err != nil || retCode != 0 {
		log.Fatalf("Could not start docker container %v %v ", stdout, err)
	}
	stopCmd := []string{"docker", "stop", "test_copy_node"}
	defer Utils.RunCmd(stopCmd, 0, false, false, nil)
	defer time.Sleep(3 * time.Second)

	time.Sleep(3 * time.Second)

	runTests := m.Run()

	os.Exit(runTests)
}

func Test_SSH_Foreground(t *testing.T) {

	sshConfig := &ssh.ClientConfig{
		User: "root",
		Auth: []ssh.AuthMethod{
			ssh.Password("root"),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	SSHHandle, err := ssh.Dial("tcp", "127.0.0.1:22", sshConfig)
	TestUtils.Assert(t, SSHHandle != nil && err == nil, "Connection failed!")
	defer SSHHandle.Close()

	logger := log.New()
	logger.Out = io.MultiWriter(os.Stdout)

	cmdResp, err := RunSSHCommand(SSHHandle, "date", 0, false, false, logger)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, cmdResp.Ctx.Done, "Command completed!")
	TestUtils.Assert(t, cmdResp.Ctx.Stdout != "", "Command stdout is set!")
	TestUtils.Assert(t, !cmdResp.Ctx.TimedOut, "Command TimedOut!")

	cmdResp, err = RunSSHCommand(SSHHandle, "dumb", 0, false, false, logger)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, cmdResp.Ctx.Done, "Command completed!")
	TestUtils.Assert(t, cmdResp.Ctx.ExitCode != 0, "Command failed!")
	TestUtils.Assert(t, cmdResp.Ctx.Stdout != "", "Command stdout is set!")
	TestUtils.Assert(t, !cmdResp.Ctx.TimedOut, "Command TimedOut!")
}

func Test_SSH_Background(t *testing.T) {

	sshConfig := &ssh.ClientConfig{
		User: "root",
		Auth: []ssh.AuthMethod{
			ssh.Password("root"),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	SSHHandle, err := ssh.Dial("tcp", "127.0.0.1:22", sshConfig)
	TestUtils.Assert(t, SSHHandle != nil && err == nil, "Connection failed!")
	defer SSHHandle.Close()

	logger := log.New()
	logger.Out = io.MultiWriter(os.Stdout)

	cmdResp, err := StartSSHBgCommand(SSHHandle, "sleep 300", false)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, !cmdResp.Ctx.Done, "Command not completed!")
	TestUtils.Assert(t, cmdResp.Handle != nil, "Command Handle set ")
	TestUtils.Assert(t, !cmdResp.Ctx.TimedOut, "Command TimedOut!")

	err = StopSSHCmd(cmdResp)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, cmdResp.Ctx.Done, "Command completed!")
	TestUtils.Assert(t, cmdResp.Handle == nil, "Command Handle not set ")
	TestUtils.Assert(t, !cmdResp.Ctx.TimedOut, "Command TimedOut!")

	cmdResp, err = StartSSHBgCommand(SSHHandle, "ping abc.com", false)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, !cmdResp.Ctx.Done, "Command not completed!")
	TestUtils.Assert(t, cmdResp.Handle != nil, "Command Handle set ")
	TestUtils.Assert(t, !cmdResp.Ctx.TimedOut, "Command TimedOut!")

	time.Sleep(2 * time.Second)
	err = StopSSHCmd(cmdResp)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, cmdResp.Ctx.Done, "Command completed!")
	TestUtils.Assert(t, cmdResp.Handle == nil, "Command Handle not set ")
	TestUtils.Assert(t, cmdResp.Ctx.Stdout != "", "Command stdout is set!")
	TestUtils.Assert(t, !cmdResp.Ctx.TimedOut, "Command TimedOut!")
}

func Test_SSH_TimedOut(t *testing.T) {
	sshConfig := &ssh.ClientConfig{
		User: "root",
		Auth: []ssh.AuthMethod{
			ssh.Password("root"),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	SSHHandle, err := ssh.Dial("tcp", "127.0.0.1:22", sshConfig)
	TestUtils.Assert(t, SSHHandle != nil && err == nil, "Connection failed!")
	defer SSHHandle.Close()

	logger := log.New()
	logger.Out = io.MultiWriter(os.Stdout)

	cmdResp, err := RunSSHCommand(SSHHandle, "ping abc.com", 5, false, false, logger)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, cmdResp.Ctx.Done, "Command completed!")
	TestUtils.Assert(t, cmdResp.Ctx.TimedOut, "Command TimedOut!")
	TestUtils.Assert(t, cmdResp.Ctx.Stdout != "", "Command stdout is set!")

}

func Test_Cmd_Foreground(t *testing.T) {
	logger := log.New()
	logger.Out = io.MultiWriter(os.Stdout)

	cmdResp, err := ExecCmd([]string{"date"}, "", 0, false, false, nil)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, cmdResp.Ctx.Done, "Command completed!")
	//TestUtils.Assert(t, cmdResp.Ctx.Stdout != "", "Command stdout is set!")
	TestUtils.Assert(t, !cmdResp.Ctx.TimedOut, "Command TimedOut!")

	cmdResp, err = ExecCmd([]string{"ping", "-c 1", "asda.com"}, "", 0, false, false, nil)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, cmdResp.Ctx.Done, "Command completed!")
	TestUtils.Assert(t, cmdResp.Ctx.ExitCode != 0, "Command failed!")
	//TestUtils.Assert(t, cmdResp.Ctx.Stdout != "", "Command stdout is set!")
	TestUtils.Assert(t, !cmdResp.Ctx.TimedOut, "Command TimedOut!")
}

func Test_Cmd_Background(t *testing.T) {
	logger := log.New()
	logger.Out = io.MultiWriter(os.Stdout)

	cmdResp, err := ExecCmd([]string{"ping", "abc.com"}, "", 0, true, false, nil)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, !cmdResp.Ctx.Done, "Command completed!")
	TestUtils.Assert(t, cmdResp.Handle != nil, "Command Handle set ")
	TestUtils.Assert(t, !cmdResp.Ctx.TimedOut, "Command TimedOut!")

	time.Sleep(2 * time.Second)
	err = StopExecCmd(cmdResp)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, cmdResp.Ctx.Done, "Command completed!")
	TestUtils.Assert(t, cmdResp.Handle == nil, "Command Handle not set ")
	//TestUtils.Assert(t, cmdResp.Ctx.Stdout != "", "Command stdout is set!")
	TestUtils.Assert(t, !cmdResp.Ctx.TimedOut, "Command TimedOut!")

}

func Test_Cmd_TimedOut(t *testing.T) {
	logger := log.New()
	logger.Out = io.MultiWriter(os.Stdout)

	cmdResp, err := ExecCmd([]string{"ping", "abc.com"}, "", 5, false, false, nil)
	TestUtils.Assert(t, err == nil, "Command succeded!")
	TestUtils.Assert(t, cmdResp.Ctx.Done, "Command completed!")
	TestUtils.Assert(t, cmdResp.Ctx.TimedOut, "Command time out!")
	TestUtils.Assert(t, cmdResp.Handle == nil, "Command Handle set ")
	TestUtils.Assert(t, cmdResp.Ctx.Stdout != "", "Command stdout is set!")

}
