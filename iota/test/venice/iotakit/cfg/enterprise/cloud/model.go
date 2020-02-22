package cloud

import (
	"context"
	"encoding/json"
	"io/ioutil"
	"os"
	"strings"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/cfgen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/base"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

const configFile = "/tmp/scale-cfg.json"

//GsCfg encapsulate all Gs configuration objects
type CloudCfg struct {
	*objClient.Client
	//naples map[string]*Naples // Naples instances
	Dscs []*cluster.DistributedServiceCard

	params *base.ConfigParams
	Cfg    *cfgen.Cfgen
}

func NewCloudCfg() *CloudCfg {
	return &CloudCfg{}
}

func (cl *CloudCfg) ObjClient() objClient.ObjClient {
	return cl.Client
}

//WriteConfig saves config
func (cl *CloudCfg) WriteConfig() {
	ofile, err := os.OpenFile(configFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0755)
	if err != nil {
		panic(err)
	}
	j, err := json.MarshalIndent(&cl.Cfg.ConfigItems, "", "  ")
	ofile.Write(j)
	ofile.Close()
}

//ReadConfig reads saved config
func (cl *CloudCfg) ReadConfig() {
	jsonFile, err := os.OpenFile(configFile, os.O_RDONLY, 0755)
	if err != nil {
		panic(err)
	}
	byteValue, _ := ioutil.ReadAll(jsonFile)

	err = json.Unmarshal(byteValue, &cl.Cfg.ConfigItems)
	if err != nil {
		panic(err)
	}
	jsonFile.Close()
}

//InitClient init client
func (cl *CloudCfg) InitClient(ctx context.Context, urls []string) {
	cl.Client = objClient.NewClient(ctx, urls).(*objClient.Client)
}

//PopulateConfig populate configuration
func (cl *CloudCfg) PopulateConfig(params *base.ConfigParams) error {

	cl.Cfg = cfgen.DefaultCfgenParams
	cl.params = params

	cl.Cfg.NumRoutingConfigs = len(params.VeniceNodes)
	cl.Cfg.NumNeighbors = 2 + len(params.Dscs) //2 Ecx and from 3rd, naples

	cl.Cfg.NumUnderlayRoutingConfigs = 1 // Same as other AS number
	cl.Cfg.NumUnderlayNeighbors = 1      //TOR AS nubr

	cl.Cfg.NumOfTenants = 2
	cl.Cfg.NumOfVRFsPerTenant = 2
	cl.Cfg.NumOfSubnetsPerVpc = 2
	cl.Cfg.NumOfIPAMPsPerTenant = 1

	cl.Cfg.NetworkSecurityPolicyParams.NumRulesPerPolicy = 10
	cl.Cfg.NetworkSecurityPolicyParams.NumIPPairsPerRule = 4
	cl.Cfg.NetworkSecurityPolicyParams.NumAppsPerRules = 3
	cl.Cfg.AppParams.NumApps = 4

	smartnics := []*cluster.DistributedServiceCard{}
	for _, naples := range params.Dscs {
		smartnics = append(smartnics, naples)
	}

	cl.Dscs = params.Dscs

	//Add sim naples too.
	for _, naples := range params.FakeDscs {
		smartnics = append(smartnics, naples)
	}

	cl.Cfg.Smartnics = smartnics

	_, err := os.Stat(configFile)
	if params.Regenerate || os.IsNotExist(err) {
		//Generate fresh config if not skip setup
		cl.Cfg.Do()
		// verify and keep the data in some file

		cl.WriteConfig()
	} else {
		cl.ReadConfig()
	}

	//TODO : modify config based on some testbed parameters such as vlan
	setupConfigs := func() error {
		return nil
	}
	return setupConfigs()
}

//IsConfigPushComplete checks whether config push is complete.
func (cl *CloudCfg) IsConfigPushComplete() (bool, error) {

	return true, nil
}

//CleanupAllConfig clean up all config
func (cl *CloudCfg) CleanupAllConfig() error {

	rClient := cl.Client
	// get all venice configs
	nodes, err := rClient.ListClusterNodes()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}

	for _, n := range nodes {
		n.Spec.RoutingConfig = ""
		if err := rClient.UpdateClusterNode(n); err != nil {
			log.Errorf("err: %s", err)
			return err
		}
	}

	// List routing config
	configs, err := rClient.ListRoutingConfig()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}

	for _, config := range configs {
		err := rClient.DeleteRoutingConfig(config)
		if err != nil {
			log.Errorf("err: %s", err)
			return err
		}
	}

	// List routing config
	tenants, err := rClient.ListTenant()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}

	for _, ten := range tenants {
		if ten.Name == globals.DefaultTenant {
			continue
		}
		networks, err := rClient.ListNetwork(ten.Name)
		if err != nil {
			log.Errorf("err: %s", err)
			return err
		}

		for _, config := range networks {
			err := rClient.DeleteNetwork(config)
			if err != nil {
				log.Errorf("err: %s", err)
				return err
			}
		}

		// List routing config
		ipams, err := rClient.ListIPAMPolicy(ten.Name)
		if err != nil {
			log.Errorf("err: %s", err)
			return err
		}
		for _, config := range ipams {
			err := rClient.DeleteIPAMPolicy(config)
			if err != nil {
				log.Errorf("err: %s", err)
				return err
			}
		}

		// List routing config
		vpcs, err := rClient.ListVPC(ten.Name)
		if err != nil {
			log.Errorf("err: %s", err)
			return err
		}

		for _, vpc := range vpcs {
			log.Infof("Deleting VPC %v", vpc)
			err := rClient.DeleteVPC(vpc)
			if err != nil {
				log.Errorf("err: %s", err)
				return err
			}
		}

		err = rClient.DeleteTenant(ten)
		if err != nil {
			log.Errorf("err: %s", err)
			return err
		}
	}

	return nil

}

