// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package ctrlerif

import (
	"context"
	"fmt"
	"math"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/debug"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/tsdb"
)

const netEvChanLength = 2000
const maxOpretry = 10

// NpmClient is the network policy mgr client
type NpmClient struct {
	sync.Mutex                                    // lock the npm client
	srvURL          string                        // NPM rpc server URL
	resolverClient  resolver.Interface            // Resolver client
	waitGrp         sync.WaitGroup                // wait group to wait on all go routines to exit
	agent           types.CtrlerIntf              // net Agent API
	netGrpcClient   *rpckit.RPCClient             // grpc client for network
	sgGrpcClient    *rpckit.RPCClient             // grpc client for security group
	epGrpcClient    *rpckit.RPCClient             // grpc client for endpoint
	sgpGrpcClient   *rpckit.RPCClient             // grpc client for security policy
	watchCtx        context.Context               // ctx for network watch
	watchCancel     context.CancelFunc            // cancel for network watch
	stopped         bool                          // is the npm client stopped?
	pendingEpCreate map[string]*netproto.Endpoint // pending endpoint create request
	pendingEpDelete map[string]*netproto.Endpoint // pending endpoint delete requests
	debugStats      *debug.Stats
	startTime       time.Time
}

// objectKey returns object key from object meta
func objectKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// NewNpmClient creates an NPM client object
func NewNpmClient(agent types.CtrlerIntf, srvURL string, resolverClient resolver.Interface) (*NpmClient, error) {
	// watch contexts
	watchCtx, watchCancel := context.WithCancel(context.Background())

	// create NpmClient object
	client := NpmClient{
		srvURL:          srvURL,
		resolverClient:  resolverClient,
		agent:           agent,
		watchCtx:        watchCtx,
		watchCancel:     watchCancel,
		pendingEpCreate: make(map[string]*netproto.Endpoint),
		pendingEpDelete: make(map[string]*netproto.Endpoint),
		startTime:       time.Now(),
	}

	client.debugStats = debug.New(fmt.Sprintf("npmif-%s", client.getAgentName())).Tsdb().Kind("netagentStats").TsdbPeriod(5 * time.Second).Build()

	// register the NPM client as a controller plugin
	err := agent.RegisterCtrlerIf(&client)
	if err != nil {
		log.Fatalf("Error registering the controller interface. Err: %v", err)
		return nil, err
	}

	// start watching objects
	go client.runNetworkWatcher(client.watchCtx)
	go client.runEndpointWatcher(client.watchCtx)
	go client.runSecurityGroupWatcher(client.watchCtx)
	go client.runSecurityPolicyWatcher(client.watchCtx)

	return &client, nil
}

// getAgentName returns a unique name for each agent instance
func (client *NpmClient) getAgentName() string {
	return "netagent-" + client.agent.GetAgentID()
}

// processNetworkEvent handles network event
func (client *NpmClient) processNetworkEvent(evt netproto.NetworkEvent) {
	var err error
	for iter := 0; iter < maxOpretry; iter++ {
		switch evt.EventType {
		case api.EventType_CreateEvent:
			// create the network
			client.debugStats.AddFloat("net_create", 1.0)
			err = client.agent.CreateNetwork(&evt.Network)
			if err != nil {
				log.Errorf("Error creating the network {%+v}. Err: %v", evt, err)
			}
		case api.EventType_UpdateEvent:
			// update the network
			client.debugStats.AddFloat("net_update", 1.0)
			err = client.agent.UpdateNetwork(&evt.Network)
			if err != nil {
				log.Errorf("Error updating the network {%+v}. Err: %v", evt, err)
			}
		case api.EventType_DeleteEvent:
			// delete the network
			client.debugStats.AddFloat("net_delete", 1.0)
			err = client.agent.DeleteNetwork(evt.Network.Tenant, evt.Network.Namespace, evt.Network.Name)
			if err != nil {
				log.Errorf("Error deleting the network {%+v}. Err: %v", evt, err)
			}
		}

		// return if there is no error
		if err == nil {
			return
		}

		// else, retry after some time, with exponential backoff
		time.Sleep(time.Second * time.Duration(math.Exp2(float64(iter))))
	}
}

