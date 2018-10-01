package env

import (
	"sync"

	"github.com/pensando/sw/venice/cmd/cache"
	"github.com/pensando/sw/venice/cmd/server/options"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/quorum"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

// CMD global state is held here.
var (
	GitVersion           string // textual based on tags. e.g: v0.1.2
	GitCommit            string // git hash
	BuildDate            string // date/time on which the build was made
	Logger               log.Logger
	Options              *options.ServerRunOptions
	RegistryURL          string
	Scheme               *runtime.Scheme
	KVStore              kvstore.Interface
	QuorumNodes          []string
	Quorum               quorum.Interface
	KVServers            []string
	LeaderService        types.LeaderService // common leader service used by MasterService, NTP Service, VIPService etc
	Mutex                sync.Mutex
	StateMgr             *cache.Statemgr
	MasterService        types.MasterService
	NodeService          types.NodeService
	VipService           types.VIPService
	SystemdService       types.SystemdService
	NtpService           types.NtpService
	K8sService           types.K8sService
	ResolverService      types.ResolverService
	NICService           types.SmartNICService
	CfgWatcherService    types.CfgWatcherService
	CertMgr              *certmgr.CertificateMgr
	AuthRPCServer        *rpckit.RPCServer // Authenticated services (requires TLS and client certificate)
	UnauthRPCServer      *rpckit.RPCServer // Unauthenticated services
	ServiceTracker       types.ServiceTracker
	ResolverClient       resolver.Interface
	RolloutMgr           types.RolloutMgr
	VeniceRolloutClient  types.VeniceRolloutClient
	ServiceRolloutClient types.ServiceRolloutClient
)
