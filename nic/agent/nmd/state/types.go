// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"context"
	"fmt"
	"net"
	"net/http"
	"sync"
	"time"

	"github.com/looplab/fsm"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	nmdapi "github.com/pensando/sw/nic/agent/nmd/api"
	"github.com/pensando/sw/nic/agent/nmd/state/ipif"
	"github.com/pensando/sw/nic/agent/protos/nmd"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/venice/utils/certsproxy"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/revproxy"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
)

const (
	nicRegistrationWaitTime = time.Duration(time.Minute * 5)
)

// NMD is the Naples management daemon instance object
type NMD struct {
	sync.Mutex                // Lock for NMD object
	sync.WaitGroup            // Wait group
	modeChange     sync.Mutex // Lock to serialize mode change requests

	store          emstore.Emstore       // Embedded DB
	nodeUUID       string                // Node's UUID
	macAddr        string                // Primary MAC addr
	cmdRegURL      string                // The URL for the CMD registration API
	cmd            nmdapi.CmdAPI         // CMD API object
	Platform       nmdapi.PlatformAPI    // Platform Agent object
	rollout        nmdapi.RolloutCtrlAPI // Rollout API Object
	Upgmgr         nmdapi.UpgMgrAPI      // Upgrade Manager API
	resolverClient resolver.Interface    // Resolver client instance

	config          nmd.DistributedServiceCard  // Naples config received via REST
	nic             *cmd.DistributedServiceCard // DistributedServiceCard object
	DSCStaticRoutes []agentTypes.DSCStaticRoute
	DSCInterfaceIPs []agentTypes.DSCInterfaceIP
	DelphiClient    clientAPI.Client
	IPClient        *ipif.IPClient

	stopNICReg         chan bool     // channel to stop NIC registration
	nicRegInitInterval time.Duration // the initial time interval between nic registration in seconds
	nicRegInterval     time.Duration // time interval between nic registration in seconds. Gets adjusted dynamically with exponential backoff
	isRegOngoing       bool          // status of ongoing nic registration task

	stopNICUpd     chan bool     // channel to stop NIC update
	nicUpdInterval time.Duration // time interval between nic updates in seconds
	isUpdOngoing   bool          // status of ongoing nic update task

	listenURL        string       // URL where http server is listening
	listener         net.Listener // socket listener
	httpServer       *http.Server // HTTP server
	isRestSrvRunning bool         // status of the REST server

	certsListenURL  string                            // URL where local processes can request certificates
	remoteCertsURLs []string                          // URLs where local process cert request are forwarder
	certsProxy      *certsproxy.CertsProxy            // the CertsProxy instance
	tlsProvider     *tlsproviders.KeyMgrBasedProvider // TLS provider holding cluster keys
	// Rollout related stuff
	ro           nmd.NaplesRollout
	objectMeta   api.ObjectMeta
	profiles     []*nmd.DSCProfile
	revProxy     *revproxy.ReverseProxyRouter
	stateMachine *NMDStateMachine
	tsdbCancel   context.CancelFunc
	rebootNeeded bool
	metrics      *NMDMetrics
	Pipeline     Pipeline

	// Naples Information
	RunningFirmware        *nmd.DSCRunningSoftware
	RunningFirmwareName    string
	RunningFirmwareVersion string
}

// Agent is the wrapper object that contains NMD and Platform components
type Agent struct {

	// NMD object
	Nmd *NMD

	// Platform object
	Platform nmdapi.PlatformAPI

	// Upgrademgr Interface
	Upgmgr nmdapi.UpgMgrAPI
}

// String returns string value of the datapath kind
func (k *Kind) String() string {
	return string(*k)
}

// Kind holds the pipeline kind. It could either be iris or apollo.
type Kind string

// Pipeline is the APIs provided by respective pipeline modules
type Pipeline interface {
	InitDelphi() interface{}
	MountDSCObjects() interface{}
	InitSysmgr()
	MountSysmgrObjects() interface{}
	RunDelphiClient(Agent) interface{}
	GetDelphiClient() clientAPI.Client
	SetNmd(interface{})
	WriteDelphiObjects() (err error)
	GetSysmgrSystemStatus() (string, string)
	GetPipelineType() string
}

// NaplesConfigResp is response to NaplesConfig request nmd.Naples
type NaplesConfigResp struct {
	ErrorMsg string
}

// NMDStateMachine wraps the nmd state machine
type NMDStateMachine struct {
	FSM *fsm.FSM
}

// ErrBadRequest is raised on bad request
type ErrBadRequest struct {
	Message string
}

// ErrInternalServer is raised on internal server errors
type ErrInternalServer struct {
	Message string
}

// NMDMetricsMeta wraps the NMD Metrics object
type NMDMetricsMeta struct {
	api.TypeMeta
	api.ObjectMeta
	metrics NMDMetrics
}

// NMDMetrics has various nmd metrics
type NMDMetrics struct {
	GetCalls api.Counter
}

func (e ErrBadRequest) Error() string {
	return fmt.Sprintf("request validation failed: %s", e.Message)
}

func (e ErrInternalServer) Error() string {
	return fmt.Sprintf("internal server error: %s", e.Message)
}
