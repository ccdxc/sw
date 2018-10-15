package startup

import (
	"context"
	"fmt"
	"net"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/apiclient"
	"github.com/pensando/sw/venice/cmd/cache"
	"github.com/pensando/sw/venice/cmd/credentials"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc/server/auth"
	certutils "github.com/pensando/sw/venice/cmd/grpc/server/certificates/utils"
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
)

const (
	maxIters              = 50
	sleepBetweenItersMsec = 100
	masterLeaderKey       = "master"
	apiServerWaitTime     = time.Second
)

func waitForAPIAndStartServices(nodeID string) {
	ii := 0
	for {
		select {
		case <-time.After(apiServerWaitTime):
			ii++
			// TODO: Needs more discussion. Would APIClient talk to this node if this node was down
			//  for a very long time? Would the certificate have rotated by then?
			apic := env.CfgWatcherService.APIClient()
			if apic == nil {
				if ii%10 == 0 {
					log.Errorf("Waiting for Pensando apiserver to come up for %v seconds", ii)
				}

				continue
			}

			opts := api.ListWatchOptions{}
			cl, err := apic.Cluster().List(context.TODO(), &opts)
			if err != nil || len(cl) == 0 {
				if ii%10 == 0 {
					log.Errorf("Waiting for Pensando apiserver to give return good Cluster for %v seconds", ii)
				}
				continue
			}

			found := false
			for _, qn := range cl[0].Spec.QuorumNodes {
				if qn == nodeID {
					found = true
					break
				}
			}
			if found == false {
				log.Debugf("Not starting NTP service since this node is no longer part of Quorum Nodes")
			} else {
				log.Debugf("starting NTP service with %v servers", cl[0].Spec.NTPServers)
				env.NtpService = services.NewNtpService(cl[0].Spec.NTPServers)
				env.NtpService.NtpConfigFile(cl[0].Spec.NTPServers)
			}
			return
		}
	}
}

func isQuorumMember(c *utils.Cluster) (bool, string) {
	for _, member := range c.QuorumNodes {
		if c.NodeID == member {
			return true, c.NodeID
		}
	}
	log.Infof("skipping starting Quorum services as this node (%s) is not part of quorum (%s)", c.NodeID, c.QuorumNodes)
	return false, ""
}

// StartQuorumServices starts services on quorum node
func StartQuorumServices(c utils.Cluster) {
	log.Debugf("Starting Quorum services on startup")

	qConfig := &quorum.Config{
		Type:              store.KVStoreTypeEtcd,
		ID:                c.UUID,
		DataDir:           env.Options.KVStore.DataDir,
		CfgFile:           env.Options.KVStore.ConfigFile,
		MemberName:        c.NodeID,
		Existing:          true,
		PeerAuthEnabled:   true,
		ClientAuthEnabled: true,
	}
	qConfig.Members = append(qConfig.Members, quorum.Member{
		Name:       c.NodeID,
		ClientURLs: []string{fmt.Sprintf("https://%s:%s", c.NodeID, env.Options.KVStore.ClientPort)},
	})

	// etcd can only bind to IP:pair ports. If we have a host name, we need to resolve
	addrs, err := net.LookupHost(c.NodeID)
	if err != nil {
		log.Errorf("Failed to retrieve node: %v, error: %v", addrs[0], err)
		return
	}

	var quorumIntf quorum.Interface
	ii := 0
	for ; ii < maxIters; ii++ {
		quorumIntf, err = store.Start(qConfig)
		if err == nil {
			break
		}
		time.Sleep(time.Second)
	}

	if err != nil {
		log.Errorf("Failed to create quorum with error: %v", err)
		return
	}

	env.QuorumNodes = make([]string, len(c.QuorumNodes))
	copy(env.QuorumNodes, c.QuorumNodes)
	env.Quorum = quorumIntf

	for _, member := range c.QuorumNodes {
		env.KVServers = append(env.KVServers, fmt.Sprintf("https://%s:%s", member, env.Options.KVStore.ClientPort))
	}

	ii = 0
	for ; ii < maxIters; ii++ {
		_, errL := env.Quorum.List()
		if errL == nil {
			break
		}
		time.Sleep(sleepBetweenItersMsec * time.Millisecond)
	}

	if ii == maxIters {
		log.Errorf("KV Store failed to come up in %v seconds", maxIters*sleepBetweenItersMsec/1000)
		return
	}

	kvStoreTLSConfig, err := etcd.GetEtcdClientCredentials()
	if err != nil {
		log.Errorf("Failed to retrieve etcd client credentials, error: %v", err)
	}

	sConfig := kstore.Config{
		Type:        kstore.KVStoreTypeEtcd,
		Servers:     env.KVServers,
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
		return
	}

	env.KVStore = kv

	err = credentials.CheckKubernetesCredentials()
	if err != nil {
		log.Infof("Invalid Kubernetes credentials (%s), generating a new set", err)
		err = credentials.GenKubernetesCredentials(c.NodeID, env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustRoots(), []string{c.VirtualIP})
		if err != nil {
			log.Errorf("Failed to generate Kubernetes credentials, error: %v", err)
			// try to proceed anyway
		}
	}

	// Create leader service before its users
	env.LeaderService = services.NewLeaderService(kv, masterLeaderKey, c.NodeID)
	env.SystemdService = services.NewSystemdService()
	env.VipService = services.NewVIPService()
	k8sConfig := services.K8sServiceConfig{
		OverriddenModules: utils.GetOverriddenModules(""),
		DisabledModules:   utils.GetDisabledModules(""),
	}
	env.K8sService = services.NewK8sService(&k8sConfig)
	env.ResolverService = services.NewResolverService(env.K8sService)
	env.MasterService = services.NewMasterService(services.WithK8sSvcMasterOption(env.K8sService),
		services.WithResolverSvcMasterOption(env.ResolverService))
	env.ServiceTracker = services.NewServiceTracker(env.ResolverService)
	env.LeaderService.Register(env.ServiceTracker) // call before starting leader service

	servers := make([]string, 0)
	for _, jj := range env.QuorumNodes {
		servers = append(servers, fmt.Sprintf("%s:%s", jj, globals.CMDResolverPort))
	}
	env.ResolverClient = resolver.New(&resolver.Config{Name: c.NodeID, Servers: servers})

	env.SystemdService.Start() // must be called before dependent services
	env.VipService.AddVirtualIPs(c.VirtualIP)
	env.MasterService.Start()
	env.LeaderService.Start()
	env.CfgWatcherService.Start()

	go waitForAPIAndStartServices(c.NodeID)

	env.NodeService = services.NewNodeService(c.NodeID)
	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}

	env.RolloutMgr = services.NewRolloutMgr()
	env.RolloutMgr.Start()
	env.VeniceRolloutClient = rolloutclient.NewVeniceRolloutClient(globals.Rollout, c.NodeID, env.ResolverClient, env.RolloutMgr)
	env.VeniceRolloutClient.Start()
	env.ServiceRolloutClient = rolloutclient.NewServiceRolloutClient(globals.Rollout, c.NodeID, env.ResolverClient, env.RolloutMgr)

	env.ServiceTracker.Run(env.ResolverClient, env.NodeService)

	if env.AuthRPCServer == nil {
		go auth.RunAuthServer(":"+env.Options.GRPCAuthPort, nil)
	}
}

