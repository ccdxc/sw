package pkg

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"path/filepath"

	"gopkg.in/yaml.v2"

	"os"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/libs"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
)

var objCache map[string]Object

// networkCache maintains a mapping of network name and the assigned subnet
var networkCache map[string]string

// endpointCache maintains a mapping of ep name and the assigned IP Address
var endpointCache map[string]string

// networkNSCache maintains a mapping of network name and its namespace
var networkNSCache map[string]string

func GenerateObjectsFromManifest(manifestFile string) (*Config, error) {
	objCache = make(map[string]Object)
	networkCache = make(map[string]string)
	networkNSCache = make(map[string]string)

	dat, err := ioutil.ReadFile(manifestFile)
	if err != nil {
		return nil, err
	}

	var c Config
	err = yaml.Unmarshal(dat, &c)
	if err != nil {
		return nil, err
	}

	// build obj cache for easier lookups
	for _, o := range c.Objects {
		objCache[o.Kind] = o
	}

	// generate configs
	err = c.generateObjs(manifestFile)
	if err != nil {
		return nil, err
	}
	return &c, nil
}

func (c *Config) generateObjs(manifestFile string) error {
	for i, o := range c.Objects {
		switch o.Kind {
		case "Namespace":
			genObj, err := c.generateNamespaces(&o, manifestFile)
			if err != nil {
				return err
			}
			c.Objects[i] = *genObj
		case "Network":
			genObj, err := c.generateNetworks(&o, manifestFile)
			if err != nil {
				return err
			}
			c.Objects[i] = *genObj
		case "Endpoint":
			genObj, err := c.generateEndpoints(&o, manifestFile)
			if err != nil {
				return err
			}
			c.Objects[i] = *genObj
		case "SGPolicy":
			genObj, err := c.generateSGPolicies(&o, manifestFile)
			if err != nil {
				return err
			}
			c.Objects[i] = *genObj
		}
	}

	return nil
}

func (c *Config) generateNamespaces(o *Object, manifestFile string) (*Object, error) {
	var namespaces []netproto.Namespace
	specFile := "generated/namespaces.json"
	// If spec file is already present in the manifest, nothing to do here
	if len(o.SpecFile) > 0 {
		return o, nil
	}
	for i := 0; i < o.Count; i++ {
		name := fmt.Sprintf("%s-%d", o.Name, i)
		ns := netproto.Namespace{
			TypeMeta: api.TypeMeta{Kind: "Namespace"},
			ObjectMeta: api.ObjectMeta{
				Tenant: "default",
				Name:   name,
			},
		}
		namespaces = append(namespaces, ns)
	}
	out, err := json.MarshalIndent(namespaces, "", "  ")
	if err != nil {
		return nil, err
	}

	// Automatically interpret the the base dir of the manifest file as the config dir to dump all the generated files
	configDir, _ := filepath.Split(manifestFile)
	fileName := fmt.Sprintf("%s%s", configDir, specFile)

	// create a generated dir in the config directory to dump the json
	genDir, _ := filepath.Split(fileName)
	if _, err := os.Stat(genDir); os.IsNotExist(err) {
		err = os.MkdirAll(genDir, 0755)
		if err != nil {
			return nil, fmt.Errorf("creating the generated directory failed. Err: %v", err)
		}
	}

	err = ioutil.WriteFile(fileName, out, 0644)
	if err != nil {
		return nil, err
	}
	o.SpecFile = specFile
	return o, nil
}

func (c *Config) generateNetworks(o *Object, manifestFile string) (*Object, error) {
	var networks []netproto.Network
	specFile := "generated/networks.json"
	// If spec file is already present in the manifest, nothing to do here
	if len(o.SpecFile) > 0 {
		return o, nil
	}

	subnets := libs.GenSubnets(o.Count)

	// Networks need to refer to Namespaces
	namespaceRef := objCache["Namespace"]

	// generate networks distributed evenly across
	for i := 0; i < o.Count; i++ {
		name := fmt.Sprintf("%s-%d", o.Name, i)
		namespace := fmt.Sprintf("%s-%d", namespaceRef.Name, i%namespaceRef.Count)
		subnet := subnets[i]
		_, gwIP, _ := libs.GenIPAddress(subnet, 4)
		nt := netproto.Network{
			TypeMeta: api.TypeMeta{Kind: "Network"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: namespace,
				Name:      name,
			},
			Spec: netproto.NetworkSpec{
				IPv4Subnet:  subnet,
				IPv4Gateway: gwIP,
				VlanID:      uint32(i + 1),
			},
		}
		networks = append(networks, nt)

		// update network cache with the generated subnets for any objects referring to it
		networkCache[name] = subnet
		networkNSCache[name] = namespace
	}
	out, err := json.MarshalIndent(&networks, "", "  ")
	if err != nil {
		return nil, err
	}

	// Automatically interpret the the base dir of the manifest file as the config dir to dump all the generated files
	configDir, _ := filepath.Split(manifestFile)

	fileName := fmt.Sprintf("%s%s", configDir, specFile)

	// create a generated dir in the config directory to dump the json
	genDir, _ := filepath.Split(fileName)
	if _, err := os.Stat(genDir); os.IsNotExist(err) {
		err = os.MkdirAll(genDir, 0755)
		if err != nil {
			return nil, fmt.Errorf("creating the generated directory failed. Err: %v", err)
		}
	}

	err = ioutil.WriteFile(fileName, out, 0644)
	if err != nil {
		return nil, err
	}
	o.SpecFile = specFile
	return o, nil
}

