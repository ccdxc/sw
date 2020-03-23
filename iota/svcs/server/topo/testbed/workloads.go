package testbed

import (
	"context"
	"errors"
	"fmt"
	"os"
	"strings"
	"time"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/agent/workload"
	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/svcs/common/copier"
	"github.com/pensando/sw/venice/utils/log"
	"golang.org/x/crypto/ssh"
	"golang.org/x/sync/errgroup"
)

var (
	apiSuccess = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK, ErrorMsg: "Api success"}
)

type iotaWorkload struct {
	name        string
	workloadMsg *iota.Workload
	workload    workload.Workload
	SSHClient   *ssh.Client //for copying
}

// AddWorkloads adds a workload on the node
func (n *TestNode) AddWorkloads(w *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {

	if n.triggerLocal {
		return n.AddWorkloadsLocal(w)
	}

	log.Infof("TOPO SVC | DEBUG | STATE | WORKLOAD: %v", w)
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Minute)
	defer cancel()
	resp, err := n.AgentClient.AddWorkloads(ctx, w)
	log.Infof("TOPO SVC | DEBUG | AddWorkload Agent . Received Response Msg: %v", resp)

	if err != nil {
		log.Errorf("Adding workload on node %v failed. Err: %v", n.Node.Name, err)
		return nil, err
	}

	if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Adding workloads on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.ApiResponse.ApiStatus)
		return resp, nil
	}

	//Store all the workloads
	for _, wl := range resp.Workloads {
		iotaWload := iotaWorkload{workloadMsg: wl}
		n.workloadMap.Store(wl.WorkloadName, iotaWload)
	}

	n.WorkloadResp = resp
	return resp, nil
}

var workloadTypeMap = map[iota.WorkloadType]string{
	iota.WorkloadType_WORKLOAD_TYPE_VM: workload.WorkloadTypeVcenter,
	//iota.WorkloadType_WORKLOAD_TYPE_VM_VCENTER: workload.WorkloadTypeVcenter,
}

func (n *TestNode) configureWorkload(wload workload.Workload, in *iota.Workload) (*iota.Workload, error) {

	var intf string

	spec := workload.InterfaceSpec{
		IntfType:      in.GetInterfaceType().String(),
		Parent:        in.GetParentInterface(),
		Name:          in.GetInterface(),
		Mac:           in.GetMacAddress(),
		IPV4Address:   in.GetIpPrefix(),
		IPV6Address:   in.GetIpv6Prefix(),
		PrimaryVlan:   int(in.GetEncapVlan()),
		SecondaryVlan: int(in.GetSecondaryEncapVlan()),
		Switch:        in.GetSwitchName(),
		NetworkName:   in.GetNetworkName(),
	}

	attachedIntf, err := wload.AddInterface(spec)
	if err != nil {
		msg := fmt.Sprintf("Error in Interface attachment %s : %s", in.GetWorkloadName(), err.Error())
		n.logger.Error(msg)
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		return resp, err
	}
	intf = attachedIntf

	err = wload.AddSecondaryIpv4Addresses(intf, in.GetSecIpPrefix())
	if err != nil {
		msg := fmt.Sprintf("Error Adding Secondary IPv4 addresses %s : %s", in.GetWorkloadName(), err.Error())
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		return resp, err
	}

	err = wload.AddSecondaryIpv6Addresses(intf, in.GetSecIpv6Prefix())
	if err != nil {
		msg := fmt.Sprintf("Error Adding Secondary IPv6 addresses %s : %s", in.GetWorkloadName(), err.Error())
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		return resp, err
	}

	/* For SRIOV case, move the parent interface inside the workload so that it is not shared */
	if in.GetInterfaceType() == iota.InterfaceType_INTERFACE_TYPE_SRIOV {
		wload.MoveInterface(in.GetInterface())
	}

	resp := *in
	resp.WorkloadStatus = apiSuccess
	resp.Interface = intf
	return &resp, nil
}

func (n *TestNode) setupWorkload(wload workload.Workload, in *iota.Workload) (*iota.Workload, error) {
	/* Create working directory and set that as base dir */
	n.logger.Println("Doing setup of esx workload")
	os.MkdirAll(constants.DstIotaEntitiesDir, 0755)
	wDir := constants.DstIotaEntitiesDir + "/" + in.GetWorkloadName()
	wload.SetBaseDir(wDir)

	wload.SetConnector(n.ClusterName, wload.Host(), n.connector)
	imageDir, _ := imageRep.GetImageDir(in.GetWorkloadImage())
	if err := wload.BringUp(in.GetWorkloadName(), imageDir,
		n.ClusterName, n.info.IPAddress, constants.IotaAgentBinaryPathLinux); err != nil {
		msg := fmt.Sprintf("Error in workload image bring up : %s : %s", in.GetWorkloadName(), err.Error())
		n.logger.Error(msg)
		resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
		return resp, err
	}

	n.logger.Printf("Bring up workload : %s done", in.GetWorkloadName())

	return n.configureWorkload(wload, in)
}

