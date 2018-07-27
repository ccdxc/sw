package veniceEntity

import (
	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/test/e2e/driverPing/hostEntity"
	"github.com/pensando/sw/test/utils/infra"
)

//VeniceEntity entity to operate on venice agent
type VeniceEntity struct {
	Host hostEntity.HostEntity
	Name string
}

//Exec Execute a command in VeniceEntity
func (e VeniceEntity) Exec(userCmd string) error {

	logrus.Infof("--------")
	logrus.Infof("%s | ==> Executing Cmd: %s", e.Name, userCmd)
	logrus.Infof("--------")

	err := infra.RunSSH(e.Host.IPAddress, userCmd)
	if err != nil {
		logrus.Errorf("Failed to execute userCmd: %s Error: %v", userCmd, err)
	}

	return err
}

//GetEntityName get name of VeniceEntity
func (e VeniceEntity) GetEntityName() string {
	return "VeniceEntity Name: " + e.Name
}

//Setup setup VeniceEntity
func (e VeniceEntity) Setup() error {

	var err error
	cmd := "sudo mkdir -p /var/run/netns/; sudo touch ~/naples/data/logs/agent.log && cd /mnt/src/sw/nic/ && sudo python3 e2etests/e2e_setup.py --config-file e2etests/naples_cfg/e2e.cfg --naplescontainer naples-v1"

	err = e.Exec(cmd)
	if err != nil {
		logrus.Errorf("%s: Setup() Failed! Failed to execute userCmd: %s: Error: %v", e.Name, cmd, err)
	}

	return err
}

//Cleanup cleanup VeniceEntity
func (e VeniceEntity) Cleanup() error {

	//delete the namespace
	err := e.Host.Exec("sudo ip -all netns delete")
	if err != nil {
		logrus.Errorf("Error in deleting namespaces, Error: %v", err)
	}

	return err
}
