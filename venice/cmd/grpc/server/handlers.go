// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package server

import (
	"crypto"
	"fmt"
	"strings"
	"time"

	context "golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/cache"
	"github.com/pensando/sw/venice/cmd/credentials"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/auth"
	certutils "github.com/pensando/sw/venice/cmd/grpc/server/certificates/utils"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/cmd/rolloutclient"
	"github.com/pensando/sw/venice/cmd/services"
	"github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/etcd"
	kstore "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/nodewatcher"
	"github.com/pensando/sw/venice/utils/quorum"
	"github.com/pensando/sw/venice/utils/quorum/store"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/version"
)

const (
	maxIters              = 500
	sleepBetweenItersMsec = 100
	apiClientWaitTimeMsec = 200

	masterLeaderKey = "master"
)

// clusterRPCHandler handles all cluster gRPC calls.
type clusterRPCHandler struct {
	peerTransportKey crypto.PublicKey
}

// PreJoin handles the prejoin request for joining a cluster. Will fail if
// already part of a cluster.
func (c *clusterRPCHandler) PreJoin(ctx context.Context, req *grpc.ClusterPreJoinReq) (*grpc.ClusterPreJoinResp, error) {
	if cluster, err := utils.GetCluster(); err != nil {
		return nil, err
	} else if cluster != nil {
		return nil, fmt.Errorf("Already part of cluster +%v", cluster)
	}
	var transportKeyBytes []byte
	if req.TransportKey != nil {
		if env.CertMgr == nil {
			cm, err := certmgr.NewGoCryptoCertificateMgr(certutils.GetCertificateMgrDir())
			if err != nil {
				return nil, fmt.Errorf("Failed to instantiate certificate manager, error: %v", err)
			}
			env.CertMgr = cm
		}
		// PreJoin messages go out to all quorum nodes during cluster formation, including the sender.
		// If CertMgr is already bootstrapped, it means that we are the sender and we don't need to do anything
		if !env.CertMgr.IsReady() {
			transportKey, err := env.CertMgr.GetKeyAgreementKey("self")
			if err != nil {
				return nil, fmt.Errorf("Error generating key-agreement-key")
			}
			if c.peerTransportKey != nil {
				log.Warnf("Overriding peerTransportKey from previous unfinished exchange")
			}
			c.peerTransportKey = env.CertMgr.UnmarshalKeyAgreementKey(req.TransportKey)
			transportKeyBytes = env.CertMgr.MarshalKeyAgreementKey(transportKey)
		}
	}
	return &grpc.ClusterPreJoinResp{
		SwVersion:    version.Version,
		TransportKey: transportKeyBytes,
	}, nil
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
		NodeID:      req.NodeId,
	}

	// Record cluster membership on local FS.
	if err := utils.SaveCluster(&cl); err != nil {
		return nil, err
	}

	var shouldStartAuthServer = false
	if req.CertMgrBundle != nil && !env.CertMgr.IsReady() {
		defer func() { c.peerTransportKey = nil }()
		err := certutils.UnpackCertMgrBundle(env.CertMgr, req.CertMgrBundle, c.peerTransportKey)
		if err != nil {
			return nil, fmt.Errorf("Error unpacking CertMgr bundle: %v", err)
		}
		err = env.CertMgr.StartCa(false)
		if err != nil || !env.CertMgr.IsReady() {
			return nil, fmt.Errorf("Error starting CertMgr: %v", err)
		}
		// Now that CA has started, Recorderclients can talk RPC to eventsProxy
		env.Recorder.StartExport()
		// Launch go routine to monitor health updates of smartNIC objects and update status
		go func() {
			env.NICService.MonitorHealth()
		}()

		if env.AuthRPCServer == nil {
			//  start the RPC server after creating the ResolverService (if applicable)
			//	Hence move the start-Auth-server code towards end of the function
			shouldStartAuthServer = true
		}
	}
	env.QuorumNodes = req.QuorumNodes

	servers := make([]string, 0)
	for _, jj := range env.QuorumNodes {
		servers = append(servers, fmt.Sprintf("%s:%s", jj, globals.CMDResolverPort))
	}
	env.ResolverClient = resolver.New(&resolver.Config{Name: req.NodeId, Servers: servers})

	// Check if quorum node.
	if req.QuorumConfig != nil {
		kvServers := make([]string, 0)
		members := make([]quorum.Member, 0)
		found := false
		for _, member := range req.QuorumConfig.QuorumMembers {
			if req.NodeId == member.Name {
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
			return nil, fmt.Errorf("%v received Join without itself in it", req.NodeId)
		}

		qConfig := &quorum.Config{
			Type:       store.KVStoreTypeEtcd,
			ID:         req.QuorumConfig.Id,
			DataDir:    env.Options.KVStore.DataDir,
			CfgFile:    env.Options.KVStore.ConfigFile,
			MemberName: req.NodeId,
			Existing:   false,
			Members:    members,
		}

		err := credentials.SetQuorumInstanceAuth(qConfig, env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustRoots())
		if err != nil {
			log.Errorf("Failed to obtain instance auth credentials for quorum with error: %v", err)
			return nil, err
		}

		err = credentials.GenQuorumClientAuth(env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustRoots())
		if err != nil {
			log.Errorf("Failed to obtain client auth credentials for quorum with error: %v", err)
			return nil, err
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

		kvStoreTLSConfig, err := etcd.GetEtcdClientCredentials()
		if err != nil {
			log.Errorf("Failed to get kvstore client credentials with error: %v", err)
			return nil, err
		}

		sConfig := kstore.Config{
			Type:        kstore.KVStoreTypeEtcd,
			Servers:     kvServers,
			Codec:       runtime.NewJSONCodec(env.Scheme),
			Credentials: kvStoreTLSConfig,
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

		err = credentials.GenKubernetesCredentials(req.NodeId, env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustRoots(), []string{req.VirtualIp})
		if err != nil {
			log.Errorf("Failed to generate Kubernetes credentials, error: %v", err)
			// try to proceed anyway
		} else {
			log.Infof("generated k8s credentials, dir: %v, err: %v", globals.KubernetesConfigDir, err)
		}

		err = credentials.GenVosAuth(env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustChain())
		if err != nil {
			log.Errorf("Failed to generate VOS credentials, error: %v", err)
			// try to proceed anyway
		}

		k8sConfig := services.K8sServiceConfig{
			OverriddenModules: utils.GetOverriddenModules(""),
			DisabledModules:   utils.GetDisabledModules(""),
		}
		env.K8sService = services.NewK8sService(&k8sConfig)
		env.ResolverService = services.NewResolverService(env.K8sService)
		env.KVStore = kv
		env.StateMgr = cache.NewStatemgr()
		// Create leader service before its users
		env.LeaderService = services.NewLeaderService(kv, masterLeaderKey, req.NodeId)
		env.SystemdService = services.NewSystemdService()
		env.VipService = services.NewVIPService()
		env.MasterService = services.NewMasterService(services.WithK8sSvcMasterOption(env.K8sService),
			services.WithResolverSvcMasterOption(env.ResolverService))
		env.NtpService = services.NewNtpService(req.NTPServers)

		env.SystemdService.Start() // must be called before dependent services
		env.VipService.AddVirtualIPs(req.VirtualIp)

		// We let the quorum nodes use the external NTP server and non-quorum nodes use the VIP for ntp server
		env.NtpService.NtpConfigFile(req.NTPServers)
		env.ServiceTracker = services.NewServiceTracker(env.ResolverService)
		env.LeaderService.Register(env.ServiceTracker)

	} else {
		// Generate Kubelet credentials only. Try to continue anyway in case of failure.
		err := credentials.GenKubeletCredentials(req.NodeId, env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustRoots())
		if err != nil {
			log.Errorf("Failed to generate Kubelet credentials, error: %v", err)
			err2 := credentials.RemoveKubeletCredentials()
			if err2 != nil {
				log.Errorf("Error removing Kubelet credentials: %v", err2)
			}
			return nil, err
		}
		env.NtpService = services.NewNtpService(req.NTPServers)
		env.NtpService.NtpConfigFile([]string{req.VirtualIp})
		env.SystemdService = services.NewSystemdService()
		env.SystemdService.Start() // must be called before dependent services

		env.ServiceTracker = services.NewServiceTracker(nil)
	}

	env.NodeService = services.NewNodeService(req.NodeId)
	env.RolloutMgr = services.NewRolloutMgr()
	env.RolloutMgr.Start()
	env.VeniceRolloutClient = rolloutclient.NewVeniceRolloutClient(globals.Rollout, req.NodeId, env.ResolverClient, env.RolloutMgr)
	env.VeniceRolloutClient.Start()

	if req.QuorumConfig != nil {
		env.ServiceRolloutClient = rolloutclient.NewServiceRolloutClient(globals.Rollout, req.NodeId, env.ResolverClient, env.RolloutMgr)
		env.MasterService.Start()
		env.LeaderService.Start()
		env.CfgWatcherService.Start()
	}

	// Start node services. Currently we are running Node services on Quorum nodes also
	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}
	env.ServiceTracker.Run(env.ResolverClient, env.NodeService)

	if shouldStartAuthServer {
		go auth.RunAuthServer(":"+env.Options.GRPCAuthPort, nil)
	}

	node := &cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Name: req.NodeId,
		},
	}
	nodewatcher.NewNodeWatcher(context.Background(), node, env.ResolverClient.(resolver.Interface), 30, env.Logger)

	return &grpc.ClusterJoinResp{}, nil
}

