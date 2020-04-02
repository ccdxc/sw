package vmware

import (
	"context"
	"fmt"
	"os"
	"testing"

	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func TestMain(m *testing.M) {

	runTests := m.Run()

	os.Exit(runTests)
}

func Test_datastore(t *testing.T) {

	vc1, err := NewVcenter(context.Background(), "192.168.69.120", "administrator@vsphere.local", "N0isystem$", "")

	TestUtils.Assert(t, err == nil, "Connected to venter")
	TestUtils.Assert(t, vc1 != nil, "Vencter context set")

	dc, err := vc1.SetupDataCenter("iota-dc")
	TestUtils.Assert(t, err == nil, "successfuly setup dc")
	dc, ok := vc1.datacenters["iota-dc"]
	TestUtils.Assert(t, ok, "successfuly setup dc")

	TestUtils.Assert(t, len(dc.clusters) == 1, "successfuly setup dc")

	c, ok := dc.clusters["iota-cluster"]
	TestUtils.Assert(t, ok, "successfuly setup cluster")
	TestUtils.Assert(t, len(c.hosts) == 1, "successfuly setup cluster")

	_, err = dc.findHost("iota-cluster", "tb36-host1.pensando.io")
	TestUtils.Assert(t, err == nil, "successfuly setup cluster")
	_, err = dc.Datastore("", "datastore1")
	TestUtils.Assert(t, err == nil, "successfuly setup cluster")

	pgName, err := dc.FindDvsPortGroup("pen-dvs", DvsPGMatchCriteria{Type: DvsVlanID, VlanID: 100})
	fmt.Printf("Error %v %v\n", pgName, err)
	TestUtils.Assert(t, err == nil, "Found matchin PG")

	vm, err := dc.vc.NewVM("iota-control-vm")
	TestUtils.Assert(t, err == nil, "Deploy VM done")
	TestUtils.Assert(t, vm != nil, "Deploy VM done")

	err = vm.ReconfigureNetwork("iota-def-network", pgName, 0)
	fmt.Printf("Error %v %v\n", pgName, err)
	TestUtils.Assert(t, err == nil, "dvs updated")
	/*host, err := NewHost(context.Background(), "10.10.2.30", "root", "N0isystem$")

	TestUtils.Assert(t, err == nil, "Connected to host")
	TestUtils.Assert(t, host != nil, "Host context set")

	ds, err := host.Datastore("datastore1")

	TestUtils.Assert(t, err == nil, "successfuly found")
	TestUtils.Assert(t, ds != nil, "Ds found")

	buf := bytes.NewBufferString("THIS IS TEST UPLOAD FILE")
	err = ds.Upload("testUpload.txt", buf)

	TestUtils.Assert(t, err == nil, "Upload done")

	err = ds.Remove("testUpload.txt")

	TestUtils.Assert(t, err == nil, "Remove done") */
}

func Test_dvs_create_delete(t *testing.T) {
	vc, err := NewVcenter(context.Background(), "barun-vc.pensando.io", "administrator@pensando.io", "N0isystem$", "")
	TestUtils.Assert(t, err == nil, "Connected to venter")
	TestUtils.Assert(t, vc != nil, "Vencter context set")

	vc.DestroyDataCenter("iota-dc")
	vc.DestroyDataCenter("iota-dc1")

	dc, err := vc.CreateDataCenter("iota-dc")

	TestUtils.Assert(t, err == nil, "successfuly created")
	TestUtils.Assert(t, dc != nil, "Ds not created")

	c, err := dc.CreateCluster("cluster1")
	TestUtils.Assert(t, err == nil, "successfuly created")
	TestUtils.Assert(t, c != nil, "Cluster not created")

	err = c.AddHost("tb36-host1.pensando.io", "root", "pen123!", "")
	TestUtils.Assert(t, err == nil, "host added")

	hostSpecs := []DVSwitchHostSpec{
		DVSwitchHostSpec{
			Name:  "tb36-host1.pensando.io",
			Pnics: []string{"vmnic1", "vmnic2"},
		},
	}
	dvsSpec := DVSwitchSpec{Hosts: hostSpecs,
		Name: "iota-dvs", Cluster: "cluster1", MaxPorts: 10,
		Pvlans: []DvsPvlanPair{DvsPvlanPair{Primary: 500,
			Secondary: 500, Type: "promiscuous"}},
		PortGroups: []DvsPortGroup{DvsPortGroup{Name: "pg1", Ports: 10, Type: "earlyBinding"}}}
	err = dc.AddDvs(dvsSpec)
	TestUtils.Assert(t, err == nil, "dvs added")

	dvs, err := dc.findDvs("iota-dvs")
	TestUtils.Assert(t, err == nil && dvs != nil, "dvs found")
	err = dc.AddPvlanPairsToDvs("iota-dvs", []DvsPvlanPair{DvsPvlanPair{Primary: 500,
		Secondary: 501, Type: "isolated"}})
	TestUtils.Assert(t, err == nil, "pvlan added")

	err = dc.AddPortGroupToDvs("iota-dvs",
		[]DvsPortGroup{DvsPortGroup{Name: "pg2", Ports: 10, Type: "earlyBinding"}})
	TestUtils.Assert(t, err == nil, "pvlan added")

	vmInfo, err := dc.DeployVM("cluster1", "tb36-host1.pensando.io",
		"build-111", 4, 4, []string{"VM Network"}, "/Users/sudhiaithal/build-114")
	fmt.Printf("Deploy VM error %v", err)
	TestUtils.Assert(t, err == nil, "Deploy VM done")

	fmt.Println("Vm info : ", vmInfo.IP)

	vm, err := dc.vc.NewVM("build-111")
	TestUtils.Assert(t, err == nil, "Deploy VM done")
	TestUtils.Assert(t, vm != nil, "Deploy VM done")

	err = vm.ReconfigureNetwork("VM Network", "pg1", 0)
	TestUtils.Assert(t, err == nil, "dvs updated")
	//	err = host.DestoryVM("build-111")
	//err = dc.RemoveHostsFromDvs(dvsSpec)
	//TestUtils.Assert(t, err == nil, "dvs updated")
}

/*
func Test_vcenter_dissocaitate(t *testing.T) {
	host, err := NewHost(context.Background(), "tb60-host1.pensando.io", "root", "pen123!")
	TestUtils.Assert(t, err == nil, "Connected to host")
	TestUtils.Assert(t, host != nil, "Host context set")

	vcenter, err := host.GetVcenterForHost()

	fmt.Printf("Vcenter IP %v", vcenter)
	TestUtils.Assert(t, err == nil, "Did not find vcenter")

	vc, err := NewVcenter(context.Background(), vcenter, "administrator@pensando.io", "N0isystem$",
		"YN69K-6YK5J-78X8T-0M3RH-0T12H")

	TestUtils.Assert(t, err == nil, "Connected to venter")
	TestUtils.Assert(t, vc != nil, "Vencter context set")

	err = vc.DisconnectHost("tb60-host1.pensando.io")
	TestUtils.Assert(t, err == nil, "Disconect succesful")

}
*/

func Test_vcenter_find_host(t *testing.T) {

	//	TestUtils.Assert(t, false, "Ds not created")

	ctx, _ := context.WithCancel(context.Background())
	vc, err := NewVcenter(ctx, "barun-vc.pensando.io", "administrator@pensando.io", "N0isystem$",
		"YN69K-6YK5J-78X8T-0M3RH-0T12H")

	TestUtils.Assert(t, err == nil, "Connected to venter")
	TestUtils.Assert(t, vc != nil, "Vencter context set")

	dc, err := vc.SetupDataCenter("ganesh-iota-dc")
	TestUtils.Assert(t, err == nil, "successfuly setup dc")
	dc, ok := vc.datacenters["ganesh-iota-dc"]
	TestUtils.Assert(t, ok, "ganesh setup dc")

	c, ok := dc.clusters["ganesh-iota-cluster"]
	TestUtils.Assert(t, ok, "successfuly setup cluster")
	TestUtils.Assert(t, len(c.hosts) == 2, "successfuly setup cluster")

	dvs, err := dc.findDvs("Pen-DVS-ganesh-iota-dc")
	TestUtils.Assert(t, err == nil, "successfuly found dvs")
	TestUtils.Assert(t, dvs != nil, "dvs nil")

	vm, err2 := dc.NewVM("node1-ep3")
	TestUtils.Assert(t, err2 == nil, "VM FOUND")
	TestUtils.Assert(t, vm != nil, "VM FOND")

	dc.SetVlanOverride("Pen-DVS-ganesh-iota-dc", "node1-ep3", 1234, 1239)

	TestUtils.Assert(t, false, "Ds not created")

	/*
		err2 = dc.ReconfigureVMNetwork(vm, "iota-def-network", "Pen-DVS-sudhiaithal-iota-dc", "iota-data-nw-1001", 0, true)
		fmt.Printf("Error %v", err2)
		TestUtils.Assert(t, err2 == nil, "Reconfig faild")

		ip, err := vm.vm.WaitForIP(ctx, true)
		fmt.Printf("VM IP %v", ip) */

	/*dvsSpec := DVSwitchSpec{
	Name: "iota-dvs", Cluster: "sudhiaithal-iota-cluster",
	MaxPorts: 10,
	Pvlans: []DvsPvlanPair{DvsPvlanPair{Primary: 1024,
		Secondary: 1024, Type: "promiscuous"}}} */

	//err = dc.AddDvs(dvsSpec)
	//TestUtils.Assert(t, err == nil, "dvs added")

	/*
		vm, err2 = dc.NewVM("node2-ep1")
		TestUtils.Assert(t, err2 == nil, "VM FOUND")
		TestUtils.Assert(t, vm != nil, "VM FOND")

		macs, err3 := vm.ReadMacs()
		for nw, mac := range macs {
			fmt.Printf("NW : %v. mac %v\n", nw, mac)
		}
		TestUtils.Assert(t, err3 != nil, "VM FOUND") */

	//dvs, err := dc.findDvs("#Pen-DVS-sudhiaithal-iota-dc")
	//TestUtils.Assert(t, err == nil && dvs != nil, "dvs found")

	//pgName, err := dc.FindDvsPortGroup("Pen-DVS-sudhiaithal-iota-dc", DvsPGMatchCriteria{Type: DvsPvlan, VlanID: int32(199)})
	//fmt.Printf("Dvs port name %v %v", pgName, err)
	//TestUtils.Assert(t, err == nil, "Connected to venter")

	//err = dc.RelaxSecurityOnPg("#Pen-DVS-sudhiaithal-iota-dc", "#Pen-PG-Network-Vlan-2")
	//fmt.Printf("Error %v\n", err)
	//TestUtils.Assert(t, err == nil, "pvlan added")

	/*
		for i := int32(0); i < 100; i += 2 {
			err = dc.AddPvlanPairsToDvs("iota-dvs", []DvsPvlanPair{DvsPvlanPair{Primary: 3003 + i,
				Secondary: 3003 + i, Type: "promiscuous"}})
			fmt.Printf("Error %v\n", err)
			TestUtils.Assert(t, err == nil, "pvlan added")

			err = dc.AddPvlanPairsToDvs("iota-dvs", []DvsPvlanPair{DvsPvlanPair{Primary: 3003 + i,
				Secondary: 3003 + i + 1, Type: "isolated"}})
			fmt.Printf("Error %v\n", err)
			TestUtils.Assert(t, err == nil, "pvlan added")
		} */

	//pgName = constants.EsxDataNWPrefix + strconv.Itoa(spec.PrimaryVlan)
	//Create the port group
	/*err = dc.AddPortGroupToDvs("Pen-DVS-sudhiaithal-iota-dc",
		[]DvsPortGroup{DvsPortGroup{Name: "my-pg",
			VlanOverride: true,
			Private:      true,
			Ports:        32, Type: "earlyBinding",
			Vlan: int32(800)}})
	fmt.Printf("Error %v\n", err)
	TestUtils.Assert(t, err == nil, "pvlan added")*/

	/*
		ctx, cancel := context.WithCancel(context.Background())
		vc, err := NewVcenter(ctx, "barun-vc.pensando.io", "administrator@pensando.io", "N0isystem$",
			"YN69K-6YK5J-78X8T-0M3RH-0T12H")

		TestUtils.Assert(t, err == nil, "Connected to venter")
		TestUtils.Assert(t, vc != nil, "Vencter context set")

		dc, err := vc.SetupDataCenter("iota-dc")
		TestUtils.Assert(t, err == nil, "successfuly setup dc")
		dc, ok := vc.datacenters["iota-dc"]
		TestUtils.Assert(t, ok, "successfuly setup dc")

		TestUtils.Assert(t, len(dc.clusters) == 1, "successfuly setup dc")

		c, ok := dc.clusters["iota-cluster"]
		TestUtils.Assert(t, ok, "successfuly setup cluster")
		TestUtils.Assert(t, len(c.hosts) == 1, "successfuly setup cluster")

		cancel()

		active, err := vc.Client().SessionManager.SessionIsActive(ctx)
		TestUtils.Assert(t, active == false, "Connected to venter active")

		ctx, cancel = context.WithCancel(context.Background())
		err = vc.Reinit(ctx)
		TestUtils.Assert(t, err == nil, "Reinit failed")
		active, err = vc.Client().SessionManager.SessionIsActive(vc.Ctx())
		TestUtils.Assert(t, active == true, "Connected to venter active")

		err = dc.setUpFinder()
		//TestUtils.Assert(t, err == nil, "Setup finder succes")

		vhost, err := dc.findHost("iota-cluster", "tb60-host1.pensando.io")
		fmt.Printf("Err %v", err)
		TestUtils.Assert(t, err == nil, "Connected to venter")
		TestUtils.Assert(t, vhost != nil, "Vencter context set")

		//Create vmkitni

		vNWs := []NWSpec{
			{Name: "my-vmk2"},
		}
		vspec := VswitchSpec{Name: "vSwitch0"}

		err = dc.AddNetworks("iota-cluster", "tb60-host1.pensando.io", vNWs, vspec)
		TestUtils.Assert(t, err == nil, "Add network failed")

		err = dc.AddKernelNic("iota-cluster", "tb60-host1.pensando.io", "my-vmk2", true)
		fmt.Printf("Err %v", err)
		TestUtils.Assert(t, err == nil, "Creted VMK")

		err = dc.RemoveKernelNic("iota-cluster", "tb60-host1.pensando.io", "my-vmk2")
		fmt.Printf("Err %v", err)
		TestUtils.Assert(t, err == nil, "Removed VMK")

		err = dc.RemoveNetworks("iota-cluster", "tb60-host1.pensando.io", vNWs)
		TestUtils.Assert(t, err == nil, "Add network failed")

	*/
}

func Test_vcenter_ovf_deploy(t *testing.T) {

	host, err := NewHost(context.Background(), "tb36-host2.pensando.io", "root", "pen123!")
	TestUtils.Assert(t, err == nil, "Connected to host")

	var intf interface{}

	intf = host

	pgs, err := host.ListNetworks()
	TestUtils.Assert(t, err == nil, "PG list failed")
	for _, pg := range pgs {
		fmt.Printf("PG %v %v %v\n", pg.Name, pg.Vlan, pg.Vswitch)
	}
	intf.(EntityIntf).DestoryVM("asds")

	TestUtils.Assert(t, !host.IsVcenter(), "NOt vcenter")

	TestUtils.Assert(t, false, "Ds not created")

	ctx, _ := context.WithCancel(context.Background())
	vc, err := NewVcenter(ctx, "barun-vc.pensando.io", "administrator@pensando.io", "N0isystem$",
		"YN69K-6YK5J-78X8T-0M3RH-0T12H")

	TestUtils.Assert(t, err == nil, "Connected to venter")
	TestUtils.Assert(t, vc != nil, "Vencter context set")

	dc, err := vc.SetupDataCenter("iota-dc")
	TestUtils.Assert(t, err == nil, "successfuly setup dc")
	dc, ok := vc.datacenters["iota-dc"]
	TestUtils.Assert(t, ok, "successfuly setup dc")

	c, ok := dc.clusters["iota-cluster"]
	TestUtils.Assert(t, ok, "successfuly setup cluster")
	TestUtils.Assert(t, len(c.hosts) == 2, "successfuly setup cluster")

	vhost, err := dc.findHost("iota-cluster", "tb60-host2.pensando.io")
	TestUtils.Assert(t, err == nil, "Connected to venter")
	TestUtils.Assert(t, vhost != nil, "Vencter context set")

	pgName, err := dc.FindDvsPortGroup("pen-dvs", DvsPGMatchCriteria{Type: DvsVlanID, VlanID: int32(3)})
	fmt.Printf("Dvs port name %v %v", pgName, err)
	TestUtils.Assert(t, err == nil, "Connected to venter")

	//TestUtils.Assert(t, false, "Ds not created")

	//vmInfo, err := host.DeployVM("build-111", 4, 4, []string{"VM Network"}, "/home/sudhiaithal/build-111/")
	/*vmInfo, err := dc.DeployVM("iota-cluster", "tb60-host2.pensando.io",
		"build-112", 4, 4, []string{"VM Network", "VM Network", "VM Network", "iota-def-network"}, "/Users/sudhiaithal/build-114")
	fmt.Printf("Deploy VM error %v", err)
	TestUtils.Assert(t, err == nil, "Deploy VM done")

	fmt.Println("Vm info : ", vmInfo.IP)*/

	/*
		ctx, cancel := context.WithCancel(context.Background())
		vc, err := NewVcenter(ctx, "barun-vc.pensando.io", "administrator@pensando.io", "N0isystem$",
			"YN69K-6YK5J-78X8T-0M3RH-0T12H")

		TestUtils.Assert(t, err == nil, "Connected to venter")
		TestUtils.Assert(t, vc != nil, "Vencter context set")

		dc, err := vc.SetupDataCenter("iota-dc")
		TestUtils.Assert(t, err == nil, "successfuly setup dc")
		dc, ok := vc.datacenters["iota-dc"]
		TestUtils.Assert(t, ok, "successfuly setup dc")

		TestUtils.Assert(t, len(dc.clusters) == 1, "successfuly setup dc")

		c, ok := dc.clusters["iota-cluster"]
		TestUtils.Assert(t, ok, "successfuly setup cluster")
		TestUtils.Assert(t, len(c.hosts) == 1, "successfuly setup cluster")

		cancel()

		active, err := vc.Client().SessionManager.SessionIsActive(ctx)
		TestUtils.Assert(t, active == false, "Connected to venter active")

		ctx, cancel = context.WithCancel(context.Background())
		err = vc.Reinit(ctx)
		TestUtils.Assert(t, err == nil, "Reinit failed")
		active, err = vc.Client().SessionManager.SessionIsActive(vc.Ctx())
		TestUtils.Assert(t, active == true, "Connected to venter active")

		err = dc.setUpFinder()
		//TestUtils.Assert(t, err == nil, "Setup finder succes")

		vhost, err := dc.findHost("iota-cluster", "tb60-host1.pensando.io")
		fmt.Printf("Err %v", err)
		TestUtils.Assert(t, err == nil, "Connected to venter")
		TestUtils.Assert(t, vhost != nil, "Vencter context set")

		//Create vmkitni

		vNWs := []NWSpec{
			{Name: "my-vmk2"},
		}
		vspec := VswitchSpec{Name: "vSwitch0"}

		err = dc.AddNetworks("iota-cluster", "tb60-host1.pensando.io", vNWs, vspec)
		TestUtils.Assert(t, err == nil, "Add network failed")

		err = dc.AddKernelNic("iota-cluster", "tb60-host1.pensando.io", "my-vmk2", true)
		fmt.Printf("Err %v", err)
		TestUtils.Assert(t, err == nil, "Creted VMK")

		err = dc.RemoveKernelNic("iota-cluster", "tb60-host1.pensando.io", "my-vmk2")
		fmt.Printf("Err %v", err)
		TestUtils.Assert(t, err == nil, "Removed VMK")

		err = dc.RemoveNetworks("iota-cluster", "tb60-host1.pensando.io", vNWs)
		TestUtils.Assert(t, err == nil, "Add network failed")

	*/
}

/*
func Test_vcenter_migration(t *testing.T) {

	ctx, _ := context.WithCancel(context.Background())
	vc, err := NewVcenter(ctx, "barun-vc.pensando.io", "administrator@pensando.io", "N0isystem$",
		"YN69K-6YK5J-78X8T-0M3RH-0T12H")

	TestUtils.Assert(t, err == nil, "Connected to venter")
	TestUtils.Assert(t, vc != nil, "Vencter context set")

	dc, err := vc.SetupDataCenter("iota-dc")
	TestUtils.Assert(t, err == nil, "successfuly setup dc")
	dc, ok := vc.datacenters["iota-dc"]
	TestUtils.Assert(t, ok, "successfuly setup dc")

	TestUtils.Assert(t, len(dc.clusters) == 1, "successfuly setup dc")

	c, ok := dc.clusters["iota-cluster"]
	TestUtils.Assert(t, ok, "successfuly setup cluster")
	TestUtils.Assert(t, len(c.hosts) == 2, "successfuly setup cluster")

	//vhost, _ := dc.findHost("iota-cluster", "tb60-host2.pensando.io")

	viewMgr := view.NewManager(dc.client.Client.Client)

	//VCObject("HostSystem")
	var hosts []mo.HostSystem
	//v.ListObj(defs.HostSystem, []string{"config", "name"}, &hosts, dc.ref)

	cView, err := viewMgr.CreateContainerView(dc.vc.Ctx(), dc.ref.Reference(), []string{}, true)
	TestUtils.Assert(t, err == nil, "View create failed")
	err = cView.Retrieve(dc.vc.Ctx(), []string{string("HostSystem")}, []string{"config", "name", "datastore"}, &hosts)
	TestUtils.Assert(t, err == nil, "View retriee ")

	for _, host := range hosts {
		fmt.Printf("HOst name %v : \n", host.Name)
		for _, ds := range host.Datastore {
			ds.Reference()
			fmt.Printf("Data store name %v %v\n", ds.String(), ds.Value)
		}
	}

	ds, _ := dc.finder.DatastoreList(dc.vc.Ctx(), "*")
	for _, d := range ds {
		fmt.Printf("Data Store %v %v ", d.Reference(), d.Name())
	}

	err = dc.LiveMigrate("my-vm", "tb60-host2.pensando.io", "tb60-host1.pensando.io", "iota-cluster")

	fmt.Printf("Err %v", err)
	TestUtils.Assert(t, err == nil, "Vmotion failed")
	//TestUtils.Assert(t, vhost != nil, "Vencter context set")
}
*/
func Test_ovf_deploy(t *testing.T) {

	host, err := NewHost(context.Background(), "tb29-host1.pensando.io", "root", "pen123!")

	TestUtils.Assert(t, err == nil, "Connected to host")
	TestUtils.Assert(t, host != nil, "Host context set")

	hostVM, err3 := host.NewVM("node1-ep4")
	TestUtils.Assert(t, hostVM != nil && err3 == nil, "VM FOND")

	/*
		vc, err := NewVcenter(context.Background(), "192.168.69.120", "administrator@vsphere.local", "N0isystem$")

		TestUtils.Assert(t, err == nil, "Connected to venter")
		TestUtils.Assert(t, vc != nil, "Vencter context set")

		vc.DestroyDataCenter("iota-dc")
		vc.DestroyDataCenter("iota-dc1")

		dc, err := vc.CreateDataCenter("iota-dc")

		TestUtils.Assert(t, err == nil, "successfuly created")
		TestUtils.Assert(t, dc != nil, "Ds not created")

		c, err := dc.CreateCluster("cluster1")
		TestUtils.Assert(t, err == nil, "successfuly created")
		TestUtils.Assert(t, c != nil, "Cluster not created")

		err = c.AddHost("tb36-host1.pensando.io", "root", "pen123!")
		TestUtils.Assert(t, err == nil, "host added") */

	/*
		dc, err = vc1.SetupDataCenter("iota-dc")
		TestUtils.Assert(t, err == nil, "successfuly setup dc")
		dc, ok := vc1.datacenters["iota-dc"]
		TestUtils.Assert(t, ok, "successfuly setup dc")

		//vmInfo, err := host.DeployVM("build-111", 4, 4, []string{"VM Network"}, "/home/sudhiaithal/build-111/")
		vmInfo, err := dc.DeployVM("cluster1", "tb36-host1.pensando.io",
			"build-111", 4, 4, []string{"VM Network"}, "/Users/sudhiaithal/build-114")
		fmt.Printf("Deploy VM error %v", err)
		TestUtils.Assert(t, err == nil, "Deploy VM done")

		fmt.Println("Vm info : ", vmInfo.IP)

		vsname := "test-vs"
		vsspec := VswitchSpec{Name: vsname}
		err = dc.AddVswitch("cluster1", "tb36-host1.pensando.io", vsspec)
		TestUtils.Assert(t, err == nil, "successfuly setup vswitch")

		vswitchs, err := dc.ListVswitches("cluster1", "tb36-host1.pensando.io")
		TestUtils.Assert(t, err == nil, "successfuly setup vswitch")
		fmt.Printf("Vswitches %v\n", vswitchs)

		err = dc.DestoryVM("build-111")
		TestUtils.Assert(t, err == nil, "destroy VM successful")

		cl, ok := dc.clusters["cluster1"]
		err = cl.DeleteHost("tb36-host1.pensando.io")
		TestUtils.Assert(t, err == nil, "delete host successull")

		host, err := NewHost(context.Background(), "tb36-host1.pensando.io", "root", "pen123!")

		TestUtils.Assert(t, err == nil, "Connected to host")
		TestUtils.Assert(t, host != nil, "Host context set")

		vsname = "test-vs"
		vsspec = VswitchSpec{Name: vsname}
		err = host.AddVswitch(vsspec)
		TestUtils.Assert(t, err == nil, "successfuly setup vswitch")

		vswitchs, err = host.ListVswitchs()
		TestUtils.Assert(t, err == nil, "successfuly setup vswitch")
		fmt.Printf("Vswitches %v\n", vswitchs)

		vmInfo, err = host.DeployVM("build-111", 4, 4, []string{"VM Network"}, "/home/sudhiaithal/build-114/")
		fmt.Printf("Deploy VM error %v", err)
		TestUtils.Assert(t, err == nil, "Deploy VM done")

		fmt.Println("Vm info : ", vmInfo.IP)

		err = host.DestoryVM("build-111")

		TestUtils.Assert(t, err == nil, "destroy VM successful")
	*/
	/*
		//vmInfo, err := host.DeployVM("build-111", 4, 4, []string{"VM Network"}, "/home/sudhiaithal/build-111/")
		vmInfo, err := dc.DeployVM("cluster1", "tb36-host1.pensando.io",
			"build-111", 4, 4, []string{}, "/Users/sudhiaithal/build-114")
		fmt.Printf("Deploy VM error %v", err)
		TestUtils.Assert(t, err == nil, "Deploy VM done")

		fmt.Println("Vm info : ", vmInfo.IP)
	*/
	/*
		TestUtils.Assert(t, len(vc1.datacenters) == 1, "successfuly setup dc")
		cluster, ok := dc.clusters["cluster1"]
		TestUtils.Assert(t, ok, "successfuly setup cluster")
		TestUtils.Assert(t, len(cluster.hosts) == 1 && cluster.hosts[0] == "tb36-host1.pensando.io",
			"successfuly setup host cluster")

	*/

	/*

		err = dc.DestroyCluster("cluster1")
		TestUtils.Assert(t, err == nil, "Cluster Destroyed")

		err = vc.DestroyDataCenter("iota-dc")
		TestUtils.Assert(t, err == nil, "Deleted succesfully")
	*/
	//vsname := "test-vs-1"
	//vsspec := VswitchSpec{Name: vsname, Pnics: []string{"vmnic2", "vmnic3"}}
	//err = host.AddVswitch(vsspec)
	//TestUtils.Assert(t, err == nil, "Vss added")

	/*
		nws := []NWSpec{{Name: "pg_test1", Vlan: 1}, {Name: "pg_test2", Vlan: 2}}

		nets, err1 := host.AddNetworks(nws, vsspec)
		TestUtils.Assert(t, err1 == nil, "Pg added")
		TestUtils.Assert(t, len(nets) == len(nws), "Pg added")

		nets = append(nets, "VM Network")
		//vmInfo, err := host.DeployVM("build-111", 4, 4, []string{"VM Network"}, "/home/sudhiaithal/build-111/")
		vmInfo, err := host.DeployVM("build-111", 4, 4, []string{"VM Network", "pg_test1"}, "//Users/sudhiaithal/Downloads/build-111/")
		TestUtils.Assert(t, err == nil, "Deploy VM done")

		fmt.Println("Vm info : ", vmInfo.IP)

		finder, _, err1 := host.client.finder()
		TestUtils.Assert(t, err1 == nil, "Client fined found")
		TestUtils.Assert(t, finder != nil, "Client fined found")

		vm, err2 := finder.VirtualMachine(host.context.context, "build-111")
		TestUtils.Assert(t, err2 == nil, "VM FOUND")
		TestUtils.Assert(t, vm != nil, "VM FOND")

		ip, _ := vm.WaitForIP(host.context.context)
		fmt.Println("IP ADDRESS OF GUEST ", ip)

		hostVM, err3 := host.NewVM("build-111")
		TestUtils.Assert(t, hostVM != nil && err3 == nil, "VM FOND")

		err = hostVM.ReconfigureNetwork("pg_test1", "pg_test2")
		TestUtils.Assert(t, err == nil, "Reconfigured")

		err = host.DestoryVM("build-111")
		TestUtils.Assert(t, err == nil, "Destroy VM done")

		err = host.RemoveNetworks(nws)
		TestUtils.Assert(t, err == nil, "Pg removed")

		TestUtils.Assert(t, err == nil, "Host context set")

	*/
}
