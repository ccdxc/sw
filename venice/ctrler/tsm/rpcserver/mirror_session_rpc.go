// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"errors"
	"io"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/tsm/rpcserver/tsproto"
	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

const maxWatchEventsPerMesage = 16

// MirrorSessionRPCServer is the mirror session RPC server
type MirrorSessionRPCServer struct {
	stateMgr *statemgr.Statemgr
}

func buildNICMirrorSession(mss *statemgr.MirrorSessionState) *tsproto.MirrorSession {
	ms := mss.MirrorSession
	tms := tsproto.MirrorSession{
		TypeMeta:   ms.TypeMeta,
		ObjectMeta: ms.ObjectMeta,
		Status:     tsproto.MirrorSessionStatus{},
	}
	tSpec := &tms.Spec
	tSpec.CaptureAt = tsproto.MirrorSrcDst_SRC_DST
	tSpec.PacketDir = tsproto.MirrorDir_BOTH
	tSpec.Enable = (mss.State == monitoring.MirrorSessionState_RUNNING)
	tSpec.PacketSize = ms.Spec.PacketSize
	tSpec.StopConditions.MaxPacketCount = ms.Spec.StopConditions.MaxPacketCount
	for _, c := range ms.Spec.Collectors {
		tc := tsproto.MirrorCollector{
			Type:      c.Type,
			ExportCfg: c.ExportCfg,
		}
		tSpec.Collectors = append(tSpec.Collectors, tc)
	}
	// XXX Move validations to apiServer
	if len(tSpec.Collectors) == 0 {
		log.Debugf("No collectors... Ignore the mirror session %v", ms.Name)
		return nil
	}
	for _, mr := range ms.Spec.MatchRules {
		tmr := tsproto.MatchRule{}
		if mr.Src == nil && mr.Dst == nil {
			log.Debugf("Ignore MatchRule with Src = * and Dst = *")
			continue
		}
		if mr.Src != nil {
			tmr.Src = &tsproto.MatchSelector{}
			tmr.Src.Endpoints = mr.Src.Endpoints
			tmr.Src.IPAddresses = mr.Src.IPAddresses
			tmr.Src.MACAddresses = mr.Src.MACAddresses
		}
		if mr.Dst != nil {
			tmr.Dst = &tsproto.MatchSelector{}
			tmr.Dst.Endpoints = mr.Dst.Endpoints
			tmr.Dst.IPAddresses = mr.Dst.IPAddresses
			tmr.Dst.MACAddresses = mr.Dst.MACAddresses
		}
		if mr.AppProtoSel != nil {
			tmr.AppProtoSel = &tsproto.AppProtoSelector{}
			for _, port := range mr.AppProtoSel.Ports {
				tmr.AppProtoSel.Ports = append(tmr.AppProtoSel.Ports, port)
			}
			for _, app := range mr.AppProtoSel.Apps {
				tmr.AppProtoSel.Apps = append(tmr.AppProtoSel.Apps, app)
			}
		}
		tSpec.MatchRules = append(tSpec.MatchRules, tmr)
	}
	if len(tSpec.MatchRules) == 0 {
		log.Debugf("No Valid MatchRules... Ignore the mirror session %v", ms.Name)
		return nil
	}
	return &tms
}

// ListMirrorSessionsRunning : List only running sessions
func (r *MirrorSessionRPCServer) ListMirrorSessionsRunning(ctx context.Context, sel *api.ObjectMeta) (*tsproto.MirrorSessionList, error) {
	var msList tsproto.MirrorSessionList
	mirrorSessions, err := r.stateMgr.ListMirrorSessions()
	if err != nil {
		return nil, err
	}
	// Walk all the mirror sessions and add RUNNING sessions to the list
	// Only the running sessions need to be sent to NICs
	for _, mss := range mirrorSessions {
		if mss.State != monitoring.MirrorSessionState_RUNNING {
			continue
		}
		tms := buildNICMirrorSession(mss)
		if tms == nil {
			log.Infof("MirrorSession %s not ready to be sent to agent", mss.Name)
			continue
		}
		log.Debugf("Found running mirror session %v: %v", mss.Name, mss.State)
		msList.MirrorSessions = append(msList.MirrorSessions, tms)
	}
	return &msList, nil
}

