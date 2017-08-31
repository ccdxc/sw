package main

import (
	"encoding/json"
	"fmt"
	"net"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"

	log "github.com/Sirupsen/logrus"
	"github.com/vishvananda/netlink"
)

const (
	nwInfoFile = "/tmp/.simNwInfo"
	simBridge  = "SimBridge"
)

// NwInfo keeps track of nw info
type NwInfo struct {
	Count uint
	Eps   map[string]*EpInfo
}

// EpInfo defines EpInfo
type EpInfo struct {
	ContainerID  string
	ContainerPid string
	OvsPort      string
	Vlan         string
	IP           string
	Mac          string
}

var vsctlPath string
var ipPath string
var nsenterPath string
var nwInfo NwInfo
var fH *os.File

// AddNwIF adds a nwIF with the specified parameters
func AddNwIF(mac, ip, vlan string) (string, string, error) {
	if ip == "" || vlan == "" {
		return "", "", fmt.Errorf("IPAddr and Vlan are required")
	}

	if mac == "" {
		mac = macFromIP(ip)
	}

	name := vlan + "==" + mac
	ep := nwInfo.Eps[name]
	if ep != nil {
		return "", "", fmt.Errorf("Already exists: %+v", ep)
	}

	ep = &EpInfo{
		Vlan: vlan,
		IP:   ip,
		Mac:  mac,
	}

	err := createEP(ep)
	if err != nil {
		return "", "", err
	}

	nwInfo.Eps[name] = ep
	return name, mac, nil
}

func saveToFile() {
	buf, err := json.Marshal(&nwInfo)
	if err != nil {
		log.Errorf("Marshal error %v", err)
		return
	}

	fH.Seek(0, 0)
	fH.Truncate(0)
	out, err := fH.Write(buf)
	if err != nil {
		log.Errorf("Error overwriting file %v -- %s", err, string(out))
	}

	fH.Sync()
}

// DeleteNwIF deletes the nwIF
func DeleteNwIF(name string) error {
	deleteEP(name)
	return nil
}

// CleanUp cleans up all veths and containers
func CleanUp() string {
	cMap := GetAllContainers()
	pMap := getAllOvsPorts()

	cCount := len(cMap)
	pCount := len(pMap)
	for c := range cMap {
		RemoveContainer(c)
	}
	for p := range pMap {
		if p != *uplinkIf {
			removePorts(p)
		}
	}
	nwInfo = NwInfo{
		Eps: make(map[string]*EpInfo),
	}

	return fmt.Sprintf("%d containers and %d ports cleaned up", cCount, pCount)
}

func createBridge() {

	needBridge := true
	out, err := exec.Command(vsctlPath, "list-br").CombinedOutput()
	if err == nil {
		lines := strings.Split(string(out), "\n")
		for _, l := range lines {
			if strings.Contains(l, simBridge) {
				log.Infof("Bridge %s exists", simBridge)
				needBridge = false
			}
		}
	}

	if needBridge {
		out, err = exec.Command(vsctlPath, "add-br", simBridge).CombinedOutput()
		if err != nil {
			log.Fatalf("Error %v creating bridge %s", err, string(out))
		}
	}

	out, err = exec.Command(vsctlPath, "list-ports", simBridge).CombinedOutput()
	if err == nil {
		lines := strings.Split(string(out), "\n")
		for _, l := range lines {
			if strings.Contains(l, *uplinkIf) {
				log.Infof("%s already added", *uplinkIf)
				return
			}
		}
	}

	out, err = exec.Command(vsctlPath, "add-port", simBridge, *uplinkIf).CombinedOutput()
	if err != nil {
		log.Fatalf("Error %v creating bridge %s", err, string(out))
	}
}

func removePorts(pName string) {
	out, err := exec.Command(vsctlPath, "del-port", simBridge, pName).CombinedOutput()
	if err != nil {
		log.Errorf("Error %v deleteing port %s", err, string(out))
	}
	//out, err = exec.Command(ipPath, "link", "del", pName, "type", "veth").CombinedOutput()
	//if err != nil {
	//	log.Errorf("Failed to delete %s - err %v, %s", pName, err, string(out))
	//}
}

