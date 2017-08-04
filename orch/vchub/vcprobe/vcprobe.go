package vcprobe

import (
	"context"
	"errors"
	"net/url"
	"reflect"
	"time"

	log "github.com/Sirupsen/logrus"
	"github.com/davecgh/go-spew/spew"
	"github.com/pensando/sw/orch/vchub/defs"
	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/property"
	"github.com/vmware/govmomi/view"
	"github.com/vmware/govmomi/vim25/types"
)

const (
	retryDelay = time.Second
)

// VCProbe represents an instance of a vCenter Interface
type VCProbe struct {
	vcURL   *url.URL
	vcID    string
	client  *govmomi.Client
	viewMgr *view.Manager
	cancel  context.CancelFunc
	ctx     context.Context
	outBox  chan<- defs.StoreMsg
}

// NewVCProbe returns a new instance of VCProbe
func NewVCProbe(vcURL *url.URL, hOutBox chan<- defs.StoreMsg) *VCProbe {
	return &VCProbe{
		vcURL:  vcURL,
		outBox: hOutBox,
	}
}

// Start creates a client and view manager
func (v *VCProbe) Start() error {
	if v.cancel != nil {
		return errors.New("Already started")
	}
	v.ctx, v.cancel = context.WithCancel(context.Background())
	// Connect and log in to vCenter
	c, err := govmomi.NewClient(v.ctx, v.vcURL, true)
	if err != nil {
		log.Errorf("Log in failed: %v", err)
		return err
	}

	v.client = c
	v.viewMgr = view.NewManager(v.client.Client)
	v.vcID = v.vcURL.Hostname() + ":" + v.vcURL.Port()
	return nil
}

// Stop stops the sessions
func (v *VCProbe) Stop() {
	if v.cancel != nil {
		v.cancel()
		v.cancel = nil
	}
}

// Run runs the probe
func (v *VCProbe) Run() {
	go v.probeSmartNICs()
	go v.probeNwIFs()
}

// GetVeth checks if the base virtual device is a vnic and returns a pointer to
// VirtualEthernetCard
func GetVeth(d types.BaseVirtualDevice) *types.VirtualEthernetCard {
	dKind := reflect.TypeOf(d).Elem()

	switch dKind {
	case reflect.TypeOf((*types.VirtualVmxnet3)(nil)).Elem():
		v3 := d.(*types.VirtualVmxnet3)
		return &v3.VirtualVmxnet.VirtualEthernetCard
	case reflect.TypeOf((*types.VirtualVmxnet2)(nil)).Elem():
		v2 := d.(*types.VirtualVmxnet2)
		return &v2.VirtualVmxnet.VirtualEthernetCard
	case reflect.TypeOf((*types.VirtualVmxnet)(nil)).Elem():
		v1 := d.(*types.VirtualVmxnet)
		return &v1.VirtualEthernetCard
	case reflect.TypeOf((*types.VirtualE1000)(nil)).Elem():
		e1 := d.(*types.VirtualE1000)
		return &e1.VirtualEthernetCard
	case reflect.TypeOf((*types.VirtualE1000e)(nil)).Elem():
		e1e := d.(*types.VirtualE1000e)
		return &e1e.VirtualEthernetCard
	default:
		return nil
	}
}

// probeSmartNICs probes the vCenter for SmartNICs
func (v *VCProbe) probeSmartNICs() error {
	root := v.client.ServiceContent.RootFolder
	kinds := []string{"HostSystem"}
	hostView, err := v.viewMgr.CreateContainerView(v.ctx, root, kinds, true)
	if err != nil {
		log.Fatalf("CreateContainerView returned %v", err)
		return err
	}

	updFunc := func(c types.ManagedObjectReference, pc []types.PropertyChange) bool {
		if c.Type != "HostSystem" {
			log.Errorf("Expected HostSystem, got %+v", c)
			return false
		}
		hostKey := c.Value
		v.updateSNIC(hostKey, pc)

		return false
	}

	hostRef := types.ManagedObjectReference{Type: "HostSystem"}
	// for now, we watch only config. might add name and customValue in the future
	hostProps := []string{"config"}

	for {
		err = property.WaitForView(v.ctx, property.DefaultCollector(v.client.Client), hostView.Reference(), hostRef, hostProps, updFunc)

		if err != nil {
			log.Errorf("property.WaitForView returned %v", err)
		}

		if v.ctx.Err() != nil {
			return err
		}

		log.Infof("probeSmartNICs property.WaitForView exited, retrying...")
		time.Sleep(retryDelay)
	}

}

