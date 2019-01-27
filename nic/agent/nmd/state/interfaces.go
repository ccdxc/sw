// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

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
	RegisterSmartNICReq(nic *cluster.SmartNIC) (grpc.RegisterNICResponse, error)

	// SmartNIC update api
	UpdateSmartNICReq(nic *cluster.SmartNIC) (*cluster.SmartNIC, error)

	// WatchSmartNICUpdates starts a CMD watchers to receive SmartNIC objects updates
	WatchSmartNICUpdates()

	// UpdateCMDClient updates the cmd client with the new resolver URLs obtained as a part of dhcp discovery
	UpdateCMDClient(resolvers []string) error
}

// NmdAPI is the API provided by NMD to CMD
type NmdAPI interface {
	RegisterCMD(cmd CmdAPI) error
	UnRegisterCMD() error
	GenClusterKeyPair() (*keymgr.KeyPair, error)
	GetAgentID() string
	GetSmartNIC() (*cluster.SmartNIC, error)
	SetSmartNIC(*cluster.SmartNIC) error
	CreateSmartNIC(nic *cluster.SmartNIC) error
	UpdateSmartNIC(nic *cluster.SmartNIC) error
	DeleteSmartNIC(nic *cluster.SmartNIC) error
	GetPlatformCertificate(nic *cluster.SmartNIC) ([]byte, error)
	GenChallengeResponse(nic *cluster.SmartNIC, challenge []byte) ([]byte, []byte, error)
}

// PlatformAPI is the API provided by PlatformAgent to NMD
type PlatformAPI interface {
	RegisterNMD(nmd NmdPlatformAPI) error
	CreateSmartNIC(nic *cluster.SmartNIC) error
	UpdateSmartNIC(nic *cluster.SmartNIC) error
	DeleteSmartNIC(nic *cluster.SmartNIC) error
	GetPlatformCertificate(nic *cluster.SmartNIC) ([]byte, error)
	GetPlatformSigner(nic *cluster.SmartNIC) (crypto.Signer, error)
}

// NmdPlatformAPI is the API provided by NMD to Platform agent
type NmdPlatformAPI interface {

	// SmartNIC register api
	RegisterSmartNICReq(nic *cluster.SmartNIC) (grpc.RegisterNICResponse, error)

	// SmartNIC update api
	UpdateSmartNICReq(nic *cluster.SmartNIC) (*cluster.SmartNIC, error)
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
	CreateUpdateSmartNICRollout(sro *protos.SmartNICRollout) error
	DeleteSmartNICRollout(sro *protos.SmartNICRollout) error
}

// UpgMgrAPI is used by NMD for calling upgmgr in naples
type UpgMgrAPI interface {
	RegisterNMD(NmdRolloutAPI) error

	StartDisruptiveUpgrade() error
	StartUpgOnNextHostReboot() error
	StartPreCheckDisruptive(version string) error
	StartPreCheckForUpgOnNextHostReboot(version string) error
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
	// WatchSmartNICRolloutUpdates establishes a watch on updates of SmartNICRollout objects
	WatchSmartNICRolloutUpdates() error
	// UpdateSmartNICRolloutStatus
	UpdateSmartNICRolloutStatus(status *protos.SmartNICRolloutStatusUpdate) error
	// Stop the watcher, client and free up any resources
	Stop()
}
