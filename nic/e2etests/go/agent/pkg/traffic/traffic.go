package traffic

import (
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"os/signal"
	"path/filepath"
	"strconv"
	"strings"
	"syscall"
	"time"

	"github.com/pkg/errors"

	yaml "gopkg.in/yaml.v2"

	Pkg "github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	_ "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests/modules"
	Common "github.com/pensando/sw/nic/e2etests/go/common"
	Infra "github.com/pensando/sw/nic/e2etests/go/infra"

	"github.com/pensando/sw/nic/agent/protos/netproto"
)

var (
	testEnvDir = fmt.Sprintf("%s/src/github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests/env", os.Getenv("GOPATH"))
)

func lastIPv4Addr(n *net.IPNet) net.IP {
	ip := make(net.IP, len(n.IP.To4()))
	binary.BigEndian.PutUint32(ip, binary.BigEndian.Uint32(n.IP.To4())|^binary.BigEndian.Uint32(net.IP(n.Mask).To4()))
	ip[3]--
	return ip
}

func defaultRoute(nw string) string {
	_, sip, _ := net.ParseCIDR(nw)
	return lastIPv4Addr(sip).String()
}

func getNetworkFromConfig(nwName string, networks []netproto.Network) *netproto.Network {
	for _, network := range networks {
		if network.GetName() == nwName {
			return &network
		}
	}
	return nil
}

var hpingRun = func(ep *Infra.Endpoint, cmd []string) error {
	if _, err := ep.AppEngine.RunCommand(cmd, 0, false); err != nil {
		return errors.Wrap(err, "Traffic generation failed")
	}
	return nil
}

func newEpFromAgentConfig(ep *netproto.Endpoint, nw *netproto.Network, intf string) *Infra.Endpoint {
	infraEp := &Infra.Endpoint{
		Name: ep.GetName(),
	}

	var vlan int
	if infraEp.Remote {
		vlan = int(nw.Spec.GetVlanID())
	} else {
		vlan = int(ep.Spec.GetUsegVlan())
	}
	vlanIntf := intf + "_" + strconv.Itoa(vlan)
	addVlanCmd := []string{"ip", "link", "add", "link", intf, "name", vlanIntf,
		"type", "vlan", "id", strconv.Itoa(vlan)}
	Common.Run(addVlanCmd, 0, false)
	cmd := []string{"ifconfig", intf, "hw", "ether", ep.Spec.GetMacAddress()}
	Common.Run(cmd, 0, false)

	infraEp.Interface.Name = vlanIntf
	infraEp.Interface.MacAddress = ep.Spec.GetMacAddress()
	infraEp.Interface.IPAddress = strings.Split(ep.Spec.GetIPv4Addresses()[0], "/")[0]
	infraEp.Init(false)

	return infraEp
}

func deleteEp(ep *Infra.Endpoint) {
	ep.Delete()
	deleteVlanCmd := []string{"ip", "link", "delete", ep.Interface.Name}
	Common.Run(deleteVlanCmd, 0, false)
}

func setUpRoute(srcEp *Infra.Endpoint, dstEp *Infra.Endpoint) error {

	if err := srcEp.AppEngine.RouteAdd(dstEp.GetIP(), defaultRoute(srcEp.GetNetwork()),
		dstEp.Interface.MacAddress); err != nil {
		return err
	}
	return dstEp.AppEngine.RouteAdd(srcEp.GetIP(), defaultRoute(dstEp.GetNetwork()),
		srcEp.Interface.MacAddress)
}

func sendGarp(srcEp *Infra.Endpoint, dstEp *Infra.Endpoint) error {

	cmd := []string{"arping", "-c", "1", "-U", srcEp.GetIP()}
	if _, err := srcEp.AppEngine.RunCommand(cmd, 0, false); err != nil {
		return err
	}

	time.Sleep(1 * time.Second)
	cmd = []string{"arping", "-c", "1", "-U", dstEp.GetIP()}
	if _, err := dstEp.AppEngine.RunCommand(cmd, 0, false); err != nil {
		return err
	}
	time.Sleep(2 * time.Second)

	return nil
}

