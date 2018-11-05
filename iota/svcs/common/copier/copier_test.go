package copier

import (
	"io/ioutil"
	"log"
	"os"
	"testing"
	"time"

	"golang.org/x/crypto/ssh"

	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
)

func Test_Copier(t *testing.T) {
	startCmd := []string{"docker", "run", "--rm", "-d", "--publish=22:22", "--name", "test_copy_node", "sickp/alpine-sshd:7.5-r2"}
	if retCode, stdout, err := Utils.RunCmd(startCmd, 0, false, false, nil); err != nil || retCode != 0 {
		t.Errorf("Could not start docker container %v %v ", stdout, err)
	}
	stopCmd := []string{"docker", "stop", "test_copy_node"}
	defer Utils.RunCmd(stopCmd, 0, false, false, nil)
	tmpFiles := []string{"/tmp/dat1", "/tmp/dat2", "/tmp/dat3"}

	time.Sleep(3 * time.Second)

	sshConfig := &ssh.ClientConfig{
		User: "root",
		Auth: []ssh.AuthMethod{
			ssh.Password("root"),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	copier := NewCopier(sshConfig)

	d1 := []byte("THIS IS  TEST FILE \n")
	for _, file := range tmpFiles {
		err := ioutil.WriteFile(file, d1, 0644)
		if err != nil {
			log.Fatalf("File creation failed")
		}
	}

	if err := copier.CopyTo("127.0.0.1:22", "/tmp", tmpFiles); err != nil {
		log.Fatalf("Copy to failed : %v", err.Error())
	}

	//Remove those files
	for _, file := range tmpFiles {
		os.Remove(file)
	}

	if err := copier.CopyFrom("127.0.0.1:22", "/tmp/", tmpFiles); err != nil {
		log.Fatalf("Copy from failed : %v", err.Error())

	}

	for _, file := range tmpFiles {
		if _, err := os.Stat(file); os.IsNotExist(err) {
			log.Fatalf("File does not exist %v", err.Error())
		}
	}
}
