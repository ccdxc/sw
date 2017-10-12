// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/venice/cmd/grpc"
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
	RegisterSmartNICReq(nic *cmd.SmartNIC) (grpc.RegisterNICResponse, error)

	// SmartNIC update api
	UpdateSmartNICReq(nic *cmd.SmartNIC) (*cmd.SmartNIC, error)
}

// NmdAPI is the API provided by NMD to CMD
type NmdAPI interface {
	RegisterCMD(cmd CmdAPI) error
	GetAgentID() string
	GetSmartNIC() (*cmd.SmartNIC, error)
	SetSmartNIC(*cmd.SmartNIC) error
	CreateSmartNIC(nic *cmd.SmartNIC) error
	UpdateSmartNIC(nic *cmd.SmartNIC) error
	DeleteSmartNIC(nic *cmd.SmartNIC) error
}

// PlatformAPI is the API provided by PlatformAgent to NMD
type PlatformAPI interface {
	RegisterNMD(nmd NmdPlatformAPI) error
	CreateSmartNIC(nic *cmd.SmartNIC) error
	UpdateSmartNIC(nic *cmd.SmartNIC) error
	DeleteSmartNIC(nic *cmd.SmartNIC) error
}

// NmdPlatformAPI is the API provided by NMD to Platform agent
type NmdPlatformAPI interface {

	// SmartNIC register api
	RegisterSmartNICReq(nic *cmd.SmartNIC) (grpc.RegisterNICResponse, error)

	// SmartNIC update api
	UpdateSmartNICReq(nic *cmd.SmartNIC) (*cmd.SmartNIC, error)
}
