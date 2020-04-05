package main

import (
	"math/rand"
	"net"
	"time"

	"github.com/pensando/netlink"

	dhcp "github.com/krolaw/dhcp4"
	"github.com/krolaw/dhcp4/conn"

	"github.com/pensando/sw/venice/utils/log"
)

type dhcpSrv struct {
	conn net.PacketConn
}

const (
	naplesInterface    = "oob_mnic0"
	dhcpServerIntf     = "srv-dhcp"
	ipAddrStart        = "172.16.10.2"
	testSubnet         = "172.16.10.0/28"
	pensandoIdentifier = "Pensando"
	leaseDuration      = time.Second * 10
	noDHCP             = iota
	configureNoVendorAtrrs
	configureEmptyVendorAtrs
	configureValidVendorAttrs
	configureMalformedVendorAttrs
	configureMalformedVendorAttrsMismatchOption43And60
)

var (
	// PensandoDHCPRequestOption constructs dhcp vendor option for pensando
	pensandoDHCPRequestOption = dhcp.Option{
		Code:  dhcp.OptionVendorClassIdentifier,
		Value: []byte(pensandoIdentifier),
	}

	veniceIPs = "42.42.42.42,84.84.84.84"
)

type lease struct {
	nic    string    // Client's CHAddr
	expiry time.Time // When the lease expires
}

type dHCPHandler struct {
	ip            net.IP        // Server IP to use
	options       dhcp.Options  // Options to send to DHCP Clients
	start         net.IP        // Start of IP range to distribute
	leaseRange    int           // Number of IPs to distribute (starting from start)
	leaseDuration time.Duration // Lease period
	leases        map[int]lease // Map to keep track of leases
}

func main() {
	var d dhcpSrv
	if err := d.setup(configureNoVendorAtrrs); err != nil {
		panic(err)
	}
	select {}

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
			dhcp.OptionVendorClassIdentifier: []byte(pensandoIdentifier),
		}
	case configureMalformedVendorAttrsMismatchOption43And60:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorSpecificInformation: []byte(pensandoIdentifier),
		}
	case configureMalformedVendorAttrs:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     []byte(pensandoIdentifier),
			dhcp.OptionVendorSpecificInformation: []byte("ř"),
		}
	case configureValidVendorAttrs:
		opts = dhcp.Options{
			dhcp.OptionSubnetMask:                []byte{255, 255, 255, 240},
			dhcp.OptionRouter:                    []byte(serverIP), // Presuming Server is also your router
			dhcp.OptionDomainNameServer:          []byte(serverIP), // Presuming Server is also your DNS server
			dhcp.OptionVendorClassIdentifier:     pensandoDHCPRequestOption.Value,
			dhcp.OptionVendorSpecificInformation: []byte(veniceIPs),
		}
	}

	handler := &dHCPHandler{
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

func (h *dHCPHandler) ServeDHCP(p dhcp.Packet, msgType dhcp.MessageType, options dhcp.Options) (d dhcp.Packet) {
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

func (h *dHCPHandler) freeLease() int {
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

func (d *dhcpSrv) setup(configureVendorAttrs int) error {
	clientMAC, _ := net.ParseMAC("42:42:42:42:42:42")

	dhcpClientMock := &netlink.Veth{
		LinkAttrs: netlink.LinkAttrs{
			Name:         naplesInterface,
			TxQLen:       1000,
			HardwareAddr: clientMAC,
		},
		PeerName: dhcpServerIntf,
	}

	// Create the veth pair
	if err := netlink.LinkAdd(dhcpClientMock); err != nil {
		return err
	}
	if err := netlink.LinkSetARPOn(dhcpClientMock); err != nil {
		return err
	}

	netlink.LinkSetUp(dhcpClientMock)
	// Assign IP Address statically for the server
	srvIntf, err := netlink.LinkByName(dhcpServerIntf)
	if err != nil {
		log.Errorf("Failed to find the server interface")
		return err
	}
	addr, _ := netlink.ParseAddr("172.16.10.1/28")

	if err := netlink.AddrAdd(srvIntf, addr); err != nil {
		log.Errorf("Failed to assign ip address %v to interface dhcpmock. Err: %v", addr.IP.String(), err)
		return err
	}

	if err := netlink.LinkSetUp(srvIntf); err != nil {
		log.Errorf("Failed to bring up the interface. Err: %v", err)
		return err
	}

	if configureVendorAttrs != noDHCP {
		go d.startDHCPServer(configureVendorAttrs)
	}
	return nil
}
