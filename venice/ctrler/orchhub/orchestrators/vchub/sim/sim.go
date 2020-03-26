package sim

import (
	"context"
	"crypto/tls"
	"fmt"
	"io/ioutil"
	"net/url"
	"os"
	"path"

	"github.com/pensando/sw/venice/utils/ref"

	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/simulator"
	"github.com/vmware/govmomi/simulator/vpx"

	// Set up simulator rest api
	_ "github.com/vmware/govmomi/vapi/simulator"
	"github.com/vmware/govmomi/vim25"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/soap"
	"github.com/vmware/govmomi/vim25/types"
)

// Datacenter contains info of a simulator DC instance
type Datacenter struct {
	Obj       *simulator.Datacenter
	client    *vim25.Client
	dvsMap    map[string]*DVS
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
	client *vim25.Client
	Obj    *simulator.HostSystem
}

// DVS contains info of a simulator distributed virtual switch instance
// This struct is not expected to be parallely accessed
type DVS struct {
	client       *vim25.Client
	Obj          *simulator.DistributedVirtualSwitch
	portgroupMap map[string]*Portgroup
}

// Portgroup contains info of a simulator distributed virtual portgroup instance
type Portgroup struct {
	Obj *simulator.DistributedVirtualPortgroup
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

	Service.TLS = new(tls.Config)
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
	v.Server.CloseClientConnections()
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
		Obj:     dc,
		client:  v.client,
		dvsMap:  make(map[string]*DVS),
		hostMap: make(map[string]*Host),
		datastore: datastore{
			name: datastoreName,
			dir:  dir,
		},
	}
	v.dcMap[name] = entry

	return entry, nil
}

// Destroy removes the DC from the inventory
func (v *Datacenter) Destroy() error {
	dcObj := object.NewDatacenter(v.client, v.Obj.Reference())
	task, err := dcObj.Destroy(context.Background())
	if err != nil {
		return err
	}
	err = task.Wait(context.Background())
	return err
}

// GetDVS returns the distributed virtual switch by name
func (v *Datacenter) GetDVS(name string) (*DVS, bool) {
	entry, ok := v.dvsMap[name]
	if ok {
		return entry, ok
	}
	dvss := simulator.Map.All("DistributedVirtualSwitch")
	for _, dvs := range dvss {
		if dvs.Entity().Name == name {
			ret := &DVS{
				client:       v.client,
				Obj:          dvs.(*simulator.DistributedVirtualSwitch),
				portgroupMap: make(map[string]*Portgroup),
			}
			v.dvsMap[name] = ret
			return ret, true
		}
	}
	return entry, ok
}

// AddDVS adds a distributed virtual switch to the DC
func (v *Datacenter) AddDVS(dvsCreateSpec *types.DVSCreateSpec) (*DVS, error) {
	ctx := context.Background()

	dc := object.NewDatacenter(v.client, v.Obj.Reference())
	folders, err := dc.Folders(ctx)
	if err != nil {
		return nil, err
	}

	task, err := folders.NetworkFolder.CreateDVS(ctx, *dvsCreateSpec)
	if err != nil {
		return nil, err
	}

	err = task.Wait(ctx)
	if err != nil {
		return nil, err
	}

	dvss := simulator.Map.All("DistributedVirtualSwitch")
	for _, dvs := range dvss {
		if dvs.Entity().Name == dvsCreateSpec.ConfigSpec.GetDVSConfigSpec().Name {
			ret := &DVS{
				client:       v.client,
				Obj:          dvs.(*simulator.DistributedVirtualSwitch),
				portgroupMap: make(map[string]*Portgroup),
			}
			v.dvsMap[dvsCreateSpec.ConfigSpec.GetDVSConfigSpec().Name] = ret
			return ret, nil
		}
	}

	return nil, fmt.Errorf("DVS create was successful but couldn't be found in inventory")
}

// GetPortgroup returns the distributed virtual portgroup by name
// This is not thread-safe
func (v *DVS) GetPortgroup(name string) (*Portgroup, bool) {
	entry, ok := v.portgroupMap[name]
	return entry, ok
}

