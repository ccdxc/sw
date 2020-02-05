package vmware

import (
	"context"
	"fmt"
	"io/ioutil"
	"net/url"
	"os"
	"path/filepath"
	"sync"
	"time"

	"github.com/vmware/govmomi/view"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pkg/errors"
	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/ovf"
	"github.com/vmware/govmomi/property"
	"github.com/vmware/govmomi/vim25/soap"
	"github.com/vmware/govmomi/vim25/types"
	//"github.com/vmware/govmomi/ovf"
)

// TLSVerify turns on TLS verification.
var TLSVerify = false

// Client encapsulates all we ask of the govmomi client; to be used in-between
// different data types. It belongs, however, to the host and the host controls
// it.
type Client struct {
	*govmomi.Client
}

// Context is a top-level context.Context wrapper for controlling actions to
// the host at a global level.
type Context struct {
	context    context.Context
	cancelFunc context.CancelFunc
}

//ConnectionCtx properties associated to connection
type ConnectionCtx struct {
	client  *Client
	context *Context
	selfCtx *Context
	finder  *find.Finder
	viewMgr *view.Manager
	pc      *property.Collector
}

//Entity common
type Entity struct {
	Name string
	URL  *url.URL

	License string

	ConnCtx       ConnectionCtx
	clientLock    sync.RWMutex
	sessionActive bool
}

func (e *Entity) Ctx() context.Context {
	return e.ConnCtx.context.context
}

func (e *Entity) Client() *govmomi.Client {
	return e.ConnCtx.client.Client
}

func (e *Entity) Finder() *find.Finder {
	return e.ConnCtx.finder
}

func (e *Entity) IsVcenter() bool {
	return e.ConnCtx.client.Client.IsVC()
}

func (e *Entity) getClientWithRLock() {
	e.clientLock.RLock()
}

func (e *Entity) releaseClientRLock() {
	e.clientLock.RUnlock()
}

//EntityIntf common interface for all vmware operations
type EntityIntf interface {
	NewVM(name string) (*VM, error)
	DestoryVM(name string) error
	VMExists(name string) bool
	DeployVM(clusterName string, hostname string,
		name string, ncpus uint, memory uint, networks []string, ovfDir string) (*VMInfo, error)
	BootVM(name string) (*VMInfo, error)
	FindDvsPortGroup(name string, mcriteria DvsPGMatchCriteria) (string, error)
	AddPortGroupToDvs(name string, pairs []DvsPortGroup) error
	AddPvlanPairsToDvs(name string, pairs []DvsPvlanPair) error
	AddKernelNic(cluster, host string, pgName string, enableVmotion bool) error
	RemoveKernelNic(cluster, host string, pgName string) error
}

//Reinit vcenter
func (vc *Entity) Reinit(ctx context.Context) error {

	connCtx := &vc.ConnCtx
	if connCtx.selfCtx != nil && connCtx.selfCtx.context.Err() == nil {
		connCtx.selfCtx.cancelFunc()
		//Give it some time for current context to cancel
		time.Sleep(3 * time.Second)
	}

	newCtx, canceFunc := context.WithCancel(ctx)
	connCtx.context = &Context{context: ctx}
	connCtx.selfCtx = &Context{context: newCtx, cancelFunc: canceFunc}

	vc.clientLock.Lock()
	defer vc.clientLock.Unlock()
	if err := vc.connect(); err != nil {
		return errors.Wrapf(err, "Reinit Connection failed")
	}

	vc.sessionActive = true

	return nil

}

func (vc *Entity) Active() bool {
	return vc.sessionActive
}

func (vc *Entity) monitorSession() {

	acquiredLock := false
	log.Infof("Starting session monitoring for %v", vc.Name)
	defer log.Infof("Exiting session monitoring for %v", vc.Name)
	defer func() {
		if acquiredLock {
			vc.clientLock.Unlock()
		}
	}()
	for true {
		if vc.ConnCtx.context.context.Err() != nil {
			log.Infof("Entity context cancelled, exiting %v", vc.Name)
			return
		}
		if vc.ConnCtx.selfCtx.context.Err() != nil {
			log.Infof("Self Entity context cancelled, exiting %v", vc.Name)
			return
		}
		vc.sessionActive, _ = vc.ConnCtx.client.SessionManager.SessionIsActive(vc.ConnCtx.context.context)
		if !vc.sessionActive {
			if !acquiredLock {
				vc.clientLock.Lock()
				acquiredLock = true
			}
			log.Errorf("Entity %v disconnected, trying reconnect", vc.Name)
			if err := vc.connect(); err == nil {
				//Release it only after successful connect
				vc.sessionActive = true
				vc.clientLock.Unlock()
				log.Errorf("Entity %v reconnected", vc.Name)
				acquiredLock = false
			}
		}

		time.Sleep(2 * time.Second)
	}
}

