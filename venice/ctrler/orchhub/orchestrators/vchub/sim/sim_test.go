package sim

import (
	"context"
	"sort"
	"strings"
	"testing"
	"time"

	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/property"
	"github.com/vmware/govmomi/view"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	. "github.com/pensando/sw/venice/utils/testutils"
)

type watchObj struct {
	Name    string
	Changes []types.PropertyChange
}

func TestList(t *testing.T) {
	vcID := "user:pass@127.0.0.1:8990"
	s, err := NewVcSim(Config{Addr: vcID})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()

	dc, err := s.AddDC("dc1")
	AssertOk(t, err, "failed to create DC")
	host, err := dc.AddHost("host1")
	AssertOk(t, err, "failed to create host1")
	host2, err := dc.AddHost("host2")
	AssertOk(t, err, "failed to create host2")
	host.ClearNics()
	host.ClearVmkNics()
	err = host.AddNic("testNIC", "aaaa:bbbb:cccc")
	AssertOk(t, err, "failed to add nic")
	vnicMac := "aaaa.bbbb.cccc"
	vnicMac2 := "aaaa.bbbb.dddd"
	vnicMac2IP := "1.1.1.1"
	vm1, err := dc.AddVM("testVM", "host1", []VNIC{
		VNIC{
			MacAddress: vnicMac,
		},
	})
	AssertOk(t, err, "failed to create vm")
	dc.AddVnic(vm1, VNIC{
		MacAddress: vnicMac2,
	})
	dc.UpdateVMIP(vm1, vnicMac2, "", []string{vnicMac2IP})

	var spec types.HostVirtualNicSpec
	spec.Mac = "0011.2233.0001"
	var dvPort types.DistributedVirtualSwitchPortConnection
	dvPort.PortKey = "10" // use some port number
	spec.DistributedVirtualPort = &dvPort
	err = host.AddVmkNic(&spec, "vmk1")
	AssertOk(t, err, "failed to add vmkNic")
	host.RemoveVmkNic("vmk1")

	var dvsCreateSpec types.DVSCreateSpec
	var dvsConfigSpec types.DVSConfigSpec
	dvsConfigSpec.GetDVSConfigSpec().Name = "dvs1"
	dvsConfigSpec.GetDVSConfigSpec().NumStandalonePorts = 512
	dvsConfigSpec.GetDVSConfigSpec().MaxPorts = 4096
	dvsCreateSpec.ConfigSpec = &types.VMwareDVSConfigSpec{
		DVSConfigSpec: dvsConfigSpec,
	}
	_, err = dc.AddDVS(&dvsCreateSpec)
	AssertOk(t, err, "failed to create DVS")
	dvs, _ := dc.GetDVS("dvs1")
	Assert(t, dvs != nil, "GetDVS failed")
	err = dvs.AddHost(host)
	AssertOk(t, err, "Failed to add host from dvs")
	err = dvs.AddHost(host2)
	AssertOk(t, err, "Failed to add host from dvs")

	pgConfigSpec0 := []types.DVPortgroupConfigSpec{
		types.DVPortgroupConfigSpec{
			Name:     "pg1",
			NumPorts: 10,
		},
		types.DVPortgroupConfigSpec{
			Name:     "pg2",
			NumPorts: 10,
		},
	}
	_, err = dvs.AddPortgroup(pgConfigSpec0)
	AssertOk(t, err, "failed to add portgroup")

	pgConfigSpec1 := []types.DVPortgroupConfigSpec{
		types.DVPortgroupConfigSpec{
			Name:     "pg3",
			NumPorts: 10,
		},
		types.DVPortgroupConfigSpec{
			Name:     "pg4",
			NumPorts: 10,
		},
	}
	_, err = dvs.AddPortgroup(pgConfigSpec1)
	AssertOk(t, err, "failed to add portgroup")
	_ = dc.UpdateVMHost(vm1, "host2")

	ctx := context.Background()

	u := s.GetURL()

	c, err := govmomi.NewClient(ctx, u, true)
	if err != nil {
		t.Fatalf("err not nil %v", err)
	}

	// Check DCs
	var dcs []mo.Datacenter
	getKind(t, c, "Datacenter", []string{"name"}, &dcs)
	AssertEquals(t, 1, len(dcs), "Recieved incorrect amount of dcs")
	AssertEquals(t, "dc1", dcs[0].Name, "DC had incorret name")

	// Check Hosts
	var hss []mo.HostSystem
	getKind(t, c, "HostSystem", []string{"name", "config"}, &hss)
	AssertEquals(t, 2, len(hss), "Recieved incorrect amount of hosts")
	sort.Slice(hss, func(i, j int) bool {
		return hss[i].Name < hss[j].Name
	})
	AssertEquals(t, "host1", hss[0].Name, "host had incorrect name")
	hasNic := false
	for _, pnic := range hss[0].Config.Network.Pnic {
		if strings.Contains(pnic.Key, "testNIC") {
			hasNic = true
		}
	}
	Assert(t, hasNic == true, "Failed to find testNIC")

	// Remove nic
	host.RemoveNic("testNIC")
	var hss2 []mo.HostSystem
	getKind(t, c, "HostSystem", []string{"name", "config"}, &hss2)
	AssertEquals(t, 2, len(hss), "Recieved incorrect amount of dcs")
	sort.Slice(hss2, func(i, j int) bool {
		return hss2[i].Name < hss2[j].Name
	})
	AssertEquals(t, "host1", hss[0].Name, "host had incorrect name")
	hasNic = false
	for _, pnic := range hss2[0].Config.Network.Pnic {
		if strings.Contains(pnic.Key, "testNIC") {
			hasNic = true
		}
	}
	Assert(t, hasNic == false, "testNIC was not deleted")

	// Check VMs
	var vms []mo.VirtualMachine
	getKind(t, c, "VirtualMachine", []string{"name", "config", "guest"}, &vms)
	AssertEquals(t, 1, len(vms), "Recieved incorrect amount of vms")
	AssertEquals(t, "testVM", vms[0].Name, "VM had incorrect name")
	vnicCount := 0
	for _, d := range vms[0].Config.Hardware.Device {
		device, ok := d.(types.BaseVirtualEthernetCard)
		if !ok {
			continue
		}
		vnicCount++
		AssertOneOf(t, device.GetVirtualEthernetCard().MacAddress, []string{vnicMac, vnicMac2})
	}
	for _, d := range vms[0].Guest.Net {
		if d.MacAddress == vnicMac2 {
			AssertEquals(t, d.IpAddress, []string{vnicMac2IP}, "IP address did not match")
		}
	}
	AssertEquals(t, 2, vnicCount, "Expected VM to have two vnics")

	// Remove vnic
	err = dc.RemoveVnic(vm1, VNIC{
		MacAddress: vnicMac2,
	})
	AssertOk(t, err, "falied to remove vnic")
	dc.RemoveVMIP(vm1, vnicMac2)

	vms = []mo.VirtualMachine{}
	getKind(t, c, "VirtualMachine", []string{"name", "config"}, &vms)
	AssertEquals(t, 1, len(vms), "Recieved incorrect amount of vms")
	AssertEquals(t, "testVM", vms[0].Name, "VM had incorrect name")
	vnicCount = 0
	for _, d := range vms[0].Config.Hardware.Device {
		device, ok := d.(types.BaseVirtualEthernetCard)
		if !ok {
			continue
		}
		vnicCount++
		AssertOneOf(t, device.GetVirtualEthernetCard().MacAddress, []string{vnicMac})
	}
	AssertEquals(t, 1, vnicCount, "Expected VM to have one vnic")

	// Check DVSs
	var dvss []mo.DistributedVirtualSwitch
	getKind(t, c, "DistributedVirtualSwitch", []string{"name", "summary"}, &dvss)
	AssertEquals(t, "dvs1", dvss[0].Name, "DVS had incorrect name")
	AssertEquals(t, int32(512), dvss[0].Summary.NumPorts, "DVS had incorrect number of ports")

	var pgs []mo.DistributedVirtualPortgroup
	getKind(t, c, "DistributedVirtualPortgroup", []string{"name", "config"}, &pgs)
	AssertEquals(t, len(pgConfigSpec0)+len(pgConfigSpec1)+1, len(pgs), "Recieved incorrect amount of portgroups")
	// Skip pgs[0] since it's uplink portgroup that created by default
	AssertEquals(t, "pg1", pgs[1].Name, "Portgroup had incorrect name")
	AssertEquals(t, int32(10), pgs[1].Config.NumPorts, "Portgroup had incorrect number of ports")
	AssertEquals(t, "pg2", pgs[2].Name, "Portgroup had incorrect name")
	AssertEquals(t, int32(10), pgs[2].Config.NumPorts, "Portgroup had incorrect number of ports")

	AssertEquals(t, "pg3", pgs[3].Name, "Portgroup had incorrect name")
	AssertEquals(t, int32(10), pgs[3].Config.NumPorts, "Portgroup had incorrect number of ports")
	AssertEquals(t, "pg4", pgs[4].Name, "Portgroup had incorrect name")
	AssertEquals(t, int32(10), pgs[4].Config.NumPorts, "Portgroup had incorrect number of ports")

	err = dc.DeleteVM(vm1)
	AssertOk(t, err, "failed to destroy vm")
	vms = []mo.VirtualMachine{}
	getKind(t, c, "VirtualMachine", []string{"name", "config"}, &vms)
	AssertEquals(t, 0, len(vms), "Recieved incorrect amount of vms")

	err = dvs.RemoveHost(host)
	AssertOk(t, err, "Failed to remove host from dvs")
	err = dvs.RemoveHost(host2)
	AssertOk(t, err, "Failed to remove host from dvs")

	err = host2.Destroy()
	AssertOk(t, err, "failed to destroy host")
}

