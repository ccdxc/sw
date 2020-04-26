package vmware

import (
	"context"
	"fmt"
	"io/ioutil"
	"log"
	"net/url"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/pkg/errors"
	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/ovf"

	//"github.com/vmware/govmomi/ovf"
	"github.com/vmware/govmomi/property"
	"github.com/vmware/govmomi/view"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/soap"
	"github.com/vmware/govmomi/vim25/types"
)

const (
	pNicPrefix = "key-vim.host.PhysicalNic-"
)

// NWSpec specifies a virtual newtwork
type NWSpec struct {
	Name string
	Vlan int32
}

// VswitchSpec specifies a virtual switch
type VswitchSpec struct {
	Name  string
	Pnics []string
}

// PortGroupSpec specifies a portgroup
type PortGroupSpec struct {
	Name    string
	Vlan    int32
	Vswitch string
}

// Host encapsulates operations on a vmware host.
type Host struct {
	hs *object.HostSystem
	Entity
}

// Vcenter managed host
type VHost struct {
	Host
}

func (h *Host) GetHostSystem() (*object.HostSystem, error) {

	h.getClientWithRLock()
	defer h.releaseClientRLock()
	finder := h.Finder()

	hs, err := finder.DefaultHostSystem(h.Ctx())
	if err != nil {
		return nil, err
	}

	return hs, nil
}

func (h *VHost) GetHostSystem() (*object.HostSystem, error) {

	h.getClientWithRLock()
	defer h.releaseClientRLock()
	finder := h.Finder()

	hs, err := finder.DefaultHostSystem(h.Ctx())
	if err != nil {
		return nil, err
	}

	return hs, nil
}

// NewHost returns a new *Host for the given hostname.
func NewHost(ctx context.Context, hostname, username, password string) (*Host, error) {
	if hostname == "" || username == "" || password == "" {
		return nil, errors.New("fields were blank during NewHost")
	}

	u, err := url.Parse(fmt.Sprintf("https://%s:%s@%s/sdk", username, password, hostname))
	if err != nil {
		return nil, errors.Wrap(err, "credentials are invalid")
	}

	host := &Host{
		Entity: Entity{
			Name: hostname,
			URL:  u,
		},
	}

	if err := host.Reinit(ctx); err != nil {
		return nil, errors.Wrapf(err, "Connection failed")
	}

	dc, err := host.Entity.Finder().DefaultDatacenter(context.Background())
	if err != nil {
		return nil, errors.Wrapf(err, "Failed to find datacenter")
	}

	host.Entity.Finder().SetDatacenter(dc)
	host.hs, err = host.Entity.Finder().DefaultHostSystem(host.Entity.Ctx())
	if err != nil {
		return nil, errors.Wrapf(err, "Not able to find host system")
	}

	return host, nil
}

//GetVcenterForHost associated vcenter
func (h *Host) GetVcenterForHost() (string, error) {

	h.getClientWithRLock()
	defer h.releaseClientRLock()
	ctx := h.Ctx()

	m := view.NewManager(h.Client().Client)

	v, err := m.CreateContainerView(ctx, h.Client().ServiceContent.RootFolder, []string{"HostSystem"}, true)
	if err != nil {
		log.Fatal(err)
	}

	defer v.Destroy(ctx)
	var hosts []mo.HostSystem
	err = v.Retrieve(ctx, []string{"HostSystem"}, nil, &hosts)
	if err != nil {
		log.Fatal(err)
	}

	for _, host := range hosts {
		return host.Summary.ManagementServerIp, nil
	}

	return "", errors.New("Host not found")
}

//NewVHost create a new Host managed by vcenter
func NewVHost(ctx context.Context, hs *object.HostSystem, pc *property.Collector, finder *find.Finder) *VHost {
	return &VHost{Host: Host{
		Entity: Entity{
			Name: hs.Name(),
			ConnCtx: ConnectionCtx{
				finder:  finder,
				context: &Context{context: ctx},
			},
		},
		hs: hs,
	},
	}

}

