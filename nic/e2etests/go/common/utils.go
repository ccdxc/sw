package common

import (
	"bufio"
	"errors"
	"fmt"
	"log"
	"net"
	"os"
	"os/exec"
	"reflect"
	"strings"
	"syscall"
	"time"
)

//CmdHandle Running command handle to stop bg process later
type CmdHandle *exec.Cmd

//CommandVerbose should command run be verbose or not
var CommandVerbose = true

//Run Run shelll command
func Run(cmdArgs []string, timeout int, background bool) (CmdHandle, error) {
	process := exec.Command(cmdArgs[0], cmdArgs[1:]...)
	var err error
	if background {
		if CommandVerbose {
			fmt.Println("Starting background command :", strings.Join(cmdArgs, " "))
		}
		//For now don't send signal to background process.
		process.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
		err = process.Start()
		return process, err
	}
	if CommandVerbose {
		fmt.Println("Running command :", strings.Join(cmdArgs, " "))
	}
	stdoutStderr, err := process.CombinedOutput()
	if CommandVerbose {
		fmt.Printf("%s\n", stdoutStderr)
		fmt.Println("Command Status : ", err)
	}
	return nil, err
}

//Stop Stop bg process running
func Stop(cmdHandle CmdHandle) {
	cmdHandle.Process.Kill()
}

//GetTypeName Get type name fom instance
func GetTypeName(myvar interface{}) string {
	return reflect.TypeOf(myvar).Name()
}

//IncrementIP address
func IncrementIP(origIP string, cidr string) (string, error) {
	ip := net.ParseIP(origIP)
	_, ipNet, err := net.ParseCIDR(cidr)
	if err != nil {
		return origIP, err
	}
	for i := len(ip) - 1; i >= 0; i-- {
		ip[i]++
		if ip[i] != 0 {
			break
		}
	}
	if !ipNet.Contains(ip) {
		log.Fatalln("CIDR overflow")
	}
	return ip.String(), nil
}

//IncrementCidr Cid address
func IncrementCidr(cidr string) (string, error) {
	_, ipNet, err := net.ParseCIDR(cidr)
	if err != nil {
		fmt.Println("Parse ", cidr)
		log.Fatalln("CIDR parse error!")
	}
	for i := 0; i < len(ipNet.IP)-1; i++ {
		ipNet.IP[i]++
		if ipNet.IP[i] != 0 {
			break
		}
	}
	return ipNet.String(), nil
}

//IncrementMacAddress Mac address
func IncrementMacAddress(mac string) (string, error) {
	macAddr, err := net.ParseMAC(mac)
	if err != nil {
		log.Fatalln("Mac Address Parse error!")
	}
	for i := len(macAddr) - 1; i > 1; i-- {
		if macAddr[i] < 255 {
			macAddr[i]++
			break
		} else {
			macAddr[i] = 1
			macAddr[i-1]++
			break
		}
	}
	return macAddr.String(), nil
}

//WaitForLineInLog Wait for line in log
func WaitForLineInLog(file string, match string, timeout time.Duration) error {
	logFile, err := os.Open(file)
	if err != nil {
		panic("Error opening Log file : " + file)
	}
	defer logFile.Close()
	var cTimeout <-chan time.Time

	if timeout != 0 {
		cTimeout = time.After(timeout)
	}
	for true {
		scanner := bufio.NewScanner(logFile)
		for scanner.Scan() {
			if strings.Contains(scanner.Text(), match) {
				return nil
			}
		}
		select {
		case <-cTimeout:
			return errors.New("Timeout : Waiting for line in log ")
		default:
		}
	}
	return nil
}
