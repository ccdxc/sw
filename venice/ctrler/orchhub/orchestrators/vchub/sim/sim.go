package sim

import (
	"context"
	"fmt"
	"io/ioutil"
	"net/url"
	"os"
	"path"

	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/simulator"
	"github.com/vmware/govmomi/simulator/vpx"
	// Set up simulator rest api
	_ "github.com/vmware/govmomi/vapi/simulator"
	"github.com/vmware/govmomi/vim25"
	"github.com/vmware/govmomi/vim25/soap"
	"github.com/vmware/govmomi/vim25/types"
)

// Datacenter contains info of a simulator DC instance
type Datacenter struct {
	obj       *simulator.Datacenter
	client    *vim25.Client
	hostMap   map[string]*Host
	datastore datastore
}

// Datacenter contains info of a simulator DC instance
type datastore struct {
	name string
	dir  string
}

// Host contains info of a simulator host instance
type Host struct {
	obj *simulator.HostSystem
}

// Config specifies configuration for a VcSim instance
type Config struct {
	Addr string
}

// VcSim is the struct for an instance
type VcSim struct {
	model   simulator.Model
	service *simulator.Service
	client  *vim25.Client
	dcMap   map[string]*Datacenter
	Server  *simulator.Server
	URL     *url.URL
	dirs    []string
}

// NewVcSim creates a new VcSim instance
func NewVcSim(config Config) (*VcSim, error) {
	s := simulator.NewServiceInstance(vpx.ServiceContent, vpx.RootFolder)
	Service := simulator.New(s)
	Service.RegisterEndpoints = true

	client, err := vim25.NewClient(context.Background(), Service)
	if err != nil {
		return nil, err
	}
	if u, err := soap.ParseURL(config.Addr); err == nil {
		Service.Listen = u
	}

	server := Service.NewServer()

	return &VcSim{
		service: Service,
		URL:     server.URL,
		client:  client,
		Server:  server,
		dcMap:   make(map[string]*Datacenter),
		dirs:    []string{},
	}, nil
}

// Destroy tears down the simulator
func (v *VcSim) Destroy() {
	for _, dir := range v.dirs {
		_ = os.RemoveAll(dir)
	}
	v.Server.Close()
}

// GetURL returns the url the simulator is running on
func (v *VcSim) GetURL() *url.URL {
	return v.URL
}

// GetDC returns the DC by name
func (v *VcSim) GetDC(name string) (*Datacenter, bool) {
	entry, ok := v.dcMap[name]
	return entry, ok
}

// AddDC adds a new datacenter with the given name
func (v *VcSim) AddDC(name string) (*Datacenter, error) {
	folder := simulator.Map.Get(vpx.RootFolder.Reference()).(*simulator.Folder)
	dc := simulator.NewDatacenter(folder)
	taskErr := simulator.RenameTask(dc, &types.Rename_Task{
		This:    dc.Reference(),
		NewName: name,
	})
	if taskErr.Fault() != nil {
		return nil, fmt.Errorf("Failed to rename the DC. Err: %+v", taskErr.Fault())
	}

	// Create directory for local datastore.
	// When a host is added in this DC, it will be assigned this datastore
	datastoreName := fmt.Sprintf("LocalDS_%s", name)
	dir, err := ioutil.TempDir("", fmt.Sprintf("govcsim-%s", datastoreName))
	if err != nil {
		return nil, fmt.Errorf("Failed to create the local datastore. Err: %s", err.Error())
	}
	v.dirs = append(v.dirs, dir)

	entry := &Datacenter{
		obj:     dc,
		client:  v.client,
		hostMap: make(map[string]*Host),
		datastore: datastore{
			name: datastoreName,
			dir:  dir,
		},
	}
	v.dcMap[name] = entry

	return entry, nil
}

// AddHost adds a host to the DC
func (v *Datacenter) AddHost(name string) (*Host, error) {
	spec := types.HostConnectSpec{
		HostName: name,
	}
	folder := simulator.Map.Get(v.obj.HostFolder.Reference()).(*simulator.Folder)
	host, _ := simulator.CreateStandaloneHost(folder, spec)

	entry := &Host{
		obj: host,
	}

	v.hostMap[name] = entry

	hostObj := object.NewHostSystem(v.client, host.Reference())
	ctx := context.Background()
	dss, err := hostObj.ConfigManager().DatastoreSystem(ctx)
	if err != nil {
		return nil, err
	}
	_, err = dss.CreateLocalDatastore(ctx, v.datastore.name, v.datastore.dir)
	if err != nil {
		return nil, fmt.Errorf("Failed to create local datastore for the host. %s", err.Error())
	}

	return entry, nil
}

