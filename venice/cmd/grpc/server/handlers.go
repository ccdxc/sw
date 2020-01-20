// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package server

import (
	"crypto"
	"fmt"
	"os"
	"strings"
	"time"

	context "golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/cmd/credentials"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/auth"
	certutils "github.com/pensando/sw/venice/cmd/grpc/server/certificates/utils"
	"github.com/pensando/sw/venice/cmd/grpc/server/health"
	"github.com/pensando/sw/venice/cmd/rolloutclient"
	"github.com/pensando/sw/venice/cmd/services"
	"github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/etcd"
	kstore "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
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
	peerTransportKey      crypto.PublicKey
	authServerStopChannel chan bool
}

// PreJoin handles the prejoin request for joining a cluster. Will fail if
// already part of a cluster.
func (c *clusterRPCHandler) PreJoin(ctx context.Context, req *grpc.ClusterPreJoinReq) (*grpc.ClusterPreJoinResp, error) {
	if cluster, err := utils.GetCluster(); err != nil {
		return nil, err
	} else if cluster != nil {
		return nil, fmt.Errorf("Already part of cluster +%v", cluster)
	}

	log.Infof("Received PreJoin request: %+v", req)

	ntpErrs := utils.SyncTimeOnce(req.NtpServers)
	if ntpErrs != nil {
		var errMsgs []string
		for _, e := range ntpErrs {
			errMsgs = append(errMsgs, e.Error())
		}
		errStr := strings.Join(errMsgs, ", ")
		log.Errorf("Unable to perform clock sync: %v", errStr)
		tmpCluster := cluster.Cluster{
			ObjectMeta: api.ObjectMeta{
				Name: req.Name,
			},
		}
		recorder.Event(eventtypes.CLOCK_SYNC_FAILED, fmt.Sprintf("Node failed to synchronize clock, errors: %v", errStr), &tmpCluster)
		// continue anyway. Either this is ok or we will catch up later
	}

	var transportKeyBytes []byte
	if req.TransportKey != nil {
		if env.CertMgr == nil {
			cm, err := certmgr.NewGoCryptoCertificateMgr(certutils.GetCertificateMgrDir())
			if err != nil {
				werr := fmt.Errorf("Failed to instantiate certificate manager, error: %v", err)
				log.Errorf(werr.Error())
				return nil, werr
			}
			env.CertMgr = cm
		}
		// PreJoin messages go out to all quorum nodes during cluster formation, including the sender.
		// If CertMgr is already bootstrapped, it means that we are the sender and we don't need to do anything
		if !env.CertMgr.IsReady() {
			transportKey, err := env.CertMgr.GetKeyAgreementKey("self")
			if err != nil {
				werr := fmt.Errorf("Error generating key-agreement-key: %v", err)
				log.Errorf(werr.Error())
				return nil, werr
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
			werr := fmt.Errorf("Error unpacking CertMgr bundle: %v", err)
			log.Errorf(werr.Error())
			return nil, werr
		}
		err = env.CertMgr.StartCa(false)
		if err != nil || !env.CertMgr.IsReady() {
			werr := fmt.Errorf("Error starting CertMgr: %v", err)
			log.Errorf(werr.Error())
			return nil, werr
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
	env.ClusterName = req.Name
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
			werr := fmt.Errorf("%v received Join without itself in it", req.NodeId)
			log.Errorf(werr.Error())
			return nil, werr
		}

		qConfig := &quorum.Config{
			Type:       store.KVStoreTypeEtcd,
			ID:         req.QuorumConfig.Id,
			DataDir:    env.Options.KVStore.DataDir,
			CfgFile:    env.Options.KVStore.ConfigFile,
			MemberName: req.NodeId,
			Existing:   req.QuorumConfig.Existing,
			Members:    members,
		}

		err := credentials.SetQuorumInstanceAuth(qConfig, env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustRoots())
		if err != nil {
			werr := fmt.Errorf("Failed to obtain instance auth credentials for quorum with error: %v", err)
			log.Errorf(werr.Error())
			return nil, werr
		}

		err = credentials.GenQuorumClientAuth(env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustRoots())
		if err != nil {
			werr := fmt.Errorf("Failed to obtain client auth credentials for quorum with error: %v", err)
			log.Errorf(werr.Error())
			return nil, werr
		}

		quorumIntf, err := store.New(qConfig)
		if err != nil {
			werr := fmt.Errorf("Failed to create quorum with error: %v", err)
			log.Errorf(werr.Error())
			return nil, werr
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
			werr := fmt.Errorf("KV Store failed to come up in %v seconds", maxIters*sleepBetweenItersMsec/1000)
			log.Errorf(werr.Error())
			return nil, werr
		}

		kvStoreTLSConfig, err := etcd.GetEtcdClientCredentials()
		if err != nil {
			werr := fmt.Errorf("Failed to get kvstore client credentials with error: %v", err)
			log.Errorf(werr.Error())
			return nil, werr
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
			werr := fmt.Errorf("Failed to create kvstore, error: %v", err)
			log.Errorf(werr.Error())
			return nil, werr
		}

		err = credentials.GenKubernetesCredentials(req.NodeId, env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustRoots(), []string{req.VirtualIp})
		if err != nil {
			log.Errorf("Failed to generate Kubernetes credentials, error: %v", err)
			// try to proceed anyway
		} else {
			log.Infof("generated k8s credentials, dir: %v, err: %v", globals.KubernetesConfigDir, err)
		}

		err = credentials.GenVosAuth(req.NodeId, env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustChain(), env.CertMgr.Ca().TrustRoots())
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
		// Create leader service before its users
		env.LeaderService = services.NewLeaderService(kv, masterLeaderKey, req.NodeId)
		env.SystemdService = services.NewSystemdService()
		env.VipService = services.NewVIPService()

		env.NtpService = services.NewNtpService(req.NTPServers, env.QuorumNodes, req.NodeId)
		env.MasterService = services.NewMasterService(req.NodeId,
			services.WithK8sSvcMasterOption(env.K8sService),
			services.WithResolverSvcMasterOption(env.ResolverService),
			services.WithNtpSvcMasterOption(env.NtpService))

		env.SystemdService.Start() // must be called before dependent services
		env.VipService.AddVirtualIPs(req.VirtualIp)

		env.ServiceTracker = services.NewServiceTracker(env.ResolverService)
		env.LeaderService.Register(env.ServiceTracker)

	} else {
		// Generate quorum client credentials even if this is a non-quorum node, because some service
		// (ApiServer for example) may still need to contact quorum services or KVStore.
		err := credentials.GenQuorumClientAuth(env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustRoots())
		if err != nil {
			werr := fmt.Errorf("Failed to obtain client auth credentials for quorum with error: %v", err)
			log.Errorf(werr.Error())
			return nil, werr
		}

		err = credentials.GenVosAuth(req.NodeId, env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustChain(), env.CertMgr.Ca().TrustRoots())
		if err != nil {
			log.Errorf("Failed to generate VOS credentials, error: %v", err)
			// try to proceed anyway
		}

		// Generate Kubelet credentials only. Try to continue anyway in case of failure.
		err = credentials.GenKubeletCredentials(req.NodeId, env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustRoots())
		if err != nil {
			log.Errorf("Failed to generate Kubelet credentials, error: %v", err)
			err2 := credentials.RemoveKubeletCredentials()
			if err2 != nil {
				log.Errorf("Error removing Kubelet credentials: %v", err2)
			}
			// try to proceed anyway
		}
		env.SystemdService = services.NewSystemdService()
		env.SystemdService.Start() // must be called before dependent services
		env.NtpService = services.NewNtpService(nil, env.QuorumNodes, req.NodeId)
		env.NtpService.NtpConfigFile(env.QuorumNodes)
		env.ServiceTracker = services.NewServiceTracker(nil)
	}

	env.NodeService = services.NewNodeService(req.NodeId)
	env.RolloutMgr = services.NewRolloutMgr()
	env.RolloutMgr.Start()
	env.VeniceRolloutClient = rolloutclient.NewVeniceRolloutClient(globals.Rollout, req.NodeId, env.ResolverClient, env.RolloutMgr)
	env.VeniceRolloutClient.Start()

	env.MetricsService = services.NewMetricsService(req.NodeId, req.Name, env.ResolverClient)
	err := env.MetricsService.Start()
	if err != nil {
		log.Errorf("Failed to start metrics service with error: %v", err)
	}

	if req.QuorumConfig != nil {
		env.ServiceRolloutClient = rolloutclient.NewServiceRolloutClient(globals.Rollout, req.NodeId, env.ResolverClient, env.RolloutMgr)
		env.MasterService.Start()
		env.LeaderService.Start()
		env.NtpService.Start()
		env.CfgWatcherService.Start()
	}

	// Start node services. Currently we are running Node services on Quorum nodes also
	env.NodeService.InitConfigFiles()
	env.CfgWatcherService.SetNodeService(env.NodeService)
	env.CfgWatcherService.SetClusterQuorumNodes(env.QuorumNodes)
	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}
	env.ServiceTracker.Run(env.ResolverClient, env.NodeService)

	if shouldStartAuthServer {
		c.authServerStopChannel = make(chan bool)
		go auth.RunAuthServer(":"+env.Options.GRPCAuthPort, c.authServerStopChannel)
	}

	env.HealthClient = health.NewClient(env.ResolverClient)

	if env.ClusterBootstrapService != nil {
		env.ClusterBootstrapService.Stop()
	}

	return &grpc.ClusterJoinResp{}, nil
}

