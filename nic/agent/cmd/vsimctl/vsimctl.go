// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"net"
	"os"
	"strings"

	"github.com/pensando/sw/venice/orch/simapi"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

const (
	defVcsimURL = "http://pen-master:18086"
)

// createVM simulates VM creation
func createVM(vsimURL, vmNAme, hostMac, ipAddr, macAddr, netName, tenant string, usegVlan uint32) error {
	epName := fmt.Sprintf("ep-%s", ipAddr)

	// request message
	nwReq := &simapi.NwIFSetReq{
		WLName:    vmNAme,
		Name:      epName,
		IPAddr:    ipAddr,
		MacAddr:   macAddr,
		Vlan:      fmt.Sprintf("%d", usegVlan),
		PortGroup: netName,
		SmartNIC:  hostMac,
	}

	log.Infof("Creating VM NwIF: {%+v}", nwReq)

	// create the hostsim container
	resp := &simapi.NwIFSetResp{}
	err := netutils.HTTPPost(vsimURL+"/nwifs/create", nwReq, resp)
	if err != nil {
		log.Errorf("Error creating ")
	}

	log.Infof("Got response: {%+v}", resp)

	return err
}

// deleteVM simulates VM deletion
func deleteVM(vsimURL, ipAddr, macAddr, tenant string, usegVlan uint32) error {
	name := fmt.Sprintf("%d==%s", usegVlan, macAddr)
	u1 := fmt.Sprintf("%s/nwifs/%s/delete", vsimURL, name)

	log.Infof("deleting by posting to URL: %s", u1)

	// issue POST request
	resp := simapi.NwIFSetResp{}
	err := netutils.HTTPPost(u1, &simapi.NwIFSetReq{}, &resp)
	if err != nil {
		log.Errorf("DeleteNwIF %s, failed on host", u1)
		return err
	}

	log.Infof("Got delete response: {%+v}", resp)

	return nil
}

func main() {
	// command line flags
	var (
		isCreate = flag.Bool("create", false, "Create VM")
		isDelete = flag.Bool("delete", false, "Delete VM")
		hostIf   = flag.String("hostif", "ntrunk0", "Host facing interface")
		hostMac  = flag.String("hostmac", "", "Host Mac address")
		ipAddr   = flag.String("ip", "", "VM ip address")
		macAddr  = flag.String("mac", "", "VM mac address")
		vlan     = flag.Uint("vlan", 0, "VM useg vlan")
		netName  = flag.String("net", "default", "VM Network")
		vmName   = flag.String("vm", "vm1", "VM Name")
		vsimURL  = flag.String("vsim", defVcsimURL, "VcSim URL")
	)
	flag.Parse()

	if !*isCreate && !*isDelete {
		fmt.Printf("Needs -create or -delete flag\n")
		flag.PrintDefaults()
		os.Exit(1)
	}

	if *ipAddr == "" || *hostIf == "" || *vlan == 0 {
		fmt.Printf("Insufficient args. Require ip-addr, vlan & host-if\n")
		os.Exit(1)
	}

	// read the mac address of the host interface if host-mac is not specified
	if *hostMac == "" {
		nodeMac, err := netutils.GetIntfMac(*hostIf)
		if err != nil {
			fmt.Printf("Error getting host interface's mac addr, please specify -hostmac. Err: %v\n", err)
			os.Exit(1)
		}
		*hostMac = nodeMac.String()
	}

	// verify ip address is in good format
	addr, _, err := net.ParseCIDR(*ipAddr)
	if err != nil {
		fmt.Printf("Invalid IP CIDR: %s. Err: %v\n", *ipAddr, err)
		os.Exit(1)
	}

	// if mac address is not specified, get it from ip
	if *macAddr == "" {
		*macAddr = fmt.Sprintf("00:50:%02x:%02x:%02x:%02x", addr[12], addr[13], addr[14], addr[15])
	}

	// create or delete based on command line
	if *isCreate {
		// verify we have required args
		if *ipAddr == "" || *macAddr == "" || *vlan == 0 {
			fmt.Printf("Insufficient args. Require ip-addr, mac-addr & vlan\n")
			os.Exit(1)
		}

		// create the endpoint in vmm/apigw
		err = createVM(*vsimURL, *vmName, *hostMac, *ipAddr, strings.ToLower(*macAddr), *netName, "default", uint32(*vlan))
		if err != nil {
			log.Errorf("Error creating vm. Err: %v", err)
			os.Exit(1)
		}
	} else if *isDelete {
		// verify we have required args
		if *ipAddr == "" || *macAddr == "" || *vlan == 0 {
			fmt.Printf("Insufficient args. Require ip-addr, mac-addr & vlan\n")
			os.Exit(1)
		}

		// delete the VM
		err = deleteVM(*vsimURL, *ipAddr, strings.ToLower(*macAddr), "default", uint32(*vlan))
		if err != nil {
			log.Errorf("Error deleting the VM: %v", err)
			os.Exit(1)
		}
	} else {
		fmt.Printf("Needs -create or -delete flag\n")
		os.Exit(1)
	}
}