func deleteRoute(srcEp *Infra.Endpoint, dstEp *Infra.Endpoint) error {

	if err := srcEp.AppEngine.RouteDelete(dstEp.GetIP(), defaultRoute(srcEp.GetNetwork())); err != nil {
		return err
	}
	return dstEp.AppEngine.RouteDelete(srcEp.GetIP(), defaultRoute(dstEp.GetNetwork()))
}

func readSuites(envPath string, suiteName string) ([]suite, error) {
	files, err := filepath.Glob(envPath + "/*.suite")
	if err != nil {
		return nil, errors.Wrap(err, "Error reading suite files")
	}

	var suites []suite
	for _, f := range files {
		yamlFile, err := os.Open(f)
		if err != nil {
			continue
		}
		byteValue, _ := ioutil.ReadAll(yamlFile)
		var tsuite suite
		yaml.Unmarshal(byteValue, &tsuite)
		if suiteName != "" {
			if suiteName == tsuite.Name {
				suites = []suite{tsuite}
			}
		} else {
			suites = append(suites, tsuite)
		}
		yamlFile.Close()
	}

	return suites, nil
}

func SetUpEPs(trafficHelper TrafficHelper, agentCfg *Pkg.AgentConfig) error {

	nwKeyMap := make(map[string]netproto.Endpoint)
	eps := getEps(agentCfg, trafficHelper.getTrafficType(), 0)
	epHandles := []*Infra.Endpoint{}
	fmt.Println("Setting up endpoints..")
	for _, ep := range eps {
		nw := getNetworkFromConfig(ep.Spec.GetNetworkName(), agentCfg.Networks)
		nwKey := strconv.Itoa(int(nw.Spec.GetVlanID()))
		if otherEp, ok := nwKeyMap[nwKey]; ok {
			fmt.Printf("Skipping EP %s(%s) setup as same Network Vlan already created for EP (%s)(%s)",
				ep.Name, ep.Spec.GetIPv4Addresses(), otherEp.Name, otherEp.Spec.GetIPv4Addresses())
			continue
		}
		nwKeyMap[nwKey] = ep
		epHandle := newEpFromAgentConfig(&ep, nw, "")
		epHandles = append(epHandles, epHandle)
	}

	fmt.Println("Setting up of endpoints complete.")

	c := make(chan os.Signal)
	signal.Notify(c, os.Interrupt, syscall.SIGTERM)
	fmt.Println("Press Ctrl-C to do clean up.")
	<-c
	fmt.Println("Cleaning up endpoints complete.")
	for _, ep := range epHandles {
		deleteEp(ep)
	}

	return nil
}

func RunTests(env string, suiteName string, trafficHelper TrafficHelper, agentCfg *Pkg.AgentConfig) error {

	envPath := testEnvDir + "/" + env
	if stat, err := os.Stat(envPath); err != nil || !stat.IsDir() {
		return errors.Errorf("Environment %s does not exist in path %s", env, envPath)
	}

	suites, err := readSuites(envPath, suiteName)
	if err != nil {
		return err
	}

	if suites == nil || len(suites) == 0 {
		return errors.Errorf("Suites (%s) not present in %s", suiteName, env)
	}

	for _, tsuite := range suites {
		if tsuite.Enabled {
			fmt.Println("Running test suite : ", tsuite.Name)
			if err := tsuite.Run(trafficHelper, agentCfg); err != nil {
				return errors.Wrapf(err, "Error running suite %s", suiteName)
			}
		} else {
			fmt.Println("Skipping test suite :", tsuite.Name)
		}
	}

	return nil
}

type baseTrafficHelper struct {
	configFile  string
	intfMap     map[string]string
	trafficType int
}

