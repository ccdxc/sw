package base

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"math/rand"
	"net"
	"os"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/staging"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/cfgen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/willf/bitset"
)

//EntBaseCfg encapsulate all Gs configuration objects
type EntBaseCfg struct {
	*objClient.Client
	//naples map[string]*Naples // Naples instances
	Dscs           []*cluster.DistributedServiceCard
	ThirdPartyDscs []*cluster.DistributedServiceCard
	subnets        []*Network               // subnets
	sgPolicies     []*NetworkSecurityPolicy // simulated security policies
	apps           []*App                   // simulated apps

	DefaultSgPolicies []*NetworkSecurityPolicy //default sg policy pushed
	Cfg               *cfgen.Cfgen
}

//TimeTrack tracker
func TimeTrack(start time.Time, name string) time.Duration {
	elapsed := time.Since(start)
	log.Infof("%s took %s\n", name, elapsed)
	return elapsed
}

const configPushStatsFile = "/tmp/scale-cfg-push-stats.json"
const configStatsFile = "/tmp/scale-cfg-stats.json"
const configFile = "/tmp/scale-cfg.json"

type configPushTime struct {
	Config string `json:"Config"`
	Object struct {
		SgPolicy string `json:"SgPolicy"`
	} `json:"Object"`
}

//ConfigPushStats keep track of config push stats
type ConfigPushStats struct {
	Stats []configPushTime `json:"Stats"`
}

//NetworkSecurityPolicy wrapper for network security policy
type NetworkSecurityPolicy struct {
	VenicePolicy *security.NetworkSecurityPolicy
}

//App wrapper for App
type App struct {
	veniceApp *security.App
}

// Network represents a Vlan with a subnet (called network in venice)
type Network struct {
	Name          string // subnet name
	vlan          uint32
	ipPrefix      string
	bitmask       *bitset.BitSet
	subnetSize    int
	veniceNetwork *network.Network
}

//func (gs *EntBaseCfg) Client() objClient.ObjClient {
//	return gs.
//}

func (gs *EntBaseCfg) createNetwork(nw *network.Network) error {
	// parse the subnet
	_, ipnet, err := net.ParseCIDR(nw.Spec.IPv4Subnet)
	if err != nil {
		log.Errorf("Error parsing subnet %v. Err: %v", nw.Spec.IPv4Subnet, err)
		return err
	}

	// read ipv4 allocation bitmap
	subnetMaskLen, maskLen := ipnet.Mask.Size()
	subnetSize := 1 << uint32(maskLen-subnetMaskLen)
	bs := bitset.New(uint(subnetSize))
	bs.Set(0)
	snet := Network{
		Name:          fmt.Sprintf("Network-Vlan-%d", nw.Spec.VlanID),
		vlan:          nw.Spec.VlanID,
		ipPrefix:      nw.Spec.IPv4Subnet,
		subnetSize:    subnetSize,
		bitmask:       bs,
		veniceNetwork: nw,
	}
	gs.subnets = append(gs.subnets, &snet)

	return nil
}

//CfgObjects config objects genereted
type CfgObjects struct {
	Networks   []*network.Network                // subnets
	SgPolicies []*security.NetworkSecurityPolicy // simulated security policies
	Apps       []*security.App                   // simulated apps

}

//ConfigParams contoller
type ConfigParams struct {
	Dscs              []*cluster.DistributedServiceCard
	VeniceNodes       []*cluster.Node
	FakeDscs          []*cluster.DistributedServiceCard
	ThirdPartyDscs    []*cluster.DistributedServiceCard
	NaplesLoopBackIPs map[string]string
	Vlans             []uint32
	Scale             bool
	Regenerate        bool
}

// WorkloadPair is a pair of workloads
type workloadPair struct {
	first  *workload.Workload
	second *workload.Workload
	proto  string
	ports  []int
}

type workloadPairCollection struct {
	err   error
	pairs []*workloadPair
}

