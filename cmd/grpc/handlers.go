package grpc

import (
	"fmt"
	context "golang.org/x/net/context"
	"net"
	"os"
	"time"

	log "github.com/Sirupsen/logrus"

	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/cmd/services"
	"github.com/pensando/sw/cmd/utils"
	kvstore "github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/quorum"
	"github.com/pensando/sw/utils/quorum/store"
	"github.com/pensando/sw/utils/rpckit"
	"github.com/pensando/sw/utils/runtime"
	"github.com/pensando/sw/utils/version"
)

const (
	maxIters              = 50
	sleepBetweenItersMsec = 100
)

// RunServer creates a gRPC server for cluster operations.
func RunServer(url, certFile, keyFile, caFile string, stopChannel chan bool) {
	// create an rpc handler object
	h := &clusterRPCHandler{}

	// create an RPC server
	rpcServer, err := rpckit.NewRPCServer(url, certFile, keyFile, caFile)
	if err != nil {
		log.Fatalf("Error creating grpc server: %v", err)
	}

	// register the RPC handler
	RegisterClusterServer(rpcServer.GrpcServer, h)
	defer func() { rpcServer.Stop() }()

	// wait forever
	<-stopChannel
}

// clusterRPCHandler handles all cluster gRPC calls.
type clusterRPCHandler struct {
}

// PreJoin handles the prejoin request for joining a cluster. Will fail if
// already part of a cluster.
func (c *clusterRPCHandler) PreJoin(ctx context.Context, req *ClusterPreJoinReq) (*ClusterPreJoinResp, error) {
	if cluster, err := utils.GetCluster(); err != nil {
		return nil, err
	} else if cluster != nil {
		return nil, fmt.Errorf("Already part of cluster +%v", cluster)
	}
	return &ClusterPreJoinResp{SwVersion: version.Version}, nil
}

// Join handles the join request for a cluster. If part of quorum, it will
// start the K/V store process with the provided configuration.
func (c *clusterRPCHandler) Join(ctx context.Context, req *ClusterJoinReq) (*ClusterJoinResp, error) {
	// Check again if not in cluster.
	if cluster, err := utils.GetCluster(); err != nil {
		return nil, err
	} else if cluster != nil {
		return nil, fmt.Errorf("Already part of cluster +%v", cluster)
	}

	// Check if quorum node.
	if req.QuorumConfig != nil {
		hostname, _ := os.Hostname()
		members := make([]quorum.Member, 0)
		found := false
		for _, member := range req.QuorumConfig.QuorumMembers {
			if hostname == member.Name {
				found = true
			}
			members = append(members, quorum.Member{
				Name:       member.Name,
				PeerURLs:   member.PeerUrls,
				ClientURLs: member.ClientUrls,
			})
		}
		if !found {
			return nil, fmt.Errorf("%v received Join without itself in it", hostname)
		}

		qConfig := &quorum.Config{
			Type:       store.KVStoreTypeEtcd,
			ID:         req.QuorumConfig.Id,
			DataDir:    env.Options.KVStore.DataDir,
			CfgFile:    env.Options.KVStore.ConfigFile,
			MemberName: hostname,
			Existing:   false,
			Members:    members,
		}

		quorumIntf, err := store.New(qConfig)
		if err != nil {
			log.Errorf("Failed to create quorum with error: %v", err)
			return nil, err
		}

		env.Quorum = quorumIntf

		ii := 0
		for ; ii < maxIters; ii++ {
			members, err := env.Quorum.List()
			if err == nil {
				if len(members) == len(req.QuorumConfig.QuorumMembers) {
					break
				}
			}
			time.Sleep(sleepBetweenItersMsec * time.Millisecond)
		}

		if ii == maxIters {
			log.Errorf("KV Store failed to come up in %v seconds", maxIters*sleepBetweenItersMsec/1000)
			return nil, fmt.Errorf("KV Store failed to come up in %v seconds", maxIters*sleepBetweenItersMsec/1000)
		}

		addrs, _ := net.LookupHost(hostname)
		sConfig := kvstore.Config{
			Type:    kvstore.KVStoreTypeEtcd,
			Servers: []string{fmt.Sprintf("http://%s:%s", addrs[0], env.Options.KVStore.ClientPort)},
			Codec:   runtime.NewJSONCodec(env.Scheme),
		}

		kv, err := kvstore.New(sConfig)
		if err != nil {
			log.Errorf("Failed to create kvstore, error: %v", err)
			return nil, err
		}

		env.KVStore = kv

		// Start leader election on quorum nodes.
		go services.NewLeaderService(hostname).Start()
	}

	// Record cluster membership on local FS.
	if err := utils.SaveCluster(&utils.Cluster{
		Name:      req.Name,
		UUID:      req.Uuid,
		VirtualIP: req.VirtualIp,
	}); err != nil {
		return nil, err
	}

	return &ClusterJoinResp{}, nil
}