// processEndpointEvent handles endpoint event.
// it retries the operation till it suceeds
func (client *NpmClient) processEndpointEvent(epevt netproto.EndpointEvent) {
	var err error
	for iter := 0; iter < maxOpretry; iter++ {
		switch epevt.EventType {
		case api.EventType_CreateEvent:
			// if we got a watch event for an endpoint thats pending response, ignore it
			client.Lock()
			_, ok := client.pendingEpCreate[objectKey(epevt.Endpoint.ObjectMeta)]
			client.Unlock()
			if !ok {
				// create the endpoint
				_, err = client.agent.CreateEndpoint(&epevt.Endpoint)
				if err != nil {
					log.Errorf("Error creating the endpoint {%+v}. Err: %v", epevt, err)
				}
			}
		case api.EventType_UpdateEvent:
			// create the endpoint
			err = client.agent.UpdateEndpoint(&epevt.Endpoint)
			if err != nil {
				log.Errorf("Error updating the endpoint {%+v}. Err: %v", epevt, err)
			}
		case api.EventType_DeleteEvent:
			// if we got a watch event for an endpoint thats pending response, ignore it
			client.Lock()
			_, ok := client.pendingEpDelete[epevt.Endpoint.Name]
			client.Unlock()
			if !ok {
				// delete the endpoint
				err = client.agent.DeleteEndpoint(epevt.Endpoint.Tenant, epevt.Endpoint.Namespace, epevt.Endpoint.Name)
				if err != nil {
					log.Errorf("Error deleting the endpoint {%+v}. Err: %v", epevt, err)
				}
			}
		}

		// return if there is no error
		if err == nil {
			return
		}

		// else, retry after some time, with exponential backoff
		time.Sleep(time.Second * time.Duration(math.Exp2(float64(iter))))
	}
}

// processSecurityGroupEvent handles security group event
func (client *NpmClient) processSecurityGroupEvent(evt netproto.SecurityGroupEvent) {
	var err error
	for iter := 0; iter < maxOpretry; iter++ {
		switch evt.EventType {
		case api.EventType_CreateEvent:
			// create the security group
			err = client.agent.CreateSecurityGroup(&evt.SecurityGroup)
			if err != nil {
				log.Errorf("Error creating the sg {%+v}. Err: %v", evt.SecurityGroup.ObjectMeta, err)
			}
		case api.EventType_UpdateEvent:
			// update the sg
			err = client.agent.UpdateSecurityGroup(&evt.SecurityGroup)
			if err != nil {
				log.Errorf("Error updating the sg {%+v}. Err: %v", evt.SecurityGroup.ObjectMeta, err)
			}
		case api.EventType_DeleteEvent:
			// delete the sg
			err = client.agent.DeleteSecurityGroup(evt.SecurityGroup.Tenant, evt.SecurityGroup.Namespace, evt.SecurityGroup.Name)
			if err != nil {
				log.Errorf("Error deleting the sg {%+v}. Err: %v", evt.SecurityGroup.ObjectMeta, err)
			}
		}

		// return if there is no error
		if err == nil {
			return
		}

		// else, retry after some time, with exponential backoff
		time.Sleep(time.Second * time.Duration(math.Exp2(float64(iter))))
	}
}

// processSecurityPolicyEvent handles security policy event
func (client *NpmClient) processSecurityPolicyEvent(evt netproto.SGPolicyEvent) {
	var err error
	for iter := 0; iter < maxOpretry; iter++ {
		switch evt.EventType {
		case api.EventType_CreateEvent:
			// create the security policy
			err = client.agent.CreateSGPolicy(&evt.SGPolicy)
			if err != nil {
				log.Errorf("Error creating the sg policy {%+v}. Err: %v", evt.SGPolicy.ObjectMeta, err)
			} else {

			}
		case api.EventType_UpdateEvent:
			// update the sg policy
			err = client.agent.UpdateSGPolicy(&evt.SGPolicy)
			if err != nil {
				log.Errorf("Error updating the sg policy {%+v}. Err: %v", evt.SGPolicy.ObjectMeta, err)
			}
		case api.EventType_DeleteEvent:
			// delete the sg
			err = client.agent.DeleteSGPolicy(evt.SGPolicy.Tenant, evt.SGPolicy.Namespace, evt.SGPolicy.Name)
			if err != nil {
				log.Errorf("Error deleting the sg policy {%+v}. Err: %v", evt.SGPolicy.ObjectMeta, err)
			}
		}

		// return if there is no error
		if err == nil {
			if evt.EventType == api.EventType_CreateEvent || evt.EventType == api.EventType_UpdateEvent {
				sgpRPCClient := netproto.NewSGPolicyApiClient(client.sgpGrpcClient.ClientConn)
				log.Errorf("Stavros 0: Sending update back {%+v}", &evt.SGPolicy)
				sgpRPCClient.UpdateSGPolicyStatus(context.Background(), &evt.SGPolicy)
			}
			return
		}

		// else, retry after some time, with exponential backoff
		time.Sleep(time.Second * time.Duration(math.Exp2(float64(iter))))
	}
}