// GetHost returns the host by name
func (v *Datacenter) GetHost(name string) (*Host, bool) {
	entry, ok := v.hostMap[name]
	return entry, ok
}

// AddNic adds a nic the host
func (v *Host) AddNic(name string, mac string) error {
	key := "key-vim.host.PhysicalNic-" + name
	pnic := types.PhysicalNic{Key: key, Device: name, Mac: mac}
	v.obj.Config.Network.Pnic = append(v.obj.Config.Network.Pnic, pnic)

	h := simulator.Map.Get(v.obj.Reference())
	simulator.Map.Update(h, []types.PropertyChange{
		{Name: "config", Val: v.obj.Config},
	})

	return nil
}

// RemoveNic removes the nic from the host
func (v *Host) RemoveNic(name string) {
	match := -1

	for ix, p := range v.obj.Config.Network.Pnic {
		if p.Device == name {
			match = ix
			break
		}
	}

	if match != -1 {
		v.obj.Config.Network.Pnic = append(v.obj.Config.Network.Pnic[:match], v.obj.Config.Network.Pnic[match+1:]...)
	}

	h := simulator.Map.Get(v.obj.Reference())
	simulator.Map.Update(h, []types.PropertyChange{
		{Name: "config", Val: v.obj.Config},
	})
}

// AddVM creates a VM with the given display name on the given host
func (v *Datacenter) AddVM(name string, hostName string) (*simulator.VirtualMachine, error) {
	// TODO: take in vnic config
	config := types.VirtualMachineConfigSpec{
		Name:    name,
		GuestId: string(types.VirtualMachineGuestOsIdentifierOtherGuest),
		Files: &types.VirtualMachineFileInfo{
			VmPathName: fmt.Sprintf("[%s]", v.datastore.name),
		},
	}

	// Adding some default devices
	var devices object.VirtualDeviceList

	scsi, err := devices.CreateSCSIController("pvscsi")
	if err != nil {
		return nil, err
	}
	ide, err := devices.CreateIDEController()
	if err != nil {
		return nil, err
	}
	cdrom, err := devices.CreateCdrom(ide.(*types.VirtualIDEController))
	if err != nil {
		return nil, err
	}
	disk := devices.CreateDisk(scsi.(types.BaseVirtualController), types.ManagedObjectReference{},
		config.Files.VmPathName+" "+path.Join(name, "disk1.vmdk"))
	disk.CapacityInKB = 1024

	// Create a vnic
	Backing := &types.VirtualEthernetCardDistributedVirtualPortBackingInfo{
		VirtualDeviceBackingInfo: types.VirtualDeviceBackingInfo{},
		Port: types.DistributedVirtualSwitchPortConnection{
			PortKey: "10",
			// TODO: fill out switchUUID and portgroupkey
		},
	}

	vnicDevice, err := devices.CreateEthernetCard("e1000", Backing)
	if err != nil {
		return nil, err
	}
	devices = append(devices, scsi, cdrom, disk, vnicDevice)

	config.DeviceChange, _ = devices.ConfigSpec(types.VirtualDeviceConfigSpecOperationAdd)

	return v.AddVMWithSpec(name, hostName, config)
}

// AddVMWithSpec creates a vm on host hostName with the given spec
func (v *Datacenter) AddVMWithSpec(name string, hostName string, spec types.VirtualMachineConfigSpec) (*simulator.VirtualMachine, error) {
	ctx := context.Background()
	host := v.hostMap[hostName]

	hostObj := object.NewHostSystem(v.client, host.obj.Reference())
	pool, err := hostObj.ResourcePool(ctx)
	if err != nil {
		return nil, err
	}

	dc := object.NewDatacenter(v.client, v.obj.Reference())
	folders, err := dc.Folders(ctx)
	if err != nil {
		return nil, err
	}

	task, err := folders.VmFolder.CreateVM(ctx, spec, pool, hostObj)
	if err != nil {
		return nil, err
	}
	err = task.Wait(ctx)
	if err != nil {
		return nil, err
	}

	vms := simulator.Map.All("VirtualMachine")
	for _, vm := range vms {
		if vm.Entity().Name == name {
			ret := vm.(*simulator.VirtualMachine)
			return ret, nil
		}
	}

	return nil, fmt.Errorf("VM create was successful but couldn't be found in inventory")
}
