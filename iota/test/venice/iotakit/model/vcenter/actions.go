// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package vcenter

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
	libstrconv "github.com/pensando/sw/venice/utils/strconv"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

const maxOpTimeout = 20 * time.Minute

func (sm *VcenterSysModel) Cleanup() error {
	// collect all log files
	sm.CollectLogs()
	sm.Tb.Cleanup()
	return nil
}

// VerifyClusterStatus verifies venice cluster status
func (sm *VcenterSysModel) VerifyClusterStatus() error {
	return sm.SysModel.VerifyClusterStatus()
}

// VerifyPolicyStatus verifies SG policy status
func (sm *VcenterSysModel) VerifyPolicyStatus(spc *objects.NetworkSecurityPolicyCollection) error {
	return sm.SysModel.VerifyPolicyStatus(spc)
}

// VerifySystemHealth checks all aspects of system, like cluster, workload, policies etc
func (sm *VcenterSysModel) VerifySystemHealth(collectLogOnErr bool) error {
	const numRetries = 10
	// verify cluster is in good health
	err := sm.VerifyClusterStatus()
	if err != nil {
		if collectLogOnErr {
			sm.CollectLogs()
		}
		return err
	}

	return nil
	//Verify Config is in sync

	for i := 0; i < numRetries; i++ {
		var done bool
		done, err = sm.IsConfigPushComplete()
		if done && err == nil {
			break
		}
		time.Sleep(5 * time.Second)

		err = errors.New("Config push incomplete")
	}
	if err != nil {
		return err
	}

	// verify policy status is good
	for i := 0; i < numRetries; i++ {
		err = sm.VerifyPolicyStatus(sm.SGPolicies())
		if err == nil {
			break
		}
		time.Sleep(time.Second)
	}
	if err != nil {
		if collectLogOnErr {
			sm.CollectLogs()
		}
		return err
	}

	// verify ping is successful across all workloads
	if sm.Tb.HasNaplesHW() {
		return nil
		for i := 0; i < numRetries; i++ {
			err = sm.PingPairs(sm.WorkloadPairs().WithinNetwork())
			if err == nil {
				break
			}
		}
		if err != nil {
			if collectLogOnErr {
				sm.CollectLogs()
			}
			return err
		}
	}

	return nil
}

// VerifyWorkloadStatus verifies workload status in venice
func (sm *VcenterSysModel) VerifyWorkloadStatus(wc *objects.WorkloadCollection) error {
	if wc.HasError() {
		return wc.Error()
	}

	//Read the workloads again
	//wc = wc.Read()

	log.Infof("TODO: Workload verfiication Skipping all workload verifcations ")
	return nil
	for _, wr := range wc.Workloads {

		wsts, err := sm.GetWorkload(&wr.VeniceWorkload.ObjectMeta)
		if err != nil {
			log.Errorf("Could not get workload %v. Err: %v", wr.VeniceWorkload.Name, err)
			return err
		}

		log.Debugf("Got workload status: %+v", wsts)

		if wsts.Spec.HostName != wr.VeniceWorkload.Spec.HostName {
			return fmt.Errorf("Invalid hostname on workload %v. Exp: %v, Got: %v", wr.VeniceWorkload.Name, wr.VeniceWorkload.Spec.HostName, wsts.Spec.HostName)
		}
		if len(wsts.Spec.Interfaces) != len(wr.VeniceWorkload.Spec.Interfaces) {
			return fmt.Errorf("Unexpected number of interfaces on workload %v. Exp: %+v, Got: %+v", wr.VeniceWorkload.Name, wr.VeniceWorkload.Spec.Interfaces, wsts.Spec.Interfaces)
		}

		name, err := libstrconv.ParseMacAddr(wr.VeniceWorkload.Spec.Interfaces[0].MACAddress)
		if err != nil {
			name = wr.VeniceWorkload.Spec.Interfaces[0].MACAddress
		}
		epMeta := api.ObjectMeta{
			Tenant:    wr.VeniceWorkload.Tenant,
			Namespace: wr.VeniceWorkload.Namespace,
			Name:      wr.VeniceWorkload.Name + "-" + name,
		}

		// get the endpoints for the workload
		ep, err := sm.GetEndpoint(&epMeta)
		if err != nil {
			log.Errorf("Could not get endpoint %v. Err: %v", epMeta.Name, err)
			return err
		}

		if ep.Status.MacAddress != wr.VeniceWorkload.Spec.Interfaces[0].MACAddress {
			return fmt.Errorf("Invalid mac address %v for workload %v", ep.Status.MacAddress, wr.VeniceWorkload.Name)
		}
		if ep.Status.HomingHostName != wr.VeniceWorkload.Spec.HostName {
			return fmt.Errorf("Invalid host name %v for endpoint on workload %v", ep.Status.HomingHostName, wr.VeniceWorkload.Name)
		}
		if ep.Status.NodeUUID != wr.NaplesUUID() {
			return fmt.Errorf("Invalid node uuid %v for endpoint on workload %v", ep.Status.NodeUUID, wr.VeniceWorkload.Name)
		}

		if wr.GetIotaWorkload().EncapVlan != ep.Status.MicroSegmentVlan {
			return fmt.Errorf("Micosegment vlan does not match for workload %v, Exoected %v, actual %v",
				ep.Status.NodeUUID, wr.GetIotaWorkload().EncapVlan, ep.Status.MicroSegmentVlan)
		}

		//TODO, add more stuff for endpoint status.
	}

	return nil
}

