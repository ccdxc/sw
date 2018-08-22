package traffic

import (
	"encoding/binary"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/pkg/errors"

	yaml "gopkg.in/yaml.v2"

	Pkg "github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	_ "github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic/tests/modules"
	Infra "github.com/pensando/sw/nic/e2etests/go/infra"

	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
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

		Remote: ep.Spec.InterfaceType == "uplink",
	}
	infraEp.Interface.Name = intf
	infraEp.Interface.MacAddress = ep.Spec.GetMacAddress()
	infraEp.Interface.IPAddress = strings.Split(ep.Spec.GetIPv4Address(), "/")[0]
	infraEp.Interface.PrefixLen, _ = strconv.Atoi(strings.Split(nw.Spec.GetIPv4Subnet(), "/")[1])
	if infraEp.Remote {
		infraEp.Interface.EncapVlan = int(nw.Spec.GetVlanID())
	} else {
		infraEp.Interface.EncapVlan = int(ep.Spec.GetUsegVlan())
	}
	infraEp.Init(false)

	return infraEp
}

func setUpRoute(srcEp *Infra.Endpoint, dstEp *Infra.Endpoint) error {

	if err := srcEp.AppEngine.RouteAdd(dstEp.GetIP(), defaultRoute(srcEp.GetNetwork()),
		dstEp.Interface.MacAddress); err != nil {
		return err
	}
	return dstEp.AppEngine.RouteAdd(srcEp.GetIP(), defaultRoute(dstEp.GetNetwork()),
		srcEp.Interface.MacAddress)
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

func RunTests(env string, suiteName string, uplinkMapFile string, agentCfg *Pkg.AgentConfig) error {

	envPath := testEnvDir + "/" + env
	if stat, err := os.Stat(envPath); err != nil || !stat.IsDir() {
		return errors.Errorf("Environment %s does not exist", env)
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
			if err := tsuite.Run(uplinkMapFile, agentCfg); err != nil {
				return errors.Wrapf(err, "Error running suite %s", suiteName)
			}
		} else {
			fmt.Println("Skipping test suite :", tsuite.Name)
		}
	}

	return nil
}
