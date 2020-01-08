// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

// FeatureStateMgr interface is implemented by feature specific statemgrs to register feature specific handlers to watch
type FeatureStateMgr interface {
	CompleteRegistration()
}

// Server interface is implemented by statemgr used by feature specific statemgrs to register their callbacks during init()
type Server interface {
	// Register is used by feature specific statemgrs to register during init
	Register(name string, svc FeatureStateMgr)
}
