package vmware

import (
	"context"
	"fmt"
	"net/url"
	"path/filepath"
	"strings"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pkg/errors"
	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"
)

//Vcenter instance
type Vcenter struct {
	Entity
	datacenters map[string]*DataCenter
}

//DataCenter instance
type DataCenter struct {
	name     string
	vc       *Vcenter
	ref      *object.Datacenter
	clusters map[string]*Cluster
}

//Cluster instance
type Cluster struct {
	dc    *DataCenter
	ref   *object.ClusterComputeResource
	hosts map[string]*VHost
}

type KernelNetworkSpec struct {
	Portgroup     string
	IPAddress     string
	Subnet        string
	MacAddress    string
	EnableVmotion bool
}

// NewVcenter returns a new *Vcenter instance
func NewVcenter(ctx context.Context, name, username, password, license string) (*Vcenter, error) {
	if name == "" || username == "" || password == "" {
		return nil, errors.New("fields were blank during NewHost")
	}

	url, err := url.Parse(fmt.Sprintf("https://%s:%s@%s/sdk", username, password, name))
	if err != nil {
		return nil, errors.Wrap(err, "credentials are invalid")
	}

	vc := Vcenter{
		Entity: Entity{
			Name:    name,
			URL:     url,
			License: license,
		},
		datacenters: make(map[string]*DataCenter),
	}

	if err := vc.Reinit(ctx); err != nil {
		return nil, errors.Wrapf(err, "Connection failed")
	}
	go vc.monitorSession()

	return &vc, nil
}

//CreateDataCenter Creates a datacenter
func (vc *Vcenter) CreateDataCenter(name string) (*DataCenter, error) {
	vc.getClientWithRLock()
	defer vc.releaseClientRLock()
	f := object.NewRootFolder(vc.Client().Client)

	dc, err := f.CreateDatacenter(vc.Ctx(), name)
	if err != nil {
		return nil, err
	}

	finder := vc.Finder()
	dc, err = finder.Datacenter(vc.Ctx(), name)
	if err != nil {
		return nil, err
	}

	finder.SetDatacenter(dc)
	dcr := &DataCenter{
		name: name,
		ref:  dc, vc: vc,
		clusters: make(map[string]*Cluster)}

	vc.datacenters[name] = dcr

	return dcr, nil
}

//DisconnectHost disconnects hosts from vcenter
func (vc *Vcenter) DisconnectHost(ip string) error {
	vc.getClientWithRLock()
	defer vc.releaseClientRLock()
	finder := find.NewFinder(vc.Client().Client, false)

	dcList, err := finder.DatacenterList(vc.Ctx(), "*")
	if err != nil {
		return err
	}

	for _, dc := range dcList {
		finder.SetDatacenter(dc)
		hosts, err := finder.HostSystemList(vc.Ctx(), "*")
		if err == nil {
			for _, host := range hosts {
				if host.Name() == ip {
					var h mo.HostSystem
					err := host.Properties(vc.Ctx(), host.Reference(), []string{"parent", "config.network"}, &h)
					if err != nil {
						return err
					}
					// Remove all vmk nics on the host
					ns, err := host.ConfigManager().NetworkSystem(vc.Ctx())
					if err != nil {
						return err
					}
					for _, vnic := range h.Config.Network.Vnic {
						// HACK skip vmk0 as it is used for host communication
						if vnic.Device == "vmk0" {
							continue
						}
						log.Infof("Remove vmk %v from host %v", vnic.Device, host.Name())
						if err := ns.RemoveVirtualNic(vc.Ctx(), vnic.Device); err != nil {
							log.Errorf("%v", err)
							// return err
						}
					}

					remove := host.Destroy
					if h.Parent.Type == "ComputeResource" {
						// Standalone host.  From the docs:
						// "Invoking remove on a HostSystem of standalone type throws a NotSupported fault.
						//  A standalone HostSystem can be removeed only by invoking remove on its parent ComputeResource."
						remove = object.NewComputeResource(host.Client(), *h.Parent).Destroy
					}

					task, err := host.Disconnect(vc.Ctx())
					if err != nil {
						return err
					}
					_, err = task.WaitForResult(vc.Ctx(), nil)
					if err != nil {
						return err
					}

					task, err = remove(vc.Ctx())
					if err != nil {
						return err
					}
					return nil
				}
			}
		}
	}

	//Did not fine the host, so return ok
	return nil
}

//SetupDataCenter setups existing dc
func (vc *Vcenter) SetupDataCenter(name string) (*DataCenter, error) {
	vc.getClientWithRLock()
	defer vc.releaseClientRLock()

	vc.ConnCtx.finder = find.NewFinder(vc.Client().Client, false)
	finder := vc.ConnCtx.finder
	dc, err := finder.Datacenter(vc.Ctx(), name)
	if err != nil {
		return nil, err
	}

	vc.ConnCtx.finder.SetDatacenter(dc)
	dcr := &DataCenter{ref: dc, vc: vc,
		clusters: make(map[string]*Cluster)}

	vc.datacenters[name] = dcr

	clusters, err := finder.ClusterComputeResourceList(vc.Ctx(), "*")
	if err == nil {
		for _, cluster := range clusters {
			cl := &Cluster{dc: dcr, ref: cluster, hosts: make(map[string]*VHost)}
			dcr.clusters[cluster.Name()] = cl
		}
	}

	for _, cluster := range dcr.clusters {
		hosts, err := finder.HostSystemList(vc.Ctx(), cluster.ref.InventoryPath+"/*")
		if err == nil {
			for _, host := range hosts {
				cluster.hosts[host.Name()] = NewVHost(vc.Ctx(), host, vc.ConnCtx.pc, finder)
			}
		}
	}

	return dcr, nil
}

//DestroyDataCenter destroys datacenter
func (vc *Vcenter) DestroyDataCenter(name string) error {
	vc.getClientWithRLock()
	defer vc.releaseClientRLock()

	dc, ok := vc.datacenters[name]
	if !ok {
		//Try to find and delete any
		finder := find.NewFinder(vc.Client().Client, false)
		dc, err := finder.Datacenter(vc.Ctx(), name)
		if err == nil {
			task, err := dc.Destroy(vc.Ctx())
			if err == nil {
				return task.Wait(vc.Ctx())
			}
			return err
		}
		return fmt.Errorf("datacenter %v not found for delete", name)
	}

	delete(vc.datacenters, name)
	if dc.ref != nil {
		task, err := dc.ref.Destroy(dc.vc.Ctx())
		dc.ref = nil
		_, err = task.WaitForResult(dc.vc.Ctx(), nil)
		if err != nil {
			return err
		}

		return err
	}

	return nil
}

