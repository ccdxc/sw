package ipif

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"math/rand"
	"net"
	"os"
	"strconv"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api/generated/cluster"

	"github.com/pensando/sw/nic/agent/nmd/mock"

	dhcp "github.com/krolaw/dhcp4"
	"github.com/krolaw/dhcp4/conn"
	"github.com/pensando/netlink"

	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	dhcpServerIntf = "srv-dhcp"
	ipAddrStart    = "172.16.10.2"
	testSubnet     = "172.16.10.0/28"
	leaseDuration  = time.Second * 10
	noDHCP         = iota
	configureNoVendorAtrrs
	configureEmptyVendorAtrs
	configureValidVendorAttrs
	configureMalformedVendorAttrs
	configureMalformedVendorAttrsMismatchOption43And60
	configureValidVendorAttrs241
	configureValidVendorAttrs241Multiple
	configureValidVendorAttrs241and242
	configureMalformedVendorAttrsDSCInterfaceIPs
	configureInvalidInfoVendorAttrsDSCInterfaceIPs
	configureValidClasslessStaticRoutes
	configureMalformedClasslessStaticRoutes
	configureInvalidClasslessStaticRoutes
)

var (
	staticIPConfig = cluster.IPConfig{
		IPAddress:  "172.16.10.10/28",
		DNSServers: []string{"172.16.10.1", "172.16.10.2"},
		DefaultGW:  "172.16.10.1",
	}
	staticInbIPConfig = cluster.IPConfig{
		IPAddress: "172.16.10.11/28",
	}
	_, allocSubnet, _           = net.ParseCIDR(testSubnet)
	veniceIPs                   = "42.42.42.42,84.84.84.84"
	option241VeniceIPs          = "1.1.1.1"
	option241VeniceIPsMultiple  = "2.2.2.2,3.3.3.3"
	option242InterfaceIPs       = "0 28 20.20.20.4 20.20.20.1,1 28 20.20.20.5 20.20.20.2"
	optionClasslessStaticRoutes = "28 20.20.20.4 20.20.20.1,28 20.20.20.5 20.20.20.2"
)

type dhcpSrv struct {
	conn net.PacketConn
}

func TestMain(m *testing.M) {
	input, err := ioutil.ReadFile("/etc/resolv.conf")
	if err != nil {
		fmt.Println(err)
	}

	exitCode := m.Run()
	// restore the contents of resolv.conf
	if err == nil {
		err = ioutil.WriteFile("/etc/resolv.conf", input, 0644)
		if err != nil {
			fmt.Println("Error overwriting /etc/resolv.conf")
			fmt.Println(err)
		}
	}

	os.Exit(exitCode)
}

//++++++++++++++++++++++++++++ Happy Path Test Cases ++++++++++++++++++++++++++++++++++++++++
func TestIPClient_DoStaticConfig(t *testing.T) {
	var d dhcpSrv
	err := d.setup(noDHCP)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")
	// Set the IP Config
	mockNMD.SetIPConfig(&staticIPConfig)
	ipAddr, _, err := ipClient.DoStaticConfig()
	AssertOk(t, err, "Failed to assign ip address statically to a mock interface")
	Assert(t, ipAddr == mockNMD.GetIPConfig().IPAddress, "Got in correct assigned IP Address")
}

func TestIPClient_DoStaticConfigInb(t *testing.T) {
	ldhcpClientMock := &netlink.Dummy{
		LinkAttrs: netlink.LinkAttrs{
			Name:   NaplesInbandInterface,
			TxQLen: 1000,
		},
	}

	// Create the veth pair
	if err := netlink.LinkAdd(ldhcpClientMock); err != nil {
		if !strings.Contains(err.Error(), "file exists") {
			log.Errorf("Interface already present. Continuing...")
		}
		log.Info("Interface already present. Continuing...")
	}

	netlink.LinkSetUp(ldhcpClientMock)
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesInbandInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")
	// Set the IP Config
	mockNMD.SetIPConfig(&staticIPConfig)
	mockNMD.SetInbandIPConfig(&staticInbIPConfig)
	ipAddr, inbIPAddr, err := ipClient.DoStaticConfig()
	AssertOk(t, err, "Failed to assign ip address statically to a mock interface")
	Assert(t, ipAddr == mockNMD.GetIPConfig().IPAddress, "Got in correct assigned IP Address")
	Assert(t, inbIPAddr == mockNMD.GetInbandIPConfig().IPAddress, "Got in correct assigned IP Address to inband")
}

