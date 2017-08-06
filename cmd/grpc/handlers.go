package grpc

import (
	"fmt"
	"os"
	"strings"
	"time"

	context "golang.org/x/net/context"

	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/cmd/services"
	"github.com/pensando/sw/cmd/utils"
	"github.com/pensando/sw/utils/kvstore"
	kstore "github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/quorum"
	"github.com/pensando/sw/utils/quorum/store"
	"github.com/pensando/sw/utils/rpckit"
	"github.com/pensando/sw/utils/runtime"
	"github.com/pensando/sw/utils/version"
)

const (
	maxIters              = 50
	sleepBetweenItersMsec = 100

	masterLeaderKey = "master"
)

// RunServer creates a gRPC server for cluster operations.
func RunServer(url, certFile, keyFile, caFile string, stopChannel chan bool) {
	// create an rpc handler object
	h := &clusterRPCHandler{}

	// create an RPC server
	rpcServer, err := rpckit.NewRPCServer("cmd", url, certFile, keyFile, caFile)
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

	// Record cluster membership on local FS.
	if err := utils.SaveCluster(&utils.Cluster{
		Name:      req.Name,
		UUID:      req.Uuid,
		VirtualIP: req.VirtualIp,
	}); err != nil {
		return nil, err
	}

	// Check if quorum node.
	if req.QuorumConfig != nil {
		kvServers := make([]string, 0)
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
			kvServers = append(kvServers, strings.Join(member.ClientUrls, ","))
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
		env.KVServers = kvServers

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

		sConfig := kstore.Config{
			Type:    kstore.KVStoreTypeEtcd,
			Servers: kvServers,
			Codec:   runtime.NewJSONCodec(env.Scheme),
		}

		var kv kvstore.Interface
		ii = 0
		for ; ii < maxIters; ii++ {
			kv, err = kstore.New(sConfig)
			if err == nil {
				break
			}
			time.Sleep(sleepBetweenItersMsec * time.Millisecond)
		}
		if err != nil {
			log.Errorf("Failed to create kvstore, error: %v", err)
			return nil, err
		}

		env.KVStore = kv
		// Create leader service before its users
		env.LeaderService = services.NewLeaderService(kv, masterLeaderKey, hostname)
		env.SystemdService = services.NewSystemdService()
		env.VipService = services.NewVIPService()
		env.MasterService = services.NewMasterService(req.VirtualIp)
		env.NtpService = services.NewNtpService(req.NTPServers)

		env.SystemdService.Start() // must be called before dependent services
		env.VipService.AddVirtualIPs(req.VirtualIp)
		env.MasterService.Start()
		env.LeaderService.Start()
		env.NtpService.Start()
	} else {
		env.NtpService = services.NewNtpService(req.NTPServers)
		env.NtpService.NtpConfigFile([]string{req.VirtualIp})
	}

	env.NodeService = services.NewNodeService(req.VirtualIp)
	// Start node services. Currently we are running Node services on Quorum nodes also
	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}

	return &ClusterJoinResp{}, nil
}