// updateSNIC is the callback that injects a message to the smartnic store
func (v *VCProbe) updateSNIC(hostKey string, pc []types.PropertyChange) {
	log.Infof("<== updateSNIC hostKey: %s ==>", hostKey)
	if len(pc) != 1 {
		log.Errorf("Only a single property expected at this time.")
		spew.Dump(pc)
		return
	}

	// build a vSphereHost based on the update
	vhNew := &defs.ESXHost{DvsMap: make(map[string]*defs.DvsInstance), PenNICs: make(map[string]*defs.NICInfo)}

	globalHostKey := v.vcID + ":" + hostKey
	msg := defs.StoreMsg{
		Op:         defs.VCOpSet,
		Property:   defs.HostPropConfig,
		Key:        globalHostKey,
		Value:      vhNew,
		Originator: v.vcID,
	}

	for _, prop := range pc {
		if prop.Op == types.PropertyChangeOpRemove || prop.Op == types.PropertyChangeOpIndirectRemove {
			msg.Op = defs.VCOpDelete
			break
		}
		hConfig, ok := prop.Val.(types.HostConfigInfo)
		if !ok {
			log.Errorf(">>>Bad prop<<<")
			spew.Dump(prop)
			return
		}

		nwInfo := hConfig.Network
		if nwInfo == nil {
			log.Errorf("Missing hConfig.Network")
			return
		}

		for _, pnic := range nwInfo.Pnic {
			// TODO check for vendor field to identify Pensando NICs
			vhNew.PenNICs[pnic.Device] = &defs.NICInfo{Mac: pnic.Mac, Name: pnic.Device}
		}

		for _, sx := range nwInfo.ProxySwitch {
			// TODO ignore if it's not our dvs
			if sx.Spec.Backing == nil {
				log.Errorf("DVS backing is nil")
				continue
			}

			backing, ok := sx.Spec.Backing.(*types.DistributedVirtualSwitchHostMemberPnicBacking)
			if !ok {
				log.Errorf("Expected DistributedVirtualSwitchHostMemberPnicBacking")
				continue
			}

			dvs := new(defs.DvsInstance)
			dvs.Name = sx.DvsName
			for _, ps := range backing.PnicSpec {
				if i, found := vhNew.PenNICs[ps.PnicDevice]; found {
					dvs.Uplinks = append(dvs.Uplinks, ps.PnicDevice)
					i.DvsUUID = sx.DvsUuid // updates map

				} else {
					// TODO: this should be an alarm
					log.Errorf("%s not recognized", ps.PnicDevice)
				}
			}
			vhNew.DvsMap[sx.DvsUuid] = dvs
		}
	}

	v.outBox <- msg
}

// probeNwIFs probes the vCenter for VNICs
func (v *VCProbe) probeNwIFs() error {
	root := v.client.ServiceContent.RootFolder
	kinds := []string{"VirtualMachine"}
	vmView, err := v.viewMgr.CreateContainerView(v.ctx, root, kinds, true)
	if err != nil {
		log.Fatalf("CreateContainerView returned %v", err)
		return err
	}

	updFunc := func(c types.ManagedObjectReference, pc []types.PropertyChange) bool {
		if c.Type != "VirtualMachine" {
			log.Errorf("Expected VirtualMachine, got %+v", c)
			return false
		}
		vmKey := c.Value
		v.updateNwIF(vmKey, pc)

		return false
	}

	vmRef := types.ManagedObjectReference{Type: "VirtualMachine"}
	vmProps := []string{"config", "name", "runtime", "tag", "customValue"}

	for {
		err = property.WaitForView(v.ctx, property.DefaultCollector(v.client.Client), vmView.Reference(), vmRef, vmProps, updFunc)

		if err != nil {
			log.Errorf("property.WaitForView returned %v", err)
		}

		if v.ctx.Err() != nil {
			return err
		}

		log.Infof("probeSmartNICs property.WaitForView exited, retrying...")
		time.Sleep(retryDelay)
	}

}

