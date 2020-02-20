package vmware

import (
	"fmt"

	"github.com/pkg/errors"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/property"
	"github.com/vmware/govmomi/vim25/types"
)

// VM encapsulates a single VM
type VM struct {
	entity *Entity
	name   string
	vm     *object.VirtualMachine
}

// VMInfo contains info about a vm
type VMInfo struct {
	Name string
	IP   string
}

// Name returns name of the vm
func (vm VM) Name() string {
	return vm.name
}

// NewVM creates a new virtual machine
func (h *Host) NewVM(name string) (*VM, error) {
	finder := h.Finder()

	vm, err := finder.VirtualMachine(h.Ctx(), name)
	if err != nil {
		return nil, err
	}

	return h.makeVM(name, vm), nil
}

// Destroy the VM
func (vm *VM) Destroy() error {
	if err := vm.PowerOff(); err != nil {
		return err
	}

	task, err := vm.vm.Destroy(vm.entity.Ctx())
	if err != nil {
		return err
	}

	return task.Wait(vm.entity.Ctx())
}

// PowerOff the VM
func (vm *VM) PowerOff() error {
	state, err := vm.vm.PowerState(vm.entity.Ctx())
	if err != nil {
		return err
	}

	if state != types.VirtualMachinePowerStatePoweredOff {
		task, err := vm.vm.PowerOff(vm.entity.Ctx())
		if err != nil {
			return err
		}

		return task.Wait(vm.entity.Ctx())
	}

	return nil
}

// Migrate VM to destination host
func (vm *VM) Migrate(host *Host, dref *types.ManagedObjectReference) error {

	href := host.hs.Reference()
	config := types.VirtualMachineRelocateSpec{
		Datastore: dref,
		Host:      &href,
	}
	task, err := vm.vm.Relocate(vm.entity.Ctx(), config,
		types.VirtualMachineMovePriorityDefaultPriority)
	if err != nil {
		return err
	}

	return task.Wait(vm.entity.Ctx())

}

//ReconfigureNetwork will connect interface connected from one network to other network
func (vm *VM) ReconfigureNetwork(currNW string, newNW string, maxReconfigs int) error {

	var task *object.Task
	var devList object.VirtualDeviceList
	var err error

	net, err := vm.entity.Finder().Network(vm.entity.Ctx(), newNW)
	if err != nil {
		return errors.Wrap(err, "Failed Find Network")
	}

	curNet, err := vm.entity.Finder().Network(vm.entity.Ctx(), currNW)
	if err != nil {
		return errors.Wrap(err, "Failed Find current Network")
	}

	curNetRef, err := curNet.EthernetCardBackingInfo(vm.entity.Ctx())
	if err != nil {
		return errors.Wrap(err, "Failed Find current Network")
	}

	nwRef, err := net.EthernetCardBackingInfo(vm.entity.Ctx())
	if err != nil {
		return errors.Wrap(err, "Failed Find Network devices")
	}

	devList, err = vm.vm.Device(vm.entity.Ctx())
	if err != nil {
		return errors.Wrap(err, "Failed to device list of VM")
	}

	reconfigs := 0
	for _, d := range devList.SelectByType((*types.VirtualEthernetCard)(nil)) {
		veth := d.GetVirtualDevice()

		switch nw := nwRef.(type) {
		case *types.VirtualEthernetCardNetworkBackingInfo:
			switch a := veth.Backing.(type) {
			case *types.VirtualEthernetCardNetworkBackingInfo:
				if veth.Backing.(*types.VirtualEthernetCardNetworkBackingInfo).DeviceName != currNW {
					fmt.Println("Skipping as current network does not match ", currNW)
					continue
				}
			case *types.VirtualEthernetCardDistributedVirtualPortBackingInfo:
				switch curn := curNetRef.(type) {
				case *types.VirtualEthernetCardDistributedVirtualPortBackingInfo:
					if a.Port.SwitchUuid != curn.Port.SwitchUuid ||
						a.Port.PortgroupKey != curn.Port.SwitchUuid {
						fmt.Println("Skipping as current network does not match ", currNW)
						continue
					}
				}
			}

			veth.Backing = &types.VirtualEthernetCardNetworkBackingInfo{
				VirtualDeviceDeviceBackingInfo: types.VirtualDeviceDeviceBackingInfo{
					VirtualDeviceBackingInfo: types.VirtualDeviceBackingInfo{},
					DeviceName:               newNW,
				},
			}
		case *types.VirtualEthernetCardDistributedVirtualPortBackingInfo:
			switch a := veth.Backing.(type) {
			case *types.VirtualEthernetCardDistributedVirtualPortBackingInfo:
				switch curn := curNetRef.(type) {
				case *types.VirtualEthernetCardDistributedVirtualPortBackingInfo:
					if a.Port.SwitchUuid != curn.Port.SwitchUuid ||
						a.Port.PortgroupKey != curn.Port.SwitchUuid {
						fmt.Println("Skipping as network already connected ", currNW)
						continue
					}
				}

			case *types.VirtualEthernetCardNetworkBackingInfo:
				if veth.Backing.(*types.VirtualEthernetCardNetworkBackingInfo).DeviceName != currNW {
					fmt.Println("Skipping as network already connected ", currNW)
					continue
				}
			}
			veth.Backing = &types.VirtualEthernetCardDistributedVirtualPortBackingInfo{
				VirtualDeviceBackingInfo: types.VirtualDeviceBackingInfo{},
				Port: types.DistributedVirtualSwitchPortConnection{
					SwitchUuid:   nw.Port.SwitchUuid,
					PortKey:      nw.Port.PortKey,
					PortgroupKey: nw.Port.PortgroupKey,
				},
			}
		}

		fmt.Println("Configuring network...", newNW)
		task, err = vm.vm.Reconfigure(vm.entity.Ctx(),
			types.VirtualMachineConfigSpec{DeviceChange: []types.BaseVirtualDeviceConfigSpec{&types.VirtualDeviceConfigSpec{Operation: types.VirtualDeviceConfigSpecOperationEdit, Device: d}}})
		if err != nil {
			return err
		}
		if err := task.Wait(vm.entity.Ctx()); err != nil {
			return errors.Wrap(err, "Reconfiguring to network failed")
		}

		reconfigs++
		if maxReconfigs != 0 && reconfigs == maxReconfigs {
			break
		}
	}

	return nil
}

//ReadMacs gets the mac address
func (vm *VM) ReadMacs() ([]string, error) {

	macs := []string{}
	// device name:network name
	property.Wait(vm.entity.Ctx(), vm.entity.ConnCtx.pc, vm.vm.Reference(), []string{"config.hardware.device"}, func(pc []types.PropertyChange) bool {
		for _, c := range pc {
			//if c.Op != types.PropertyChangeOpAssign {
			//continue
			//}

			changedDevices := c.Val.(types.ArrayOfVirtualDevice).VirtualDevice
			for _, device := range changedDevices {
				if nic, ok := device.(types.BaseVirtualEthernetCard); ok {
					mac := nic.GetVirtualEthernetCard().MacAddress
					if mac == "" {
						continue
					}
					macs = append(macs, mac)
				}
			}
		}
		return true
	})

	return macs, nil
}
