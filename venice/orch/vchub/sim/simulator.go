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

	"github.com/pensando/vic/pkg/vsphere/simulator"
	"github.com/pensando/vic/pkg/vsphere/simulator/esx"

	"github.com/pensando/sw/venice/orch/simapi"
	"github.com/pensando/sw/venice/utils/log"
	n "github.com/pensando/sw/venice/utils/netutils"
)

// Inventory maintains inventory info for the simulator
type Inventory struct {
	model    *simulator.Model
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

	simulator.IncrGlobalVersion()
	time.Sleep(250 * time.Millisecond)
	simulator.IncrGlobalVersion()
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

	simulator.AddPnicToHost(hosts[hostIndex], name, mac)

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
	log.Infof("==================================\n")
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
	log.Infof("++++++++++++++++++++++++++++++++++\n")
	for k, e := range hosts {
		log.Infof("HostKey=== %s", k)
		for _, v := range e.Vm {
			log.Infof("VMKey: %s", v.Reference().Value)
		}
	}

	log.Infof("==================================\n")
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
	s.model = m
	return fmt.Sprintf("%s", srv.URL), nil
}

// AddHost can be used to test adding hosts
func (s *Inventory) AddHost(vcAddr, simURL, mac string) {
	m := s.vCenters[vcAddr].model
	h, _ := m.AddHost(simURL, mac)
	s.servers[mac] = &serverInfo{
		simURL:    simURL,
		snicMac:   mac,
		hSystem:   h,
		workLoads: make(map[string]*wlInfo),
	}
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
	simulator.IncrGlobalVersion()
	time.Sleep(250 * time.Millisecond)
	simulator.IncrGlobalVersion()
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
func (s *Inventory) CreateNwIF(snicMac string, r *simapi.NwIFSetReq) (*simapi.NwIFSetResp, error) {
	// find a matching host
	srv := s.servers[snicMac]
	if srv == nil {
		return nil, fmt.Errorf("snic not found")
	}

	if r.IPAddr == "" {
		return nil, fmt.Errorf("IP address is required")
	}

	hostURL := srv.simURL
	// if mac address is empty, generate from IP addr
	if r.MacAddr == "" {
		r.MacAddr = macFromIP(r.IPAddr)
	}

	vm, err := s.model.AddVM(srv.hSystem, r.WLName)
	if err != nil {
		return nil, err
	}
	name, err := vm.AddVeth(r.MacAddr, r.PortGroup, r.Vlan)
	if err != nil {
		return nil, err
	}

	resp := &simapi.NwIFSetResp{}
	if hostURL != "" {
		err = n.HTTPPost(hostURL+"/nwifs/create", r, resp)
		if err != nil {
			vm.RemoveVeth(name)
			vm.Destroy()
			return nil, err
		}
	} else {
		resp.UUID = r.MacAddr + r.Vlan
		resp.MacAddr = r.MacAddr
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

// DeleteNwIF deletes a nwif
func (s *Inventory) DeleteNwIF(snicMac string, id string) *simapi.NwIFDelResp {
	// find a matching host
	srv := s.servers[snicMac]
	if srv == nil {
		return nil
	}

	wl := srv.workLoads[id]
	if wl == nil {
		log.Errorf("DeleteNwIF %s, %s not found", snicMac, id)
		return nil
	}
	hostURL := srv.simURL

	resp := &simapi.NwIFDelResp{}
	if hostURL != "" {
		r := &simapi.NwIFSetReq{}
		u1 := fmt.Sprintf("%s/nwifs/%s/delete", hostURL, id)
		err := n.HTTPPost(u1, r, resp)
		if err != nil {
			log.Errorf("DeleteNwIF %s, %s failed on host", hostURL, id)
			return nil
		}
	}

	wl.vm.RemoveVeth(wl.vethName)
	time.Sleep(300 * time.Millisecond)
	wl.vm.Destroy()
	delete(srv.workLoads, id)
	return resp
}

func macFromIP(IP string) string {
	ipAddr := net.ParseIP(IP)
	return fmt.Sprintf("00:50:%02x:%02x:%02x:%02x", ipAddr[12], ipAddr[13], ipAddr[14], ipAddr[15])
}