func (gs *EntBaseCfg) ObjClient() objClient.ObjClient {
	return gs.Client
}

//InitClient init client
func (gs *EntBaseCfg) InitClient(ctx context.Context, urls []string) {
	gs.Client = objClient.NewClient(ctx, urls).(*objClient.Client)
}

//WriteConfig saves config
func (gs *EntBaseCfg) WriteConfig() {
	ofile, err := os.OpenFile(configFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0755)
	if err != nil {
		panic(err)
	}
	j, err := json.MarshalIndent(&gs.Cfg.ConfigItems, "", "  ")
	ofile.Write(j)
	ofile.Close()
}

//ReadConfig reads saved config
func (gs *EntBaseCfg) ReadConfig() {
	jsonFile, err := os.OpenFile(configFile, os.O_RDONLY, 0755)
	if err != nil {
		panic(err)
	}
	byteValue, _ := ioutil.ReadAll(jsonFile)

	err = json.Unmarshal(byteValue, &gs.Cfg.ConfigItems)
	if err != nil {
		panic(err)
	}
	jsonFile.Close()
}

//PopulateConfig populate configuration
func (gs *EntBaseCfg) PopulateConfig(params *ConfigParams) error {

	gs.Cfg = cfgen.DefaultCfgenParams
	gs.Cfg.NetworkSecurityPolicyParams.NumPolicies = 1

	cfg := gs.Cfg

	//Reset config stats so that we can start fresh

	if params.Scale {
		/* 10K * 4*4 * 3 == 640K hal rules */
		cfg.NetworkSecurityPolicyParams.NumRulesPerPolicy = 10000
		cfg.NetworkSecurityPolicyParams.NumIPPairsPerRule = 4
		cfg.NetworkSecurityPolicyParams.NumAppsPerRules = 2 //1 rule added by gen
		cfg.WorkloadParams.WorkloadsPerHost = 32
		cfg.AppParams.NumApps = 1200
	} else {
		cfg.NetworkSecurityPolicyParams.NumRulesPerPolicy = 10
		cfg.NetworkSecurityPolicyParams.NumIPPairsPerRule = 4
		cfg.NetworkSecurityPolicyParams.NumAppsPerRules = 3
		cfg.WorkloadParams.WorkloadsPerHost = 16
		cfg.AppParams.NumApps = 4
	}

	smartnics := []*cluster.DistributedServiceCard{}
	for _, naples := range params.Dscs {
		smartnics = append(smartnics, naples)
	}

	gs.Dscs = params.Dscs
	gs.ThirdPartyDscs = params.ThirdPartyDscs

	//Add sim naples too.
	for _, naples := range params.FakeDscs {
		smartnics = append(smartnics, naples)
	}

	cfg.Smartnics = smartnics

	generateConfig := func() error {
		defer TimeTrack(time.Now(), "Config generation")
		// generate the configuration now
		cfg.Do()
		return nil
	}

	updateScaleConfig := func() error {

		genWorkloadPairs := func(wloads []*workload.Workload, count int) *workloadPairCollection {
			var pairs workloadPairCollection
			for i := 0; i < len(wloads); i++ {
				for j := i + 1; j <= len(wloads)-1; j++ {
					//pairs = append(pairs, []string{content[i], content[j]})
					if wloads[i].Spec.Interfaces[0].ExternalVlan == wloads[j].Spec.Interfaces[0].ExternalVlan {
						pairs.pairs = append(pairs.pairs, &workloadPair{first: wloads[i], second: wloads[j]})
						if count > 0 && len(pairs.pairs) == count {
							return &pairs
						}
					}
				}
			}
			return &pairs
		}

		//Get all real workloads first
		wloads := []*workload.Workload{}
		for _, o := range cfg.ConfigItems.Hosts {
			for _, realNaples := range params.Dscs {
				//Get Real host first
				if o.Spec.GetDSCs()[0].MACAddress == realNaples.Status.PrimaryMAC {
					for _, wload := range cfg.ConfigItems.Workloads {
						if wload.Spec.HostName == o.Name {
							wloads = append(wloads, wload)
						}
					}
				}
			}
		}
		pairs := genWorkloadPairs(wloads, 0)
		localPairs := workloadPairCollection{}
		remotePairs := workloadPairCollection{}

		rand.Shuffle(len(localPairs.pairs), func(i, j int) {
			localPairs.pairs[i], localPairs.pairs[j] = localPairs.pairs[j], localPairs.pairs[i]
		})
		rand.Shuffle(len(remotePairs.pairs), func(i, j int) {
			remotePairs.pairs[i], remotePairs.pairs[j] = remotePairs.pairs[j], remotePairs.pairs[i]
		})

		for _, pair := range pairs.pairs {
			if pair.first.Spec.HostName == pair.second.Spec.HostName {
				localPairs.pairs = append(localPairs.pairs, pair)
			} else {
				remotePairs.pairs = append(remotePairs.pairs, pair)
			}
		}

		localIndex := 0
		remoteIndex := 0
		for _, pol := range cfg.ConfigItems.SGPolicies {
			for _, rule := range pol.Spec.Rules {
				for index := range rule.FromIPAddresses {
					if index%2 == 0 && localIndex < len(localPairs.pairs) {
						rule.FromIPAddresses[index] = localPairs.pairs[localIndex].first.Spec.Interfaces[0].IpAddresses[0]
						rule.ToIPAddresses[index] = localPairs.pairs[localIndex].second.Spec.Interfaces[0].IpAddresses[0]
						localIndex++
					} else if remoteIndex < len(remotePairs.pairs) {
						rule.FromIPAddresses[index] = remotePairs.pairs[remoteIndex].first.Spec.Interfaces[0].IpAddresses[0]
						rule.ToIPAddresses[index] = remotePairs.pairs[remoteIndex].second.Spec.Interfaces[0].IpAddresses[0]
						remoteIndex++
					}
				}
			}

		}
		return nil
	}

	if params.Regenerate {
		//Generate fresh config if not skip setup
		generateConfig()
		// verify and keep the data in some file

		if params.Scale {
			err := updateScaleConfig()
			if err != nil {
				log.Errorf("Error updating scaling config %v", err)
				return err
			}
		}
		gs.WriteConfig()
	} else {
		gs.ReadConfig()
	}

	setupConfigs := func() error {

		log.Infof("Setting up configs..")
		nwMap := make(map[uint32]uint32)
		tbVlans := make([]uint32, len(params.Vlans))
		copy(tbVlans, params.Vlans)

		for _, o := range cfg.ConfigItems.Networks {
			if len(tbVlans) == 0 {
				return errors.New("Not enough vlans in the testbed for the config")
			}
			nwMap[o.Spec.VlanID] = tbVlans[0]
			//Change the network vlan to use the vlan ID allocated by testbed
			o.Spec.VlanID = tbVlans[0]
			tbVlans = tbVlans[1:]
			err := gs.createNetwork(o)
			if err != nil {
				return err
			}

		}
		for _, o := range cfg.ConfigItems.Workloads {
			if wireVlan, ok := nwMap[o.Spec.Interfaces[0].ExternalVlan]; ok {
				o.Spec.Interfaces[0].ExternalVlan = wireVlan
			} else {
				return fmt.Errorf("No testbed vlan found for external vlan %v", o.Spec.Interfaces[0].ExternalVlan)
			}
		}

		return nil
	}

	return setupConfigs()
}

