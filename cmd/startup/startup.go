package startup

import (
	"fmt"
	"net"
	"os"
	"time"

	kstore "github.com/pensando/sw/utils/kvstore/store"
	k8sclient "k8s.io/client-go/kubernetes"
	k8srest "k8s.io/client-go/rest"

	"github.com/pensando/sw/cmd/apiclient"
	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/cmd/services"
	"github.com/pensando/sw/cmd/utils"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/quorum"
	"github.com/pensando/sw/utils/quorum/store"
	"github.com/pensando/sw/utils/runtime"
)

const (
	maxIters              = 50
	sleepBetweenItersMsec = 100
	masterLeaderKey       = "master"
)

// StartQuorumServices starts services on quorum node
func StartQuorumServices(c utils.Cluster) {
	log.Debugf("Starting Quorum services on startup")
	hostname, _ := os.Hostname()
	addrs, err := net.LookupHost(hostname)
	if err != nil {
		log.Errorf("Failed to lookup host %v, error: %v", hostname, err)
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
		ClientURLs: []string{fmt.Sprintf("http://%s:%s", addrs[0], env.Options.KVStore.ClientPort)},
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
	config := &k8srest.Config{
		Host: fmt.Sprintf("%v:%v", c.VirtualIP, globals.KubeAPIServerPort),
	}
	env.K8sService = services.NewK8sService(k8sclient.NewForConfigOrDie(config))
	env.ResolverService = services.NewResolverService(env.K8sService)
	env.MasterService = services.NewMasterService(c.VirtualIP, services.WithK8sSvcMasterOption(env.K8sService),
		services.WithResolverSvcMasterOption(env.ResolverService))
	env.NtpService = services.NewNtpService(c.NTPServers)
	env.CfgWatcherService = apiclient.NewCfgWatcherService(env.Logger)

	env.SystemdService.Start() // must be called before dependent services
	env.VipService.AddVirtualIPs(c.VirtualIP)
	env.MasterService.Start()
	env.LeaderService.Start()
	env.CfgWatcherService.Start()

	// We let the quorum nodes use the external NTP server and non-quorum nodes use the VIP for ntp server
	env.NtpService.NtpConfigFile(c.NTPServers)

	env.NodeService = services.NewNodeService(c.VirtualIP)
	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}
}

// StartNodeServices starts services running on non-quorum node
func StartNodeServices(VirtualIP string) {
	log.Debugf("Starting node services on startup")
	env.NtpService = services.NewNtpService(nil)
	env.NtpService.NtpConfigFile([]string{VirtualIP})
	env.SystemdService = services.NewSystemdService()
	env.SystemdService.Start()

	env.NodeService = services.NewNodeService(VirtualIP)
	if err := env.NodeService.Start(); err != nil {
		log.Errorf("Failed to start node services with error: %v", err)
	}
}

// OnStart restore state and start services as applicable
func OnStart() {
	var cluster *utils.Cluster
	var err error
	if cluster, err = utils.GetCluster(); err != nil || cluster == nil {
		log.Debugf("OnStart cluster:%v err:%v possibly not part of cluster yet", cluster, err)
		return
	}

	if cluster.QuorumNodes == nil {
		StartNodeServices(cluster.VirtualIP)
		return
	}

	StartQuorumServices(*cluster)
}
