package testbed

import (
	"context"
	"fmt"
	"sync"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pkg/errors"
	"golang.org/x/sync/errgroup"
)

func (n *VcenterNode) downloadImages(in *iota.WorkloadMsg) error {

	//First download all VM images which we don't know about
	n.logger.Info("Downloading images...")
	for _, wload := range in.Workloads {
		err := imageRep.DownloadImage(wload.GetWorkloadImage())
		if err != nil {
			return errors.Wrapf(err, "Download image failed")
		}
	}
	return nil
}

// AddWorkloads adds a workload on the vcenter node
func (n *VcenterNode) AddWorkloads(req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {

	workloadNodes := []TestNodeInterface{}
	var wlNode map[string]*iota.WorkloadMsg
	var wlRespNode map[string]*iota.WorkloadMsg
	wlNode = make(map[string]*iota.WorkloadMsg)
	var m sync.Mutex
	wlRespNode = make(map[string]*iota.WorkloadMsg)

	log.Info("Adding workloads via vcenter...")
	req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}
	for _, w := range req.Workloads {
		node := n.GetControlledNode(w.NodeName)
		if node == nil {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("AddWorkloads found to unprovisioned node : %v", w.NodeName)
			return req, nil
		}

		if _, ok := wlNode[w.NodeName]; !ok {
			wlNode[w.NodeName] = &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{}}
		}
		workloadInfo := wlNode[w.NodeName]

		workloadInfo.Workloads = append(workloadInfo.Workloads, w)
		added := false
		for _, workloadNode := range workloadNodes {
			if workloadNode.GetNodeInfo().Name == node.GetNodeInfo().Name {
				added = true
				break
			}
		}
		if !added {
			workloadNodes = append(workloadNodes, node)
		}
	}

	//First download all required images
	if err := n.downloadImages(req); err != nil {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Download Workload failed : %v", err.Error())
		return req, nil
	}

	// Add workloads
	addWorkloads := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)
		for _, node := range workloadNodes {
			node := node
			pool.Go(func() error {
				workloadInfo, _ := wlNode[node.GetNodeInfo().Name]
				resp, err := node.AddWorkloads(workloadInfo)
				m.Lock()
				wlRespNode[node.GetNodeInfo().Name] = resp
				m.Unlock()
				return err
			})

		}
		return pool.Wait()
	}

	err := addWorkloads(context.Background())
	if err != nil {
		log.Errorf("Vcenter | AddWorkloads |AddWorkloads Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Vcenter| AddWorkloads |AddWorkloads Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		return req, nil
	}
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK

	//Assoicate response
	for _, node := range workloadNodes {
		resp, ok := wlRespNode[node.GetNodeInfo().Name]
		if !ok {
			continue
		}
		if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			req.ApiResponse.ErrorMsg = resp.ApiResponse.ErrorMsg
			req.ApiResponse.ApiStatus = resp.ApiResponse.ApiStatus
			log.Errorf("TOPO SVC | AddWorkloads | Workload  failed with  %v", resp.ApiResponse.ErrorMsg)
			break
		}
		for _, respWload := range resp.Workloads {
			for index, reqWload := range req.Workloads {
				if reqWload.GetNodeName() == node.GetNodeInfo().Name && reqWload.WorkloadName == respWload.WorkloadName {
					req.Workloads[index] = respWload
					if respWload.WorkloadStatus != nil && respWload.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
						req.ApiResponse.ErrorMsg = respWload.WorkloadStatus.ErrorMsg
						req.ApiResponse.ApiStatus = respWload.WorkloadStatus.ApiStatus
						log.Errorf("Vcenter | AddWorkloads | Workload add %v failed with  %v", respWload.GetWorkloadName(), req.ApiResponse.ErrorMsg)
					}
					break
				}
			}
		}

	}

	return req, nil
}

// DeleteWorkloads adds a workload on the vcenter node
func (n *VcenterNode) DeleteWorkloads(req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {

	workloadNodes := []TestNodeInterface{}
	var wlNode map[string]*iota.WorkloadMsg
	var wlRespNode map[string]*iota.WorkloadMsg
	wlNode = make(map[string]*iota.WorkloadMsg)
	var m sync.Mutex
	wlRespNode = make(map[string]*iota.WorkloadMsg)

	log.Info("Deleting workloads via vcenter...")
	req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}
	for _, w := range req.Workloads {
		node := n.GetControlledNode(w.NodeName)
		if node == nil {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("AddWorkloads found to unprovisioned node : %v", w.NodeName)
			return req, nil
		}

		if _, ok := wlNode[w.NodeName]; !ok {
			wlNode[w.NodeName] = &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{}}
		}
		workloadInfo := wlNode[w.NodeName]

		workloadInfo.Workloads = append(workloadInfo.Workloads, w)
		added := false
		for _, workloadNode := range workloadNodes {
			if workloadNode.GetNodeInfo().Name == node.GetNodeInfo().Name {
				added = true
				break
			}
		}
		if !added {
			workloadNodes = append(workloadNodes, node)
		}
	}

	// Delete workloads
	deleteWorkloads := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)
		for _, node := range workloadNodes {
			node := node
			pool.Go(func() error {
				workloadInfo, _ := wlNode[node.GetNodeInfo().Name]
				resp, err := node.DeleteWorkloads(workloadInfo)
				m.Lock()
				wlRespNode[node.GetNodeInfo().Name] = resp
				m.Unlock()
				return err
			})

		}
		return pool.Wait()
	}

	err := deleteWorkloads(context.Background())
	if err != nil {
		log.Errorf("Vcenter | DeleteWorkloads |DeleteWorkloads Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Vcenter| DeleteWorkloads |DeleteWorkloads Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		return req, nil
	}
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK

	//Assoicate response
	for _, node := range workloadNodes {
		resp, ok := wlRespNode[node.GetNodeInfo().Name]
		if !ok {
			continue
		}
		for _, respWload := range resp.Workloads {
			for index, reqWload := range req.Workloads {
				if reqWload.GetNodeName() == node.GetNodeInfo().Name && reqWload.WorkloadName == respWload.WorkloadName {
					req.Workloads[index] = respWload
					if respWload.WorkloadStatus != nil && respWload.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
						req.ApiResponse.ErrorMsg = respWload.WorkloadStatus.ErrorMsg
						log.Errorf("Vcenter | DeleteWorkloads | Workload add %v failed with  %v", respWload.GetWorkloadName(), req.ApiResponse.ErrorMsg)
					}
					break
				}
			}
		}

	}

	return req, nil
}

