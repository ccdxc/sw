package runner

import (
	"encoding/json"
	"fmt"
	"io"
	"net"
	"os"
	"path/filepath"

	"github.com/pkg/sftp"
	"github.com/sirupsen/logrus"
	"golang.org/x/crypto/ssh"
)

const (
	scriptPath = "/tmp/git/pensando/sw/test/suites/run.sh "
	jsonPath   = "/warmd.json"
)

func getSSHClient(ip net.IP) (*ssh.Client, error) {
	return ssh.Dial("tcp", fmt.Sprintf("%s:22", ip), &ssh.ClientConfig{
		User:            "vm",
		Auth:            []ssh.AuthMethod{ssh.Password("vm")},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	})
}

func runSSH(ip net.IP, command string) error {
	sclient, err := getSSHClient(ip)
	if err != nil {
		return err
	}
	sSession, err := sclient.NewSession()
	if err != nil {
		return err
	}

	sOutPipe, err := sSession.StdoutPipe()
	if err != nil {
		return err
	}

	modes := ssh.TerminalModes{}
	if err := sSession.RequestPty("xterm", 40, 80, modes); err != nil {
		return err
	}

	go io.Copy(os.Stdout, sOutPipe)

	if err := sSession.Start(command); err != nil {
		return err
	}

	return sSession.Wait()
}

func getVMIP() net.IP {
	warmd, err := os.Open(jsonPath)
	if err != nil {
		panic(err)
	}

	hosts := map[string]interface{}{}

	if err := json.NewDecoder(warmd).Decode(&hosts); err != nil {
		panic(err)
	}

	instanceMap := hosts["Instances"].(map[string]interface{})

	instances := []string{}
	for key := range instanceMap {
		instances = append(instances, key)
	}

	return net.ParseIP(instanceMap[instances[0]].(string))
}

// RunSingle runs a make target on a single instance
func RunSingle(makeTarget string) error {
	cmd := scriptPath + makeTarget
	return RunCmd(cmd)
}

// RunCmd runs a target cmd on a single instance
func RunCmd(cmd string) error {
	server := getVMIP()
	err := runSSH(server, cmd)

	os.Stdout.Sync()
	return err
}

// CopyLogs copies logs
func CopyLogs(logs []string, destFolder string) error {
	ip := getVMIP()
	sshC, err := getSSHClient(ip)
	if err != nil {
		return err
	}

	sftpC, err := sftp.NewClient(sshC)
	if err != nil {
		logrus.Error(err)
		return err
	}
	defer sftpC.Close()

	for _, log := range logs {
		src, err := sftpC.Open(log)
		if err != nil {
			logrus.Errorf("Skipping %s - %v", log, err)
			continue
		}

		dest, err := os.Create(filepath.Join(destFolder, filepath.Base(log)))
		if err != nil {
			return err
		}

		_, err = io.Copy(dest, src)
		if err != nil {
			return err
		}
		src.Close()
		dest.Close()
		logrus.Infof("Wrote %s ", log)
	}

	return nil
}
