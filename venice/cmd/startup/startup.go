package startup

import (
	"context"
	"fmt"
	"net"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/apiclient"
	"github.com/pensando/sw/venice/cmd/cache"
	"github.com/pensando/sw/venice/cmd/credentials"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc/server/auth"
	certutils "github.com/pensando/sw/venice/cmd/grpc/server/certificates/utils"
	"github.com/pensando/sw/venice/cmd/grpc/server/health"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/cmd/rolloutclient"
	"github.com/pensando/sw/venice/cmd/services"
	"github.com/pensando/sw/venice/cmd/utils"
	rolloututils "github.com/pensando/sw/venice/ctrler/rollout/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/etcd"
	kstore "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/quorum"
	etcdquorum "github.com/pensando/sw/venice/utils/quorum/etcd"
	"github.com/pensando/sw/venice/utils/quorum/store"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	maxIters                     = 50
	sleepBetweenItersMsec        = 100
	maxAllowedEtcdStartupTimeSec = 120
	masterLeaderKey              = "master"
	apiServerWaitTime            = time.Second
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
				log.Infof("starting NTP service with %v servers", cl[0].Spec.NTPServers)
				env.NtpService = services.NewNtpService(cl[0].Spec.NTPServers, cl[0].Spec.QuorumNodes, nodeID)
				env.MasterService.UpdateNtpService(env.NtpService)
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

	// Without etcd, CMD cannot do pretty much anything, so perform inifinite retries
	var quorumIntf quorum.Interface
	lastEtcdStartAttempt := time.Now()
	for i := 0; ; i++ {
		quorumIntf, err = store.Start(qConfig)
		if err == nil {
			break
		}
		log.Errorf("Failed to create quorum with error: %v", err)
		time.Sleep(time.Second)

		// Etcd startup may fail in such a way that the client times out or gets back
		// an error immediately. Before attempting a restart we wait for maxIters tries
		// (performed 1s apart) or maxAllowedEtcdStartupTime, whichever happens first.
		if (i+1)%maxIters == 0 ||
			time.Since(lastEtcdStartAttempt) > (maxAllowedEtcdStartupTimeSec*time.Second) {
			// We might be hitting https://github.com/etcd-io/etcd/issues/10655, invoke workaround
			etcdquorum.RestartLocalEtcdInstance(qConfig)
		}
	}

	env.ClusterName = c.Name
	env.QuorumNodes = make([]string, len(c.QuorumNodes))
	copy(env.QuorumNodes, c.QuorumNodes)
	env.Quorum = quorumIntf

	for _, member := range c.QuorumNodes {
		env.KVServers = append(env.KVServers, fmt.Sprintf("https://%s:%s", member, env.Options.KVStore.ClientPort))
	}

	ii := 0
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

	// create resolver before creating the services
	servers := make([]string, 0)
	for _, jj := range env.QuorumNodes {
		servers = append(servers, fmt.Sprintf("%s:%s", jj, globals.CMDResolverPort))
	}
	env.ResolverClient = resolver.New(&resolver.Config{Name: c.NodeID, Servers: servers})

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
	env.MasterService = services.NewMasterService(
		c.NodeID,
		services.WithK8sSvcMasterOption(env.K8sService),
		services.WithResolverSvcMasterOption(env.ResolverService),
		services.WithNtpSvcMasterOption(env.NtpService))
	env.ServiceTracker = services.NewServiceTracker(env.ResolverService)
	env.LeaderService.Register(env.ServiceTracker) // call before starting leader service

	env.SystemdService.Start() // must be called before dependent services
	env.VipService.AddVirtualIPs(c.VirtualIP)

	env.RolloutMgr = services.NewRolloutMgr()
	env.RolloutMgr.Start()
	env.VeniceRolloutClient = rolloutclient.NewVeniceRolloutClient(globals.Rollout, c.NodeID, env.ResolverClient, env.RolloutMgr)
	env.VeniceRolloutClient.Start()
	env.ServiceRolloutClient = rolloutclient.NewServiceRolloutClient(globals.Rollout, c.NodeID, env.ResolverClient, env.RolloutMgr)

	env.MasterService.Start()
	env.LeaderService.Start()
	env.CfgWatcherService.Start()

	go waitForAPIAndStartServices(c.NodeID)

	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}
	env.ServiceTracker.Run(env.ResolverClient, env.NodeService)

	env.MetricsService = services.NewMetricsService(c.NodeID, c.Name, env.ResolverClient)
	err = env.MetricsService.Start()
	if err != nil {
		log.Errorf("Failed to start metrics service with error: %v", err)
	}
	if env.AuthRPCServer == nil {
		go auth.RunAuthServer(":"+env.Options.GRPCAuthPort, nil)
	}
	if env.HealthClient == nil {
		env.HealthClient = health.NewClient(env.ResolverClient)
	}
}

