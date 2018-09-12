// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
package smartnic

import (
	"time"
)

func SetNICRegTimeout(t time.Duration) {
	nicRegTimeout = t
}
