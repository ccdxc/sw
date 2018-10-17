package agent

import (
	"strconv"

	"github.com/pkg/errors"

	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	Common "github.com/pensando/sw/iota/svcs/common"
)

const (
	workloadImage = "registry.test.pensando.io:5000/pensando/nic/e2e:2.0"
)

var (
	workloadDir = Common.DstIotaWorkloadsDir
)

type workload interface {
	BringUp(name string, image string) error
	RunCommand(cmd []string, timeout uint32, background bool, shell bool) (int32, string, string, error)
	AddInterface(name string, macAddress string, ipaddress string, vlan int) error
	MoveInterface(name string) error
	IsWorkloadHealthy() bool
	SendArpProbe(ip string, intf string, vlan int) error
	TearDown()
}

type containerWorkload struct {
	containerHandle *Utils.Container
}

func vlanIntf(name string, vlan int) string {
	return name + "_" + strconv.Itoa(vlan)
}

func (app *containerWorkload) BringUp(name string, image string) error {
	var err error
	app.containerHandle, err = Utils.NewContainer(name, image, "", workloadDir)
	return err
}

func (app *containerWorkload) AddVlanInterface(parentIntf string, parentMacAddress string, vlan int) (string, error) {
	return "", nil
}

func (app *containerWorkload) MoveInterface(name string) error {

	ifconfigCmd := []string{"ifconfig", name, "up"}
	if retCode, stdout, _ := Utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return errors.Errorf("Could not bring up parent interface %s : %s", name, stdout)
	}

	if err := app.containerHandle.AttachInterface(name); err != nil {
		return errors.Wrap(err, "Interface attach failed")
	}

	return nil

}

func (app *containerWorkload) SendArpProbe(ip string, intf string, vlan int) error {

	if ip == "" {
		return nil
	}

	if vlan != 0 {
		intf = vlanIntf(intf, vlan)
	}
	arpCmd := []string{"arping", "-c", "5", "-U", ip, "-I", intf}
	if retCode, stdout, _, _ := app.RunCommand(arpCmd, 0, false, false); retCode != 0 {
		return errors.Errorf("Could not send arprobe for  %s (%s) : %s", ip, intf, stdout)
	}

	return nil

}

func (app *containerWorkload) AddInterface(name string, macAddress string, ipaddress string, vlan int) error {

	ifconfigCmd := []string{"ifconfig", name, "up"}
	if retCode, stdout, _ := Utils.Run(ifconfigCmd, 0, false, false, nil); retCode != 0 {
		return errors.Errorf("Could not bring up parent interface %s : %s", name, stdout)
	}
	intfToAttach := name

	if vlan != 0 {
		vlanintf := vlanIntf(name, vlan)
		addVlanCmd := []string{"ip", "link", "add", "link", name, "name", vlanintf, "type", "vlan", "id", strconv.Itoa(vlan)}
		if retCode, stdout, _ := Utils.Run(addVlanCmd, 0, false, false, nil); retCode != 0 {
			return errors.Errorf("IP link create to add vlan failed %s:%d, err :%s", name, vlan, stdout)
		}
		intfToAttach = vlanintf
	}

	if err := app.containerHandle.AttachInterface(intfToAttach); err != nil {
		return errors.Wrap(err, "Interface attach failed")
	}

	if macAddress != "" {
		if err := app.containerHandle.SetMacAddress(intfToAttach, macAddress, 0); err != nil {
			return errors.Wrapf(err, "Set Mac Address failed")
		}
	}

	if ipaddress != "" {
		if err := app.containerHandle.SetIPAddress(intfToAttach, ipaddress, 0); err != nil {
			return errors.Wrapf(err, "Set IP Address failed")
		}
	}

	return nil
}

func (app *containerWorkload) RunCommand(cmd []string, timeout uint32, background bool, shell bool) (int32, string, string, error) {
	return app.containerHandle.RunCommand(cmd, timeout, background, shell)
}

func (app *containerWorkload) IsWorkloadHealthy() bool {
	return app.containerHandle.IsHealthy()
}

func (app *containerWorkload) TearDown() {
	app.containerHandle.Stop()
}

func newWorkload() workload {
	return &containerWorkload{}
}