// createVethPair creates veth interface pairs with specified name
func createVethPair(name1, name2 string) error {
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

func createPorts(pid string, ep *EpInfo) error {
	nwInfo.Count++
	ovsport := fmt.Sprintf("ovsport%d", nwInfo.Count)
	peer := fmt.Sprintf("endpoint%d", nwInfo.Count)
	ep.OvsPort = ovsport

	err := createVethPair(ovsport, peer)
	if err != nil {
		log.Errorf("Failed to create %s - err %v", ovsport, err)
	}

	nsPid, err := strconv.Atoi(pid)
	if err != nil {
		log.Errorf("Atoi error %v", err)
	}

	err = moveToNS(nsPid, peer)
	if err != nil {
		log.Errorf("Failed to move to container ns - %v", err)
		log.Errorf("nsPid: %d pid: %s", nsPid, pid)
		return err
	}

	// set IP and Mac
	if ep.Mac == "" {
		ep.Mac = macFromIP(ep.IP)
	}

	out, err := exec.Command(nsenterPath, "-t", pid, "-n", "-F", "--", ipPath,
		"link", "set", "dev", peer, "address", ep.Mac).CombinedOutput()
	if err != nil {
		log.Errorf("Failed to set mac address - %v, %s", err, string(out))
		return err
	}

	out, err = exec.Command(nsenterPath, "-t", pid, "-n", "-F", "--", ipPath,
		"address", "add", ep.IP, "dev", peer).CombinedOutput()
	if err != nil {
		log.Errorf("Failed to set ip address - %v, %s", err, string(out))
		return err
	}

	// mark the links up
	out, err = exec.Command(nsenterPath, "-t", pid, "-n", "-F", "--", ipPath,
		"link", "set", "dev", peer, "up").CombinedOutput()
	if err != nil {
		log.Errorf("Failed to mark link up - %v, %s", err, string(out))
		return err
	}

	out, err = exec.Command(vsctlPath, "add-port", simBridge, ovsport).CombinedOutput()
	if err != nil {
		log.Errorf("Failed to port to ovs - %v, %s", err, string(out))
		return err
	}
	tag := fmt.Sprintf("tag=%s", ep.Vlan)
	out, err = exec.Command(vsctlPath, "set", "port", ovsport, tag).CombinedOutput()
	if err != nil {
		log.Errorf("Failed to set vlan - %v, %s", err, string(out))
		return err
	}
	out, err = exec.Command(ipPath, "link", "set", "dev", ovsport, "up").CombinedOutput()
	if err != nil {
		log.Errorf("Failed to mark ovs link up - %v, %s", err, string(out))
		return err
	}

	return nil
}

func deleteEP(name string) {
	v := nwInfo.Eps[name]
	if v == nil {
		return
	}

	RemoveContainer(v.ContainerID)
	removePorts(v.OvsPort)
	delete(nwInfo.Eps, name)
}

func createEP(ep *EpInfo) error {

	c, pid, err := RunContainer()
	if err != nil {
		log.Errorf("Failed to create container")
		return err
	}

	ep.ContainerID = c
	ep.ContainerPid = pid
	return createPorts(pid, ep)
}

func syncPorts() {
	cMap := GetAllContainers()
	pMap := getAllOvsPorts()
	for name, v := range nwInfo.Eps {
		cOK := cMap[v.ContainerID]
		if !cOK {
			v.ContainerID = ""
		}

		pOK := pMap[v.OvsPort]
		if !cOK || !pOK {
			deleteEP(name)
			createEP(v)
		}
	}
}

func getAllOvsPorts() map[string]bool {
	res := make(map[string]bool)
	out, err := exec.Command(vsctlPath, "list-ports", simBridge).CombinedOutput()

	if err != nil {
		return res
	}

	lines := strings.Split(string(out), "\n")
	for _, port := range lines {
		if port != "" {
			res[port] = true
		}
	}

	return res
}

func moveToNS(pid int, ifname string) error {
	// find the link
	link, err := getLink(ifname)
	if err != nil {
		log.Errorf("unable to find link %q. Error %q", ifname, err)
		return err
	}

	// move to the desired netns
	err = netlink.LinkSetNsPid(link, pid)
	if err != nil {
		log.Errorf("unable to move interface %s to pid %d. Error: %s",
			ifname, pid, err)
		return err
	}

	return nil
}

// getLink is a wrapper that fetches the netlink corresponding to the ifname
func getLink(ifname string) (netlink.Link, error) {
	// find the link
	link, err := netlink.LinkByName(ifname)
	if err != nil {
		if !strings.Contains(err.Error(), "Link not found") {
			log.Errorf("unable to find link %q. Error: %q", ifname, err)
			return link, err
		}
		// try once more as sometimes (somehow) link creation is taking
		// sometime, causing link not found error
		time.Sleep(1 * time.Second)
		link, err = netlink.LinkByName(ifname)
		if err != nil {
			log.Errorf("unable to find link %q. Error %q", ifname, err)
		}
		return link, err
	}
	return link, err
}

func macFromIP(IP string) string {
	ipAddr := net.ParseIP(IP)
	return fmt.Sprintf("00:50:%02x:%02x:%02x:%02x", ipAddr[12], ipAddr[13], ipAddr[14], ipAddr[15])
}

func initExecPaths() {
	var err error

	vsctlPath, err = exec.LookPath("ovs-vsctl")
	if err != nil {
		log.Fatalf("Error %v finding ovs-vsctl", err)
	}
	ipPath, err = exec.LookPath("ip")
	if err != nil {
		log.Fatalf("Error %v finding ip path", err)
	}
	nsenterPath, err = exec.LookPath("nsenter")
	if err != nil {
		log.Fatalf("Error %v finding nsenter path", err)
	}
}

// InitNetworking perform some initialization
func InitNetworking() {

	initExecPaths()

	nwInfo.Eps = make(map[string]*EpInfo)
	fHandle, err := os.OpenFile(nwInfoFile, os.O_RDWR|os.O_CREATE, 0644)
	if err != nil {
		log.Fatalf("Unable to create %s. err -- %v", nwInfoFile, err)
	}

	fH = fHandle

	fInfo, err := fH.Stat()
	if err == nil && fInfo.Size() != 0 {
		buf := make([]byte, fInfo.Size())
		_, err = fH.Read(buf)
		if err == nil {
			json.Unmarshal(buf, &nwInfo)
		}
	}

	createBridge()

	syncPorts()
}
