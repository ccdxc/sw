package qemuHelpers

import (
	"net"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/test/utils/infra"
)

func prepareQemuCommand(sshIP, sshUser, sshPort, userCmd string) string {
	userCmdStr := "\"" + userCmd + "\""
	qemuSSHCmdStr := "ssh -p '" + sshPort + "' '" + sshUser + "@" + sshIP + "'" + " 'sh -c "
	command := qemuSSHCmdStr + userCmdStr + "'"

	return command
}

//StartQemu start qemu machine
func StartQemu(hostIP net.IP) error {

	err := infra.RunSSH(hostIP, "nohup sh -c \"cd /mnt/src/sw/platform; . ./pensando-helpers.sh; export SIMSOCK_PATH=/home/vm/naples/data/simsock-turin; start_qemu > qemu.out 2> qemu.err & \"")

	if err != nil {
		logrus.Errorf("Failed to start Qemu. Error: %v", err)
	}

	return err
}

//StopQemu stop qemu machine
func StopQemu(hostIP net.IP) error {
	err := infra.RunSSH(hostIP, "pkill -f qemu")

	if err != nil {
		logrus.Errorf("Failed to stop Qemu. Error: %v", err)
	}

	return err
}

//RunCmdInQemuHost run user command in qemu machine
func RunCmdInQemuHost(hostIP net.IP, sshIP, sshUser, sshPort, userCmd string) error {

	command := prepareQemuCommand(sshIP, sshUser, sshPort, userCmd)

	logrus.Infof("------")
	logrus.Infof("%s | ==> Executing Cmd: %s ", "Qemu", command)
	logrus.Infof("------")

	err := infra.RunSSH(hostIP, command)

	if err != nil {
		logrus.Errorf("Command failed in Qemu. Error: %v", err)
		logrus.Infof("Failed command was: %s", userCmd)
	}

	return err
}
