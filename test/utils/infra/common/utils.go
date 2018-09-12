package common

import (
	"bytes"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"syscall"
	"time"

	"github.com/pkg/errors"
	"golang.org/x/crypto/ssh"
)

const (
	maxIPRetries = 3
)

//CmdHandle Running command handle to stop bg process later
type CmdHandle *exec.Cmd

//RunCmd run shell command
func RunCmd(cmdArgs []string, timeout int, background bool, shell bool,
	env []string) (int, string, error) {

	var process *exec.Cmd
	if shell {
		fullCmd := strings.Join(cmdArgs, " ")
		newCmdArgs := []string{"nohup", "sh", "-c", fullCmd}
		process = exec.Command(newCmdArgs[0], newCmdArgs[1:]...)
	} else {
		process = exec.Command(cmdArgs[0], cmdArgs[1:]...)
	}
	process.Env = os.Environ()
	for _, env := range env {
		process.Env = append(process.Env, env)
	}
	if background {
		fmt.Println("Starting background command :", strings.Join(cmdArgs, " "))
		//For now don't send signal to background process.
		process.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
		if err := process.Start(); err != nil {
			return 1, "", errors.Wrapf(err, "Background process start failed!")
		}
		return 0, "", nil
	}
	exitCode := 0
	stdoutStderr, err := process.CombinedOutput()
	if err != nil {
		if exitError, ok := err.(*exec.ExitError); ok {
			ws := exitError.Sys().(syscall.WaitStatus)
			exitCode = ws.ExitStatus()
		} else {
			exitCode = 1
		}
		return exitCode, string(stdoutStderr), errors.Wrap(err, string(stdoutStderr))
	}
	return exitCode, string(stdoutStderr), nil
}

//Run Run shell command
var Run = RunCmd

//Stop Stop bg process running
func Stop(cmdHandle CmdHandle) {
	cmdHandle.Process.Kill()
}

//SetUpIPAddress for interface
func SetUpIPAddress(intfName string, ip string) error {
	for i := 0; i < maxIPRetries; i++ {
		cmd := []string{"ip", "addr", "flush", "dev", intfName}
		if _, stdout, err := Run(cmd, 0, false, true, nil); err != nil {
			return errors.Wrap(err, stdout)
		}
		cmd = []string{"ip", "addr", "add", ip, "dev", intfName}
		if _, stdout, err := Run(cmd, 0, false, true, nil); err != nil {
			return errors.Wrap(err, stdout)
		}
		intfs, _ := net.Interfaces()
		for _, intf := range intfs {
			if intf.Name == intfName {
				addrs, _ := intf.Addrs()
				for _, addr := range addrs {
					if addr.String() == ip {
						return nil
					}
				}
			}
		}
	}
	return errors.Errorf("IP address not set to interface : %s", intfName)
}

