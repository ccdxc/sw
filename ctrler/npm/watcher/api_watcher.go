// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package watcher

import (
	"context"
	"fmt"
	"time"

	"github.com/Sirupsen/logrus"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/orch"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/rpckit"

	"google.golang.org/grpc"
)

// handleApisrvWatch handles api server watch events
func (w *Watcher) handleApisrvWatch(apicl apiclient.Services) {
	// network watcher
	opts := api.ListWatchOptions{}
	ctx := context.Background()
	netWatcher, err := apicl.NetworkV1().Network().Watch(ctx, &opts)
	if err != nil {
		logrus.Errorf("Failed to start watch (%s)\n", err)
		return
	}

	// sg watcher
	sgWatcher, err := apicl.SecurityGroupV1().SecurityGroup().Watch(ctx, &opts)
	if err != nil {
		logrus.Errorf("Failed to start watch (%s)\n", err)
		return
	}

	// sg policy watcher
	sgpWatcher, err := apicl.SgpolicyV1().Sgpolicy().Watch(ctx, &opts)
	if err != nil {
		logrus.Errorf("Failed to start watch (%s)\n", err)
		return
	}

	// ep object watcher
	epWatcher, err := apicl.EndpointV1().Endpoint().Watch(ctx, &opts)
	if err != nil {
		logrus.Errorf("Failed to start watch (%s)\n", err)
		return
	}

	// wait for events
	for {
		select {
		case evt, ok := <-netWatcher.EventChan():
			if !ok {
				logrus.Errorf("Error receiving from apisrv watcher")
				return
			}

			w.netWatcher <- *evt
		case evt, ok := <-sgWatcher.EventChan():
			if !ok {
				logrus.Errorf("Error receiving from apisrv watcher")
				return
			}

			w.netWatcher <- *evt

		case evt, ok := <-sgpWatcher.EventChan():
			if !ok {
				logrus.Errorf("Error receiving from apisrv watcher")
				return
			}
			w.netWatcher <- *evt

		case evt, ok := <-epWatcher.EventChan():
			if !ok {
				logrus.Errorf("Error receiving from apisrv watcher")
				return
			}
			w.vmmEpWatcher <- *evt
		}
	}
}

// runApisrvWatcher run API server watcher forever
func (w *Watcher) runApisrvWatcher(apisrvURL string) {
	// if we have no URL, exit
	if apisrvURL == "" {
		return
	}

	// create logger
	config := log.GetDefaultConfig("NpmApiWatcher")
	l := log.GetNewLogger(config)

	// loop forever
	for {
		apicl, err := apiclient.NewGrpcAPIClient(apisrvURL, l, grpc.WithInsecure(), grpc.WithTimeout(time.Second))
		if err != nil {
			logrus.Warnf("Failed to connect to gRPC server [%s]\n", apisrvURL)

			// wait for a second and retry connecting to api server
			time.Sleep(time.Second)
			continue
		}

		logrus.Infof("API client connected {%+v}", apicl)

		// handle api server watch events
		w.handleApisrvWatch(apicl)
		time.Sleep(time.Second)
	}
}

func (w *Watcher) handleVmmEvents(stream orch.OrchApi_WatchNwIFsClient) {
	// loop till connection closes
	for {
		// keep receving events
		evt, err := stream.Recv()
		if err != nil {
			logrus.Errorf("Error receving from nw if watcher. Err: %v", err)
			return
		}

		// convert evet type
		var evtType kvstore.WatchEventType
		switch evt.E.Event {
		case orch.WatchEvent_Create:
			evtType = kvstore.Created
		case orch.WatchEvent_Update:
			evtType = kvstore.Updated
		case orch.WatchEvent_Delete:
			evtType = kvstore.Deleted
		}

		// loop thru each nw if
		for _, nif := range evt.Nwifs {
			// convert attribute map to slice
			var attrs []string
			for k, v := range nif.Attributes {
				attrs = append(attrs, fmt.Sprintf("%s:%s", k, v))
			}
			// convert attributes
			// convert Nw IF to an endpoint
			ep := network.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Name:   nif.ObjectMeta.Name,
					Tenant: nif.ObjectMeta.Tenant,
				},
				Spec: network.EndpointSpec{},
				Status: network.EndpointStatus{
					Network:            nif.Status.PortGroup,
					EndpointUUID:       nif.ObjectMeta.Name,
					WorkloadName:       nif.ObjectMeta.Name,
					WorkloadUUID:       nif.ObjectMeta.Name,
					WorkloadAttributes: attrs,
					MacAddress:         nif.Status.MacAddress,
					HomingHostAddr:     nif.Status.SmartNIC_ID,
					HomingHostName:     nif.Status.SmartNIC_ID,
					MicroSegmentVlan:   uint32(nif.Config.LocalVLAN),
				},
			}

			// create watch event
			watchEvent := kvstore.WatchEvent{
				Type:   evtType,
				Object: &ep,
			}

			// inject into watch channel
			w.vmmEpWatcher <- watchEvent
		}

	}
}

// runVmmWatcher runs grpc client to watch VMM events
func (w *Watcher) runVmmWatcher(vmmURL string) {
	// if we have no URL, exit
	if vmmURL == "" {
		return
	}

	// loop forever
	for {
		// create a grpc client
		rpcClient, err := rpckit.NewRPCClient("NpmVmmWatcher", vmmURL, "", "", "")
		if err != nil {
			logrus.Warnf("Error connecting to grpc server. Err: %v", err)

			// wait for a bit and retry connecting
			time.Sleep(time.Second)
			continue
		}

		// create vmm client
		vmmClient := orch.NewOrchApiClient(rpcClient.ClientConn)

		// keep receiving events
		stream, err := vmmClient.WatchNwIFs(context.Background(), &orch.WatchSpec{})
		if err != nil {
			logrus.Errorf("Error watching vmm nw if")
			// wait for a bit and retry connecting
			time.Sleep(time.Second)
			continue
		}

		// handle vmm events
		w.handleVmmEvents(stream)
		time.Sleep(time.Second)
	}
}