func (c *Client) finder() (*find.Finder, *object.Datacenter, error) {
	finder := find.NewFinder(c.Client.Client, false)
	dc, err := finder.DefaultDatacenter(context.Background())
	if err != nil {
		return nil, nil, err
	}

	finder.SetDatacenter(dc)

	return finder, dc, nil
}

func (h *Host) cancel() {
	//	h.Ctx().cancelFunc()
}

// Close closes any connections and forcibly stops any active work.
func (h *Host) Close() error {
	h.cancel()
	h.Client().CloseIdleConnections()
	return nil
}

// GetAllVms gets all vms on this host
func (h *Host) GetAllVms() ([]*VM, error) {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	vms, err := h.Finder().VirtualMachineList(h.Ctx(), "*")
	if err != nil {
		//May be no VMs at all
		return nil, nil
	}

	vmHandles := []*VM{}
	for _, vm := range vms {
		vmHandle, _ := h.NewVM(vm.Name())
		vmHandles = append(vmHandles, vmHandle)
	}

	return vmHandles, nil
}

func (h *Host) hostNetworkSystem() (*object.HostNetworkSystem, error) {

	hs, err := h.Entity.Finder().DefaultHostSystem(h.Ctx())
	if err != nil {
		fmt.Printf("Error %v\n", err)
		return nil, err
	}

	return hs.ConfigManager().NetworkSystem(h.Ctx())

}

func (h *VHost) hostNetworkSystem() (*object.HostNetworkSystem, error) {

	return h.hostNetworkSystemByName(h.Name)
}

func (h *Host) hostNetworkSystemByName(name string) (*object.HostNetworkSystem, error) {
	hsList, err := h.Finder().HostSystemList(h.Ctx(), "*")
	if err != nil {
		return nil, err
	}

	for _, hs := range hsList {
		if hs.Name() == name {
			return hs.ConfigManager().NetworkSystem(h.Ctx())
		}
	}

	return nil, fmt.Errorf("Did not find host network system for %v", name)
}

func (h *Host) vSwitchExists(name string, ns *object.HostNetworkSystem) bool {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	var mns mo.HostNetworkSystem

	err := h.Entity.ConnCtx.pc.RetrieveOne(h.Ctx(), ns.Reference(), []string{"networkInfo.vswitch"}, &mns)
	if err != nil {
		return false
	}

	for _, vs := range mns.NetworkInfo.Vswitch {
		if vs.Name == name {
			return true
		}
	}

	return false
}

// RemoveVswitch removes vswitch from target host
func (h *Host) RemoveVswitch(vspec VswitchSpec) error {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return err
	}

	// if the switch not exists, just return
	if !h.vSwitchExists(vspec.Name, ns) {
		return nil
	}

	return ns.RemoveVirtualSwitch(h.Ctx(), vspec.Name)
}

// AddVswitch adds vswitch to target host
func (h *Host) AddVswitch(vspec VswitchSpec) error {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return err
	}

	// if the switch exists, just return
	if h.vSwitchExists(vspec.Name, ns) {
		return nil
	}

	spec := &types.HostVirtualSwitchSpec{
		NumPorts: 128, // default, but required!
	}
	if len(vspec.Pnics) > 0 {
		spec.Bridge = &types.HostVirtualSwitchBondBridge{
			NicDevice: vspec.Pnics,
		}
	}

	return ns.AddVirtualSwitch(h.Ctx(), vspec.Name, spec)
}

func addVirtualSwitch(ctx context.Context, ns *object.HostNetworkSystem, vspec VswitchSpec) error {
	spec := &types.HostVirtualSwitchSpec{
		NumPorts: 128, // default, but required!
	}
	if len(vspec.Pnics) > 0 {
		spec.Bridge = &types.HostVirtualSwitchBondBridge{
			NicDevice: vspec.Pnics,
		}
	}

	return ns.AddVirtualSwitch(ctx, vspec.Name, spec)
}

