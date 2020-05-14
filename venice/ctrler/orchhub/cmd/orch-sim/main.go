package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"net/url"
	"os"
	"sort"

	simulator "github.com/vmware/govmomi/simulator"
	"github.com/vmware/govmomi/vim25/types"

	"encoding/json"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
)

// SimParams are the parameters passed to simulator
type SimParams struct {
	// Instances are the number of simulators to be started
	Instances int
	// NamespacesPerOrch count of number of namespaces per orchestrator
	NamespacesPerOrch int
	// HostPerNamespace hosts per Namespace
	HostPerNamespace int
	// WorkloadsPerHost workloads per host
	WorkloadsPerHost int
	// InterfacesPerWorkload interfaces to be created in each workload
	InterfacesPerWorkload int
	// NetworkName is the name of venice network
	NetworkName string
	// ConfigPath is the path where IOTA generated config file is present
	ConfigPath string
	// Port on which the simulator must be started
	Port int
}

var (
	params        SimParams
	topology      map[string]Orchestrator
	allInterfaces map[string]Host
	networks      map[int32]Network
)

const (
	// PensandoOUI is pensando oui
	PensandoOUI = "00ae.cd"
	// RandomOUI is random oui for workload interfaces
	RandomOUI     = "00be.ef"
	orchBase      = "orchestrator"
	hostBase      = "host"
	workloadBase  = "workload"
	namespaceBase = "dc"
)

// Network is the network object for orch-sim
type Network struct {
	Name   string
	VlanID int32
}

// Interface struct for VNIC information
type Interface struct {
	Host       string
	Workload   string
	MacAddress string
	IPAddress  string
	PGName     string
}

// Workload holds information about each simulated workload
type Workload struct {
	Name       string
	Host       string
	Interfaces []Interface
	VM         *simulator.VirtualMachine
}

// Host holds information about each simulated host
type Host struct {
	Name      string
	Namespace string
	Workloads map[string]Workload
	Mac       string
	SimHost   *sim.Host
}

// Namespace holds information about each Datacenter
type Namespace struct {
	Name  string
	Hosts map[string]Host
	DC    *sim.Datacenter
	DVS   *sim.DVS
}

// Orchestrator is each simulator instance
type Orchestrator struct {
	Name       string
	Namespaces map[string]Namespace
	URI        string
	User       string
	Password   string
	Sim        *sim.VcSim
}

// Destroy is used to shut down all simulators
func Destroy() {
	if topology == nil {
		fmt.Printf("\ntopology not set")
		return
	}

	for _, o := range topology {
		if o.Sim != nil {
			o.Sim.Destroy()
		}
	}
}

func createNamespace(name string, orch *Orchestrator) (*Namespace, error) {
	namespace := Namespace{}
	namespace.Name = name
	namespace.Hosts = make(map[string]Host)
	dc, err := orch.Sim.AddDC(namespace.Name)
	if err != nil {
		fmt.Printf("\nFailed to add DC %v to simulator. Err : %v", namespace.Name, err)
		return nil, fmt.Errorf("Failed to add DC %v to simulator. Err : %v", namespace.Name, err)
	}

	var dvsCreateSpec types.DVSCreateSpec
	var dvsConfigSpec types.DVSConfigSpec

	dvsConfigSpec.GetDVSConfigSpec().Name = vchub.CreateDVSName(namespace.Name)
	dvsConfigSpec.GetDVSConfigSpec().NumStandalonePorts = 512
	dvsConfigSpec.GetDVSConfigSpec().MaxPorts = 4096
	dvsCreateSpec.ConfigSpec = &types.VMwareDVSConfigSpec{
		DVSConfigSpec: dvsConfigSpec,
	}

	_, err = dc.AddDVS(&dvsCreateSpec)
	if err != nil {
		fmt.Printf("\nFailed to add DVS %v to DC. Err : %v", dvsConfigSpec.GetDVSConfigSpec().Name, err)
		return nil, fmt.Errorf("Failed to add DVS %v to DC. Err : %v", dvsConfigSpec.GetDVSConfigSpec().Name, err)
	}

	dvs, _ := dc.GetDVS(vchub.CreateDVSName(namespace.Name))
	if dvs == nil {
		fmt.Printf("\nFailed to retrieve DVS from DC")
		return nil, fmt.Errorf("Failed to retrieve DVS %v from DC", vchub.CreateDVSName(namespace.Name))
	}

	pgConfigSpec := []types.DVPortgroupConfigSpec{}
	for _, nw := range networks {
		pgConfigSpec = append(pgConfigSpec, types.DVPortgroupConfigSpec{
			Name:     vchub.CreatePGName(nw.Name),
			Type:     string(types.DistributedVirtualPortgroupPortgroupTypeEarlyBinding),
			NumPorts: 100,
			DefaultPortConfig: &types.VMwareDVSPortSetting{
				Vlan: &types.VmwareDistributedVirtualSwitchPvlanSpec{
					PvlanId: nw.VlanID,
				},
			},
		})
	}

	_, err = dvs.AddPortgroup(pgConfigSpec)
	if err != nil {
		return nil, err
	}

	namespace.DC = dc
	namespace.DVS = dvs

	return &namespace, nil
}

