package common

import (
	"fmt"
	"io/ioutil"
	"log"
	"net"
	"os"
	"os/exec"
	"strings"
	"syscall"
	"time"

	"github.com/pkg/errors"
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
		newCmdArgs := []string{"sh", "-c", fullCmd}
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

	return nil
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