// AddVswitch adds vswitch to target host
func (h *VHost) AddVswitch(vspec VswitchSpec) error {
	ns, err := h.hostNetworkSystemByName(h.Name)
	if err != nil {
		return err
	}

	// if the switch exists, just return
	if h.Host.vSwitchExists(vspec.Name, ns) {
		return nil
	}

	return addVirtualSwitch(h.Ctx(), ns, vspec)
}

// AddNetworks add specified network for
func (h *VHost) AddNetworks(specs []NWSpec, vsSpec VswitchSpec) error {

	h.getClientWithRLock()
	defer h.releaseClientRLock()
	var nets []string

	// create our private vswitch
	err := h.AddVswitch(vsSpec)
	if err != nil {
		//return nets, err
	}

	ns, err := h.hostNetworkSystemByName(h.Name)
	if err != nil {
		return err
	}

	// allow everything on the port group
	allow := true
	nsp := &types.HostNetworkSecurityPolicy{
		AllowPromiscuous: &allow,
		MacChanges:       &allow,
		ForgedTransmits:  &allow,
	}
	for _, s := range specs {
		pgs := types.HostPortGroupSpec{
			Name:        s.Name,
			VlanId:      s.Vlan,
			VswitchName: vsSpec.Name,
			Policy:      types.HostNetworkPolicy{Security: nsp},
		}
		err = ns.AddPortGroup(h.Ctx(), pgs)
		if err != nil {
			return err
		}

		nets = append(nets, s.Name)
	}

	return nil

}

// ListVswitchs lists all vswitches in the ESX host
func (h *Host) ListVswitchs() ([]VswitchSpec, error) {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return nil, err
	}
	var mns mo.HostNetworkSystem

	err = h.Entity.ConnCtx.pc.RetrieveOne(h.Ctx(), ns.Reference(), []string{"networkInfo.vswitch"}, &mns)
	if err != nil {
		return nil, err
	}

	specs := []VswitchSpec{}
	for _, vs := range mns.NetworkInfo.Vswitch {
		nics := []string{}
		for _, n := range vs.Pnic {
			nics = append(nics, strings.TrimPrefix(n, pNicPrefix))
		}
		specs = append(specs, VswitchSpec{
			Name:  vs.Name,
			Pnics: nics,
		})
	}

	return specs, nil
}

// ListVswitchs lists all vswitches in the ESX host
func (h *VHost) ListVswitchs() ([]VswitchSpec, error) {
	return h.Host.ListVswitchs()
}

// RemoveNetworks removes the specified virtual networks
func (h *Host) RemoveNetworks(specs []NWSpec) error {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return err
	}

	for _, s := range specs {
		err = ns.RemovePortGroup(h.Ctx(), s.Name)
		if err != nil {
			return fmt.Errorf("Failed to remove %s - %v", s.Name, err)
		}
	}

	return nil
}

// RemoveNetworks removes the specified virtual networks
func (h *VHost) RemoveNetworks(specs []NWSpec) error {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	return h.Host.RemoveNetworks(specs)
}

// AddNetworks adds the specified virtual networks
func (h *Host) AddNetworks(specs []NWSpec, vsSpec VswitchSpec) error {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	var nets []string

	// create our private vswitch
	err := h.AddVswitch(vsSpec)
	if err != nil {
		//return nets, err
	}

	ns, err := h.hostNetworkSystem()
	if err != nil {
		return err
	}

	// allow everything on the port group
	allow := true
	nsp := &types.HostNetworkSecurityPolicy{
		AllowPromiscuous: &allow,
		MacChanges:       &allow,
		ForgedTransmits:  &allow,
	}
	for _, s := range specs {
		pgs := types.HostPortGroupSpec{
			Name:        s.Name,
			VlanId:      s.Vlan,
			VswitchName: vsSpec.Name,
			Policy:      types.HostNetworkPolicy{Security: nsp},
		}
		err = ns.AddPortGroup(h.Ctx(), pgs)
		if err != nil {
			return err
		}

		nets = append(nets, s.Name)
	}

	return nil
}