//StagingBuffer
type stagingBuffer struct {
	name    string
	tenant  string
	ctx     context.Context
	stagecl apiclient.Services
	urls    []string
}

//AddHost to statging buffer
func (buf *stagingBuffer) AddHost(host *cluster.Host) error {

	_, err := buf.stagecl.ClusterV1().Host().Create(buf.ctx, host)
	return err
}

//AddApp to statging buffer
func (buf *stagingBuffer) AddApp(app *security.App) error {

	_, err := buf.stagecl.SecurityV1().App().Create(buf.ctx, app)
	return err
}

//AddWorkload to statging buffer
func (buf *stagingBuffer) AddWorkload(workload *workload.Workload) error {

	_, err := buf.stagecl.WorkloadV1().Workload().Create(buf.ctx, workload)
	return err
}

//AddNetowrkSecurityPolicy to statging buffer
func (buf *stagingBuffer) AddNetowrkSecurityPolicy(policy *security.NetworkSecurityPolicy) error {

	_, err := buf.stagecl.SecurityV1().NetworkSecurityPolicy().Create(buf.ctx, policy)
	return err
}

//Commit the buffer
func (buf *stagingBuffer) Commit() error {

	urls := buf.urls

	restcl, err := apiclient.NewRestAPIClient(urls[0])
	if err != nil {
		return err
	}

	action := &staging.CommitAction{}
	action.Name = buf.name
	action.Tenant = buf.tenant
	_, err = restcl.StagingV1().Buffer().Commit(buf.ctx, action)
	if err != nil {
		fmt.Printf("*** Failed to commit Staging Buffer(%s)\n", err)
		return err
	}
	return nil
}