func TestDHCPSpecControllers(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureNoVendorAtrrs)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")
	err = ipClient.DoDHCPConfig()
	defer ipClient.StopDHCPConfig()
	time.Sleep(2 * time.Second)
	_ = ipClient.GetInterfaceIPs()

	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	curState := ipClient.dhcpState.CurState
	AssertEquals(t, dhcpDone.String(), curState, "DHCP State must be done as the vendor options are specified via spec")

	// Ensure that there are no VeniceIPs
	AssertEquals(t, true, ipClient.dhcpState.VeniceIPs["1.1.1.1"], " dhcp venice IPs must match static controllers")

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")
}

func TestDHCPValidVendorAttributes241Code(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureValidVendorAttrs241)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")

	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, dhcpDone.String(), ipClient.dhcpState.CurState, "DHCP State must reflect DHCP Done")

	// Ensure that there are expected Venice IPs
	veniceIPs := strings.Split(option241VeniceIPs, ",")
	for _, v := range veniceIPs {
		AssertEquals(t, true, ipClient.dhcpState.VeniceIPs[v], "Failed to find a Venice IP. %v", v)
	}

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")
}

func TestDHCPValidVendorAttributes241CodeMultiple(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureValidVendorAttrs241Multiple)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")

	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, dhcpDone.String(), ipClient.dhcpState.CurState, "DHCP State must reflect DHCP Done")

	// Ensure that there are expected Venice IPs
	veniceIPs := strings.Split(option241VeniceIPsMultiple, ",")
	for _, v := range veniceIPs {
		AssertEquals(t, true, ipClient.dhcpState.VeniceIPs[v], "Failed to find a Venice IP. %v", v)
	}

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")
}

