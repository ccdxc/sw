// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package utils

import "strings"

// IsEmpty checks if the given string is empty
func IsEmpty(str string) bool {
	return len(strings.TrimSpace(str)) == 0
}