// GetVeniceWorkloadByDisplayName gets venices workload using display name
func (sm *VcenterSysModel) GetVeniceWorkloadByDisplayName(name string) (*workload.Workload, error) {
	wll, err := sm.ObjClient().ListWorkload()
	if err != nil {
		log.Errorf("Could not get workload list from Venice. Err: %v", err)
		return nil, err
	}
	for _, w := range wll {
		if w.Labels == nil {
			continue
		}
		if dispName, ok := w.Labels["io.pensando.vcenter.display-name"]; ok {
			if dispName == name {
				return w, nil
			}
		}
	}
	return nil, nil
}

// VerifyWorkloadMigrationStatus verifies workload migration status in venice
func (sm *VcenterSysModel) VerifyWorkloadMigrationStatus(wc *objects.WorkloadCollection) error {
	if wc.HasError() {
		return wc.Error()
	}

	// log.Infof("TODO: Workload verfiication Skipping all workload verifcations ")
	// return nil
	for _, wr := range wc.Workloads {
		wsts, err := sm.GetWorkload(&wr.VeniceWorkload.ObjectMeta)
		if err != nil {
			log.Errorf("Could not get workload %v. Err: %v", wr.VeniceWorkload.Name, err)
			return err
		}
		log.Debugf("Got workload status: %+v", wsts)
		vwl, err := sm.GetVeniceWorkloadByDisplayName(wr.VeniceWorkload.Name)
		if err != nil {
			log.Errorf("Could not get workload from Venice %+v", wr.VeniceWorkload.ObjectMeta)
			return err
		}
		// check the migration status
		if vwl.Status.MigrationStatus == nil {
			log.Infof("Got workload : %+v", vwl)
			return fmt.Errorf("Workload %v did not migrate", wr.VeniceWorkload.Name)
		}
		if vwl.Status.MigrationStatus.Stage != workload.WorkloadMigrationStatus_MIGRATION_DONE.String() {
			log.Infof("Got workload : %+v", vwl)
			return fmt.Errorf("Workload %v migration.stage is not correct", wr.VeniceWorkload.Name)
		}
		if vwl.Status.MigrationStatus.Status != workload.WorkloadMigrationStatus_DONE.String() {
			log.Infof("Got workload : %+v", vwl)
			return fmt.Errorf("Workload %v migration.status is not correct", wr.VeniceWorkload.Name)
		}
	}
	return nil
}

//MoveWorkloads not supported here
func (sm *VcenterSysModel) MoveWorkloads(wc *objects.WorkloadCollection, hc *objects.HostCollection) error {

	if len(hc.Hosts) != 1 {
		return fmt.Errorf("Invalid number of hosts for move %v", len(hc.Hosts))
	}

	for len(wc.Workloads) == 0 {
		return fmt.Errorf("No workloads selected for move")
	}

	orch, err := sm.GetOrchestrator()
	if err != nil {
		return err
	}
	wMove := &iota.WorkloadMoveMsg{
		OrchestratorNode: orch.Name,
	}
	for _, w := range wc.Workloads {
		wMove.WorkloadMoves = append(wMove.WorkloadMoves, &iota.WorkloadMove{
			WorkloadName: w.Name(),
			DstNodeName:  hc.Hosts[0].Name(),
			SrcNodeName:  w.NodeName(),
		})
	}

	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)

	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	moveResp, err := topoClient.MoveWorkloads(ctx, wMove)
	cancel()

	if err != nil {
		return fmt.Errorf("Workload Move failed. API Status: err: %v", err)
	} else if moveResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Workload Move failed.. API Status: resp: %+v", moveResp.ApiResponse)
	}

	for _, mv := range moveResp.WorkloadMoves {
		if mv.ApiResponse.ApiStatus == iota.APIResponseType_API_STATUS_OK {
			wObj, ok := sm.WorkloadsObjs[mv.WorkloadName]
			if !ok {
				log.Errorf("Workload moved, but not found in model %v", mv.WorkloadName)
			} else {
				wObj.SetNodeName(mv.DstNodeName)
			}
		}
	}

	return nil
}

// GetFwLogObjectCount gets the object count for firewall logs under the bucket with the given name
func (sm *VcenterSysModel) GetFwLogObjectCount(tenantName string, bucketName string, objectKeyPrefix string) (int, error) {
	return 0, fmt.Errorf("not implemented")
}

// GetExclusiveServices node on the fly
func (sm *VcenterSysModel) GetExclusiveServices() ([]string, error) {
	return []string{"pen-orchhub"}, nil
}

//RemoveNetworks remove networks from switch
func (sm *VcenterSysModel) RemoveNetworks(switchName string) error {

	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)

	orch, err := sm.GetOrchestrator()
	if err != nil {
		return err
	}

	removeNetworkMsg := &iota.NetworksMsg{
		Switch:           switchName,
		OrchestratorNode: orch.Name,
	}

	removeResp, err := topoClient.RemoveNetworks(context.Background(), removeNetworkMsg)
	if err != nil {
		return fmt.Errorf("Failed to remove networks Err: %v", err)
	} else if removeResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to rremove networks API Status: %+v | Err: %v", removeResp.ApiResponse, err)
	}

	return nil
}