//DisableDhcpOnInterface disable DHCP on interface
func DisableDhcpOnInterface(intfName string) error {

	config := `TYPE=Ethernet
PROXY_METHOD=none
BROWSER_ONLY=no
BOOTPROTO=static
DEFROUTE=yes
IPV4_FAILURE_FATAL=no
IPV6INIT=yes
IPV6_AUTOCONF=yes
IPV6_DEFROUTE=yes
IPV6_FAILURE_FATAL=no
NM_CONTROLLED=no
IPV6_ADDR_GEN_MODE=stable-privacy
ONBOOT=no` + fmt.Sprintf("\nNAME=%s\nDEVICE=%s\n", intfName, intfName)

	fname := "/etc/sysconfig/network-scripts/ifcfg-" + intfName
	if err := ioutil.WriteFile(fname, []byte(config), 0644); err != nil {
		return err
	}

	time.Sleep(1 * time.Second)
	cmd := []string{"service", "network", "restart"}
	if _, stdout, err := Run(cmd, 0, false, true, nil); err != nil {
		return errors.Wrap(err, stdout)
	}
	cmd = []string{"ifconfig", intfName, "up"}
	if _, stdout, err := Run(cmd, 0, false, true, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	return nil
}

//GetIPAddressOfInterface Get IP address of the interface
func GetIPAddressOfInterface(intfName string) (string, error) {
	intfs, _ := net.Interfaces()
	for _, intf := range intfs {
		if intf.Name == intfName {
			addrs, _ := intf.Addrs()
			for _, addr := range addrs {
				// check the address type and if it is not a loopback the display it
				if ipnet, ok := addr.(*net.IPNet); ok && !ipnet.IP.IsLoopback() {
					if ipnet.IP.To4() != nil {
						return ipnet.IP.String(), nil
					}
				}
			}
		}
	}
	return "", errors.New("Ip address not found")
}

//IncrementIP address
func IncrementIP(origIP string, cidr string, incr byte) (string, error) {
	ip := net.ParseIP(origIP)
	_, ipNet, err := net.ParseCIDR(cidr)
	if err != nil {
		return origIP, err
	}
	for i := len(ip) - 1; i >= 0; i-- {
		ip[i] += incr
		if ip[i] != 0 {
			break
		}
	}
	if !ipNet.Contains(ip) {
		log.Fatalln("CIDR overflow")
	}
	return ip.String(), nil
}

//LogWriter Helper to write to stdout and file.
type LogWriter log.Logger

func (w *LogWriter) Write(b []byte) (int, error) {
	(*log.Logger)(w).Print(string(b))
	return len(b), nil
}

//SudoCmd sudo cmd constructor
var SudoCmd = func(cmd string) string {
	return "sudo " + cmd
}

//RunSSHCommand run command over SSH
func RunSSHCommand(SSHHandle *ssh.Client, cmd string, sudo bool, bg bool, logger *log.Logger) (retCode int, stdout, stderr []string) {
	logger.Println("Running cmd " + cmd)
	var stdoutBuf, stderrBuf bytes.Buffer
	sshSession, err := SSHHandle.NewSession()
	if err != nil {
		logger.Println("SSH session creation failed!")
		return -1, nil, nil
	}

	modes := ssh.TerminalModes{
		ssh.ECHO:          0,     // disable echoing
		ssh.TTY_OP_ISPEED: 14400, // input speed = 14.4kbaud
		ssh.TTY_OP_OSPEED: 14400, // output speed = 14.4kbaud
	}

	if err = sshSession.RequestPty("xterm", 80, 40, modes); err != nil {
		logger.Println("SSH session Pty creation failed!")
		return -1, nil, nil
	}

	sshOut, err := sshSession.StdoutPipe()
	if err != nil {
		logger.Println("SSH session StdoutPipe creation failed!")
		return -1, nil, nil
	}
	sshErr, err := sshSession.StderrPipe()
	if err != nil {
		logger.Println("SSH session StderrPipe creation failed!")
		return -1, nil, nil
	}

	shout := io.MultiWriter(&stdoutBuf, (*LogWriter)(logger))
	ssherr := io.MultiWriter(&stderrBuf, (*LogWriter)(logger))

	go func() {
		io.Copy(shout, sshOut)
	}()
	go func() {

		io.Copy(ssherr, sshErr)
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
	if err = sshSession.Run(cmd); err != nil {
		logger.Println("failed command : " + cmd)
		switch v := err.(type) {
		case *ssh.ExitError:
			retCode = v.Waitmsg.ExitStatus()
		default:
			retCode = -1
		}
	} else {
		logger.Println("sucess command : " + cmd)
		retCode = 0
	}

	stdout = strings.Split(stdoutBuf.String(), "\n")
	stderr = strings.Split(stderrBuf.String(), "\n")
	logger.Println(stdout)
	logger.Println(stderr)
	logger.Println("Return code : " + strconv.Itoa(retCode))

	return retCode, stdout, stderr

}

//GetIntfMatchingMac get interface matching mac
var GetIntfMatchingMac = func(macaddr string) string {
	intfs, _ := net.Interfaces()
	for _, intf := range intfs {
		if intf.HardwareAddr.String() == macaddr {
			/* Mac address matched */
			return intf.Name
		}
	}
	return ""
}