func (vc *Entity) connect() error {
	client, err := govmomi.NewClient(vc.ConnCtx.context.context, vc.URL, !TLSVerify)
	if err != nil {
		log.Errorf("Connection to enitity %v failed", vc.Name)
		return err
	}
	gclient := &Client{client}
	vc.ConnCtx.client = gclient
	vc.ConnCtx.pc = property.DefaultCollector(gclient.Client.Client)
	vc.ConnCtx.viewMgr = view.NewManager(gclient.Client.Client)
	vc.ConnCtx.finder = find.NewFinder(vc.Client().Client, false)
	log.Infof("Connected to Entity %v successfully", vc.Name)
	return nil
}
func (entity *Entity) getNWMap(ovfDir string, networks []string) ([]types.OvfNetworkMapping, error) {
	var nm []types.OvfNetworkMapping

	if len(networks) <= 1 {
		// use default networking
		return nm, nil
	}

	file, err := getOvf(ovfDir)
	if err != nil {
		return nil, err
	}

	r, err := os.Open(file)
	if err != nil {
		return nil, err
	}

	defer r.Close()
	e, err := ovf.Unmarshal(r)
	if err != nil {
		return nil, fmt.Errorf("failed to parse ovf: %s", err.Error())
	}

	if e.Network == nil || len(e.Network.Networks) <= 1 {
		// single network on the ovf, use default networking
		return nm, nil
	}

	if len(e.Network.Networks) < len(networks) {
		return nil, fmt.Errorf("ovf has only %d networks, user specified %d networks", len(e.Network.Networks), len(networks))
	}

	for ix, ovfNet := range e.Network.Networks {
		if ix >= len(networks) {
			break
		}

		hostNet := networks[ix]
		if netObj, err := entity.ConnCtx.finder.Network(entity.Ctx(),
			hostNet); err == nil {
			nm = append(nm, types.OvfNetworkMapping{
				Name:    ovfNet.Name,
				Network: netObj.Reference(),
			})
		} else {
			return nil, fmt.Errorf("Network %s not found - %v", hostNet, err)
		}
	}

	return nm, nil
}

func (entity *Entity) getImportSpec(rp *object.ResourcePool, ds *Datastore, name string,
	ovfDir string, networks []string) (*types.OvfCreateImportSpecResult, error) {
	ovfSrc, err := getOvfSrc(ovfDir)
	if err != nil {
		return nil, err
	}

	nwMap, err := entity.getNWMap(ovfDir, networks)
	if err != nil {
		return nil, err
	}

	cisp := types.OvfCreateImportSpecParams{
		DiskProvisioning: "thin",
		EntityName:       name,
		NetworkMapping:   nwMap,
	}

	mgr := ovf.NewManager(entity.ConnCtx.client.Client.Client)
	spec, err := mgr.CreateImportSpec(entity.Ctx(), string(ovfSrc), rp, ds.datastore.Reference(), cisp)
	if err != nil {
		return nil, err
	}
	if spec.Error != nil {
		return nil, errors.New(spec.Error[0].LocalizedMessage)
	}
	if spec.Warning != nil {
		for _, w := range spec.Warning {
			fmt.Printf("Warning : %s", w.LocalizedMessage)
		}
	}

	return spec, nil
}

// Datastore returns a *Datastore which allows you to perform operations on it.
func (entity *Entity) Datastore(name string) (*Datastore, error) {
	finder, dc, err := entity.ConnCtx.client.finder()
	if err != nil {
		return nil, err
	}

	var ds *object.Datastore
	if name == "" {
		ds, err = finder.DefaultDatastore(entity.Ctx())

	} else {
		ds, err = finder.Datastore(entity.Ctx(), name)
	}
	if err != nil {
		return nil, err
	}

	return &Datastore{
		datastore:  ds,
		datacenter: dc,
		Entity:     entity,
		//client:     entity.client,
		//context:    entity.context,
	}, nil
}

