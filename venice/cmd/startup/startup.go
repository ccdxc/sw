package startup

import (
	"context"
	"fmt"
	"net"
	"os"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/apiclient"
	"github.com/pensando/sw/venice/cmd/cache"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/grpc/server"
	"github.com/pensando/sw/venice/cmd/grpc/server/auth"
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
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	maxIters              = 50
	sleepBetweenItersMsec = 100
	masterLeaderKey       = "master"
	apiServerWaitTime     = time.Second
)

func waitForAPIAndStartServices() {
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

			hostname, err := os.Hostname()
			if err != nil {
				if ii%10 == 0 {
					log.Errorf("getting my Hostname fails with err %v for %v seconds", err, ii)
				}
				continue
			}

			found := false
			for _, qn := range cl[0].Spec.QuorumNodes {
				if qn == hostname {
					found = true
					break
				}
				if net.ParseIP(qn) != nil {
					found, _ = netutils.IsAConfiguredIP(qn)
					if found {
						break
					}
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

// StartQuorumServices starts services on quorum node
func StartQuorumServices(c utils.Cluster) {
	log.Debugf("Starting Quorum services on startup")
	hostname, err := os.Hostname()
	if err != nil {
		log.Errorf("Failed to determine hostname. error: %v", err)
		return
	}

	found := false
	for _, member := range c.QuorumNodes {
		if hostname == member {
			found = true
			break
		}
		if net.ParseIP(member) != nil {
			found, _ = netutils.IsAConfiguredIP(member)
			if found {
				hostname = member
				break
			}
		}
	}

	if !found {
		log.Infof("skipping starting Quorum services as this node (%s) is not part of quorum (%s)", hostname, c.QuorumNodes)
		return
	}

	qConfig := &quorum.Config{
		Type:       store.KVStoreTypeEtcd,
		ID:         c.UUID,
		DataDir:    env.Options.KVStore.DataDir,
		CfgFile:    env.Options.KVStore.ConfigFile,
		MemberName: hostname,
		Existing:   true,
	}
	qConfig.Members = append(qConfig.Members, quorum.Member{
		Name:       hostname,
		ClientURLs: []string{fmt.Sprintf("http://%s:%s", hostname, env.Options.KVStore.ClientPort)},
	})

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
		env.KVServers = append(env.KVServers, fmt.Sprintf("http://%s:%s", member, env.Options.KVStore.ClientPort))
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

	sConfig := kstore.Config{
		Type:    kstore.KVStoreTypeEtcd,
		Servers: env.KVServers,
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
		return
	}

	env.KVStore = kv
	// Create leader service before its users
	env.LeaderService = services.NewLeaderService(kv, masterLeaderKey, hostname)
	env.SystemdService = services.NewSystemdService()
	env.VipService = services.NewVIPService()
	env.K8sService = services.NewK8sService()
	env.ResolverService = services.NewResolverService(env.K8sService)
	env.StateMgr = cache.NewStatemgr()
	env.CfgWatcherService = apiclient.NewCfgWatcherService(env.Logger, globals.APIServer, env.StateMgr)
	env.MasterService = services.NewMasterService(c.VirtualIP, services.WithK8sSvcMasterOption(env.K8sService),
		services.WithResolverSvcMasterOption(env.ResolverService))

	env.SystemdService.Start() // must be called before dependent services
	env.VipService.AddVirtualIPs(c.VirtualIP)
	env.MasterService.Start()
	env.LeaderService.Start()
	env.CfgWatcherService.Start()

	go waitForAPIAndStartServices()

	env.NodeService = services.NewNodeService(c.NodeID, c.VirtualIP)
	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}

	// Create and register the RPC handler for SmartNIC service
	go server.RegisterSmartNICServer(env.StateMgr)
}

// StartNodeServices starts services running on non-quorum node
func StartNodeServices(nodeID, VirtualIP string) {
	log.Debugf("Starting node services on startup")
	env.NtpService = services.NewNtpService(nil)
	env.NtpService.NtpConfigFile([]string{VirtualIP})
	env.SystemdService = services.NewSystemdService()
	env.SystemdService.Start()

	env.NodeService = services.NewNodeService(nodeID, VirtualIP)
	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}
}

// OnStart restore state and start services as applicable
func OnStart() {
	// CertMgr runs both on quorum and controller nodes
	// It needs to be instantiated before cluster is formed, but it will be
	// ready to issue certificates only after the cluster is formed
	if env.CertMgr == nil {
		cm, err := certmgr.NewDefaultCertificateMgr()
		if err != nil {
			log.Errorf("Failed to instantiate certificate manager, error: %v", err)
			return
		}
		env.CertMgr = cm
	}

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
	if env.AuthRPCServer == nil {
		go auth.RunAuthServer(":"+env.Options.GRPCAuthPort, nil)
	}

	if cluster.QuorumNodes == nil {
		StartNodeServices(cluster.NodeID, cluster.VirtualIP)
		return
	}

	StartQuorumServices(*cluster)
}
