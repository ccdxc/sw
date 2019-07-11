package health

import (
	"fmt"
	"sync"
	"time"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/cmd/cache"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	heartbeatInterval = 30 * time.Second
)

// RPCServer is the server running on the leader node to receive heartbeats
type RPCServer struct {
	sync.Mutex
	nodes          map[string]time.Time
	stop           chan struct{}
	updateInterval time.Duration
}

// Client is the client used to send heartbeats to the leader
type Client struct {
	resolverClient  resolver.Interface
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

// Stop stops the thread that updates the health of the nodes to api-server
func (s *RPCServer) Stop() {
	s.stop <- struct{}{}
}

func (s *RPCServer) updateCondition(node *cache.NodeState, alive bool, lastHeartbeat time.Time) {
	var status cluster.ConditionStatus
	if alive {
		status = cluster.ConditionStatus_TRUE
	} else {
		status = cluster.ConditionStatus_UNKNOWN
	}
	for i := 0; i < len(node.Status.Conditions); i++ {
		cond := &node.Status.Conditions[i]
		if cond.Type == cluster.NodeCondition_HEALTHY.String() {
			// true to unknown
			if cond.Status == cluster.ConditionStatus_TRUE.String() {
				if status == cluster.ConditionStatus_UNKNOWN {
					recorder.Event(eventtypes.NODE_UNREACHABLE,
						fmt.Sprintf("Venice node %s is %s", node.GetName(), eventtypes.NODE_UNREACHABLE.String()), node.Node)
				}
			}

			// unknown to true
			if cond.Status == cluster.ConditionStatus_UNKNOWN.String() {
				if status == cluster.ConditionStatus_TRUE {
					recorder.Event(eventtypes.NODE_HEALTHY,
						fmt.Sprintf("Venice node %s is %s", node.GetName(), cluster.NodeCondition_HEALTHY.String()), node.Node)
				}
			}

			// update
			cond.LastTransitionTime = lastHeartbeat.UTC().Format(time.RFC3339)
			cond.Status = status.String()
			return
		}
	}

	// create
	if status == cluster.ConditionStatus_UNKNOWN {
		recorder.Event(eventtypes.NODE_UNREACHABLE,
			fmt.Sprintf("Venice node %s is %s", node.GetName(), eventtypes.NODE_UNREACHABLE.String()), node.Node)
	}
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
		if _, ok := s.nodes[node.Name]; !ok {
			s.updateCondition(node, false, time.Now())
		} else {
			s.updateCondition(node, time.Since(s.nodes[node.Name]) < s.updateInterval,
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
		case <-ticker.C:
			s.update()
		case <-s.stop:
			log.Infof("Stop")
			return
		}
	}
}

// Heartbeat is the callback called when a heartbeat is received
func (s *RPCServer) Heartbeat(ctx context.Context, req *grpc.HeartbeatRequest) (*grpc.HeartbeatResponse, error) {
	s.Lock()
	defer s.Unlock()
	if _, ok := s.nodes[req.NodeID]; !ok { // receiving heart beat for the first time
		node := &cluster.Node{}
		node.Defaults("all")
		node.Name = req.NodeID
		recorder.Event(eventtypes.NODE_HEALTHY,
			fmt.Sprintf("Venice node %s is %s", req.GetNodeID(), cluster.NodeCondition_HEALTHY.String()), node)
	}
	s.nodes[req.NodeID] = time.Now()
	log.Infof("Got heartbeat from %s", req.NodeID)
	return &grpc.HeartbeatResponse{}, nil
}

// NewClient creates a new Client
func NewClient(resolverClient resolver.Interface) *Client {

	client := &Client{
		resolverClient: resolverClient,
		stop:           make(chan struct{}),
	}
	client.start(heartbeatInterval)

	return client
}

// Start starts the periodic heartbeats to the Leader
func (c *Client) start(interval time.Duration) {
	go func() {
		// Try to connect. It make take a few tries
		for {
			select {
			case <-c.stop:
				log.Infof("Stopping health client while trying to connect")
				return
			default:

			}
			rpcClient, err := rpckit.NewRPCClient(
				"health-client", globals.CmdNICUpdatesSvc,
				rpckit.WithBalancer(balancer.New(c.resolverClient)))
			if err != nil {
				time.Sleep(time.Second)
				continue
			}
			defer rpcClient.Close()

			c.heartbeatClient = grpc.NewNodeHeartbeatClient(
				rpcClient.ClientConn)
			break
		}

		// Send periodic heartbeats
		clstr, _ := utils.GetCluster()
		req := &grpc.HeartbeatRequest{
			NodeID: clstr.NodeID,
		}

		ticker := time.NewTicker(interval)
		defer ticker.Stop()
		for {
			select {
			case <-ticker.C:
				log.Infof("Sending heartbeat with NodeID %s", req.NodeID)
				c.heartbeatClient.Heartbeat(context.Background(), req)
			case <-c.stop:
				log.Infof("Stopping health client")
				return
			}
		}
	}()
}

// Stop stops the periodic heartbeats to the leader
func (c *Client) Stop() {
	close(c.stop)
}
