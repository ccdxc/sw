/*
 *
 * Copyright 2017, Pensando Systems
 * All rights reserved.
 *
 */

package sim

import (
	"flag"
	"fmt"
	"runtime"
	"time"

	"github.com/pensando/vic/pkg/vsphere/simulator"
	"github.com/pensando/vic/pkg/vsphere/simulator/esx"
)

type simInfo struct {
	model  *simulator.Model
	server *simulator.Server
}

var vCenters map[string]*simInfo

// Setup sets up the simulator
func Setup() {
	vCenters = make(map[string]*simInfo)
}

// TearDown cleans up the simulator
func TearDown() {
	if vCenters == nil {
		return
	}

	for _, i := range vCenters {
		i.server.Close()
		i.model.Remove()
	}

	vCenters = nil
}

// Simulate simulates a vCenter with the specified IP:port, hosts and VMs
// To modify inventory, just call it again with the same IP:port
// Returns url of the simulated VC
func Simulate(addr string, hosts, vms int) (string, error) {
	i := vCenters[addr]
	if i != nil {
		i.server.Close()
		i.model.Remove()
		time.Sleep(time.Second)
	}

	m := simulator.VPX()
	m.ClusterHost = hosts
	m.Host = hosts
	m.Machine = vms

	f := flag.Lookup("httptest.serve")
	f.Value.Set(addr)
	tag := " (govmomi simulator)"
	m.ServiceContent.About.Name += tag
	m.ServiceContent.About.OsType = runtime.GOOS + "-" + runtime.GOARCH

	esx.HostSystem.Summary.Hardware.Vendor += tag

	err := m.Create()
	if err != nil {
		return "", err
	}

	s := m.Service.NewServer()
	i = &simInfo{model: m, server: s}
	vCenters[addr] = i
	return fmt.Sprintf("%s", s.URL), nil
}

// AddSmartNIC adds a smartnic to the inventory
func AddSmartNIC(name, mac string) error {
	hosts := esx.GetHostList()
	if len(hosts) == 0 {
		return fmt.Errorf("No hosts found")
	}

	esx.AddPnicToHost(hosts[0], name, mac)

	return nil
}

// AddNwIF adds a nwif to the inventory
func AddNwIF(mac, portGroup string) (string, error) {
	vms := simulator.GetVMList()
	if len(vms) < 1 {
		return "", fmt.Errorf("No vms found")
	}
	return vms[0].AddVeth(mac, portGroup)
}

// DeleteNwIF deletes a nwif from the inventory
func DeleteNwIF(name string) error {
	vms := simulator.GetVMList()
	if len(vms) < 1 {
		return fmt.Errorf("No vms found")
	}
	return vms[0].RemoveVeth(name)
}
