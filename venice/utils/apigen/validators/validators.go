package validators

import (
	"fmt"
	"reflect"
	"regexp"
	"strconv"
	"strings"
	"sync"
	"time"

	govldtr "github.com/asaskevich/govalidator"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/runtime"
)

// DummyVar is used to avoid import unused errors
type DummyVar bool

// kindsMap maintains a map of the Kind to api group, initialized the first time
//  it is used.
var (
	groupMap     map[string][]string
	kindsMapOnce sync.Once
)

type regexpEntry struct {
	Str     string
	HelpStr string
	Regexp  *regexp.Regexp
}

// RegexpList contains mapping from regexp name to the regex string,
// the helpstring to appear in the swagger, and a pointer to a regexp
var RegexpList = map[string]regexpEntry{
	"name": {
		Str:     `^[a-zA-Z0-9][\w\-\.]*[a-zA-Z0-9]$`,
		Regexp:  regexp.MustCompile(`^[a-zA-Z0-9][\w\-\.]*[a-zA-Z0-9]$`),
		HelpStr: "must start and end with alpha numeric and can have alphanumeric, -, _, .",
	},
	"alphanum": {
		Str:     `^[a-zA-Z0-9]+$`,
		Regexp:  regexp.MustCompile(`^[a-zA-Z0-9]+$`),
		HelpStr: "must be alpha-numerics",
	},
	"alpha": {
		Str:     `^[a-zA-Z]+$`,
		Regexp:  regexp.MustCompile(`^[a-zA-Z]+$`),
		HelpStr: "must be only alphabets",
	},
	"num": {
		Str:     `^[0-9]+$`,
		Regexp:  regexp.MustCompile(`^[0-9]+$`),
		HelpStr: "must be only numerics",
	},
	"email": {
		Str:     `^[a-zA-Z0-9_\+\-\.]+@([a-zA-Z0-9\-]+\.)+[a-zA-Z]{2,4}$`,
		Regexp:  regexp.MustCompile(`^[a-zA-Z0-9_\+\-\.]+@([a-zA-Z0-9\-]+\.)+[a-zA-Z]{2,4}$`), // http://regexlib.com/Search.aspx?k=email
		HelpStr: "must be a valid email",
	},
}

func populateGroupMap() {
	schema := runtime.GetDefaultScheme()
	groupMap = schema.Kinds()
}

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

// EmptyOr returns no error if the value is empty. If it isn't, it is passed
// to the supplied validator
func EmptyOr(fn interface{}, in string, args []string) error {
	if in == "" {
		return nil
	}
	if len(args) == 0 {
		return fn.(func(string) error)(in)
	}
	return fn.(func(string, []string) error)(in, args)
}

// StrEnum validates that the the input string belongs to the set
//  of values enumerated in the enum specified.
//  This validator is built into the code generator and hence needs no
//  implementation here.

// StrLen validates that the input string is in the range
//  args[0]  :  minimum length of string
//  args[1]  :  maximum length of string, if negative there is no max length
func StrLen(in string, args []string) error {
	min, ok := convInt(args[0])
	if !ok || len(in) < int(min) {
		return fmt.Errorf("Value must have length of at least %d", min)
	}

	max, ok := convInt(args[1])
	if !ok || (max >= 0 && len(in) > int(max)) {
		return fmt.Errorf("Value must have length of at most %d", max)
	}
	return nil
}

// IntRange validates that the input integer is in the range
//  args[0]  :  minimum value
//  args[1]  :  maximum value
func IntRange(i interface{}, args []string) error {
	in, ok := getint(i)
	if !ok {
		return fmt.Errorf("Value must be an integer")
	}
	min, ok := convInt(args[0])
	if !ok {
		return fmt.Errorf("Internal error: min arg is not an integer")
	}
	max, ok := convInt(args[1])
	if !ok {
		return fmt.Errorf("Internal error: max arg is not an integer")
	}
	if in < min {
		return fmt.Errorf("Value must be at least %d", min)
	}
	if in > max {
		return fmt.Errorf("Value must be at most %d", max)
	}
	return nil
}