func (gs *EntBaseCfg) pushConfigViaStagingBuffer() error {

	defer TimeTrack(time.Now(), "Committing Via Config buffer")

	urls := gs.Client.Urls()
	veniceContext := gs.Client.Context()
	restcl, err := apiclient.NewRestAPIClient(urls[0])
	if err != nil {
		return err
	}

	createStagingBuffer := func() (*stagingBuffer, error) {
		fmt.Printf("creating Staging Buffer\n")
		// Create a buffer
		buffer := &stagingBuffer{name: "TestBuffer", tenant: "default"}
		copy(buffer.urls, urls)
		buf := staging.Buffer{}
		buf.Name = buffer.name
		buf.Tenant = buffer.tenant
		_, err := restcl.StagingV1().Buffer().Create(veniceContext, &buf)
		if err != nil {
			fmt.Printf("*** Failed to create Staging Buffer(%s)\n", err)
			return nil, err
		}

		// Staging Client
		stagecl, err := apiclient.NewStagedRestAPIClient(urls[0], buffer.name)
		if err != nil {
			return nil, err
		}

		buffer.stagecl = stagecl
		buffer.ctx = veniceContext

		return buffer, nil
	}
	stagingBuf, err := createStagingBuffer()
	if err != nil {
		return err
	}

	for _, o := range gs.Cfg.ConfigItems.Hosts {
		err := stagingBuf.AddHost(o)
		if err != nil {
			log.Errorf("Error creating host: %+v. Err: %v", o, err)
			return err
		}
	}

	for _, w := range gs.Cfg.ConfigItems.Workloads {
		err := stagingBuf.AddWorkload(w)
		if err != nil {
			log.Errorf("Error creating workload Err: %v", err)
			return err
		}
	}

	for _, app := range gs.Cfg.ConfigItems.Apps {
		err := stagingBuf.AddApp(app)
		if err != nil {
			log.Errorf("Error creating app Err: %v", err)
			return err
		}
	}

	for _, pol := range gs.Cfg.ConfigItems.SGPolicies {
		err := stagingBuf.AddNetowrkSecurityPolicy(pol)
		if err != nil {
			log.Errorf("Error creating app Err: %v", err)
			return err
		}
	}

	//Finally commit it
	return stagingBuf.Commit()
}