//CreateCluster Creates a cluster
func (dc *DataCenter) CreateCluster(name string) (*Cluster, error) {
	dc.getClientWithRLock()
	defer dc.releaseClientRLock()

	err := dc.setUpFinder()
	if err != nil {
		return nil, errors.Wrapf(err, "Error setup datacenter")
	}
	folders, err := dc.ref.Folders(dc.vc.Ctx())
	if err != nil {
		return nil, err
	}

	_, err = folders.HostFolder.CreateCluster(dc.vc.Ctx(),
		name, types.ClusterConfigSpecEx{})
	if err != nil {
		return nil, err
	}

	//Read again for cluster ref
	clusters, err := dc.Finder().ClusterComputeResourceList(dc.vc.Ctx(), "*")
	if err == nil {
		for _, cl := range clusters {
			if cl.Name() == name {
				cl := &Cluster{dc: dc, ref: cl, hosts: make(map[string]*VHost)}
				dc.clusters[name] = cl
				return cl, nil
			}
		}
	}

	return nil, errors.New("Cluster create failed")
}

func (dc *DataCenter) Finder() *find.Finder {
	return dc.vc.Finder()
}

func (dc *DataCenter) Ctx() context.Context {
	return dc.vc.Ctx()
}

//DestroyCluster destroys cluster
func (dc *DataCenter) DestroyCluster(name string) error {
	dc.getClientWithRLock()
	defer dc.releaseClientRLock()

	err := dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setup datacenter")
	}
	cluster, ok := dc.clusters[name]
	if !ok {
		return fmt.Errorf("Cluster %v not found for delete", name)
	}

	delete(dc.clusters, name)
	if cluster.ref != nil {
		task, err := cluster.ref.Destroy(cluster.dc.vc.Ctx())
		if err == nil {
			task.Wait(cluster.dc.vc.Ctx())
		}
		cluster.ref = nil
		return err
	}

	return nil
}

//AddHost adds host to cluster
func (cl *Cluster) AddHost(ip, username, password, sslKey string) error {
	cl.dc.getClientWithRLock()
	defer cl.dc.releaseClientRLock()

	err := cl.dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setup datacenter")
	}
	spec := types.HostConnectSpec{HostName: ip,
		Password: password, UserName: username, Force: true, SslThumbprint: sslKey}

	task, err := cl.ref.AddHost(cl.dc.vc.Ctx(), spec, true, &cl.dc.vc.Entity.License, nil)

	if err != nil {
		log.Errorf("TOPO SVC | AddHost  | Failed to add host %v. Error for AddHost %v", ip, err.Error())
		return err
	}

	_, err = task.WaitForResult(cl.dc.vc.Ctx(), nil)
	if err != nil {
		log.Errorf("TOPO SVC | AddHost  | Failed to add host %v. Error for WaitForResult %v", ip, err.Error())
		return err
	}

	hosts, err := cl.ref.Hosts(cl.dc.vc.Ctx())
	if err != nil || len(hosts) == 0 {
		return fmt.Errorf("Host  %v not found to add", ip)
	}

	var hostRef *object.HostSystem
	for _, host := range hosts {
		if host.Name() == ip {
			hostRef = host
			break
		}
	}

	if hostRef == nil {
		return fmt.Errorf("Host %v not found to add", ip)
	}

	vhost := NewVHost(cl.dc.vc.Ctx(), hostRef, cl.dc.vc.ConnCtx.pc, cl.dc.Finder())
	cl.hosts[ip] = vhost
	return nil
}

//DeleteHost adds host to cluster
func (cl *Cluster) DeleteHost(ip string) error {
	cl.dc.getClientWithRLock()
	defer cl.dc.releaseClientRLock()

	err := cl.dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setup datacenter")
	}
	host, err := cl.dc.findHost(cl.ref.Name(), ip)
	if err != nil {
		return nil
	}
	remove := host.Host.hs.Destroy

	/*
		if host.Host.hs. == "ComputeResource" {
			// Standalone host.  From the docs:
			// "Invoking remove on a HostSystem of standalone type throws a NotSupported fault.
			//  A standalone HostSystem can be removeed only by invoking remove on its parent ComputeResource."
			remove = object.NewComputeResource(host.Client(), *h.Parent).Destroy
		}*/

	task, err := host.hs.Disconnect(cl.dc.vc.Ctx())
	if err != nil {
		return err
	}
	_, err = task.WaitForResult(cl.dc.vc.Ctx(), nil)
	if err != nil {
		return err
	}

	task, err = remove(cl.dc.vc.Ctx())
	if err != nil {
		return err
	}

	_, err = task.WaitForResult(cl.dc.vc.Ctx(), nil)
	if err != nil {
		return err
	}

	delete(cl.hosts, ip)

	return nil
}

//Datastore new datastore
func (dc *DataCenter) Datastore(cluster, hostName string) (*Datastore, error) {
	var err error

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	err = dc.setUpFinder()
	if err != nil {
		return nil, errors.Wrapf(err, "Error setup datacenter")
	}

	dsList, err := dc.Finder().DatastoreList(dc.Ctx(), "*")
	if err != nil {
		return nil, err
	}

	hs, err := dc.findHost(cluster, hostName)
	if err != nil {
		return nil, err
	}

	for _, ds := range dsList {
		hosts, err := ds.AttachedHosts(dc.Ctx())
		if err != nil {
			return nil, err
		}

		for _, h := range hosts {
			if h.Reference() == hs.hs.Reference() {
				return &Datastore{
					datastore:  ds,
					datacenter: dc.ref,
					Entity:     &dc.vc.Entity,
				}, nil
			}
		}

	}

	return nil, fmt.Errorf("Host %v did not find a datastore", hostName)
}

func (dc *DataCenter) findHost(clusterName string, hostname string) (*VHost, error) {
	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	cluster, ok := dc.clusters[clusterName]
	if !ok {
		return nil, fmt.Errorf("Cluster %v not found to deploy", clusterName)
	}

	if vhost, ok := cluster.hosts[hostname]; ok {
		//vhost.context = dc.vc.context
		vhost.Entity.ConnCtx = dc.vc.ConnCtx
		return vhost, nil
	}

	//Try to find

	var hostRef *object.HostSystem
	hosts, err := dc.Finder().HostSystemList(dc.vc.Ctx(), cluster.ref.InventoryPath+"/*")
	if err == nil {
		for _, host := range hosts {
			if host.Name() == hostname {
				hostRef = host
				break
			}
		}
	} else {
		return nil, fmt.Errorf("No hosts in the cluster for %v %v %v", clusterName, err.Error(), cluster.ref.InventoryPath+"/*")
	}

	if hostRef == nil {
		return nil, fmt.Errorf("Could not find host  %v in cluster %v", hostname, clusterName)
	}

	vhost := NewVHost(dc.vc.Ctx(), hostRef, dc.vc.ConnCtx.pc, dc.Finder())
	cluster.hosts[hostname] = vhost

	return vhost, nil
}