// runNetworkWatcher runs network watcher loop
func (client *NpmClient) runNetworkWatcher(ctx context.Context) {
	// setup wait group
	client.waitGrp.Add(1)
	defer client.waitGrp.Done()

	for {
		// create a grpc client
		rpcClient, err := rpckit.NewRPCClient(client.getAgentName(), client.srvURL,
			rpckit.WithBalancer(balancer.New(client.resolverClient)), rpckit.WithRemoteServerName(globals.Npm))
		if err != nil {
			log.Errorf("Error connecting to grpc server. Err: %v", err)

			if client.isStopped() {
				return
			}
			time.Sleep(time.Second)
			continue
		}
		client.netGrpcClient = rpcClient

		// start the watch
		networkRPCClient := netproto.NewNetworkApiClient(rpcClient.ClientConn)
		stream, err := networkRPCClient.WatchNetworks(ctx, &api.ObjectMeta{})
		if err != nil {
			rpcClient.Close()
			log.Errorf("Error watching network: Err: %v", err)

			if client.isStopped() {
				return
			}
			time.Sleep(time.Second)
			continue
		}

		// loop till the end
		for {
			// receive from stream
			evtList, err := stream.Recv()
			if err != nil {
				log.Errorf("Error receiving from watch channel. Exiting network watch. Err: %v", err)

				if client.isStopped() {
					rpcClient.Close()
					return
				}
				time.Sleep(time.Second)
				break
			}

			for _, evt := range evtList.NetworkEvents {
				t, err := evt.Network.ModTime.Time()
				if err == nil && client.startTime.Before(t) && evt.EventType != api.EventType_DeleteEvent {
					latency := time.Since(t)
					if latency >= 0 {
						tsdb.LogField("latency", evt.Network.ObjectMeta, "net_latency", float64(latency))
					}
				}
			}

			// process the event
			for _, evt := range evtList.NetworkEvents {
				go client.processNetworkEvent(*evt)
			}
		}

		rpcClient.Close()
	}
}

// runEndpointWatcher runs endpoint watcher loop
func (client *NpmClient) runEndpointWatcher(ctx context.Context) {
	// setup wait group
	client.waitGrp.Add(1)
	defer client.waitGrp.Done()

	for {
		// create a grpc client
		rpcClient, err := rpckit.NewRPCClient(client.getAgentName(), client.srvURL,
			rpckit.WithBalancer(balancer.New(client.resolverClient)), rpckit.WithRemoteServerName(globals.Npm))
		if err != nil {
			log.Errorf("Error connecting to grpc server. Err: %v", err)

			if client.isStopped() {
				return
			}
			time.Sleep(time.Second)
			continue
		}
		client.epGrpcClient = rpcClient

		// start the watch
		endpointRPCClient := netproto.NewEndpointApiClient(rpcClient.ClientConn)
		stream, err := endpointRPCClient.WatchEndpoints(ctx, &api.ObjectMeta{})
		if err != nil {
			rpcClient.Close()
			log.Errorf("Error watching endpoint. Err: %v", err)

			if client.isStopped() {
				return
			}
			time.Sleep(time.Second)
			continue
		}

		// loop till the end
		for {
			// receive from stream
			evt, err := stream.Recv()
			if err != nil {
				log.Errorf("Error receiving from watch channel. Exiting endpoint watch. Err: %v", err)

				if client.isStopped() {
					rpcClient.Close()
					return
				}
				time.Sleep(time.Second)
				break
			}

			log.Infof("Ctrlerif: agent %s got Endpoint watch event: {%+v}", client.getAgentName(), evt)

			// process the endpoint event
			go client.processEndpointEvent(*evt)
		}
		rpcClient.Close()
	}
}

// runSecurityGroupWatcher runs sg watcher loop
func (client *NpmClient) runSecurityGroupWatcher(ctx context.Context) {
	// setup wait group
	client.waitGrp.Add(1)
	defer client.waitGrp.Done()

	for {
		// create a grpc client
		rpcClient, err := rpckit.NewRPCClient(client.getAgentName(), client.srvURL,
			rpckit.WithBalancer(balancer.New(client.resolverClient)), rpckit.WithRemoteServerName(globals.Npm))
		if err != nil {
			log.Errorf("Error connecting to grpc server. Err: %v", err)

			if client.isStopped() {
				return
			}
			time.Sleep(time.Second)
			continue
		}
		client.sgGrpcClient = rpcClient

		// start the watch
		sgRPCClient := netproto.NewSecurityApiClient(rpcClient.ClientConn)
		stream, err := sgRPCClient.WatchSecurityGroups(ctx, &api.ObjectMeta{})
		if err != nil {
			rpcClient.Close()
			log.Errorf("Error watching security group. Err: %v", err)

			if client.isStopped() {
				return
			}

			time.Sleep(time.Second)
			continue
		}

		// loop till the end
		for {
			// receive from stream
			evt, err := stream.Recv()
			if err != nil {
				log.Errorf("Error receiving from watch channel. Exiting security group watch. Err: %v", err)

				if client.isStopped() {
					rpcClient.Close()
					return
				}

				time.Sleep(time.Second)
				break
			}

			log.Infof("Ctrlerif: agent %s got Security group watch event: Type: {%+v} sg:{%+v}", client.getAgentName(), evt.EventType, evt.SecurityGroup.ObjectMeta)

			go client.processSecurityGroupEvent(*evt)
		}
		rpcClient.Close()
	}
}

