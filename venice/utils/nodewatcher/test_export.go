// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package nodewatcher

import (
	"time"
)

// override functions only to be used in tests

// SetMinimumFrequency sets the value of the nodewatcher minimum frequency
func SetMinimumFrequency(f time.Duration) {
	minFrequency = f
}

// GetMinimumFrequency returns the value of the nodewatcher minimum frequency
func GetMinimumFrequency() time.Duration {
	return minFrequency
}