// AddWorkloadsLocal adds when no agent
func (n *TestNode) AddWorkloadsLocal(in *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	addWorkload := func(in *iota.Workload) *iota.Workload {

		if _, ok := iota.WorkloadType_name[(int32)(in.GetWorkloadType())]; !ok {
			msg := fmt.Sprintf("Workload Type %d not supported", in.GetWorkloadType())
			resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
			return resp
		}

		wloadKey := in.GetWorkloadName()
		var iotaWload iotaWorkload
		log.Infof("Adding workload : %s", in.GetWorkloadName())
		if item, ok := n.workloadMap.Load(wloadKey); ok {
			msg := fmt.Sprintf("Trying to add workload %s, which already exists ", wloadKey)
			log.Error(msg)
			resp, _ := n.configureWorkload(item.(iotaWorkload).workload, in)
			return resp
		}

		wlType, ok := workloadTypeMap[in.GetWorkloadType()]
		if !ok {
			msg := fmt.Sprintf("Workload type %v not found", in.GetWorkloadType())
			n.logger.Error(msg)
			resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
			return resp
		}

		iotaWload.workload = workload.NewWorkload(wlType, in.GetWorkloadName(), n.info.Name, n.logger)

		if iotaWload.workload == nil {
			msg := fmt.Sprintf("Trying to add workload of invalid type : %v", wlType)
			n.logger.Error(msg)
			resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
			return resp
		}

		n.logger.Printf("Setting up workload : %s", in.GetWorkloadName())
		resp, err := n.setupWorkload(iotaWload.workload, in)

		if err != nil || resp.GetWorkloadStatus().GetApiStatus() != iota.APIResponseType_API_STATUS_OK {
			logger.Errorf("Tearing down workload %v", iotaWload.workload.Name())
			iotaWload.workload.TearDown()
			return resp
		}

		if err := iotaWload.workload.SendArpProbe(strings.Split(in.GetIpPrefix(), "/")[0], constants.EsxDataVMInterface,
			0); err != nil {
			msg := fmt.Sprintf("Error in sending arp probe : %s", err.Error())
			n.logger.Error(msg)
			resp = &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR, ErrorMsg: msg}}
			n.workloadMap.Delete(in.GetWorkloadName())
			iotaWload.workload.TearDown()
			return resp
		}

		iotaWload.workloadMsg = in
		resp.MgmtIp = iotaWload.workload.MgmtIP()
		n.workloadMap.Store(wloadKey, iotaWload)
		n.logger.Printf("Added workload : %s (%s) %s", in.GetWorkloadName(), in.GetWorkloadType(), resp)
		return resp
	}

	pool, _ := errgroup.WithContext(context.Background())
	maxParallelThreads := 5
	currThreads := 0
	scheduleWloads := []*iota.Workload{}
	wloadIndex := []int{}

	for index, wload := range in.Workloads {
		currThreads++
		scheduleWloads = append(scheduleWloads, wload)
		wloadIndex = append(wloadIndex, index)
		if currThreads == maxParallelThreads-1 || index+1 == len(in.Workloads) {
			for thread, wl := range scheduleWloads {
				wload := wl
				index := wloadIndex[thread]
				pool.Go(func() error {
					resp := addWorkload(wload)
					in.Workloads[index] = resp
					return nil
				})
			}
			pool.Wait()
			scheduleWloads = []*iota.Workload{}
			wloadIndex = []int{}
			currThreads = 0
		}
	}

	pool.Wait()
	for _, wload := range in.Workloads {
		if wload.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			in.ApiResponse = wload.WorkloadStatus
			return in, nil
		}
	}
	in.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}

	return in, nil
}