// ListNetworks return networks in the system
func (h *Host) ListNetworks() ([]PortGroupSpec, error) {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return nil, err
	}
	var mns mo.HostNetworkSystem

	err = h.ConnCtx.pc.RetrieveOne(h.Ctx(), ns.Reference(), []string{"networkInfo.portgroup"}, &mns)
	if err != nil {
		return nil, err
	}

	specs := []PortGroupSpec{}
	for _, pg := range mns.NetworkInfo.Portgroup {
		specs = append(specs, PortGroupSpec{
			Name:    pg.Spec.Name,
			Vlan:    pg.Spec.VlanId,
			Vswitch: pg.Spec.VswitchName,
		})
	}

	return specs, nil
}

// ListNetworks return networks in the system
func (h *VHost) ListNetworks() ([]PortGroupSpec, error) {
	return h.Host.ListNetworks()
}

// AddKernelNic adds the specified kernel nic
func (h *Host) AddKernelNic(cluster, host string, portGroupName string, enableVmotion bool) error {
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return err
	}

	spec := types.HostVirtualNicSpec{
		Ip: &types.HostIpConfig{Dhcp: true},
		//Portgroup:           portGroupName,
		//Mtu:                 1500,
		//TsoEnabled:          types.NewBool(true),
		//NetStackInstanceKey: "defaultTcpipStack",
	}
	name, err := ns.AddVirtualNic(h.Ctx(), portGroupName, spec)
	if err != nil {
		return err
	}

	if enableVmotion {
		vnicMgr, err := h.hs.ConfigManager().VirtualNicManager(h.Ctx())
		if err != nil {
			return errors.Wrap(err, "Unble to get vnic manager")
		}

		err = vnicMgr.SelectVnic(h.Ctx(), "vmotion", name)
		if err != nil {
			return errors.Wrap(err, "enabling vmotion failed")
		}
	}

	return err
}

// RemoveKernelNic removes the specified kernel nic
func (h *Host) RemoveKernelNic(cluster, host string, vnicName string) error {
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return err
	}

	pc := property.DefaultCollector(h.Client().Client)
	var mns mo.HostNetworkSystem

	err = pc.RetrieveOne(h.Ctx(), ns.Reference(), []string{"networkInfo.vnic"}, &mns)
	if err != nil {
		return err
	}

	for _, pg := range mns.NetworkInfo.Vnic {
		if vnicName == pg.Spec.Portgroup {
			return ns.RemoveVirtualNic(h.Ctx(), pg.Device)
		}
	}
	return nil
}