func (c *clusterRPCHandler) Disjoin(ctx context.Context, req *grpc.ClusterDisjoinReq) (*grpc.ClusterDisjoinResp, error) {
	var err error
	// Stop CertificatesMgr and cleanup CA keys
	if env.CertMgr != nil {
		err = env.CertMgr.Close()
		if err != nil {
			env.Logger.Errorf("Error %v while stopping CertificatesMgr", err)
		}
		env.CertMgr = nil
	}
	if env.SystemdService != nil {
		// Services that are started by systemd need to be explicitly stopped
		services := []string{"pen-kubelet", "pen-kube-scheduler", "pen-kube-apiserver", "pen-kube-controller-manager", "pen-etcd"}
		for _, s := range services {
			serviceName := s + ".service"
			err = env.SystemdService.StopUnit(serviceName)
			if err != nil {
				env.Logger.Errorf("Error %v while stopping %s", err, serviceName)
			}
		}
		// Now start the Nodecleanup service, which will stop and remove all running docker containers(except cmd)
		err2 := env.SystemdService.StartUnit("pen-nodecleanup.service")
		if err2 != nil {
			env.Logger.Errorf("Error %v while cleaning up node", err2)
		}
		if err == nil {
			err = err2
		}
	}
	if env.ServiceTracker != nil {
		env.ServiceTracker.Stop()
	}
	if env.ResolverClient != nil {
		r := env.ResolverClient.(resolver.Interface)
		r.Deregister(env.ServiceTracker)
		r.Stop()
		env.ResolverClient = nil
	}
	env.ServiceTracker = nil
	if env.SystemdService != nil {
		env.SystemdService.Stop()
		env.SystemdService = nil
	}
	return &grpc.ClusterDisjoinResp{}, err
}

