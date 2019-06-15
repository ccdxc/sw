// +build linux

package ipif

import (
	"encoding/binary"
	"fmt"
	"net"
	"strings"
	"time"

	"github.com/pensando/sw/api/generated/cluster"

	"github.com/amahagaonkar/dhcp-client"
	"github.com/krolaw/dhcp4"
	"github.com/vishvananda/netlink"

	"github.com/pensando/sw/nic/agent/nmd/api"
	"github.com/pensando/sw/venice/utils/log"
)

// NewIPClient returns a new IPClient instance
func NewIPClient(nmd api.NmdAPI, intf string) (*IPClient, error) {
	dhcpState := DHCPState{
		nmd:       nmd,
		VeniceIPs: make(map[string]bool),
	}
	link, err := netlink.LinkByName(intf)
	if err != nil {
		log.Errorf("Failed to look up interface %v during static config. Err: %v", intf, err)
		return nil, fmt.Errorf("failed to look up interface %v during static config. Err: %v", intf, err)
	}
	if err := netlink.LinkSetUp(link); err != nil {
		log.Errorf("Failed to set the interface up. Intf: %v | Err: %v", link.Attrs().Name, err)
		return nil, fmt.Errorf("failed to set the interface up. Intf: %v | Err: %v", link.Attrs().Name, err)
	}

	ipClient := IPClient{
		nmd:       nmd,
		intf:      link,
		dhcpState: &dhcpState,
	}

	return &ipClient, nil
}

// DoStaticConfig performs static IPAddress/Default GW configuration. It returns the assigned IP address inline
func (c *IPClient) DoStaticConfig() (string, error) {
	ipConfig := c.nmd.GetIPConfig()
	addr, _ := netlink.ParseAddr(ipConfig.IPAddress)

	if addr == nil {
		log.Errorf("Could not parse IP Address from %v", ipConfig.IPAddress)
		return "", fmt.Errorf("could not parse IP Address from %v", ipConfig.IPAddress)
	}

	// Assign IP Address statically
	if err := netlink.AddrReplace(c.intf, addr); err != nil {
		log.Errorf("Failed to assign ip address %v to interface %v. Err: %v", ipConfig.IPAddress, c.intf, err)
		return "", err
	}
	//// Assign default gw TODO: Verify if the route added by AddrAdd is good enough
	//if len(ipConfig.DefaultGW) != 0 {
	//	defaultRoute := &netlink.Route{
	//		LinkIndex: intf.Attrs().Index,
	//		Gw:        net.ParseIP(ipConfig.DefaultGW),
	//	}
	//	err := netlink.RouteAdd(defaultRoute)
	//	if err != nil {
	//		log.Errorf("Failed to add default gw %v for the interface %v. Err: %v", ipConfig.DefaultGW, c.intf, err)
	//		return "", err
	//	}
	//}

	return addr.String(), nil
}

// DoDHCPConfig performs dhcp on the management interface and obtains venice co-ordinates
func (c *IPClient) DoDHCPConfig() error {
	pktSock, err := dhcp.NewPacketSock(c.intf.Attrs().Index, 68, 67)
	if err != nil {
		log.Errorf("Failed to create packet sock Err: %v", err)
		return err
	}

	client, err := dhcp.New(dhcp.HardwareAddr(c.intf.Attrs().HardwareAddr), dhcp.Connection(pktSock))
	if err != nil {
		log.Errorf("Failed to instantiate new DHCP Client. Err: %v", err)
		return err
	}
	// Start the control loop and keep polling for err
	c.dhcpState.Client = client

	if err := c.startDHCP(client); err != nil {
		log.Errorf("Failed to start dhcp client. Err: %v", err)
		return err
	}

	return nil
}

func (c *IPClient) startDHCP(client *dhcp.Client) (err error) {
	disc := client.DiscoverPacket()
	disc.AddOption(PensandoDHCPRequestOption.Code, PensandoDHCPRequestOption.Value)

	disc.PadToMinSize()
	if err = client.SendPacket(disc); err != nil {
		log.Errorf("Failed to send discover packet. Err: %v", err)
		return
	}

	log.Info("Discover Sent")
	offer, err := client.GetOffer(&disc)
	if err != nil {
		log.Errorf("Failed to get offer packet. Err: %v", err)
		c.dhcpState.CurState = dhcpTimedout.String()
		return
	}

	req := client.RequestPacket(&offer)
	req.AddOption(PensandoDHCPRequestOption.Code, PensandoDHCPRequestOption.Value)

	req.PadToMinSize()
	if err = client.SendPacket(req); err != nil {
		c.dhcpState.CurState = dhcpSent.String()
		log.Errorf("Failed to send request packet. Err: %v", err)
		return
	}

	//c.dhcpState.SetState(dhcpSent)

	ack, err := client.GetAcknowledgement(&req)
	log.Infof("Get Ack Returned: %v", ack)
	if err != nil {
		log.Errorf("Failed to get ack. Err: %v", err)
		//c.dhcpState.SetState(dhcpTimedout)
		return
	}

	return c.dhcpState.updateDHCPState(ack, c.intf)

}