// AddKernelNic adds the specified kernel nic
func (h *VHost) AddKernelNic(nwspec KernelNetworkSpec) error {

	ns, err := h.hostNetworkSystem()
	if err != nil {
		return err
	}

	var net object.NetworkReference
	found := false

L:
	for i := 0; i < 20; i++ {

		netList, err := h.Finder().NetworkList(h.Ctx(), "*")
		if err != nil {
			return errors.Wrap(err, "Failed list Networks")
		}
		for _, nw := range netList {
			splitStr := strings.Split(nw.GetInventoryPath(), "/")
			fmt.Printf("Found network %v\n", nw.GetInventoryPath())
			name := splitStr[len(splitStr)-1]
			if name == nwspec.Portgroup {
				net, err = h.Finder().Network(h.Ctx(), name)
				if err != nil {
					return errors.Wrap(err, "Network not found")
				}
				found = true
				break L
			}
		}
		time.Sleep(1 * time.Second)
	}

	if !found {
		return fmt.Errorf("Network %v not found", nwspec.Portgroup)
	}

	portGroupName := ""
	tso := true
	spec := types.HostVirtualNicSpec{
		Ip: &types.HostIpConfig{
			Dhcp: true,
		},

		Portgroup:           "",
		Mtu:                 1500,
		TsoEnabled:          &tso,
		NetStackInstanceKey: "defaultTcpipStack",
	}
	if nwspec.IPAddress != "" {
		spec.Ip.Dhcp = false
		spec.Ip.IpAddress = nwspec.IPAddress
		spec.Ip.SubnetMask = nwspec.Subnet
	}
	if nwspec.MacAddress != "" {
		spec.Mac = nwspec.MacAddress
	}
	objPg, ok := net.(*object.DistributedVirtualPortgroup)
	if ok {
		spec.DistributedVirtualPort = &types.DistributedVirtualSwitchPortConnection{}
		spec.DistributedVirtualPort.PortgroupKey = objPg.Reference().Value
		pgConfig, err := net.EthernetCardBackingInfo(h.Ctx())
		if err != nil {
			return errors.Wrap(err, "Failed Find PG backing config")
		}

		if vPortConfig, ok := pgConfig.(*types.VirtualEthernetCardDistributedVirtualPortBackingInfo); ok {
			spec.DistributedVirtualPort.SwitchUuid = vPortConfig.Port.SwitchUuid
		} else {
			return errors.Wrap(err, "Cannot find DVS PG port confign info")
		}
	} else {
		// standard PG
		portGroupName = nwspec.Portgroup
		spec.Portgroup = nwspec.Portgroup
	}

	name, err := ns.AddVirtualNic(h.Ctx(), portGroupName, spec)
	if err != nil {
		return err
	}

	if nwspec.EnableVmotion {
		vnicMgr, err := h.hs.ConfigManager().VirtualNicManager(h.Ctx())
		if err != nil {
			return errors.Wrap(err, "Unble to get vnic manager")
		}

		err = vnicMgr.SelectVnic(h.Ctx(), "vmotion", name)
		if err != nil {
			return errors.Wrap(err, "enabling vmotion failed")
		}
	}

	return nil
}

// RemoveKernelNic removes the kernel nic on a givne pg
func (h *VHost) RemoveKernelNic(pgName string) error {
	fmt.Printf("Remove vmknics\n")
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return err
	}

	pc := property.DefaultCollector(h.Client().Client)
	var mns mo.HostNetworkSystem

	err = pc.RetrieveOne(h.Ctx(), ns.Reference(), []string{"networkInfo.vnic"}, &mns)
	if err != nil {
		return err
	}
	net, err := h.Finder().Network(h.Ctx(), pgName)
	if err != nil {
		return err
	}
	objPg, ok := net.(*object.DistributedVirtualPortgroup)
	if !ok {
		// standard PG
		for _, vmknic := range mns.NetworkInfo.Vnic {
			if pgName == vmknic.Spec.Portgroup {
				err = ns.RemoveVirtualNic(h.Ctx(), vmknic.Device)
				if err != nil {
					return err
				}
			}
		}
	} else {
		for _, vmknic := range mns.NetworkInfo.Vnic {
			if vmknic.Spec.DistributedVirtualPort == nil {
				continue
			}
			if objPg.Reference().Value == vmknic.Spec.DistributedVirtualPort.PortgroupKey {
				err = ns.RemoveVirtualNic(h.Ctx(), vmknic.Device)
				if err != nil {
					return err
				}
			}
		}
	}
	return nil
}

func (h *Host) boot(name string, ncpus uint, memory uint) (*VMInfo, error) {
	vm, err := h.Finder().VirtualMachine(h.Ctx(), name)

	if err != nil {
		return nil, errors.Wrap(err, "Could not find VM")
	}

	if err = h.reconfigureVM(vm, ncpus, memory); err != nil {
		return nil, errors.Wrap(err, "Reconfiguration failed")
	}

	task, err := vm.PowerOn(h.Ctx())
	if err != nil {
		return nil, errors.Wrap(err, "Power on task start failed")
	}
	err = task.Wait(h.Ctx())
	if err != nil {
		return nil, errors.Wrap(err, "Power on task failed")
	}

	var ip string
	ip, err = vm.WaitForIP(h.Ctx(), true)
	if err != nil {
		return nil, errors.Wrap(err, "Wait for IP failed")
	}

	return &VMInfo{Name: name, IP: ip}, nil // first arg is string, second is net.IP.
}

