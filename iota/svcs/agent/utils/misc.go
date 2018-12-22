package utils

import (
	"bytes"
	"fmt"
	"io"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"os/exec"
	"strings"
	"syscall"
	"time"
    "runtime"

	log "github.com/sirupsen/logrus"

	"golang.org/x/crypto/ssh"

	"github.com/pkg/errors"
	"github.com/pkg/sftp"
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

	stdoutStderr := ""
	exitCode := 0
	done := make(chan error)

	go func() {
		if background {
			fmt.Println("Starting background command :", strings.Join(cmdArgs, " "))
			//For now don't send signal to background process.
			process.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
			if err := process.Start(); err != nil {
				exitCode = 1
				stdoutStderr = ""
				done <- errors.Wrapf(err, "Background process start failed!")
				return
			}
			done <- nil
		} else {
			output, cmdErr := process.CombinedOutput()
			if cmdErr != nil {
				if exitError, ok := cmdErr.(*exec.ExitError); ok {
					ws := exitError.Sys().(syscall.WaitStatus)
					exitCode = ws.ExitStatus()
				} else {
					exitCode = 1
				}
				stdoutStderr = string(output)
				done <- errors.Wrap(cmdErr, string(output))
				return
			}
			exitCode = 0
			stdoutStderr = string(output)
			done <- nil
			return
		}
	}()

	if timeout != 0 {
		timeoutEvent := time.After(time.Duration(timeout) * time.Second)

		select {
		case <-timeoutEvent:
			// Timeout happened first, kill the process and print a message.
			process.Process.Signal(os.Kill)
			exitCode = 1
			return exitCode, stdoutStderr, errors.New("Command timed out")
		case err := <-done:
			return exitCode, stdoutStderr, err
		}
	}

	err := <-done
	return exitCode, stdoutStderr, err

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

func writeDhcpInterfaceDisableFile(intfName string, file string) error {

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

	fname := file
	return ioutil.WriteFile(fname, []byte(config), 0644)
}

//DisableDhcpOnInterfaceRemote disable DHCP on Remote interface
func DisableDhcpOnInterfaceRemote(conn *ssh.Client, intfName string) error {

	tmpFile := "/tmp/." + intfName
	dstFile := "/etc/sysconfig/network-scripts/ifcfg-" + intfName

	sftp, err := sftp.NewClient(conn)
	if sftp == nil || err != nil {
		return errors.New("Cannot create sftp client to disable dhcp interface")
	}

	defer sftp.Close()

	if err = writeDhcpInterfaceDisableFile(intfName, tmpFile); err != nil {
		return errors.Wrap(err, "Error in creating temp file")
	}

	out, err := ioutil.ReadFile(tmpFile)
	if err != nil {
		return errors.Wrap(err, "Error in reading temp file")
	}

	f, err := sftp.Create(tmpFile)
	if err != nil {
		return errors.Wrap(err, "Error in creating dst file")
	}
	f.Chmod(0766)

	if _, err = f.Write(out); err != nil {
		return errors.Wrap(err, "Error in writing file")
	}
	f.Close()

	time.Sleep(1 * time.Second)

	cmd := "sudo cp " + tmpFile + " " + dstFile + " && " + "sudo service network restart"

	// Create a session. It is one session per command.
	session, err := conn.NewSession()
	if err != nil {
		return err
	}
	defer session.Close()
	var b bytes.Buffer  // import "bytes"
	session.Stdout = &b // get output

	// Finally, run the command
	return session.Run(cmd)
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

var _transport = &http.Transport{
	Proxy: http.ProxyFromEnvironment,
	Dial: (&net.Dialer{
		Timeout:   0,
		KeepAlive: 0,
	}).Dial,
	TLSHandshakeTimeout: 10 * time.Second,
}

var _httpClient = &http.Client{Transport: _transport}

//RestHelper to do rest work
func restHelper(method string, url string, data *[]byte) error {

	var buffer io.Reader
	if data != nil {
		buffer = bytes.NewBuffer(*data)
	} else {
		buffer = (io.Reader)(nil)
	}
	req, reqErr := http.NewRequest(method, url, buffer)
	if reqErr != nil {
		return reqErr
	}
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Connection", "close")
	req.Close = true

	resp, err := _httpClient.Do(req)
	if err != nil || resp.StatusCode != 200 {
		return err
	}
	resp.Body.Close()

	return nil
}

//GetIntfMacAddress get interface address.
func GetIntfMacAddress(intfName string) (string, error) {
	intfs, _ := net.Interfaces()
	for _, intf := range intfs {
		if intf.Name == intfName {
			/* Mac address matched */
			return intf.HardwareAddr.String(), nil
		}
	}
	return "", errors.New("Interface not found")
}

//GetIntfsMatchingPrefix get interface with prefix name
var GetIntfsMatchingPrefix = func(prefix string) []string {
	ret := []string{}
	intfs, _ := net.Interfaces()
	for _, intf := range intfs {
		if strings.Contains(intf.Name, prefix) {
			ret = append(ret, intf.Name)
		}
	}
	return ret
}
// GetIntfsMatchingDevicePrefix get intfs matching device prefix
func GetIntfsMatchingDevicePrefixLinux(devicePrefix string) ([]string, error) {
	hostIntfs := []string{}

	pciIntfMap := make(map[string]string)
	cmd := []string{"systool", "-c", "net"}

	_, stdout, err := Run(cmd, 0, false, false, nil)
	if err != nil {
		return nil, errors.Wrap(err, stdout)
	}

	lines := strings.Split(stdout, "\n")
	for idx, line := range lines {
		if strings.Contains(line, "Class Device") {
			if (idx+1) < len(lines) && strings.Contains(lines[idx+1], "Device") {
				pci := strings.Replace(lines[idx+1], " ", "", -1)
				pci = strings.Split(pci, "=")[1]
				intfName := strings.Replace(line, " ", "", -1)
				intfName = strings.Split(intfName, "=")[1]
				pciIntfMap[pci] = intfName[1 : len(intfName)-1]
			}
		}
	}

	cmd = []string{"lspci", "|", "grep", "Ethernet"}
	_, stdout, err = Run(cmd, 0, false, true, nil)
	if err != nil {
		return nil, errors.Wrap(err, stdout)
	}

	for _, line := range strings.Split(stdout, "\n") {
		if strings.Contains(line, devicePrefix) {
			pci := strings.Split(line, " ")[0]
			for pciAddr, intf := range pciIntfMap {
				if strings.Contains(pciAddr, pci) {
                    if intf != "eno1" && intf != "eno2" {
    					hostIntfs = append(hostIntfs, intf)
                    }
				}
			}

		}
	}
	return hostIntfs, nil
}

// GetIntfsMatchingDevicePrefix get intfs matching device prefix
func GetIntfsMatchingDevicePrefixFreeBSD(devicePrefix string) ([]string, error) {
    hostIntfs := []string{}
	cmd := []string{"ifconfig", "-l"}

	_, stdout, err := Run(cmd, 0, false, true, nil)
	if err != nil {
		return nil, errors.Wrap(err, stdout)
	}
    for _, line := range strings.Split(stdout, "\n") {
        for _, intf := range strings.Split(line, " ") {
            if intf != "ix0" && intf != "ix1" && intf != "lo0" && intf != "tun0" && intf != "" {
                hostIntfs = append(hostIntfs, intf)
            }
        }
    }
    return hostIntfs, nil
}


// GetIntfsMatchingDevicePrefix get intfs matching device prefix
func GetIntfsMatchingDevicePrefix(devicePrefix string) ([]string, error) {
    if runtime.GOOS == "freebsd" {
        return GetIntfsMatchingDevicePrefixFreeBSD(devicePrefix)
    } else {
        return GetIntfsMatchingDevicePrefixLinux(devicePrefix)
    }
}

//RestHelper is a wrapper for rest
var RestHelper = restHelper
