// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package ctrlerif

import (
	"context"
	"fmt"
	"sync"
	"time"

	"google.golang.org/grpc/connectivity"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/generated/nimbus"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	debugStats "github.com/pensando/sw/venice/utils/debug/stats"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const evChanLength = 2000
const maxOpretry = 10

// NpmClient is the network policy mgr client
type NpmClient struct {
	sync.Mutex                         // lock the npm client
	srvURL          string             // NPM rpc server URL
	resolverClient  resolver.Interface // Resolver client
	waitGrp         sync.WaitGroup     // wait group to wait on all go routines to exit
	agent           types.CtrlerIntf   // net Agent API
	rpcClient       *rpckit.RPCClient
	clientFactory   *rpckit.RPCClientFactory      // client factory
	watchCtx        context.Context               // ctx for network watch
	watchCancel     context.CancelFunc            // cancel for network watch
	stopped         bool                          // is the npm client stopped?
	pendingEpCreate map[string]*netproto.Endpoint // pending endpoint create request
	pendingEpDelete map[string]*netproto.Endpoint // pending endpoint delete requests
	debugStats      *debugStats.Stats
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
		clientFactory:   rpckit.NewClientFactory(agent.GetAgentID()),
	}

	client.debugStats = debugStats.New(fmt.Sprintf("npmif-%s", client.getAgentName())).Tsdb().Kind("netagentStats").TsdbPeriod(5 * time.Second).Build()

	// register the NPM client as a controller plugin
	err := agent.RegisterCtrlerIf(&client)
	if err != nil {
		log.Fatalf("Error registering the controller interface. Err: %v", err)
		return nil, err
	}

	client.waitGrp.Add(1)
	go client.runWatchLoop(watchCtx)

	return &client, nil
}

// getAgentName returns a unique name for each agent instance
func (client *NpmClient) getAgentName() string {
	return "netagent-" + client.agent.GetAgentID()
}

func (client *NpmClient) runWatchLoop(ctx context.Context) {
	// setup wait group
	defer client.waitGrp.Done()

	// loop forever trying to connect to NPM
	for {
		// create a grpc client
		rpcClient, err := client.clientFactory.NewRPCClient(client.getAgentName(), client.srvURL,
			rpckit.WithBalancer(balancer.New(client.resolverClient)), rpckit.WithRemoteServerName(globals.Npm))
		if err != nil {
			log.Errorf("Error connecting to grpc server. Err: %v", err)

			if client.isStopped() {
				log.Warnf("%s: NPMClient stopped. exiting..", client.getAgentName())
				return
			}
			time.Sleep(time.Second)
			continue
		}
		client.rpcClient = rpcClient

		log.Infof("Client %s is Connected to %v", client.getAgentName(), client.srvURL)

		nimbusClient, err := nimbus.NewNimbusClient(client.getAgentName(), client.srvURL, rpcClient)
		if err != nil {
			log.Errorf("Error creating nimbus client. Err: %v", err)
			time.Sleep(time.Second)
			continue
		}

		// watch objects
		go nimbusClient.WatchNetworks(client.watchCtx, client.agent)
		go nimbusClient.WatchEndpoints(client.watchCtx, client.agent)
		go nimbusClient.WatchSecurityGroups(client.watchCtx, client.agent)
		go nimbusClient.WatchSecurityProfiles(client.watchCtx, client.agent)
		go nimbusClient.WatchAggregate(client.watchCtx, []string{"App", "NetworkSecurityPolicy"}, client.agent)
		go nimbusClient.WatchIPAMPolicys(client.watchCtx, client.agent)

		// spawn worker thread to update network interface status to npm and watch for updates
		go client.netifWorker()

		// wait for all watchers to exit
		time.Sleep(time.Millisecond * 100) // wait a little before above go routines can increment wait group counters
		nimbusClient.Wait()
		log.Infof("%s: Closing rpc connection", client.getAgentName())

		rpcClient.Close()
		if ctx.Err() != nil {
			log.Warnf("Watch context error %v. exiting..", ctx.Err())
			return
		}
		if client.isStopped() {
			log.Warnf("%s: NPMClient stopped. exiting..", client.getAgentName())
			return
		}
		time.Sleep(time.Second)
		log.Infof("%s: Trying reconnecting to server", client.getAgentName())
	}
}

// IsConnected returns true if GRPC connection is ready
func (client *NpmClient) IsConnected() bool {
	if (client.rpcClient == nil) || (client.rpcClient.ClientConn == nil) || (client.rpcClient.ClientConn.GetState() != connectivity.Ready) {
		return false
	}

	return true
}

// Stop stops npm client and all watching go routines
func (client *NpmClient) Stop() {
	log.Infof("%s: stopping NPMClient..", client.getAgentName())
	client.Lock()
	client.stopped = true
	client.Unlock()
	client.watchCancel()
	client.waitGrp.Wait()
	log.Infof("%s: stopping NPMClient complete.", client.getAgentName())
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
	rpcClient, err := client.clientFactory.NewRPCClient(client.getAgentName(), client.srvURL,
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
	rpcClient, err := client.clientFactory.NewRPCClient(client.getAgentName(), client.srvURL,
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

// createNpmNetIfs creates network-interface object in npm to reflect the interface status to the user
func (client *NpmClient) netifWorker() {

	netIfRPCClient := netproto.NewInterfaceApiClient(client.rpcClient.ClientConn)
	idPrefix := client.agent.GetAgentID()

	// fetch interfaes discovered by hw and populate them in npm
	netifs := client.agent.ListInterface()
	for _, n := range netifs {
		netif := *n
		netif.ObjectMeta.Name = idPrefix + "-" + netif.ObjectMeta.Name
		if _, err := netIfRPCClient.CreateInterface(context.Background(), &netif); err != nil {
			log.Errorf("Error resp from netctrler for netif create {%+v}. Err: %v", netif, err)
			continue
		}
	}

	// TODO: pull a loop here that watches delphi for link status updates, etc. from HAL
}
