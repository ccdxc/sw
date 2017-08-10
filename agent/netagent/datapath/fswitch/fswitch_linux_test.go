// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package fswitch

import (
	"errors"
	"net"
	"os"
	"testing"
	"time"

	"github.com/mdlayher/ethernet"
	"github.com/mdlayher/raw"
	"github.com/pensando/sw/agent/netagent/netutils"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/utils/log"
	. "github.com/pensando/sw/utils/testutils"
)

// dummy struct for the test
type dpi struct {
	// empty
}

func (d *dpi) EndpointLearnNotif(ep *netproto.Endpoint) error {
	return nil
}

func TestFswitch(t *testing.T) {
	if os.Getuid() != 0 {
		t.Skip("Must run the tests as root. creating veth pairs needs this permission")
	}

	// create fswitch
	fs, err := NewFswitch(&dpi{}, "datapath")
	AssertOk(t, err, "error creating the fswitch")

	// create a veth pair
	err = netutils.CreateVethPair("vport1", "vport2")
	AssertOk(t, err, "Error creating veth pairs")
	defer netutils.DeleteVethPair("vport1", "vport2")

	// bring up both interfaces
	err = netutils.SetIntfUp("vport1")
	AssertOk(t, err, "Error bringing up veth interface")
	err = netutils.SetIntfUp("vport2")
	AssertOk(t, err, "Error bringing up veth interface")

	time.Sleep(time.Millisecond)

	// add both veths to the switch
	err = fs.AddPort("vport1")
	AssertOk(t, err, "Error adding veth to fswitch")
	err = fs.AddPort("vport2")
	AssertOk(t, err, "Error adding veth to fswitch")

	time.Sleep(time.Millisecond * 10)

	// frame to send
	frame := &ethernet.Frame{
		// Broadcast frame to all machines on same network segment.
		Destination: ethernet.Broadcast,
		// Identify our machine as the sender.
		Source: net.HardwareAddr{0xde, 0xad, 0xbe, 0xef, 0xde, 0xad},
		// Identify frame with an unused EtherType.
		EtherType: ethernet.EtherTypeIPv4,
		// Send a simple message.
		Payload: []byte("hello world"),
	}

	// send a frame
	err = fs.SendFrame("vport1", frame)
	AssertOk(t, err, "Error sending the frame")

	// create and send an ARP packet
	arp, err := NewArpPacket(OperationRequest, frame.Source, net.ParseIP("10.1.1.1"), ethernet.Broadcast, net.ParseIP("20.1.1.1"))
	AssertOk(t, err, "Error creating arp packet")
	frame.Payload, err = arp.MarshalBinary()
	frame.EtherType = ethernet.EtherTypeARP
	AssertOk(t, err, "Error marshaling arp packat")
	err = fs.SendFrame("vport1", frame)
	AssertOk(t, err, "Error sending the frame")

	time.Sleep(time.Millisecond)
}

type fsetup struct {
	switches  [2]*Fswitch
	vethPairs map[string]string
	ipSocks   map[string]*raw.Conn // IP listeners
	arpSocks  map[string]*raw.Conn // ARP listeners
}

// createEndpoint creates an endpoint
func createEndpoint(ipaddr, macAddr string, usegVlan uint32) *netproto.Endpoint {
	return &netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   ipaddr,
		},
		Status: netproto.EndpointStatus{
			IPv4Address: ipaddr,
			MacAddress:  macAddr,
			UsegVlan:    usegVlan,
		},
	}
}

func createSocket(port string) (*raw.Conn, *raw.Conn, error) {
	// Select the interface to receive traffic.
	ifi, err := net.InterfaceByName(port)
	if err != nil {
		log.Errorf("failed to open interface: %v", err)
		return nil, nil, err
	}

	// Open a raw socket for EtherType
	arpSock, err := raw.ListenPacket(ifi, 0x0806)
	if err != nil {
		log.Errorf("failed to listen: %v", err)
		return nil, nil, err
	}

	ipSock, err := raw.ListenPacket(ifi, 0x0800)
	if err != nil {
		log.Errorf("failed to listen: %v", err)
		return nil, nil, err
	}

	return ipSock, arpSock, nil
}

func createVethPair(p1, p2 string) error {
	err := netutils.CreateVethPair(p1, p2)
	if err != nil {
		return err
	}
	err = netutils.SetIntfUp(p1)
	if err != nil {
		return err
	}
	err = netutils.SetIntfUp(p2)
	if err != nil {
		return err
	}

	return nil
}

