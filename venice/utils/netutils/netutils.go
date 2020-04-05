// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netutils

import (
	"bytes"
	"errors"
	"fmt"
	"net"
	"os"
	"strings"
	"sync"
	"time"
	"unsafe"

	"github.com/pensando/netlink"

	"github.com/pensando/sw/venice/globals"
)

var errPortAllocFailed = errors.New("could not find a free port")
var endianNess string

func init() {
	// Determine endianness
	var test uint32 = 0xff
	firstByte := (*byte)(unsafe.Pointer(&test))
	if *firstByte == 0 {
		endianNess = "big"
	} else {
		endianNess = "little"
	}
}

// IPv4Uint32ToString converts uint32 ip address to a string
func IPv4Uint32ToString(ipUint32 uint32) string {
	var b1, b2, b3, b4 byte

	if endianNess == "little" {
		b1, b2, b3, b4 = byte(ipUint32>>24), byte(ipUint32>>16),
			byte(ipUint32>>8), byte(ipUint32)
	} else {
		b1, b2, b3, b4 = byte(ipUint32), byte(ipUint32>>8),
			byte(ipUint32>>16), byte((ipUint32)>>24)
	}

	return fmt.Sprintf("%d.%d.%d.%d", b1, b2, b3, b4)
}

// IPv4ToUint32 converts ipaddr string to a uint32
func IPv4ToUint32(ipaddr string) (uint32, error) {
	var ipUint32 uint32

	ip := net.ParseIP(ipaddr).To4()
	if ip == nil {
		return 0, errors.New("ipv4 to uint32 conversion: invalid ip format")
	}
	if endianNess == "little" {
		ipUint32 = (uint32(ip[3]) | (uint32(ip[2]) << 8) |
			(uint32(ip[1]) << 16) | (uint32(ip[0]) << 24))
	} else {
		ipUint32 = uint32(ip[0]) | (uint32(ip[1]) << 8) |
			(uint32(ip[2]) << 16) | (uint32(ip[3]) << 24)
	}

	return ipUint32, nil
}

// TestListenAddr holds the TCP Endpoint on which a test server can listen.
type TestListenAddr struct {
	sync.Mutex
	ListenURL net.Addr
	Port      int
}

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

// FindInterfaceForIP finds the interface on which the provided IP is present or
// can be configured. It returns the interface, network and whether there is an
// exact match.
func FindInterfaceForIP(ipAddr string) (*net.Interface, *net.IPNet, bool, error) {
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
	_, _, found, err := FindInterfaceForIP(ipAddr)
	return found, err
}

// IsAConfiguredIP returns true if one of the interfaces on this machine has this IP
func IsAConfiguredIP(ipAddr string) (bool, error) {
	ip := net.ParseIP(ipAddr)
	if ip == nil {
		return false, fmt.Errorf("%v is not an IP address", ipAddr)
	}
	intfs, err := net.Interfaces()
	if err != nil {
		return false, err
	}
	for ii := range intfs {
		addrs, err := intfs[ii].Addrs()
		if err != nil {
			return false, err
		}
		for jj := range addrs {
			addr, _, err := net.ParseCIDR(addrs[jj].String())
			if err != nil {
				return false, err
			}
			if string(addr) == string(ip) {
				return true, nil
			}
		}
	}
	return false, nil
}

// AddSecondaryIP adds the provided ip address as a secondary IP to an interface
// that can have it. If no interface can have it, it returns an error.
func AddSecondaryIP(ipAddr string) error {
	ip := net.ParseIP(ipAddr)
	if ip == nil {
		return fmt.Errorf("%v is not an IP address", ipAddr)
	}
	intf, network, found, err := FindInterfaceForIP(ipAddr)
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
	if err = netlink.AddrAdd(link, &netlink.Addr{Label: fmt.Sprintf("%s:pens", intf.Name), IPNet: network}); err != nil {
		return err
	}
	go ARPSendGratuitous(intf, ip, 3, time.Second)
	return nil
}

// DeleteIP deletes the specified address from the interface it is found on.
func DeleteIP(ipAddr string) error {
	ip := net.ParseIP(ipAddr)
	if ip == nil {
		return fmt.Errorf("%v is not an IP address", ipAddr)
	}
	intf, network, found, err := FindInterfaceForIP(ipAddr)
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

// GetAvailablePort returns an available port as 127.0.0.1:PORT
func (t *TestListenAddr) GetAvailablePort() error {
	t.Lock()
	defer t.Unlock()
	addr, err := net.ResolveTCPAddr("tcp", "localhost:0")

	if err != nil {
		return errPortAllocFailed
	}

	// Try listening
	l, err := net.ListenTCP("tcp", addr)
	if err != nil {
		return errPortAllocFailed
	}
	defer l.Close()
	t.ListenURL = l.Addr()
	t.Port = t.ListenURL.(*net.TCPAddr).Port
	return nil
}

// AppendIPIfNotPresent adds the IP address to a slice if it is not already there
// This is ok for small slices. For big slices, maintain an auxiliary map.
func AppendIPIfNotPresent(ip net.IP, ips []net.IP) []net.IP {
	for _, a := range ips {
		if a.Equal(ip) {
			return ips
		}
	}
	return append(ips, ip)
}

// IsPensandoMACAddress returns true if the supplied string represents a valid, 48-bit MAC address for a Pensando device
func IsPensandoMACAddress(addr string) bool {
	mac, err := net.ParseMAC(addr)
	return err == nil && len(mac) == 6 && bytes.HasPrefix(mac, globals.PensandoOUI)
}