//findVM  Find VM on a host on a datacenter
func (dc *DataCenter) findVM(hostRef *object.HostSystem, vmName string) (*VM, error) {
	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	vms, err := dc.Finder().VirtualMachineList(dc.vc.Ctx(), hostRef.InventoryPath+"/*")
	if err != nil {
		return nil, fmt.Errorf("Did not find VM  %v", vmName)
	}

	for _, vm := range vms {
		if vm.Name() == vmName {
			return dc.vc.makeVM(vmName, vm), nil
		}
	}

	return nil, fmt.Errorf("Did not find VM  %v", vmName)
}

func (dc *DataCenter) setUpFinder() error {

	finder := find.NewFinder(dc.vc.Client().Client, false)
	dcr, err := finder.Datacenter(dc.vc.Ctx(), dc.ref.Name())
	if err != nil {
		return err
	}
	dc.vc.ConnCtx.finder = finder
	//dc.Finder().SetDatacenter(dcr)
	finder.SetDatacenter(dcr)
	return nil

}

//DeployVM deploys vm on data center
func (dc *DataCenter) DeployVM(clusterName string, hostname string,
	name string, ncpus uint, memory uint, networks []string, ovfDir string) (*VMInfo, error) {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	err := dc.setUpFinder()
	if err != nil {
		return nil, errors.Wrapf(err, "Error setting up datacenter")
	}

	hostRef, err := dc.findHost(clusterName, hostname)
	if err != nil {
		return nil, err
	}

	ds, err := dc.Datastore(clusterName, hostname)
	if err != nil {
		return nil, errors.Wrapf(err, "Data store find failed")
	}

	return dc.vc.DeployVMOnDataStore(ds, hostRef.hs, name, ncpus, memory, networks, ovfDir)
}

//DeleteHost deletes hosts from cluster
func (dc *DataCenter) DeleteHost(cluster string, ip string) error {
	dc.getClientWithRLock()
	defer dc.releaseClientRLock()

	err := dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setup datacenter")
	}
	host, err := dc.findHost(cluster, ip)
	if err != nil {
		return nil
	}
	remove := host.Host.hs.Destroy

	/*
		if host.Host.hs. == "ComputeResource" {
			// Standalone host.  From the docs:
			// "Invoking remove on a HostSystem of standalone type throws a NotSupported fault.
			//  A standalone HostSystem can be removeed only by invoking remove on its parent ComputeResource."
			remove = object.NewComputeResource(host.Client(), *h.Parent).Destroy
		}*/

	task, err := host.hs.Disconnect(dc.vc.Ctx())
	if err != nil {
		return err
	}
	_, err = task.WaitForResult(dc.vc.Ctx(), nil)
	if err != nil {
		return err
	}

	task, err = remove(dc.vc.Ctx())
	if err != nil {
		return err
	}

	_, err = task.WaitForResult(dc.vc.Ctx(), nil)
	if err != nil {
		return err
	}

	return nil
}

//AddVswitch add a vswitch to host
func (dc *DataCenter) AddVswitch(cluster, host string, vspec VswitchSpec) error {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	err := dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setting up datacenter")
	}
	vhost, err := dc.findHost(cluster, host)
	if err != nil {
		return errors.Wrap(err, "Error finding host")
	}

	return vhost.AddVswitch(vspec)
}

//AddKernelNic add a vswitch to host
func (dc *DataCenter) AddKernelNic(cluster, host string, nwSpec KernelNetworkSpec) error {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	err := dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setting up datacenter")
	}
	vhost, err := dc.findHost(cluster, host)
	if err != nil {
		return errors.Wrap(err, "Error finding host")
	}

	return vhost.AddKernelNic(nwSpec)
}

//RemoveKernelNic add a vswitch to host
func (dc *DataCenter) RemoveKernelNic(cluster, host string, pgName string) error {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	err := dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setting up datacenter")
	}
	vhost, err := dc.findHost(cluster, host)
	if err != nil {
		return errors.Wrap(err, "Error finding host")
	}

	return vhost.RemoveKernelNic(pgName)
}

//AddNetworks add a vswitch to host
func (dc *DataCenter) AddNetworks(cluster, host string, specs []NWSpec, vsSpec VswitchSpec) error {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	err := dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setting up datacenter")
	}
	vhost, err := dc.findHost(cluster, host)
	if err != nil {
		return errors.Wrap(err, "Error finding host")
	}

	return vhost.AddNetworks(specs, vsSpec)
}

//RemoveNetworks removes networks
func (dc *DataCenter) RemoveNetworks(cluster, host string, specs []NWSpec) error {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	err := dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setting up datacenter")
	}
	vhost, err := dc.findHost(cluster, host)
	if err != nil {
		return errors.Wrap(err, "Error finding host")
	}

	return vhost.RemoveNetworks(specs)
}

//ListVswitches lists all vswitces
func (dc *DataCenter) ListVswitches(cluster, host string) ([]VswitchSpec, error) {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	err := dc.setUpFinder()
	if err != nil {
		return nil, errors.Wrapf(err, "Error setup datacenter")
	}
	vhost, err := dc.findHost(cluster, host)
	if err != nil {
		return nil, errors.Wrap(err, "Error finding host")
	}

	return vhost.ListVswitchs()
}

type DVSwitchHostSpec struct {
	Name  string
	Pnics []string
}

type DvsPortGroup struct {
	Name         string
	Ports        int32
	Type         string
	VlanOverride bool
	Vlan         int32
	Private      bool
}

// VswitchSpec specifies a virtual switch
type DVSwitchSpec struct {
	Name       string
	Cluster    string
	Hosts      []DVSwitchHostSpec
	MaxPorts   int32
	Pvlans     []DvsPvlanPair
	PortGroups []DvsPortGroup
	Version    string
}

type DvsPvlanPair struct {
	Primary   int32
	Secondary int32
	Type      string
}

func (dc *DataCenter) findDvs(name string) (*object.DistributedVirtualSwitch, error) {
	net, err := dc.Finder().Network(dc.vc.Ctx(), name)
	if err != nil {
		return nil, err
	}

	dvs, ok := net.(*object.DistributedVirtualSwitch)
	if !ok {
		return nil, errors.New("cannot find dvs")
	}

	return dvs, nil
}

