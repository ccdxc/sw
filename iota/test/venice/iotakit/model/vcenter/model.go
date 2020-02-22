package vcenter

import (
	"context"
	"errors"
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/workload"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/base"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/enterprise"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

//VcenterSysModel representing model for vcenter
type VcenterSysModel struct {
	enterprise.SysModel
	orchestrator *objects.Orchestrator
}

//GetOrchestrator returns orchestroa
func (sm *VcenterSysModel) getOrchestrator() (*objects.Orchestrator, error) {

	for _, node := range sm.Tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VCENTER_NODE {
			return objects.NewOrchestrator(sm.ObjClient(),
				sm.Tb.GetDC(),
				node.NodeName,
				node.NodeMgmtIP,
				sm.Tb.Params.Provision.Vars["VcenterUsername"],
				sm.Tb.Params.Provision.Vars["VcenterPassword"],
			), nil
		}
	}

	return nil, errors.New("Vcenter orchestrator not found")
}

func (sm *VcenterSysModel) GetOrchestrator() (*objects.Orchestrator, error) {
	return sm.orchestrator, nil
}

func (sm *VcenterSysModel) getOrchRuleCookie() string {
	//For later deletion
	return "iota-orch-disable"
}

//DisconnectVeniceAndOrchestrator disonnect vcenter and venice
func (sm *VcenterSysModel) DisconnectVeniceAndOrchestrator() error {
	trig := sm.Tb.NewTrigger()

	//For later deletion
	cookie := sm.getOrchRuleCookie()

	orch, _ := sm.GetOrchestrator()

	for _, venice := range sm.VeniceNodeMap {
		cmd := fmt.Sprintf("sudo iptables -A INPUT -s %v -j DROP  -m comment --comment %s",
			strings.Split(orch.IP, "/")[0], cookie)
		trig.AddCommand(cmd, venice.Name()+"_venice", venice.Name())
		trig.AddCommandWithRetriesOnFailures(cmd, venice.Name()+"_venice",
			venice.Name(), 3)
	}

	// run the trigger
	resp, err := trig.Run()
	if err != nil {
		log.Errorf("Error running disonnect vcenter command on venice. Err: %v", resp)
		return fmt.Errorf("Error running disonnect  naples command on venice. Err: %v", resp)
	}

	for _, cmd := range resp {
		if cmd.ExitCode != 0 {
			log.Errorf("Error running deny on venice for vcenter . Err: %v", cmd)
			return fmt.Errorf("Error running deny on venice vcenter . Err: %v", cmd)
		}
	}

	return nil
}

//AllowVeniceAndOrchestrator allows venice to be connected from venice
func (sm *VcenterSysModel) AllowVeniceAndOrchestrator() error {
	trig := sm.Tb.NewTrigger()
	//For later deletion
	cookie := sm.getOrchRuleCookie()

	for _, venice := range sm.VeniceNodeMap {
		cmd := fmt.Sprintf("sudo iptables -L INPUT --line-numbers | grep  -w %v | grep -i drop | awk '{print $1}' | sort -n -r | xargs -n 1 sudo iptables -D INPUT $1",
			cookie)
		trig.AddCommandWithRetriesOnFailures(cmd, venice.Name()+"_venice",
			venice.Name(), 3)
	}

	// run the trigger should serial as we are modifying ipables.
	resp, err := trig.Run()
	if err != nil {
		log.Errorf("Error running disonnect command on venice. Err: %v", resp)
		//Ignore error as the rule may not be there.
	}

	for _, cmd := range resp {
		if cmd.ExitCode != 0 {
			log.Errorf("Error running allow on venice for naples . Err: %v", cmd)
			return fmt.Errorf("Error running allow on venice naples . Err: %v", cmd)
		}
	}

	return nil
}

