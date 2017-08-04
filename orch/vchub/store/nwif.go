package store

import (
	"context"
	"reflect"
	"strconv"

	log "github.com/Sirupsen/logrus"

	swapi "github.com/pensando/sw/api"
	"github.com/pensando/sw/orch"
	"github.com/pensando/sw/orch/vchub/defs"
	"github.com/pensando/sw/utils/kvstore"
)

const (
	nwifPath = "/vchub/nwifs/"
	nwifKind = "NwIF"
)

// nwifStore maintains relevant information about nwif objects for
// the store instance. The information is maintained in terms of vSphere object
// properties, indexed by vm key.
type nwifStore struct {
	ctx        context.Context
	vmMap      map[string]*vmInfo
	host2VMs   map[string]map[string]bool // hostkey to set of vmkeys
	snicFinder func(string, string) string
}

type vmInfo struct {
	vmName    string
	vmConfig  *defs.VMConfig  // vm config property
	vmRunTime *defs.VMRuntime // power status etc
	attr      map[string]string
}

// NwIFCreate creates a NwIF object in the kv store
func NwIFCreate(ctx context.Context, ID string, n *orch.NwIF) error {
	key := nwifPath + ID

	//if the object already exists, compare and update only if there is a change.
	old := &orch.NwIF{}
	err := kvStore.Get(ctx, key, old)
	if err == nil {
		n.ObjectMeta.ResourceVersion = old.ObjectMeta.ResourceVersion
		if reflect.DeepEqual(old, n) {
			return nil
		}

		log.Infof("NwIFCreate: old %+v new %+v", old, n)

		return kvUpdate(ctx, key, n)
	}

	log.Infof("NwIFCreate: %+v", n)
	return kvCreate(ctx, key, n)
}

// NwIFDelete deletes a NwIF object from the kv store
func NwIFDelete(ctx context.Context, ID string) error {
	log.Infof("NwIFDelete %s", ID)
	key := nwifPath + ID
	return kvDelete(ctx, key)
}

// NwIFUpdate updates a NwIF object in the kv store
func NwIFUpdate(ctx context.Context, ID string, n *orch.NwIF) error {
	log.Infof("NwIFUpdate: %+v %+v", ID, n)
	key := nwifPath + ID
	return kvUpdate(ctx, key, n)
}

// NwIFWatchAll is used for watching all NwIF objects in the kv store
func NwIFWatchAll(ctx context.Context, refVersion string) (kvstore.Watcher, error) {
	return kvStore.PrefixWatch(ctx, nwifPath, refVersion)
}

// NwIFList lists all NwIF objects in the kv store
func NwIFList(ctx context.Context) (*orch.NwIFList, error) {
	list := orch.NwIFList{ListMeta: &swapi.ListMeta{}}
	err := kvStore.List(ctx, nwifPath, &list)
	return &list, err
}

func newNwifStore(ctx context.Context, f func(string, string) string) *nwifStore {
	return &nwifStore{
		ctx:        ctx,
		vmMap:      make(map[string]*vmInfo),
		host2VMs:   make(map[string]map[string]bool),
		snicFinder: f,
	}
}

func formNwIFKey(vmKey, mac string) string {
	return vmKey + "::" + mac
}

func (ns *nwifStore) setConfig(vmKey string, c *defs.VMConfig) {
	currVM := ns.vmMap[vmKey]
	if currVM == nil {
		currVM = nullvmInfo()
	}

	if reflect.DeepEqual(currVM.vmConfig, c) {
		return // no change
	}

	// create all NwIF objects

	for _, vnic := range c.Vnics {
		snicID := ns.snicFinder(currVM.vmRunTime.HostKey, vnic.SwitchUUID)
		k := formNwIFKey(vmKey, vnic.MacAddress)
		nwif := &orch.NwIF{
			ObjectKind:       nwifKind,
			ObjectAPIVersion: "v1",
			ObjectMeta:       &swapi.ObjectMeta{UUID: k},
			Config:           &orch.NwIF_Config{LocalVLAN: getLocalVLAN(vnic)},
			Status: &orch.NwIF_Status{
				MacAddress: vnic.MacAddress,
				// TODO: change this to PG Name
				PortGroup:   vnic.PortgroupKey,
				Switch:      vnic.SwitchUUID,
				SmartNIC_ID: snicID,
			},
			Attributes: currVM.attr,
		}

		err := NwIFCreate(ns.ctx, k, nwif)
		if err != nil {
			log.Errorf("NwIFCreate returned %v", err)
		}
	}

	// delete any old ones that are not present now
	if currVM.vmConfig != nil {
		for m := range currVM.vmConfig.Vnics {
			_, found := c.Vnics[m]
			if !found {
				err := NwIFDelete(ns.ctx, formNwIFKey(vmKey, m))

				if err != nil {
					log.Errorf("NwIFDelete returned %v", err)
				}
			}
		}
	}

	// save the new data
	currVM.vmConfig = c
	ns.vmMap[vmKey] = currVM
}

