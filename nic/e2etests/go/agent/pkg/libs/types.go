package libs

import "errors"

var (
	HEIMDALL_RANDOM_SEED              int64 = 42
	ErrInsufficientIPAddressGenerated       = errors.New("could not generate the required nunmber of IP Addresses")
	ErrHeimdallSkip                         = errors.New("skip heimdall ops")
)
