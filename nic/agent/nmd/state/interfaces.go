// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"github.com/pensando/sw/api/generated/cluster"
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
	RegisterSmartNICReq(nic *cluster.SmartNIC) (grpc.RegisterNICResponse, error)

	// SmartNIC update api
	UpdateSmartNICReq(nic *cluster.SmartNIC) (*cluster.SmartNIC, error)
}

// NmdAPI is the API provided by NMD to CMD
type NmdAPI interface {
	RegisterCMD(cmd CmdAPI) error
	GetAgentID() string
	GetSmartNIC() (*cluster.SmartNIC, error)
	SetSmartNIC(*cluster.SmartNIC) error
	CreateSmartNIC(nic *cluster.SmartNIC) error
	UpdateSmartNIC(nic *cluster.SmartNIC) error
	DeleteSmartNIC(nic *cluster.SmartNIC) error
}

// PlatformAPI is the API provided by PlatformAgent to NMD
type PlatformAPI interface {
	RegisterNMD(nmd NmdPlatformAPI) error
	CreateSmartNIC(nic *cluster.SmartNIC) error
	UpdateSmartNIC(nic *cluster.SmartNIC) error
	DeleteSmartNIC(nic *cluster.SmartNIC) error
}

// NmdPlatformAPI is the API provided by NMD to Platform agent
type NmdPlatformAPI interface {

	// SmartNIC register api
	RegisterSmartNICReq(nic *cluster.SmartNIC) (grpc.RegisterNICResponse, error)

	// SmartNIC update api
	UpdateSmartNICReq(nic *cluster.SmartNIC) (*cluster.SmartNIC, error)
}
