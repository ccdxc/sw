// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package fswitch

import (
	"errors"
	"fmt"
	"net"
	"strings"
	"sync"

	"github.com/google/gopacket"
	"github.com/google/gopacket/pcap"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"

	"github.com/mdlayher/ethernet"
	"github.com/mdlayher/raw"
)

// constants
const (
	DefaultVlan = 0
	DefaultVRF  = "default"
)

// EthTypeAll is an alias to syscall.ETH_P_ALL
const EthTypeAll = 0x3

// DpIntf is the interface provided by the creator
type DpIntf interface {
	EndpointLearnNotif(ep *netproto.Endpoint) error
}

// FwdEntry forwarding lookup entry
type FwdEntry struct {
	MacAddr  net.HardwareAddr   // Mac addr
	IPv4Addr net.IP             // IP addr
	Network  string             // network
	Vrf      string             // VRF
	Lif      *LifEntry          // outgoing lif
	Endpoint *netproto.Endpoint // endpoint
}

// LifEntry represents a <port, vlan> combination
type LifEntry struct {
	Port    string // port, this lif exists on
	Vlan    uint32 // LIF's vlan
	Network string // network this LIF belongs to
}

// Fswitch is a switch instance
type Fswitch struct {
	sync.Mutex                                    // lock the switch for map writes
	dpi             DpIntf                        // interface provided by the creator
	uplink          string                        // uplink interface
	ports           map[string]string             // workload facing ports
	socks           map[string]*raw.Conn          // ARP listeners
	pcapHandles     map[string]*pcap.Handle       // Pcap handles
	localEndpoints  map[string]*netproto.Endpoint // local endpoints
	remoteEndpoints map[string]*netproto.Endpoint // remote endpoints
	macaddrTable    map[string]*FwdEntry          // forwarding table indexed by mac addr, network
	ipaddrTable     map[string]*FwdEntry          // forwarding table indexed by ip addr, vrf
	lifTable        map[string]*LifEntry          // Lif table indexed by port,vlan
}

// runListener loops forever and receives messages
func (fs *Fswitch) runListener(port string) error {
	// Select the interface to receive traffic.
	ifi, err := net.InterfaceByName(port)
	if err != nil {
		log.Errorf("failed to open interface %s: %v", port, err)
		return err
	}

	// Open a raw socket for EtherType
	sock, err := raw.ListenPacket(ifi, EthTypeAll)
	if err != nil {
		log.Errorf("failed to listen: %v", err)
		return err
	}

	// save it in db
	fs.socks[port] = sock

	// Keep reading frames.
	return fs.runPcapListener(port)
}

func (fs *Fswitch) runPcapListener(port string) error {
	var f ethernet.Frame

	// start a pcap listener on the port
	inactive, err := pcap.NewInactiveHandle(port)
	if err != nil {
		log.Fatalf("could not create: %v", err)
	}

	// create pcap handle
	handle, err := inactive.Activate()
	if err != nil {
		log.Fatal("PCAP Activate error:", err)
	}
	fs.pcapHandles[port] = handle

	// set direction to receive only incoming packets
	err = handle.SetDirection(pcap.DirectionIn)
	if err != nil {
		log.Fatal("PCAP SetDirection error:", err)
	}

	// find the pcap decoder
	dec, ok := gopacket.DecodersByLayerName["Ethernet"]
	if !ok {
		log.Fatalln("No decoder found")
	}

	// create packet source
	source := gopacket.NewPacketSource(handle, dec)
	source.NoCopy = true
	source.DecodeStreamsAsDatagrams = true

	// loop forever
	go func() {
		for packet := range source.Packets() {

			// Unpack Ethernet frame into Go representation.
			err = (&f).UnmarshalBinary(packet.Data())
			if err != nil {
				log.Fatalf("failed to unmarshal ethernet frame: %v", err)
			}

			// Display source of message and message itself.
			log.Debugf("Switch Received Ethernet frame [%s->%s] vlan {%+v} etype 0x%x: %v", f.Source.String(), f.Destination.String(), f.VLAN, uint(f.EtherType), f.Payload)

			// process the frame
			err = fs.processFrame(port, &f)
			if err != nil {
				log.Errorf("Error processing frame. Err: %v", err)
			}
		}
	}()

	return nil
}

