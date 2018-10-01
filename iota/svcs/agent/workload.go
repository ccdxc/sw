package agent

import (
	Utils "github.com/pensando/sw/iota/svcs/agent/utils"
)

type workload interface {
	BringUp(name string, args ...string) error
	RunCommand(cmd []string, timeout uint32, background bool, shell bool) (int, string, string, error)
	AttachInterface(name string, macAddress string, ipaddress string, prefixLen int) error
	AddVlanInterface(parentIntf string, parentMacAddress string, vlan int) (string, error)
	IsWorkloadHealthy() bool
	TearDown()
}

type containerWorkload struct {
	containerHandle *Utils.Container
}

func (app *containerWorkload) BringUp(name string, args ...string) error {
	var err error
	app.containerHandle, err = Utils.NewContainer(name, args[0], "")
	return err
}

func (app *containerWorkload) AddVlanInterface(parentIntf string, parentMacAddress string, vlan int) (string, error) {
	return "", nil
}

func (app *containerWorkload) AttachInterface(name string, macAddress string, ipaddress string, prefixLen int) error {
	return nil
}

func (app *containerWorkload) RunCommand(cmd []string, timeout uint32, background bool, shell bool) (int, string, string, error) {
	return 0, "", "", nil
}

func (app *containerWorkload) IsWorkloadHealthy() bool {
	return true
}

func (app *containerWorkload) TearDown() {
}

func newWorkload() workload {
	return &containerWorkload{}
}
