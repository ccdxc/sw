// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package ctrlerif

import (
	"context"
	"fmt"
	"sync"

	"github.com/pensando/sw/agent/netagent"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/rpckit"
)

// NpmClient is the network policy mgr client
type NpmClient struct {
	sync.Mutex                                      // lock the npm client
	agent             netagent.CtrlerIntf           // net Agent API
	grpcClient        *rpckit.RPCClient             // grpc client
	networkRPCClient  netproto.NetworkApiClient     // network RPC client
	endpointRPCClient netproto.EndpointApiClient    // endpoint RPC client
	sgRPCClient       netproto.SecurityApiClient    // security group RPC client
	watchCtx          context.Context               // ctx for network watch
	watchCancel       context.CancelFunc            // cancel for network watch
	pendingEpCreate   map[string]*netproto.Endpoint // pending endpoint create request
	pendingEpDelete   map[string]*netproto.Endpoint // pending endpoint delete requests
}

// objectKey returns object key from object meta
func objectKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// NewNpmClient creates an NPM client object
func NewNpmClient(agent netagent.CtrlerIntf, srvURL string) (*NpmClient, error) {
	// create a grpc client
	rpcClient, err := rpckit.NewRPCClient("netagent", srvURL, "", "", "")
	if err != nil {
		log.Errorf("Error connecting to grpc server. Err: %v", err)
		return nil, err
	}

	// create API clients
	networkRPCClient := netproto.NewNetworkApiClient(rpcClient.ClientConn)
	endpointRPCClient := netproto.NewEndpointApiClient(rpcClient.ClientConn)
	sgRPCClient := netproto.NewSecurityApiClient(rpcClient.ClientConn)

	// watch contexts
	watchCtx, watchCancel := context.WithCancel(context.Background())

	// create NpmClient object
	client := NpmClient{
		agent:             agent,
		grpcClient:        rpcClient,
		networkRPCClient:  networkRPCClient,
		endpointRPCClient: endpointRPCClient,
		sgRPCClient:       sgRPCClient,
		watchCtx:          watchCtx,
		watchCancel:       watchCancel,
		pendingEpCreate:   make(map[string]*netproto.Endpoint),
		pendingEpDelete:   make(map[string]*netproto.Endpoint),
	}

	// register the NPM client as a controller plugin
	err = agent.RegisterCtrlerIf(&client)
	if err != nil {
		log.Fatalf("Error registering the controller interface. Err: %v", err)
		return nil, err
	}

	// start watching objects
	go client.runNetworkWatcher(client.watchCtx)
	go client.runEndpointWatcher(client.watchCtx)
	go client.runSecurityGroupWatcher(client.watchCtx)

	return &client, nil
}

// runNetworkWatcher runs network watcher loop
func (client *NpmClient) runNetworkWatcher(ctx context.Context) {
	stream, err := client.networkRPCClient.WatchNetworks(ctx, &api.ObjectMeta{})
	if err != nil {
		log.Fatalf("Error watching network")
	}

	// loop till the end
	for {
		// receive from stream
		evt, err := stream.Recv()
		if err != nil {
			log.Errorf("Error receving from watch channel. Exiting network watch. Err: %v", err)
			return
		}

		log.Infof("Ctrlerif: Got Network watch event: {%+v}", evt)

		switch evt.EventType {
		case api.EventType_CreateEvent:
			// create the network
			err = client.agent.CreateNetwork(&evt.Network)
			if err != nil {
				log.Errorf("Error creating the network {%+v}. Err: %v", evt, err)
			}
		case api.EventType_UpdateEvent:
			// update the network
			err = client.agent.UpdateNetwork(&evt.Network)
			if err != nil {
				log.Errorf("Error updating the network {%+v}. Err: %v", evt, err)
			}
		case api.EventType_DeleteEvent:
			// delete the network
			err = client.agent.DeleteNetwork(&evt.Network)
			if err != nil {
				log.Errorf("Error deleting the network {%+v}. Err: %v", evt, err)
			}
		}
	}
}