// AddHostsToDvs add hosts to dvs
func (dc *DataCenter) AddHostsToDvs(vspec DVSwitchSpec) error {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	dvs, err := dc.findDvs(vspec.Name)
	if err != nil {
		return errors.Wrap(err, "Dvs not found")
	}

	if len(vspec.Hosts) == 0 {
		fmt.Printf("no hosts added 1 \n")
		return nil
	}
	var s mo.DistributedVirtualSwitch
	err = dvs.Properties(dc.Ctx(), dvs.Reference(), []string{"config"}, &s)
	if err != nil {
		return err
	}

	existing := make(map[string]bool)
	// TODO: host.pnic.info command
	for _, member := range s.Config.GetDVSConfigInfo().Host {
		existing[member.Config.Host.Value] = true
	}

	config := &types.DVSConfigSpec{
		ConfigVersion: s.Config.GetDVSConfigInfo().ConfigVersion,
	}

	for _, host := range vspec.Hosts {

		hostRef, err := dc.findHost(vspec.Cluster, host.Name)
		if err != nil {
			return err
		}

		ref := hostRef.Host.hs.Reference()
		if existing[ref.Value] {
			//fmt.Fprintf(os.Stderr, "%s is already a member of %s\n", host.InventoryPath, dvs.InventoryPath)
			fmt.Printf("no already added \n")
			continue
		}

		hostConfig := types.DistributedVirtualSwitchHostMemberConfigSpec{
			Host:      hostRef.Host.hs.Reference(),
			Operation: string(types.ConfigSpecOperationAdd),
		}
		backing := new(types.DistributedVirtualSwitchHostMemberPnicBacking)
		for _, pnic := range host.Pnics {
			backing.PnicSpec = append(backing.PnicSpec,
				types.DistributedVirtualSwitchHostMemberPnicSpec{PnicDevice: pnic})
		}
		hostConfig.Backing = backing
		config.Host = append(config.Host, hostConfig)
	}

	if len(config.Host) == 0 {
		return nil
	}

	task, err := dvs.Reconfigure(dc.Ctx(), config)
	if err != nil {
		return err
	}

	_, err = task.WaitForResult(dc.Ctx())
	return err
}

// DisconnectAllHostFromDvs add hosts to dvs
func (dc *DataCenter) DisconnectAllHostFromDvs(vspec DVSwitchSpec) error {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	dvs, err := dc.findDvs(vspec.Name)
	if err != nil {
		//DVs not found return
		return nil
	}

	var s mo.DistributedVirtualSwitch
	err = dvs.Properties(dc.Ctx(), dvs.Reference(), []string{"config"}, &s)
	if err != nil {
		return err
	}

	existing := make(map[string]bool)
	// TODO: host.pnic.info command
	for _, member := range s.Config.GetDVSConfigInfo().Host {
		existing[member.Config.Host.Value] = true
	}

	config := &types.DVSConfigSpec{
		ConfigVersion: s.Config.GetDVSConfigInfo().ConfigVersion,
	}

	for value, exist := range existing {

		if exist {
			hostConfig := types.DistributedVirtualSwitchHostMemberConfigSpec{
				Host:      types.ManagedObjectReference{Value: value, Type: "HostSystem"},
				Operation: string(types.ConfigSpecOperationRemove),
			}
			config.Host = append(config.Host, hostConfig)
		}

	}

	if len(config.Host) == 0 {
		return nil
	}

	task, err := dvs.Reconfigure(dc.Ctx(), config)
	if err != nil {
		return err
	}

	_, err = task.WaitForResult(dc.Ctx())
	return err
}

// AddPvlanPairsToDvs adds pvlan to dvs
func (dc *DataCenter) AddPvlanPairsToDvs(name string, pairs []DvsPvlanPair) error {

	dc.getClientWithLock()
	defer dc.releaseClientLock()
	dvs, err := dc.findDvs(name)
	if err != nil {
		return errors.Wrap(err, "Dvs not found")
	}

	var s mo.DistributedVirtualSwitch
	err = dvs.Properties(dc.Ctx(), dvs.Reference(), []string{"config"}, &s)
	if err != nil {
		return err
	}

	config := &types.VMwareDVSConfigSpec{
		PvlanConfigSpec: []types.VMwareDVSPvlanConfigSpec{},
		DVSConfigSpec: types.DVSConfigSpec{
			ConfigVersion: s.Config.GetDVSConfigInfo().ConfigVersion,
		},
	}

	for _, pair := range pairs {
		config.PvlanConfigSpec = append(config.PvlanConfigSpec,
			types.VMwareDVSPvlanConfigSpec{
				PvlanEntry: types.VMwareDVSPvlanMapEntry{
					PrimaryVlanId:   pair.Primary,
					PvlanType:       pair.Type,
					SecondaryVlanId: pair.Secondary,
				},
				Operation: string(types.ConfigSpecOperationAdd),
			})
	}
	task, err := dvs.Reconfigure(dc.Ctx(), config)
	if err != nil {
		return err
	}

	_, err = task.WaitForResult(dc.Ctx())
	return err
}

//VlanType pvlan/trunk
type VlanType int

const (
	//DvsVlanID ID
	DvsVlanID VlanType = iota
	// DvsVlanTrunk trunk
	DvsVlanTrunk
	//DvsPvlan trunk
	DvsPvlan

	//PgName match
	PgName
)

//DvsMatchCriteria
type DvsPGMatchCriteria struct {
	VlanID int32
	Type   VlanType
	Uplink bool
	PgName string
}

//FindDvsPortGroup find dvs matching criteria
func (dc *DataCenter) FindDvsPortGroup(name string, mcriteria DvsPGMatchCriteria) (string, error) {

	err := dc.setUpFinder()
	if err != nil {
		return "", errors.Wrapf(err, "Error setup datacenter")
	}

	dvs, err := dc.findDvs(name)
	if err != nil {
		return "", errors.Wrap(err, "Dvs not found")
	}

	getNwName := func(uuid, key string) (string, error) {
		netList, err := dc.Finder().NetworkList(dc.vc.Ctx(), "*")
		if err != nil {
			return "", errors.Wrap(err, "Failed Fetch networks")
		}

		for _, net := range netList {
			nwRef, err := net.EthernetCardBackingInfo(dc.Ctx())
			if err != nil {
				continue
			}
			switch nw := nwRef.(type) {
			case *types.VirtualEthernetCardDistributedVirtualPortBackingInfo:
				if nw.Port.SwitchUuid == uuid && nw.Port.PortgroupKey == key {
					return net.GetInventoryPath(), nil
				}
			}

		}
		return "", errors.Wrap(err, "Failed Fetch networks")
	}
	// Set base search criteria
	criteria := types.DistributedVirtualSwitchPortCriteria{
		UplinkPort: &mcriteria.Uplink,

		//Connected: types.NewBool(true),

		//Active:     types.NewBool(cmd.active),
		//UplinkPort: types.NewBool(cmd.uplinkPort),
		//Inside:     types.NewBool(cmd.inside),
	}
	res, err := dvs.FetchDVPorts(dc.vc.Ctx(), &criteria)
	if err != nil {
		return "", err
	}

	for _, port := range res {
		var vlanID int32
		setting := port.Config.Setting.(*types.VMwareDVSPortSetting)

		switch vlan := setting.Vlan.(type) {
		case *types.VmwareDistributedVirtualSwitchVlanIdSpec:
			if mcriteria.Type == DvsVlanID && vlan.VlanId == mcriteria.VlanID {
				return getNwName(port.DvsUuid, port.PortgroupKey)
			}
			vlanID = vlan.VlanId
		case *types.VmwareDistributedVirtualSwitchTrunkVlanSpec:
			if mcriteria.Type == DvsVlanTrunk {
				for _, vlan := range vlan.VlanId {
					if vlan.Start >= mcriteria.VlanID && vlan.End <= mcriteria.VlanID {
						return getNwName(port.DvsUuid, port.PortgroupKey)
					}
				}
			}
		case *types.VmwareDistributedVirtualSwitchPvlanSpec:
			vlanID = vlan.PvlanId
			if mcriteria.Type == DvsVlanTrunk && vlanID == mcriteria.VlanID {
				return getNwName(port.DvsUuid, port.PortgroupKey)
			}
			if mcriteria.Type == DvsPvlan && vlanID == mcriteria.VlanID {
				return getNwName(port.DvsUuid, port.PortgroupKey)
			}

		}

	}

	return "", errors.New("Portgroup not found")
}