// StartNodeServices starts services running on non-quorum node
func StartNodeServices(nodeID, VirtualIP string) {
	log.Debugf("Starting node services on startup")
	env.NtpService = services.NewNtpService(nil)
	env.NtpService.NtpConfigFile([]string{VirtualIP})
	env.SystemdService = services.NewSystemdService()
	env.SystemdService.Start()

	env.NodeService = services.NewNodeService(nodeID)
	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}

	servers := make([]string, 0)
	for _, jj := range env.QuorumNodes {
		servers = append(servers, fmt.Sprintf("%s:%s", jj, globals.CMDResolverPort))
	}
	env.ResolverClient = resolver.New(&resolver.Config{Name: nodeID, Servers: servers})
	env.RolloutMgr = services.NewRolloutMgr()
	env.RolloutMgr.Start()
	env.VeniceRolloutClient = rolloutclient.NewVeniceRolloutClient(globals.Rollout, nodeID, env.ResolverClient, env.RolloutMgr)
	env.VeniceRolloutClient.Start()

	env.ServiceTracker = services.NewServiceTracker(nil)
	env.ServiceTracker.Run(env.ResolverClient, env.NodeService)

	if env.AuthRPCServer == nil {
		go auth.RunAuthServer(":"+env.Options.GRPCAuthPort, nil)
	}

	node := &cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: "Node",
		},
		ObjectMeta: api.ObjectMeta{
			Name: nodeID,
		},
	}
	nodewatcher.NewNodeWatcher(context.Background(), node, env.ResolverClient.(resolver.Interface), 30, env.Logger)
}

// OnStart restore state and start services as applicable
func OnStart() {
	// CertMgr runs both on quorum and controller nodes
	// It needs to be instantiated before cluster is formed, but it will be
	// ready to issue certificates only after the cluster is formed
	if env.CertMgr == nil {
		cm, err := certmgr.NewGoCryptoCertificateMgr(certutils.GetCertificateMgrDir())
		if err != nil {
			log.Errorf("Failed to instantiate certificate manager, error: %v", err)
			return
		}
		env.CertMgr = cm
	}

	// These must be instantiated regardless of whether node is part of a cluster or not,
	// as they are needed  by the gRPC server that listens for cluster events
	env.StateMgr = cache.NewStatemgr()
	env.CfgWatcherService = apiclient.NewCfgWatcherService(env.Logger, globals.APIServer, env.StateMgr)

	var cluster *utils.Cluster
	var err error
	if cluster, err = utils.GetCluster(); err != nil || cluster == nil {
		log.Debugf("OnStart cluster:%v err:%v possibly not part of cluster yet", cluster, err)
		return
	}

	// if we are already part of a cluster we can start the CA
	err = env.CertMgr.StartCa(false)
	if err != nil {
		log.Errorf("Node is part of cluster %+v but failed to start CA with err: %v", cluster, err)
		return
	}
	// Now that CA has started, Recorderclients can talk RPC to eventsProxy
	env.Recorder.StartExport()
	// Launch go routine to monitor health updates of smartNIC objects and update status
	go func() {
		env.NICService.MonitorHealth()
	}()

	env.QuorumNodes = cluster.QuorumNodes
	quorumMember, _ := isQuorumMember(cluster)
	if !quorumMember {
		StartNodeServices(cluster.NodeID, cluster.VirtualIP)
		return
	}

	StartQuorumServices(*cluster)
}