// runEndpointWatcher runs endpoint watcher loop
func (client *NpmClient) runEndpointWatcher(ctx context.Context) {
	// start the watch
	stream, err := client.endpointRPCClient.WatchEndpoints(ctx, &api.ObjectMeta{})
	if err != nil {
		log.Fatalf("Error watching endpoint")
	}

	// loop till the end
	for {
		// receive from stream
		evt, err := stream.Recv()
		if err != nil {
			log.Errorf("Error receving from watch channel. Exiting endpoint watch. Err: %v", err)
			return
		}

		log.Infof("Ctrlerif: Got Endpoint watch event: {%+v}", evt)

		switch evt.EventType {
		case api.EventType_CreateEvent:
			// if we got a watch event for an endpoint thats pending response, ignore it
			client.Lock()
			_, ok := client.pendingEpCreate[objectKey(evt.Endpoint.ObjectMeta)]
			client.Unlock()
			if !ok {
				// create the endpoint
				_, err = client.agent.CreateEndpoint(&evt.Endpoint)
				if err != nil {
					log.Errorf("Error creating the endpoint {%+v}. Err: %v", evt, err)
				}
			}
		case api.EventType_UpdateEvent:
			// create the endpoint
			err = client.agent.UpdateEndpoint(&evt.Endpoint)
			if err != nil {
				log.Errorf("Error updating the endpoint {%+v}. Err: %v", evt, err)
			}
		case api.EventType_DeleteEvent:
			// if we got a watch event for an endpoint thats pending response, ignore it
			client.Lock()
			_, ok := client.pendingEpDelete[evt.Endpoint.Name]
			client.Unlock()
			if !ok {
				// delete the endpoint
				err = client.agent.DeleteEndpoint(&evt.Endpoint)
				if err != nil {
					log.Errorf("Error deleting the endpoint {%+v}. Err: %v", evt, err)
				}
			}
		}

	}
}

// runSecurityGroupWatcher runs sg watcher loop
func (client *NpmClient) runSecurityGroupWatcher(ctx context.Context) {
	stream, err := client.sgRPCClient.WatchSecurityGroups(ctx, &api.ObjectMeta{})
	if err != nil {
		log.Fatalf("Error watching security group")
	}

	// loop till the end
	for {
		// receive from stream
		evt, err := stream.Recv()
		if err != nil {
			log.Errorf("Error receving from watch channel. Exiting security group watch. Err: %v", err)
			return
		}

		log.Infof("Ctrlerif: Got Security group watch event: {%+v}", evt)

		switch evt.EventType {
		case api.EventType_CreateEvent:
			// create the security group
			err = client.agent.CreateSecurityGroup(&evt.SecurityGroup)
			if err != nil {
				log.Errorf("Error creating the sg {%+v}. Err: %v", evt, err)
			}
		case api.EventType_UpdateEvent:
			// update the sg
			err = client.agent.UpdateSecurityGroup(&evt.SecurityGroup)
			if err != nil {
				log.Errorf("Error updating the sg {%+v}. Err: %v", evt, err)
			}
		case api.EventType_DeleteEvent:
			// delete the sg
			err = client.agent.DeleteSecurityGroup(&evt.SecurityGroup)
			if err != nil {
				log.Errorf("Error deleting the sg {%+v}. Err: %v", evt, err)
			}
		}
	}
}

// Stop stops npm client and all watching go routines
func (client *NpmClient) Stop() {
	client.watchCancel()
	client.grpcClient.Close()
}

// EndpointCreateReq creates an endpoint
func (client *NpmClient) EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	// keep track of pending request
	client.Lock()
	client.pendingEpCreate[epinfo.Name] = epinfo
	client.Unlock()

	// make an RPC call to controller
	ep, err := client.endpointRPCClient.CreateEndpoint(context.Background(), epinfo)
	if err != nil {
		log.Errorf("Error resp from netctrler for ep create {%+v}. Err: %v", epinfo, err)
		return nil, err
	}

	// delete pending req
	client.Lock()
	delete(client.pendingEpCreate, epinfo.Name)
	client.Unlock()

	return ep, err
}

// EndpointAgeoutNotif sends an endpoint ageout notification to controller
func (client *NpmClient) EndpointAgeoutNotif(epinfo *netproto.Endpoint) error {
	// FIXME: to be implemented
	return nil
}

// EndpointDeleteReq deletes an endpoint
func (client *NpmClient) EndpointDeleteReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	// keep track of pending request
	client.Lock()
	client.pendingEpDelete[epinfo.Name] = epinfo
	client.Unlock()

	// make an RPC call to controller
	ep, err := client.endpointRPCClient.DeleteEndpoint(context.Background(), epinfo)
	if err != nil {
		log.Errorf("Error resp from netctrler for ep delete {%+v}. Err: %v", epinfo, err)
		return nil, err
	}

	// delete pending req
	client.Lock()
	delete(client.pendingEpDelete, epinfo.Name)
	client.Unlock()

	return ep, err
}
