package infra

import (
	"encoding/json"
	"fmt"
	"io"
	"net"
	"os"
	"path"
	"path/filepath"

	"github.com/pensando/test-infra/public"
	"github.com/pkg/sftp"
	"github.com/sirupsen/logrus"
	"golang.org/x/crypto/ssh"
)

const (
	scriptPath = "/import/src/github.com/pensando/sw/test/ci_targets/run.sh "
)

var (
	//can overidden by unittest
	jsonPath = "/warmd.json"
)

func getSSHClient(ip net.IP) (*ssh.Client, error) {
	return ssh.Dial("tcp", fmt.Sprintf("%s:22", ip), &ssh.ClientConfig{
		User:            "vm",
		Auth:            []ssh.AuthMethod{ssh.Password("vm")},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	})
}

//RunSSH run a command over SSH to remote machine
func RunSSH(ip net.IP, command string) error {
	sclient, err := getSSHClient(ip)
	if err != nil {
		return err
	}
	return runSSHWithClient(sclient, command)
}

func runSSHWithClient(sclient *ssh.Client, command string) error {
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

	warmdEnv := public.WarmdEnv{}

	if err := json.NewDecoder(warmd).Decode(&warmdEnv); err != nil {
		panic(err)
	}

	if len(warmdEnv.Instances) == 0 {
		return nil
	}
	return net.ParseIP(warmdEnv.Instances[0].NodeMgmtIP)
}

// RunSingle runs a make target on a single instance
func RunSingle(makeTarget string) error {
	cmd := scriptPath + makeTarget
	return RunCmd(cmd)
}

// RunCmd runs a target cmd on a single instance
func RunCmd(cmd string) error {
	server := getVMIP()
	err := RunSSH(server, cmd)

	os.Stdout.Sync()
	return err
}

// CopyRemoteLogs copies logs
func CopyRemoteLogs(logs []string, destFolder string) error {
	ip := getVMIP()
	sshC, err := getSSHClient(ip)
	if err != nil {
		return err
	}
	return copyRemoteLogsWithSSHClient(sshC, logs, destFolder)
}
func copyRemoteLogsWithSSHClient(sshC *ssh.Client, logs []string, destFolder string) error {
	sftpC, err := sftp.NewClient(sshC)
	if err != nil {
		logrus.Error(err)
		return err
	}
	defer sftpC.Close()

	for _, log := range logs {
		dest, err := sftpC.Create(destFolder)
		if err != nil {
			logrus.Errorf("Skipping %s - %v", log, err)
			continue
		}

		//path := filepath.Join(destFolder, filepath.Base(log))
		logrus.Infof("Downloading %s to %s", log, destFolder)
		src, err := os.Open(log)
		if err != nil {
			return err
		}

		size, err := io.Copy(dest, src)
		if err != nil {
			return err
		}
		src.Close()
		dest.Close()

		statInfo, err := sftpC.Stat(destFolder)
		if err != nil {
			return err
		}
		if size != statInfo.Size() {
			return fmt.Errorf("Downloaded size %d is not equal to file size %d", size, statInfo.Size())
		}

		logrus.Infof("Wrote %s to (size %d)", log, size)
	}

	return nil
}

// CopyLogs copies logs
func CopyLogs(logs []string, destFolder string) error {
	ip := getVMIP()
	sshC, err := getSSHClient(ip)
	if err != nil {
		return err
	}

	return copyLogsWithSSHClient(sshC, logs, destFolder)
}

func copyLogsWithSSHClient(sshC *ssh.Client, logs []string, destFolder string) error {
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

		path := filepath.Join(destFolder, filepath.Base(log))
		logrus.Infof("Downloading %s to %s", log, path)
		dest, err := os.Create(path)
		if err != nil {
			return err
		}

		size, err := io.Copy(dest, src)
		if err != nil {
			return err
		}
		src.Close()
		dest.Close()

		statInfo, err := os.Stat(path)
		if err != nil {
			return err
		}
		if size != statInfo.Size() {
			return fmt.Errorf("Downloaded size %d is not equal to file size %d", size, statInfo.Size())
		}

		logrus.Infof("Wrote %s to %s (size %d)", log, path, size)
	}

	return nil
}

//UploadFile uploads one file through sftp
func UploadFile(localName, remoteName string) error {
	lf, err := os.Open(localName)
	if err != nil {
		return err
	}
	defer lf.Close()

	ip := getVMIP()

	sclient, err := getSSHClient(ip)
	if err != nil {
		return err
	}
	defer sclient.Close()
	sftpC, err := sftp.NewClient(sclient)
	if err != nil {
		return err
	}
	defer sftpC.Close()

	if err := sftpC.MkdirAll(path.Dir(remoteName)); err != nil {
		return err
	}

	rf, err := sftpC.Create(remoteName)
	if err != nil {
		return err
	}
	defer rf.Close()

	_, err = io.Copy(rf, lf)
	return err
}