func createSetup(t *testing.T) *fsetup {
	// create setup instance
	setup := fsetup{
		vethPairs: make(map[string]string),
		ipSocks:   make(map[string]*raw.Conn),
		arpSocks:  make(map[string]*raw.Conn),
	}
	// create uplink pairs
	err := createVethPair("uplink1", "uplink2")
	AssertOk(t, err, "Error creating uplink veth pairs")
	setup.vethPairs["uplink1"] = "uplink2"
	setup.vethPairs["uplink2"] = "uplink1"

	// create switches
	fs1, err := NewFswitch(&setup, "uplink1")
	AssertOk(t, err, "Error creating the switch1")
	fs2, err := NewFswitch(&setup, "uplink2")
	AssertOk(t, err, "Error creating the switch2")
	setup.switches[0] = fs1
	setup.switches[1] = fs2

	// add two ports to first switch
	err = createVethPair("svport11", "cvport11")
	AssertOk(t, err, "Error creating veth pairs")
	err = createVethPair("svport12", "cvport12")
	AssertOk(t, err, "Error creating veth pairs")
	err = fs1.AddPort("svport11")
	AssertOk(t, err, "Error adding port to switch")
	err = fs1.AddPort("svport12")
	AssertOk(t, err, "Error adding port to switch")

	// add two ports to second switch
	err = createVethPair("svport21", "cvport21")
	AssertOk(t, err, "Error creating veth pairs")
	err = createVethPair("svport22", "cvport22")
	AssertOk(t, err, "Error creating veth pairs")
	err = fs2.AddPort("svport21")
	AssertOk(t, err, "Error adding port to switch")
	err = fs2.AddPort("svport22")
	AssertOk(t, err, "Error adding port to switch")

	// endpoints
	ep11 := createEndpoint("11.11.11.11/24", "11:11:11:11:11:11", 0)
	ep12 := createEndpoint("12.12.12.12/24", "12:12:12:12:12:12", 0)
	ep21 := createEndpoint("21.21.21.21/24", "21:21:21:21:21:21", 0)
	ep22 := createEndpoint("22.22.22.22/24", "22:22:22:22:22:22", 0)

	// add endpoints to first switch
	err = fs1.AddLocalEndpoint("svport11", ep11)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs1.AddLocalEndpoint("svport12", ep12)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs1.AddRemoteEndpoint(ep21)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs1.AddRemoteEndpoint(ep22)
	AssertOk(t, err, "Error adding endpoint to switch")

	// add endpoints to second switch
	err = fs2.AddLocalEndpoint("svport21", ep21)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs2.AddLocalEndpoint("svport22", ep22)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs2.AddRemoteEndpoint(ep11)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs2.AddRemoteEndpoint(ep12)
	AssertOk(t, err, "Error adding endpoint to switch")

	// open sockets
	setup.ipSocks["cvport11"], setup.arpSocks["cvport11"], err = createSocket("cvport11")
	AssertOk(t, err, "Error creating sockets")
	setup.ipSocks["cvport12"], setup.arpSocks["cvport12"], err = createSocket("cvport12")
	AssertOk(t, err, "Error creating sockets")
	setup.ipSocks["cvport21"], setup.arpSocks["cvport21"], err = createSocket("cvport21")
	AssertOk(t, err, "Error creating sockets")
	setup.ipSocks["cvport22"], setup.arpSocks["cvport22"], err = createSocket("cvport22")
	AssertOk(t, err, "Error creating sockets")

	return &setup
}

func (st *fsetup) EndpointLearnNotif(ep *netproto.Endpoint) error {
	return nil
}

func (st *fsetup) sendMessage(port string, frame *ethernet.Frame) error {
	// find the connection
	conn, ok := st.arpSocks[port]
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

	// send the frame
	addr := &raw.Addr{HardwareAddr: frame.Destination}
	_, err = conn.WriteTo(b, addr)
	if err != nil {
		log.Errorf("Error sending the frame {%+v}. Err: %v", frame, err)
		return err
	}

	return nil
}