func (dc *DataCenter) FetchDVPortGroupsNames(name string) ([]string, error) {
	err := dc.setUpFinder()
	if err != nil {
		return nil, errors.Wrapf(err, "Error setup datacenter")
	}

	dvs, err := dc.findDvs(name)
	if err != nil {
		return nil, errors.Wrap(err, "Dvs not found")
	}

	// Set base search criteria
	//active := true
	criteria := types.DistributedVirtualSwitchPortCriteria{
		//UplinkPort: &mcriteria.Uplink,
		//Connected:  &active,

		//Connected: types.NewBool(true),

		//Active:     types.NewBool(cmd.active),
		//UplinkPort: types.NewBool(cmd.uplinkPort),
		//Inside:     types.NewBool(cmd.inside),
	}
	res, err := dvs.FetchDVPorts(dc.vc.Ctx(), &criteria)
	if err != nil {
		return nil, err
	}
	netList, err := dc.Finder().NetworkList(dc.vc.Ctx(), "*")
	if err != nil {
		return nil, errors.Wrap(err, "Failed Fetch networks")
	}

	nwKey := func(uuid, key string) string {
		return uuid + "-" + key
	}
	nwMap := make(map[string]string)
	for _, net := range netList {
		nwRef, err := net.EthernetCardBackingInfo(dc.Ctx())
		if err != nil {
			continue
		}
		switch nw := nwRef.(type) {
		case *types.VirtualEthernetCardDistributedVirtualPortBackingInfo:
			nwMap[nwKey(nw.Port.SwitchUuid, nw.Port.PortgroupKey)] = filepath.Base(net.GetInventoryPath())
		}

	}

	nws := make(map[string]bool)
	log.Infof("Number od DV Ports %v %v", len(res), len(nwMap))
	for _, port := range res {
		setting := port.Config.Setting.(*types.VMwareDVSPortSetting)

		switch setting.Vlan.(type) {
		case *types.VmwareDistributedVirtualSwitchVlanIdSpec:
			pgName, ok := nwMap[nwKey(port.DvsUuid, port.PortgroupKey)]
			if ok {
				nws[pgName] = true
			}
		case *types.VmwareDistributedVirtualSwitchPvlanSpec:
			pgName, ok := nwMap[nwKey(port.DvsUuid, port.PortgroupKey)]
			if ok {
				nws[pgName] = true
			}

		}

	}

	pgs := []string{}
	for pg, _ := range nws {
		pgs = append(pgs, pg)
	}
	return pgs, nil
}

// RemovePG removes the pg with the given name
func (dc *DataCenter) RemovePG(pgName string) error {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()

	err := dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setup datacenter")
	}

	net, err := dc.Finder().Network(dc.vc.Ctx(), pgName)
	if err != nil {
		return errors.Wrap(err, "Failed Fetch network")
	}

	objPg, ok := net.(*object.DistributedVirtualPortgroup)
	if !ok {
		return errors.New("Failed at getting dvs port group object")
	}

	fmt.Printf("PG %v", objPg)

	task, err := objPg.Destroy(dc.vc.Ctx())
	if err != nil {
		// Failed to delete PG
		// TODO: Generate Event and mark object?
		return err
	}

	_, err = task.WaitForResult(dc.vc.Ctx(), nil)
	if err != nil {
		// Failed to delete PG
		// TODO: Generate Event and mark object?
		return err
	}

	return nil
}

// RemoveAllPortGroupsFromDvs removes the pg with the given name
func (dc *DataCenter) RemoveAllPortGroupsFromDvs(switchName string) error {

	pgs, err := dc.FetchDVPortGroupsNames(switchName)
	if err != nil {
		return err
	}

	for _, pg := range pgs {
		err = dc.RemovePG(pg)
		if err != nil {
			return err
		}

	}

	return nil
}

//FetchDVPorts fetches port keys matching
func (dc *DataCenter) FetchDVPorts(name string, mcriteria DvsPGMatchCriteria) ([]types.DistributedVirtualPort, error) {

	err := dc.setUpFinder()
	if err != nil {
		return nil, errors.Wrapf(err, "Error setup datacenter")
	}

	dvs, err := dc.findDvs(name)
	if err != nil {
		return nil, errors.Wrap(err, "Dvs not found")
	}

	// Set base search criteria
	//active := true
	criteria := types.DistributedVirtualSwitchPortCriteria{
		//UplinkPort: &mcriteria.Uplink,
		//Connected:  &active,

		//Connected: types.NewBool(true),

		//Active:     types.NewBool(cmd.active),
		//UplinkPort: types.NewBool(cmd.uplinkPort),
		//Inside:     types.NewBool(cmd.inside),
	}
	res, err := dvs.FetchDVPorts(dc.vc.Ctx(), &criteria)
	if err != nil {
		return nil, err
	}
	netList, err := dc.Finder().NetworkList(dc.vc.Ctx(), "*")
	if err != nil {
		return nil, errors.Wrap(err, "Failed Fetch networks")
	}

	nwKey := func(uuid, key string) string {
		return uuid + "-" + key
	}
	nwMap := make(map[string]string)
	for _, net := range netList {
		nwRef, err := net.EthernetCardBackingInfo(dc.Ctx())
		if err != nil {
			continue
		}
		switch nw := nwRef.(type) {
		case *types.VirtualEthernetCardDistributedVirtualPortBackingInfo:
			nwMap[nwKey(nw.Port.SwitchUuid, nw.Port.PortgroupKey)] = filepath.Base(net.GetInventoryPath())
		}

	}

	portKeys := []types.DistributedVirtualPort{}
	log.Infof("Number od DV Ports %v %v", len(res), len(nwMap))
	for _, port := range res {
		var vlanID int32
		setting := port.Config.Setting.(*types.VMwareDVSPortSetting)

		switch vlan := setting.Vlan.(type) {
		case *types.VmwareDistributedVirtualSwitchVlanIdSpec:
			if mcriteria.Type == DvsVlanID && vlan.VlanId == mcriteria.VlanID {
				portKeys = append(portKeys, port)
			}
			if mcriteria.Type == PgName {
				pgName, ok := nwMap[nwKey(port.DvsUuid, port.PortgroupKey)]
				if ok && pgName == mcriteria.PgName {
					portKeys = append(portKeys, port)
				}
			}
			vlanID = vlan.VlanId
		case *types.VmwareDistributedVirtualSwitchTrunkVlanSpec:
			if mcriteria.Type == DvsVlanTrunk {
				for _, vlan := range vlan.VlanId {
					if vlan.Start >= mcriteria.VlanID && vlan.End <= mcriteria.VlanID {
						portKeys = append(portKeys, port)
					}
				}
			}
		case *types.VmwareDistributedVirtualSwitchPvlanSpec:
			vlanID = vlan.PvlanId
			if mcriteria.Type == DvsVlanTrunk && vlanID == mcriteria.VlanID {
				portKeys = append(portKeys, port)
			}
			if mcriteria.Type == DvsPvlan && vlanID == mcriteria.VlanID {
				portKeys = append(portKeys, port)
			}

			if mcriteria.Type == PgName {
				pgName, ok := nwMap[nwKey(port.DvsUuid, port.PortgroupKey)]
				if ok && pgName == mcriteria.PgName {
					portKeys = append(portKeys, port)
				}
			}

		}

	}

	return portKeys, nil
}

