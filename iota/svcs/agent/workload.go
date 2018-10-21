package agent

import (
	"strconv"

	"github.com/pkg/errors"

	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
	Common "github.com/pensando/sw/iota/svcs/common"
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

type bareMetalWorkload struct {
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

var runArpCmd = func(app workload, ip string, intf string) error {
	arpCmd := []string{"arping", "-c", "5", "-U", ip, "-I", intf}
	if retCode, stdout, _, _ := app.RunCommand(arpCmd, 0, false, false); retCode != 0 {
		return errors.Errorf("Could not send arprobe for  %s (%s) : %s", ip, intf, stdout)
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

	return runArpCmd(app, ip, intf)

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
	if app.containerHandle != nil {
		app.containerHandle.Stop()
	}
}

func (app *bareMetalWorkload) BringUp(name string, image string) error {
	return nil
}

func (app *bareMetalWorkload) AddVlanInterface(parentIntf string, parentMacAddress string, vlan int) (string, error) {
	return "", nil
}

func (app *bareMetalWorkload) MoveInterface(name string) error {

	return nil

}

func (app *bareMetalWorkload) SendArpProbe(ip string, intf string, vlan int) error {

	if ip == "" {
		return nil
	}

	if vlan != 0 {
		intf = vlanIntf(intf, vlan)
	}

	return runArpCmd(app, ip, intf)

}

func (app *bareMetalWorkload) AddInterface(name string, macAddress string, ipaddress string, vlan int) error {

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

	if macAddress != "" {
		setMacAddrCmd := []string{"ifconfig", intfToAttach, "hw", "ether", macAddress}
		if retCode, stdout, err := Utils.Run(setMacAddrCmd, 0, false, false, nil); retCode != 0 {
			return errors.Wrap(err, stdout)
		}
	}

	if ipaddress != "" {
		cmd := []string{"ifconfig", intfToAttach, ipaddress}
		if retCode, stdout, err := Utils.Run(cmd, 0, false, false, nil); retCode != 0 {
			return errors.Wrap(err, stdout)
		}
	}

	return nil
}

func (app *bareMetalWorkload) RunCommand(cmd []string, timeout uint32, background bool, shell bool) (int32, string, string, error) {
	retCode, stdout, err := Utils.Run(cmd, (int)(timeout), background, shell, nil)
	return (int32)(retCode), stdout, "", err
}

func (app *bareMetalWorkload) IsWorkloadHealthy() bool {
	return true
}

func (app *bareMetalWorkload) TearDown() {
}

func newWorkload() workload {
	return &containerWorkload{}
}

func newBareMetalWorkload() workload {
	return &bareMetalWorkload{}
}