func (st *fsetup) recvArpFrame(port string) (*ethernet.Frame, *ArpPacket, error) {
	b := make([]byte, 1500)
	var f ethernet.Frame
	var arp ArpPacket

	// find the connection
	conn, ok := st.arpSocks[port]
	if !ok {
		log.Errorf("Could not find the connection for port %s", port)
		return nil, nil, errors.New("Connection not found")
	}

	n, addr, err := conn.ReadFrom(b)
	if err != nil {
		log.Errorf("failed to receive message: %v", err)
		return nil, nil, err
	}

	// Unpack Ethernet frame into Go representation.
	err = (&f).UnmarshalBinary(b[:n])
	if err != nil {
		log.Errorf("failed to unmarshal ethernet frame: %v", err)
		return nil, nil, err
	}

	// Display source of message and message itself.
	log.Debugf("Received Ethernet frame on %s [%s->%s] 0x%x: %v", port, addr.String(), f.Destination.String(), uint(f.EtherType), f.Payload)

	// process ARP message
	err = (&arp).UnmarshalBinary(f.Payload)
	if err != nil {
		log.Errorf("failed to unmarshal arp message: %v", err)
		return nil, nil, err
	}

	return &f, &arp, nil
}

func (st *fsetup) recvIPFrame(port string) (*ethernet.Frame, error) {
	b := make([]byte, 1500)
	var f ethernet.Frame

	// find the connection
	conn, ok := st.ipSocks[port]
	if !ok {
		log.Errorf("Could not find the connection for port %s", port)
		return nil, errors.New("Connection not found")
	}

	n, addr, err := conn.ReadFrom(b)
	if err != nil {
		log.Errorf("failed to receive message: %v", err)
		return nil, err
	}

	// Unpack Ethernet frame into Go representation.
	err = (&f).UnmarshalBinary(b[:n])
	if err != nil {
		log.Errorf("failed to unmarshal ethernet frame: %v", err)
		return nil, err
	}

	// Display source of message and message itself.
	log.Debugf("Received Ethernet frame on %s [%s->%s] 0x%x: %v", port, addr.String(), f.Destination.String(), uint(f.EtherType), f.Payload)

	return &f, nil
}

func (st *fsetup) delete(t *testing.T) {
	//  init variables
	fs1 := st.switches[0]
	fs2 := st.switches[1]
	ep11 := createEndpoint("11.11.11.11/24", "11:11:11:11:11:11", 0)
	ep12 := createEndpoint("12.12.12.12/24", "12:12:12:12:12:12", 0)
	ep21 := createEndpoint("21.21.21.21/24", "21:21:21:21:21:21", 0)
	ep22 := createEndpoint("22.22.22.22/24", "22:22:22:22:22:22", 0)

	// add endpoints to first switch
	err := fs1.DelLocalEndpoint("svport11", ep11)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs1.DelLocalEndpoint("svport12", ep12)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs1.DelRemoteEndpoint(ep21)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs1.DelRemoteEndpoint(ep22)
	AssertOk(t, err, "Error adding endpoint to switch")

	// add endpoints to second switch
	err = fs2.DelLocalEndpoint("svport21", ep21)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs2.DelLocalEndpoint("svport22", ep22)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs2.DelRemoteEndpoint(ep11)
	AssertOk(t, err, "Error adding endpoint to switch")
	err = fs2.DelRemoteEndpoint(ep12)
	AssertOk(t, err, "Error adding endpoint to switch")

	// delete port from the switch
	err = fs1.DelPort("svport11")
	AssertOk(t, err, "Error deleting ports from switch")
	err = fs1.DelPort("svport12")
	AssertOk(t, err, "Error deleting ports from switch")
	err = fs2.DelPort("svport21")
	AssertOk(t, err, "Error deleting ports from switch")
	err = fs2.DelPort("svport22")
	AssertOk(t, err, "Error deleting ports from switch")
}

func (st *fsetup) cleanup(t *testing.T) {

	// delete veth pairs
	err := netutils.DeleteVethPair("uplink1", "uplink2")
	AssertOk(t, err, "Error deleting uplink veth pairs")
	err = netutils.DeleteVethPair("svport11", "cvport11")
	AssertOk(t, err, "Error deleting veth pairs")
	err = netutils.DeleteVethPair("svport12", "cvport12")
	AssertOk(t, err, "Error deleting veth pairs")
	err = netutils.DeleteVethPair("svport21", "cvport21")
	AssertOk(t, err, "Error deleting veth pairs")
	err = netutils.DeleteVethPair("svport22", "cvport22")
	AssertOk(t, err, "Error deleting veth pairs")
}

