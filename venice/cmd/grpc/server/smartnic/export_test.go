// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

// *** USE ROUTINES IN THIS FILE ONLY FOR UNIT AND INTEG TESTS

package smartnic

import (
	"time"
)

// SetNICRegTimeout sets the CMD-side timeout for overall NIC admission process
// (including creating the object in ApiServer if needed)
func SetNICRegTimeout(t time.Duration) {
	nicRegTimeout = t
}

// GetNICRegTimeout gets the CMD-side timeout for overall NIC admission process
// (including creating the object in ApiServer if needed)
func GetNICRegTimeout() time.Duration {
	return nicRegTimeout
}
