package dhcp

import (
	"encoding/binary"
	"fmt"
	"github.com/krolaw/dhcp4"
	"github.com/vishvananda/netlink"
	"log"
	"net"
	"time"
)

func DoDHCPConfig() error {
	dhcpIntf := "mock"

	intf, err := netlink.LinkByName(dhcpIntf)
	if err != nil {
		log.Printf("Failed to look up interface %v during static config. Err: %v", dhcpIntf, err)
		return err
	}

	pktSock, err := NewPacketSock(intf.Attrs().Index, 68, 67)
	if err != nil {
		log.Printf("Failed to create packet sock Err: %v", err)
		return err
	}

	client, err := New(HardwareAddr(intf.Attrs().HardwareAddr), Connection(pktSock))
	if err != nil {
		log.Printf("Failed to instantiate new DHCP Client. Err: %v", err)
		return err
	}

	dhcpIP, veniceIPs, leaseTime, ack, err := startDHCPClient(client)
	if err  != nil {
		log.Printf("Failed to start dhcp client. Err: %v", err)
		return err
	}
	// Set NMDIPConfig here and then call static assignments
	log.Printf("YIADDR: %s", dhcpIP.String())
	log.Printf("VENICE IPS: %v", veniceIPs)

	// Kick off a renewal process. TODO Ensure ack is re-entrant.
	expiry := time.After(leaseTime)
	go func(ack dhcp4.Packet) {
		for {
			select {
			case <- expiry:
				client.Renew(ack)
			}
		}
	}(ack)


	return nil
}

func startDHCPClient(client *Client)(dhcpIP net.IP, veniceIPs []net.IP, leaseDuration time.Duration, ack dhcp4.Packet, err error) {
	// Make these as constants
	batchSize := 4
	PensandoIdentifier := "Pensando"
	options := []dhcp4.Option{
		{
			Code:  dhcp4.OptionVendorClassIdentifier,
			Value: []byte(PensandoIdentifier),
		},
	}

	d := client.DiscoverPacket()
	for _, opt := range options {
		d.AddOption(opt.Code, opt.Value)
	}
	d.PadToMinSize()
	if err = client.SendPacket(d); err != nil {
		log.Printf("Failed to send discover packet. Err: %v", err)
		return
	}
	o, err := client.GetOffer(&d)
	if err != nil {
		log.Printf("Failed to get offer packet. Err: %v", err)
		return
	}

	r := client.RequestPacket(&o)
	for _, opt := range options {
		r.AddOption(opt.Code, opt.Value)
	}
	r.PadToMinSize()
	if err = client.SendPacket(r); err != nil {
		log.Printf("Failed to send request packet. Err: %v", err)
		return
	}

	ack, err = client.GetAcknowledgement(&r)
	if err != nil {
		log.Printf("Failed to get ack. Err: %v", err)
		return
	}


	log.Printf("FUCK YEAH!: %v", ack)
	log.Printf("Offer Packet Options: %v", ack.ParseOptions())
	log.Printf("Offer YIAddr: %v", ack.YIAddr().String())

	opts := ack.ParseOptions()
	vendorOptions := opts[dhcp4.OptionVendorSpecificInformation]
	log.Printf("VAL LEN: %v", len(vendorOptions))
	rawVeniceIPs := vendorOptions[2:]
	log.Printf("IPV4 Addresss MOD: %v", len(rawVeniceIPs) % 4 == 0)

	var parsedVendorInformation [][]byte
	for batchSize < len(rawVeniceIPs) {
		rawVeniceIPs, parsedVendorInformation = rawVeniceIPs[batchSize:], append(parsedVendorInformation, rawVeniceIPs[0:batchSize:batchSize])
	}
	parsedVendorInformation = append(parsedVendorInformation, rawVeniceIPs)
	for _, v := range parsedVendorInformation {
		veniceIPs = append(veniceIPs, net.IP{v[0], v[1], v[2], v[3]})
	}

	leaseDuration, err = time.ParseDuration(fmt.Sprintf("%ds", binary.BigEndian.Uint32(opts[dhcp4.OptionIPAddressLeaseTime])))
	log.Printf("Parsed Lease Time: %v", leaseDuration.Seconds())
	return ack.YIAddr(), veniceIPs, leaseDuration, ack, nil
}