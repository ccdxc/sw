// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"errors"
	"fmt"
	"strings"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

const maxWatchEventsPerMesage = 16

// MirrorSessionRPCServer is the mirror session RPC server
type MirrorSessionRPCServer struct {
	stateMgr *statemgr.Statemgr
}

func buildNICMirrorSession(mss *statemgr.MirrorSessionState) *netproto.MirrorSession {
	ms := mss.MirrorSession
	tms := netproto.MirrorSession{
		TypeMeta:   ms.TypeMeta,
		ObjectMeta: ms.ObjectMeta,
	}
	tSpec := &tms.Spec
	//tSpec.CaptureAt = netproto.MirrorSrcDst_SRC_DST
	tSpec.MirrorDirection = netproto.MirrorDir_BOTH
	//tSpec.Enable = (mss.State == monitoring.MirrorSessionState_ACTIVE)
	tSpec.PacketSize = ms.Spec.PacketSize
	//tSpec.PacketFilters = ms.Spec.PacketFilters

	for _, c := range ms.Spec.Collectors {
		var export monitoring.MirrorExportConfig
		if c.ExportCfg != nil {
			export = *c.ExportCfg
		}
		tc := netproto.MirrorCollector{
			//Type:      c.Type,
			ExportCfg: netproto.MirrorExportConfig{Destination: export.Destination},
		}
		tSpec.Collectors = append(tSpec.Collectors, tc)
	}
	for _, mr := range ms.Spec.MatchRules {
		tmr := netproto.MatchRule{}
		if mr.Src == nil && mr.Dst == nil {
			log.Debugf("Ignore MatchRule with Src = * and Dst = *")
			continue
		}
		if mr.Src != nil {
			tmr.Src = &netproto.MatchSelector{}
			tmr.Src.Addresses = mr.Src.IPAddresses
			//tmr.Src.MACAddresses = mr.Src.MACAddresses
		}
		if mr.Dst != nil {
			tmr.Dst = &netproto.MatchSelector{}
			tmr.Dst.Addresses = mr.Dst.IPAddresses
			if mr.AppProtoSel != nil {
				for _, pp := range mr.AppProtoSel.ProtoPorts {
					var protoPort netproto.ProtoPort
					components := strings.Split(pp, "/")
					switch len(components) {
					case 1:
						protoPort.Protocol = components[0]
					case 2:
						protoPort.Protocol = components[0]
						protoPort.Port = components[1]
					case 3:
						protoPort.Protocol = components[0]
						protoPort.Port = fmt.Sprintf("%s/%s", components[1], components[2])
					default:
						continue
					}
					tmr.Dst.ProtoPorts = append(tmr.Dst.ProtoPorts, &protoPort)
				}
			}
			//tmr.Dst.MACAddresses = mr.Dst.MACAddresses
		}
		//if mr.AppProtoSel != nil {
		//	tmr.AppProtoSel = &netproto.AppProtoSelector{}
		//	for _, port := range mr.AppProtoSel.ProtoPorts {
		//		tmr.AppProtoSel.Ports = append(tmr.AppProtoSel.Ports, port)
		//	}
		//	for _, app := range mr.AppProtoSel.Apps {
		//		tmr.AppProtoSel.Apps = append(tmr.AppProtoSel.Apps, app)
		//	}
		//}
		tSpec.MatchRules = append(tSpec.MatchRules, tmr)
	}
	return &tms
}

// ListMirrorSessionsActive : List only sessions that are running/scheduled i.e. not in ERR_xxx state
func (r *MirrorSessionRPCServer) ListMirrorSessionsActive(ctx context.Context, sel *api.ObjectMeta) (*netproto.MirrorSessionList, error) {
	var msList netproto.MirrorSessionList
	mirrorSessions, err := r.stateMgr.ListMirrorSessions()
	if err != nil {
		return nil, err
	}
	// Walk all the mirror sessions and add non-errored sessions to the list
	// Only the running sessions need to be sent to NICs
	for _, mss := range mirrorSessions {
		mss.Mutex.Lock()
		if mss.State != monitoring.MirrorSessionState_ACTIVE {
			mss.Mutex.Unlock()
			continue
		}
		tms := buildNICMirrorSession(mss)
		mss.Mutex.Unlock()
		log.Debugf("Found running mirror session %v: %v", mss.Name, mss.State)
		msList.MirrorSessions = append(msList.MirrorSessions, tms)
	}
	return &msList, nil
}