// AddPortgroup adds one or multiple distributed virtual portgroups to DVS
// This is not thread-safe
func (v *DVS) AddPortgroup(pgConfigSpec []types.DVPortgroupConfigSpec) ([]Portgroup, error) {
	// We need to count the uplink portgroup as well
	numPortgroup := len(pgConfigSpec) + 1
	ret := make([]Portgroup, numPortgroup)
	ctx := context.Background()

	dvs := object.NewDistributedVirtualSwitch(v.client, v.Obj.Reference())

	task, err := dvs.AddPortgroup(ctx, pgConfigSpec)
	if err != nil {
		return nil, err
	}

	_, err = task.WaitForResult(ctx, nil)
	if err != nil {
		return nil, err
	}

	pgs := simulator.Map.All("DistributedVirtualPortgroup")
	i := 0
	for _, pg := range pgs {
		if v.portgroupMap[pg.Entity().Name] == nil {
			ret[i].Obj = pg.(*simulator.DistributedVirtualPortgroup)
			v.portgroupMap[pg.Entity().Name] = &ret[i]
			i++
		}
	}

	return ret, nil
}

// AddHost adds a host to the DC
func (v *Datacenter) AddHost(name string) (*Host, error) {
	spec := types.HostConnectSpec{
		HostName: name,
	}
	folder := simulator.Map.Get(v.Obj.HostFolder.Reference()).(*simulator.Folder)
	h, _ := simulator.CreateStandaloneHost(folder, spec)

	// Make a deep-copy so that all Pnic and other objects are not shared across hosts
	host := ref.DeepCopy(*h).(simulator.HostSystem)

	entry := &Host{
		client: v.client,
		Obj:    &host,
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

// Destroy removes the host from the inventory
func (v *Host) Destroy() error {
	hostObj := object.NewHostSystem(v.client, v.Obj.Reference())
	task, err := hostObj.Destroy(context.Background())
	if err != nil {
		return err
	}
	err = task.Wait(context.Background())
	return err
}

// AddNic adds a nic the host
func (v *Host) AddNic(name string, mac string) error {
	key := "key-vim.host.PhysicalNic-" + name
	pnic := types.PhysicalNic{Key: key, Device: name, Mac: mac}
	v.Obj.Config.Network.Pnic = append(v.Obj.Config.Network.Pnic, pnic)

	h := simulator.Map.Get(v.Obj.Reference())
	simulator.Map.Update(h, []types.PropertyChange{
		{Name: "config", Val: v.Obj.Config},
	})

	return nil
}

// RemoveNic removes the nic from the host
func (v *Host) RemoveNic(name string) {
	match := -1

	for ix, p := range v.Obj.Config.Network.Pnic {
		if p.Device == name {
			match = ix
			break
		}
	}

	if match != -1 {
		v.Obj.Config.Network.Pnic = append(v.Obj.Config.Network.Pnic[:match], v.Obj.Config.Network.Pnic[match+1:]...)
	}

	h := simulator.Map.Get(v.Obj.Reference())
	simulator.Map.Update(h, []types.PropertyChange{
		{Name: "config", Val: v.Obj.Config},
	})
}

// ClearNics remove all pnics
func (v *Host) ClearNics() {
	v.Obj.Config.Network.Pnic = []types.PhysicalNic{}
	h := simulator.Map.Get(v.Obj.Reference())
	simulator.Map.Update(h, []types.PropertyChange{
		{Name: "config", Val: v.Obj.Config},
	})
}

// VNIC is a VMs vnic
type VNIC struct {
	MacAddress   string
	PortKey      string
	PortgroupKey string
}

// ClearVmkNics remove all vmknics
func (v *Host) ClearVmkNics() {
	v.Obj.Config.Network.Vnic = []types.HostVirtualNic{}
	h := simulator.Map.Get(v.Obj.Reference())
	simulator.Map.Update(h, []types.PropertyChange{
		{Name: "config", Val: v.Obj.Config},
	})
}

// AddVmkNic adds a Vmknic to the host
func (v *Host) AddVmkNic(spec *types.HostVirtualNicSpec, name string) error {
	key := "key-vim.host.VirtualNic-" + name
	vnic := types.HostVirtualNic{Key: key, Device: name, Spec: *spec}
	v.Obj.Config.Network.Vnic = append(v.Obj.Config.Network.Vnic, vnic)

	// fmt.Printf("Added vmkNic %s on %s host, total nics %d\n", name, v.Obj.Self.Value, len(v.Obj.Config.Network.Vnic))

	h := simulator.Map.Get(v.Obj.Reference())
	simulator.Map.Update(h, []types.PropertyChange{
		{Name: "config", Val: v.Obj.Config},
	})

	return nil
}

// RemoveVmkNic removes the nic from the host
func (v *Host) RemoveVmkNic(name string) {
	match := -1

	for ix, p := range v.Obj.Config.Network.Vnic {
		if p.Device == name {
			match = ix
			break
		}
	}

	if match != -1 {
		v.Obj.Config.Network.Vnic = append(v.Obj.Config.Network.Vnic[:match], v.Obj.Config.Network.Vnic[match+1:]...)
		// fmt.Printf("Removed vmkNic %s from %s host, total nics %d\n", name, v.Obj.Self.Value, len(v.Obj.Config.Network.Vnic))

		h := simulator.Map.Get(v.Obj.Reference())
		simulator.Map.Update(h, []types.PropertyChange{
			{Name: "config", Val: v.Obj.Config},
		})
	}
}

// AddVM creates a VM with the given display name on the given host
func (v *Datacenter) AddVM(name string, hostName string, vnics []VNIC) (*simulator.VirtualMachine, error) {
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

	devices = append(devices, scsi, cdrom, disk)

	// Create a vnic
	for _, vnic := range vnics {
		vnicDevice, err := createVnicDevice(vnic)
		if err != nil {
			return nil, err
		}
		devices = append(devices, vnicDevice)
	}

	config.DeviceChange, _ = devices.ConfigSpec(types.VirtualDeviceConfigSpecOperationAdd)

	return v.AddVMWithSpec(name, hostName, config)
}

// AddVMWithSpec creates a vm on host hostName with the given spec
func (v *Datacenter) AddVMWithSpec(name string, hostName string, spec types.VirtualMachineConfigSpec) (*simulator.VirtualMachine, error) {
	ctx := context.Background()
	host := v.hostMap[hostName]

	hostObj := object.NewHostSystem(v.client, host.Obj.Reference())
	pool, err := hostObj.ResourcePool(ctx)
	if err != nil {
		return nil, err
	}

	dc := object.NewDatacenter(v.client, v.Obj.Reference())
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

// DeleteVM removes the VM from the inventory
func (v *Datacenter) DeleteVM(vm *simulator.VirtualMachine) error {
	vmObj := object.NewVirtualMachine(v.client, vm.Reference())
	task, err := vmObj.Destroy(context.Background())
	if err != nil {
		return err
	}
	err = task.Wait(context.Background())
	return err
}

// AddVnic adds vnic info to the given vm
func (v *Datacenter) AddVnic(vmSim *simulator.VirtualMachine, vnic VNIC) error {
	vm := object.NewVirtualMachine(v.client, vmSim.Reference())
	vnicDevice, err := createVnicDevice(vnic)
	if err != nil {
		return err
	}
	err = vm.AddDevice(context.Background(), vnicDevice)

	return err
}

// RemoveVnic removes vnic info from the given vm
func (v *Datacenter) RemoveVnic(vmSim *simulator.VirtualMachine, vnic VNIC) error {
	vm := object.NewVirtualMachine(v.client, vmSim.Reference())
	var devices object.VirtualDeviceList
	// VirtualMachineConfigSpec
	config := types.VirtualMachineConfigSpec{}
	var vmMo mo.VirtualMachine
	err := vm.Properties(context.Background(), vmSim.Reference(), []string{"config"}, &vmMo)
	if err != nil {
		return err
	}
	// Find vnic with matching spec
	for _, d := range vmMo.Config.Hardware.Device {
		vnicCard, ok := d.(types.BaseVirtualEthernetCard)
		if ok && vnicCard.GetVirtualEthernetCard().MacAddress == vnic.MacAddress {
			// remove
			devices = append(devices, d)
		}
	}
	if len(devices) == 0 {
		return fmt.Errorf("No match for %s", vnic.MacAddress)
	}

	config.DeviceChange, _ = devices.ConfigSpec(types.VirtualDeviceConfigSpecOperationRemove)
	task, err := vm.Reconfigure(context.Background(), config)
	if err != nil {
		return err
	}

	err = task.Wait(context.Background())
	if err != nil {
		return err
	}

	return nil
}

func createVnicDevice(vnic VNIC) (types.BaseVirtualDevice, error) {
	var devices object.VirtualDeviceList
	Backing := &types.VirtualEthernetCardDistributedVirtualPortBackingInfo{
		VirtualDeviceBackingInfo: types.VirtualDeviceBackingInfo{},
		Port: types.DistributedVirtualSwitchPortConnection{
			PortKey:      vnic.PortKey,
			PortgroupKey: vnic.PortgroupKey,
		},
	}

	vnicDevice, err := devices.CreateEthernetCard("e1000", Backing)
	if err != nil {
		return nil, err
	}
	vnicDeviceCard := vnicDevice.(types.BaseVirtualEthernetCard)
	vnicDeviceCard.GetVirtualEthernetCard().MacAddress = vnic.MacAddress
	return vnicDevice, nil
}

// UpdateVMHost updates the host for the virtual machine
func (v *Datacenter) UpdateVMHost(vm *simulator.VirtualMachine, hostName string) error {
	vmMap := simulator.Map.Get(vm.Reference())
	host := v.hostMap[hostName]
	if host == nil {
		return fmt.Errorf("Host not found %s", hostName)
	}
	hRef := host.Obj.Reference()
	vm.Runtime.Host = &hRef
	simulator.Map.Update(vmMap, []types.PropertyChange{
		{Name: "runtime", Val: vm.Runtime},
	})
	return nil
}

// UpdateVMIP adds guest IP information
func (v *Datacenter) UpdateVMIP(vm *simulator.VirtualMachine, macAddr string, netName string, ips []string) error {
	vmMap := simulator.Map.Get(vm.Reference())
	nicInfo := vm.Guest.Net
	newItem := types.GuestNicInfo{
		MacAddress: macAddr,
		Network:    netName,
		IpAddress:  ips,
	}

	added := false
	for i, item := range vm.Guest.Net {
		if item.MacAddress == macAddr {
			added = true
			vm.Guest.Net[i] = newItem
			break
		}
	}

	if !added {
		nicInfo = append(nicInfo, newItem)
	}

	simulator.Map.Update(vmMap, []types.PropertyChange{
		{Name: "guest", Val: vm.Guest},
	})
	return nil
}

// RemoveVMIP removes guest IP information
func (v *Datacenter) RemoveVMIP(vm *simulator.VirtualMachine, macAddr string) error {
	vmMap := simulator.Map.Get(vm.Reference())

	indexToRemove := -1
	for i, item := range vm.Guest.Net {
		if item.MacAddress == macAddr {
			indexToRemove = i
			break
		}
	}

	if indexToRemove >= 0 {
		vm.Guest.Net = append(vm.Guest.Net[:indexToRemove], vm.Guest.Net[indexToRemove+1:]...)
		simulator.Map.Update(vmMap, []types.PropertyChange{
			{Name: "guest", Val: vm.Guest},
		})
	}

	return nil
}

// AddHost adds a host to the DVS
func (v *DVS) AddHost(host *Host) error {
	ref := host.Obj.Reference()
	for _, member := range v.Obj.Config.GetDVSConfigInfo().Host {
		if member.Config.Host.Value == ref.Value {
			return fmt.Errorf("Host already Added")
		}
	}
	newMember := types.DistributedVirtualSwitchHostMember{
		Config: types.DistributedVirtualSwitchHostMemberConfigInfo{
			Host: &ref,
		},
	}
	dvsProxy := types.HostProxySwitch{
		ConfigNumPorts: 512,
		DvsName:        v.Obj.Name,
	}
	// add all pnics as connected to dvs uplinks
	for _, pnic := range host.Obj.Config.Network.Pnic {
		dvsProxy.Pnic = append(dvsProxy.Pnic, pnic.Key)
	}
	host.Obj.Config.Network.ProxySwitch = append(host.Obj.Config.Network.ProxySwitch, dvsProxy)
	v.Obj.Config.GetDVSConfigInfo().Host = append(v.Obj.Config.GetDVSConfigInfo().Host, newMember)
	d := simulator.Map.Get(v.Obj.Reference())
	simulator.Map.Update(d, []types.PropertyChange{
		{Name: "config", Val: v.Obj.Config},
	})
	h := simulator.Map.Get(host.Obj.Reference())
	simulator.Map.Update(h, []types.PropertyChange{
		{Name: "config", Val: host.Obj.Config},
	})
	return nil
}

// RemoveHost remove a host from the DVS
func (v *DVS) RemoveHost(host *Host) error {
	ref := host.Obj.Reference()
	newDVSHostInfo := []types.DistributedVirtualSwitchHostMember{}
	for _, member := range v.Obj.Config.GetDVSConfigInfo().Host {
		if member.Config.Host.Value == ref.Value {
			continue
		}
		newDVSHostInfo = append(newDVSHostInfo, member)
	}
	newDVSProxy := []types.HostProxySwitch{}
	for _, proxySwitch := range host.Obj.Config.Network.ProxySwitch {
		if v.Obj.Name == proxySwitch.DvsName {
			continue
		}
		newDVSProxy = append(newDVSProxy, proxySwitch)
	}
	host.Obj.Config.Network.ProxySwitch = newDVSProxy
	v.Obj.Config.GetDVSConfigInfo().Host = newDVSHostInfo
	d := simulator.Map.Get(v.Obj.Reference())
	simulator.Map.Update(d, []types.PropertyChange{
		{Name: "config", Val: v.Obj.Config},
	})
	h := simulator.Map.Get(host.Obj.Reference())
	simulator.Map.Update(h, []types.PropertyChange{
		{Name: "config", Val: host.Obj.Config},
	})
	return nil
}
