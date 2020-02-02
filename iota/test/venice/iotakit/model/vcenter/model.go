package vcenter

import (
	"context"
	"errors"
	"fmt"
	"os"
	"strings"

	"github.com/pensando/sw/api/generated/workload"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/enterprise"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

//VcenterSysModel representing model for vcenter
type VcenterSysModel struct {
	enterprise.SysModel
}

//GetOrchestrator returns orchestroa
func (sm *VcenterSysModel) GetOrchestrator() (*objects.Orchestrator, error) {

	for _, node := range sm.Tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VCENTER_NODE {
			return &objects.Orchestrator{
				Username: sm.Tb.Params.Provision.Vars["VcenterUsername"],
				Password: sm.Tb.Params.Provision.Vars["VcenterPassword"],
				License:  sm.Tb.Params.Provision.Vars["VcenterLicense"],
				IP:       node.NodeMgmtIP,
				Name:     node.NodeName,
			}, nil
		}
	}

	return nil, errors.New("Vcenter orchestrator not found")
}

//SetupWorkloads bring up.Workloads on host
func (sm *VcenterSysModel) SetupWorkloads(scale bool) error {

	var wc objects.WorkloadCollection

	hosts, err := sm.ListRealHosts()
	if err != nil {
		log.Error("Error finding real hosts to run traffic tests")
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
				wl := objects.NewWorkload(host, workloadObj, sm.Tb.Topo.WorkloadType, sm.Tb.Topo.WorkloadImage)
				sm.WorkloadsObjs[workloadObj.Name] = wl
				if err != nil {
					log.Errorf("Error creating workload %v", workloadObj)
					return err
				}
				sm.WorkloadsObjs[workloadObj.Name].SetIotaWorkload(gwrk)
				wc.Workloads = append(wc.Workloads, wl)
			}

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

	if err := sm.SetupDefaultCommon(ctx, scale, scaleData); err != nil {
		return err
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
