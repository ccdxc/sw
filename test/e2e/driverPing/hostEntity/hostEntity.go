package hostEntity

import (
	"net"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/test/utils/infra"
)

//HostEntity entity for operating on host machine
type HostEntity struct {
	IPAddress net.IP
}

////// APIs for HostEntity

//Exec user command in host
func (e HostEntity) Exec(userCmd string) error {

	logrus.Infof("-----------------------")
	logrus.Infof("%s | ==> Executing Cmd: %s", e.GetEntityName(), userCmd)
	logrus.Infof("-----------------------")

	err := infra.RunSSH(e.IPAddress, userCmd)
	if err != nil {
		logrus.Errorf("Failed to execute userCmd: %s Error: %v", userCmd, err)
	}

	return err
}

//GetEntityName get host IP address
func (e HostEntity) GetEntityName() string {
	return "Host IP: " + e.IPAddress.String()
}

//Setup setup host machine
func (e HostEntity) Setup() error {
	return nil
}

//Cleanup cleanup host machine
func (e HostEntity) Cleanup() error {
	return nil
}
