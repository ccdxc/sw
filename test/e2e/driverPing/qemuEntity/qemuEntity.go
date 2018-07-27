package qemuEntity

import (
	"fmt"
	"os"
	"time"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/test/e2e/driverPing/hostEntity"
	"github.com/pensando/sw/test/e2e/driverPing/qemuHelpers"
	"github.com/pensando/sw/test/e2e/driverPing/remoteEntity"
)

//QemuEntity entity to operate on qemu machine
type QemuEntity struct {
	Host            hostEntity.HostEntity
	QemuSSHIP       string
	QemuSSHUserName string
	QemuSSHPwd      string
	QemuSSHPortNum  string
	Name            string
}

type qemuSetupSteps struct {
	re            remoteEntity.RemoteEntity
	cmdToExec     string
	waitInSeconds time.Duration
}

///// APIs for QemuEntity

//qemuSSH helper function for qemu
func qemuSSH(user, ip, port string) string {
	//return "ssh '" + user + "@" + ip + " -p " + port +"'"
	return "ssh -p '" + port + "' '" + user + "@" + ip + "'"
}

//Exec run user command in qemu machine
func (e QemuEntity) Exec(userCmd string) error {
	return qemuHelpers.RunCmdInQemuHost(e.Host.IPAddress, e.QemuSSHIP, e.QemuSSHUserName, e.QemuSSHPortNum, userCmd)
}

//GetEntityName get qemu entity name
func (e QemuEntity) GetEntityName() string {
	return "Qemu [Host IP: " + e.Host.IPAddress.String() + "]"
}

//Setup setup qemu machine
func (e QemuEntity) Setup() error {
	var err error
	cmdList := []qemuSetupSteps{
		{re: e.Host, cmdToExec: "ls -l /home/vm/naples/data/simsock-turin; nohup sh -c \"cd /mnt/src/sw/platform; . ./pensando-helpers.sh; export  SIMSOCK_PATH=/home/vm/naples/data/simsock-turin; start_qemu > qemu.out 2> qemu.err & \"", waitInSeconds: 100},
		{re: e.Host, cmdToExec: "sshpass -p \"" + e.QemuSSHPwd + "\" ssh-copy-id " + e.QemuSSHUserName + "@" + e.QemuSSHIP + " -p " + e.QemuSSHPortNum, waitInSeconds: 0},
		{re: e.Host, cmdToExec: qemuSSH(e.QemuSSHUserName, e.QemuSSHIP, e.QemuSSHPortNum) + " 'sh -c \"cat /home/" + e.QemuSSHUserName + "/.ssh/id_rsa.pub\"' >> ~/.ssh/authorized_keys", waitInSeconds: 0},
		{re: e.Host, cmdToExec: "echo \"StrictHostKeyChecking no\" | " + qemuSSH(e.QemuSSHUserName, e.QemuSSHIP, e.QemuSSHPortNum) + " sudo 'sh -c  \"cat >> /etc/ssh/ssh_config\"'", waitInSeconds: 0},
		{re: e, cmdToExec: "sudo mkdir -p /mnt/src; sudo umount /mnt/src; sudo sshfs -o allow_other,IdentityFile=/home/" + e.QemuSSHUserName + "/.ssh/id_rsa vm@" + e.Host.IPAddress.String() + ":/mnt/src /mnt/src", waitInSeconds: 0},
	}

	for _, obj := range cmdList {

		err = obj.re.Exec(obj.cmdToExec)

		if err != nil {
			logrus.Errorf("%s: Setup() Failed! Failed to execute cmd: %s: Error: %v", obj.re.GetEntityName(), obj.cmdToExec, err)
			e.Host.Cleanup()
			os.Exit(-1)
		}

		//Wait for command to be finished
		if obj.waitInSeconds != 0 {
			fmt.Println("Waiting for Qemu to be up ...")
			time.Sleep(obj.waitInSeconds * time.Second)
		}
	} //end of for

	fmt.Println("Qemu setup finished!!!")

	return err
}

//Cleanup clean up qemu machine
func (e QemuEntity) Cleanup() error {

	//Kill qemu process in Host
	err := qemuHelpers.StopQemu(e.Host.IPAddress)
	if err != nil {
		logrus.Errorf("Error in clean-up(StopQemu), Error: %v", err)
	}

	return err
}
