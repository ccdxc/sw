package health

import (
	"sync"
	"time"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/cache"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// RPCServer is the server running on the leader node to receive hearbeats
type RPCServer struct {
	sync.Mutex
	nodes          map[string]time.Time
	stop           chan struct{}
	updateInterval time.Duration
}

// Client is the client used to send seartbeats to the leader
type Client struct {
	resolverClient  resolver.Interface
	rpcClient       *rpckit.RPCClient
	heartbeatClient grpc.NodeHeartbeatClient
	stop            chan struct{}
}

// NewRPCServer creates and starts an RPCServer instance
func NewRPCServer(updateInterval time.Duration) *RPCServer {
	rpcServer := &RPCServer{
		nodes:          make(map[string]time.Time),
		stop:           make(chan struct{}),
		updateInterval: updateInterval,
	}
	go rpcServer.periodicUpdate(updateInterval)
	return rpcServer
}

func (s *RPCServer) updateCondition(node *cache.NodeState, alive bool, lastHeartbeat time.Time) {
	var status cluster.ConditionStatus
	if alive {
		status = cluster.ConditionStatus_TRUE
	} else {
		status = cluster.ConditionStatus_UNKNOWN
	}
	for _, cond := range node.Status.Conditions {
		if cond.Type == cluster.NodeCondition_HEALTHY.String() {
			// update
			cond.LastTransitionTime = lastHeartbeat.UTC().Format(time.RFC3339)
			cond.Status = status.String()
			return
		}
	}

	// create
	node.Status.Conditions = append(node.Status.Conditions,
		cluster.NodeCondition{
			Type:               cluster.NodeCondition_HEALTHY.String(),
			LastTransitionTime: lastHeartbeat.UTC().Format(time.RFC3339),
			Status:             status.String(),
		})
}

func (s *RPCServer) update() {
	nodes, err := env.StateMgr.ListNodes()
	if err != nil {
		// Not much we can do other than return
		log.Infof("No nodes")
		return
	}
	log.Infof("Total node count: %v", len(nodes))
	for _, node := range nodes {
		log.Infof("Node: %+v", node)
		node.Lock()
		defer node.Unlock()
		if _, ok := s.nodes[node.Name]; ok == false {
			s.updateCondition(node, false, time.Now())
		} else {
			s.updateCondition(node, time.Now().Sub(s.nodes[node.Name]) < s.updateInterval,
				s.nodes[node.Name])
		}
		env.StateMgr.UpdateNode(node.Node, true)
		log.Infof("Updating Node %+v", node)
	}
}

func (s *RPCServer) periodicUpdate(interval time.Duration) {
	ticker := time.NewTicker(interval)
	defer ticker.Stop()
	for {
		select {
		case _ = <-ticker.C:
			s.update()
		case _ = <-s.stop:
			log.Infof("Stop")
			return
		}
	}
}

// Heartbeat is the callback called when a heartbeat is received
func (s *RPCServer) Heartbeat(ctx context.Context, req *grpc.HeartbeatRequest) (*grpc.HeartbeatResponse, error) {
	s.Lock()
	defer s.Unlock()
	s.nodes[req.NodeID] = time.Now()
	log.Infof("Got heartbeat from %s", req.NodeID)
	return &grpc.HeartbeatResponse{}, nil
}

// NewClient creates a new Client
func NewClient(resolverClient resolver.Interface) *Client {
	rpcClient, err := rpckit.NewRPCClient(
		"health-client", globals.CmdNICUpdatesSvc,
		rpckit.WithBalancer(balancer.New(resolverClient)))
	if err != nil {
		return nil
	}

	heartbeatClient := grpc.NewNodeHeartbeatClient(rpcClient.ClientConn)

	return &Client{
		resolverClient:  resolverClient,
		rpcClient:       rpcClient,
		heartbeatClient: heartbeatClient,
		stop:            make(chan struct{}),
	}
}

// Start starts the periodic headbeats to the Leader
func (s *Client) Start(interval time.Duration) {
	// Send periodic heartbeats
	cluster, _ := utils.GetCluster()
	req := &grpc.HeartbeatRequest{
		NodeID: cluster.NodeID,
	}
	go func() {
		ticker := time.NewTicker(interval)
		defer ticker.Stop()
		for {
			select {
			case _ = <-ticker.C:
				log.Infof("Sending heartbeat with NodeID %s", req.NodeID)
				s.heartbeatClient.Heartbeat(context.Background(), req)
			case _ = <-s.stop:
				log.Infof("Stop")
				return
			}
		}
	}()
}

// Stop stops the periodic heartbeats to the leader
func (s *Client) Stop() {
	s.stop <- struct{}{}
}