func (dc *DataCenter) getClientWithRLock() {
	dc.vc.clientLock.RLock()
}

func (dc *DataCenter) releaseClientRLock() {
	dc.vc.clientLock.RUnlock()
}

func (entity *Entity) importVapp(dir string, spec *types.OvfCreateImportSpecResult,
	rp *object.ResourcePool, host *object.HostSystem, folder *object.Folder) (retErr error) {

	importctx, cancel := context.WithTimeout(entity.Ctx(), 30*time.Minute)
	defer cancel()

	lease, err := rp.ImportVApp(importctx, spec.ImportSpec, folder, host)
	if err != nil {
		return err
	}

	info, err := lease.Wait(importctx, spec.FileItem)
	if err != nil {
		return err
	}

	u := lease.StartUpdater(importctx, info)
	defer u.Done()

	for _, item := range info.Items {
		file := item.Path

		f, err := os.Open(filepath.Join(dir, file))
		if err != nil {
			return err
		}
		defer f.Close()

		fi, err := f.Stat()
		if err != nil {
			return err
		}

		if err := lease.Upload(importctx, item, f, soap.Upload{ContentLength: fi.Size()}); err != nil {
			return err
		}
	}

	lease.Complete(importctx)

	return nil
}

func (entity *Entity) boot(name string, ncpus uint, memory uint) (*VMInfo, error) {
	vm, err := entity.ConnCtx.finder.VirtualMachine(entity.Ctx(), name)

	if err != nil {
		return nil, errors.Wrap(err, "Could not find VM")
	}

	if err = reconfigureVM(entity.Ctx(),
		vm, ncpus, memory); err != nil {
		return nil, errors.Wrap(err, "Reconfiguration failed")
	}

	task, err := vm.PowerOn(entity.Ctx())
	if err != nil {
		return nil, errors.Wrap(err, "Power on task start failed")
	}
	err = task.Wait(entity.Ctx())
	if err != nil {
		return nil, errors.Wrap(err, "Power on task failed")
	}

	var ip string
	ip, err = vm.WaitForIP(entity.Ctx())
	if err != nil {
		return nil, errors.Wrap(err, "Wait for IP failed")
	}

	return &VMInfo{Name: name, IP: ip}, nil // first arg is string, second is net.IP.
}

func (entity *Entity) makeVM(name string, vm *object.VirtualMachine) *VM {
	return &VM{
		entity: entity,
		name:   name,
		vm:     vm,
	}
}

// NewVM creates a new virtual machine
func (entity *Entity) NewVM(name string) (*VM, error) {
	entity.getClientWithRLock()
	defer entity.releaseClientRLock()
	vm, err := entity.Finder().VirtualMachine(entity.Ctx(), name)
	if err != nil {
		return nil, err
	}

	return entity.makeVM(name, vm), nil
}

func getOvf(dir string) (string, error) {
	files, err := filepath.Glob(filepath.Join(dir, "*.ovf"))
	if err != nil {
		return "", err
	}

	if len(files) == 0 {
		return "", errors.New("missing ovf file in archive")
	}

	if len(files) > 1 {
		return "", errors.New("more than one ovf in archive. giggity?")
	}

	return files[0], nil
}

func getOvfSrc(dir string) ([]byte, error) {
	file, err := getOvf(dir)
	if err != nil {
		return nil, err
	}
	return ioutil.ReadFile(file)
}

func (entity *Entity) wipeOut(name string) error {
	vm, err := entity.NewVM(name)
	if err != nil {
		return err
	}

	err = vm.Destroy()
	if err != nil {
		return err
	}

	return nil
}

// RemoveVM removes a vm
func (entity *Entity) RemoveVM(name string) error {
	entity.getClientWithRLock()
	defer entity.releaseClientRLock()
	return entity.wipeOut(name)
}

// DestoryVM destroys the vm
func (entity *Entity) DestoryVM(name string) error {
	entity.getClientWithRLock()
	defer entity.releaseClientRLock()
	return entity.RemoveVM(name)
}

// VMExists returns true if VM by the name exists
func (entity *Entity) VMExists(name string) bool {

	entity.getClientWithRLock()
	defer entity.releaseClientRLock()
	_, err := entity.Finder().VirtualMachine(entity.Ctx(), name)

	return err == nil
}