// IntMin validates that the input integer is at  least the given int
//  args[0]  :  minimum value
func IntMin(i interface{}, args []string) error {
	in, ok := getint(i)
	if !ok {
		return fmt.Errorf("Value must be an integer")
	}
	min, ok := convInt(args[0])
	if !ok {
		return fmt.Errorf("Internal error: min arg is not an integer")
	}
	if in < min {
		return fmt.Errorf("Value must be at least %d", min)
	}
	return nil
}

// CIDR checks if the string is an valid CIDR notiation (IPV4 & IPV6)
//   see tests for valid formats
func CIDR(in string) error {
	if govldtr.IsCIDR(in) {
		return nil
	}
	return fmt.Errorf("Value must be in valid CIDR notation")
}

// IPAddr Checks the input is an IP address in dot notation (V4 or V6)
//   see tests for valid formats
func IPAddr(in string) error {
	if govldtr.IsIP(in) {
		return nil
	}
	return fmt.Errorf("Value must be a valid IP in dot notation")
}

// IPv4 Checks the input is an IPv4 address in dot notation
//   see tests for valid formats
func IPv4(in string) error {
	if govldtr.IsIPv4(in) {
		return nil
	}
	return fmt.Errorf("Value must be a valid IPv4 address")
}

// HostAddr checks the input is either an hostname or address
//   see tests for valid formats
func HostAddr(in string) error {
	if govldtr.IsHost(in) {
		return nil
	}
	return fmt.Errorf("Value must be a valid IP or valid DNS name")
}

// MacAddr verifies it is a valid MAC address is in dot format - aaaa.bbbb.cccc
//   see tests for valid formats
func MacAddr(in string) error {
	r := regexp.MustCompile(`^([0-9a-fA-F]{4}\.){2}([0-9a-fA-F]{4})$`)
	if r.MatchString(in) {
		return nil
	}

	return fmt.Errorf("Value must be a valid MAC address of the format aaaa.bbbb.cccc")
}

// URI validates the input is a valid URI
//   see tests for valid formats
func URI(in string) error {
	if govldtr.IsRequestURI(in) {
		return nil
	}
	return fmt.Errorf("Value must be a valid URI")
}

// UUID validates the input is a valid UUID
//   see tests for valid formats
func UUID(in string) error {
	if govldtr.IsUUID(in) {
		return nil
	}
	return fmt.Errorf("Value must be a valid UUID")
}

// Duration validates that inputs is a valid time Duration string
//  args[0]  :  minimum time duration inclusive, 0 if any duration is allowed
//  args[1]  :  maximum time duration inclusive, 0 if any duration is allowed
func Duration(in string, args []string) error {
	if in == "" {
		return fmt.Errorf("Value cannot be empty")
	}
	min, err := time.ParseDuration(args[0])
	if err != nil {
		return fmt.Errorf("Internal error: min arg is not a duration")
	}
	max, err := time.ParseDuration(args[1])
	if err != nil {
		return fmt.Errorf("Internal error: max arg is not a duration")
	}

	inDuration, err := time.ParseDuration(in)
	if err != nil {
		return fmt.Errorf("Value must be a valid duration")
	}

	if min != 0 && min.Nanoseconds() > inDuration.Nanoseconds() {
		return fmt.Errorf("Value duration must be at least %s", args[0])
	}
	if max != 0 && inDuration.Nanoseconds() > max.Nanoseconds() {
		return fmt.Errorf("Value duration must be at most %s", args[1])
	}
	return nil
}

// XXX We can make these maps programmable by the user, or allow user defined entries
// since user can always specify numerical values, it may be ok to use static map
var l4ProtoMap = map[string]int32{
	// L4 protocols indicated by ip.protocol field
	"ipprotocol": 00,
	"icmp":       01,
	"tcp":        06,
	"udp":        17,
	"esp":        50,
	"ah":         51,
	"icmpv6":     58,
}

