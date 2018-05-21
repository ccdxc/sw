// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package server

import (
	"crypto"
	"fmt"
	"net"
	"os"
	"strings"
	"time"

	context "golang.org/x/net/context"

	"github.com/pensando/sw/venice/cmd/cache"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/auth"
	certutils "github.com/pensando/sw/venice/cmd/grpc/server/certificates/utils"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/cmd/services"
	"github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/kvstore"
	kstore "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
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

	services.ContainerInfoMap = utils.GetContainerInfo()

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
		if env.AuthRPCServer == nil {
			//  start the RPC server after creating the ResolverService (if applicable)
			//	Hence move the start-Auth-server code towards end of the function
			shouldStartAuthServer = true
		}
	}
	env.QuorumNodes = req.QuorumNodes

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
			if !found && net.ParseIP(member.Name) != nil {
				found, _ = netutils.IsAConfiguredIP(member.Name)
				if found {
					hostname = member.Name
				}
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

		env.K8sService = services.NewK8sService()
		env.ResolverService = services.NewResolverService(env.K8sService)
		env.KVStore = kv
		env.StateMgr = cache.NewStatemgr()
		// Create leader service before its users
		env.LeaderService = services.NewLeaderService(kv, masterLeaderKey, hostname)
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

		env.MasterService.Start()
		env.LeaderService.Start()
		env.CfgWatcherService.Start()
	} else {
		env.NtpService = services.NewNtpService(req.NTPServers)
		env.NtpService.NtpConfigFile([]string{req.VirtualIp})
		env.SystemdService = services.NewSystemdService()
		env.SystemdService.Start() // must be called before dependent services

		env.ServiceTracker = services.NewServiceTracker(nil)
	}

	servers := make([]string, 0)
	for _, jj := range env.QuorumNodes {
		servers = append(servers, fmt.Sprintf("%s:%s", jj, globals.CMDResolverPort))
	}
	env.ResolverClient = resolver.New(&resolver.Config{Name: req.NodeId, Servers: servers})
	env.NodeService = services.NewNodeService(req.NodeId)
	// Start node services. Currently we are running Node services on Quorum nodes also
	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}
	env.ServiceTracker.Run(env.ResolverClient, env.NodeService)

	if shouldStartAuthServer {
		go auth.RunAuthServer(":"+env.Options.GRPCAuthPort, nil)
	}

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
		err = env.SystemdService.StopUnit("pen-kubelet.service")
		if err != nil {
			env.Logger.Errorf("Error %v while stopping pen-kubelet", err)
		}
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
		go func() {
			env.NICService.MonitorHealth()
		}()

		// Register smartNIC gRPC server
		log.Debugf("Registering SmartNIC RPCserver")
		grpc.RegisterSmartNICRegistrationServer(env.UnauthRPCServer.GrpcServer, nicServer)
		return
	}
	log.Fatalf("Failed to register SmartNIC RPCserver, apiClient not up")
}
