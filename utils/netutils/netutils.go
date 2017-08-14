// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netutils

import (
	"fmt"
	"net"
	"os"
	"strings"

	"github.com/vishvananda/netlink"
)

// nameSliceFromMap return slice of DNS names from Map
func nameSliceFromMap(namesMap map[string]bool) []string {
	var names []string

	for k := range namesMap {
		names = append(names, k)
	}
	return names
}

//NameAndIPs returns slices of names and IP addresses of this host, including the FQDN
// names will have hostname and FQDN (if they are different).
// ips will have one or more if IPv4/IPv6 addresses which are result of DNS lookup for
// this hosts FQDN
func NameAndIPs() (names []string, ips []net.IP) {
	namesMap := make(map[string]bool)

	hostname, err := os.Hostname()
	if err != nil {
		return nil, nil
	}
	namesMap[strings.ToLower(hostname)] = true

	addrs, err := net.LookupIP(hostname)
	if err != nil {
		return nameSliceFromMap(namesMap), ips
	}

	for _, addr := range addrs {
		ip, err := addr.MarshalText()
		if err != nil {
			continue
		}
		hosts, err := net.LookupAddr(string(ip))
		if err != nil || len(hosts) == 0 {
			continue
		}
		name := strings.ToLower(strings.TrimSuffix(hosts[0], "."))
		namesMap[name] = true
		ips = append(ips, addr)
	}
	return nameSliceFromMap(namesMap), ips
}

// findInterfaceForIP finds the interface on which the provided IP is present or
// can be configured. It returns the interface, network and whether there is an
// exact match.
func findInterfaceForIP(ipAddr string) (*net.Interface, *net.IPNet, bool, error) {
	ip := net.ParseIP(ipAddr)
	if ip == nil {
		return nil, nil, false, fmt.Errorf("%v is not an IP address", ipAddr)
	}
	intfs, err := net.Interfaces()
	if err != nil {
		return nil, nil, false, err
	}
	for ii := range intfs {
		addrs, err := intfs[ii].Addrs()
		if err != nil {
			return nil, nil, false, err
		}
		// Check exact match first.
		for jj := range addrs {
			addr, network, err := net.ParseCIDR(addrs[jj].String())
			if err != nil {
				return nil, nil, false, err
			}
			if string(addr) == string(ip) {
				return &intfs[ii], network, true, nil
			}
		}
		// Check if there is a network that can have this IP.
		for jj := range addrs {
			_, network, err := net.ParseCIDR(addrs[jj].String())
			if err != nil {
				return nil, nil, false, err
			}
			if network.Contains(ip) {
				return &intfs[ii], network, false, nil
			}
		}
	}
	return nil, nil, false, nil
}

// HasIP checks if the provided ip address is configured on any interface.
func HasIP(ipAddr string) (bool, error) {
	_, _, found, err := findInterfaceForIP(ipAddr)
	return found, err
}

// AddSecondaryIP adds the provided ip address as a secondary IP to an interface
// that can have it. If no interface can have it, it returns an error.
func AddSecondaryIP(ipAddr string) error {
	ip := net.ParseIP(ipAddr)
	if ip == nil {
		return fmt.Errorf("%v is not an IP address", ipAddr)
	}
	intf, network, found, err := findInterfaceForIP(ipAddr)
	if err != nil {
		return err
	}
	if found {
		return fmt.Errorf("%v is already configured", ipAddr)
	}
	if intf == nil {
		return fmt.Errorf("%v cannot be added to any interface", ipAddr)
	}
	link, err := netlink.LinkByName(intf.Name)
	if err != nil {
		return err
	}
	network.IP = ip
	return netlink.AddrAdd(link, &netlink.Addr{Label: fmt.Sprintf("%s:pens", intf.Name), IPNet: network})
}

// DeleteIP deletes the specified address from the interface it is found on.
func DeleteIP(ipAddr string) error {
	ip := net.ParseIP(ipAddr)
	if ip == nil {
		return fmt.Errorf("%v is not an IP address", ipAddr)
	}
	intf, network, found, err := findInterfaceForIP(ipAddr)
	if err != nil {
		return err
	}
	if !found {
		// Not configured on any interface.
		return nil
	}
	link, err := netlink.LinkByName(intf.Name)
	if err != nil {
		return err
	}
	network.IP = ip
	return netlink.AddrDel(link, &netlink.Addr{IPNet: network})
}