var l3ProtoMap = map[string]int32{
	// L3 protocols indicated by ethertype
	"ethertype": 00,
	"arp":       0x806,
}
var icmpv6TypeStrings = map[string]int{
	"destination unreachable":                          1,
	"packet too big":                                   2,
	"time exceeded":                                    3,
	"parameter problem":                                4,
	"echo request":                                     128,
	"echo reply":                                       129,
	"multicast listener query":                         130,
	"multicast listener report":                        131,
	"multicast listener done":                          132,
	"router solicitation":                              133,
	"router advertisement":                             134,
	"neighbor solicitation":                            135,
	"neighbor advertisement":                           136,
	"redirect message":                                 137,
	"router renumbering":                               138,
	"icmp node information query":                      139,
	"icmp node information response":                   140,
	"inverse neighbor discovery solicitation message":  141,
	"inverse neighbor discovery advertisement message": 142,
	"version 2 multicast listener report":              143,
	"home agent address discovery request message":     144,
	"home agent address discovery reply message":       145,
	"mobile prefix solicitation":                       146,
	"mobile prefix advertisement":                      147,
	"certification path solicitation message":          148,
	"certification path advertisement message":         149,
	"multicast router advertisement":                   151,
	"multicast router solicitation":                    152,
	"multicast router termination":                     153,
	"fmipv6 messages":                                  154,
	"rpl control message":                              155,
	"ilnpv6 locator update message":                    156,
	"duplicate address request":                        157,
	"duplicate address confirmation":                   158,
	"mpl control message":                              159,
	"extended echo request":                            160,
	"extended echo reply":                              161,
}
var icmpTypeStrings = map[string]int{
	"echo reply":              0,
	"destination unreachable": 3,
	"redirect":                5,
	"echo":                    8,
	"router advertisement":    9,
	"router solicitation":     10,
	"time exceeded":           11,
	"parameter problem":       12,
	"timestamp":               13,
	"timestamp reply":         14,
	"photuris":                40,
	"extended echo request":   42,
	"extended echo reply":     43,
}

func knownIcmpTypeCodeVldtr(l3l4Info []string) bool {
	proto := strings.ToLower(l3l4Info[0])
	if len(l3l4Info) > 3 {
		return false
	}
	switch len(l3l4Info) {
	case 1:
		return true
	case 2:
		if proto == "icmp" {
			if _, ok := icmpTypeStrings[l3l4Info[1]]; ok {
				return true
			}
		}
		if proto == "icmpv6" {
			if _, ok := icmpv6TypeStrings[l3l4Info[1]]; ok {
				return true
			}
		}
	case 3:
		if proto == "icmp" {
			if _, ok := icmpTypeStrings[l3l4Info[1]]; ok {
				// Too many variations of codes per type, allow an 8-bit numeric value
				if _, err := strconv.ParseInt(l3l4Info[2], 0, 8); err == nil {
					return true
				}
			}
		}
		if proto == "icmpv6" {
			if _, ok := icmpv6TypeStrings[l3l4Info[1]]; ok {
				// Too many variations of codes per type, allow an 8-bit numeric value
				if _, err := strconv.ParseInt(l3l4Info[2], 0, 8); err == nil {
					return true
				}
			}
		}
	default:
	}
	return false
}

