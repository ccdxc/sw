// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package fswitch

import (
	"errors"
	"fmt"
	"net"
	"sync"

	log "github.com/Sirupsen/logrus"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"

	"github.com/mdlayher/ethernet"
	"github.com/mdlayher/raw"
)

// constants
const (
	DefaultVlan = 0
	DefaultVRF  = "default"
)

// FwdEntry forwarding lookup entry
type FwdEntry struct {
	MacAddr  net.HardwareAddr   // Mac addr
	IPv4Addr net.IP             // IP addr
	Vlan     uint32             // vlan
	Vrf      string             // VRF
	Port     string             // port name
	Endpoint *netproto.Endpoint // endpoint
}

// Fswitch is a switch instance
type Fswitch struct {
	sync.Mutex                                    // lock the switch for map writes
	uplink          string                        // uplink interface
	ports           map[string]string             // workload facing ports
	ipSocks         map[string]*raw.Conn          // IP listeners
	arpSocks        map[string]*raw.Conn          // ARP listeners
	localEndpoints  map[string]*netproto.Endpoint // local endpoints
	remoteEndpoints map[string]*netproto.Endpoint // remote endpoints
	macaddrTable    map[string]*FwdEntry          // forwarding table indexed by mac addr, vlan
	ipaddrTable     map[string]*FwdEntry          // forwarding table indexed by ip addr, vrf
}

// main listener loop for a socket
func (fs *Fswitch) listnerLoop(conn *raw.Conn, port string, mtu int) {
	// Accept frames up to interface's MTU in size.
	b := make([]byte, mtu)
	var f ethernet.Frame

	defer conn.Close()

	for {
		n, addr, err := conn.ReadFrom(b)
		if err != nil {
			log.Errorf("failed to receive message: %v", err)
			return
		}

		// Unpack Ethernet frame into Go representation.
		err = (&f).UnmarshalBinary(b[:n])
		if err != nil {
			log.Fatalf("failed to unmarshal ethernet frame: %v", err)
		}

		// Display source of message and message itself.
		log.Debugf("Switch Received Ethernet frame [%s->%s] 0x%x: %v", addr.String(), f.Destination.String(), uint(f.EtherType), f.Payload)

		// process the frame
		err = fs.processFrame(port, &f)
		if err != nil {
			log.Errorf("Error processing frame. Err: %v", err)
		}
	}
}

// runListener loops forever and receives messages
func (fs *Fswitch) runListener(port string) error {
	// Select the interface to receive traffic.
	ifi, err := net.InterfaceByName(port)
	if err != nil {
		log.Errorf("failed to open interface: %v", err)
		return err
	}

	// Open a raw socket for EtherType
	arpSock, err := raw.ListenPacket(ifi, 0x0806)
	if err != nil {
		log.Errorf("failed to listen: %v", err)
		return err
	}

	ipSock, err := raw.ListenPacket(ifi, 0x0800)
	if err != nil {
		log.Errorf("failed to listen: %v", err)
		return err
	}

	// save it in db
	fs.ipSocks[port] = ipSock
	fs.arpSocks[port] = arpSock

	// Keep reading frames.
	go fs.listnerLoop(ipSock, port, ifi.MTU)
	go fs.listnerLoop(arpSock, port, ifi.MTU)

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
	conn, ok := fs.arpSocks[port]
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
func (fs *Fswitch) addFwdEntry(port string, ep *netproto.Endpoint) error {
	// build forwarding entry
	mac, _ := net.ParseMAC(ep.Status.MacAddress)
	ip, _, _ := net.ParseCIDR(ep.Status.IPv4Address)
	fwd := FwdEntry{
		MacAddr:  mac,
		IPv4Addr: ip,
		Vlan:     ep.Status.UsegVlan,
		Vrf:      DefaultVRF,
		Port:     port,
		Endpoint: ep,
	}

	// add to mac table
	macKey := fmt.Sprintf("%d|%s", ep.Status.UsegVlan, ep.Status.MacAddress)
	fs.macaddrTable[macKey] = &fwd

	// add to ip addr table
	ipKey := fmt.Sprintf("%s|%s", fwd.Vrf, fwd.IPv4Addr.String())
	fs.ipaddrTable[ipKey] = &fwd

	log.Infof("Added fwd entry: {%+v}", fwd)

	return nil
}

// delFwdEntry deletes forwarding entry
func (fs *Fswitch) delFwdEntry(port string, ep *netproto.Endpoint) error {
	// delete from mac table
	macKey := fmt.Sprintf("%d|%s", ep.Status.UsegVlan, ep.Status.MacAddress)
	delete(fs.macaddrTable, macKey)

	// delete from ip addr table
	ip, _, _ := net.ParseCIDR(ep.Status.IPv4Address)
	ipKey := fmt.Sprintf("%s|%s", DefaultVRF, ip.String())
	delete(fs.ipaddrTable, ipKey)

	return nil
}

// AddLocalEndpoint adds local endpoint
func (fs *Fswitch) AddLocalEndpoint(port string, ep *netproto.Endpoint) error {
	// add a forwarding entry
	err := fs.addFwdEntry(port, ep)
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
	err := fs.delFwdEntry(port, ep)
	if err != nil {
		log.Errorf("Error adding fwd entry. Err: %v", err)
		return err
	}

	// remove from local epdb
	delete(fs.localEndpoints, ep.Name)

	return nil
}

// AddRemoteEndpoint adds remote endpoint
func (fs *Fswitch) AddRemoteEndpoint(ep *netproto.Endpoint) error {
	// add a forwarding entry
	err := fs.addFwdEntry(fs.uplink, ep)
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
	err := fs.delFwdEntry(fs.uplink, ep)
	if err != nil {
		log.Errorf("Error adding fwd entry. Err: %v", err)
		return err
	}

	// remove from remote epdb
	delete(fs.remoteEndpoints, ep.Name)

	return nil
}

// NewFswitch creates a new switch instance and returns
func NewFswitch(uplink string) (*Fswitch, error) {
	// create a switch instance
	fs := Fswitch{
		uplink:          uplink,
		ports:           make(map[string]string),
		ipSocks:         make(map[string]*raw.Conn),
		arpSocks:        make(map[string]*raw.Conn),
		localEndpoints:  make(map[string]*netproto.Endpoint),
		remoteEndpoints: make(map[string]*netproto.Endpoint),
		macaddrTable:    make(map[string]*FwdEntry),
		ipaddrTable:     make(map[string]*FwdEntry),
	}

	// start listener on the uplink
	fs.runListener(uplink)

	return &fs, nil
}