//SetupWorkloads bring up.Workloads on host
func (sm *VcenterSysModel) SetupWorkloads(scale bool) error {

	var wc objects.WorkloadCollection

	/*
		hosts, err := sm.ListRealHosts()
		if err != nil {
			log.Error("Error finding real hosts to run traffic tests")
			return err
		}

			nws, err := sm.ListNetwork()
			if err != nil {
				log.Error("Error finding networks.")
				return err
			}

			getHost := func(uuid string) *objects.Host {
				nodeMac := strings.Replace(uuid, ":", "", -1)
				nodeMac = strings.Replace(nodeMac, ".", "", -1)
				for _, host := range hosts {
					objMac := strings.Replace(host.VeniceHost.GetSpec().DSCs[0].MACAddress, ".", "", -1)
					if objMac == nodeMac {
						return host
					}
				}
				return nil
			}
	*/

	skipSetup := os.Getenv("SKIP_SETUP")
	// if we are skipping setup we dont need to bringup the workload
	if skipSetup != "" {
		//Tru to setup state of.Workloads
		delWloads := &iota.WorkloadMsg{WorkloadOp: iota.Op_DELETE, Workloads: []*iota.Workload{}}
		// first get a list of all existing.Workloads from iota
		gwlm := &iota.WorkloadMsg{
			ApiResponse: &iota.IotaAPIResponse{},
			WorkloadOp:  iota.Op_GET,
		}
		topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
		getResp, err := topoClient.GetWorkloads(context.Background(), gwlm)
		log.Debugf("Got get workload resp: %+v, err: %v", getResp, err)
		if err != nil {
			log.Errorf("Failed to instantiate Apps. Err: %v", err)
			return fmt.Errorf("Error creating IOTA workload. err: %v", err)
		} else if getResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			log.Errorf("Failed to instantiate Apps. resp: %+v.", getResp.ApiResponse)
			return fmt.Errorf("Error creating IOTA workload. Resp: %+v", getResp.ApiResponse)
		}

		log.Info("Doing Workload bring up check")
		for _, naples := range sm.NaplesNodes {
			log.Infof("Naples Found %v", naples.Name())
		}
		for _, gwrk := range getResp.Workloads {
			//Add to delete list
			delWloads.Workloads = append(delWloads.Workloads, gwrk)
			continue
			/*
				host, ok := sm.NaplesHosts[gwrk.NodeName]
				if !ok {
					host, ok = sm.ThirdPartyHosts[gwrk.NodeName]
					if !ok {
						msg := fmt.Sprintf("Did not find naples for workload %v %v", gwrk.NodeName, gwrk.WorkloadName)
						log.Error(msg)
						delWloads.Workloads = append(delWloads.Workloads, gwrk)
						continue
					}
				} else {
					//Make sure naples uuid and host uuid match
					host = getHost(host.Naples.Nodeuuid)
					if host == nil {
						msg := fmt.Sprintf("Did not find associated host for workload %v %v", gwrk.NodeName, gwrk.WorkloadName)
						log.Error(msg)
						//Add to delete list
						delWloads.Workloads = append(delWloads.Workloads, gwrk)
						continue
					}
				}

				wloads, err := sm.ListWorkloadsOnHost(host.VeniceHost)
				if err != nil {
					log.Errorf("Error finding real.Workloads on hosts, inititing delete")
					delWloads.Workloads = append(delWloads.Workloads, gwrk)
					continue
				}
				var workloadObj *workload.Workload
				for _, w := range wloads {
					if w.Name == gwrk.WorkloadName {
						workloadObj = w
						break
					}
				}
				if workloadObj == nil {
					//Workload not present on host, delete it
					delWloads.Workloads = append(delWloads.Workloads, gwrk)
				} else {
					added := false
					for _, nw := range nws {
						if nw.Spec.VlanID == workloadObj.Spec.Interfaces[0].ExternalVlan {
							wl := objects.NewWorkload(host, workloadObj, sm.Tb.Topo.WorkloadType, sm.Tb.Topo.WorkloadImage,
								sm.Tb.GetSwitch(), nw.Name)
							sm.WorkloadsObjs[workloadObj.Name] = wl
							if err != nil {
								log.Errorf("Error creating workload %v", workloadObj)
								return err
							}
							sm.WorkloadsObjs[workloadObj.Name].SetIotaWorkload(gwrk)
							wc.Workloads = append(wc.Workloads, wl)
							added = true
						}
					}
					if !added {
						//Network does not exists, delete workload too
						delWloads.Workloads = append(delWloads.Workloads, gwrk)
					}
				}
			*/
		}

		if len(delWloads.Workloads) != 0 {
			delResp, err := topoClient.DeleteWorkloads(context.Background(), delWloads)
			log.Debugf("Got get workload resp: %+v, err: %v", delResp, err)
			if err != nil {
				log.Errorf("Failed to delete old Apps. Err: %v", err)
				return fmt.Errorf("Error deleting IOTA workload. err: %v", err)
			}

		}
	}

	if len(wc.Workloads) != 0 {
		workloads := []*workload.Workload{}
		for _, wr := range wc.Workloads {
			wr.VeniceWorkload.ObjectMeta.Labels = map[string]string{"MgmtIp": wr.GetMgmtIP()}
			workloads = append(workloads, wr.VeniceWorkload)
		}

		if err := sm.CreateWorkloads(workloads); err != nil {
			log.Errorf("unable to update the.Workloads label")
		}
	}

	return nil
}