func TestDHCPValidVendorAttributes241and242Code(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureValidVendorAttrs241and242)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")

	//Create a mock interfaces for testing the interface IP config
	interfaceIds := []int8{0}
	for _, interfaceID := range interfaceIds {
		mockInterface := &netlink.Dummy{
			LinkAttrs: netlink.LinkAttrs{
				Name:   DSCIfIDToInterfaceName[interfaceID],
				TxQLen: 1000,
			},
		}

		// Create the mock interface
		if err := netlink.LinkAdd(mockInterface); err != nil {
			if !strings.Contains(err.Error(), "file exists") {
				log.Info("File exists")
			}
			log.Info("Interface already present. Continuing...")
		}

		netlink.LinkSetUp(mockInterface)
	}

	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP: %v", err)

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, dhcpDone.String(), ipClient.dhcpState.CurState, "DHCP State must reflect DHCP Done")

	// Ensure that there are expected Venice IPs
	veniceIPs := strings.Split(option241VeniceIPsMultiple, ",")
	for _, v := range veniceIPs {
		AssertEquals(t, true, ipClient.dhcpState.VeniceIPs[v], "Failed to find a Venice IP. %v", v)
	}

	// Ensure that there are expected interface IPs
	interfaceIPs := strings.Split(option242InterfaceIPs, ",")
	for _, i := range interfaceIPs {
		ipInfo := strings.Split(i, " ")

		var found bool
		var interfaceIPInfo InterfaceIP
		for _, ip := range ipClient.dhcpState.InterfaceIPs {
			if ipInfo[0] == strconv.Itoa(int(ip.IfID)) {
				found = true
				interfaceIPInfo = ip
				break
			}
		}

		AssertEquals(t, true, found, "Failed to find a Interface IP. %v", i)

		AssertEquals(t, strconv.Itoa(int(interfaceIPInfo.PrefixLen)), ipInfo[1], "Failed to find a Interface IP. %v", i)
		AssertEquals(t, interfaceIPInfo.IPAddress.String(), ipInfo[2], "Failed to find a Interface IP. %v", i)
		AssertEquals(t, interfaceIPInfo.GwIP.String(), ipInfo[3], "Failed to find a Interface IP. %v", i)

		//Check if the ip address assigned on DSC interface is correct
		ifid, _ := strconv.Atoi(ipInfo[0])
		mockInterface, err := netlink.LinkByName(DSCIfIDToInterfaceName[int8(ifid)])
		AssertOk(t, err, "Failed to find link: %v. Err: %v", DSCIfIDToInterfaceName[int8(ifid)], err)
		if err != nil {
			continue
		}

		ipAddresses, err := netlink.AddrList(mockInterface, netlink.FAMILY_V4)

		var ok bool
		for _, a := range ipAddresses {
			if a.IP.Equal(interfaceIPInfo.IPAddress) {
				ok = true
				break
			}
		}

		AssertEquals(t, true, ok, "The interface IP Address should match vendor specified IP address")
		break
	}

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")

	//Tear Down the mock interfaces created
	for _, interfaceID := range interfaceIds {
		mockIntf, err := netlink.LinkByName(DSCIfIDToInterfaceName[interfaceID])
		if err != nil {
			log.Errorf("TearDown Failed to look up the interfaces. Err: %v", err)
			continue
		}

		if err := netlink.LinkDel(mockIntf); err != nil {
			log.Errorf("TearDown Failed to delete the interfaces. Err: %v", err)
		}
	}
}
func TestDHCPValidClasslessStaticRoutesOption(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureValidClasslessStaticRoutes)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")

	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, dhcpDone.String(), ipClient.dhcpState.CurState, "DHCP State must reflect DHCP Done")

	// Ensure that there are expected static routes
	staticRoutes := strings.Split(optionClasslessStaticRoutes, ",")
	for _, r := range staticRoutes {
		routeInfo := strings.Split(r, " ")

		var found bool
		var dscRouteInfo StaticRoute
		for _, route := range ipClient.dhcpState.StaticRoutes {
			if routeInfo[0] == strconv.Itoa(int(route.DestPrefixLen)) && routeInfo[1] == route.DestAddr.String() {
				found = true
				dscRouteInfo = route
				break
			}
		}

		AssertEquals(t, true, found, "Failed to find a static route for destination. %v", r)
		AssertEquals(t, dscRouteInfo.NextHopAddr.String(), routeInfo[2], "Failed to find a static route for destination. %v", r)
	}

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")
	sRoutes := ipClient.GetStaticRoutes()
	if len(sRoutes) == 0 {
		t.Fatalf("Failed to get static routes.")
	}
}

//++++++++++++++++++++++++++++ Corner Test Cases ++++++++++++++++++++++++++++++++++++++++

func TestDHCPValidVendorAttributes(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureValidVendorAttrs)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")

	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	state := ipClient.GetDHCPState()
	AssertEquals(t, dhcpDone.String(), state, "DHCP State must reflect DHCP Done")

	// Ensure that there are expected Venice IPs
	veniceIPs := strings.Split(veniceIPs, ",")
	for _, v := range veniceIPs {
		AssertEquals(t, true, ipClient.dhcpState.VeniceIPs[v], "Failed to find a Venice IP. %v", v)
	}

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")
}

func TestDHCPRenewal(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureValidVendorAttrs)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")
	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}

	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, dhcpDone.String(), ipClient.dhcpState.CurState, "DHCP State must reflect DHCP Done")

	// Ensure that there are expected Venice IPs
	veniceIPs := strings.Split(veniceIPs, ",")
	for _, v := range veniceIPs {
		AssertEquals(t, true, ipClient.dhcpState.VeniceIPs[v], "Failed to find a Venice IP. %v", v)
	}
	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")
	ackPktBeforeRenewal := ipClient.dhcpState.AckPacket
	time.Sleep(2 * leaseDuration)
	ackPktAfterRenewal := ipClient.dhcpState.AckPacket
	if bytes.Equal(ackPktBeforeRenewal, ackPktAfterRenewal) {

	}
}