// runSecurityPolicyWatcher runs sg policy watcher loop
func (client *NpmClient) runSecurityPolicyWatcher(ctx context.Context) {
	// setup wait group
	client.waitGrp.Add(1)
	defer client.waitGrp.Done()

	for {
		// create a grpc client
		rpcClient, err := rpckit.NewRPCClient(client.getAgentName(), client.srvURL,
			rpckit.WithBalancer(balancer.New(client.resolverClient)), rpckit.WithRemoteServerName(globals.Npm))
		if err != nil {
			log.Errorf("Error connecting to grpc server. Err: %v", err)

			if client.isStopped() {
				return
			}
			time.Sleep(time.Second)
			continue
		}
		client.sgpGrpcClient = rpcClient

		// start the watch
		sgpRPCClient := netproto.NewSGPolicyApiClient(rpcClient.ClientConn)
		stream, err := sgpRPCClient.WatchSGPolicys(ctx, &api.ObjectMeta{})
		if err != nil {
			rpcClient.Close()
			log.Errorf("Error watching security policy. Err: %v", err)

			if client.isStopped() {
				return
			}

			time.Sleep(time.Second)
			continue
		}

		// loop till the end
		for {
			// receive from stream
			evt, err := stream.Recv()
			if err != nil {
				log.Errorf("Error receiving from watch channel. Exiting security policy watch. Err: %v", err)

				if client.isStopped() {
					rpcClient.Close()
					return
				}

				time.Sleep(time.Second)
				break
			}

			log.Infof("Ctrlerif: agent %s got Security policy watch event: Type: {%+v} sg:{%+v}", client.getAgentName(), evt.EventType, evt.SGPolicy.ObjectMeta)

			go client.processSecurityPolicyEvent(*evt)
		}
		rpcClient.Close()
	}
}

// Stop stops npm client and all watching go routines
func (client *NpmClient) Stop() {
	client.Lock()
	client.stopped = true
	client.Unlock()
	client.watchCancel()
	client.waitGrp.Wait()
	if client.netGrpcClient != nil {
		client.netGrpcClient.Close()
	}
	if client.sgGrpcClient != nil {
		client.sgGrpcClient.Close()
	}
	if client.epGrpcClient != nil {
		client.epGrpcClient.Close()
	}
	if client.sgpGrpcClient != nil {
		client.sgpGrpcClient.Close()
	}
}

// isStopped is NPM client stopped
func (client *NpmClient) isStopped() bool {
	client.Lock()
	defer client.Unlock()
	return client.stopped
}

// EndpointCreateReq creates an endpoint
func (client *NpmClient) EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, error) {
	// keep track of pending request
	client.Lock()
	client.pendingEpCreate[epinfo.Name] = epinfo
	client.Unlock()

	// create a grpc client
	rpcClient, err := rpckit.NewRPCClient(client.getAgentName(), client.srvURL,
		rpckit.WithBalancer(balancer.New(client.resolverClient)), rpckit.WithRemoteServerName(globals.Npm))
	if err != nil {
		log.Errorf("Error connecting to grpc server. Err: %v", err)
		return nil, err
	}
	defer rpcClient.Close()

	// make an RPC call to controller
	endpointRPCClient := netproto.NewEndpointApiClient(rpcClient.ClientConn)
	ep, err := endpointRPCClient.CreateEndpoint(context.Background(), epinfo)
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

	// create a grpc client
	rpcClient, err := rpckit.NewRPCClient(client.getAgentName(), client.srvURL,
		rpckit.WithBalancer(balancer.New(client.resolverClient)), rpckit.WithRemoteServerName(globals.Npm))
	if err != nil {
		log.Errorf("Error connecting to grpc server. Err: %v", err)
		return nil, err
	}
	defer rpcClient.Close()

	// make an RPC call to controller
	endpointRPCClient := netproto.NewEndpointApiClient(rpcClient.ClientConn)
	ep, err := endpointRPCClient.DeleteEndpoint(context.Background(), epinfo)
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