// SetupDefaultConfig sets up a default config for the system
func (sm *VcenterSysModel) SetupDefaultConfig(ctx context.Context, scale, scaleData bool) error {

	err := sm.InitConfig(scale, scaleData)
	if err != nil {
		return err
	}

	//objects.NewOrchestrator(
	if err := sm.SetupDefaultCommon(ctx, scale, scaleData); err != nil {
		return err
	}

	bkCtx, cancelFunc := context.WithTimeout(context.Background(), 10*time.Minute)
	defer cancelFunc()

	//Give it some time for hosts to be discovered.
L:
	for true {
		select {
		case <-bkCtx.Done():
			return fmt.Errorf("Error associating hosts: %s", err)
		default:
			err = sm.AssociateHosts()
			if err != nil {
				return fmt.Errorf("Error associating hosts: %s", err)
			}

			if len(sm.NaplesHosts) == len(sm.NaplesNodes)+len(sm.FakeNaples) {
				break L
			}
			time.Sleep(2 * time.Second)
		}
	}

	//Reassociate workloads to hosts
	hc := sm.Hosts()
	wloads, err := sm.ListWorkload()
	if err != nil {
		return err
	}

	newHostMap := make(map[string]string)
	hostUsedMap := make(map[string]bool)
	for _, wl := range wloads {
		if newHost, ok := newHostMap[wl.Spec.HostName]; ok {
			wl.Spec.HostName = newHost
			continue
		}
		for _, host := range hc.Hosts {
			if _, ok := hostUsedMap[host.VeniceHost.Name]; !ok {
				hostUsedMap[host.VeniceHost.Name] = true
				newHostMap[wl.Spec.HostName] = host.VeniceHost.Name
				wl.Spec.HostName = host.VeniceHost.Name
				break
			}
		}
	}

	return sm.SetupWorkloads(scale)
}

// deleteWorkload deletes a workload
func (sm *VcenterSysModel) deleteWorkload(wr *objects.Workload) error {
	// FIXME: free mac addr for the workload
	// FIXME: free useg vlan
	// FIXME: free ip address for the workload

	// delete venice workload not need in venice
	//sm.CfgModel.DeleteWorkloads([]*workload.Workload{wr.VeniceWorkload})
	//Ignore the error as we don't care

	wrkLd := &iota.WorkloadMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		WorkloadOp:  iota.Op_DELETE,
		Workloads:   []*iota.Workload{wr.GetIotaWorkload()},
	}

	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	appResp, err := topoClient.DeleteWorkloads(context.Background(), wrkLd)

	if err != nil || appResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Failed to instantiate Apps. %v/%v", err, appResp.ApiResponse.ApiStatus)
		return fmt.Errorf("Error deleting IOTA workload. Resp: %+v, err: %v", appResp.ApiResponse, err)
	}

	delete(sm.WorkloadsObjs, wr.VeniceWorkload.Name)

	return nil
}

func (sm *VcenterSysModel) TeardownWorkloads(wc *objects.WorkloadCollection) error {
	// Teardown the workloads
	for _, wrk := range wc.Workloads {
		err := sm.deleteWorkload(wrk)
		if err != nil {
			log.Infof("Delete workload failed : %v", err.Error())
			return err
		}
	}

	log.Info("Delete workload successful")
	return nil
}