// AddPortGroupToDvs adds port group to dvs
func (dc *DataCenter) AddPortGroupToDvs(name string, pairs []DvsPortGroup) error {

	dc.getClientWithLock()
	defer dc.releaseClientLock()
	if len(pairs) == 0 {
		return nil
	}

	dvs, err := dc.findDvs(name)
	if err != nil {
		return errors.Wrap(err, "Dvs not found")
	}

	var s mo.DistributedVirtualSwitch
	err = dvs.Properties(dc.Ctx(), dvs.Reference(), []string{"config"}, &s)
	if err != nil {
		return err
	}

	pgSpec := []types.DVPortgroupConfigSpec{}
	allow := true
	for _, pair := range pairs {

		spec := types.DVPortgroupConfigSpec{
			Name:     pair.Name,
			NumPorts: pair.Ports,
			Type:     pair.Type,
			DefaultPortConfig: &types.VMwareDVSPortSetting{
				SecurityPolicy: &types.DVSSecurityPolicy{
					MacChanges: &types.BoolPolicy{Value: &allow, InheritablePolicy: types.InheritablePolicy{
						Inherited: true,
					}},
					AllowPromiscuous: &types.BoolPolicy{Value: &allow, InheritablePolicy: types.InheritablePolicy{
						Inherited: true,
					}},
					ForgedTransmits: &types.BoolPolicy{Value: &allow, InheritablePolicy: types.InheritablePolicy{
						Inherited: true,
					}},
					InheritablePolicy: types.InheritablePolicy{
						Inherited: true,
					},
				},
			},
			Policy: &types.VMwareDVSPortgroupPolicy{
				DVPortgroupPolicy: types.DVPortgroupPolicy{
					//BlockOverrideAllowed:               true,
					//ShapingOverrideAllowed:             false,
					VendorConfigOverrideAllowed: true,
					LivePortMovingAllowed:       true,
					PortConfigResetAtDisconnect: true,

					//NetworkResourcePoolOverrideAllowed: types.NewBool(false),
					//TrafficFilterOverrideAllowed:       types.NewBool(false),
				},
				SecurityPolicyOverrideAllowed: true,
				VlanOverrideAllowed:           pair.VlanOverride,
				//UplinkTeamingOverrideAllowed:  false,
				//SecurityPolicyOverrideAllowed: false,
			},
		}

		if pair.Private {
			spec.DefaultPortConfig = &types.VMwareDVSPortSetting{
				Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
					PvlanId: pair.Vlan,
				},
			}
		} else {
			spec.DefaultPortConfig = &types.VMwareDVSPortSetting{
				Vlan: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
					VlanId: pair.Vlan,
				},
			}
		}

		pgSpec = append(pgSpec, spec)

	}
	task, err := dvs.AddPortgroup(dc.Ctx(), pgSpec)
	if err != nil && !strings.Contains(err.Error(), "already exists") {
		return err
	}

	_, err = task.WaitForResult(dc.Ctx())
	if err != nil && !strings.Contains(err.Error(), "already exists") {
		return err
	}

	pSpec := []types.DVPortConfigSpec{}
	for _, pair := range pairs {
		vType := DvsPvlan
		if !pair.Private {
			vType = DvsVlanID
		}
		ports, err := dc.FetchDVPorts(name, DvsPGMatchCriteria{Type: vType, VlanID: pair.Vlan})
		if err != nil {
			return err
		}

		for _, port := range ports {
			spec := types.DVPortConfigSpec{
				Key: port.Key,
				//Name:          name,
				Scope:         port.Config.Scope,
				ConfigVersion: port.Config.ConfigVersion,
				Operation:     "edit",
				Setting: &types.VMwareDVSPortSetting{
					SecurityPolicy: &types.DVSSecurityPolicy{
						MacChanges:       &types.BoolPolicy{Value: &allow},
						ForgedTransmits:  &types.BoolPolicy{Value: &allow},
						AllowPromiscuous: &types.BoolPolicy{Value: &allow},
					},
				},
			}
			pSpec = append(pSpec, spec)
		}
	}
	task, err = dvs.ReconfigureDVPort(dc.Ctx(), pSpec)
	if err != nil {
		return err
	}
	_, err = task.WaitForResult(dc.Ctx())
	return err
}

//ReconfigureVMNetwork reconfigures VM network
func (dc *DataCenter) ReconfigureVMNetwork(vm *VM, currNW string, switcName string, newNW string, maxReconfigs int, relaxSecurity bool) error {
	dc.getClientWithLock()
	defer dc.releaseClientLock()
	if err := vm.ReconfigureNetwork(currNW, newNW, maxReconfigs); err != nil {
		return err
	}

	if relaxSecurity {
		return dc.relaxSecurityOnPg(switcName, newNW)
	}

	return nil
}