// updateNwIF is the callback that injects a message to the nwif store
func (v *VCProbe) updateNwIF(vmKey string, pc []types.PropertyChange) {
	var m defs.StoreMsg
	globalVMKey := v.vcID + ":" + vmKey
	log.Infof("<== updateNwIF vmKey: %s ==>", globalVMKey)
	//spew.Dump(pc)
	for _, prop := range pc {
		switch defs.VCProp(prop.Name) {
		case defs.VMPropConfig:
			op := getStoreOp(prop.Op)
			value := &defs.VMConfig{}
			if op != defs.VCOpDelete {
				vnics, err := getVirtualNICs(&prop)
				if err != nil {
					continue
				}
				value = &defs.VMConfig{Vnics: vnics}
			}
			m = defs.StoreMsg{
				Op:         getStoreOp(prop.Op),
				Property:   defs.VMPropConfig,
				Key:        globalVMKey,
				Value:      value,
				Originator: v.vcID,
			}
		case defs.VMPropName:
			m = defs.StoreMsg{
				Op:         getStoreOp(prop.Op),
				Property:   defs.VMPropName,
				Key:        globalVMKey,
				Value:      prop.Val.(string),
				Originator: v.vcID,
			}
		case defs.VMPropRT:
			rt := prop.Val.(types.VirtualMachineRuntimeInfo)
			m = defs.StoreMsg{
				Op:         getStoreOp(prop.Op),
				Property:   defs.VMPropRT,
				Key:        globalVMKey,
				Value:      &defs.VMRuntime{HostKey: v.vcID + ":" + rt.Host.Value, PowerState: string(rt.PowerState)},
				Originator: v.vcID,
			}
		//case defs.VMPropTag:
		//case defs.VMPropCustom:
		default:
			log.Errorf("Unknown property %s", prop.Name)
			continue
		}

		v.outBox <- m
	}
}

func getStoreOp(op types.PropertyChangeOp) defs.VCOp {
	switch op {
	case types.PropertyChangeOpRemove:
		return defs.VCOpDelete
	case types.PropertyChangeOpIndirectRemove:
		return defs.VCOpDelete
	default:
		return defs.VCOpSet
	}
}

// getVirtualNICs extracts vnic info from the property
func getVirtualNICs(p *types.PropertyChange) (map[string]*defs.VirtualNIC, error) {
	var name string
	res := make(map[string]*defs.VirtualNIC)

	vmConfig, ok := p.Val.(types.VirtualMachineConfigInfo)
	if !ok {
		log.Errorf("Expected VirtualMachineConfigInfo, got %s", reflect.TypeOf(p.Val).Name())
		return nil, errors.New("reflect error")
	}

	for _, d := range vmConfig.Hardware.Device {
		vec := GetVeth(d)
		if vec != nil {
			back, ok := vec.Backing.(*types.VirtualEthernetCardDistributedVirtualPortBackingInfo)
			if !ok {
				log.Errorf("Expected types.VirtualEthernetCardDistributedVirtualPortBackingInfo, got %s", reflect.TypeOf(vec.Backing).Name())
				continue
			}
			desc, ok := vec.VirtualDevice.DeviceInfo.(*types.Description)
			if ok {
				name = desc.Label
			} else {
				name = ""
			}
			vnic := &defs.VirtualNIC{
				Name:         name,
				MacAddress:   vec.MacAddress,
				PortgroupKey: back.Port.PortgroupKey,
				PortKey:      back.Port.PortKey,
				SwitchUUID:   back.Port.SwitchUuid,
			}

			res[vec.MacAddress] = vnic
		}
	}

	return res, nil
}