func (gs *EntBaseCfg) pushConfigViaRest() error {

	cfg := gs.Cfg

	rClient := gs.Client

	CreateHosts := func() error {
		defer TimeTrack(time.Now(), "Creating hosts")
		for _, o := range cfg.ConfigItems.Hosts {
			err := rClient.CreateHost(o)
			if err != nil {
				log.Errorf("Error creating host: %+v. Err: %v", o, err)
				return err
			}
		}
		return nil
	}

	if err := CreateHosts(); err != nil {
		return err
	}

	createWorkloads := func() error {
		defer TimeTrack(time.Now(), "Create workloads")
		err := rClient.CreateWorkloads(cfg.ConfigItems.Workloads)
		if err != nil {
			return err
		}
		return nil
	}

	if err := createWorkloads(); err != nil {
		return err
	}

	for _, o := range cfg.ConfigItems.Apps {
		if err := rClient.CreateApp(o); err != nil {
			return fmt.Errorf("error creating app: %s", err)
		}
	}

	if len(cfg.ConfigItems.SGPolicies) > 1 {
		panic("can't have more than one sgpolicy")
	}

	for _, o := range cfg.ConfigItems.SGPolicies {
		createSgPolicy := func() error {
			defer TimeTrack(time.Now(), "Create Sg Policy")
			if err := rClient.CreateNetworkSecurityPolicy(o); err != nil {
				return fmt.Errorf("error creating sgpolicy: %s", err)
			}
			return nil
		}
		if err := createSgPolicy(); err != nil {
			return err
		}
	}

	//Append default Sg polcies
	gs.DefaultSgPolicies = []*NetworkSecurityPolicy{}
	for _, sgPolicy := range gs.Cfg.ConfigItems.SGPolicies {
		gs.DefaultSgPolicies = append(gs.DefaultSgPolicies, &NetworkSecurityPolicy{VenicePolicy: sgPolicy})
	}

	return nil
}

type veniceConfigStat struct {
	KindHistogram struct {
		App struct {
			MinMs  float64 `json:"MinMs"`
			MaxMs  float64 `json:"MaxMs"`
			MeanMs float64 `json:"MeanMs"`
		} `json:"App"`
		Endpoint struct {
			MinMs  float64 `json:"MinMs"`
			MaxMs  float64 `json:"MaxMs"`
			MeanMs float64 `json:"MeanMs"`
		} `json:"Endpoint"`
		NetworkSecurityPolicy struct {
			MinMs  float64 `json:"MinMs"`
			MaxMs  float64 `json:"MaxMs"`
			MeanMs float64 `json:"MeanMs"`
		} `json:"NetworkSecurityPolicy"`
	} `json:"KindHistogram"`
	NodeConfigHistogram []struct {
		NodeID    string `json:"NodeID"`
		KindStats struct {
			App struct {
				MinMs  float64 `json:"MinMs"`
				MaxMs  float64 `json:"MaxMs"`
				MeanMs float64 `json:"MeanMs"`
			} `json:"App"`
			Endpoint struct {
				MinMs  float64 `json:"MinMs"`
				MaxMs  float64 `json:"MaxMs"`
				MeanMs float64 `json:"MeanMs"`
			} `json:"Endpoint"`
			NetworkSecurityPolicy struct {
				MinMs  float64 `json:"MinMs"`
				MaxMs  float64 `json:"MaxMs"`
				MeanMs float64 `json:"MeanMs"`
			} `json:"NetworkSecurityPolicy"`
		} `json:"KindStats"`
		AggrStats struct {
			MinMs  float64 `json:"MinMs"`
			MaxMs  float64 `json:"MaxMs"`
			MeanMs float64 `json:"MeanMs"`
		} `json:"AggrStats"`
	} `json:"NodeConfigHistogram"`
}

