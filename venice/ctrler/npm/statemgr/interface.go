// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"github.com/pensando/sw/api/generated/cluster"
)

//EventType type of event
type EventType string

// event types
const (
	CreateEvent EventType = "Create"
	UpdateEvent EventType = "Update"
	DeleteEvent EventType = "Delete"
)

// FeatureStateMgr interface is implemented by feature specific statemgrs to register feature specific handlers to watch
type FeatureStateMgr interface {
	CompleteRegistration()
	ProcessDSCEvent(ev EventType, dsc *cluster.DistributedServiceCard)
}

// Server interface is implemented by statemgr used by feature specific statemgrs to register their callbacks during init()
type Server interface {
	// Register is used by feature specific statemgrs to register during init
	Register(name string, svc FeatureStateMgr)
}
