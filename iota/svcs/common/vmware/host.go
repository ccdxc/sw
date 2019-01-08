package vmware

import (
	"context"
	"fmt"
	"io/ioutil"
	"net/url"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/pkg/errors"
	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/ovf"
	//"github.com/vmware/govmomi/ovf"
	"github.com/vmware/govmomi/property"
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

// Host encapsulates operations on a vmware host.
type Host struct {
	Hostname string
	URL      *url.URL

	client  *Client
	context *Context
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

	ctx, cancel := context.WithCancel(ctx)
	client, err := govmomi.NewClient(ctx, u, !TLSVerify)
	if err != nil {
		cancel()
		return nil, err
	}
	return &Host{
		Hostname: hostname,
		URL:      u,
		context:  &Context{context: ctx, cancelFunc: cancel},
		client:   &Client{client},
	}, nil
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
	h.context.cancelFunc()
}

// Close closes any connections and forcibly stops any active work.
func (h *Host) Close() error {
	h.cancel()
	h.client.CloseIdleConnections()
	return nil
}

func (h *Host) GetAllVms() ([]*VM, error) {
	finder, _, err := h.client.finder()
	if err != nil {
		return nil, err
	}

	vms, err := finder.VirtualMachineList(h.context.context, "*")
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
	finder, _, err := h.client.finder()
	if err != nil {
		return nil, err
	}

	hs, err := finder.DefaultHostSystem(h.context.context)
	if err != nil {
		return nil, err
	}

	return hs.ConfigManager().NetworkSystem(h.context.context)
}

func (h *Host) vSwitchExists(name string, ns *object.HostNetworkSystem) bool {
	pc := property.DefaultCollector(h.client.Client.Client)
	var mns mo.HostNetworkSystem

	err := pc.RetrieveOne(h.context.context, ns.Reference(), []string{"networkInfo.vswitch"}, &mns)
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
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return err
	}

	// if the switch not exists, just return
	if !h.vSwitchExists(vspec.Name, ns) {
		return nil
	}

	return ns.RemoveVirtualSwitch(h.context.context, vspec.Name)
}

// AddVswitch adds vswitch to target host
func (h *Host) AddVswitch(vspec VswitchSpec) error {
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

	return ns.AddVirtualSwitch(h.context.context, vspec.Name, spec)
}

// ListVswitchs lists all vswitches in the ESX host
func (h *Host) ListVswitchs() ([]VswitchSpec, error) {
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return nil, err
	}
	pc := property.DefaultCollector(h.client.Client.Client)
	var mns mo.HostNetworkSystem

	err = pc.RetrieveOne(h.context.context, ns.Reference(), []string{"networkInfo.vswitch"}, &mns)
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

// RemoveNetworks removes the specified virtual networks
func (h *Host) RemoveNetworks(specs []NWSpec) error {
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return err
	}

	for _, s := range specs {
		err = ns.RemovePortGroup(h.context.context, s.Name)
		if err != nil {
			return fmt.Errorf("Failed to remove %s - %v", s.Name, err)
		}
	}

	return nil
}

// AddNetworks adds the specified virtual networks
func (h *Host) AddNetworks(specs []NWSpec, vsSpec VswitchSpec) ([]string, error) {
	var nets []string

	// create our private vswitch
	err := h.AddVswitch(vsSpec)
	if err != nil {
		return nets, err
	}

	ns, err := h.hostNetworkSystem()
	if err != nil {
		return nets, err
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
		err = ns.AddPortGroup(h.context.context, pgs)
		if err != nil {
			return nets, err
		}

		nets = append(nets, s.Name)
	}

	return nets, nil
}

// ListNetworks return networks in the system
func (h *Host) ListNetworks() ([]PortGroupSpec, error) {
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return nil, err
	}
	pc := property.DefaultCollector(h.client.Client.Client)
	var mns mo.HostNetworkSystem

	err = pc.RetrieveOne(h.context.context, ns.Reference(), []string{"networkInfo.portgroup"}, &mns)
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

// RemoveKernelNic removes the specified kernel nic
func (h *Host) RemoveKernelNic(vnicName string) error {
	ns, err := h.hostNetworkSystem()
	if err != nil {
		return err
	}

	return ns.RemoveVirtualNic(h.context.context, vnicName)
}