func (d *DHCPState) updateDHCPState(ack dhcp4.Packet, mgmtLink netlink.Link) (err error) {
	d.Lock()
	defer d.Unlock()

	// TODO move error handling out
	if ack == nil {
		return fmt.Errorf("failed to get any dhcp server acks")
	}

	d.AckPacket = ack
	d.AckPacket.PadToMinSize()

	log.Infof("Offer YIAddr: %v", d.AckPacket.YIAddr().String())
	log.Infof("Offer GIADDR: %v", d.AckPacket.YIAddr().String())

	d.GwIP = d.AckPacket.GIAddr()

	opts := d.AckPacket.ParseOptions()
	d.IPNet = net.IPNet{
		IP:   d.AckPacket.YIAddr(),
		Mask: opts[dhcp4.OptionSubnetMask],
	}
	classOpts := opts[dhcp4.OptionVendorClassIdentifier]
	for _, vIP := range d.nmd.GetVeniceIPs() {
		d.VeniceIPs[vIP] = true
	}

	if len(d.VeniceIPs) == 0 {
		log.Infof("Found Vendor Class Options: %s", string(classOpts))
		vendorOptions := opts[dhcp4.OptionVendorSpecificInformation]

		if len(vendorOptions) == 0 {
			d.CurState = missingVendorAttributes.String()
		} else {
			d.parseVeniceIPs(vendorOptions)
		}
	} else {
		log.Info("Venice co-ordinates provided via spec. Ignoring vendor options")
	}

	d.LeaseDuration, err = time.ParseDuration(fmt.Sprintf("%ds", binary.BigEndian.Uint32(opts[dhcp4.OptionIPAddressLeaseTime])))
	// Start renewal routine
	// Kick off a renewal process.
	go d.startRenewLoop(d.AckPacket, mgmtLink)
	// Set NMDIPConfig here and then call static assignments
	log.Infof("YIADDR: %s", d.IPNet.IP.String())
	log.Infof("YIMASK: %s", d.IPNet.Mask.String())
	// Update the state to done if valid venice IPs are found
	log.Infof("VENICE IPS: %v", d.VeniceIPs)
	if len(d.VeniceIPs) > 0 {
		d.CurState = dhcpDone.String()
		var veniceIPs []string
		for veniceIP := range d.VeniceIPs {
			veniceIPs = append(veniceIPs, veniceIP)
		}
		d.nmd.SetVeniceIPs(veniceIPs)
	}
	// Assign IP Address here
	addr := &netlink.Addr{
		IPNet: &d.IPNet,
	}
	if err := netlink.AddrAdd(mgmtLink, addr); err != nil {
		log.Errorf("Failed to assign ip address %v to interface %v. Err: %v", addr, mgmtLink.Attrs().Name, err)
	}
	ipCfg := &cluster.IPConfig{
		IPAddress: addr.String(),
	}
	d.nmd.SetIPConfig(ipCfg)
	return
}

func (d *DHCPState) startRenewLoop(ackPacket dhcp4.Packet, mgmtLink netlink.Link) {
	log.Infof("Starting DHCP renewal loop for interface: %v", mgmtLink.Attrs().Name)
	defer func() {
		if err := recover(); err != nil {
			log.Errorf("Renewal control loop exited. Err: %v", err)
			log.Infof("Restarting renew loop...")
			d.startRenewLoop(d.AckPacket, mgmtLink)
		}
	}()
	ticker := time.NewTicker(d.LeaseDuration)
	for {
		select {
		case <-ticker.C:
			d := d
			_, newAck, _ := d.Client.Renew(ackPacket)
			if err := d.updateDHCPState(newAck, mgmtLink); err != nil {
				log.Errorf("Failed to update DHCP State. Err: %v", err)
			}
		}
	}
}

// DoNTPSync does ntp sync with the venice IPs
func (c *IPClient) DoNTPSync() error {
	return nil
}

// GetIPClientIntf returns the current interface for the instantiated IPClient
func (c *IPClient) GetIPClientIntf() string {
	return c.intf.Attrs().Name
}

func (d *DHCPState) parseVeniceIPs(vendorOpts []byte) {
	log.Info("Found Raw Vendor Specified Attributes: ", vendorOpts)
	// Check for code 241 if not parse the entire vendor attrs as string
	if vendorOpts[0] == 241 {
		var veniceIPs [][]byte
		// parse code 241 and a series of 4-Octet IP Addresses
		option241IPAddresses := vendorOpts[2:]
		log.Infof("Found vendor opts for option code 241. %v", option241IPAddresses)

		for IPv4OctetSize <= len(option241IPAddresses) {
			option241IPAddresses, veniceIPs = option241IPAddresses[IPv4OctetSize:], append(veniceIPs, option241IPAddresses[0:IPv4OctetSize:IPv4OctetSize])
		}
		log.Infof("Parsed 241 options. %v", veniceIPs)

		for _, v := range veniceIPs {
			IP := net.IP{v[0], v[1], v[2], v[3]}
			d.VeniceIPs[IP.String()] = true
		}
		if len(d.VeniceIPs) == 0 {
			log.Errorf("Could not find any valid venice IP from parsed vendor attrs: %v", string(vendorOpts))
			d.CurState = missingVendorAttributes.String()
		}

		return
	}
	// Split as Comma Separated Venice IPs or FQDNS. TODO. If FQDN add better error handling in case of invalid options
	controllers := strings.Split(string(vendorOpts), ",")
	// TODO add support for FQDNs.
	for _, c := range controllers {
		IP := net.ParseIP(c)
		if len(IP) == 0 {
			// Ignore and continue
			log.Errorf("Found invalid VENICE IP: %v", c)
			continue
		}
		d.VeniceIPs[IP.String()] = true
	}
	if len(d.VeniceIPs) == 0 {
		log.Errorf("Could not find any valid venice IP from parsed vendor attrs: %v", string(vendorOpts))
		d.CurState = missingVendorAttributes.String()
	}
}