// RegisterSmartNICRegistrationServer creates and register smartNIC server with retries
func RegisterSmartNICRegistrationServer(smgr *cache.Statemgr) {

	// Start smartNIC gRPC server
	for i := 0; i < maxIters; i++ {

		// create new smartNIC server
		nicServer, err := smartnic.NewRPCServer(env.CfgWatcherService,
			smartnic.HealthWatchInterval,
			smartnic.DeadInterval,
			globals.NmdRESTPort,
			smgr)
		if err != nil {
			time.Sleep(apiClientWaitTimeMsec * time.Millisecond)
			continue
		}

		// Update NIC service
		env.NICService = nicServer
		// Launch go routine to monitor health updates of smartNIC objects and update status
		if cluster, err := utils.GetCluster(); err == nil && cluster != nil {
			// only start here if cmd is getting restarted and is already part of cluster.
			// If its not yet part of cluster, then MonitorHealth() is done as part of ClusterCreateOP/ClusterJoin
			go func() {
				env.NICService.MonitorHealth()
			}()
		}

		// Register smartNIC gRPC server
		log.Debugf("Registering SmartNIC RPCserver")
		grpc.RegisterSmartNICRegistrationServer(env.UnauthRPCServer.GrpcServer, nicServer)
		return
	}
	log.Fatalf("Failed to register SmartNIC RPCserver, apiClient not up")
}
