// +build linux

package ipif

import (
	"context"
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"strconv"
	"strings"
	"time"

	"github.com/krolaw/dhcp4"
	"github.com/pensando/dhcp-client"
	"github.com/vishvananda/netlink"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/nmd/api"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// NewIPClient returns a new IPClient instance
func NewIPClient(nmd api.NmdAPI, intf string, pipeline string) (*IPClient, error) {
	dhcpState := DHCPState{
		nmd:                  nmd,
		VeniceIPs:            make(map[string]bool),
		pipeline:             pipeline,
		SecondaryIntfClients: make(map[int]*dhcp.Client),
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
		nmd:                 nmd,
		primaryIntf:         link,
		secondaryInterfaces: getSecondaryMgmtLink(pipeline, intf),
		dhcpState:           &dhcpState,
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
	if err := netlink.AddrReplace(c.primaryIntf, addr); err != nil {
		log.Errorf("Failed to assign ip address %v to interface %v. Err: %v", ipConfig.IPAddress, c.primaryIntf, err)
		return "", err
	}
	// Assign default gw TODO: Verify if the route added by AddrAdd is good enough
	if len(ipConfig.DefaultGW) != 0 {
		defaultRoute := &netlink.Route{
			LinkIndex: c.primaryIntf.Attrs().Index,
			Gw:        net.ParseIP(ipConfig.DefaultGW),
		}
		err := netlink.RouteAdd(defaultRoute)
		if err != nil {
			log.Errorf("Failed to add default gw %v for the interface %v. Usually happens when the default gateway is already setup. Err: %v", ipConfig.DefaultGW, c.primaryIntf, err)
		}
	}
	c.nmd.SetMgmtInterface(c.primaryIntf.Attrs().Name)

	return addr.String(), nil
}

// DoDHCPConfig performs dhcp on the management interface and obtains venice co-ordinates
func (c *IPClient) DoDHCPConfig() error {

	dhcpCtx, cancel := context.WithCancel(context.Background())
	if c.dhcpState.DhcpCancel != nil {
		log.Info("Clearing existing dhcp go routines")
		c.dhcpState.DhcpCancel()
	}
	c.dhcpState.DhcpCancel = cancel
	c.dhcpState.DhcpCtx = dhcpCtx

	// This is needed to enable 1st tick on a large ticker duration. TODO investigate leaks here
	if err := c.doDHCP(); err == nil {
		log.Info("DHCP successful")
		return nil
	}

	ticker := time.NewTicker(AdmissionRetryDuration)
	go func(ctx context.Context) {
		c := c
		for {
			select {
			case <-ticker.C:
				log.Info("Retrying DHCP Config")
				if err := c.doDHCP(); err != nil {
					log.Errorf("DHCP Config failed due to %v | Retrying...", err)
					if (c.dhcpState.PrimaryIntfClient) != nil {
						if err := c.dhcpState.PrimaryIntfClient.Close(); err != nil {
							log.Errorf("Failed to cancel DoDHCPConfig. Err: %v", err)
						}
					}
				} else {
					log.Info("DHCP was successful. Exiting the DHCP retry loop")
					return
				}
			case <-ctx.Done():
				if (c.dhcpState.PrimaryIntfClient) != nil {
					if err := c.dhcpState.PrimaryIntfClient.Close(); err != nil {
						log.Errorf("Failed to cancel DoDHCPConfig. Err: %v", err)
					}
				}
				return
			}
		}
	}(c.dhcpState.DhcpCtx)
	return nil
}

// StopDHCPConfig will stops the DHCP Control Loop
func (c *IPClient) StopDHCPConfig() {
	if c.dhcpState.DhcpCancel != nil {
		log.Info("Cancelling DHCP Control loop")
		c.dhcpState.DhcpCancel()
	}
}

func (c *IPClient) doDHCP() error {
	if (c.dhcpState.PrimaryIntfClient) != nil {
		if err := c.dhcpState.PrimaryIntfClient.Close(); err != nil {
			log.Errorf("Failed to close pktsock during doDHCP init for primary interface. Err: %v", err)
		}
	}

	for _, c := range c.dhcpState.SecondaryIntfClients {
		if c != nil {
			if err := c.Close(); err != nil {
				log.Errorf("Failed to close pktsock during doDHCP init for secondary interfaces. Err: %v", err)
			}
		}
	}

	c.dhcpState.PrimaryIntfClient = instantiateDHCPClient(c.primaryIntf)

	for idx, link := range c.secondaryInterfaces {
		c.dhcpState.SecondaryIntfClients[idx] = instantiateDHCPClient(link)
	}

	if err := c.startDHCP(c.dhcpState.PrimaryIntfClient, c.primaryIntf); err != nil {
		log.Errorf("Failed to start dhcp client on primary interface. Err: %v", err)
		for idx, client := range c.dhcpState.SecondaryIntfClients {
			log.Info("Trying DHCP on secondary interface")
			err := c.startDHCP(client, c.secondaryInterfaces[idx])
			if err == nil {
				return nil
			}
		}
		log.Infof("Trying DHCP on secondary interface")
		return err
	}

	return nil
}

func (c *IPClient) startDHCP(client *dhcp.Client, intf netlink.Link) (err error) {
	if client == nil {
		return errors.New("Failed to find dhcp client")
	}
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

	ack, err := client.GetAcknowledgement(&req)
	log.Infof("Get Ack Returned: %v", ack)
	if err != nil {
		log.Errorf("Failed to get ack. Err: %v", err)
		return
	}

	return c.dhcpState.updateDHCPState(ack, intf)

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

	log.Infof("Found Vendor Class Options: %s", string(classOpts))
	vendorOptions := opts[dhcp4.OptionVendorSpecificInformation]

	if len(vendorOptions) != 0 {
		// If venice IPs are already supplied,
		// no need to parse vendor options for venice IPs
		shouldParseVeniceIPs := false
		if len(d.VeniceIPs) == 0 {
			shouldParseVeniceIPs = true
		} else {
			log.Info("Venice co-ordinates provided via spec. Ignoring vendor options")
		}
		d.parseVendorOptions(vendorOptions, shouldParseVeniceIPs)
	}

	if len(d.VeniceIPs) == 0 {
		d.CurState = missingVendorAttributes.String()
	}

	staticRouteOptions := opts[dhcp4.OptionClasslessRouteFormat]

	if len(staticRouteOptions) != 0 {
		log.Infof("Static Route Options found: %v ", staticRouteOptions)
		d.parseClasslessStaticRoutes(staticRouteOptions)
	}

	dur, err := time.ParseDuration(fmt.Sprintf("%ds", binary.BigEndian.Uint32(opts[dhcp4.OptionIPAddressLeaseTime])))
	if err != nil {
		log.Errorf("Failed to parse the lease duration. Err: %v", err)
	}

	d.LeaseDuration = dur

	renewCtx, renewCancel := context.WithCancel(context.Background())
	if d.RenewCancel != nil {
		log.Info("Clearing existing dhcp go routines")
		d.RenewCancel()
		d.RenewCancel = nil
	}
	d.RenewCancel = renewCancel
	d.RenewCtx = renewCtx

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

	if len(d.InterfaceIPs) > 0 {
		interfaceIPs := make(map[uint32]*cluster.IPConfig)
		for _, interfaceIP := range d.InterfaceIPs {
			interfaceIPs[uint32(interfaceIP.IfID)] = &cluster.IPConfig{
				IPAddress: interfaceIP.IPAddress.String() + "/" + strconv.Itoa(int(interfaceIP.PrefixLen)),
				DefaultGW: interfaceIP.GwIP.String(),
			}
		}
		d.nmd.SetInterfaceIPs(interfaceIPs)
	}

	// Assign IP Address here
	addr := &netlink.Addr{
		IPNet: &d.IPNet,
	}

	if err := netlink.AddrAdd(mgmtLink, addr); err != nil {
		// Make this a non error TODO
		if !strings.Contains(err.Error(), "file exists") {
			log.Errorf("Failed to assign ip address %v to interface %v. Err: %v", addr, mgmtLink.Attrs().Name, err)
			return err

		}
		log.Infof("IP Address %v already present on interface %v. Err: %v", addr, mgmtLink.Attrs().Name, err)
	}
	ipCfg := &cluster.IPConfig{
		IPAddress: addr.String(),
	}
	curIPConfig := d.nmd.GetIPConfig()
	if curIPConfig != nil && curIPConfig.IPAddress != ipCfg.IPAddress {
		log.Infof("IP Address for Naples got updated. Pushing the new configs to processes.")
		d.nmd.PersistState(true)
	}

	log.Infof("IPConfig during DHCP: %v", ipCfg)
	d.nmd.SetIPConfig(ipCfg)
	d.nmd.SetMgmtInterface(mgmtLink.Attrs().Name)

	// Set the interface IPs
	// Note: This is a temporary code. This config needs to handled by the netagent
	d.setInterfaceIPs()
	d.setStaticRoutes()
	return
}

func (d *DHCPState) startRenewLoop(ackPacket dhcp4.Packet, mgmtLink netlink.Link) {
	log.Infof("Starting DHCP renewal loop for interface: %v", mgmtLink.Attrs().Name)
	ticker := time.NewTicker(d.LeaseDuration)
	for {
		select {
		case <-ticker.C:
			d := d
			_, newAck, err := d.PrimaryIntfClient.Renew(ackPacket)
			if err != nil {
				log.Infof("Failed to get the new Ack Packet.  Err: %v | Retrying...", err)
			} else {
				log.Infof("Renewed Ack Packet: %v. ", newAck)
				if err := d.updateDHCPState(newAck, mgmtLink); err != nil {
					log.Errorf("Failed to update DHCP State. Err: %v", err)
				}
			}

		case <-d.RenewCtx.Done():
			log.Info("Got cancel for renewals")
			return
		case <-d.DhcpCtx.Done():
			log.Info("Killing renewal loop")
			return
		}
	}
}

// DoNTPSync does ntp sync with the venice IPs
func (c *IPClient) DoNTPSync() error {
	return nil
}

// GetIPClientIntf returns the current interface for the instantiated IPClient
func (c *IPClient) GetIPClientIntf() string {
	return c.primaryIntf.Attrs().Name
}

// GetDHCPState get current dhcp state
func (c *IPClient) GetDHCPState() string {
	c.dhcpState.Lock()
	defer c.dhcpState.Unlock()
	return c.dhcpState.CurState
}

// GetStaticRoutes returns the set of static routes received via dhcp
func (c *IPClient) GetStaticRoutes() []StaticRoute {
	c.dhcpState.Lock()
	defer c.dhcpState.Unlock()
	log.Infof("GetStaticRoutes: %v", c.dhcpState.StaticRoutes)
	return c.dhcpState.StaticRoutes
}

// GetInterfaceIPs returns the set of interface IP information received via DHCP
func (c *IPClient) GetInterfaceIPs() []InterfaceIP {
	log.Infof("GetInterfaceIPs: %v", c.dhcpState.InterfaceIPs)
	return c.dhcpState.InterfaceIPs
}

func (d *DHCPState) parseVendorOptions(vendorOpts []byte, shouldParseVeniceIPs bool) {
	// Vendor Options are of the format
	//  -------------------------------------------
	// |Code|Len|Data Item|Code|Len|Data Item|.....|
	//  -------------------------------------------
	log.Infof("Found Raw Vendor Specified Attributes: %v", vendorOpts)
	for len(vendorOpts) > 0 {
		log.Infof("Vendor Specific Information Code: %d", vendorOpts[0])

		if len(vendorOpts) < 2 {
			log.Errorf("Not enough information in vendor specific suboption %d", vendorOpts[0])
			return
		}

		switch vendorOpts[0] {
		case VeniceCoordinatesSubOptID:
			subOptionLength := int(vendorOpts[1]) + 2
			if len(vendorOpts) < subOptionLength {
				log.Errorf("Not enough information in vendor specific suboption %d", vendorOpts[0])
				return
			}
			// Parse for Venice IPs only if it was not already supplied through config
			if shouldParseVeniceIPs {
				d.parseVeniceIPs(vendorOpts[:subOptionLength])
			}
			vendorOpts = vendorOpts[subOptionLength:]

		case DSCInterfaceIPsSubOptID:
			subOptionLength := int(vendorOpts[1]) + 2
			if len(vendorOpts) < subOptionLength {
				log.Errorf("Not enough information in vendor specific suboption %d", vendorOpts[0])
				return
			}
			d.parseInterfaceIPs(vendorOpts[:subOptionLength])
			vendorOpts = vendorOpts[subOptionLength:]

		default:
			// If the Venice IPs are comma seperated values or FQDNS
			// Also, parse for Venice IPs only if it was not already supplied through config
			if shouldParseVeniceIPs {
				d.parseVeniceIPs(vendorOpts)
			}
			return
		}
	}
}

func (d *DHCPState) parseVeniceIPs(vendorOpts []byte) {
	log.Info("Found Raw Vendor Specified Attributes: ", vendorOpts)
	// Check for code 241 if not parse the entire vendor attrs as string
	if vendorOpts[0] == VeniceCoordinatesSubOptID {
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

func (d *DHCPState) parseInterfaceIPs(vendorOpts []byte) {
	// Parse DHCP vendor options for ip address info of uplink interfaces
	// Each option is tuple of { ifid, prefix-len, if-ipaddress, gw-ipaddress }
	// Which are of data types { integer 8, integer 8, ip-address, ip-address }
	ipInterfaceInfoSize := 10
	rawInterfaceIPs := vendorOpts[2:]

	var interfaceIPs [][]byte

	for ipInterfaceInfoSize <= len(rawInterfaceIPs) {
		rawInterfaceIPs, interfaceIPs = rawInterfaceIPs[ipInterfaceInfoSize:], append(interfaceIPs, rawInterfaceIPs[0:ipInterfaceInfoSize])
	}

	for _, ipInfo := range interfaceIPs {
		interfaceIP := InterfaceIP{}
		interfaceIP.IfID = int8(ipInfo[0])
		interfaceIP.PrefixLen = int8(ipInfo[1])
		interfaceIP.IPAddress = net.IP{ipInfo[2], ipInfo[3], ipInfo[4], ipInfo[5]}
		interfaceIP.GwIP = net.IP{ipInfo[6], ipInfo[7], ipInfo[8], ipInfo[9]}

		if err := d.isValidInterfaceIPInfo(interfaceIP); err == nil {
			d.InterfaceIPs = append(d.InterfaceIPs, interfaceIP)
		} else {
			log.Errorf("Found invalid DSC interface IP info: %v. Err: %v", interfaceIP, err)
		}
	}
}

func (d *DHCPState) isValidInterfaceIPInfo(intfIP InterfaceIP) error {
	// checks to verify if the DSC interface IP info parsed from vendor specific options is valid
	if _, exists := DSCIfIDToInterfaceName[intfIP.IfID]; !exists {
		return fmt.Errorf("No interface corresponding to interface IP info exists")
	}

	if intfIP.PrefixLen > 32 {
		return fmt.Errorf("Invalid prefix length specified in interface IP info")
	}

	return nil
}

func (d *DHCPState) setInterfaceIPs() {
	// Configure the interface IPs on the uplink interfaces
	for _, interfaceIP := range d.InterfaceIPs {
		var linkName string
		if d.pipeline == globals.NaplesPipelineApollo {
			linkName = ApuluDSCIfIDToInterfaceName[interfaceIP.IfID]
		} else {
			linkName = DSCIfIDToInterfaceName[interfaceIP.IfID]
		}

		link, err := netlink.LinkByName(linkName)
		if err != nil {
			log.Errorf("Failed to look up interface %v during interface config. Err: %v", linkName, err)
			continue
		}

		ipAddrString := interfaceIP.IPAddress.String() + "/" + strconv.Itoa(int(interfaceIP.PrefixLen))
		ipAddr, err := netlink.ParseAddr(ipAddrString)
		if err != nil {
			log.Errorf("Could not parse IP Address from %v", ipAddrString)
			continue
		}

		// Assign IP Address statically
		if err := netlink.AddrReplace(link, ipAddr); err != nil {
			log.Errorf("Failed to assign ip address %v to interface %v. Err: %v", ipAddrString, linkName, err)
			continue
		}

		if err := netlink.LinkSetUp(link); err != nil {
			log.Errorf("Failed to bring interface %v link up. Err: %v", linkName, err)
		}
	}
}

func (d *DHCPState) parseClasslessStaticRoutes(routes []byte) {
	minRouteOptionSize := 5

	for len(routes) >= minRouteOptionSize {
		prefixLen := uint32(routes[0])

		routes = routes[1:]
		var destination net.IP
		if prefixLen > 24 {
			if len(routes) < 4 {
				break
			}
			destination = net.IP{routes[0], routes[1], routes[2], routes[3]}
			routes = routes[4:]
		} else if prefixLen > 16 {
			if len(routes) < 3 {
				break
			}
			destination = net.IP{routes[0], routes[1], routes[2], 0}
			routes = routes[3:]
		} else if prefixLen > 8 {
			if len(routes) < 2 {
				break
			}
			destination = net.IP{routes[0], routes[1], 0, 0}
			routes = routes[2:]
		} else {
			if len(routes) < 1 {
				break
			}
			destination = net.IP{routes[0], 0, 0, 0}
			routes = routes[1:]
		}

		if len(routes) < 4 {
			break
		}
		gateway := net.IP{routes[0], routes[1], routes[2], routes[3]}
		routes = routes[4:]

		staticRoute := StaticRoute{DestAddr: destination, DestPrefixLen: prefixLen, NextHopAddr: gateway}

		if err := d.isValidStaticRouteInfo(staticRoute); err == nil {
			log.Infof("Appending StaticRoute")
			d.StaticRoutes = append(d.StaticRoutes, StaticRoute{DestAddr: destination, DestPrefixLen: prefixLen, NextHopAddr: gateway})
		} else {
			log.Errorf("Found invalid DSC static route info: %v. Err: %v", staticRoute, err)
		}
	}
	log.Infof("Static Routes: %v", d.StaticRoutes)
}

func (d *DHCPState) isValidStaticRouteInfo(route StaticRoute) error {
	// checks to verify if the DSC static route info parsed from DHCP options is valid

	if route.DestPrefixLen > 32 {
		return fmt.Errorf("Invalid prefix length specified in interface IP info")
	}

	return nil
}

func (d *DHCPState) setStaticRoutes() {
	// Configure the static routes
	log.Infof("Setting the classless static routes got from DHCP")
	for _, r := range d.StaticRoutes {
		destStr := r.DestAddr.String() + "/" + strconv.Itoa(int(r.DestPrefixLen))
		_, dest, err := net.ParseCIDR(destStr)
		if err != nil {
			log.Errorf("Failed to Parse Destination address %v in static route %v. Err: %v", destStr, r, err)
			continue
		}

		route := &netlink.Route{
			Dst: dest,
			Gw:  r.NextHopAddr,
		}
		if err := netlink.RouteAdd(route); err != nil {
			log.Errorf("Failed to configure static route %v. Err: %v", route, err)
		}
	}
}

func getSecondaryMgmtLink(pipeline, primaryIntf string) (secondaryMgmtLinks []netlink.Link) {
	switch pipeline {
	case globals.NaplesPipelineApollo:
		dsc0, err := netlink.LinkByName(ApuluINB0Interface)
		if err != nil {
			log.Errorf("Failed to look up interface %v during IP Client init. Err: %v", ApuluINB0Interface, err)
		}
		dsc1, err := netlink.LinkByName(ApuluINB1Interface)
		if err != nil {
			log.Errorf("Failed to look up interface %v during IP Client init. Err: %v", ApuluINB1Interface, err)
		}

		if dsc0 != nil {
			if err := netlink.LinkSetUp(dsc0); err != nil {
				log.Errorf("Failed to set interface %v up during IP Client init. Err: %v", ApuluINB0Interface, err)
			}
			secondaryMgmtLinks = append(secondaryMgmtLinks, dsc0)
		}

		if dsc1 != nil {
			if err := netlink.LinkSetUp(dsc1); err != nil {
				log.Errorf("Failed to set interface %v up during IP Client init. Err: %v", ApuluINB1Interface, err)
			}
			secondaryMgmtLinks = append(secondaryMgmtLinks, dsc0)
		}
		return
	case globals.NaplesPipelineIris:
		oob, err := netlink.LinkByName(NaplesOOBInterface)
		if err != nil {
			log.Errorf("Failed to look up interface %v during IP Client init. Err: %v", NaplesOOBInterface, err)
		}
		if oob != nil {
			if err := netlink.LinkSetUp(oob); err != nil {
				log.Errorf("Failed to set interface %v up during IP Client init. Err: %v", NaplesOOBInterface, err)
			}
		}
		secondaryMgmtLinks = append(secondaryMgmtLinks, oob)
		return
	default:
		return
	}
}

func instantiateDHCPClient(link netlink.Link) *dhcp.Client {
	pktSock, err := dhcp.NewPacketSock(link.Attrs().Index, 68, 67)
	if err != nil {
		log.Errorf("Failed to create packet sock Err: %v", err)
		return nil
	}

	client, err := dhcp.New(dhcp.HardwareAddr(link.Attrs().HardwareAddr), dhcp.Connection(pktSock))
	if err != nil {
		log.Errorf("Failed to  instantiate primary DHCP Client. Err: %v", err)
	}
	return client
}
