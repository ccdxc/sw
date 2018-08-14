package libs

import "fmt"

var (
	HEIMDALL_RANDOM_SEED              int64 = 42
	ErrInsufficientIPAddressGenerated       = fmt.Errorf("could not generate the required nunmber of IP Addresses")
)