func TestDHCPEmptyVendorAttributes(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureEmptyVendorAtrs)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")

	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, missingVendorAttributes.String(), ipClient.dhcpState.CurState, "DHCP State must reflect Missing Vendor Attributes")

	// Ensure that there are no VeniceIPs
	AssertEquals(t, 0, len(ipClient.dhcpState.VeniceIPs), "On Missing Vendor Attributes VeniceIPs in dhcp state should be empty")

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	found := false
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, false, found, "The interface IP Address should be empty as no vendor attributes are available")
}

func TestDHCPMalformedVendorAttributesOption43And60(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureMalformedVendorAttrsMismatchOption43And60)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")
	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, missingVendorAttributes.String(), ipClient.dhcpState.CurState, "DHCP State must reflect Missing Vendor Attributes")

	// Ensure that there are no VeniceIPs
	AssertEquals(t, 0, len(ipClient.dhcpState.VeniceIPs), "On Missing Vendor Attributes VeniceIPs in dhcp state should be empty")

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	found := false
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, false, found, "The interface IP address must be empty for malformed option 43 and 60")
}

func TestDHCPMalformedVendorAttributes(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureMalformedVendorAttrs)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")
	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, missingVendorAttributes.String(), ipClient.dhcpState.CurState, "DHCP State must reflect Missing Vendor Attributes")

	// Ensure that there are no VeniceIPs
	AssertEquals(t, 0, len(ipClient.dhcpState.VeniceIPs), "On Missing Vendor Attributes VeniceIPs in dhcp state should be empty")

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	found := false
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, false, found, "The interface IP address must be empty for malformed vendor attributes")
}

func TestDHCPMalformedVendorAttributesDSCInterfaceIPs(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureMalformedVendorAttrsDSCInterfaceIPs)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")

	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP: %v", err)

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, dhcpDone.String(), ipClient.dhcpState.CurState, "DHCP State must reflect DHCP Done")

	// Ensure that there are expected Venice IPs
	veniceIPs := strings.Split(option241VeniceIPsMultiple, ",")
	for _, v := range veniceIPs {
		AssertEquals(t, true, ipClient.dhcpState.VeniceIPs[v], "Failed to find a Venice IP. %v", v)
	}

	// Ensure that no interface IPs are recorded
	AssertEquals(t, 0, len(ipClient.dhcpState.InterfaceIPs), "On incomplete Option 242 in vendor attributes, Interface IPs in dhcp state should be empty")

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")
}

func TestDHCPInvalidVendorAttributesDSCInterfaceIPs(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureInvalidInfoVendorAttrsDSCInterfaceIPs)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")

	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP: %v", err)

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, dhcpDone.String(), ipClient.dhcpState.CurState, "DHCP State must reflect DHCP Done")

	// Ensure that there are expected Venice IPs
	veniceIPs := strings.Split(option241VeniceIPsMultiple, ",")
	for _, v := range veniceIPs {
		AssertEquals(t, true, ipClient.dhcpState.VeniceIPs[v], "Failed to find a Venice IP. %v", v)
	}

	// Ensure that no interface IPs are recorded
	AssertEquals(t, 0, len(ipClient.dhcpState.InterfaceIPs), "On invalid info in Option 242 vendor attributes, Interface IPs in dhcp state should be empty")

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")
}

func TestDHCPMalformedClasslessStaticRoutesOption(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureMalformedClasslessStaticRoutes)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")

	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, dhcpDone.String(), ipClient.dhcpState.CurState, "DHCP State must reflect DHCP Done")

	// Ensure that no static routes are recorded
	AssertEquals(t, 0, len(ipClient.dhcpState.StaticRoutes), "On malformed info in option ClasslessRouteFormat, static routes in dhcp state should be empty")

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")
}

