// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package api

import (
	"crypto"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/keymgr"
)

//
//	Platform  ----------------> NMD ---------> CMD
//             NmdPlatformAPI         CmdAPI
//
//	Platform  <---------------- NMD <--------- CMD
//               PlatformAPI           NmdAPI
//

// CmdAPI is the API provided by CMD to NMD
type CmdAPI interface {

	// SmartNIC register api
	RegisterSmartNICReq(nic *cluster.DistributedServiceCard) (grpc.RegisterNICResponse, error)

	// SmartNIC update api
	UpdateSmartNICReq(nic *cluster.DistributedServiceCard) error

	// WatchSmartNICUpdates starts a CMD watchers to receive SmartNIC objects updates
	WatchSmartNICUpdates()

	// Stop stops the CMD watchers and releases all resources
	Stop()

	// IsSmartNICWatcherRunning returns true if there is active SmartNIC watch on CMD
	IsSmartNICWatcherRunning() bool
}

// NmdAPI is the API provided by NMD to CMD
type NmdAPI interface {
	RegisterCMD(cmd CmdAPI) error
	UnRegisterCMD() error
	UpdateCMDClient(resolvers []string) error
	GenClusterKeyPair() (*keymgr.KeyPair, error)
	GetAgentID() string
	GetPrimaryMAC() string
	GetControllerIps() []string
	GetSmartNIC() (*cluster.DistributedServiceCard, error)
	SetSmartNIC(*cluster.DistributedServiceCard) error
	CreateSmartNIC(nic *cluster.DistributedServiceCard) error
	UpdateSmartNIC(nic *cluster.DistributedServiceCard) error
	DeleteSmartNIC(nic *cluster.DistributedServiceCard) error
	GetPlatformCertificate(nic *cluster.DistributedServiceCard) ([]byte, error)
	GenChallengeResponse(nic *cluster.DistributedServiceCard, challenge []byte) ([]byte, []byte, error)
	GetIPConfig() *cluster.IPConfig
	SetIPConfig(cfg *cluster.IPConfig)
	SetVeniceIPs(veniceIPs []string)
	GetVeniceIPs() []string
	SetInterfaceIPs(interfaceIPs map[uint32]*cluster.IPConfig)
	PersistState(updateDelphi bool) error
	SetMgmtInterface(intf string)
	SetNetworkMode(intf string)
	SetDSCID(name string)
	GetParsedControllers() []string
}

// PlatformAPI is the API provided by PlatformAgent to NMD
type PlatformAPI interface {
	RegisterNMD(nmd NmdPlatformAPI) error
	CreateSmartNIC(nic *cluster.DistributedServiceCard) error
	UpdateSmartNIC(nic *cluster.DistributedServiceCard) error
	DeleteSmartNIC(nic *cluster.DistributedServiceCard) error
	GetPlatformCertificate(nic *cluster.DistributedServiceCard) ([]byte, error)
	GetPlatformSigner(nic *cluster.DistributedServiceCard) (crypto.Signer, error)
}

// NmdPlatformAPI is the API provided by NMD to Platform agent
type NmdPlatformAPI interface {

	// SmartNIC register api
	RegisterSmartNICReq(nic *cluster.DistributedServiceCard) (grpc.RegisterNICResponse, error)

	// SmartNIC update api
	UpdateSmartNICReq(nic *cluster.DistributedServiceCard) error
}

// ==================================================================================
//	UpgMgr    <---------------- NMD <--------- Rollout
//              UpgmgrAPI      RolloutAPI
//
//	UpgMgr    ----------------> NMD ---------> Rollout
//              NmdRolloutAPI      RolloutCtrlAPI
//

// RolloutAPI is the API provided by NMD to Rollout
type RolloutAPI interface {
	GetPrimaryMAC() string
	RegisterROCtrlClient(RolloutCtrlAPI) error
	CreateUpdateDSCRollout(sro *protos.DSCRollout) error
	DeleteDSCRollout() error
}

// UpgMgrAPI is used by NMD for calling upgmgr in naples
type UpgMgrAPI interface {
	RegisterNMD(NmdRolloutAPI) error

	StartDisruptiveUpgrade(firmwarePkgName string) error
	StartUpgOnNextHostReboot(firmwarePkgName string) error
	StartPreCheckDisruptive(version string) error
	StartPreCheckForUpgOnNextHostReboot(version string) error
	IsUpgClientRegistered() error
	IsUpgradeInProgress() bool
}

// NmdRolloutAPI is the API called by upgmgr agent to nmd
type NmdRolloutAPI interface {
	UpgSuccessful()
	UpgFailed(errStrList *[]string)
	UpgPossible()
	UpgNotPossible(errStrList *[]string)
	UpgAborted(errStrList *[]string)
}

// RolloutCtrlAPI is the API provided by Rollout Ctrler to NMD
type RolloutCtrlAPI interface {
	// WatchDSCRolloutUpdates establishes a watch on updates of DSCRollout objects
	WatchDSCRolloutUpdates() error
	// UpdateDSCRolloutStatus
	UpdateDSCRolloutStatus(status *protos.DSCRolloutStatusUpdate) error
	// Stop the watcher, client and free up any resources
	Stop()
	// IsSmartNICWatcherRunning returns true if there is active SmartNIC watch on CMD
	IsSmartNICWatcherRunning() bool
}
