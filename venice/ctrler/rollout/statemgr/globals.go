// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package statemgr

import "errors"

var (
	// ErrIncorrectObjectType is returned when type deferencing from memdb.Object is of invalid type
	ErrIncorrectObjectType = errors.New("incorrect object type")
)

// These object kids are in the statemgr
const (
	kindVeniceRollout   = "VeniceRollout"
	kindServiceRollout  = "ServiceRollout"
	kindSmartNICRollout = "SmartNICRollout"
	kindNode            = "Node"
	kindRollout         = "Rollout"
	kindSmartNIC        = "SmartNIC"
)