func getKind(t *testing.T, client *govmomi.Client, kind string, props []string, dst interface{}) {
	ctx := context.Background()
	viewMgr := view.NewManager(client.Client)
	v, err := viewMgr.CreateContainerView(ctx, client.Client.ServiceContent.RootFolder, []string{kind}, true)
	if err != nil {
		t.Fatalf("err not nil")
	}

	defer v.Destroy(ctx)

	err = v.Retrieve(ctx, []string{kind}, props, dst)
	if err != nil {
		t.Fatalf("err not nil %v", err)
	}
}

func TestWatch(t *testing.T) {
	t.Skip("Events for objects created after the watch has started are not being received. Still debugging this issue.")

	vcID := "user:pass@127.0.0.1:8990"
	s, err := NewVcSim(Config{Addr: vcID})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()

	ctx1 := context.Background()
	cancelCtx, cancelWatch := context.WithCancel(ctx1)

	u := s.GetURL()
	c, err := govmomi.NewClient(cancelCtx, u, true)
	if err != nil {
		t.Fatalf("err not nil %v", err)
	}
	client := c.Client
	viewMgr := view.NewManager(client)

	v, err := viewMgr.CreateContainerView(cancelCtx, c.Client.ServiceContent.RootFolder, []string{"Datacenter"}, true)
	if err != nil {
		t.Fatalf("err not nil")
	}

	defer v.Destroy(cancelCtx)

	hostProps := []string{"name"}
	hostRef := types.ManagedObjectReference{Type: "Datacenter"}
	filter := new(property.WaitFilter)
	filter = filter.Add(v.Reference(), hostRef.Type, hostProps)

	resCh := make(chan watchObj)

	updFunc := func(updates []types.ObjectUpdate) bool {
		t.Logf("call")
		for _, update := range updates {
			resCh <- watchObj{
				Name:    update.Obj.Value,
				Changes: update.ChangeSet,
			}
		}
		// Must return false, returning true will cause waitForUpdates to exit.
		return false
	}

	go func() {
		err = property.WaitForUpdates(cancelCtx, property.DefaultCollector(client), filter, updFunc)
	}()

	_, err = s.AddDC("test-dc-1")
	AssertOk(t, err, "failed to create DC")

	items := []watchObj{}
	for len(items) != 1 {
		select {
		case obj := <-resCh:
			items = append(items, obj)
		case <-time.After(5 * time.Second):
			cancelWatch()
			t.Fatalf("Failed to receive all messages. Only received %d items. %v", len(items), items)
		}
	}
	cancelWatch()

}
