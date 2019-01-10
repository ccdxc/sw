// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"errors"
	"fmt"
	"strconv"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/ctrler/npm/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// AppRPCServer serves security rpc requests
type AppRPCServer struct {
	stateMgr *statemgr.Statemgr // reference to network manager
}

// convertApp converts from npm state to netproto app
func convertApp(aps *statemgr.AppState) *netproto.App {
	var protoPorts []string

	// convert protocol/port
	for _, pp := range aps.Spec.ProtoPorts {
		protoPorts = append(protoPorts, fmt.Sprintf("%s/%s", pp.Protocol, pp.Ports))
	}

	// build sg message
	app := netproto.App{
		TypeMeta:   aps.TypeMeta,
		ObjectMeta: aps.ObjectMeta,
		Spec: netproto.AppSpec{
			ProtoPorts:     protoPorts,
			AppIdleTimeout: aps.Spec.Timeout,
			ALG:            &netproto.ALG{},
		},
	}
	if aps.Spec.ALG != nil {
		app.Spec.ALGType = aps.Spec.ALG.Type

		switch aps.Spec.ALG.Type {
		case "ICMP":
			icmp := security.Icmp{}
			if aps.Spec.ALG.Icmp != nil {
				icmp = *aps.Spec.ALG.Icmp
			}
			ictype, _ := strconv.Atoi(icmp.Type)
			icode, _ := strconv.Atoi(icmp.Code)

			app.Spec.ALG.ICMP = &netproto.ICMP{
				Type: uint32(ictype),
				Code: uint32(icode),
			}
		case "DNS":
			dns := security.Dns{}
			if aps.Spec.ALG.Dns != nil {
				dns = *aps.Spec.ALG.Dns
			}
			app.Spec.ALG.DNS = &netproto.DNS{
				DropMultiQuestionPackets: dns.DropMultiQuestionPackets,
				DropLargeDomainPackets:   dns.DropLargeDomainNamePackets,
				DropLongLabelPackets:     dns.DropLongLabelPackets,
				MaxMessageLength:         dns.MaxMessageLength,
				QueryResponseTimeout:     dns.QueryResponseTimeout,
			}
		case "FTP":
			ftp := security.Ftp{}
			if aps.Spec.ALG.Ftp != nil {
				ftp = *aps.Spec.ALG.Ftp
			}
			app.Spec.ALG.FTP = &netproto.FTP{
				AllowMismatchIPAddresses: ftp.AllowMismatchIPAddress,
			}
		case "SunRPC":
			for _, sunrpc := range aps.Spec.ALG.Sunrpc {
				pgmID, _ := strconv.Atoi(sunrpc.ProgramID)
				app.Spec.ALG.SUNRPC = append(app.Spec.ALG.SUNRPC,
					&netproto.RPC{
						ProgramID:        uint32(pgmID),
						ProgramIDTimeout: sunrpc.Timeout,
					})
			}
		case "MSRPC":
			for _, msrpc := range aps.Spec.ALG.Msrpc {
				pgmID, _ := strconv.Atoi(msrpc.ProgramUUID)
				app.Spec.ALG.MSRPC = append(app.Spec.ALG.MSRPC,
					&netproto.RPC{
						ProgramID:        uint32(pgmID),
						ProgramIDTimeout: msrpc.Timeout,
					})
			}
		case "TFTP":
		case "RTSP":
		}
	}

	return &app
}

// GetApp gets sg policy
func (s *AppRPCServer) GetApp(ctx context.Context, ometa *api.ObjectMeta) (*netproto.App, error) {
	// find the sgp
	aps, err := s.stateMgr.FindApp(ometa.Tenant, ometa.Name)
	if err != nil {
		log.Errorf("Could not find the sg %s|%s", ometa.Tenant, ometa.Name)
		return nil, fmt.Errorf("could not find the sg policy %v", ometa.Name)
	}

	return convertApp(aps), nil
}

// ListApps returns a list of sg policies
func (s *AppRPCServer) ListApps(context.Context, *api.ObjectMeta) (*netproto.AppList, error) {
	var appList netproto.AppList

	// get all apps
	apps, err := s.stateMgr.ListApps()
	if err != nil {
		return nil, err
	}

	// walk all Apps in the list
	for _, aps := range apps {
		appList.Apps = append(appList.Apps, convertApp(aps))
	}

	return &appList, nil
}

// WatchApps watches for App changes
func (s *AppRPCServer) WatchApps(sel *api.ObjectMeta, stream netproto.AppApi_WatchAppsServer) error {
	// watch for changes
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)
	s.stateMgr.WatchObjects("App", watchChan)
	defer s.stateMgr.StopWatchObjects("App", watchChan)

	// get a list of all Apps
	apps, err := s.ListApps(context.Background(), sel)
	if err != nil {
		log.Errorf("Error getting a list of sgs. Err: %v", err)
		return err
	}

	ctx := stream.Context()

	// send the objects out as a stream
	for _, app := range apps.Apps {
		watchEvt := netproto.AppEvent{
			EventType: api.EventType_CreateEvent,
			App:       *app,
		}
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	// loop forever on watch channel
	for {
		select {
		// read from channel
		case evt, ok := <-watchChan:
			if !ok {
				log.Errorf("Error reading from channel. Closing watch")
				return errors.New("error reading from channel")
			}

			// get event type from memdb event
			var etype api.EventType
			switch evt.EventType {
			case memdb.CreateEvent:
				etype = api.EventType_CreateEvent
			case memdb.UpdateEvent:
				etype = api.EventType_UpdateEvent
			case memdb.DeleteEvent:
				etype = api.EventType_DeleteEvent
			}

			// convert to sg policy object
			aps, err := statemgr.AppStateFromObj(evt.Obj)
			if err != nil {
				continue
			}

			// construct the netproto object
			watchEvt := netproto.AppEvent{
				EventType: etype,
				App:       *convertApp(aps),
			}

			err = stream.Send(&watchEvt)
			if err != nil {
				log.Errorf("Error sending stream. Err: %v", err)
				return err
			}
		case <-ctx.Done():
			return ctx.Err()
		}
	}

}

// NewAppRPCServer returns a security RPC server
func NewAppRPCServer(stateMgr *statemgr.Statemgr) (*AppRPCServer, error) {
	return &AppRPCServer{stateMgr: stateMgr}, nil
}
