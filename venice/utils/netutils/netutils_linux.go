// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package netutils

import (
	"fmt"
	"net"
	"runtime"
	"syscall"
	"time"

	"github.com/pensando/netlink"
	"github.com/pensando/netns"

	"github.com/pensando/sw/venice/utils/log"
)

// CreateVethPair creates veth interface pairs with specified name
func CreateVethPair(name1, name2 string) error {
	log.Infof("Creating Veth pairs with name: %s, %s", name1, name2)

	// Veth pair params
	veth := &netlink.Veth{
		LinkAttrs: netlink.LinkAttrs{
			Name:   name1,
			TxQLen: 0,
		},
		PeerName: name2,
	}

	// Create the veth pair
	if err := netlink.LinkAdd(veth); err != nil {
		log.Errorf("error creating veth pair: %v", err)
		return err
	}

	return nil
}

// DeleteVethPair deletes veth interface pairs
func DeleteVethPair(name1, name2 string) error {
	log.Infof("Deleting Veth pairs with name: %s, %s", name1, name2)

	// Veth pair params
	veth := &netlink.Veth{
		LinkAttrs: netlink.LinkAttrs{
			Name:   name1,
			TxQLen: 0,
		},
		PeerName: name2,
	}

	// Delete the veth pair
	if err := netlink.LinkDel(veth); err != nil {
		log.Errorf("error deleting veth pair: %v", err)
		return err
	}

	return nil
}

// MoveIntfToNamespace moves intf to namespace
func MoveIntfToNamespace(intfName, nsPath string) error {
	// if we are moving to current namespace, ignore it
	if nsPath == "/proc/self/ns/net" {
		return nil
	}

	// Lock the OS Thread so we don't accidentally switch namespaces
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	// Find the network interface identified by the SrcName attribute.
	iface, err := netlink.LinkByName(intfName)
	if err != nil {
		return fmt.Errorf("failed to get link by name %q: %v", intfName, err)
	}

	newNs, err := netns.GetFromPath(nsPath)
	if err != nil {
		return fmt.Errorf("failed get network namespace %q: %v", nsPath, err)
	}
	defer newNs.Close()
	if err = netlink.LinkSetNsFd(iface, int(newNs)); err != nil {
		return fmt.Errorf("failed to set namespace on link %q: %v", intfName, err)
	}

	return nil
}

// SetupIntfInNamespace configures interface in namespace
func SetupIntfInNamespace(intfName, nsIntfName, nsPath string, ipv4AddrMask, ipv6AddrMask *net.IPNet) error {
	// if we are moving to current namespace, ignore it
	if nsPath == "/proc/self/ns/net" {
		return nil
	}

	// Lock the OS Thread so we don't accidentally switch namespaces
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	// find the new namespace
	newNs, err := netns.GetFromPath(nsPath)
	if err != nil {
		return fmt.Errorf("failed get network namespace %q: %v", nsPath, err)
	}
	defer newNs.Close()

	// Save the current network namespace
	origns, err := netns.Get()
	if err != nil {
		log.Errorf("Error getting current namespace. Err: %v", err)
		return err
	}

	// go back to original namespace on return
	defer func() {
		netns.Set(origns)
		origns.Close()
	}()

	// change to new namespace
	err = netns.Set(newNs)
	if err != nil {
		log.Errorf("Error changing to namespace: %s. Err: %v", nsPath, err)
		return err
	}

	// Find the network interface
	iface, err := netlink.LinkByName(intfName)
	if err != nil {
		return fmt.Errorf("failed to get link by name %q: %v", intfName, err)
	}

	// Down the interface before configuring
	if err = netlink.LinkSetDown(iface); err != nil {
		return fmt.Errorf("failed to set link down: %v", err)
	}

	// change the interface name
	if err = netlink.LinkSetName(iface, nsIntfName); err != nil {
		return fmt.Errorf("failed to set link name: %v", err)
	}

	// set IPv4 address
	ipAddr := &netlink.Addr{IPNet: ipv4AddrMask, Label: ""}
	if err = netlink.AddrAdd(iface, ipAddr); err != nil {
		return fmt.Errorf("failed to set link address: %v", err)
	}

	// set IPv6 address
	if ipv6AddrMask != nil {
		ipAddr := &netlink.Addr{IPNet: ipv6AddrMask, Label: "", Flags: syscall.IFA_F_NODAD}
		if err = netlink.AddrAdd(iface, ipAddr); err != nil {
			return fmt.Errorf("failed to set link IPv6 address: %v", err)
		}
	}

	// Up the interface.
	cnt := 0
	for err = netlink.LinkSetUp(iface); err != nil && cnt < 3; cnt++ {
		log.Debugf("retrying link setup because of: %v", err)
		time.Sleep(10 * time.Millisecond)
		err = netlink.LinkSetUp(iface)
	}
	if err != nil {
		return fmt.Errorf("failed to set link up: %v", err)
	}

	return nil
}

// SetIntfUp brings up the interface
func SetIntfUp(intfName string) error {
	// get the interface handle
	iface, err := netlink.LinkByName(intfName)
	if err != nil {
		return fmt.Errorf("failed to get link by name %q: %v", intfName, err)
	}

	// up the interface
	cnt := 0
	for err = netlink.LinkSetUp(iface); err != nil && cnt < 3; cnt++ {
		log.Debugf("retrying link setup because of: %v", err)
		time.Sleep(10 * time.Millisecond)
		err = netlink.LinkSetUp(iface)
	}
	if err != nil {
		return fmt.Errorf("failed to set link up: %v", err)
	}

	return nil
}

// GetIntfMac returns interface's mac address
func GetIntfMac(intfName string) (net.HardwareAddr, error) {
	// get the interface handle
	iface, err := netlink.LinkByName(intfName)
	if err != nil {
		return nil, fmt.Errorf("failed to get link by name %q: %v", intfName, err)
	}

	return iface.Attrs().HardwareAddr, nil
}