// RelaxSecurityOnPg relaxes security on PG
func (dc *DataCenter) relaxSecurityOnPg(name string, pgName string) error {

	dvs, err := dc.findDvs(name)
	if err != nil {
		return errors.Wrap(err, "Dvs not found")
	}

	var s mo.DistributedVirtualSwitch
	err = dvs.Properties(dc.Ctx(), dvs.Reference(), []string{"config"}, &s)
	if err != nil {
		return err
	}

	net, err := dc.Finder().Network(dc.Ctx(), pgName)
	if err != nil {
		return fmt.Errorf("Port group for %s is not present", pgName)
	}

	objPg, ok := net.(*object.DistributedVirtualPortgroup)
	if !ok {
		return fmt.Errorf("Failed at getting dvs port group object %v", pgName)
	}

	var dvsPg mo.DistributedVirtualPortgroup
	err = objPg.Properties(dc.Ctx(), objPg.Reference(), []string{"config"}, &dvsPg)
	if err != nil {
		return err
	}

	allow := true
	spec := types.DVPortgroupConfigSpec{
		Name:          dvsPg.Name,
		ConfigVersion: dvsPg.Config.ConfigVersion,
		DefaultPortConfig: &types.VMwareDVSPortSetting{
			SecurityPolicy: &types.DVSSecurityPolicy{
				MacChanges: &types.BoolPolicy{Value: &allow, InheritablePolicy: types.InheritablePolicy{
					Inherited: true,
				}},
				AllowPromiscuous: &types.BoolPolicy{Value: &allow, InheritablePolicy: types.InheritablePolicy{
					Inherited: true,
				}},
				ForgedTransmits: &types.BoolPolicy{Value: &allow, InheritablePolicy: types.InheritablePolicy{
					Inherited: true,
				}},
				InheritablePolicy: types.InheritablePolicy{
					Inherited: true,
				},
			},
		},
		Policy: &types.VMwareDVSPortgroupPolicy{
			DVPortgroupPolicy: types.DVPortgroupPolicy{
				BlockOverrideAllowed:               true,
				ShapingOverrideAllowed:             false,
				VendorConfigOverrideAllowed:        false,
				LivePortMovingAllowed:              false,
				PortConfigResetAtDisconnect:        true,
				NetworkResourcePoolOverrideAllowed: types.NewBool(false),
				TrafficFilterOverrideAllowed:       types.NewBool(false),
			},
			VlanOverrideAllowed:           true,
			UplinkTeamingOverrideAllowed:  false,
			SecurityPolicyOverrideAllowed: true,
			IpfixOverrideAllowed:          types.NewBool(false),
		},
	}

	task, err := objPg.Reconfigure(dc.Ctx(), spec)

	_, err = task.WaitForResult(dc.Ctx(), nil)
	if err != nil {
		return fmt.Errorf("Error reconfiguring %v", pgName)
	}

	pSpec := []types.DVPortConfigSpec{}
	ports, err := dc.FetchDVPorts(name, DvsPGMatchCriteria{Type: PgName, PgName: pgName})
	if err != nil {
		return err
	}

	if len(ports) != 0 {
		for _, port := range ports {
			spec := types.DVPortConfigSpec{
				Key: port.Key,
				//Name:          name,
				Scope:         port.Config.Scope,
				ConfigVersion: port.Config.ConfigVersion,
				Operation:     "edit",
				Setting: &types.VMwareDVSPortSetting{
					SecurityPolicy: &types.DVSSecurityPolicy{
						MacChanges:       &types.BoolPolicy{Value: &allow},
						ForgedTransmits:  &types.BoolPolicy{Value: &allow},
						AllowPromiscuous: &types.BoolPolicy{Value: &allow},
					},
				},
			}
			pSpec = append(pSpec, spec)
		}
		task, err = dvs.ReconfigureDVPort(dc.Ctx(), pSpec)
		if err != nil {
			return err
		}
		_, err = task.WaitForResult(dc.Ctx())
		return err
	}

	return nil
}

// RemoveHostsFromDvs remove hosts from dvs
func (dc *DataCenter) RemoveHostsFromDvs(vspec DVSwitchSpec) error {

	if len(vspec.Hosts) == 0 {
		return nil
	}

	dvs, err := dc.findDvs(vspec.Name)
	if err != nil {
		return errors.Wrap(err, "Dvs not found")
	}

	var s mo.DistributedVirtualSwitch
	err = dvs.Properties(dc.Ctx(), dvs.Reference(), []string{"config"}, &s)
	if err != nil {
		return err
	}

	existing := make(map[string]bool)
	for _, member := range s.Config.GetDVSConfigInfo().Host {
		existing[member.Config.Host.Value] = true
	}

	config := &types.DVSConfigSpec{
		ConfigVersion: s.Config.GetDVSConfigInfo().ConfigVersion,
	}

	for _, host := range vspec.Hosts {
		hostRef, err := dc.findHost(vspec.Cluster, host.Name)
		if err != nil {
			return err
		}
		ref := hostRef.Host.hs.Reference()
		if !existing[ref.Value] {
			continue
		}
		hostConfig := types.DistributedVirtualSwitchHostMemberConfigSpec{
			Host:      hostRef.Host.hs.Reference(),
			Operation: string(types.ConfigSpecOperationRemove),
		}
		config.Host = append(config.Host, hostConfig)
	}

	if len(config.Host) == 0 {
		return nil
	}

	task, err := dvs.Reconfigure(dc.Ctx(), config)
	if err != nil {
		return err
	}

	_, err = task.WaitForResult(dc.Ctx())
	return err
}

// AddDvs adds vswitch to target host
func (dc *DataCenter) AddDvs(vspec DVSwitchSpec) error {

	_, err := dc.findDvs(vspec.Name)
	if err != nil {

		pvlanConfigSpec := []types.VMwareDVSPvlanConfigSpec{}
		for _, pair := range vspec.Pvlans {
			pvlanConfigSpec = append(pvlanConfigSpec,
				types.VMwareDVSPvlanConfigSpec{
					PvlanEntry: types.VMwareDVSPvlanMapEntry{
						PrimaryVlanId:   pair.Primary,
						PvlanType:       pair.Type,
						SecondaryVlanId: pair.Secondary,
					},
					Operation: "add",
				})
		}

		spec := types.DVSCreateSpec{
			ConfigSpec: &types.VMwareDVSConfigSpec{
				DVSConfigSpec: types.DVSConfigSpec{
					Name:               vspec.Name,
					NumStandalonePorts: vspec.MaxPorts,
				},
				PvlanConfigSpec: pvlanConfigSpec,
			},
			ProductInfo: &types.DistributedVirtualSwitchProductSpec{
				Version: vspec.Version,
			},
		}

		folders, _ := dc.ref.Folders(dc.Ctx())

		dtask, err := folders.NetworkFolder.CreateDVS(dc.Ctx(), spec)
		if err != nil {
			return err
		}

		_, err = dtask.WaitForResult(dc.Ctx(), nil)
		if err != nil {
			return err
		}
	}

	err = dc.AddPortGroupToDvs(vspec.Name, vspec.PortGroups)
	if err != nil {
		return err
	}
	return dc.AddHostsToDvs(vspec)
}