// StartNodeServices starts services running on non-quorum node
func StartNodeServices(nodeID, clusterID, VirtualIP string) {
	log.Debugf("Starting node services on startup")
	env.SystemdService = services.NewSystemdService()
	env.SystemdService.Start()
	env.NtpService = services.NewNtpService(nil, env.QuorumNodes, nodeID)
	env.NtpService.NtpConfigFile(env.QuorumNodes)

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

	env.MetricsService = services.NewMetricsService(nodeID, clusterID, env.ResolverClient)
	err := env.MetricsService.Start()
	if err != nil {
		log.Errorf("Failed to start metrics service with error: %v", err)
	}

	if env.HealthClient == nil {
		env.HealthClient = health.NewClient(env.ResolverClient)
	}

	if env.AuthRPCServer == nil {
		go auth.RunAuthServer(":"+env.Options.GRPCAuthPort, nil)
	}
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
	env.CfgWatcherService = apiclient.NewCfgWatcherService(env.Logger, globals.APIServer)
	env.StateMgr = cache.NewStatemgr(env.CfgWatcherService)
	env.ClusterBootstrapService = services.NewClusterBootstrapService(":" + env.Options.RESTPort)

	// Instantiate handler for NIC registration and updates.
	// Actual server instantiation, registration and start is done at a later time,
	// after we know if we are part of a cluster and if we are leader.
	env.NICService = smartnic.NewRPCServer(
		smartnic.HealthWatchInterval,
		smartnic.DeadInterval,
		globals.NmdRESTPort,
		env.StateMgr,
		rolloututils.VersionChecker{})

	var cluster *utils.Cluster
	var err error
	if cluster, err = utils.GetCluster(); err != nil || cluster == nil {
		log.Debugf("OnStart cluster:%v err:%v possibly not part of cluster yet", cluster, err)
		err = env.ClusterBootstrapService.Start()
		if err != nil {
			log.Fatalf("Node is not part of a cluster. Error starting cluster bootstrap endpoint on port %v: %v", env.Options.RESTPort, err)
		}
		return
	}

	// if we are already part of a cluster we can start the CA
	err = env.CertMgr.StartCa(false)
	if err != nil {
		log.Errorf("Node is part of cluster %+v but failed to start CA with err: %v", cluster, err)
		return
	}
	// Now that CA has started, Recorder clients can talk RPC to eventsProxy
	env.Recorder.StartExport()

	env.ClusterName = cluster.Name
	env.QuorumNodes = cluster.QuorumNodes

	// start node service
	env.NodeService = services.NewNodeService(cluster.NodeID)
	env.NodeService.InitConfigFiles()

	// update config watcher with node service and quorum nodes
	env.CfgWatcherService.SetNodeService(env.NodeService)
	env.CfgWatcherService.SetClusterQuorumNodes(env.QuorumNodes)

	// start SmartNIC monitoring routine
	go func() {
		env.NICService.MonitorHealth()
	}()

	quorumMember, _ := isQuorumMember(cluster)
	if !quorumMember {
		StartNodeServices(cluster.NodeID, cluster.Name, cluster.VirtualIP)
		return
	}

	StartQuorumServices(*cluster)
}