func TestDHCPInvalidClasslessStaticRoutesOption(t *testing.T) {
	var d dhcpSrv
	err := d.setup(configureInvalidClasslessStaticRoutes)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")

	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, dhcpDone.String(), ipClient.dhcpState.CurState, "DHCP State must reflect DHCP Done")

	// Ensure that no static routes are recorded
	AssertEquals(t, 0, len(ipClient.dhcpState.StaticRoutes), "On invalid info in option ClasslessRouteFormat, static routes in dhcp state should be empty")

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")
}

func TestDHCPTimedout(t *testing.T) {
	var d dhcpSrv
	err := d.setup(noDHCP)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")
	err = ipClient.DoDHCPConfig()
	time.Sleep(15 * time.Second)
	AssertEquals(t, ipClient.dhcpState.CurState, dhcpTimedout.String(), "DHCP should timeout when there is no dhcp server configured")
}

func TestDHCPRetries(t *testing.T) {
	var d dhcpSrv
	mockNMD := mock.CreateMockNMD(t.Name())
	err := d.setup(noDHCP)
	AssertOk(t, err, "Setup Failed")
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")
	err = ipClient.DoDHCPConfig()
	time.Sleep(2 * time.Minute)
	AssertEquals(t, ipClient.dhcpState.CurState, dhcpTimedout.String(), "DHCP should timeout when there is no dhcp server configured")
	d.tearDown()

	err = d.setup(configureValidVendorAttrs)
	AssertOk(t, err, "Setup Failed")
	time.Sleep(2 * time.Minute)
	d.tearDown()
}

func TestInvalidInterface(t *testing.T) {
	mockNMD := mock.CreateMockNMD(t.Name())
	_, err := NewIPClient(mockNMD, "Some Invalid Interface", "")
	Assert(t, err != nil, "IPClient creates on non existent interfaces must fail")
}

func TestInvalidStaticIPAssignment(t *testing.T) {
	var d dhcpSrv
	err := d.setup(noDHCP)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	// override mock nmd with bad ip config
	badIPConfig := &cluster.IPConfig{
		IPAddress: "0.0.0.256/33",
	}
	mockNMD.SetIPConfig(badIPConfig)
	AssertOk(t, err, "IPClient creates must succeed")
	ipAddr, _, err := ipClient.DoStaticConfig()
	Assert(t, err != nil, "Static IP Assignment with bad config must fail")
	AssertEquals(t, "", ipAddr, "Static IP Address with bad ip config must not return a valid ip address")
}

func TestRenewalLoopPanics(t *testing.T) {
	t.Skip("This is no longer needed")
	var d dhcpSrv
	err := d.setup(configureNoVendorAtrrs)
	AssertOk(t, err, "Setup Failed")
	defer d.tearDown()
	AssertOk(t, err, "Failed to start DHCP Server for testing")
	mockNMD := mock.CreateMockNMD(t.Name())
	ipClient, err := NewIPClient(mockNMD, NaplesMockInterface, "")
	AssertOk(t, err, "IPClient creates must succeed")
	// Clear spec controllers
	mockNMD.Naples.Spec.Controllers = []string{}
	err = ipClient.DoDHCPConfig()
	time.Sleep(5 * time.Second)
	// Check DHCP Config should succeed
	AssertOk(t, err, "Failed to perform DHCP")

	// Ensure obtained IP Addr is in the allocated subnet
	AssertEquals(t, true, allocSubnet.Contains(ipClient.dhcpState.IPNet.IP), "Obtained a YIADDR is not in the expected subnet")

	// Ensure dhcp state is missing vendor attributes
	AssertEquals(t, missingVendorAttributes.String(), ipClient.dhcpState.CurState, "DHCP State must reflect Missing Vendor Attributes")

	// Ensure that there are no VeniceIPs
	AssertEquals(t, 0, len(ipClient.dhcpState.VeniceIPs), "On Missing Vendor Attributes VeniceIPs in dhcp state should be empty")

	// Ensure the IP Assigned on the interface is indeed the YIADDR
	ipAddr, err := netlink.AddrList(ipClient.primaryIntf, netlink.FAMILY_V4)
	AssertOk(t, err, "Must be able to look up IP Address for mock interface")
	var found bool
	for _, a := range ipAddr {
		if a.IP.Equal(ipClient.dhcpState.IPNet.IP) {
			found = true
			break
		}
	}
	AssertEquals(t, true, found, "The interface IP Address should match YIADDR")
	d.conn.Close()
	time.Sleep(leaseDuration)

	ipClient.StopDHCPConfig()
}