func (c *Config) generateEndpoints(o *Object, manifestFile string) (*Object, error) {
	var endpoints []netproto.Endpoint
	specFile := "generated/endpoints.json"
	// If spec file is already present in the manifest, nothing to do here
	if len(o.SpecFile) > 0 {
		return o, nil
	}

	// EPs need to refer to Namespaces and Networks
	namespaceRef := objCache["Namespace"]
	networkRef := objCache["Network"]
	endpointsPerNetwork := o.Count / networkRef.Count

	// generate endpoints distributed evenly across networks
	for i := 0; i < networkRef.Count; i++ {
		for j := 0; j < endpointsPerNetwork; j++ {
			var ifName, ifType, nodeUUID string
			name := fmt.Sprintf("%s-%d", o.Name, i*endpointsPerNetwork+j)
			network := fmt.Sprintf("%s-%d", networkRef.Name, i)
			namespace := networkNSCache[network]
			epSubnet := networkCache[network]

			epAddrBlock, _, err := libs.GenIPAddress(epSubnet, endpointsPerNetwork)
			if err != nil {
				return nil, err
			}
			epMACBlock, err := libs.GenMACAddresses(o.Count)
			if err != nil {
				return nil, err
			}
			endpointIP := fmt.Sprintf("%s/32", epAddrBlock[j])
			endpointMAC := epMACBlock[i*j%namespaceRef.Count]

			// evenly distribute auto generated endpoints to be local and remote
			if j < endpointsPerNetwork/2 {
				ifType = "lif"
				ifName = fmt.Sprintf("lif%d", (i*j%namespaceRef.Count%LIF_COUNT)+LIF_START)

			} else {

				ifType = "uplink"
				ifName = fmt.Sprintf("uplink%d", (i*endpointsPerNetwork+j+1)%2+1)
				nodeUUID = "GWUUID" // This will ensure that the EP is remote

			}

			ep := netproto.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "default",
					Namespace: namespace,
					Name:      name,
				},
				Spec: netproto.EndpointSpec{
					NetworkName:   network,
					UsegVlan:      uint32(i*endpointsPerNetwork + j + 1),
					IPv4Address:   endpointIP,
					MacAddress:    endpointMAC,
					InterfaceType: ifType,
					Interface:     ifName,
					NodeUUID:      nodeUUID,
				},
			}
			endpoints = append(endpoints, ep)

			// update ep cache cache with the generated ip address for the EP
			networkCache[name] = endpointIP
		}
	}
	out, err := json.MarshalIndent(endpoints, "", "  ")
	if err != nil {
		return nil, err
	}

	// Automatically interpret the the base dir of the manifest file as the config dir to dump all the generated files
	configDir, _ := filepath.Split(manifestFile)
	fileName := fmt.Sprintf("%s%s", configDir, specFile)

	// create a generated dir in the config directory to dump the json
	genDir, _ := filepath.Split(fileName)
	if _, err := os.Stat(genDir); os.IsNotExist(err) {
		err = os.MkdirAll(genDir, 0755)
		if err != nil {
			return nil, fmt.Errorf("creating the generated directory failed. Err: %v", err)
		}
	}

	err = ioutil.WriteFile(fileName, out, 0755)
	if err != nil {
		return nil, err
	}
	o.SpecFile = specFile
	return o, nil
}

func (c *Config) generateSGPolicies(o *Object, manifestFile string) (*Object, error) {
	// TODO add auto gen referring to EPs here.
	return o, nil
}