func (cl *CloudCfg) pushConfigViaRest() error {

	rClient := cl.Client

	//Create underlay config
	for _, r := range cl.Cfg.ConfigItems.UnderlayRouteConfig {
		err := rClient.CreateRoutingConfig(r)
		if err != nil {
			log.Errorf("Error creating routing config : %+v. Err: %v", r, err)
			return err
		}
	}

	for index, r := range cl.Cfg.ConfigItems.RouteConfig {
		node := cl.params.VeniceNodes[index]

		r.Spec.BGPConfig.RouterId = node.Name

		dscs := cl.params.Dscs
		for i := 2; i < len(r.Spec.BGPConfig.Neighbors); i++ {
			neigh := r.Spec.BGPConfig.Neighbors[i]
			neigh.IPAddress = strings.Split(dscs[i-2].Status.IPConfig.IPAddress, "/")[0]
		}

		err := rClient.CreateRoutingConfig(r)
		if err != nil {
			log.Errorf("Error creating routing config : %+v. Err: %v", r, err)
			return err
		}
		//Update cluster
		node.Spec = cluster.NodeSpec{
			RoutingConfig: r.Name,
		}

		err = rClient.UpdateClusterNode(node)
		if err != nil {
			log.Errorf("Error updating cluster node %+v. Err: %v", node, err)
			return err
		}
	}

	for _, ten := range cl.Cfg.ConfigItems.Tenants {
		err := rClient.CreateTenant(ten)
		if err != nil {
			log.Errorf("Error creating tenant: %+v. Err: %v", ten, err)
			return err
		}
	}

	for _, vrf := range cl.Cfg.ConfigItems.VRFs {
		log.Infof("Creating vrf %v", vrf)
		err := rClient.CreateVPC(vrf)
		if err != nil {
			log.Errorf("Error creating vpc %+v. Err: %v", vrf, err)
			return err
		}
	}

	for _, pol := range cl.Cfg.ConfigItems.IPAMPs {
		err := rClient.CreateIPAMPolicy(pol)
		if err != nil {
			log.Errorf("Error creating ipam %+v. Err: %v", pol, err)
			return err
		}
	}

	for _, sub := range cl.Cfg.ConfigItems.Subnets {
		err := rClient.CreateNetwork(sub)
		if err != nil {
			log.Errorf("Error creating subnet %+v. Err: %v", sub, err)
			return err
		}
	}

	return nil
}

//PushConfig populate configuration
func (cl *CloudCfg) PushConfig() error {

	if err := cl.pushConfigViaRest(); err != nil {
		log.Errorf("Config push failed %v", err)
		return err
	}
	return nil
}

//GetCfgObjects gets all generated objects
func (gs *CloudCfg) GetCfgObjects() base.CfgObjects {

	objects := base.CfgObjects{}

	return objects
}
