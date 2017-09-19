// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/orch/simapi"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

const (
	defHostsimURL = "http://localhost:5050"
	defApisrvURL  = "pen-master:" + globals.APIServerPort
)

// createVM simulates VM creation
func createVM(grpcClient apiclient.Services, hostsimURL, nodeID, ipAddr, macAddr, netName, tenant string, usegVlan uint32) error {
	epName := fmt.Sprintf("ep-%s", ipAddr)

	// endpoint info
	ep := network.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:   epName,
			Tenant: tenant,
		},
		Status: network.EndpointStatus{
			NodeUUID:         nodeID,
			MacAddress:       macAddr,
			IPv4Address:      ipAddr,
			MicroSegmentVlan: usegVlan,
			Network:          netName,
		},
	}
	_, err := grpcClient.EndpointV1().Endpoint().Create(context.Background(), &ep)
	if err != nil {
		log.Errorf("Error creating endpoint. Err: %v", err)
		return err
	}

	// request message
	nwReq := &simapi.NwIFSetReq{
		Name:      epName,
		IPAddr:    ipAddr,
		MacAddr:   macAddr,
		Vlan:      fmt.Sprintf("%d", usegVlan),
		PortGroup: netName,
	}

	// create the hostsim container
	resp := &simapi.NwIFSetResp{}
	err = netutils.HTTPPost(hostsimURL+"/nwifs/create", nwReq, resp)
	if err != nil {
		log.Errorf("Error creating ")
	}
	return err
}

// deleteVM simulates VM deletion
func deleteVM(grpcClient apiclient.Services, hostsimURL, ipAddr, macAddr, tenant string, usegVlan uint32) error {
	epName := fmt.Sprintf("ep-%s", ipAddr)

	// endpoint key
	ometa := api.ObjectMeta{
		Name:   epName,
		Tenant: tenant,
	}

	_, err := grpcClient.EndpointV1().Endpoint().Delete(context.Background(), &ometa)
	if err != nil {
		log.Errorf("Error deleting endpoint. Err: %v", err)
	}

	name := fmt.Sprintf("%d==%s", usegVlan, macAddr)
	u1 := fmt.Sprintf("%s/nwifs/%s/delete", hostsimURL, name)
	err = netutils.HTTPPost(u1, &simapi.NwIFSetReq{}, &simapi.NwIFSetResp{})
	if err != nil {
		log.Errorf("DeleteNwIF %s, failed on host", u1)
		return err
	}

	return nil
}

func main() {
	// command line flags
	var (
		isCreate   = flag.Bool("create", false, "Create VM")
		isDelete   = flag.Bool("delete", false, "Delete VM")
		hostIf     = flag.String("hostif", "ntrunk0", "Host facing interface")
		ipAddr     = flag.String("ip", "", "VM ip address")
		macAddr    = flag.String("mac", "", "VM mac address")
		vlan       = flag.Uint("vlan", 0, "VM useg vlan")
		netName    = flag.String("net", "default", "VM Network")
		hostSimURL = flag.String("hsim", defHostsimURL, "HostSim URL")
		apisrvURL  = flag.String("apisrv", defApisrvURL, "API server URL")
	)
	flag.Parse()

	// read the mac address of the host interface
	nodeMac, err := netutils.GetIntfMac(*hostIf)
	if err != nil {
		log.Fatalf("Error getting host interface's mac addr. Err: %v", err)
	}

	// create a rest client to apigw
	l := log.GetNewLogger(log.GetDefaultConfig("SimCtl"))
	grpcClient, err := apiclient.NewGrpcAPIClient(*apisrvURL, l)
	if err != nil {
		log.Fatalf("cannot create REST client. Err: %v", err)
	}

	// create or delete based on command line
	if *isCreate {
		// verify we have required args
		if *ipAddr == "" || *macAddr == "" || *vlan == 0 {
			fmt.Printf("Insufficient args. Require ip-addr, mac-addr & vlan\n")
			os.Exit(1)
		}

		// create the endpoint in vmm/apigw
		err = createVM(grpcClient, *hostSimURL, nodeMac.String(), *ipAddr, strings.ToLower(*macAddr), *netName, "default", uint32(*vlan))
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
		err = deleteVM(grpcClient, *hostSimURL, *ipAddr, strings.ToLower(*macAddr), "default", uint32(*vlan))
		if err != nil {
			log.Errorf("Error deleting the VM: %v", err)
			os.Exit(1)
		}
	} else {
		fmt.Printf("Needs -create or -delete flag\n")
		os.Exit(1)
	}
}
