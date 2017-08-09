/*
 *
 * Copyright 2017, Pensando Systems
 * All rights reserved.
 *
 */

package vchub

import (
	"fmt"
	"github.com/pensando/sw/utils/log"
	context "golang.org/x/net/context"
	"net"
	"reflect"
	"time"

	"github.com/pensando/sw/orch"
	"github.com/pensando/sw/orch/vchub/store"
	"github.com/pensando/sw/utils/kvstore"
	"google.golang.org/grpc"
)

const (
	smartNICs      = "/vchub/smartNICs"
	nwIFs          = "/vchub/nwIFs"
	reconnectDelay = 2 * time.Second
)

// VchServer implements vchub.apiserver
type VchServer struct {
	name      string
	listener  net.Listener
	stats     orch.Stats
	listenURL string
	errCh     chan error
}

var asInstance *VchServer

// ListSmartNICs implements the RPC
func (as *VchServer) ListSmartNICs(c context.Context, f *orch.Filter) (*orch.SmartNICList, error) {
	// TODO: support filter
	return store.SmartNICList(c)
}

// WatchSmartNICs implements the watch RPC
func (as *VchServer) WatchSmartNICs(ws *orch.WatchSpec, stream orch.OrchApi_WatchSmartNICsServer) error {
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

			e := &orch.SmartNICEvent{}
			var apiEv orch.WatchEvent
			switch event.Type {
			case kvstore.WatcherError:
				as.stats.StoreWatchErrCount++
				watcher.Stop()
				break session

			case kvstore.Created:
				apiEv.Event = orch.WatchEvent_Create

			case kvstore.Updated:
				apiEv.Event = orch.WatchEvent_Update

			case kvstore.Deleted:
				apiEv.Event = orch.WatchEvent_Delete

			// tbd - handling watch outside the store retention window
			default:
				as.stats.StoreWatchBadEventCount++
				log.Errorf("Store returned bad event: %v", event.Type)
				watcher.Stop()
				break session
			}

			sn, ok := event.Object.(*orch.SmartNIC)
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
func (as *VchServer) ListNwIFs(c context.Context, f *orch.Filter) (*orch.NwIFList, error) {
	// ignore filter for now - tbd
	return store.NwIFList(c)
}

// WatchNwIFs implements the watch RPC
func (as *VchServer) WatchNwIFs(ws *orch.WatchSpec, stream orch.OrchApi_WatchNwIFsServer) error {
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

			e := &orch.NwIFEvent{}
			e.Reset()
			var apiEv orch.WatchEvent
			switch event.Type {
			case kvstore.WatcherError:
				as.stats.StoreWatchErrCount++
				watcher.Stop()
				break session

			case kvstore.Created:
				apiEv.Event = orch.WatchEvent_Create

			case kvstore.Updated:
				apiEv.Event = orch.WatchEvent_Update

			case kvstore.Deleted:
				apiEv.Event = orch.WatchEvent_Delete

			// tbd - handling watch outside the store retention window
			default:
				log.Errorf("Store returned bad event: %v", event.Type)
				as.stats.StoreWatchBadEventCount++
				watcher.Stop()
				break session
			}

			nif, ok := event.Object.(*orch.NwIF)
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
func (as *VchServer) WatchNwIFMigration(f *orch.Filter, stream orch.OrchApi_WatchNwIFMigrationServer) error {
	return nil // stub for now.
}

// Inspect returns server statistics
func (as *VchServer) Inspect(c context.Context, e *orch.Empty) (*orch.Stats, error) {
	return &as.stats, nil
}

// StartVCHServer starts the vchub api server
func StartVCHServer(listenURL string) (*VchServer, error) {
	lis, err := net.Listen("tcp", listenURL)
	if err != nil {
		log.Infof("failed to listen: %v", err)
		return nil, err
	}
	errCh := make(chan error)
	s := grpc.NewServer()
	asInstance = &VchServer{
		name:      "VCHub-API",
		listener:  lis,
		listenURL: listenURL,
		errCh:     errCh,
	}
	orch.RegisterOrchApiServer(s, asInstance)
	go func() {
		err := s.Serve(lis)
		if asInstance.listener != nil {
			errCh <- err
		}

		close(errCh)
	}()
	log.Infof("VCHub API server started at %s", listenURL)
	return asInstance, nil
}

// ErrOut returns a channel that gives an error indication
func (as *VchServer) ErrOut() <-chan error {
	return as.errCh
}

// StopServer stops the vchub api server
func (as *VchServer) StopServer() {
	if as.listener != nil {
		l := as.listener
		as.listener = nil
		l.Close()
		log.Infof("VCHub API server at %s stopped", as.listenURL)
	}
}