/*
//ResetConfigStats Config Stats
func (gs *EntBaseCfg) ResetConfigStats() {
	var rawData veniceRawData
	sm.resetConfigStats(&rawData)
}

//PullConfigStats pulls config stats
func (gs *EntBaseCfg) PullConfigStats(configStats interface{}) error {

	return sm.doConfigPostAction("config-stats", configStats)
}


//ReadConfigStats read config stats
func (gs *EntBaseCfg) ReadConfigStats() {

	writeMeasuredStat := func() error {

		var configStat veniceConfigStat
		var rawData veniceRawData
		err := gs.PullConfigStats(&rawData)
		if err != nil {
			log.Infof("Config  stat Failed %v", err)
			return err
		}

		err = json.Unmarshal([]byte(rawData.Diagnostics.String), &configStat)
		if err != nil {
			log.Infof("Config state marshalling Failed %v", err)
			return err
		}

		ofile, err := os.OpenFile(ConfigStatsFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0755)
		if err != nil {
			panic(err)
		}
		j, err := json.MarshalIndent(&configStat, "", "  ")
		ofile.Write(j)
		ofile.Close()

		return nil
	}

	writeMeasuredStat()
}

func (gs *EntBaseCfg) ClearConfigPushStat() {
	os.Remove(configPushStatsFile)
}

*/

//PushConfig populate configuration
func (gs *EntBaseCfg) PushConfig() error {
	cfgPushTime := configPushTime{}
	cfgPushStats := ConfigPushStats{}

	readStatConfig := func() {
		jsonFile, err := os.OpenFile(configPushStatsFile, os.O_RDONLY, 0755)
		if err != nil {
			//file not created yet. ignore.
			return
		}
		byteValue, _ := ioutil.ReadAll(jsonFile)

		err = json.Unmarshal(byteValue, &cfgPushStats)
		if err != nil {
			panic(err)
		}
		jsonFile.Close()
	}

	writeStatConfig := func() {
		ofile, err := os.OpenFile(configPushStatsFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0755)
		if err != nil {
			panic(err)
		}
		j, err := json.MarshalIndent(&cfgPushStats, "", "  ")
		ofile.Write(j)
		ofile.Close()
	}

	configPushCheck := func(done chan error) {
		startTime := time.Now()
		iter := 1
		for ; iter <= 1500; iter++ {
			//Check every second
			time.Sleep(time.Second * time.Duration(iter))
			complete, err := gs.IsConfigPushComplete()
			if complete && err == nil {
				cfgPushTime.Config = TimeTrack(startTime, "Config Push").String()
				done <- nil
			}
		}
		done <- fmt.Errorf("Config push incomplete")
	}

	var err error
	err = gs.CleanupAllConfig()
	if err != nil {
		return err
	}

	if os.Getenv("USE_STAGING_BUFFER") != "" {
		err = gs.pushConfigViaStagingBuffer()
	} else {
		err = gs.pushConfigViaRest()
	}

	if err != nil {
		return err
	}

	policyPushCheck := func(done chan error) {

		rClient := gs.Client

		for _, o := range gs.Cfg.ConfigItems.SGPolicies {
			// verify that sgpolicy object has reached all naples
			startTime := time.Now()
			iter := 1
			for ; iter <= 2000; iter++ {
				time.Sleep(time.Second * time.Duration(iter))
				retSgp, err := rClient.GetNetworkSecurityPolicy(&o.ObjectMeta)
				if err != nil {
					done <- fmt.Errorf("error getting back policy %s %v", o.ObjectMeta.Name, err.Error())
					return
				} else if retSgp.Status.PropagationStatus.Updated == int32(len(gs.Dscs)) {
					duration := TimeTrack(startTime, "Sg Policy Push").String()
					cfgPushTime.Object.SgPolicy = duration
					done <- nil
					return
				}
				log.Warnf("Propagation stats did not match for policy %v. %+v", o.ObjectMeta.Name, retSgp.Status.PropagationStatus)
			}
			done <- fmt.Errorf("unable to update policy '%s' on all naples %+v",
				o.ObjectMeta.Name, o.Status.PropagationStatus)
		}
	}
	doneChan := make(chan error, 2)
	go policyPushCheck(doneChan)
	go configPushCheck(doneChan)

	for i := 0; i < 2; i++ {
		retErr := <-doneChan
		if retErr != nil {
			err = retErr
		}
	}

	readStatConfig()
	cfgPushStats.Stats = append(cfgPushStats.Stats, cfgPushTime)
	writeStatConfig()

	return nil
}