func getThirdPartyNic(name, mac string) *cluster.DistributedServiceCard {

	return &cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{
			Kind: "DistributedServiceCard",
		},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Spec: cluster.DistributedServiceCardSpec{
			ID: "host-1",
			IPConfig: &cluster.IPConfig{
				IPAddress: "1.2.3.4/32",
			},
			MgmtMode:    "NETWORK",
			NetworkMode: "OOB",
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: "ADMITTED",
			PrimaryMAC:     mac,
			IPConfig: &cluster.IPConfig{
				IPAddress: "1.2.3.4",
			},
		},
	}
}

func (sm *VcenterSysModel) modifyConfig() error {

	cfgObjects := sm.GetCfgObjects()

	orch, err := sm.GetOrchestrator()
	if err != nil {
		log.Errorf("Get orchestrator failed %v", err)
		return err
	}
	//Modify netwrork object with scope
	for _, nw := range cfgObjects.Networks {
		nw.Spec.Orchestrators = []*network.OrchestratorInfo{}
		nw.Spec.Orchestrators = append(nw.Spec.Orchestrators, &network.OrchestratorInfo{
			Name:      orch.Name,
			Namespace: orch.DC,
		})
	}

	return nil
}

// InitConfig sets up a default config for the system
func (sm *VcenterSysModel) InitConfig(scale, scaleData bool) error {
	skipSetup := os.Getenv("SKIP_SETUP")
	skipConfig := os.Getenv("SKIP_CONFIG")
	cfgParams := &base.ConfigParams{
		Scale:      scale,
		Regenerate: skipSetup == "",
		Vlans:      sm.Tb.AllocatedVlans(),
	}
	for _, naples := range sm.NaplesNodes {
		cfgParams.Dscs = append(cfgParams.Dscs, naples.SmartNic)
	}

	index := 0
	for name, node := range sm.ThirdPartyNodes {
		node.Node.Nodeuuid = "50df.9ac7.c24" + fmt.Sprintf("%v", index)
		n := getThirdPartyNic(name, node.Node.Nodeuuid)
		cfgParams.Dscs = append(cfgParams.Dscs, n)
		index++
	}

	for _, naples := range sm.FakeNaples {
		cfgParams.Dscs = append(cfgParams.Dscs, naples.SmartNic)
	}

	err := sm.PopulateConfig(cfgParams)
	if err != nil {
		return err
	}

	if skipConfig == "" {

		orch, err := sm.getOrchestrator()
		if err != nil {
			return err
		}
		sm.orchestrator = orch

		err = sm.DisconnectVeniceAndOrchestrator()
		if err != nil {
			return err
		}
		defer sm.AllowVeniceAndOrchestrator()

		time.Sleep(2 * time.Second)

		err = sm.CleanupAllConfig()
		if err != nil {
			return err
		}

		err = orch.Delete()
		if err != nil {
			//Ignore as it may not be present
			log.Infof("Error deleting orch config %v", err)
		}

		time.Sleep(120 * time.Second)

		err = orch.Commit()
		if err != nil {
			log.Errorf("Error commiting orchestration config %v", err)
			return err
		}

		err = sm.modifyConfig()
		if err != nil {
			return err
		}
		err = sm.PushConfig()
		if err != nil {
			return err
		}
		defer sm.AllowVeniceAndOrchestrator()

		ok, err := sm.IsConfigPushComplete()
		if !ok || err != nil {
			return err
		}

	}

	return nil
}

func getVcenterNetworkName(n *network.Network) string {
	return "#Pen-PG-" + n.Name
}

// Networks returns a list of subnets
func (sm *VcenterSysModel) Networks() *objects.NetworkCollection {
	snc := objects.NetworkCollection{}
	nws, err := sm.CfgModel.ListNetwork("")
	if err != nil {
		log.Errorf("Error listing networks %v", err)
		return nil
	}
	for _, sn := range nws {
		snc.AddSubnet(&objects.Network{Name: getVcenterNetworkName(sn), VeniceNetwork: sn})
	}

	return &snc
}