//DeleteAllHosts delete all hosts from datacenter
func (dc *DataCenter) DeleteAllHosts() error {

	hosts, err := dc.Finder().HostSystemList(dc.vc.Ctx(), "*")
	if err == nil {
		for _, host := range hosts {
			remove := host.Destroy

			task, err := host.Disconnect(dc.vc.Ctx())
			if err != nil {
				return err
			}
			_, err = task.WaitForResult(dc.vc.Ctx(), nil)
			if err != nil {
				return err
			}
			task, err = remove(dc.vc.Ctx())
			if err != nil {
				return err
			}

			_, err = task.WaitForResult(dc.vc.Ctx(), nil)
			if err != nil {
				return err
			}

		}
	}
	return nil
}

//DeleteAllVMs delete all VMS from datacenter
func (dc *DataCenter) DeleteAllVMs() error {

	vms, err := dc.Finder().VirtualMachineList(dc.vc.Ctx(), "*")
	if err == nil {
		for _, vm := range vms {
			task, err := vm.PowerOff(dc.Ctx())
			if err == nil {
				task.WaitForResult(dc.Ctx())
			}
			task, err = vm.Destroy(dc.Ctx())
			if err == nil {
				task.WaitForResult(dc.Ctx())
			}
		}
	}
	return nil
}

func (dc *DataCenter) getDatastoreRefForHost(hostName string) (*types.ManagedObjectReference, error) {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()

	var hosts []mo.HostSystem
	cView, err := dc.vc.ConnCtx.viewMgr.CreateContainerView(dc.vc.Ctx(), dc.ref.Reference(), []string{}, true)
	if err != nil {
		return nil, err
	}
	err = cView.Retrieve(dc.vc.Ctx(), []string{string("HostSystem")}, []string{"config", "name", "datastore"}, &hosts)
	if err != nil {
		return nil, err
	}

	for _, host := range hosts {
		if host.Name == hostName {
			for _, ds := range host.Datastore {
				ref := ds.Reference()
				return &ref, nil
			}

		}
	}

	return nil, fmt.Errorf("Host storage not found %v", hostName)

}

//LiveMigrate migrates live VM on data center
func (dc *DataCenter) LiveMigrate(vmName, srcHostName, dstHostName, clusterName string, abortTime int) error {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()
	err := dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setting up datacenter")
	}

	srcHostRef, err := dc.findHost(clusterName, srcHostName)
	if err != nil {
		return err
	}

	dstHostRef, err := dc.findHost(clusterName, dstHostName)
	if err != nil {
		return err
	}

	vmInst, err := dc.findVM(srcHostRef.Host.hs, vmName)
	if err != nil {
		return err
	}

	dstHostDatastoreRef, err := dc.getDatastoreRefForHost(dstHostName)
	if err != nil {
		return err
	}

	return vmInst.Migrate(&dstHostRef.Host, dstHostDatastoreRef, abortTime)
}

func (dc *DataCenter) BootVM(name string) (*VMInfo, error) {

	dc.getClientWithRLock()
	defer dc.releaseClientRLock()

	return dc.vc.BootVM(name)
}

func (dc *DataCenter) DestoryVM(name string) error {

	log.Infof("Destroy vm with %v", dc.vc)
	dc.getClientWithRLock()
	defer dc.releaseClientRLock()

	return dc.vc.DestoryVM(name)
}

func (dc *DataCenter) NewVM(name string) (*VM, error) {
	dc.getClientWithRLock()
	defer dc.releaseClientRLock()

	return dc.vc.NewVM(name)
}

func (dc *DataCenter) VMExists(name string) bool {
	dc.getClientWithRLock()
	defer dc.releaseClientRLock()

	return dc.vc.VMExists(name)
}

//SetVlanOverride sets vlan override for the VM
func (dc *DataCenter) SetVlanOverride(dvsName string, vmName string, accessVlan, overridevlan int) error {

	err := dc.setUpFinder()
	if err != nil {
		return errors.Wrapf(err, "Error setup datacenter")
	}

	vm, err := dc.NewVM(vmName)
	if err != nil {
		return errors.Wrap(err, "Error finding VM")
	}

	dc.getClientWithLock()
	defer dc.releaseClientLock()

	dvs, err := dc.findDvs(dvsName)
	if err != nil {
		return errors.Wrap(err, "Dvs not found")
	}

	devList, err := vm.vm.Device(vm.entity.Ctx())
	if err != nil {
		return errors.Wrap(err, "Failed to device list of VM")
	}

	// Set base search criteria
	criteria := types.DistributedVirtualSwitchPortCriteria{}
	res, err := dvs.FetchDVPorts(dc.vc.Ctx(), &criteria)
	if err != nil {
		return err
	}

	allow := true
	pSpec := []types.DVPortConfigSpec{}
	for _, d := range devList.SelectByType((*types.VirtualEthernetCard)(nil)) {
		veth := d.GetVirtualDevice()

		switch a := veth.Backing.(type) {
		case *types.VirtualEthernetCardDistributedVirtualPortBackingInfo:
			for _, port := range res {
				setting, ok := port.Config.Setting.(*types.VMwareDVSPortSetting)
				if !ok {
					continue
				}
				vlanA, ok := setting.Vlan.(*types.VmwareDistributedVirtualSwitchVlanIdSpec)
				if !ok {
					continue
				}
				if vlanA.VlanId == int32(accessVlan) && port.DvsUuid == a.Port.SwitchUuid &&
					port.Key == a.Port.PortKey {

					fmt.Printf("Found Matching port %v %v", port.Key, port.PortgroupKey)

					spec := types.DVPortConfigSpec{
						Key: port.Key,
						//Name:          name,
						Scope:         port.Config.Scope,
						ConfigVersion: port.Config.ConfigVersion,
						Operation:     "edit",
						Setting: &types.VMwareDVSPortSetting{
							SecurityPolicy: &types.DVSSecurityPolicy{
								MacChanges:       &types.BoolPolicy{Value: &allow},
								ForgedTransmits:  &types.BoolPolicy{Value: &allow},
								AllowPromiscuous: &types.BoolPolicy{Value: &allow},
							},
							Vlan: &types.VmwareDistributedVirtualSwitchVlanIdSpec{
								VlanId: int32(overridevlan),
							},
						},
					}
					pSpec = append(pSpec, spec)

				}
			}
		}

	}

	if len(pSpec) != 0 {
		task, err := dvs.ReconfigureDVPort(dc.Ctx(), pSpec)
		if err != nil {
			return err
		}

		if err != nil {
			return err
		}
		_, err = task.WaitForResult(dc.Ctx())
		if err != nil {
			return err
		}
	}
	return nil
}