// BootVM botts up a new vm
func (h *Host) BootVM(name string) (*VMInfo, error) {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	ds, err := h.Datastore("")

	if err != nil {
		return nil, err
	}

	finder := ds.Entity.Finder()
	if err != nil {
		return nil, err
	}

	vm, err := finder.VirtualMachine(h.Ctx(), name)

	if err != nil {
		return nil, err
	}
	task, err := vm.PowerOn(h.Ctx())
	if err != nil {
		return nil, err
	}
	err = task.Wait(h.Ctx())
	if err != nil {
		return nil, err
	}

	var ip string
	//Wait for only IPV4 address
	ip, err = vm.WaitForIP(h.Ctx(), true)
	if err != nil {
		return nil, err
	}

	return &VMInfo{Name: name, IP: ip}, nil // first arg is string, second is net.IP.
}

// PowerOffVM powers off a vm
func (h *Host) PowerOffVM(name string) error {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	ds, err := h.Datastore("datastore1")

	if err != nil {
		return err
	}

	finder := ds.Entity.Finder()

	vm, err := finder.VirtualMachine(h.Ctx(), name)

	if err != nil {
		return err
	}
	task, err := vm.PowerOff(h.Ctx())
	if err != nil {
		return err
	}
	err = task.Wait(h.Ctx())
	if err != nil {
		return err
	}

	return nil
}

// GetVMIP gets VM's ip address
func (h *Host) GetVMIP(name string) (string, error) {
	h.getClientWithRLock()
	defer h.releaseClientRLock()

	finder := h.Finder()

	vm, err := finder.VirtualMachine(h.Ctx(), name)

	if err != nil {
		return "", err
	}

	return vm.WaitForIP(h.Ctx(), true)
}

//PoweredOn VM powered on
func (h *Host) PoweredOn(name string) (bool, error) {

	h.getClientWithRLock()
	defer h.releaseClientRLock()
	finder := h.Finder()

	vm, err := finder.VirtualMachine(h.Ctx(), name)

	if err != nil {
		return false, err
	}

	state, err := vm.PowerState(h.Ctx())
	if err != nil {
		return false, err
	}

	if state == types.VirtualMachinePowerStatePoweredOn {
		return true, nil

	}

	return false, nil
}

// Datastore returns a *Datastore which allows you to perform operations on it.
func (h *Host) Datastore(name string) (*Datastore, error) {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	finder := h.Finder()

	var ds *object.Datastore
	var err error
	if name == "" {
		ds, err = finder.DefaultDatastore(h.Ctx())

	} else {
		ds, err = finder.Datastore(h.Ctx(), name)
	}
	if err != nil {
		return nil, err
	}

	return &Datastore{
		datastore: ds,
		Entity:    &h.Entity,
	}, nil
}

