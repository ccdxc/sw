/*
 *
 * Copyright 2017, Pensando Systems
 * All rights reserved.
 *
 */

package vchub

import (
	"fmt"
	log "github.com/Sirupsen/logrus"
	context "golang.org/x/net/context"
	"net"
	"reflect"
	"time"

	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/orch/vchub/api"
	"github.com/pensando/sw/orch/vchub/store"
	"github.com/pensando/sw/utils/kvstore"
	"google.golang.org/grpc"
)

const (
	smartNICs      = "/vchub/smartNICs"
	nwIFs          = "/vchub/nwIFs"
	reconnectDelay = 2 * time.Second
)

// server is used to implement vchub.apiserver
type vchServer struct {
	name     string
	listener net.Listener
	stats    api.Stats
}

var asInstance *vchServer

// ListSmartNICs implements the RPC
func (as *vchServer) ListSmartNICs(c context.Context, f *api.Filter) (*api.SmartNICList, error) {
	// TODO: support filter
	return store.SmartNICList(c)
}

// WatchSmartNICs implements the watch RPC
func (as *vchServer) WatchSmartNICs(ws *api.WatchSpec, stream api.VCHubApi_WatchSmartNICsServer) error {
	watchVer := ws.GetRefversion()
	// ignore filter for now - tbd
	for {
		watcher, err := store.SmartNICWatchAll(stream.Context(), watchVer)
		if err != nil {
			log.Errorf("Unable to watch store -- %v", err)
			as.stats.StoreWatchFailCount++
			return err
		}

	session:
		for {
			wc := watcher.EventChan()
			event, active := <-wc
			if !active {
				as.stats.WatchCloseCount++
				return fmt.Errorf("Watch session closed")
			}

			e := &api.SmartNICEvent{}
			var apiEv api.WatchEvent
			switch event.Type {
			case kvstore.WatcherError:
				as.stats.StoreWatchErrCount++
				watcher.Stop()
				break session

			case kvstore.Created:
				apiEv.Event = api.WatchEvent_Create

			case kvstore.Updated:
				apiEv.Event = api.WatchEvent_Update

			case kvstore.Deleted:
				apiEv.Event = api.WatchEvent_Delete

			// tbd - handling watch outside the store retention window
			default:
				as.stats.StoreWatchBadEventCount++
				log.Errorf("Store returned bad event: %v", event.Type)
				watcher.Stop()
				break session
			}

			sn, ok := event.Object.(*api.SmartNIC)
			if !ok {
				log.Errorf("Unexpected object type %q, expected SmartNIC", reflect.TypeOf(event.Object).Name())
				as.stats.StoreWatchBadObjCount++
				continue
			}

			e.E = &apiEv
			e.Smartnics = append(e.Smartnics, sn)
			if err := stream.Send(e); err != nil {
				log.Infof("Send failed - %v", err)
				as.stats.GrpcSendErrCount++
				watcher.Stop()
				return err
			}

			meta := sn.GetObjectMeta()
			if meta != nil {
				watchVer = meta.GetResourceVersion()
			}
		}

		time.Sleep(reconnectDelay) // delay before reconnecting
	}
}

// ListNwIFs implements the RPC
func (as *vchServer) ListNwIFs(c context.Context, f *api.Filter) (*api.NwIFList, error) {
	// ignore filter for now - tbd
	return store.NwIFList(c)
}

// WatchNwIFs implements the watch RPC
func (as *vchServer) WatchNwIFs(ws *api.WatchSpec, stream api.VCHubApi_WatchNwIFsServer) error {
	// ignore filter for now - TODO
	watchVer := ws.GetRefversion()
	for {
		watcher, err := store.NwIFWatchAll(stream.Context(), watchVer)
		if err != nil {
			log.Errorf("Unable to watch store -- %v", err)
			as.stats.StoreWatchFailCount++
			return err
		}

	session:
		for {
			wc := watcher.EventChan()
			event, active := <-wc
			if !active {
				as.stats.WatchCloseCount++
				return fmt.Errorf("Watch session closed")
			}

			e := &api.NwIFEvent{}
			e.Reset()
			var apiEv api.WatchEvent
			switch event.Type {
			case kvstore.WatcherError:
				as.stats.StoreWatchErrCount++
				watcher.Stop()
				break session

			case kvstore.Created:
				apiEv.Event = api.WatchEvent_Create

			case kvstore.Updated:
				apiEv.Event = api.WatchEvent_Update

			case kvstore.Deleted:
				apiEv.Event = api.WatchEvent_Delete

			// tbd - handling watch outside the store retention window
			default:
				log.Errorf("Store returned bad event: %v", event.Type)
				as.stats.StoreWatchBadEventCount++
				watcher.Stop()
				break session
			}

			nif, ok := event.Object.(*api.NwIF)
			if !ok {
				log.Errorf("Unexpected object type %q, expected SmartNIC", reflect.TypeOf(event.Object).Name())
				as.stats.StoreWatchBadObjCount++
				continue
			}

			e.E = &apiEv
			e.Nwifs = append(e.Nwifs, nif)
			if err := stream.Send(e); err != nil {
				log.Infof("Send failed - %v", err)
				as.stats.GrpcSendErrCount++
				watcher.Stop()
				return err
			}

			meta := nif.GetObjectMeta()
			if meta != nil {
				watchVer = meta.GetResourceVersion()
			}

		}

		time.Sleep(reconnectDelay) // delay before reconnecting
	}
}

//WatchNwIFMigration implements the RPC for migration notifications
func (as *vchServer) WatchNwIFMigration(f *api.Filter, stream api.VCHubApi_WatchNwIFMigrationServer) error {
	return nil // stub for now.
}

// Inspect returns server statistics
func (as *vchServer) Inspect(c context.Context, e *api.Empty) (*api.Stats, error) {
	return &as.stats, nil
}

// startVCHServer starts the vchub api server
func startVCHServer() {
	lis, err := net.Listen("tcp", ":"+globals.VCHubAPIPort)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	s := grpc.NewServer()
	asInstance = &vchServer{
		name:     "VCHub-API",
		listener: lis,
	}
	api.RegisterVCHubApiServer(s, asInstance)
	go func() {
		err := s.Serve(lis)
		if asInstance.listener != nil {
			log.Fatalf("Server exited %v", err)
		}
	}()
	log.Infof("VCHub API server started")
}

// stopVCHServer stops the vchub api server
func stopVCHServer() {
	if asInstance != nil && asInstance.listener != nil {
		asInstance.listener.Close()
		asInstance.listener = nil
		log.Infof("VCHub API server stopped")
	}
}