func TestSecondaryMgmtInterfacesApulu(t *testing.T) {
	dscMAC, _ := net.ParseMAC("00:10:fa:6e:38:4a")

	// Create fake mock interfaces
	fakeDsc0 := &netlink.Veth{
		LinkAttrs: netlink.LinkAttrs{
			Name:         "dsc0",
			TxQLen:       1000,
			HardwareAddr: dscMAC,
		},
		PeerName: "dsc1",
	}
	defer netlink.LinkDel(fakeDsc0)
	err := netlink.LinkAdd(fakeDsc0)
	AssertOk(t, err, "Failed to create mock interfaces")
	secondaryApuluLinks := getSecondaryMgmtLink(globals.NaplesPipelineApollo, "mock")
	if len(secondaryApuluLinks) != 2 {
		t.Fatalf("Failed to get secondary mgmt links for Apulu")
	}
}

func TestSecondaryMgmtInterfacesIris(t *testing.T) {
	dscMAC, _ := net.ParseMAC("00:10:fa:6e:38:4b")

	// Create fake mock interfaces
	fakeOOB := &netlink.Veth{
		LinkAttrs: netlink.LinkAttrs{
			Name:         "oob_mnic0",
			TxQLen:       1000,
			HardwareAddr: dscMAC,
		},
		PeerName: "foo",
	}
	defer netlink.LinkDel(fakeOOB)
	err := netlink.LinkAdd(fakeOOB)
	AssertOk(t, err, "Failed to create mock interfaces")
	secondaryApuluLinks := getSecondaryMgmtLink(globals.NaplesPipelineIris, "mock")
	if len(secondaryApuluLinks) != 1 {
		t.Fatalf("Failed to get secondary mgmt links for Apulu")
	}

	netlink.LinkDel(fakeOOB)
}

//++++++++++++++++++++++++++++ Test Utility Functions ++++++++++++++++++++++++++++++++++++++++

func (d *dhcpSrv) setup(configureVendorAttrs int) error {
	clientMAC, _ := net.ParseMAC("42:42:42:42:42:42")

	dhcpClientMock := &netlink.Veth{
		LinkAttrs: netlink.LinkAttrs{
			Name:         NaplesMockInterface,
			TxQLen:       1000,
			HardwareAddr: clientMAC,
		},
		PeerName: dhcpServerIntf,
	}

	// Create the veth pair
	if err := netlink.LinkAdd(dhcpClientMock); err != nil {
		if !strings.Contains(err.Error(), "file exists") {
			fmt.Println("alok " + err.Error())
			return err
		}
		log.Info("Interface already present. Continuing...")
	}
	if err := netlink.LinkSetARPOn(dhcpClientMock); err != nil {
		fmt.Println("alok1 " + err.Error())
		return err
	}

	netlink.LinkSetUp(dhcpClientMock)
	// Assign IP Address statically for the server
	srvIntf, err := netlink.LinkByName(dhcpServerIntf)
	if err != nil {
		if !strings.Contains(err.Error(), "file exists") {
			log.Errorf("Failed to find the server interface")
			fmt.Println("alok2 " + err.Error())
			return err
		}
		log.Info("Interface  already up. Continuing...")
	}
	addr, _ := netlink.ParseAddr("172.16.10.1/28")

	if err := netlink.AddrAdd(srvIntf, addr); err != nil {
		if !strings.Contains(err.Error(), "file exists") {
			log.Errorf("Failed to assign ip address %v to interface dhcpmock. Err: %v", addr.IP.String(), err)
			fmt.Println("alok3 " + err.Error())
			return err
		}
		log.Info("Address already present. Continuing...")
	}

	if err := netlink.LinkSetUp(srvIntf); err != nil {
		fmt.Println("alok4 " + err.Error())
		log.Errorf("Failed to bring up the interface. Err: %v", err)
		return err
	}

	if configureVendorAttrs != noDHCP {
		go d.startDHCPServer(configureVendorAttrs)
	}

	return nil
}

