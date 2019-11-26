// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// TechSupportRPCServer is the techsupport RPC server
type TechSupportRPCServer struct {
	stateMgr *statemgr.Statemgr
}

func memdbToAPIEventType(in memdb.EventType) api.EventType {
	switch in {
	case memdb.CreateEvent:
		return api.EventType_CreateEvent
	case memdb.UpdateEvent:
		return api.EventType_UpdateEvent
	case memdb.DeleteEvent:
		return api.EventType_DeleteEvent
	default:
		log.Errorf("Unknown memdb event %v", in)
		return api.EventType(-1)
	}
}

// buildNodeTechSupportRequest returns a TechSupportRequest for a node given the originating controller request
func buildNodeTechSupportRequest(tsr *monitoring.TechSupportRequest, nodeName string) *tsproto.TechSupportRequest {
	spec := &tsproto.TechSupportRequestSpec{
		InstanceID: tsr.Status.InstanceID,
		Labels:     tsr.Spec.CollectionSelector,
		Destinations: []*tsproto.DestinationSpec{
			&tsproto.DestinationSpec{
				Proto: tsproto.DestinationSpec_Venice,
			},
		},
		Verbosity: tsr.Spec.Verbosity,
	}
	atsr := &tsproto.TechSupportRequest{
		TypeMeta:   tsr.TypeMeta,
		ObjectMeta: tsr.ObjectMeta,
		Spec:       *spec,
	}
	return atsr
}

// a request is active (needs to be worked on) if it is not completed or failed
func isActiveTechSupportRequest(tsr *monitoring.TechSupportRequest) bool {
	return tsr.Status.Status != monitoring.TechSupportJobStatus_Completed.String() &&
		tsr.Status.Status != monitoring.TechSupportJobStatus_Failed.String()
}

// listTechSupportRequestsActive list only requests that are not completed
func (r *TechSupportRPCServer) listTechSupportRequestsActive(ctx context.Context, nodeName, nodeKind string) ([]*statemgr.TechSupportRequestState, error) {
	var tsrsList []*statemgr.TechSupportRequestState
	objsList := r.stateMgr.ListTechSupportObjectState(statemgr.KindTechSupportRequest)
	// Walk all the techsupport requests and add active (not completed, not failed) sessions to the list
	for _, objs := range objsList {
		tsrs := objs.(*statemgr.TechSupportRequestState)
		tsrs.Lock()
		tsr := tsrs.TechSupportRequest
		if isActiveTechSupportRequest(tsr) && r.nodeSelectorMatch(nodeName, nodeKind, tsr.Spec.NodeSelector) {
			tsrsList = append(tsrsList, tsrs)
		}
		tsrs.Unlock()
	}
	return tsrsList, nil
}

func (r *TechSupportRPCServer) nodeSelectorMatch(nodeName, kind string, selector *monitoring.TechSupportRequestSpec_NodeSelectorSpec) bool {
	// this may match either a Controller and a SmartNIC node
	for _, n := range selector.Names {
		if n == nodeName {
			return true
		}
	}

	// if there was no match in the node names list, we need to retrieve the node object and
	// check object labels against the selector
	if selector.Labels != nil {
		nodeState, err := r.stateMgr.FindTechSupportObject(nodeName, "", kind)
		if err != nil {
			log.Errorf("Error searching object with name %s kind %s in statemgr: %v", nodeName, kind, err)
			return false
		}
		nodeState.Lock()
		defer nodeState.Unlock()
		objMeta := nodeState.GetObjectMeta()
		if objMeta.Labels != nil {
			return selector.Labels.Matches(labels.Set(objMeta.Labels))
		}
	}

	return false
}

func getTechSupportNodeResult(tsr *monitoring.TechSupportRequest, nodeName, nodeKind string) *monitoring.TechSupportNodeResult {
	if nodeKind == statemgr.KindSmartNICNode && tsr.Status.DSCResults != nil {
		return tsr.Status.DSCResults[nodeName]
	} else if nodeKind == statemgr.KindControllerNode && tsr.Status.ControllerNodeResults != nil {
		return tsr.Status.ControllerNodeResults[nodeName]
	}
	return nil
}

