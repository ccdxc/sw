package runner

import (
	"encoding/json"
	"fmt"
	"io"
	"net"
	"os"

	"golang.org/x/crypto/ssh"
)

const (
	scriptPath = "/tmp/git/pensando/sw/test/suites/run.sh "
	jsonPath   = "/warmd.json"
)

func runSSH(ip net.IP, command string) error {
	sclient, err := ssh.Dial("tcp", fmt.Sprintf("%s:22", ip), &ssh.ClientConfig{
		User:            "vm",
		Auth:            []ssh.AuthMethod{ssh.Password("vm")},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	})
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

// RunSingle runs a make target on a single instance
func RunSingle(makeTarget string) error {
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

	server := net.ParseIP(instanceMap[instances[0]].(string))
	cmd := scriptPath + makeTarget
	err = runSSH(server, cmd)

	os.Stdout.Sync()
	return err
}
