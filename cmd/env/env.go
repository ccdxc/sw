package env

import (
	"sync"

	"github.com/pensando/sw/cmd/server/options"
	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/quorum"
	"github.com/pensando/sw/utils/rpckit"
	"github.com/pensando/sw/utils/runtime"
)

// CMD global state is held here.
var (
	Logger            log.Logger
	Options           *options.ServerRunOptions
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
)
