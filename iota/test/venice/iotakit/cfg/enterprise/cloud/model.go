package cloud

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/workload"
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

	if params.Scale {
		cl.Cfg.NumOfTenants = 1
		cl.Cfg.NumOfVRFsPerTenant = 50
		cl.Cfg.NumOfSubnetsPerVpc = 20
		cl.Cfg.NumOfIPAMPsPerTenant = 1
		cl.Cfg.NumUnderlayRoutingConfigs = 1 // Same as other AS number
		cl.Cfg.NumUnderlayNeighbors = 1      //TOR AS nubr

	} else {
		cl.Cfg.NumOfTenants = 1
		cl.Cfg.NumOfVRFsPerTenant = 1
		cl.Cfg.NumOfSubnetsPerVpc = 8
		cl.Cfg.NumOfIPAMPsPerTenant = 1
		cl.Cfg.NumUnderlayRoutingConfigs = 1 // Same as other AS number
		cl.Cfg.NumUnderlayNeighbors = 1      //TOR AS nubr
	}

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

	// get all venice configs
	veniceHosts, err := rClient.ListHost()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}

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

	dscs, err := rClient.ListSmartNIC()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}

	for _, dsc := range dscs {
		filter := fmt.Sprintf("spec.type=host-pf,status.dsc=%v", dsc.Status.PrimaryMAC)
		hostNwIntfs, err := rClient.ListNetowrkInterfacesByFilter(filter)
		if err != nil {
			return err
		}

		for _, nwIntf := range hostNwIntfs {
			nwIntf.Spec.AttachNetwork = ""
			nwIntf.Spec.AttachTenant = ""

			err = rClient.UpdateNetworkInterface(nwIntf)
			if err != nil {
				log.Errorf("updating interface failed %v", err.Error())
				return err
			}
		}

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

	for _, obj := range veniceHosts {
		if err := rClient.DeleteHost(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}

	return nil

}

//Cloud mode we don't allow to create workloads, so return from cache
func (cl *CloudCfg) ListWorkload() (objs []*workload.Workload, err error) {

	wloads := []*workload.Workload{}
	for _, wl := range cl.Cfg.ConfigItems.TenantWorkloads {
		wloads = append(wloads, wl)
	}

	return wloads, nil
}

func (cl *CloudCfg) setupLoopbacks() error {

	var loppbackIntfs []*network.NetworkInterface
	var err error
	rClient := cl.Client

	bkCtx, cancelFunc := context.WithTimeout(context.Background(), 10*time.Minute)
	defer cancelFunc()

	expectedLoopbacks := len(cl.params.Dscs) + len(cl.params.FakeDscs)
L:
	for true {
		select {
		case <-bkCtx.Done():
			loopbackDSCs := make(map[string]bool)
			for _, dsc := range cl.params.Dscs {
				loopbackDSCs[dsc.Name] = false
			}
			for _, dsc := range cl.params.FakeDscs {
				loopbackDSCs[dsc.Name] = false
			}
			for _, intf := range loppbackIntfs {
				loopbackDSCs[strings.Split(intf.Name, "-")[0]] = true
			}

			for dsc, val := range loopbackDSCs {
				if !val {
					log.Infof("DSC missing loopback interface %v", dsc)
				}
			}
			log.Errorf("Expected %v , found %v ", expectedLoopbacks, len(loppbackIntfs))
			return fmt.Errorf("Error finding all loopback interfaces : %s", err)
		default:
			loppbackIntfs, err = rClient.ListNetworkLoopbackInterfaces()
			if err != nil {
				log.Errorf("Error querying loopback interfaces: Err: %v", err)
				return err
			}
			if (len(cl.params.Dscs) + len(cl.params.FakeDscs)) == len(loppbackIntfs) {
				break L
			}
			time.Sleep(2 * time.Second)
		}
	}

	for _, intf := range loppbackIntfs {

		uuid := strings.Split(intf.Name, "-")[0]

		loopbackIP, ok := cl.params.NaplesLoopBackIPs[uuid]
		if !ok {
			log.Errorf("Error finding loopback IP for %v %v Err: %v", intf.Name, uuid, err)
			return err
		}
		intf.Spec.IPAllocType = "static"
		intf.Spec.IPConfig = &cluster.IPConfig{
			IPAddress: strings.Split(loopbackIP, "/")[0] + "/32",
		}
		err = rClient.UpdateNetworkInterface(intf)
		if err != nil {
			log.Errorf("Error updating loopback interface: %v , Err: %v", intf, err)
			return err
		}
	}
	return err
}

func (cl *CloudCfg) pushConfigViaRest() error {

	rClient := cl.Client

	createHosts := func() error {
		for _, o := range cl.Cfg.ConfigItems.Hosts {
			err := rClient.CreateHost(o)
			if err != nil {
				log.Errorf("Error creating host: %+v. Err: %v", o, err)
				return err
			}
		}
		return nil
	}

	if err := createHosts(); err != nil {
		return err
	}

	//First setup loopback IP
	if err := cl.setupLoopbacks(); err != nil {
		return err
	}

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
		err := rClient.CreateVPC(vrf)
		if err != nil {
			log.Errorf("Error creating vpc %+v. Err: %v", vrf, err)
			return err
		}
	}

	/*
		for _, pol := range cl.Cfg.ConfigItems.IPAMPs {
			err := rClient.CreateIPAMPolicy(pol)
				if err != nil {
					log.Errorf("Error creating ipam %+v. Err: %v", pol, err)
					return err
				}
		}*/

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
