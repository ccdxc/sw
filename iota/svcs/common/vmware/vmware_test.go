package vmware

import (
	"context"
	"os"
	"testing"

	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func TestMain(m *testing.M) {

	runTests := m.Run()

	os.Exit(runTests)
}

func Test_datastore(t *testing.T) {

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

func Test_ovf_deploy(t *testing.T) {

	host, err := NewHost(context.Background(), "192.168.69.181", "root", "ubuntu123")

	TestUtils.Assert(t, err == nil, "Connected to host")
	TestUtils.Assert(t, host != nil, "Host context set")

	ds, err := host.Datastore("datastore1")

	TestUtils.Assert(t, err == nil, "successfuly found")
	TestUtils.Assert(t, ds != nil, "Ds found")

	vsname := "test-vs-1"
	vsspec := VswitchSpec{Name: vsname, Pnics: []string{"vmnic4"}}
	err = host.AddVswitch(vsspec)
	TestUtils.Assert(t, err == nil, "Vss added")

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
