package runtime

import (
	"fmt"
	"strconv"
)

// MustUint32 returns uint32 of a given string or crash.
func MustUint32(numStr string) uint32 {
	num, err := strconv.Atoi(numStr)
	if err != nil || num < 0 {
		panic(fmt.Sprintf("Not an unsigned number: %v, error: %v", numStr, err))
	}
	return uint32(num)
}