// DeleteWorkloadsLocal adds when no agent
func (n *TestNode) DeleteWorkloadsLocal(in *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	deleteWorkload := func(in *iota.Workload) *iota.Workload {

		var ok bool
		var item interface{}
		n.logger.Printf("Deleting workload : %s", in.GetWorkloadName())

		wloadKey := in.GetWorkloadName()

		if item, ok = n.workloadMap.Load(wloadKey); !ok {
			msg := fmt.Sprintf("Trying to delete workload %s which does not exist", wloadKey)
			n.logger.Error(msg)
			resp := &iota.Workload{WorkloadStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: msg}}
			return resp
		}

		item.(iotaWorkload).workload.TearDown()
		n.workloadMap.Delete(wloadKey)
		n.logger.Printf("Deleted workload : %s", in.GetWorkloadName())
		resp := &iota.Workload{WorkloadStatus: apiSuccess}
		return resp

	}

	pool, _ := errgroup.WithContext(context.Background())
	maxParallelThreads := 5
	currThreads := 0
	scheduleWloads := []*iota.Workload{}
	wloadIndex := []int{}

	for index, wload := range in.Workloads {
		currThreads++
		scheduleWloads = append(scheduleWloads, wload)
		wloadIndex = append(wloadIndex, index)
		if currThreads == maxParallelThreads-1 || index+1 == len(in.Workloads) {
			for thread, wl := range scheduleWloads {
				wload := wl
				index := wloadIndex[thread]
				pool.Go(func() error {
					resp := deleteWorkload(wload)
					in.Workloads[index] = resp
					return nil
				})
			}
			pool.Wait()
			scheduleWloads = []*iota.Workload{}
			wloadIndex = []int{}
			currThreads = 0
		}
	}

	pool.Wait()
	for _, wload := range in.Workloads {
		if wload.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			in.ApiResponse = wload.WorkloadStatus
			return in, errors.New(wload.WorkloadStatus.ErrorMsg)
		}

		//Storeall the workloads
		n.workloadMap.Delete(wload.WorkloadName)

	}
	in.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}

	return in, nil
}

// DeleteWorkloads delete  workloads on the node
func (n *TestNode) DeleteWorkloads(w *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {

	if n.triggerLocal {
		return n.DeleteWorkloadsLocal(w)
	}

	if n.AgentClient == nil {
		log.Errorf("Deleting workload on node %v failed (no agent)", n.Node.Name)
		return w, errors.New("No agent initialized")
	}
	log.Infof("TOPO SVC | DEBUG | STATE | WORKLOAD: %v", w)
	resp, err := n.AgentClient.DeleteWorkloads(context.Background(), w)
	log.Infof("TOPO SVC | DEBUG | DeleteWorkload Agent . Received Response Msg: %v", resp)

	if err != nil {
		log.Errorf("Deleting workload on node %v failed. Err: %v", n.Node.Name, err)
		return nil, err
	}

	if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Deleting workloads on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.ApiResponse.ApiStatus)
		//return nil, fmt.Errorf("Deleting workload on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.ApiResponse.ApiStatus)
	}

	for _, wload := range w.Workloads {
		//Delete the workloads
		n.workloadMap.Delete(wload.WorkloadName)

	}

	n.WorkloadResp = resp
	return resp, nil
}

// CopyTo copies a file to the node
func (n *iotaWorkload) CopyTo(cfg *ssh.ClientConfig, dstDir string, files []string) error {
	var copyHandle *copier.Copier

	addr := fmt.Sprintf("%s:%d", n.workload.MgmtIP(), constants.SSHPort)

	if n.SSHClient == nil {
		copyHandle = copier.NewCopier(cfg)
	} else {
		copyHandle = copier.NewCopierWithSSHClient(n.SSHClient, cfg)
	}

	if err := copyHandle.CopyTo(addr, dstDir, files); err != nil {
		log.Errorf("TOPO SVC | CopyTo node %v failed, IPAddress: %v , Err: %v", n.workload.Name(), addr, err)
		return fmt.Errorf("CopyTo node failed, TestNode: %v, IPAddress: %v , Err: %v", n.workload.Name(), addr, err)
	}

	//Update SSH client for future
	if n.SSHClient != nil {
		n.SSHClient = copyHandle.SSHClient
	}
	return nil
}

// CopyFrom copies a file to the node
func (n *iotaWorkload) CopyFrom(cfg *ssh.ClientConfig, dstDir string, files []string) error {
	//copier := copier.NewCopier(cfg)
	var copyHandle *copier.Copier
	if n.SSHClient == nil {
		copyHandle = copier.NewCopier(cfg)
	} else {
		copyHandle = copier.NewCopierWithSSHClient(n.SSHClient, cfg)
	}

	addr := fmt.Sprintf("%s:%d", n.workload.MgmtIP(), constants.SSHPort)

	if err := copyHandle.CopyFrom(addr, dstDir, files); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | CopyFrom node %v failed, IPAddress: %v , Err: %v", n.workload.Name(), addr, err)
		return fmt.Errorf("CopyFrom node failed, TestNode: %v, IPAddress: %v , Err: %v", n.workload.Name(), addr, err)
	}

	//Update SSH client for future
	n.SSHClient = copyHandle.SSHClient

	return nil
}