// updateTechSupportNodeResult updates the node-specific result in a TechSupportRequest, recomputes the overall result status
// and pushes the update back to StateMgr and from there to ApiServer
func (r *TechSupportRPCServer) updateTechSupportNodeResult(tsr *monitoring.TechSupportRequest, nodeName, nodeKind, instanceID string, result *tsproto.TechSupportRequestStatus) error {
	if tsr.Status.InstanceID != "" && tsr.Status.InstanceID != instanceID {
		log.Infof("Ignoring update for old request. Node name: %s Req Name: %s, Req ID: %s, Upd ID: %s",
			nodeName, tsr.ObjectMeta.Name, tsr.Status.InstanceID, instanceID)
		return nil
	}

	var jobStatus string

	switch result.Status {
	case tsproto.TechSupportRequestStatus_Queued, tsproto.TechSupportRequestStatus_Scheduled:
		jobStatus = monitoring.TechSupportJobStatus_Scheduled.String()

	case tsproto.TechSupportRequestStatus_InProgress:
		jobStatus = monitoring.TechSupportJobStatus_Running.String()
		// If at least 1 job picked up the request, it is running
		if tsr.Status.Status == monitoring.TechSupportJobStatus_Scheduled.String() {
			tsr.Status.Status = monitoring.TechSupportJobStatus_Running.String()
		}

	case tsproto.TechSupportRequestStatus_Completed:
		jobStatus = monitoring.TechSupportJobStatus_Completed.String()

	case tsproto.TechSupportRequestStatus_Failed:
		jobStatus = monitoring.TechSupportJobStatus_Failed.String()
		// At least 1 job failed, so the request as a whole failed.
		tsr.Status.Status = monitoring.TechSupportJobStatus_Failed.String()

	default:
		log.Errorf("Unknown jobStatus %v", result.Status)
	}

	updResult := &monitoring.TechSupportNodeResult{
		StartTime: result.StartTime,
		EndTime:   result.EndTime,
		Status:    jobStatus,
		URI:       result.URI,
		Reason:    result.Reason,
	}

	switch nodeKind {
	case statemgr.KindSmartNICNode:
		tsr.Status.DSCResults[nodeName] = updResult
	case statemgr.KindControllerNode:
		tsr.Status.ControllerNodeResults[nodeName] = updResult
	default:
		return fmt.Errorf("Unknown node kind: %s", nodeKind)
	}

	// If all jobs completed, the request as a whole is completed
	isComplete := func(results map[string]*monitoring.TechSupportNodeResult) bool {
		for _, s := range results {
			if s.Status != monitoring.TechSupportJobStatus_Completed.String() {
				return false
			}
		}
		return true
	}

	state, err := r.stateMgr.GetTechSupportObjectState(tsr)
	if err != nil {
		log.Infof("No state found for object %s, cannot close context.", tsr.GetObjectMeta().Name)
	}

	if isComplete(tsr.Status.DSCResults) && isComplete(tsr.Status.ControllerNodeResults) {
		for _, key := range tsr.Spec.NodeSelector.Names {
			_, okSN := tsr.Status.DSCResults[key]
			_, okCN := tsr.Status.ControllerNodeResults[key]

			if !okSN && !okCN {
				tsr.Status.Status = monitoring.TechSupportJobStatus_Failed.String()
				goto exit
			}
		}

		if len(tsr.Status.ControllerNodeResults) > 0 {
			log.Infof("Techsupport for controller nodes completed. Collecting config snapshot.")
			err = r.stateMgr.CreateSnapshot(tsr)
			if err != nil {
				log.Errorf("Failed to create snapshot. Err : %v", err)
			}
		}

		tsr.Status.Status = monitoring.TechSupportJobStatus_Completed.String()
		if err != nil {
			log.Infof("No state found for object %s, cannot close context.", tsr.GetObjectMeta().Name)
		}

		err = r.stateMgr.UpdateTechSupportObject(tsr)
		// If we are executing these lines, tsr is guaranteed to be of techsupport kind.
		state.(*statemgr.TechSupportRequestState).CancelFunc()
		return err
	}

exit:
	return r.stateMgr.UpdateTechSupportObject(tsr)
}

// NewTechSupportRPCServer returns a RPC server for TechSupport API
func NewTechSupportRPCServer(stateMgr *statemgr.Statemgr) *TechSupportRPCServer {
	return &TechSupportRPCServer{stateMgr: stateMgr}
}