// BootVM botts up a new vm
func (entity *Entity) BootVM(name string) (*VMInfo, error) {
	entity.getClientWithRLock()
	defer entity.releaseClientRLock()
	vm, err := entity.Finder().VirtualMachine(entity.Ctx(), name)

	if err != nil {
		return nil, err
	}
	task, err := vm.PowerOn(entity.Ctx())
	if err != nil {
		return nil, err
	}
	err = task.Wait(entity.Ctx())
	if err != nil {
		return nil, err
	}

	var ip string
	ip, err = vm.WaitForIP(entity.Ctx())
	if err != nil {
		return nil, err
	}

	return &VMInfo{Name: name, IP: ip}, nil // first arg is string, second is net.IP.
}

func reconfigureVM(ctx context.Context, vm *object.VirtualMachine, ncpus uint, memory uint) error {
	bigMemory := int64(memory * 1024)

	var task *object.Task

	var err error

	t := true

	task, err = vm.Reconfigure(ctx, types.VirtualMachineConfigSpec{
		MemoryReservationLockedToMax: &t,
		NumCPUs:                      int32(ncpus),
		MemoryMB:                     bigMemory,
	})
	if err != nil {
		return err
	}

	return task.Wait(ctx)
}

// DeployVMOnDataStore deploys a VM on a datastore
func (entity *Entity) DeployVMOnDataStore(ds *Datastore,
	host *object.HostSystem, name string, ncpus uint, memory uint,
	networks []string, ovfDir string) (*VMInfo, error) {

	entity.getClientWithRLock()
	defer entity.releaseClientRLock()
	rp, err := host.ResourcePool(entity.Ctx())
	if err != nil {
		return nil, errors.Wrap(err, "Get resource pool failed")
	}

	spec, err := entity.getImportSpec(rp, ds, name, ovfDir, networks)
	if err != nil {
		return nil, errors.Wrap(err, "Get import spec failed")
	}

	folder, _ := entity.Finder().DefaultFolder(entity.Ctx())

	err = entity.importVapp(ovfDir, spec, rp, host, folder)
	if err != nil {
		return nil, errors.Wrap(err, "Import vapp failed")
	}

	//SLeep for a while before booting.
	time.Sleep(5 * time.Second)
	return entity.boot(name, ncpus, memory)
}

// CloneVM clone a the given vm
func (entity *Entity) cloneVMOnHost(host *object.HostSystem, name string, newName string,
	ncpus uint, memory uint) (*VMInfo, error) {

	vm, err := entity.Finder().VirtualMachine(entity.Ctx(), name)
	if err != nil {
		log.Fatal(err)
	}

	rp, err := host.ResourcePool(entity.Ctx())
	if err != nil {
		return nil, errors.Wrap(err, "Get resource pool failed")
	}

	ref := rp.Reference()

	spec := types.VirtualMachineCloneSpec{
		Location: types.VirtualMachineRelocateSpec{
			Pool: &ref,
		},
	}

	folder, _ := entity.Finder().DefaultFolder(entity.Ctx())

	task, err := vm.Clone(entity.Ctx(), folder, newName, spec)
	if err != nil {
		return nil, err
	}

	err = task.Wait(entity.Ctx())
	if err != nil {
		return nil, err
	}

	return entity.boot(newName, ncpus, memory)
}

//CloneVM deploys vm on data center
func (dc *DataCenter) CloneVM(clusterName string, hostname string,
	name string, newName string, ncpus uint, memory uint) (*VMInfo, error) {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	hostRef, err := dc.findHost(clusterName, hostname)
	if err != nil {
		return nil, err
	}

	return dc.vc.cloneVMOnHost(hostRef.hs, name, newName, ncpus, memory)
}

// AddVswitch adds vswitch to target host
func (entity *Entity) AddVswitch(vspec VswitchSpec) error {
	entity.getClientWithRLock()
	defer entity.releaseClientRLock()
	return nil
}

// AddNetworks adds the specified virtual networks
func (entity *Entity) AddNetworks(specs []NWSpec, vsSpec VswitchSpec) ([]string, error) {
	entity.getClientWithRLock()
	defer entity.releaseClientRLock()
	return nil, nil
}

// DeployVM deploys a vm
func (entity *Entity) DeployVM(clusterName string, hostname string,
	name string, ncpus uint, memory uint, networks []string, ovfDir string) (*VMInfo, error) {

	entity.getClientWithRLock()
	defer entity.releaseClientRLock()
	return nil, errors.New("Unimplemented method")
}