//MoveWorkloads workload move not supported.
func (n *VcenterNode) MoveWorkloads(ctx context.Context, req *iota.WorkloadMoveMsg) (*iota.WorkloadMoveMsg, error) {

	n.Lock()
	defer n.Unlock()
	req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}

	moveKey := func(moveReq *iota.WorkloadMove) string {
		return moveReq.WorkloadName + "-" + moveReq.DstNodeName
	}
	type moveRequest struct {
		srcNodeName  string
		dstNodeName  string
		srcHost      string
		dstHost      string
		workloadName string
		vlanOverride uint32
		err          error
	}
	dupCheck := make(map[string]bool)
	moveRequests := []*moveRequest{}
	for _, mvReq := range req.GetWorkloadMoves() {

		mSrcNode, ok := n.managedNodes[mvReq.SrcNodeName]
		if !ok {
			msg := fmt.Sprintf("Did not find  src node %s for workload move ", mvReq.SrcNodeName)
			log.Error(msg)
			req.ApiResponse.ErrorMsg = msg
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			return req, nil
		}

		mvDstNode, ok := n.managedNodes[mvReq.DstNodeName]
		if !ok {
			msg := fmt.Sprintf("Did not find dst node %s for workload move ", mvReq.DstNodeName)
			log.Error(msg)
			req.ApiResponse.ErrorMsg = msg
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			return req, nil
		}

		//Check whether workload exisits in src
		_, ok = mSrcNode.(*EsxNode).workloadMap.Load(mvReq.WorkloadName)
		if !ok {
			msg := fmt.Sprintf("Did not find workload %v on src node %s for workload move ", mvReq.WorkloadName, mvReq.SrcNodeName)
			log.Error(msg)
			req.ApiResponse.ErrorMsg = msg
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			return req, nil
		}
		key := moveKey(mvReq)
		if _, ok := dupCheck[key]; ok {
			msg := fmt.Sprintf("Duplicate workload move request  %v %v -> %v", mvReq.WorkloadName, mvReq.SrcNodeName, mvReq.DstNodeName)
			log.Error(msg)
			req.ApiResponse.ErrorMsg = msg
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			return req, nil
		}
		dupCheck[key] = true

		moveRequests = append(moveRequests, &moveRequest{srcHost: mSrcNode.GetNodeInfo().IPAddress,
			srcNodeName: mvReq.SrcNodeName, dstNodeName: mvReq.DstNodeName, vlanOverride: mvReq.VlanOverride,
			dstHost: mvDstNode.GetNodeInfo().IPAddress, workloadName: mvReq.WorkloadName})

		//All Good to trigger vmotion
	}

	// Add workloads
	moveWorkloads := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)
		for _, mvReq := range moveRequests {
			mvReq := mvReq
			pool.Go(func() error {
				err := n.dc.LiveMigrate(mvReq.workloadName,
					mvReq.srcHost, mvReq.dstHost, n.ClusterName)
				if err != nil {
					msg := fmt.Sprintf("Workload migrate Name : %v, Src : %v, Dst %v failed : %v",
						mvReq.workloadName, mvReq.srcHost, mvReq.dstHost, err.Error())
					log.Error(msg)
					mvReq.err = errors.New(msg)
				}
				return err
			})

		}
		return pool.Wait()
	}

	moveWorkloads(ctx)

	//AssociateResponse now
	for index, mvReq := range moveRequests {
		if mvReq.err != nil {
			req.WorkloadMoves[index].ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}
			req.WorkloadMoves[index].ApiResponse.ErrorMsg = mvReq.err.Error()
		} else {
			req.WorkloadMoves[index].ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}
			//Update the local workload state

			//Swap the workloads as move is complete
			mSrcNode, _ := n.managedNodes[mvReq.srcNodeName]
			mDstNode, _ := n.managedNodes[mvReq.dstNodeName]
			iotaWload, _ := mSrcNode.(*EsxNode).workloadMap.Load(mvReq.workloadName)
			wload := iotaWload.(iotaWorkload)
			wload.workloadMsg.NodeName = mvReq.dstNodeName
			mDstNode.(*EsxNode).workloadMap.Store(mvReq.workloadName, iotaWload)
			mSrcNode.(*EsxNode).workloadMap.Delete(mvReq.workloadName)

		}
	}

	return req, nil
}
