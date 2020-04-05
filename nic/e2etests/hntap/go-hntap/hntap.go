package hntap

import (
	"flag"
	"fmt"
	"net"
	"strconv"
	"time"

	"github.com/google/gopacket"
	"github.com/google/gopacket/pcap"
	log "github.com/sirupsen/logrus"
	"github.com/pensando/netlink"
)

// setupVethPairWithBridge creates veth interface pairs with specified name
func setupVethPairWithBridge(brname, brIPnet,
	local, localIPnet,
	peer, peerIPnet,
	peerNSpid string) error {

	log.Infof("Creating bridge %s with IP %s", brname, brIPnet)
	br := &netlink.Bridge{LinkAttrs: netlink.LinkAttrs{Name: brname}}
	if err := netlink.LinkAdd(br); err != nil {
		log.Errorf("Could not create bridge %s: %v", brname, err)
		return err
	}
	brIP, brIPNet, err := net.ParseCIDR(brIPnet)
	if err != nil {
		log.Errorf("Error parsing IP %s for bridge %s: %v", brIPnet, brname, err)
		return err
	}

	var address = &net.IPNet{IP: brIP, Mask: brIPNet.Mask}
	var addr = &netlink.Addr{IPNet: address}
	if err := netlink.AddrAdd(br, addr); err != nil {
		log.Errorf("Error configuring IP(%s) on the bridge %s: %v", brIPnet, brname, err)
		return err
	}

	log.Infof("Creating Veth pairs with local/peer names: %s, %s", local, peer)

	// Veth pair params
	veth := &netlink.Veth{
		LinkAttrs: netlink.LinkAttrs{
			Name:   local,
			TxQLen: 0,
		},
		PeerName: peer,
	}

	// Create the veth pair
	if err := netlink.LinkAdd(veth); err != nil {
		log.Errorf("error creating veth pair: %v", err)
		return err
	}

	lintf, err := netlink.LinkByName(local)
	if err != nil {
		log.Errorf("Could not find local interface %s: %v", local, err)
		return err
	}

	if err := netlink.LinkSetUp(lintf); err != nil {
		log.Errorf("Error bringing UP local interface %s: %v", local, err)
		return err
	}

	IP, IPNet, err := net.ParseCIDR(localIPnet)
	if err != nil {
		log.Errorf("Error parsing IP %s for interface %s: %v", localIPnet, local, err)
		return err
	}

	address = &net.IPNet{IP: IP, Mask: IPNet.Mask}
	addr = &netlink.Addr{IPNet: address}
	if err := netlink.AddrAdd(lintf, addr); err != nil {
		log.Errorf("Error configuring IP(%s) on the interface %s: %v", localIPnet, local, err)
		return err
	}

	pintf, err := netlink.LinkByName(peer)
	if err != nil {
		log.Errorf("Could not find peer interface %s: %v", peer, err)
		return err
	}

	nsPid, err := strconv.Atoi(peerNSpid)
	if err != nil {
		log.Errorf("Atoi error %v", err)
	}

	// move to the desired netns
	err = netlink.LinkSetNsPid(pintf, nsPid)
	if err != nil {
		log.Errorf("unable to move peer interface %s to pid %d. Error: %v",
			peer, nsPid, err)
		return err
	}

	if err := netlink.LinkSetUp(pintf); err != nil {
		log.Errorf("Error bringing UP peer interface %s: %v", peer, err)
		return err
	}

	pIP, pIPNet, err := net.ParseCIDR(peerIPnet)
	if err != nil {
		log.Errorf("Error parsing IP %s for interface %s: %v", peerIPnet, peer, err)
		return err
	}

	address = &net.IPNet{IP: pIP, Mask: pIPNet.Mask}
	addr = &netlink.Addr{IPNet: address}
	if err := netlink.AddrAdd(pintf, addr); err != nil {
		log.Errorf("Error configuring IP(%s) on the interface %s: %v", peerIPnet, peer, err)
		return err
	}

	return nil
}

func setupTapIf(name, IPsubnet string) error {
	log.Infof("Creating Tap interface %s with IP %s", name, IPsubnet)
	intf := &netlink.Dummy{
		LinkAttrs: netlink.LinkAttrs{
			Name: name,
		},
	}
	if err := netlink.LinkAdd(intf); err != nil {
		log.Errorf("Error creating interface %s: %v", name, err)
		return err
	}

	if err := netlink.LinkSetUp(intf); err != nil {
		log.Errorf("Error bringing UP interface %s: %v", name, err)
		return err
	}

	IP, IPNet, err := net.ParseCIDR(IPsubnet)
	if err != nil {
		log.Errorf("Error parsing IP %s for interface %s: %v", IPsubnet, name, err)
		return err
	}

	var address = &net.IPNet{IP: IP, Mask: IPNet.Mask}
	var addr = &netlink.Addr{IPNet: address}
	if err := netlink.AddrAdd(intf, addr); err != nil {
		log.Errorf("Error configuring IP(%s) on the interface %s: %v", IPsubnet, name, err)
		return err
	}

	return nil
}