func (c *clusterRPCHandler) Disjoin(ctx context.Context, req *grpc.ClusterDisjoinReq) (*grpc.ClusterDisjoinResp, error) {
	env.Logger.Infof("Received disjoin request %+v", req)
	var err error
	if c.authServerStopChannel != nil {
		close(c.authServerStopChannel)
		c.authServerStopChannel = nil
	}
	if env.MasterService != nil {
		env.MasterService.Stop()
	}
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
	if env.K8sService != nil {
		env.K8sService.Stop()
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
	if env.MetricsService != nil {
		env.MetricsService.Stop()
		env.MetricsService = nil
	}
	if env.HealthClient != nil {
		env.HealthClient.Stop()
		env.HealthClient = nil
	}
	if env.ServiceRolloutClient != nil {
		env.ServiceRolloutClient.Stop()
	}
	if env.CfgWatcherService != nil {
		env.CfgWatcherService.Stop()
	}
	if env.NodeService != nil {
		env.NodeService.Stop()
	}
	if env.LeaderService != nil {
		env.LeaderService.Stop()
	}

	// Cleanup all credentials
	cerrs := credentials.RemoveQuorumAuth()
	if err != nil {
		env.Logger.Errorf("Error removing quorum credentials: %v", cerrs)
	}
	cerr := credentials.RemoveKubernetesCredentials()
	if err != nil {
		env.Logger.Errorf("Error removing kubernetes credentials: %v", cerr)
	}
	cerrs = credentials.RemoveElasticAuth()
	if err != nil {
		env.Logger.Errorf("Error removing elastic credentials: %v", cerrs)
	}
	cerr = credentials.RemoveVosAuth()
	if err != nil {
		env.Logger.Errorf("Error removing Vos credentials: %v", cerr)
	}

	// Cleanup etcd data dir
	os.RemoveAll(globals.EtcdMountedDataDir)

	env.ClusterName = ""
	env.QuorumNodes = []string{}
	cerr = utils.DeleteCluster()
	if err != nil {
		env.Logger.Errorf("Error deleting cluster: %v", cerr)
	}

	if env.ClusterBootstrapService != nil {
		env.ClusterBootstrapService.Start()
	}

	return &grpc.ClusterDisjoinResp{}, err
}