type TrafficHelper interface {
	init() error
	getTrafficInterface(string) (string, error)
	GetAllTrafficInterfaces() []string
	getTrafficType() int
}

type uplinkToUplinkTrafficHelper struct {
	baseTrafficHelper
}

func (b *baseTrafficHelper) getTrafficType() int {
	return b.trafficType
}

func (b *baseTrafficHelper) getTrafficInterface(intf string) (string, error) {
	return b.intfMap[intf], nil
}

func (b *baseTrafficHelper) GetAllTrafficInterfaces() []string {
	var intfs []string
	for intf, _ := range b.intfMap {
		intfs = append(intfs, intf)
	}
	return intfs
}

func readUplinkConfigFile(configFile string) (map[string]string, error) {
	jsonFile, err := ioutil.ReadFile(configFile)
	if err != nil {
		return nil, errors.Wrapf(err, "Err opening uplink map file : %s", configFile)
	}

	uplinkMap := map[string]string{}
	if err = json.Unmarshal(jsonFile, &uplinkMap); err != nil {
		return nil, errors.Wrapf(err, "Err parsing uplink map file : %s", configFile)
	}

	return uplinkMap, nil

}

func (u *uplinkToUplinkTrafficHelper) init() error {

	var err error
	u.intfMap, err = readUplinkConfigFile(u.configFile)
	return err
}

type hostToHostTrafficHelper struct {
	baseTrafficHelper
}

var getIntfMatchingMac = func(macaddr string) string {
	intfs, _ := net.Interfaces()
	for _, intf := range intfs {
		if intf.HardwareAddr.String() == macaddr {
			/* Mac address matched */
			return intf.Name
		}
	}
	return ""
}

func readHostConfigFile(configFile string) (map[string]string, error) {
	sDevices, err := Pkg.ReadStationDevices(configFile)

	if err != nil {
		return nil, errors.Wrap(err, "Error reading station device file")
	}
	hostIfMap := map[string]string{}
	for _, device := range sDevices {
		intf := fmt.Sprintf("lif%d", device.LifID)
		hostIfMap[intf] = intf
	}

	return hostIfMap, nil
}

func (h *hostToHostTrafficHelper) init() error {

	var err error
	h.intfMap, err = readHostConfigFile(h.configFile)
	return err
}

type uplinkToHostTrafficHelper struct {
	baseTrafficHelper
	hostConfigFile   string
	uplinkConfigFile string
}

func (h *uplinkToHostTrafficHelper) init() error {
	var err error
	if h.intfMap, err = readHostConfigFile(h.hostConfigFile); err != nil {
		return err
	}

	var uMap map[string]string
	if uMap, err = readUplinkConfigFile(h.uplinkConfigFile); err != nil {
		return err
	}

	for k, v := range uMap {
		h.intfMap[k] = v
	}

	return nil
}

func GetTrafficHelper(trafficType int, files ...string) (TrafficHelper, error) {

	var helper TrafficHelper
	switch trafficType {
	case TrafficHostToHost:
		helper = &hostToHostTrafficHelper{baseTrafficHelper: baseTrafficHelper{configFile: files[0], trafficType: trafficType}}
	case TrafficUplinkToUplink:
		helper = &uplinkToUplinkTrafficHelper{baseTrafficHelper: baseTrafficHelper{configFile: files[0], trafficType: trafficType}}
	case TrafficUplinkToHost:
		helper = &uplinkToHostTrafficHelper{hostConfigFile: files[0], uplinkConfigFile: files[1],
			baseTrafficHelper: baseTrafficHelper{trafficType: trafficType}}
	}

	if err := helper.init(); err != nil {
		return nil, err
	}

	return helper, nil

}

func init() {

	if len(os.Getenv("TRAFFIC_TEST_DIR")) != 0 {
		testEnvDir = os.Getenv("TRAFFIC_TEST_DIR")
	}

}
