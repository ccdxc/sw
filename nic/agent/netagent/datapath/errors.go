package datapath

import "errors"

// ErrIPParse is returned on failing to parse hyphen separated IP Range.
var ErrIPParse = errors.New("hal datapath could not parse the IP")

// ErrInvalidMatchType is returned on an invalid match type
var ErrInvalidMatchType = errors.New("invalid match selector type")

// ErrHALUnavailable is returned when agent can't talk to to HAL.
var ErrHALUnavailable = errors.New("agent could not connect to HAL")

// ErrInvalidNatActionType is returned on an invalid NAT Action
var ErrInvalidNatActionType = errors.New("invalid NAT Action Type")

// ErrInvalidIPSecSAType is returned on an invalid IPSec Policy SA Action
var ErrInvalidIPSecSAType = errors.New("invalid IPSec SA Action")

// ErrIPMissing is returned when the IP field is not specified in an Encrypt SA Action
var ErrIPMissing = errors.New("ipsec encrypt SA needs local and remote gateway IP")

// ErrPortParse is returned on failing to parse a single port or a hyphen separated port range
var ErrPortParse = errors.New("hal datapath could not parse the ports")