// ListMirrorSessions api to list
func (r *MirrorSessionRPCServer) ListMirrorSessions(ctx context.Context, sel *api.ObjectMeta) (*netproto.MirrorSessionEventList, error) {
	tmsList, err := r.ListMirrorSessionsActive(context.Background(), sel)
	if err != nil {
		log.Errorf("Error getting a list of MirrorSessions. Err: %v", err)
		return nil, err
	}
	evList := &netproto.MirrorSessionEventList{}
	for _, tms := range tmsList.MirrorSessions {
		evList.MirrorSessionEvents = append(evList.MirrorSessionEvents,
			&netproto.MirrorSessionEvent{
				EventType:     api.EventType_CreateEvent,
				MirrorSession: tms,
			})
	}
	return evList, nil
}

// WatchMirrorSessions watches mirror session objects for changes and sends them as streaming rpc
// This function is invoked when agent (grpc client) performs stream.Recv() on this watch stream service
func (r *MirrorSessionRPCServer) WatchMirrorSessions(sel *api.ObjectMeta, stream netproto.MirrorSessionApiV1_WatchMirrorSessionsServer) error {
	// watch for changes
	watcher := memdb.Watcher{Name: "mirror-session"}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	defer close(watcher.Channel)
	r.stateMgr.WatchObjects("MirrorSession", &watcher)
	defer r.stateMgr.StopWatchObjects("MirrorSession", &watcher)

	log.Debugf("Find existing sessions to be sent to an agent")
	// get a list of all existing mirror sessions
	tmsList, err := r.ListMirrorSessionsActive(context.Background(), sel)
	if err != nil {
		log.Errorf("Error getting a list of MirrorSessions. Err: %v", err)
		return err
	}

	ctx := stream.Context()
	watchEvtList := netproto.MirrorSessionEventList{}

	// send the objects out as a stream
	for _, tms := range tmsList.MirrorSessions {
		watchEvtList.MirrorSessionEvents = append(watchEvtList.MirrorSessionEvents,
			&netproto.MirrorSessionEvent{
				EventType:     api.EventType_CreateEvent,
				MirrorSession: tms,
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
		case evt, ok := <-watcher.Channel:
			if !ok {
				log.Errorf("tms:Error reading from channel. Closing watch")
				return errors.New("Error reading from channel")
			}
			watchEvtList = netproto.MirrorSessionEventList{}
			mss := evt.Obj.(*statemgr.MirrorSessionState)
			mss.Mutex.Lock()
			// Errorred session are not sent to Naples, so avoid all create/update/del transactions
			if mss.State == monitoring.MirrorSessionState_ERR_NO_MIRROR_SESSION {
				mss.Mutex.Unlock()
				continue
			}

			// get event type from memdb event
			var etype api.EventType
			switch evt.EventType {
			case memdb.CreateEvent:
				etype = api.EventType_CreateEvent
			case memdb.UpdateEvent:
				// mirror sessions in STOPPED state (timer expiry) will be sent to agents
				// as an update with enable = false
				etype = api.EventType_UpdateEvent
			case memdb.DeleteEvent:
				etype = api.EventType_DeleteEvent
			default:
				log.Errorf("Unknown memdb event %v", evt.EventType)
				mss.Mutex.Unlock()
				continue
			}

			// only active session will be sent to naples, scheduled session not included
			if (etype == api.EventType_CreateEvent || etype == api.EventType_UpdateEvent) && mss.State != monitoring.MirrorSessionState_ACTIVE {
				mss.Mutex.Unlock()
				continue
			}

			// convert to netproto object
			tms := buildNICMirrorSession(mss)
			mss.Mutex.Unlock()

			// construct the netproto object
			watchEvt := netproto.MirrorSessionEvent{
				EventType:     etype,
				MirrorSession: tms,
			}
			log.Infof("Send Mirror session %s to agent - event %v state %v", mss.Name, etype, mss.Status.ScheduleState)
			watchEvtList.MirrorSessionEvents = append(watchEvtList.MirrorSessionEvents, &watchEvt)
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

// NewMirrorSessionRPCServer returns a RPC server for mirror sessions
func NewMirrorSessionRPCServer(stateMgr *statemgr.Statemgr) (*MirrorSessionRPCServer, error) {
	return &MirrorSessionRPCServer{stateMgr: stateMgr}, nil
}