func (d *dhcpSrv) tearDown() error {
	mockIntf, err := netlink.LinkByName(NaplesMockInterface)
	if err != nil {
		log.Errorf("TearDown Failed to look up the interfaces. Err: %v", err)
		return err
	}

	if err := netlink.LinkDel(mockIntf); err != nil {
		log.Errorf("TearDown Failed to delete the interfaces. Err: %v", err)
		return err
	}
	if d.conn != nil {
		return d.conn.Close()
	}
	return nil
}

type lease struct {
	nic    string    // Client's CHAddr
	expiry time.Time // When the lease expires
}

type DHCPHandler struct {
	ip            net.IP        // Server IP to use
	options       dhcp.Options  // Options to send to DHCP Clients
	start         net.IP        // Start of IP range to distribute
	leaseRange    int           // Number of IPs to distribute (starting from start)
	leaseDuration time.Duration // Lease period
	leases        map[int]lease // Map to keep track of leases
}

func (h *DHCPHandler) ServeDHCP(p dhcp.Packet, msgType dhcp.MessageType, options dhcp.Options) (d dhcp.Packet) {
	switch msgType {

	case dhcp.Discover:
		free, nic := -1, p.CHAddr().String()
		log.Infof("DISCOVER FOR: %v", nic)
		for i, v := range h.leases { // Find previous lease
			if v.nic == nic {
				free = i
				goto reply
			}
		}
		if free = h.freeLease(); free == -1 {
			return
		}
	reply:
		return dhcp.ReplyPacket(p, dhcp.Offer, h.ip, dhcp.IPAdd(h.start, free), h.leaseDuration,
			h.options.SelectOrderOrAll(options[dhcp.OptionParameterRequestList]))

	case dhcp.Request:
		if server, ok := options[dhcp.OptionServerIdentifier]; ok && !net.IP(server).Equal(h.ip) {
			return nil // Message not for this dhcp server
		}
		reqIP := net.IP(options[dhcp.OptionRequestedIPAddress])
		if reqIP == nil {
			reqIP = net.IP(p.CIAddr())
		}

		if len(reqIP) == 4 && !reqIP.Equal(net.IPv4zero) {
			if leaseNum := dhcp.IPRange(h.start, reqIP) - 1; leaseNum >= 0 && leaseNum < h.leaseRange {
				if l, exists := h.leases[leaseNum]; !exists || l.nic == p.CHAddr().String() {
					h.leases[leaseNum] = lease{nic: p.CHAddr().String(), expiry: time.Now().Add(h.leaseDuration)}
					return dhcp.ReplyPacket(p, dhcp.ACK, h.ip, reqIP, h.leaseDuration,
						h.options.SelectOrderOrAll(options[dhcp.OptionParameterRequestList]))
				}
			}
		}
		return dhcp.ReplyPacket(p, dhcp.NAK, h.ip, nil, 0, nil)

	case dhcp.Release, dhcp.Decline:
		nic := p.CHAddr().String()
		for i, v := range h.leases {
			if v.nic == nic {
				delete(h.leases, i)
				break
			}
		}
	}
	return nil
}

func (h *DHCPHandler) freeLease() int {
	now := time.Now()
	b := rand.Intn(h.leaseRange) // Try random first
	for _, v := range [][]int{[]int{b, h.leaseRange}, []int{0, b}} {
		for i := v[0]; i < v[1]; i++ {
			if l, ok := h.leases[i]; !ok || l.expiry.Before(now) {
				return i
			}
		}
	}
	return -1
}

