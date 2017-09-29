package impl

import (
	"reflect"
	"strconv"

	govldtr "github.com/asaskevich/govalidator"
)

// DummyVar is used to avoid import unused errors
type DummyVar bool

// convInt is a utility function to get convert string to integer
func convInt(in string) (int64, bool) {
	ret, err := strconv.ParseInt(in, 10, 64)
	if err != nil {
		return 0, false
	}
	return ret, true
}

// getint is a utility function to get value from integer types
func getint(in interface{}) (int64, bool) {
	v := reflect.ValueOf(in)
	if v.Kind() == reflect.Ptr {
		v = v.Elem()
	}
	switch in.(type) {
	case uint32, uint64, uint:
		return int64(v.Uint()), true
	case int32, int64, int:
		return v.Int(), true
	default:
		return 0, false
	}
}

// StrEnum validates that the the input string belongs to the set
//  of values enumerated in the enum specified.
//  This validator is built into the code generator and hence needs no
//  implementation here.

// StrLen validates that the input string is in the range
//  args[0]  :  minimum length of string
//  args[1]  :  maximum length of string
func StrLen(in string, args []string) bool {
	min, ok := convInt(args[0])
	if !ok {
		return false
	}
	max, ok := convInt(args[1])
	if !ok {
		return false
	}
	if len(in) < int(min) || len(in) > int(max) {
		return false
	}
	return true
}

// IntRange validates that the input integer is in the range
//  args[0]  :  minimum value
//  args[1]  :  maximum value
func IntRange(i interface{}, args []string) bool {
	in, ok := getint(i)
	if !ok {
		return false
	}
	min, ok := convInt(args[0])
	if !ok {
		return false
	}
	max, ok := convInt(args[1])
	if !ok {
		return false
	}
	if in < min || in > max {
		return false
	}
	return true
}

// IPAddr Checks the input is an IP address in dot notation (V4 or V6)
//   see tests for valid formats
func IPAddr(i interface{}) bool {
	in := i.(string)
	return govldtr.IsIP(in)
}

// IPv4 Checks the input is an IPv4 address in dot notation
//   see tests for valid formats
func IPv4(i interface{}) bool {
	in := i.(string)
	return govldtr.IsIPv4(in)
}

// HostAddr checks the input is either an hostname or address
//   see tests for valid formats
func HostAddr(i interface{}) bool {
	in := i.(string)
	return govldtr.IsHost(in)
}

// MacAddr verifies it is a valid MAC address in one of the supported notations
//   see tests for valid formats
func MacAddr(i interface{}) bool {
	in := i.(string)
	return govldtr.IsMAC(in)
}

// URI validates the input is a valid URI
//   see tests for valid formats
func URI(i interface{}) bool {
	in := i.(string)
	return govldtr.IsRequestURI(in)
}

// UUID validates the input is a valid UUID
//   see tests for valid formats
func UUID(i interface{}) bool {
	in := i.(string)
	return govldtr.IsUUID(in)
}