func (gs *EntBaseCfg) CleanupAllConfig() error {
	var err error

	rClient := gs.Client

	// get all venice configs
	veniceHosts, err := rClient.ListHost()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceSGPolicies, err := rClient.ListNetworkSecurityPolicy()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceNetworks, err := rClient.ListNetwork("")
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceApps, err := rClient.ListApp()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceWorkloads, err := rClient.ListWorkload()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}

	mirrorSessions, err := rClient.ListMirrorSessions()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}

	log.Infof("Cleanup: hosts %d, sgpolicy %d workloads %d hosts %d networks %d mirror sessions %d",
		len(veniceHosts), len(veniceSGPolicies), len(veniceWorkloads), len(veniceHosts), len(veniceNetworks),
		len(mirrorSessions))

	// delete venice objects
	for _, obj := range mirrorSessions {
		if err := rClient.DeleteMirrorSession(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}

	// delete venice objects
	for _, obj := range veniceSGPolicies {
		if err := rClient.DeleteNetworkSecurityPolicy(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}
	for _, obj := range veniceApps {
		if err := rClient.DeleteApp(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}
	if err := rClient.DeleteWorkloads(veniceWorkloads); err != nil {
		err = fmt.Errorf("Error deleting workloads Err: %v", err)
		log.Errorf("%s", err)
		return err
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

//IsConfigPushComplete checks whether config push is complete.
func (gs *EntBaseCfg) IsConfigPushComplete() (bool, error) {

	var configStatus objClient.VeniceConfigStatus
	var rawData objClient.VeniceRawData

	rClient := gs.Client

	err := rClient.PullConfigStatus(&rawData)
	if err != nil {
		log.Infof("Config  Failed %v", err)
		return false, err
	}

	err = json.Unmarshal([]byte(rawData.Diagnostics.String), &configStatus)
	if err != nil {
		log.Infof("Config marshalling Failed %v", err)
		return false, err
	}

	workloads, err := rClient.ListWorkload()
	if err != nil {
		return false, err
	}
	if configStatus.KindObjects.Endpoint != len(workloads) {
		log.Infof("Endpoints not synced with NPM yet. %v %v", configStatus.KindObjects.Endpoint, len(workloads))
		return false, nil
	}

	policies, err := rClient.ListNetworkSecurityPolicy()
	if err != nil {
		return false, err
	}
	if configStatus.KindObjects.NetworkSecurityPolicy != len(policies) {
		log.Infof("policies not synced with NPM yet.")
		return false, nil
	}

	for _, node := range configStatus.NodesStatus {
		if !node.KindStatus.App.Status.Create {
			log.Infof("App Creates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.App.Status.Update {
			log.Infof("App updates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.App.Status.Delete {
			//log.Infof("App deletes not synced for node %v", node.NodeID)
			//return false, nil
		}

		if !node.KindStatus.Endpoint.Status.Create {
			log.Infof("Endpoint Creates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.Endpoint.Status.Update {
			log.Infof("Endpoint updates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.Endpoint.Status.Delete {
			log.Infof("Endpoint deletes not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.SgPolicy.Status.Create {
			log.Infof("SgPolicy Creates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.SgPolicy.Status.Update {
			log.Infof("SgPolicy updates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.SgPolicy.Status.Delete {
			log.Infof("SgPolicy deletes not synced for node %v", node.NodeID)
			return false, nil
		}
	}

	return true, nil
}

//GetCfgObjects gets all generated objects
func (gs *EntBaseCfg) GetCfgObjects() CfgObjects {

	objects := CfgObjects{}

	for _, app := range gs.apps {
		objects.Apps = append(objects.Apps, app.veniceApp)
	}

	for _, nw := range gs.subnets {
		objects.Networks = append(objects.Networks, nw.veniceNetwork)
	}

	for _, pol := range gs.sgPolicies {
		objects.SgPolicies = append(objects.SgPolicies, pol.VenicePolicy)
	}

	return objects
}