func createHost(hostName, hostMAC string, namespace *Namespace) (*Host, error) {
	host := Host{}
	host.Name = hostName
	host.Namespace = namespace.Name
	host.Workloads = make(map[string]Workload)
	host.Mac = hostMAC
	h, err := namespace.DC.AddHost(host.Name)
	if err != nil {
		fmt.Printf("\nFailed to add host %v to DC. Err : %v", host.Name, err)
		return nil, fmt.Errorf("Failed to add host %v to DC. Err : %v", host.Name, err)
	}

	h.ClearNics()
	h.ClearVmkNics()
	err = h.AddNic("vmnic0", host.Mac)
	if err != nil {
		fmt.Printf("\nFailed to add NIC %v to host. Err : %v", host.Mac, err)
		return nil, fmt.Errorf("Failed to add NIC %v to host. Err : %v", host.Mac, err)
	}

	err = namespace.DVS.AddHost(h)
	if err != nil {
		fmt.Printf("\nfailed to add host %v to DVS. Err : %v", host.Name, err)
	}

	host.SimHost = h
	return &host, nil
}

func createWorkload(workloadName string, host *Host, interfaces []Interface, namespace *Namespace) (*Workload, error) {
	workload := Workload{}
	workload.Name = workloadName
	workload.Host = host.Name
	vnics := []sim.VNIC{}

	for _, ifc := range interfaces {
		pg, ok := namespace.DVS.GetPortgroup(ifc.PGName)
		if !ok {
			return nil, fmt.Errorf("failed to get PG, %v", vchub.CreatePGName(params.NetworkName))
		}

		vnics = append(vnics, sim.VNIC{MacAddress: ifc.MacAddress,
			PortgroupKey: pg.Obj.Reference().Value,
			PortKey:      "11"})
	}
	vm, err := namespace.DC.AddVM(workload.Name, workload.Host, vnics)
	if err != nil {
		fmt.Printf("\nFailed to add VM %v to DC. Err : %v", workload.Name, err)
		return nil, fmt.Errorf("Failed to add VM %v to DC. Err : %v", workload.Name, err)
	}

	workload.Interfaces = interfaces
	workload.VM = vm
	return &workload, nil
}

