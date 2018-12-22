package vmware

import (
	"fmt"

	"github.com/pkg/errors"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/types"
)

// VM encapsulates a single VM
type VM struct {
	host *Host
	name string
	vm   *object.VirtualMachine
}

type VMInfo struct {
	Name string
	IP   string
}

func (vm VM) Name() string {
	return vm.name
}

func (h *Host) makeVM(name string, vm *object.VirtualMachine) *VM {
	return &VM{
		host: h,
		name: name,
		vm:   vm,
	}
}

// NewVM creates a new virtual machine
func (h *Host) NewVM(name string) (*VM, error) {
	finder, _, err := h.client.finder()
	if err != nil {
		return nil, err
	}

	vm, err := finder.VirtualMachine(h.context.context, name)
	if err != nil {
		return nil, err
	}

	return h.makeVM(name, vm), nil
}

// Destroy the VM
func (vm *VM) Destroy() error {
	state, err := vm.vm.PowerState(vm.host.context.context)
	if err != nil {
		return err
	}

	if state != types.VirtualMachinePowerStatePoweredOff {
		task, err := vm.vm.PowerOff(vm.host.context.context)
		if err != nil {
			return err
		}

		if err := task.Wait(vm.host.context.context); err != nil {
			return err
		}
	}

	task, err := vm.vm.Destroy(vm.host.context.context)
	if err != nil {
		return err
	}

	return task.Wait(vm.host.context.context)
}

//ReconfigureNetwork will connect interface connected from one network to other network
func (vm *VM) ReconfigureNetwork(currNW string, newNW string) error {

	var task *object.Task
	var devList object.VirtualDeviceList
	var err error

	devList, err = vm.vm.Device(vm.host.context.context)
	if err != nil {
		return errors.Wrap(err, "Failed to device list of VM")
	}

	for _, d := range devList.SelectByType((*types.VirtualEthernetCard)(nil)) {
		veth := d.GetVirtualDevice()

		if veth.Backing.(*types.VirtualEthernetCardNetworkBackingInfo).DeviceName != currNW {
			fmt.Println("Skipping as network  ", veth.Backing.(*types.VirtualEthernetCardNetworkBackingInfo).DeviceName, currNW)
			continue
		}
		veth.Backing = &types.VirtualEthernetCardNetworkBackingInfo{
			VirtualDeviceDeviceBackingInfo: types.VirtualDeviceDeviceBackingInfo{
				VirtualDeviceBackingInfo: types.VirtualDeviceBackingInfo{},
				DeviceName:               newNW,
			},
		}

		fmt.Println("Configuring network...", newNW)
		task, err = vm.vm.Reconfigure(vm.host.context.context, types.VirtualMachineConfigSpec{DeviceChange: []types.BaseVirtualDeviceConfigSpec{&types.VirtualDeviceConfigSpec{Operation: types.VirtualDeviceConfigSpecOperationEdit, Device: d}}})
		if err != nil {
			return err
		}
		if err := task.Wait(vm.host.context.context); err != nil {
			return errors.Wrap(err, "Reconfiguring to network failed")
		}
	}

	return nil
}