func (h *Host) getOvf(dir string) (string, error) {
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

func (h *Host) getOvfSrc(dir string) ([]byte, error) {
	file, err := h.getOvf(dir)
	if err != nil {
		return nil, err
	}
	return ioutil.ReadFile(file)
}

func (h *Host) getNWMap(ovfDir string, networks []string, finder *find.Finder) ([]types.OvfNetworkMapping, error) {
	var nm []types.OvfNetworkMapping

	if len(networks) <= 1 {
		// use default networking
		return nm, nil
	}

	file, err := h.getOvf(ovfDir)
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
		if netObj, err := finder.Network(h.Ctx(), hostNet); err == nil {
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

func (h *Host) getImportSpec(name string, ds *Datastore, ovfDir string, finder *find.Finder, networks []string) (*types.OvfCreateImportSpecResult, error) {
	ovfSrc, err := h.getOvfSrc(ovfDir)
	if err != nil {
		return nil, err
	}

	nwMap, err := h.getNWMap(ovfDir, networks, finder)
	if err != nil {
		return nil, err
	}

	cisp := types.OvfCreateImportSpecParams{
		DiskProvisioning: "thin",
		EntityName:       name,
		NetworkMapping:   nwMap,
	}

	rp, err := finder.DefaultResourcePool(h.Ctx())
	if err != nil {
		return nil, err
	}

	mgr := ovf.NewManager(h.Client().Client)
	spec, err := mgr.CreateImportSpec(h.Ctx(), string(ovfSrc), rp, ds.datastore.Reference(), cisp)
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

func (h *Host) reconfigureVM(vm *object.VirtualMachine, ncpus uint, memory uint) error {
	bigMemory := int64(memory * 1024)

	var task *object.Task

	var err error

	t := true

	task, err = vm.Reconfigure(h.Ctx(), types.VirtualMachineConfigSpec{
		MemoryReservationLockedToMax: &t,
		NumCPUs:                      int32(ncpus),
		MemoryMB:                     bigMemory,
	})
	if err != nil {
		return err
	}

	return task.Wait(h.Ctx())
}

func (h *Host) importVapp(dir string, spec *types.OvfCreateImportSpecResult, finder *find.Finder) (retErr error) {
	ctx, cancel := context.WithTimeout(h.Ctx(), 30*time.Minute)
	defer cancel()

	host, err := finder.DefaultHostSystem(ctx)
	if err != nil {
		return err
	}

	folder, err := finder.DefaultFolder(ctx)
	if err != nil {
		return err
	}

	rp, err := finder.DefaultResourcePool(ctx)
	if err != nil {
		return err
	}

	lease, err := rp.ImportVApp(ctx, spec.ImportSpec, folder, host)
	if err != nil {
		return err
	}

	info, err := lease.Wait(ctx, spec.FileItem)
	if err != nil {
		return err
	}

	u := lease.StartUpdater(ctx, info)
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

		if err := lease.Upload(ctx, item, f, soap.Upload{ContentLength: fi.Size()}); err != nil {
			return err
		}
	}

	lease.Complete(ctx)

	return nil
}

/*
// DeployVMOnDataStore deploys a VM on a datastore
func (h *Host) DeployVMOnDataStore(ds *Datastore, name string, ncpus uint, memory uint, networks []string, ovfDir string) (*VMInfo, error) {

	finder, _, err := ds.client.finder()
	if err != nil {
		return nil, err
	}

	var spec *types.OvfCreateImportSpecResult

	spec, err = h.getImportSpec(name, ds, ovfDir, finder, networks)
	if err != nil {
		return nil, err
	}

	err = h.importVapp(ovfDir, spec, finder)
	if err != nil {
		return nil, errors.Wrap(err, "Import vapp failed")
	}

	//SLeep for a while before booting.
	time.Sleep(5 * time.Second)
	return h.boot(name, ncpus, memory)
}
*/

// DeployVM deploys a vm
func (h *Host) DeployVM(clName, hostName, name string, ncpus uint, memory uint, networks []string, ovfDir string) (*VMInfo, error) {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	ds, err := h.Datastore("")

	if err != nil {
		return nil, err
	}

	return h.DeployVMOnDataStore(ds, h.hs, name, ncpus, memory, networks, ovfDir)
}

// CloneVM clones a VM
func (h *Host) CloneVM(name, newName string, ncpus uint, memory uint, networks []string, ovfDir string) (*VMInfo, error) {
	h.getClientWithRLock()
	defer h.releaseClientRLock()
	return h.cloneVMOnHost(h.hs, name, newName, ncpus, memory)
}

func (h *Host) FindDvsPortGroup(name string, mcriteria DvsPGMatchCriteria) (string, error) {

	return "", errors.New("NOT IMPLEMENTED")
}

func (h *Host) AddPortGroupToDvs(name string, pairs []DvsPortGroup) error {
	return errors.New("NOT IMPLEMENTED")
}