// WatchTechSupportRequests watches TechSupportRequest objects for changes and sends them as streaming rpc to nodes
func (r *TechSupportRPCServer) WatchTechSupportRequests(params *tsproto.WatchTechSupportRequestsParameters, stream tsproto.TechSupportApi_WatchTechSupportRequestsServer) error {
	/*  This is the implemented behavior:
	- TechSupportRequest objects can only be created and deleted by user, not updated,
		so the node selector (name + label selector) is immutable
	- When a new TechSupportRequest object is created, the node selector is
		evaluated and all selected nodes are notified
	- If a new node selector comes along and it matches the node selector, the notification is sent only if
		the TechSupportRequest is in Running state. It is not sent if it is Completed or Failed.

	This function always attempt to proceed in the presence of errors, except for the case in which
	reading from the event channel returns an error
	*/

	nodeName := params.NodeName
	if nodeName == "" {
		return fmt.Errorf("NodeName not provided in WatchTechSupportRequests call")
	}

	nodeKind := params.NodeKind
	if nodeKind != statemgr.KindControllerNode && nodeKind != statemgr.KindSmartNICNode {
		return fmt.Errorf("Invalid NodeKind %s in WatchTechSupportRequests call", nodeKind)
	}

	watcher := memdb.Watcher{Name: "tech-support"}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	defer close(watcher.Channel)
	r.stateMgr.WatchObjects(statemgr.KindTechSupportRequest, &watcher)
	defer r.stateMgr.StopWatchObjects(statemgr.KindTechSupportRequest, &watcher)

	tsrStateList, err := r.listTechSupportRequestsActive(context.Background(), nodeName, nodeKind)
	if err != nil {
		log.Errorf("Error getting list of TechSupportRequests. Err: %v", err)
	}

	tsrEventList := tsproto.TechSupportRequestEventList{}
	for _, tsrState := range tsrStateList {
		tsrState.Lock()
		tsr := tsrState.TechSupportRequest

		tsrEventList.Events = append(tsrEventList.Events,
			&tsproto.TechSupportRequestEvent{
				EventType: api.EventType_CreateEvent,
				Request:   buildNodeTechSupportRequest(tsr, nodeName),
			})

		if getTechSupportNodeResult(tsr, nodeName, nodeKind) == nil {
			err := r.updateTechSupportNodeResult(tsr, nodeName, nodeKind, tsr.Status.InstanceID, &tsproto.TechSupportRequestStatus{Status: tsproto.TechSupportRequestStatus_Queued})
			if err != nil {
				log.Errorf("Error updating TechSupportRequest state in ApiServer, nodeName: %s, tsr: %+v, err: %v", nodeName, tsr, err)
			}
		}
		log.Infof("Sending event %v to node %s for TechSupportRequest %v", api.EventType_CreateEvent, nodeName, tsr)
		tsrState.Unlock()
	}

	if len(tsrEventList.Events) > 0 {
		err = stream.Send(&tsrEventList)
		if err != nil {
			log.Errorf("Error sending TechSupportRequest events list: %v", err)
		}
	}

	// Receives notifications from StateMgr when a TechSupportRequest object is created/deleted
	ctx := stream.Context()
	for {
		select {

		case evt, ok := <-watcher.Channel:
			if !ok {
				log.Errorf("Error reading from TechSupportRequest update channel: %v", err)
				return errors.New("Error reading from TechSupportRequest update channel")
			}

			eventType := memdbToAPIEventType(evt.EventType)
			// skip update notifications because spec changes are not allowed
			// and we are not interested in our own status changes
			if eventType == api.EventType_UpdateEvent {
				continue
			}

			tsrs := evt.Obj.(*statemgr.TechSupportRequestState)
			tsrs.Lock()
			if !r.nodeSelectorMatch(nodeName, nodeKind, tsrs.TechSupportRequest.Spec.NodeSelector) {
				tsrs.Unlock()
				continue
			}
			nodeTSR := buildNodeTechSupportRequest(tsrs.TechSupportRequest, nodeName)
			if eventType == api.EventType_CreateEvent {
				r.updateTechSupportNodeResult(tsrs.TechSupportRequest, nodeName, nodeKind, nodeTSR.Spec.InstanceID, &tsproto.TechSupportRequestStatus{Status: tsproto.TechSupportRequestStatus_Scheduled})
			}
			tsrs.Unlock()

			tsrEventList := tsproto.TechSupportRequestEventList{
				Events: []*tsproto.TechSupportRequestEvent{
					{
						EventType: eventType,
						Request:   nodeTSR,
					},
				},
			}
			log.Infof("Sending event %v to node %s for TechSupportRequest %s", eventType, nodeName, nodeTSR.GetObjectMeta().Name)
			err = stream.Send(&tsrEventList)
			if err != nil {
				log.Errorf("Error sending event %v to node %s for TechSupportRequest %s", eventType, nodeName, nodeTSR.GetObjectMeta().Name)
			}

		case <-ctx.Done():
			return ctx.Err()
		}
	}
}

// UpdateTechSupportResult receives a status update from the node and pushes it to ApiServer
func (r *TechSupportRPCServer) UpdateTechSupportResult(ctx context.Context, params *tsproto.UpdateTechSupportResultParameters) (*tsproto.UpdateTechSupportResultResponse, error) {
	nodeTSR := params.Request
	nodeName := params.NodeName
	nodeKind := params.NodeKind

	objState, err := r.stateMgr.GetTechSupportObjectState(nodeTSR)
	if err != nil {
		log.Errorf("Error updating status for request %+v", nodeTSR)
		return nil, fmt.Errorf("Error updating status for request %s", nodeTSR.ObjectMeta.Name)
	}
	objState.Lock()
	defer objState.Unlock()
	ctrlrTSR := objState.(*statemgr.TechSupportRequestState).TechSupportRequest
	err = r.updateTechSupportNodeResult(ctrlrTSR, nodeName, nodeKind, nodeTSR.Spec.InstanceID, &nodeTSR.Status)
	if err != nil {
		log.Errorf("Error updating status for request %+v: %+v", nodeTSR, err)
		return nil, fmt.Errorf("Error updating status for request %s", nodeName)
	}

	return &tsproto.UpdateTechSupportResultResponse{Status: "ok"}, nil
}
