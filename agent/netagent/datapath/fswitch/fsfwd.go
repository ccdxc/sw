// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package fswitch

import (
	"errors"
	"fmt"
	"net"

	"github.com/mdlayher/ethernet"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/utils/log"
)

// forwarding logic of Fswitch

// processFrame process an incoming frame
func (fs *Fswitch) processFrame(port string, frame *ethernet.Frame) error {
	switch frame.EtherType {
	case ethernet.EtherTypeARP:
		return fs.processARP(port, frame)
	case ethernet.EtherTypeIPv4:
		return fs.processIPv4(port, frame)
	default:
		log.Errorf("Unknown ethertype 0x%x", uint(frame.EtherType))
		return errors.New("Unknown Ethertype")
	}
}

// processARP processes an incoming ARP frame
func (fs *Fswitch) processARP(port string, frame *ethernet.Frame) error {
	// parse ARP message
	arp := new(ArpPacket)
	err := arp.UnmarshalBinary(frame.Payload)
	if err != nil {
		log.Errorf("Error parsing ARP message. Err: %v", err)
		return err
	}

	// handle based on ARP type
	switch arp.Operation {
	case OperationRequest:
		return fs.processARPRequest(port, arp, frame.VLAN)
	case OperationReply:
		// we can ignore ARP responses for now
		log.Infof("Received ARP responseon port %s vlan{%v}, {%+v}", port, frame.VLAN, arp)
	default:
		log.Errorf("Unknown ARP operation %d", arp.Operation)
		return errors.New("Unknown ARP operation")
	}

	return nil
}

// processARPRequest processes ARP request
func (fs *Fswitch) processARPRequest(port string, arp *ArpPacket, vlan *ethernet.VLAN) error {
	log.Infof("Switch Processing ARP req from port %s. {%+v}", port, arp)

	// learn the source
	err := fs.learnIPv4(port, vlan, arp.SenderIP, arp.SenderHardwareAddr)
	if err != nil {
		log.Errorf("Error learning source IP. Err: %v", err)
	}

	// lookup IP address in endpoint DB
	ipKey := fmt.Sprintf("%s|%s", DefaultVRF, arp.TargetIP.String())
	fwd, ok := fs.ipaddrTable[ipKey]
	if !ok {
		log.Errorf("Could not find ip table entry for %v", arp.TargetIP)
		return errors.New("Target IP not found")
	}

	// create ARP response
	resp, err := NewArpPacket(OperationReply, fwd.MacAddr, arp.TargetIP, arp.SenderHardwareAddr, arp.SenderIP)
	if err != nil {
		log.Errorf("Error building arp response. Err: %v", err)
		return err
	}

	// marshall the resp
	payload, err := resp.MarshalBinary()
	if err != nil {
		log.Errorf("Error marchaling ARP resp. Err: %v", err)
		return err
	}

	// create Ethernet packet
	frame := &ethernet.Frame{
		Destination: arp.SenderHardwareAddr,
		Source:      fwd.MacAddr,
		VLAN:        vlan,
		EtherType:   ethernet.EtherTypeARP,
		Payload:     payload,
	}

	log.Infof("Switch Sending ARP resp on port %v {%+v} {%+v} ", port, resp, frame)

	// send response
	return fs.SendFrame(port, frame)
}

// processIPv4 forwards an IPv4 packet.
// Since fswitch is in L2 switching mode, we just forward based on dest mac
func (fs *Fswitch) processIPv4(port string, frame *ethernet.Frame) error {
	// check if we have a vlan id
	var vlanID uint16
	if frame.VLAN != nil {
		vlanID = frame.VLAN.ID
	}

	// find the incoming lif
	inLif, err := fs.findLif(port, uint32(vlanID))
	if err != nil {
		log.Errorf("Can not find the incoming LIF for port: %s, vlan: %d. Err: %v", port, vlanID, err)
		return err
	}

	// find the forwarding entry in mac table
	macKey := fmt.Sprintf("%s|%s", inLif.Network, frame.Destination)
	fwd, ok := fs.macaddrTable[macKey]
	if !ok {
		log.Warnf("Received packet for unknown dest %s", macKey)
		return errors.New("Unknown mac addr")
	}

	// dont forward from same lif-to-lif or uplink-to-uplink
	if inLif == fwd.Lif || (port == fs.uplink && fwd.Lif.Port == fs.uplink) {
		return nil
	}

	// override the vlan
	frame.VLAN = nil
	if fwd.Lif.Vlan != 0 {
		frame.VLAN = &ethernet.VLAN{ID: uint16(fwd.Lif.Vlan)}
	}

	log.Infof("Forwarding mac %s from port,vlan %s/%d to port,vlan %s,%d", macKey, port, vlanID, fwd.Lif.Port, fwd.Lif.Vlan)

	// forward to dest port
	return fs.SendFrame(fwd.Lif.Port, frame)
}

// learnIPv4 learns <IP, Mac> if IP address is unknown
func (fs *Fswitch) learnIPv4(port string, vlan *ethernet.VLAN, ipAddr net.IP, macAddr net.HardwareAddr) error {
	// dont learn on uplink interfaces
	if port == fs.uplink {
		return nil
	}
	// check if we have a vlan id
	var vlanID uint16
	if vlan != nil {
		vlanID = vlan.ID
	}

	// check if we know the IP
	ipKey := fmt.Sprintf("%s|%s", DefaultVRF, ipAddr.String())
	_, ok := fs.ipaddrTable[ipKey]
	if ok {
		// we know the IP address, we are done
		return nil
	}

	// build endpoint info
	ep := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:   "",
			Tenant: "default", // FIXME: where should we get the tenant from?
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "",
			NetworkName:  "",
		},
		Status: netproto.EndpointStatus{
			IPv4Address: ipAddr.String(),
			MacAddress:  macAddr.String(),
			UsegVlan:    uint32(vlanID),
		},
	}

	// trigger an IP learn event
	return fs.dpi.EndpointLearnNotif(&ep)
}