// AddPort adds a port to switch
func (fs *Fswitch) AddPort(port string) error {
	// lock for db changes
	fs.Lock()
	defer fs.Unlock()

	// check if port already exists
	_, ok := fs.ports[port]
	if ok {
		log.Errorf("Port %s already exists", port)
		return errors.New("Port already esists")
	}

	// add it to db
	fs.ports[port] = port

	// start listener on the port
	return fs.runListener(port)
}

// DelPort removes a port from the switch
func (fs *Fswitch) DelPort(port string) error {
	// lock for db changes
	fs.Lock()
	defer fs.Unlock()

	// check if port already exists
	_, ok := fs.ports[port]
	if !ok {
		log.Errorf("Port %s does not exists", port)
		return errors.New("Port does not esists")
	}

	// delete from db
	delete(fs.ports, port)

	return nil
}

// SendFrame sends a frame out on a port
func (fs *Fswitch) SendFrame(port string, frame *ethernet.Frame) error {
	// find the socket by name
	fs.Lock()
	conn, ok := fs.socks[port]
	fs.Unlock()
	if !ok {
		log.Errorf("Could not find the connection for port %s", port)
		return errors.New("Connection not found")
	}

	// marshall the frame
	b, err := frame.MarshalBinary()
	if err != nil {
		log.Errorf("failed to marshal frame: %v", err)
		return err
	}

	// address
	addr := &raw.Addr{HardwareAddr: frame.Destination}

	// send the frame
	_, err = conn.WriteTo(b, addr)
	if err != nil {
		log.Errorf("Error sending the frame {%+v}. Err: %v", frame, err)
		return err
	}

	return nil
}

// addFwdEntry adds forwarding entries in ip and mac table
func (fs *Fswitch) addFwdEntry(ntwork string, ep *netproto.Endpoint, lif *LifEntry) error {
	// build forwarding entry
	mac, _ := net.ParseMAC(ep.Status.MacAddress)
	ip, _, _ := net.ParseCIDR(ep.Status.IPv4Address)
	fwd := FwdEntry{
		MacAddr:  mac,
		IPv4Addr: ip,
		Network:  ntwork,
		Vrf:      DefaultVRF,
		Lif:      lif,
		Endpoint: ep,
	}

	// add to mac table
	macKey := fmt.Sprintf("%s|%s", ntwork, mac.String())
	fs.macaddrTable[macKey] = &fwd

	// add to ip addr table
	ipKey := fmt.Sprintf("%s|%s", fwd.Vrf, fwd.IPv4Addr.String())
	fs.ipaddrTable[ipKey] = &fwd

	log.Infof("Added fwd entry: {%+v}, Lif: {%+v}", fwd, lif)

	return nil
}

// delFwdEntry deletes forwarding entry
func (fs *Fswitch) delFwdEntry(ntwork string, ep *netproto.Endpoint) error {
	// delete from mac table
	macKey := fmt.Sprintf("%s|%s", ntwork, strings.ToLower(ep.Status.MacAddress))
	delete(fs.macaddrTable, macKey)

	// delete from ip addr table
	ip, _, _ := net.ParseCIDR(ep.Status.IPv4Address)
	ipKey := fmt.Sprintf("%s|%s", DefaultVRF, ip.String())
	delete(fs.ipaddrTable, ipKey)

	return nil
}

// add lif adds a lif to lif table
func (fs *Fswitch) addLif(port, ntwork string, vlan uint32) (*LifEntry, error) {
	// create lif entry
	lif := LifEntry{
		Port:    port,
		Vlan:    vlan,
		Network: ntwork,
	}

	// add it to the table
	lifKey := fmt.Sprintf("%s|%d", port, vlan)
	fs.lifTable[lifKey] = &lif

	return &lif, nil
}

// delLif deletes a LIF from lif table
func (fs *Fswitch) delLif(port string, vlan uint32) error {
	// delete it from the table
	lifKey := fmt.Sprintf("%s|%d", port, vlan)
	delete(fs.lifTable, lifKey)

	return nil
}