func cleanupTapIf(name string) error {
	intf, err := netlink.LinkByName(name)
	if err != nil {
		log.Errorf("Could not find interface %s: %v", name, err)
		return err
	}
	if err := netlink.LinkDel(intf); err != nil {
		log.Errorf("Error deleting interface %s: %v", name, err)
		return err
	}
	return nil
}

func createTapIfHost() error {
	log.Infof("Creating Tap interface hntap_host0 for Host-tap")
	setupTapIf("hntap_host0", "10.0.100.1/24")
	return nil
}

func createTapIfNet() error {
	log.Infof("Creating Tap interface hntap_net0 for Network-tap")
	setupTapIf("hntap_net0", "10.0.200.1/24")
	return nil
}

func cleanupTapIfHost() error {
	log.Infof("Cleaning up Tap interface hntap_host0 for Host-tap")
	return cleanupTapIf("hntap_host0")
}

func cleanupTapIfNet() error {
	log.Infof("Cleaning up Tap interface hntap_net0 for Network-tap")
	return cleanupTapIf("hntap_net0")
}

func createVethPairHost() error {
	return setupVethPairWithBridge("hntap_hostbr0", "10.0.101.1/24",
		"hntap_hostveth0", "10.0.101.2/24",
		"hntap_hostveth1", "10.0.101.3/24",
		"100") // NS-PID to be set properly
}

func createVethPairNet() error {
	return setupVethPairWithBridge("hntap_netbr0", "10.0.201.1/24",
		"hntap_netveth0", "10.0.201.2/24",
		"hntap_netveth1", "10.0.201.3/24",
		"200") // NS-PID to be set properly
}

func cleanupVethPairHost() error {
	log.Infof("Cleaning up Veth pair and bridge for Host-tap")
	cleanupTapIf("hntap_hostbr0")
	cleanupTapIf("hntap_hostveth0")
	cleanupTapIf("hntap_hostveth1")
	return nil
}

func cleanupVethPairNet() error {
	log.Infof("Cleaning up Veth pair and bridge for Network-tap")
	cleanupTapIf("hntap_netbr0")
	cleanupTapIf("hntap_netveth0")
	cleanupTapIf("hntap_netveth1")
	return nil
}

var handle *pcap.Handle
var device string

func listenOnDevice(device string, packetCount int) error {

	// Open the Host device
	handle, err := pcap.OpenLive(device, 1024, false, 30*time.Second)
	if err != nil {
		log.Errorf("Could not open device %s for packet capture: %v", device, err)
		log.Fatal(err)
		return err
	}
	defer handle.Close()
	log.Infof("Opened device %s to capture packets", device)

	// Use the handle as a packet source to process all packets
	var packetRcvd int
	packetSource := gopacket.NewPacketSource(handle, handle.LinkType())
	for packet := range packetSource.Packets() {
		// Process packet here
		fmt.Println(packet)

		// Use packet.Data() ******
		packetRcvd++
		log.Infof("Device: %s Received packet count %v", device, packetRcvd)
		if packetRcvd > packetCount {
			break
		}
	}
	return nil
}

func handleHostTx(done chan bool) {

	if hntapMode == "native" {
		device = "hntap_host0"
	} else {
		device = "hntap_hostveth0"
	}

	listenOnDevice(device, 3)

	done <- true
}

func handleHostRx(done chan bool) {
	done <- true
}

func handleNetTx(done chan bool) {
	done <- true
}

func handleNetRx(done chan bool) {

	if hntapMode == "native" {
		device = "hntap_net0"
	} else {
		device = "hntap_netveth0"
	}

	listenOnDevice(device, 3)
	done <- true
}

func handleHostTxRx(done chan bool) {
	hosttxChan := make(chan bool)
	hostrxChan := make(chan bool)
	go handleHostTx(hosttxChan)
	go handleHostRx(hostrxChan)
	<-hosttxChan
	<-hostrxChan

	cleanupTapIfHost()
	cleanupVethPairHost()
	done <- true
}

func handleNetworkTxRx(done chan bool) {
	nettxChan := make(chan bool)
	netrxChan := make(chan bool)
	go handleNetTx(nettxChan)
	go handleNetRx(netrxChan)
	<-nettxChan
	<-netrxChan

	cleanupTapIfNet()
	cleanupVethPairNet()
	done <- true
}

var hntapMode string

func main() {

	argptr := flag.String("type", "native", "a string")
	flag.Parse()

	fmt.Println("argptr:", *argptr)
	hntapMode = *argptr

	/*
	 * By default, create a TAP interface each for host side and network side.
	 * With the "enable-vm" option, we'll create a Veth-pair each for host side and
	 * network side, with a bridge to communicate across VMs/containers simulating the
	 * host/network.
	 */

	if *argptr == "native" {
		log.Infof("Running Host+Network tapper in native mode")
		createTapIfHost()
		createTapIfNet()

	} else {
		log.Infof("Running in Host+Network tapper multi-VM mode")
		createVethPairHost()
		createVethPairNet()
	}

	hostchan := make(chan bool)
	netchan := make(chan bool)
	go handleHostTxRx(hostchan)
	go handleNetworkTxRx(netchan)
	<-hostchan
	<-netchan
	log.Infof("Done with hntap")
}
