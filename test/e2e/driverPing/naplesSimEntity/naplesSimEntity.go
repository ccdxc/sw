package naplesSimEntity

import (
	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/test/e2e/driverPing/hostEntity"
	"github.com/pensando/sw/test/utils/infra"
)

//NaplesSimEntity entity to operate on naples-sim container
type NaplesSimEntity struct {
	Host hostEntity.HostEntity
	Name string
}

//Exec Execute a command in NaplesSimEntity
func (e NaplesSimEntity) Exec(userCmd string) error {

	logrus.Infof("-----------")
	logrus.Infof("%s | ==> Executing Cmd: '%s' ", e.Name, userCmd)
	logrus.Infof("-----------")

	//	naplesSimCmd := "docker exec " + e.Name + " " + "sh -c ''" + userCmd + "''"
	err := infra.RunSSH(e.Host.IPAddress, userCmd)
	if err != nil {
		logrus.Errorf("Failed to execute userCmd: %s Error: %v", userCmd, err)
	}

	return err
}

//GetEntityName Get name of NaplesSimEntity
func (e NaplesSimEntity) GetEntityName() string {
	return "NaplesSimEntity Name: " + e.Name
}

//Setup setup NaplesSimEntity
func (e NaplesSimEntity) Setup() error {

	cmdToExec := "cd /mnt/src/sw/nic/obj/images && ../../sim/naples/start-naples-docker.sh --qemu"
	err := e.Exec(cmdToExec)

	return err
}

//Cleanup cleanup NaplesSimEntity
func (e NaplesSimEntity) Cleanup() error {

	//stop naples-sim container
	err := e.Exec("docker stop naples-v1 && sudo rm -rf ~/naples/data/*")
	if err != nil {
		logrus.Errorf("Error in clean-up(stopNaplesSim), Error: %v", err)
	}

	return err
}