func (d *dhcpSrv) startDHCPServer(configureVendorAttrs int) error {
	var opts dhcp.Options
	serverIP := net.IP{172, 16, 10, 1}

	switch configureVendorAttrs {
	case configureNoVendorAtrrs:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:       []byte{255, 255, 255, 240},
			dhcp.OptionRouter:           []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer: []byte(serverIP), // Presuming Server is also your DNS server
		}
	case configureEmptyVendorAtrs:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:            []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:      []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier: []byte(PensandoIdentifier),
		}
	case configureMalformedVendorAttrsMismatchOption43And60:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorSpecificInformation: []byte(PensandoIdentifier),
		}
	case configureMalformedVendorAttrs:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     []byte(PensandoIdentifier),
			dhcp.OptionVendorSpecificInformation: []byte("ř"),
		}
	case configureValidVendorAttrs:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     PensandoDHCPRequestOption.Value,
			dhcp.OptionVendorSpecificInformation: []byte(veniceIPs),
		}
	case configureValidVendorAttrs241:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     PensandoDHCPRequestOption.Value,
			dhcp.OptionVendorSpecificInformation: []byte{241, 4, 1, 1, 1, 1},
		}
	case configureValidVendorAttrs241Multiple:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     PensandoDHCPRequestOption.Value,
			dhcp.OptionVendorSpecificInformation: []byte{241, 8, 2, 2, 2, 2, 3, 3, 3, 3},
		}
	case configureValidVendorAttrs241and242:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     PensandoDHCPRequestOption.Value,
			dhcp.OptionVendorSpecificInformation: []byte{241, 8, 2, 2, 2, 2, 3, 3, 3, 3, 242, 20, 0, 28, 20, 20, 20, 4, 20, 20, 20, 1, 1, 28, 20, 20, 20, 5, 20, 20, 20, 2},
		}
	case configureMalformedVendorAttrsDSCInterfaceIPs:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     PensandoDHCPRequestOption.Value,
			dhcp.OptionVendorSpecificInformation: []byte{241, 8, 2, 2, 2, 2, 3, 3, 3, 3, 242, 20, 0, 28, 20, 20, 20, 4},
		}
	case configureInvalidInfoVendorAttrsDSCInterfaceIPs:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     PensandoDHCPRequestOption.Value,
			dhcp.OptionVendorSpecificInformation: []byte{241, 8, 2, 2, 2, 2, 3, 3, 3, 3, 242, 20, 12, 28, 20, 20, 20, 4, 20, 20, 20, 1, 1, 33, 20, 20, 20, 5, 20, 20, 20, 2},
		}
	case configureValidClasslessStaticRoutes:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     PensandoDHCPRequestOption.Value,
			dhcp.OptionVendorSpecificInformation: []byte{241, 4, 1, 1, 1, 1},
			dhcp.OptionClasslessRouteFormat:      []byte{28, 20, 20, 20, 4, 20, 20, 20, 1, 28, 20, 20, 20, 5, 20, 20, 20, 2},
		}
	case configureMalformedClasslessStaticRoutes:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     PensandoDHCPRequestOption.Value,
			dhcp.OptionVendorSpecificInformation: []byte{241, 4, 1, 1, 1, 1},
			dhcp.OptionClasslessRouteFormat:      []byte{28, 20, 20, 20, 4, 20},
		}
	case configureInvalidClasslessStaticRoutes:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     PensandoDHCPRequestOption.Value,
			dhcp.OptionVendorSpecificInformation: []byte{241, 4, 1, 1, 1, 1},
			dhcp.OptionClasslessRouteFormat:      []byte{34, 20, 20, 20, 4, 20, 20, 20, 1},
		}
	}

	handler := &DHCPHandler{
		ip:            serverIP,
		leaseDuration: leaseDuration,
		start:         net.ParseIP(ipAddrStart),
		leaseRange:    10,
		leases:        make(map[int]lease, 10),
		options:       opts,
	}
	c, err := conn.NewUDP4BoundListener(dhcpServerIntf, ":67")
	if err != nil {
		log.Errorf("Failed to start DHCP Server. Err: %v", err)
		return err
	}
	d.conn = c
	go dhcp.Serve(d.conn, handler)
	return nil
}