func (h *Host) boot(name string, ncpus uint, memory uint, finder *find.Finder) (*VMInfo, error) {
	vm, err := finder.VirtualMachine(h.context.context, name)

	if err != nil {
		return nil, err
	}

	if err = h.reconfigureVM(vm, ncpus, memory); err != nil {
		return nil, err
	}

	task, err := vm.PowerOn(h.context.context)
	if err != nil {
		return nil, err
	}
	err = task.Wait(h.context.context)
	if err != nil {
		return nil, err
	}

	var ip string
	ip, err = vm.WaitForIP(h.context.context)
	if err != nil {
		return nil, err
	}

	return &VMInfo{Name: name, IP: ip}, nil // first arg is string, second is net.IP.
}

func (h *Host) BootVM(name string) (*VMInfo, error) {
	ds, err := h.Datastore("datastore1")

	if err != nil {
		return nil, err
	}

	finder, _, err := ds.client.finder()
	if err != nil {
		return nil, err
	}

	vm, err := finder.VirtualMachine(h.context.context, name)

	if err != nil {
		return nil, err
	}
	task, err := vm.PowerOn(h.context.context)
	if err != nil {
		return nil, err
	}
	err = task.Wait(h.context.context)
	if err != nil {
		return nil, err
	}

	var ip string
	ip, err = vm.WaitForIP(h.context.context)
	if err != nil {
		return nil, err
	}

	return &VMInfo{Name: name, IP: ip}, nil // first arg is string, second is net.IP.
}

func (h *Host) PowerOffVM(name string) error {
	ds, err := h.Datastore("datastore1")

	if err != nil {
		return err
	}

	finder, _, err := ds.client.finder()
	if err != nil {
		return err
	}

	vm, err := finder.VirtualMachine(h.context.context, name)

	if err != nil {
		return err
	}
	task, err := vm.PowerOff(h.context.context)
	if err != nil {
		return err
	}
	err = task.Wait(h.context.context)
	if err != nil {
		return err
	}

	return nil
}

func (h *Host) GetVMIP(name string) (string, error) {

	finder, _, err := h.client.finder()
	if err != nil {
		return "", err
	}

	vm, err := finder.VirtualMachine(h.context.context, name)

	if err != nil {
		return "", err
	}

	return vm.WaitForIP(h.context.context)
}

// Datastore returns a *Datastore which allows you to perform operations on it.
func (h *Host) Datastore(name string) (*Datastore, error) {
	finder, dc, err := h.client.finder()
	if err != nil {
		return nil, err
	}

	var ds *object.Datastore
	if name == "" {
		ds, err = finder.DefaultDatastore(h.context.context)

	} else {
		ds, err = finder.Datastore(h.context.context, name)
	}
	if err != nil {
		return nil, err
	}

	return &Datastore{
		datastore:  ds,
		datacenter: dc,
		host:       h,
		client:     h.client,
		context:    h.context,
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
		if netObj, err := finder.Network(h.context.context, hostNet); err == nil {
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

	rp, err := finder.DefaultResourcePool(h.context.context)
	if err != nil {
		return nil, err
	}

	mgr := ovf.NewManager(h.client.Client.Client)
	spec, err := mgr.CreateImportSpec(h.context.context, string(ovfSrc), rp, ds.datastore.Reference(), cisp)
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

func (h *Host) wipeOut(name string) error {
	vm, err := h.NewVM(name)
	if err != nil {
		return err
	}

	if err := vm.Destroy(); err != nil {
		return err
	}

	return nil
}

func (h *Host) RemoveVm(name string) error {
	return h.wipeOut(name)
}

func (h *Host) reconfigureVM(vm *object.VirtualMachine, ncpus uint, memory uint) error {
	bigMemory := int64(memory * 1024)

	var task *object.Task

	var err error

	t := true

	task, err = vm.Reconfigure(h.context.context, types.VirtualMachineConfigSpec{
		MemoryReservationLockedToMax: &t,
		NumCPUs:  int32(ncpus),
		MemoryMB: bigMemory,
	})
	if err != nil {
		return err
	}

	return task.Wait(h.context.context)
}

func (h *Host) importVapp(dir string, spec *types.OvfCreateImportSpecResult, finder *find.Finder) (retErr error) {
	ctx, cancel := context.WithTimeout(h.context.context, 30*time.Minute)
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

	return lease.Complete(ctx)
}

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
		return nil, err
	}

	return h.boot(name, ncpus, memory, finder)
}

func (h *Host) DeployVM(name string, ncpus uint, memory uint, networks []string, ovfDir string) (*VMInfo, error) {
	ds, err := h.Datastore("datastore1")

	if err != nil {
		return nil, err
	}

	return h.DeployVMOnDataStore(ds, name, ncpus, memory, networks, ovfDir)
}

func (h *Host) DestoryVM(name string) error {
	return h.RemoveVm(name)
}
