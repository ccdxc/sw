package infra

import (
	"testing"

	Common "github.com/pensando/sw/test/utils/infra/common"
	TestUtils "github.com/pensando/sw/venice/utils/testutils"
)

func testRun(cmdArgs []string, timeout int, background bool, shell bool,
	env []string) (int, string, error) {
	return 0, "", nil
}

func init() {
	Common.Run = testRun
}

func TestEpInterface(t *testing.T) {
	newIntf := NewInterface("test", 0, "")
	if newIntf.Name != "test" {
		t.Errorf("Interface Name failed")
	}
	if newIntf.Vlan != 0 {
		t.Errorf("Vlan check failed!")
	}

	if newIntf.MacAddress != "" {
		t.Errorf("Mac address check failed")
	}
	TestUtils.AssertEquals(t, len(newIntf._VlanIntfs), 0, "0 sub-interface check")

	newIntf.AddVlanInterface("test_1", 100)
	newIntf.SetIPAddress("10.10.10.1", 24)

	TestUtils.AssertEquals(t, len(newIntf._VlanIntfs), 1, "Interface count success")

	newIntf.PrintInterfaceInformation()

	newIntf.SetIPAddress("1.2.3.4", 24)

	TestUtils.AssertEquals(t, newIntf.IPAddress, "1.2.3.4", "Ip address check")
	TestUtils.AssertEquals(t, newIntf.PrefixLen, 24, "Prefix length check")
}

func TestNS(t *testing.T) {
	newNs := NewNS("test")
	newNs.Init(false)

	TestUtils.AssertEquals(t, newNs.Name, "test", "NS Name check")
	TestUtils.AssertEquals(t, newNs._CmdPrefix, []string{"ip", "netns", "exec", "test"}, "Cmd prefix check")

	newNs.AttachInterface("test_intf")
	newNs.SetMacAddress("test_intf", "00:aa:bb:cc:dd:ee", 0)
	newNs.AddVlan("test_intf", 100)
	TestUtils.AssertEquals(t, len(newNs._Interfaces), 1, "Interface count check")
	TestUtils.AssertEquals(t, len(newNs._Interfaces["test_intf"]._VlanIntfs),
		1, "Interface count check")
	newNs.SetMacAddress("test_intf", "00:aa:bb:cc:dd:ff", 100)
	TestUtils.AssertEquals(t, newNs._Interfaces["test_intf"].MacAddress, "00:aa:bb:cc:dd:ee", "Mac address check")
	TestUtils.AssertEquals(t, newNs._Interfaces["test_intf"]._VlanIntfs[100].MacAddress,
		"00:aa:bb:cc:dd:ff", "Mac address check")

	newNs.SetIPAddress("test_intf", "2.3.4.4", 24, 0)
	newNs.SetIPAddress("test_intf", "2.3.4.5", 24, 100)
	TestUtils.AssertEquals(t, newNs._Interfaces["test_intf"].IPAddress, "2.3.4.4", "IP address check")
	TestUtils.AssertEquals(t, newNs._Interfaces["test_intf"]._VlanIntfs[100].IPAddress,
		"2.3.4.5", "IP address check")
	newNs.AddArpEntry("1.2.3.4", "00:aa:bb:cc:dd:ee")
	newNs.PrintAppInformation()
	newNs.DeleteVlan("test_intf", 100)
	newNs.MoveInterface("test_intf", 1)
	TestUtils.AssertEquals(t, len(newNs._Interfaces), 1, "Interface count check")
	TestUtils.AssertEquals(t, len(newNs._Interfaces["test_intf"]._VlanIntfs),
		0, "Interface count check")
	newNs.Reset()

}

func TestContainer(t *testing.T) {

	newContainer, _ := NewContainer("test-container", "alpine", "")

	if newContainer == nil {
		t.Fatalf("New Container Creation failed")
	}
	defer newContainer.Stop()
	newContainer.PrintAppInformation()
	newContainer.RunCommand([]string{"ls", "-lrt"}, 0, false, false)

	newContainer.AttachInterface("test_intf")
	newContainer.SetMacAddress("test_intf", "00:aa:bb:cc:dd:ee", 0)
	newContainer.AddVlan("test_intf", 100)
	TestUtils.AssertEquals(t, len(newContainer._Interfaces), 1, "Interface count check")
	TestUtils.AssertEquals(t, len(newContainer._Interfaces["test_intf"]._VlanIntfs),
		1, "Interface count check")
	newContainer.SetMacAddress("test_intf", "00:aa:bb:cc:dd:ff", 100)
	TestUtils.AssertEquals(t, newContainer._Interfaces["test_intf"].MacAddress, "00:aa:bb:cc:dd:ee", "Mac address check")
	TestUtils.AssertEquals(t, newContainer._Interfaces["test_intf"]._VlanIntfs[100].MacAddress,
		"00:aa:bb:cc:dd:ff", "Mac address check")

	newContainer.GetInterfaces()
	newContainer.SetIPAddress("test_intf", "2.3.4.4", 24, 0)
	newContainer.SetIPAddress("test_intf", "2.3.4.5", 24, 100)
	TestUtils.AssertEquals(t, newContainer._Interfaces["test_intf"].IPAddress, "2.3.4.4", "IP address check")
	TestUtils.AssertEquals(t, newContainer._Interfaces["test_intf"]._VlanIntfs[100].IPAddress,
		"2.3.4.5", "IP address check")
	newContainer.AddArpEntry("1.2.3.4", "00:aa:bb:cc:dd:ee")
	newContainer.PrintAppInformation()
	newContainer.DeleteVlan("test_intf", 100)
	newContainer.MoveInterface("test_intf", 1)
	TestUtils.AssertEquals(t, len(newContainer._Interfaces), 1, "Interface count check")
	TestUtils.AssertEquals(t, len(newContainer._Interfaces["test_intf"]._VlanIntfs),
		0, "Interface count check")
	newContainer.Stop()
}