func sendArpReq(t *testing.T, st *fsetup, srcMac, srcIP, destIP, port string) {
	// build the ethernet frame
	sm, _ := net.ParseMAC(srcMac)
	arpReq, err := NewArpPacket(OperationRequest, sm, net.ParseIP(srcIP), ethernet.Broadcast, net.ParseIP(destIP))
	AssertOk(t, err, "Error creating arp request")
	payload, err := arpReq.MarshalBinary()
	AssertOk(t, err, "Error marchaling arp req")
	frame := ethernet.Frame{
		Destination: ethernet.Broadcast,
		Source:      sm,
		EtherType:   ethernet.EtherTypeARP,
		Payload:     payload,
	}

	// send the message
	err = st.sendMessage(port, &frame)
	AssertOk(t, err, "Error sending the message")
}

func sendEthFrame(t *testing.T, st *fsetup, srcMac, dstMac, port string, ethertype ethernet.EtherType, len int) {
	// build the ethernet frame
	sm, _ := net.ParseMAC(srcMac)
	dm, _ := net.ParseMAC(dstMac)
	payload := make([]byte, len)
	frame := ethernet.Frame{
		Destination: dm,
		Source:      sm,
		EtherType:   ethertype,
		Payload:     payload,
	}

	// send the message
	err := st.sendMessage(port, &frame)
	AssertOk(t, err, "Error sending the message")
}

func TestFswitchFwd(t *testing.T) {
	if os.Getuid() != 0 {
		t.Skip("Must run the tests as root. creating veth pairs needs this permission")
	}

	// create the setup
	st := createSetup(t)
	// fs1 := st.switches[0]
	// fs2 := st.switches[1]
	defer st.cleanup(t)
	time.Sleep(time.Millisecond * 10)

	// send an ARP request from cvport11
	sendArpReq(t, st, "11:11:11:11:11:11", "11.11.11.11", "12.12.12.12", "cvport11")
	time.Sleep(time.Millisecond)
	respFrame, arpResp, err := st.recvArpFrame("cvport11")
	AssertOk(t, err, "Error receiving the message")
	Assert(t, (respFrame.EtherType == ethernet.EtherTypeARP), "Invalid message type", respFrame)
	Assert(t, (arpResp.SenderHardwareAddr.String() == "12:12:12:12:12:12"), "Invalid arp resp", arpResp)

	// send arp request for remote endpoint
	sendArpReq(t, st, "12:12:12:12:12:12", "12.12.12.12", "21.21.21.21", "cvport12")
	time.Sleep(time.Millisecond)
	respFrame, _, err = st.recvArpFrame("cvport12")
	AssertOk(t, err, "Error receiving the message")
	Assert(t, (respFrame.EtherType == ethernet.EtherTypeARP), "Invalid message type", respFrame)

	// send an ip packet from one port and receive it on other
	sendEthFrame(t, st, "11:11:11:11:11:11", "12:12:12:12:12:12", "cvport11", ethernet.EtherTypeIPv4, 100)
	time.Sleep(time.Millisecond * 10)
	respFrame, err = st.recvIPFrame("cvport12")
	AssertOk(t, err, "Error receiving the message")
	Assert(t, (respFrame.EtherType == ethernet.EtherTypeIPv4), "Invalid message type", respFrame)

	// send ip frame on one switch and receive it on another
	sendEthFrame(t, st, "11:11:11:11:11:11", "21:21:21:21:21:21", "cvport11", ethernet.EtherTypeIPv4, 100)
	time.Sleep(time.Millisecond * 10)
	respFrame, err = st.recvIPFrame("cvport21")
	AssertOk(t, err, "Error receiving the message")
	Assert(t, (respFrame.EtherType == ethernet.EtherTypeIPv4), "Invalid message type", respFrame)

	// tets error conditions
	sendArpReq(t, st, "11:11:11:11:11:11", "11.11.11.11", "15.15.15.15", "cvport11")
	time.Sleep(time.Millisecond * 10)
	sendEthFrame(t, st, "11:11:11:11:11:11", "15:15:15:15:15:15", "cvport11", ethernet.EtherTypeIPv4, 100)
	time.Sleep(time.Millisecond * 10)

	// done
	time.Sleep(time.Millisecond * 10)
	st.delete(t)
}