func strProtoPortVldtr(in string, allowRange bool) error {
	// Acceptable string formats are -
	// L4_proto/port E.g. tcp/1234
	// L3proto E.g. arp
	// L4_proto_number E.g. ipproto/17
	// L3_proto_number E.g. ethertype/0x806
	// special case for icmp: icmp/<type>/<code>

	port := in
	l3l4Info := strings.Split(port, "/")
	l3l4proto := strings.ToLower(strings.TrimSpace(l3l4Info[0]))
	l4Info, l4ok := l4ProtoMap[l3l4proto]
	l3Info, l3ok := l3ProtoMap[l3l4proto]

	if !l3ok && !l4ok {
		return fmt.Errorf("Protocol must be a valid L3 or L4 <protocol>/<port>")
	}
	l4proto := l4Info
	l3proto := l3Info

	if l4ok {
		if l4proto == 0 {
			if len(l3l4Info) < 2 {
				// allowed format is ipproto/<proto_num>
				return fmt.Errorf("Value had invalid format for %s, expected %s/<port>", l3l4proto, l3l4proto)
			}
			if len(l3l4Info) > 2 {
				// allowed format is ipproto/<proto_num>
				return fmt.Errorf("Value had invalid format for %s, unexpected second '/' ", l3l4proto)
			}
		} else if l3l4proto == "icmp" {
			if len(l3l4Info) > 3 {
				return fmt.Errorf("Value had invalid format for %s, unexpected third '/' ", l3l4proto)
			}
			if knownIcmpTypeCodeVldtr(l3l4Info) {
				return nil
			}
			// if known type-code validation fails, try numeric values
		} else if len(l3l4Info) > 2 {
			return fmt.Errorf("Value had invalid format for %s, unexpected second '/' ", l3l4proto)
		}
	}
	if l3ok {
		if l3proto == 0 {
			if len(l3l4Info) < 2 {
				return fmt.Errorf("Value had invalid format for %s, expected %s/<protocol number>", l3l4proto, l3l4proto)
			}
			if len(l3l4Info) > 2 {
				return fmt.Errorf("Value had invalid format for %s, unexpected second '/' ", l3l4proto)
			}
		} else if len(l3l4Info) > 1 {
			return fmt.Errorf("Value had invalid format for %s, unexpected second '/' ", l3l4proto)
		}
	}
	// rest of the parameters must be numeric values
	for p := range l3l4Info {
		if p == 0 {
			continue
		}
		var err error
		if p == 1 && l3ok && l3proto == 0 {
			// ethertype are 16 bit values
			_, err = strconv.ParseUint(l3l4Info[p], 0, 16)
			if err != nil {
				return fmt.Errorf("Value had invalid format for %s, protocol number must be a 16 bit value", l3l4proto)
			}
		} else if p == 1 && l4ok && l4proto == 0 {
			// ip protocol are 8 bit values
			_, err = strconv.ParseUint(l3l4Info[p], 0, 8)
			if err != nil {
				return fmt.Errorf("Value had invalid format for %s, protocol number must be an 8 bit value", l3l4proto)
			}
		} else {
			if !allowRange {
				_, err = strconv.ParseUint(l3l4Info[p], 0, 16)
				if err != nil {
					return fmt.Errorf("Value had invalid format for %s, protocol number must be a 16 bit value", l3l4proto)
				}
			} else { // validate port range
				totalPorts := 0
				portList := strings.Split(l3l4Info[p], ",")
				for _, pr := range portList {
					ports := strings.Split(pr, "-")
					for _, port := range ports {
						i, err := strconv.Atoi(port)
						if err != nil {
							return fmt.Errorf("port %v must be an integer value", port)
						}
						if 0 > i || i > 65535 {
							return fmt.Errorf("port %v outside range", port)
						}
						totalPorts++
					}
					if len(ports) == 2 {
						first, _ := strconv.Atoi(ports[0])
						second, _ := strconv.Atoi(ports[1])

						if first == 0 && second == 0 {
							return fmt.Errorf("invalid port range %v. upper and lower port range bounds must not be 0", pr)
						}

						if first > second {
							return fmt.Errorf("Invalid port range %v. first number bigger than second", pr)
						}
						totalPorts += second - first - 1 // first & last ports are already counted
					} else if len(ports) > 2 {
						return fmt.Errorf("Invalid port range format: %v", pr)
					}
				}
				if totalPorts > 50 {
					return fmt.Errorf("too large port range, limit 50")
				}
			}
		}
	}
	return nil
}

// ProtoPortRange validates L3/L4 protocol and port range
func ProtoPortRange(port string) error {
	return strProtoPortVldtr(port, true)
}

// ProtoPort validates L3/L4 protocol and port
func ProtoPort(port string) error {
	return strProtoPortVldtr(port, false)
}

// ValidKind validates that the kind is one of the registered Kinds
func ValidKind(in string) error {
	if len(globals.Kind2Category(in)) == 0 {
		return fmt.Errorf("Value must be a valid kind")
	}
	return nil
}

// ValidGroup validates the API group is one of the known groups
func ValidGroup(in string) error {
	kindsMapOnce.Do(populateGroupMap)
	if _, ok := groupMap[in]; !ok {
		return fmt.Errorf("Value must be a valid API group")
	}
	return nil
}

// RegExp validates input string against the named regexp specified in args[0]
func RegExp(in string, args []string) error {
	entry, ok := RegexpList[args[0]]
	if !ok {
		return fmt.Errorf("Internal error: regexp name given not recognized")
	}
	if !entry.Regexp.Match([]byte(in)) {
		return fmt.Errorf("Value %s", entry.HelpStr)
	}
	return nil
}
