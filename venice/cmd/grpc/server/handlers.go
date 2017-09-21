package server

import (
	"fmt"
	"os"
	"strings"
	"time"

	context "golang.org/x/net/context"
	k8sclient "k8s.io/client-go/kubernetes"
	k8srest "k8s.io/client-go/rest"

	"github.com/pensando/sw/venice/cmd/apiclient"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/services"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	kstore "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/quorum"
	"github.com/pensando/sw/venice/utils/quorum/store"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/version"
)

const (
	maxIters              = 50
	sleepBetweenItersMsec = 100
	apiClientWaitTimeMsec = 200

	masterLeaderKey = "master"
)

// clusterRPCHandler handles all cluster gRPC calls.
type clusterRPCHandler struct {
}

// PreJoin handles the prejoin request for joining a cluster. Will fail if
// already part of a cluster.
func (c *clusterRPCHandler) PreJoin(ctx context.Context, req *grpc.ClusterPreJoinReq) (*grpc.ClusterPreJoinResp, error) {
	if cluster, err := utils.GetCluster(); err != nil {
		return nil, err
	} else if cluster != nil {
		return nil, fmt.Errorf("Already part of cluster +%v", cluster)
	}
	return &grpc.ClusterPreJoinResp{SwVersion: version.Version}, nil
}

// Join handles the join request for a cluster. If part of quorum, it will
// start the K/V store process with the provided configuration.
func (c *clusterRPCHandler) Join(ctx context.Context, req *grpc.ClusterJoinReq) (*grpc.ClusterJoinResp, error) {
	// Check again if not in cluster.
	if cluster, err := utils.GetCluster(); err != nil {
		return nil, err
	} else if cluster != nil {
		return nil, fmt.Errorf("Already part of cluster +%v", cluster)
	}

	cl := utils.Cluster{
		Name:        req.Name,
		UUID:        req.Uuid,
		VirtualIP:   req.VirtualIp,
		QuorumNodes: req.QuorumNodes,
		NTPServers:  req.NTPServers,
	}

	// Record cluster membership on local FS.
	if err := utils.SaveCluster(&cl); err != nil {
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

		env.QuorumNodes = req.QuorumNodes
		env.Quorum = quorumIntf
		env.KVServers = kvServers

		ii := 0
		for ; ii < maxIters; ii++ {
			members, errL := env.Quorum.List()
			if errL == nil {
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
		config := &k8srest.Config{
			Host: fmt.Sprintf("%v:%v", req.VirtualIp, globals.KubeAPIServerPort),
		}
		env.K8sService = services.NewK8sService(k8sclient.NewForConfigOrDie(config))
		env.ResolverService = services.NewResolverService(env.K8sService)
		env.MasterService = services.NewMasterService(req.VirtualIp, services.WithK8sSvcMasterOption(env.K8sService),
			services.WithResolverSvcMasterOption(env.ResolverService))
		env.NtpService = services.NewNtpService(req.NTPServers)
		env.CfgWatcherService = apiclient.NewCfgWatcherService(env.Logger)

		env.SystemdService.Start() // must be called before dependent services
		env.VipService.AddVirtualIPs(req.VirtualIp)
		env.MasterService.Start()
		env.LeaderService.Start()
		env.CfgWatcherService.Start()

		// We let the quorum nodes use the external NTP server and non-quorum nodes use the VIP for ntp server
		env.NtpService.NtpConfigFile(req.NTPServers)

		// create and register the RPC handler for service object.
		types.RegisterServiceAPIServer(env.RPCServer.GrpcServer, service.NewRPCHandler(env.ResolverService))

		// Create and register the RPC handler for SmartNIC service
		go RegisterSmartNICServer()

	} else {
		env.NtpService = services.NewNtpService(req.NTPServers)
		env.NtpService.NtpConfigFile([]string{req.VirtualIp})
		env.SystemdService = services.NewSystemdService()
		env.SystemdService.Start() // must be called before dependent services
	}

	env.NodeService = services.NewNodeService(req.VirtualIp)
	// Start node services. Currently we are running Node services on Quorum nodes also
	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}

	return &grpc.ClusterJoinResp{}, nil
}

func (c *clusterRPCHandler) Disjoin(ctx context.Context, req *grpc.ClusterDisjoinReq) (*grpc.ClusterDisjoinResp, error) {
	err := env.SystemdService.StopUnit("pen-kubelet.service")
	if err != nil {
		env.Logger.Infof("Error %v while stopping pen-kubelet", err)
	}
	err2 := env.SystemdService.StartUnit("pen-nodecleanup.service")
	if err2 != nil {
		env.Logger.Infof("Error %v while cleaning up node", err2)
	}
	if err == nil {
		err = err2
	}
	return &grpc.ClusterDisjoinResp{}, err
}

// RegisterSmartNICServer creates and register smartNIC server with retries
func RegisterSmartNICServer() {
	for i := 0; i < maxIters; i++ {
		if env.CfgWatcherService.APIClient() != nil {
			grpc.RegisterSmartNICServer(env.RPCServer.GrpcServer, smartnic.NewRPCServer(env.CfgWatcherService.APIClient().Cluster(),
				env.CfgWatcherService.APIClient().Node(), env.CfgWatcherService.APIClient().SmartNIC()))
			return
		}
		time.Sleep(apiClientWaitTimeMsec * time.Millisecond)
	}
	log.Fatalf("Failed to register smartNIC RPCserver, apiClient not up")
}
