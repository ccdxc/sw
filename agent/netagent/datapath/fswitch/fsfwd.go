// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package fswitch

import (
	"errors"
	"fmt"

	log "github.com/Sirupsen/logrus"
	"github.com/mdlayher/ethernet"
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
		log.Infof("Received ARP response {%+v}", arp)
	default:
		log.Errorf("Unknown ARP operation %d", arp.Operation)
		return errors.New("Unknown ARP operation")
	}

	return nil
}

// processARPRequest processes ARP request
func (fs *Fswitch) processARPRequest(port string, arp *ArpPacket, vlan *ethernet.VLAN) error {
	log.Infof("Switch Processing ARP req from port %s. {%+v}", port, arp)

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

	log.Infof("Switch Sending ARP resp on port {%+v} {%+v}", port, resp, frame)

	// send response
	return fs.SendFrame(port, frame)
}

// processIPv4 forwards an IPv4 packet.
// Since fswitch is in L2 switching mode, we just forward based on dest mac
func (fs *Fswitch) processIPv4(port string, frame *ethernet.Frame) error {
	// check if we have a vlna id
	var vlanID uint16
	if frame.VLAN != nil {
		vlanID = frame.VLAN.ID
	}

	// find the forwarding entry in mac table
	macKey := fmt.Sprintf("%d|%s", vlanID, frame.Destination)
	fwd, ok := fs.macaddrTable[macKey]
	if !ok {
		log.Warnf("Received packet for unknown dest %s", macKey)
		return errors.New("Unknown mac addr")
	}

	log.Infof("Forwarding mac %s from port %s to port %s", macKey, port, fwd.Port)

	// forward to dest port
	return fs.SendFrame(fwd.Port, frame)
}
