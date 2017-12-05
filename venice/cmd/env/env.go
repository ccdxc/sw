package env

import (
	"sync"

	"github.com/pensando/sw/venice/cmd/server/options"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/quorum"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

// CMD global state is held here.
var (
	Logger            log.Logger
	Options           *options.ServerRunOptions
	RegistryURL       string
	Scheme            *runtime.Scheme
	KVStore           kvstore.Interface
	QuorumNodes       []string
	Quorum            quorum.Interface
	KVServers         []string
	LeaderService     types.LeaderService // common leader service used by MasterService, NTP Service, VIPService etc
	Mutex             sync.Mutex
	MasterService     types.MasterService
	NodeService       types.NodeService
	VipService        types.VIPService
	SystemdService    types.SystemdService
	NtpService        types.NtpService
	K8sService        types.K8sService
	ResolverService   types.ResolverService
	RPCServer         *rpckit.RPCServer
	CfgWatcherService types.CfgWatcherService
	CertMgr           *certmgr.CertificateMgr
)
