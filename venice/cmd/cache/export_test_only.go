// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

// *** USE ROUTINES IN THIS FILE ONLY FOR UNIT AND INTEG TESTS

package cache

import (
	"time"
)

// SetAPIServerRPCTimeout sets the CMD-side timeout for APIServer update RPCs
func SetAPIServerRPCTimeout(t time.Duration) {
	apiServerRPCTimeout = t
}