// WatchMirrorSessions watches mirror session objects for changes and sends them as streaming rpc
// This function is invoked when agent (grpc client) performs stream.Recv() on this watch stream service
func (r *MirrorSessionRPCServer) WatchMirrorSessions(sel *api.ObjectMeta, stream tsproto.MirrorSessionApi_WatchMirrorSessionsServer) error {
	// watch for changes
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)
	r.stateMgr.WatchObjects("MirrorSession", watchChan)
	defer r.stateMgr.StopWatchObjects("MirrorSession", watchChan)

	log.Debugf("Find existing sessions to be sent to an agent")
	// get a list of all existing mirror sessions
	tmsList, err := r.ListMirrorSessionsRunning(context.Background(), sel)
	if err != nil {
		log.Errorf("Error getting a list of MirrorSessions. Err: %v", err)
		return err
	}

	ctx := stream.Context()
	watchEvtList := tsproto.MirrorSessionEventList{}

	// send the objects out as a stream
	for _, tms := range tmsList.MirrorSessions {
		watchEvtList.MirrorSessionEvents = append(watchEvtList.MirrorSessionEvents,
			&tsproto.MirrorSessionEvent{
				EventType:     api.EventType_CreateEvent,
				MirrorSession: *tms,
			})
	}
	if len(watchEvtList.MirrorSessionEvents) > 0 {
		log.Infof("Streaming %v session to an agent", len(watchEvtList.MirrorSessionEvents))
		err = stream.Send(&watchEvtList)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	// loop forever on watch channel
	// Receives changes from memDB when a MirrorSession object is created/updated/deleted
	log.Debugf("----- Start memdb and timer watch on mirror sessions -----")
	for {
		select {
		// read from channel
		case evt, ok := <-watchChan:
			if !ok {
				log.Errorf("tms:Error reading from channel. Closing watch")
				return errors.New("Error reading from channel")
			}
			watchEvtList = tsproto.MirrorSessionEventList{}
			mss := evt.Obj.(*statemgr.MirrorSessionState)

			// get event type from memdb event
			var etype api.EventType
			switch evt.EventType {
			case memdb.CreateEvent:
				if mss.State == monitoring.MirrorSessionState_SCHEDULED {
					log.Infof("Scheduled Mirror session %s not sent to agent", mss.Name)
					continue
				}
				etype = api.EventType_CreateEvent
			case memdb.UpdateEvent:
				log.Infof("Update on mirror session %v", mss.Name)
				mss.Mutex.Lock()
				if mss.State == monitoring.MirrorSessionState_READY_TO_RUN {
					mss.State = monitoring.MirrorSessionState_RUNNING
					etype = api.EventType_CreateEvent
				} else if mss.State == monitoring.MirrorSessionState_SCHEDULED {
					mss.Mutex.Unlock()
					log.Infof("Scheduled Mirror session %s not sent to agent", mss.Name)
					continue
				} else {
					etype = api.EventType_UpdateEvent
				}
				mss.Mutex.Unlock()
			case memdb.DeleteEvent:
				log.Infof("Delete event on mirror session %v", mss.Name)
				etype = api.EventType_DeleteEvent
			default:
				log.Errorf("Unknown memdb event %v", evt.EventType)
				continue
			}
			// convert to tsproto object
			tms := buildNICMirrorSession(mss)
			if tms == nil {
				log.Debugf("Mirror session %s not ready for sending it to agent", mss.Name)
				continue
			}

			// construct the tsproto object
			watchEvt := tsproto.MirrorSessionEvent{
				EventType:     etype,
				MirrorSession: *tms,
			}
			watchEvtList.MirrorSessionEvents = append(watchEvtList.MirrorSessionEvents, &watchEvt)
			log.Infof("Send Mirror session %s to agent", mss.Name)
			err = stream.Send(&watchEvtList)
			if err != nil {
				log.Errorf("Error sending stream. Err: %v", err)
				return err
			}
		case <-ctx.Done():
			return ctx.Err()
		}
	}
	// done
}

// GetMirrorSessionsStatus : Streaming interface to receive mirror session state and captured packets from smartNICs
func (r *MirrorSessionRPCServer) GetMirrorSessionsStatus(stream tsproto.MirrorSessionApi_GetMirrorSessionsStatusServer) error {
	// Receive captured packets from an Agent
	for {
		mspList, err := stream.Recv()
		if err == io.EOF {
			stream.SendAndClose(&tsproto.PacketAckDummy{})
			return nil
		}
		r.stateMgr.UpdateNICMirrorSessionsStatus(mspList)
	}
}

// NewMirrorSessionRPCServer returns a RPC server for mirror sessions
func NewMirrorSessionRPCServer(stateMgr *statemgr.Statemgr) (*MirrorSessionRPCServer, error) {
	return &MirrorSessionRPCServer{stateMgr: stateMgr}, nil
}
