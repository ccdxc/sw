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
	"net"
	"reflect"
	"runtime"
	"time"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/orch/simapi"
	"github.com/pensando/sw/venice/utils/log"
	n "github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/vic/pkg/vsphere/simulator"
	"github.com/pensando/vic/pkg/vsphere/simulator/esx"
)

// Inventory maintains inventory info for the simulator
type Inventory struct {
	vCenters map[string]*simInfo
	servers  map[string]*serverInfo // indexed by snic mac
}

type serverInfo struct {
	simURL    string // url to access hostsim
	snicMac   string
	hSystem   *simulator.HostSystem
	workLoads map[string]*wlInfo
}

type wlInfo struct {
	wlUUID   string
	nwIFUUID string
	vm       *simulator.VirtualMachine
	vethName string
}

type simInfo struct {
	model  *simulator.Model
	server *simulator.Server
}

var vCenters map[string]*simInfo

var instCount int

// Setup sets up the simulator
func Setup() {
	vCenters = make(map[string]*simInfo)
	instCount++
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
	instCount--
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
	m.ClusterHost = 1
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
func AddSmartNIC(hostIndex int, name, mac string) error {
	hosts := esx.GetHostList()
	if len(hosts) <= hostIndex {
		return fmt.Errorf("Specified host not found, %d hosts available", len(hosts))
	}

	esx.AddPnicToHost(hosts[hostIndex], name, mac)

	return nil
}

// AddNwIF adds a nwif to the inventory
func AddNwIF(mac, portGroup string) (string, error) {
	vms := simulator.GetVMList()
	if len(vms) < 1 {
		return "", fmt.Errorf("No vms found")
	}
	return vms[0].AddVeth(mac, portGroup, "")
}

// DeleteNwIF deletes a nwif from the inventory
func DeleteNwIF(name string) error {
	vms := simulator.GetVMList()
	if len(vms) < 1 {
		return fmt.Errorf("No vms found")
	}
	return vms[0].RemoveVeth(name)
}

// PrintInventory used for dumping inventory
func PrintInventory() {
	hosts := make(map[string]*simulator.HostSystem)
	vms := simulator.GetVMList()
	log.Infof("==================================")
	for _, vm := range vms {
		vmID := vm.Reference().Value
		href := vm.Runtime.Host.Reference()
		hostID := href.Value
		log.Infof("%s  <=  %s", vmID, hostID)
		hh := simulator.Map.Get(href)
		if hh != nil {
			hs := hh.(*simulator.HostSystem)
			if hs != nil && hs.Config != nil {
				hosts[hostID] = hs
			}
		}
	}
	log.Infof("++++++++++++++++++++++++++++++++++")
	for k, e := range hosts {
		log.Infof("HostKey=== %s", k)
		for _, v := range e.Vm {
			log.Infof("VMKey: %s", v.Reference().Value)
		}
	}

	log.Infof("==================================")
}

// New returns an instance of the simulator
func New() simapi.OrchSim {
	if instCount != 0 {
		log.Fatalf("Only one instance of simulator supported")
	}

	instCount++

	return &Inventory{
		vCenters: make(map[string]*simInfo),
		servers:  make(map[string]*serverInfo),
	}
}

// Run runs the simulator
func (s *Inventory) Run(addr string, snicMacs []string, vms int) (string, error) {
	m := simulator.VPX()
	m.ClusterHost = 1
	m.Host = len(snicMacs)
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

	s.updateInv(snicMacs)
	srv := m.Service.NewServer()
	i := &simInfo{model: m, server: srv}
	s.vCenters[addr] = i
	return fmt.Sprintf("%s", srv.URL), nil
}
func getVeth(d types.BaseVirtualDevice) *types.VirtualEthernetCard {
	dKind := reflect.TypeOf(d).Elem()
	if dKind == reflect.TypeOf((*types.VirtualE1000)(nil)).Elem() {
		e1 := d.(*types.VirtualE1000)
		return &e1.VirtualEthernetCard
	}

	return nil
}

func (s *Inventory) updateInv(snicMacs []string) {
	vms := simulator.GetVMList()
	hMap := make(map[string]*simulator.HostSystem)
	for _, vm := range vms {
		href := vm.Runtime.Host.Reference()
		hh := simulator.Map.Get(href)
		if hh != nil {
			hs := hh.(*simulator.HostSystem)
			if hs != nil && hs.Config != nil && len(hs.Config.Network.Pnic) > 0 {
				hMap[href.Value] = hs
			}
		}

		if len(hMap) == len(snicMacs) {
			break
		}
	}

	hostCount := 0
	for _, e := range hMap {
		mac := snicMacs[hostCount]
		e.Config.Network.Pnic[0].Mac = mac
		s.servers[mac] = &serverInfo{
			snicMac:   mac,
			hSystem:   e,
			workLoads: make(map[string]*wlInfo),
		}
		hostCount++
	}

	// remove all pre-created vnics.
	for _, vm := range vms {
		for _, d := range vm.Config.Hardware.Device {
			veth := getVeth(d)
			if veth == nil {
				continue
			}

			unitNum := *veth.VirtualDevice.UnitNumber
			err := vm.RemoveVeth(fmt.Sprintf("Veth-%d", unitNum-7))
			if err != nil {
				log.Errorf("RemoveVeth returned %v", err)
			}
		}
	}
}

// Destroy destroys the simulator
func (s *Inventory) Destroy() {
	for _, i := range s.vCenters {
		i.server.Close()
		i.model.Remove()
	}

	instCount--
}

// SetHostURL sets the hostsim URL to reach a host
func (s *Inventory) SetHostURL(snicMac, hostURL string) error {
	srv := s.servers[snicMac]
	if srv == nil {
		return fmt.Errorf("No host with snic %s", snicMac)
	}

	srv.simURL = hostURL
	r := &simapi.NwIFSetReq{}
	resp := &simapi.NwIFSetResp{}
	n.HTTPPost(hostURL+"/nwifs/cleanup", r, resp)
	return nil
}

// CreateNwIF creates a vnic in the simulator
func (s *Inventory) CreateNwIF(hostURL string, r *simapi.NwIFSetReq) (*simapi.NwIFSetResp, error) {
	// find a matching host
	for _, srv := range s.servers {
		if srv.simURL == hostURL {
			if r.IPAddr == "" {
				return nil, fmt.Errorf("IP address is required")
			}

			// if mac address is empty, generate from IP addr
			if r.MacAddr == "" {
				r.MacAddr = macFromIP(r.IPAddr)
			}

			vmRef := srv.hSystem.Vm[0].Reference()
			vv := simulator.Map.Get(vmRef)
			vm := vv.(*simulator.VirtualMachine)
			name, err := vm.AddVeth(r.MacAddr, r.PortGroup, r.Vlan)
			if err != nil {
				return nil, err
			}

			resp := &simapi.NwIFSetResp{}
			err = n.HTTPPost(hostURL+"/nwifs/create", r, resp)
			if err != nil {
				vm.RemoveVeth(name)
			}
			wl := &wlInfo{
				wlUUID:   resp.WlUUID,
				nwIFUUID: resp.UUID,
				vm:       vm,
				vethName: name,
			}

			srv.workLoads[resp.UUID] = wl

			return resp, err
		}
	}
	return nil, fmt.Errorf("No server with %s url", hostURL)
}

// DeleteNwIF not yet
func (s *Inventory) DeleteNwIF(hostURL string, id string) *simapi.NwIFDelResp {
	// find a matching host
	for _, srv := range s.servers {
		if srv.simURL == hostURL {
			wl := srv.workLoads[id]
			if wl == nil {
				log.Errorf("DeleteNwIF %s, %s not found", hostURL, id)
				return nil
			}

			r := &simapi.NwIFSetReq{}
			resp := &simapi.NwIFDelResp{}
			u1 := fmt.Sprintf("%s/nwifs/%s/delete", hostURL, id)
			err := n.HTTPPost(u1, r, resp)
			if err != nil {
				log.Errorf("DeleteNwIF %s, %s failed on host", hostURL, id)
				return nil
			}

			wl.vm.RemoveVeth(wl.vethName)
			delete(srv.workLoads, id)
			return resp
		}
	}
	return nil
}

func macFromIP(IP string) string {
	ipAddr := net.ParseIP(IP)
	return fmt.Sprintf("00:50:%02x:%02x:%02x:%02x", ipAddr[12], ipAddr[13], ipAddr[14], ipAddr[15])
}