// findLif finds a lif by vlan
func (fs *Fswitch) findLif(port string, vlan uint32) (*LifEntry, error) {
	// find the lif by port, vlan
	lifKey := fmt.Sprintf("%s|%d", port, vlan)
	lif, ok := fs.lifTable[lifKey]
	if !ok {
		// find the lif by wildcard
		lifKey = fmt.Sprintf("%s|%d", port, 0)
		lif, ok = fs.lifTable[lifKey]
		if !ok {
			return nil, errors.New("Lif not found")
		}
	}

	return lif, nil
}

// AddLocalEndpoint adds local endpoint
func (fs *Fswitch) AddLocalEndpoint(port string, ep *netproto.Endpoint) error {
	// add a lif for the local endpoint
	lif, err := fs.addLif(port, ep.Spec.NetworkName, ep.Status.UsegVlan)
	if err != nil {
		return err
	}

	// add a forwarding entry
	err = fs.addFwdEntry(ep.Spec.NetworkName, ep, lif)
	if err != nil {
		log.Errorf("Error adding fwd entry. Err: %v", err)
		return err
	}

	// add to local epdb
	fs.localEndpoints[ep.Name] = ep

	return nil
}

// DelLocalEndpoint deletes local endpoint
func (fs *Fswitch) DelLocalEndpoint(port string, ep *netproto.Endpoint) error {
	// delete fwd entries
	err := fs.delFwdEntry(ep.Spec.NetworkName, ep)
	if err != nil {
		log.Errorf("Error adding fwd entry. Err: %v", err)
		return err
	}

	// delete lif for the local endpoint
	err = fs.delLif(port, ep.Status.UsegVlan)
	if err != nil {
		return nil
	}

	// remove from local epdb
	delete(fs.localEndpoints, ep.Name)

	return nil
}

// AddRemoteEndpoint adds remote endpoint
func (fs *Fswitch) AddRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network) error {
	// find the uplink lif
	lif, err := fs.findLif(fs.uplink, nw.Status.AllocatedVlanID)
	if err != nil {
		log.Errorf("Could not find the uplink LIF %s, %d", fs.uplink, nw.Status.AllocatedVlanID)
		return err
	}

	// add a forwarding entry
	err = fs.addFwdEntry(ep.Spec.NetworkName, ep, lif)
	if err != nil {
		log.Errorf("Error adding fwd entry. Err: %v", err)
		return err
	}

	// add to remote epdb
	fs.remoteEndpoints[ep.Name] = ep

	return nil
}

// DelRemoteEndpoint deletes remote endpoint
func (fs *Fswitch) DelRemoteEndpoint(ep *netproto.Endpoint) error {
	// delete fwd entries
	err := fs.delFwdEntry(ep.Spec.NetworkName, ep)
	if err != nil {
		log.Errorf("Error adding fwd entry. Err: %v", err)
		return err
	}

	// remove from remote epdb
	delete(fs.remoteEndpoints, ep.Name)

	return nil
}

// CreateNetwork creates a network in datapath
func (fs *Fswitch) CreateNetwork(nw *netproto.Network) error {
	// create the port,vlan LIF on the uplink
	_, err := fs.addLif(fs.uplink, nw.Name, nw.Status.AllocatedVlanID)
	if err != nil {
		return err
	}

	return nil
}

// DeleteNetwork deletes a network from datapath
func (fs *Fswitch) DeleteNetwork(nw *netproto.Network) error {
	return fs.delLif(fs.uplink, nw.Status.AllocatedVlanID)
}

// NewFswitch creates a new switch instance and returns
func NewFswitch(dpi DpIntf, uplink string) (*Fswitch, error) {
	// create a switch instance
	fs := Fswitch{
		dpi:             dpi,
		uplink:          uplink,
		ports:           make(map[string]string),
		socks:           make(map[string]*raw.Conn),
		pcapHandles:     make(map[string]*pcap.Handle),
		localEndpoints:  make(map[string]*netproto.Endpoint),
		remoteEndpoints: make(map[string]*netproto.Endpoint),
		macaddrTable:    make(map[string]*FwdEntry),
		ipaddrTable:     make(map[string]*FwdEntry),
		lifTable:        make(map[string]*LifEntry),
	}

	// start listener on the uplink
	fs.runListener(uplink)

	return &fs, nil
}