func createOrchestrator(orchName, URI string) (*Orchestrator, error) {
	orch := Orchestrator{}
	orch.Name = orchName
	orch.Namespaces = make(map[string]Namespace)
	orch.URI = URI
	orch.User = "user"
	orch.Password = "password"

	// Create simulator
	u := &url.URL{
		//Scheme: "https",
		Scheme: "http",
		Host:   orch.URI,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(orch.User, orch.Password)
	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	if err != nil {
		fmt.Printf("\nFailed to start simulator : %v", err)
		return nil, fmt.Errorf("failed to start simulator : %v", err)
	}
	orch.Sim = s

	return &orch, nil
}

func generateTopology() error {
	fmt.Println("Generating Topology")
	topology = make(map[string]Orchestrator)
	allInterfaces = make(map[string]Host)
	macSeed := 1000
	ifSeed := 1000

	for i := 0; i < params.Instances; i++ {
		orchName := fmt.Sprintf("%v-%d", params.Port, i)
		orchURI := fmt.Sprintf(":%d", params.Port+i)

		orch, err := createOrchestrator(orchName, orchURI)
		if err != nil {
			return nil
		}

		for j := 0; j < params.NamespacesPerOrch; j++ {
			name := fmt.Sprintf("%v-%v-%d", orch.Name, namespaceBase, j)
			namespace, err := createNamespace(name, orch)
			if err != nil {
				return err
			}

			for k := 0; k < params.HostPerNamespace; k++ {
				hostName := fmt.Sprintf("%v-%v-%d", namespace.Name, hostBase, k)
				hostMac := fmt.Sprintf("%vff.%04x", PensandoOUI, macSeed)
				macSeed++
				host, err := createHost(hostName, hostMac, namespace)
				if err != nil {
					return nil
				}

				for l := 0; l < params.WorkloadsPerHost; l++ {
					workloadName := fmt.Sprintf("%v-%v-%d", host.Name, workloadBase, l)
					interfaces := []Interface{}
					for m := 0; m < params.InterfacesPerWorkload; m++ {
						ifc := Interface{}
						ifc.MacAddress = fmt.Sprintf("%vff.%04x", RandomOUI, ifSeed+m)
						ifc.Host = host.Name
						ifc.Workload = workloadName
						ifc.PGName = vchub.CreatePGName(params.NetworkName)
						interfaces = append(interfaces, ifc)
					}

					workload, err := createWorkload(workloadName, host, interfaces, namespace)
					if err != nil {
						return err
					}
					ifSeed = ifSeed + params.InterfacesPerWorkload

					host.Workloads[workload.Name] = *workload
				}

				allInterfaces[host.Mac] = *host
				namespace.Hosts[host.Name] = *host
			}

			orch.Namespaces[namespace.Name] = *namespace
		}

		topology[orch.Name] = *orch
	}
	fmt.Println("Topology Generated")
	return nil
}

func generateTopologyUsingConfig() error {
	topology = make(map[string]Orchestrator)
	allInterfaces = make(map[string]Host)

	fmt.Printf("\nGenerating Topology using config file %v", params.ConfigPath)
	jsonFile, err := os.Open(params.ConfigPath)
	if err != nil {
		return err
	}

	defer jsonFile.Close()

	orchName := fmt.Sprintf("vcsim-%v", params.Port)
	orchURI := fmt.Sprintf(":%d", params.Port)

	orch, err := createOrchestrator(orchName, orchURI)
	if err != nil {
		return err
	}

	byteValue, _ := ioutil.ReadAll(jsonFile)
	var result map[string]interface{}
	json.Unmarshal([]byte(byteValue), &result)

	nws := result["Networks"].([]interface{})
	for _, nw := range nws {
		networkName := nw.(map[string]interface{})["meta"].(map[string]interface{})["name"].(string)
		vlanID := int32(nw.(map[string]interface{})["spec"].(map[string]interface{})["vlan-id"].(float64))
		networks[vlanID] = Network{Name: networkName, VlanID: vlanID}
	}

	name := fmt.Sprintf("%v-%v", orch.Name, namespaceBase)
	namespace, err := createNamespace(name, orch)
	if err != nil {
		return err
	}

	fmt.Println(namespace)
	hosts := result["Hosts"].([]interface{})
	for _, host := range hosts {
		hostName := host.(map[string]interface{})["meta"].(map[string]interface{})["name"].(string)
		hostMac := host.(map[string]interface{})["spec"].(map[string]interface{})["dscs"].([]interface{})[0].(map[string]interface{})["mac-address"].(string)
		h, err := createHost(hostName, hostMac, namespace)
		if err != nil {
			return nil
		}
		namespace.Hosts[hostName] = *h
		allInterfaces[hostMac] = *h
	}

	workloads := result["Workloads"].([]interface{})
	for _, workload := range workloads {
		workloadName := workload.(map[string]interface{})["meta"].(map[string]interface{})["name"].(string)
		workloadSpec := workload.(map[string]interface{})["spec"].(map[string]interface{})
		hostName := workloadSpec["host-name"].(string)
		fmt.Printf("\nWorkload Name : %v", workloadName)
		fmt.Printf("\nHost Name : %v\n", hostName)
		ifs := workloadSpec["interfaces"].([]interface{})
		interfaces := []Interface{}
		for _, i := range ifs {
			ifc := Interface{}
			ifc.Workload = workloadName
			ifc.Host = hostName
			ifc.MacAddress = i.(map[string]interface{})["mac-address"].(string)
			vlanID := int32(i.(map[string]interface{})["external-vlan"].(float64))
			ifc.PGName = vchub.CreatePGName(networks[vlanID].Name)
			interfaces = append(interfaces, ifc)

			host := namespace.Hosts[hostName]
			w, err := createWorkload(workloadName, &host, interfaces, namespace)
			if err != nil {
				return err
			}
			host.Workloads[w.Name] = *w
		}
	}

	orch.Namespaces[namespace.Name] = *namespace
	topology[orch.Name] = *orch

	return nil
}

func printTopology() {
	for orchKey, orchValue := range topology {
		fmt.Println("\n======================================================================================")
		fmt.Printf("\nORCHESTRATOR : %v URL : %v", orchKey, orchValue.URI)
		for namespaceKey, namespaceValue := range orchValue.Namespaces {
			fmt.Printf("\nNAMESPACE : %v", namespaceKey)
			for hostKey, hostValue := range namespaceValue.Hosts {
				fmt.Printf("\n\tHOST : %v\t\tMac : %v", hostKey, hostValue.Mac)

				for workloadKey, workloadValue := range hostValue.Workloads {
					fmt.Printf("\n\t\t WORKLOAD : %v", workloadKey)
					fmt.Printf("\n\t\t\t IFS : %v", workloadValue.Interfaces)
				}
			}
			fmt.Println("\n======================================================================================")
		}
		fmt.Printf("\n\n")
	}
	fmt.Printf("\nEnsure networks : %v are created on Venice\n", networks)
}

func listAllPensandoInterfaces() {
	keyList := []string{}

	for k := range allInterfaces {
		keyList = append(keyList, k)
	}

	sort.Strings(keyList)

	for _, k := range keyList {
		fmt.Printf("\n%v  \tHOST : %v", k, allInterfaces[k].Name)
	}
}

func validateParams() error {
	if len(params.ConfigPath) > 0 {
		fmt.Println("All scale parameters other then port and network name will be ignored as IOTA configuration is passed")
		return nil
	}

	if params.Instances < 1 || params.NamespacesPerOrch < 1 || params.HostPerNamespace < 1 || params.WorkloadsPerHost < 1 || params.InterfacesPerWorkload < 1 {
		return fmt.Errorf("incorrect value passed as parameters. All values must be >= 1")
	}

	if len(params.NetworkName) == 0 {
		return fmt.Errorf("valid network name should be passed")
	}

	return nil
}

func main() {
	defer Destroy()

	instPtr := flag.Int("orch", 1, "instances of compute orchestrator simulator to be created")
	// TODO : fix multiple namespaces in sim issue
	// Currently we do a map of all port groups within a simulator instance
	// The port groups across various namespaces will have the same name eg. #Pen-PG-Network-Vlan-1
	// This leads to out of range panics in sim
	//namespacePtr := flag.Int("ns", 1, "namespaces per orchestrator")
	hostPtr := flag.Int("h", 1, "hosts per namespace")
	workloadPtr := flag.Int("w", 1, "workloads per host")
	ifcPtr := flag.Int("i", 1, "interfaces per workload")
	printTopoPtr := flag.Bool("p", false, "print topology")
	printInterfacesPtr := flag.Bool("l", false, "print interface list")
	flag.StringVar(&params.NetworkName, "n", "Network-Vlan-1", "Venice Network Name")
	portPtr := flag.Int("port", 20000, "port to start vcsim on")
	flag.StringVar(&params.ConfigPath, "c", "", "IOTA generated config file")

	flag.Parse()

	params.Instances = *instPtr
	// TODO : Add support for multiple namespace per orchestrator
	//params.NamespacesPerOrch = *namespacePtr
	params.NamespacesPerOrch = 1
	params.HostPerNamespace = *hostPtr
	params.WorkloadsPerHost = *workloadPtr
	params.InterfacesPerWorkload = *ifcPtr
	params.Port = *portPtr

	if err := validateParams(); err != nil {
		fmt.Printf("\nParameter validation failed. Err : %v", err)
		return
	}
	networks = make(map[int32]Network)

	if len(params.ConfigPath) > 0 {
		if err := generateTopologyUsingConfig(); err != nil {
			fmt.Printf("\nTopology generation failed. Err : %v", err)
		}
	} else {
		networks[1] = Network{Name: params.NetworkName, VlanID: 1}
		if err := generateTopology(); err != nil {
			fmt.Printf("\nTopology generation failed. Err : %v", err)
			return
		}
	}

	if *printTopoPtr {
		printTopology()
	}

	if *printInterfacesPtr {
		listAllPensandoInterfaces()
	}

	fmt.Printf("Orchestrators running on port : %v", params.Port)
	select {}
}
