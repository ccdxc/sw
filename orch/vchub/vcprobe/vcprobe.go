package vcprobe

import (
	"context"
	"errors"
	"net/url"
	"reflect"

	log "github.com/Sirupsen/logrus"
	"github.com/davecgh/go-spew/spew"
	"github.com/pensando/sw/orch/vchub/defs"
	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/property"
	"github.com/vmware/govmomi/view"
	"github.com/vmware/govmomi/vim25/types"
)

// VCProbe represents an instance of a vCenter interface
type VCProbe struct {
	vcURL      *url.URL
	vcID       string
	client     *govmomi.Client
	viewMgr    *view.Manager
	cancel     context.CancelFunc
	ctx        context.Context
	hostOutBox chan<- defs.HostMsg
}

// NewVCProbe returns a new instance of VCProbe
func NewVCProbe(vcURL *url.URL, hOutBox chan<- defs.HostMsg) *VCProbe {
	return &VCProbe{
		vcURL:      vcURL,
		hostOutBox: hOutBox,
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
	v.lookForSmartNICs()
}

// IsNetworkDevice checks if the base virtual device is a vnic
func IsNetworkDevice(d types.BaseVirtualDevice) bool {
	dKind := reflect.TypeOf(d).Elem()

	switch dKind {
	case reflect.TypeOf((*types.VirtualVmxnet3)(nil)).Elem():
		return true
	case reflect.TypeOf((*types.VirtualVmxnet2)(nil)).Elem():
		return true
	case reflect.TypeOf((*types.VirtualVmxnet)(nil)).Elem():
		return true
	case reflect.TypeOf((*types.VirtualE1000)(nil)).Elem():
		return true
	case reflect.TypeOf((*types.VirtualE1000e)(nil)).Elem():
		return true
	default:
		return false
	}
}

// lookForSmartNICs probes the vCenter for SmartNICs
func (v *VCProbe) lookForSmartNICs() error {
	root := v.client.ServiceContent.RootFolder
	kinds := []string{"HostSystem"}
	hostView, err := v.viewMgr.CreateContainerView(v.ctx, root, kinds, true)
	if err != nil {
		log.Errorf("CreateContainerView returned %v", err)
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

	err = property.WaitForView(v.ctx, property.DefaultCollector(v.client.Client), hostView.Reference(), hostRef, hostProps, updFunc)

	log.Infof("vchub.vcprobe lookForSmartNICs property.WaitForView exited")
	if err != nil {
		log.Errorf("property.WaitForView returned %v", err)
	}

	return err
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
	msg := defs.HostMsg{
		Op:         defs.VCOpSet,
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
			vhNew.PenNICs[pnic.Device] = &defs.NICInfo{Mac: pnic.Mac}
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

	v.hostOutBox <- msg
}