func (ns *nwifStore) deleteConfig(vmKey string) {
	currVM := ns.vmMap[vmKey]

	if currVM != nil && currVM.vmConfig != nil {
		for m := range currVM.vmConfig.Vnics {
			err := NwIFDelete(ns.ctx, formNwIFKey(vmKey, m))
			if err != nil {
				log.Errorf("NwIFDelete returned %v", err)
			}
		}
	}

	delete(ns.vmMap, vmKey)
	if currVM != nil {
		oldHostKey := currVM.vmRunTime.HostKey
		h2vm := ns.host2VMs[oldHostKey]
		if h2vm != nil {
			delete(h2vm, vmKey)
		}
	}
}

func (ns *nwifStore) setRuntime(vmKey string, r *defs.VMRuntime) {
	currVM := ns.vmMap[vmKey]
	if currVM == nil {
		currVM = nullvmInfo()
	}

	if reflect.DeepEqual(currVM.vmRunTime, r) {
		return // no change
	}

	oldHostKey := currVM.vmRunTime.HostKey
	currVM.vmRunTime = r
	// create all NwIF objects
	ns.createVMNwIFs(vmKey, currVM)

	// save the new data
	ns.vmMap[vmKey] = currVM
	h2vm := ns.host2VMs[r.HostKey]
	if h2vm == nil {
		h2vm = make(map[string]bool)
	}

	h2vm[vmKey] = true
	ns.host2VMs[r.HostKey] = h2vm

	if oldHostKey != r.HostKey {
		h2vm = ns.host2VMs[oldHostKey]
		if h2vm != nil {
			delete(h2vm, vmKey)
		}
	}
}

func getLocalVLAN(vnic *defs.VirtualNIC) int32 {

	vlan, err := strconv.Atoi(vnic.PortKey)
	if err != nil {
		vlan = 0
	}

	return int32(vlan)

}

func (ns *nwifStore) processVMConfig(op defs.VCOp, key string, val *defs.VMConfig) {
	switch op {
	case defs.VCOpSet:
		ns.setConfig(key, val)
	case defs.VCOpDelete:
		ns.deleteConfig(key)
	}
}

func (ns *nwifStore) processVMRunTime(op defs.VCOp, key string, r *defs.VMRuntime) {
	switch op {
	case defs.VCOpSet:
		ns.setRuntime(key, r)
		// delete is only handled for config
	}
}

func (ns *nwifStore) processVMName(op defs.VCOp, key string, n string) {
	switch op {
	case defs.VCOpSet:
		currVM := ns.vmMap[key]
		if currVM == nil {
			currVM = nullvmInfo()
		}

		if currVM.vmName == n {
			return // no change
		}

		currVM.vmName = n
		// create all NwIF objects
		ns.createVMNwIFs(key, currVM)
		ns.vmMap[key] = currVM
		// delete is only handled for config
	}
}

func (ns *nwifStore) createVMNwIFs(vmKey string, vm *vmInfo) {
	// create all NwIF objects
	for _, vnic := range vm.vmConfig.Vnics {
		snicID := ns.snicFinder(vm.vmRunTime.HostKey, vnic.SwitchUUID)
		k := formNwIFKey(vmKey, vnic.MacAddress)
		nwif := &orch.NwIF{
			ObjectKind:       nwifKind,
			ObjectAPIVersion: "v1",
			ObjectMeta:       &swapi.ObjectMeta{UUID: k},
			Config:           &orch.NwIF_Config{LocalVLAN: getLocalVLAN(vnic)},
			Status: &orch.NwIF_Status{
				MacAddress: vnic.MacAddress,
				// TODO: change this to PG Name
				PortGroup:   vnic.PortgroupKey,
				Switch:      vnic.SwitchUUID,
				SmartNIC_ID: snicID,
			},
			Attributes: vm.attr,
		}

		err := NwIFCreate(ns.ctx, k, nwif)
		if err != nil {
			log.Errorf("NwIFCreate returned %v", err)
		}
	}
}

// hostUpdate handle host changes
func (ns *nwifStore) hostUpdate(hostKey string) {
	vms := ns.host2VMs[hostKey]
	for vmKey := range vms {
		vmInfo := ns.vmMap[vmKey]
		if vmInfo != nil {
			ns.createVMNwIFs(vmKey, vmInfo)
		}
	}
}

func nullvmInfo() *vmInfo {
	return &vmInfo{
		vmConfig: &defs.VMConfig{
			Vnics: make(map[string]*defs.VirtualNIC),
		},
		vmRunTime: &defs.VMRuntime{},
		attr:      make(map[string]string),
	}
}
