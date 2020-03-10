package validators

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

func TestStrLen(t *testing.T) {
	cases := []struct {
		input  string
		args   []string
		errMsg string
	}{
		{
			input:  "",
			args:   []string{"10", "25"},
			errMsg: "Value must have length of at least 10",
		},
		{
			input:  "Abcdefghijklmnopqrstuvwzyz",
			args:   []string{"10", "25"},
			errMsg: "Value must have length of at most 25",
		},
		{
			input:  "Abcdefghijklmnopqrstuvwzyz",
			args:   []string{"10", "30"},
			errMsg: "",
		},
		{
			input:  "Abcdefghijklmnopqrstuvwzyz",
			args:   []string{"10", "-10"},
			errMsg: "",
		},
	}
	for i, c := range cases {
		err := StrLen(c.input, c.args)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestIntRange(t *testing.T) {
	cases := []struct {
		input  int32
		args   []string
		errMsg string
	}{
		{
			input:  -20,
			args:   []string{"10", "25"},
			errMsg: "Value must be at least 10",
		},
		{
			input:  20,
			args:   []string{"10", "25"},
			errMsg: "",
		},
		{
			input:  10,
			args:   []string{"10", "25"},
			errMsg: "",
		},
		{
			input:  25,
			args:   []string{"10", "25"},
			errMsg: "",
		},
		{
			input:  25,
			args:   []string{"-11", "25"},
			errMsg: "",
		},
		{
			input:  -12,
			args:   []string{"-11", "25"},
			errMsg: "Value must be at least -11",
		},
	}
	for i, c := range cases {
		err := IntRange(c.input, c.args)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestIntMin(t *testing.T) {
	cases := []struct {
		input  int32
		args   []string
		errMsg string
	}{
		{
			input:  -20,
			args:   []string{"10"},
			errMsg: "Value must be at least 10",
		},
		{
			input:  0,
			args:   []string{"10"},
			errMsg: "Value must be at least 10",
		},
		{
			input:  10,
			args:   []string{"10"},
			errMsg: "",
		},
		{
			input:  100,
			args:   []string{"10", "25"},
			errMsg: "",
		},
	}
	for i, c := range cases {
		err := IntMin(c.input, c.args)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestCIDR(t *testing.T) {
	cases := []struct {
		s      string
		errMsg string
	}{
		{s: "10.1.1.1/16", errMsg: ""},
		{s: "10.1.1.1", errMsg: "Value must be in valid CIDR notation"},
		{s: "2001:db8::/32", errMsg: ""},
		{s: "2001:0db8:85a3:0000:0000:8a2e:0370:7334/64", errMsg: ""},
		{s: "193.138.3.20/60", errMsg: "Value must be in valid CIDR notation"},
		{s: "500.323.2.23/43", errMsg: "Value must be in valid CIDR notation"},
		{s: "", errMsg: "Value must be in valid CIDR notation"},
	}
	for i, c := range cases {
		err := CIDR(c.s)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}
func TestIpAddr(t *testing.T) {
	cases := []struct {
		s      string
		errMsg string
	}{
		{s: "10.1.1.1", errMsg: ""},
		{s: "10.1.1.", errMsg: "Value must be a valid IP in dot notation"},
		{s: "10.1.1", errMsg: "Value must be a valid IP in dot notation"},
		{s: "10.01.001.4", errMsg: ""},
		{s: ".10.1.1.100", errMsg: "Value must be a valid IP in dot notation"},
		{s: "localhost", errMsg: "Value must be a valid IP in dot notation"},
		{s: "169.1.1.11111", errMsg: "Value must be a valid IP in dot notation"},
		{s: "169.1.1,11", errMsg: "Value must be a valid IP in dot notation"},
		{s: "10.1.1.0/24", errMsg: "Value must be a valid IP in dot notation"},
		{s: "A.1.C.0/24", errMsg: "Value must be a valid IP in dot notation"},
		{s: "ff02::5", errMsg: ""},
		{s: "0012:3dae:2341:1:0:4:c:d:a", errMsg: "Value must be a valid IP in dot notation"},
		{s: "0012:3dae:2341:1:0:4:c:d", errMsg: ""},
	}
	for i, c := range cases {
		err := IPAddr(c.s)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestIpV4(t *testing.T) {
	cases := []struct {
		s      string
		errMsg string
	}{
		{s: "10.1.1.1", errMsg: ""},
		{s: "10.1.1.", errMsg: "Value must be a valid IPv4 address"},
		{s: "10.1.1", errMsg: "Value must be a valid IPv4 address"},
		{s: ".10.1.1.100", errMsg: "Value must be a valid IPv4 address"},
		{s: "localhost", errMsg: "Value must be a valid IPv4 address"},
		{s: "169.1.1.11111", errMsg: "Value must be a valid IPv4 address"},
		{s: "169.1.1,11", errMsg: "Value must be a valid IPv4 address"},
		{s: "0012:3dae:2341:1:0:4:c:d", errMsg: "Value must be a valid IPv4 address"},
	}
	for i, c := range cases {
		err := IPv4(c.s)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestHostAddr(t *testing.T) {
	cases := []struct {
		s      string
		errMsg string
	}{
		{s: "10.1.1.1", errMsg: ""},
		{s: "10.1.1.", errMsg: ""},
		{s: "10.1.1", errMsg: ""},
		{s: "10.01.001.4", errMsg: ""},
		{s: "localhost", errMsg: ""},
		{s: "A.1.C.0", errMsg: ""},
		{s: "ff02::5", errMsg: ""},
		{s: "0012:3dae:2341:1:0:4:c:d", errMsg: ""},
		{s: "testhost.local", errMsg: ""},
		{s: "testhost.local.", errMsg: ""},
		{s: ".10.1.1.100", errMsg: "Value must be a valid IP or valid DNS name"},
		{s: "169.1.1,11", errMsg: "Value must be a valid IP or valid DNS name"},
		{s: "10.1.1.0/24", errMsg: "Value must be a valid IP or valid DNS name"},
		{s: "0012:3dae:2341:1:0:4:c:d:a", errMsg: "Value must be a valid IP or valid DNS name"},
		{s: ".testhost.local", errMsg: "Value must be a valid IP or valid DNS name"},
	}
	for i, c := range cases {
		err := HostAddr(c.s)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestMacAddr(t *testing.T) {
	expMacErr := "Value must be a valid MAC address of the format aaaa.bbbb.cccc"

	cases := []struct {
		s      string
		errMsg string
	}{
		{s: "aa:bb:cc:dd:00:00", errMsg: expMacErr},
		{s: "aaBB.ccDD.0000", errMsg: ""},
		{s: "aa-BB-cc-DD-00-00", errMsg: expMacErr},
		{s: "aaBB.ccDD.0000", errMsg: ""},
		{s: "aa.BB.cc.DD.00.00", errMsg: expMacErr},
		{s: "aaBB.ccDD.00.00", errMsg: expMacErr},
		{s: "aa:BB:cc:DD:00:00", errMsg: expMacErr},
		{s: "aaXY.ccDD.0000", errMsg: expMacErr},
		{s: "aaBB.ccDD.00000", errMsg: expMacErr},
		{s: ",,,,......abcd", errMsg: expMacErr},
		{s: "aaBB,ccDD,0000", errMsg: expMacErr},
		{s: "aaBB..ccDD.0000", errMsg: expMacErr},
		{s: "aaBB:ccDD:0000", errMsg: expMacErr},
		{s: "aaBB.ccDD.0000,aaBB.ccDD.0000", errMsg: expMacErr},
		{s: "aaBB,aaBB.ccDD.0000", errMsg: expMacErr},
		{s: "zzzz,aaBB.ccDD.0000", errMsg: expMacErr},
		{s: "aaBB.aaBB.ccDD.0000", errMsg: expMacErr},
	}
	for i, c := range cases {
		err := MacAddr(c.s)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestURI(t *testing.T) {
	cases := []struct {
		s      string
		errMsg string
	}{
		{s: "abc.x.y", errMsg: "Value must be a valid URI"},
		{s: "https://abc.x.i", errMsg: ""},
		{s: "https://abc.x.in", errMsg: ""},
		{s: "https://10.1.1.1", errMsg: ""},
		{s: "https://10.1.1.1:8000", errMsg: ""},
		{s: "ldap://10.1.1.1:800", errMsg: ""},
		{s: "sd://testservice/xyz", errMsg: ""},
		{s: ".http:testservice/xyz", errMsg: "Value must be a valid URI"},
		{s: "ref:testservice/xyz", errMsg: ""},
		{s: "10.1.1.1", errMsg: "Value must be a valid URI"},
		{s: "/path/to/x", errMsg: ""},
		{s: "path/to/x", errMsg: "Value must be a valid URI"},
	}
	for i, c := range cases {
		err := URI(c.s)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestUUID(t *testing.T) {
	cases := []struct {
		s      string
		errMsg string
	}{
		{s: "49943a2c-9d76-11e7-abc4-cec278b6b50a", errMsg: ""},
		{s: "970fd5c3-ebde-4a42-8212-d4bac833ddac", errMsg: ""},
		{s: "00000000-0000-0000-0000-000000000000", errMsg: ""},
		{s: "970fd5c3ebde4a428212d4bac833ddac", errMsg: "Value must be a valid UUID"},
		{s: "uuid:970fd5c3ebde4a428212d4bac833ddac", errMsg: "Value must be a valid UUID"},
	}
	for i, c := range cases {
		err := UUID(c.s)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestL3L4Proto(t *testing.T) {

	cases := []struct {
		s      string
		errMsg string
	}{
		// Good cases
		{s: "TCP/1234"},
		{s: "Udp"},
		{s: "udp/65535"},
		{s: "IcMp"},
		{s: "aRP"},
		{s: "ipprotocol/17"},
		{s: "ethertype/0x806"},
		{s: "icmp/1"},
		{s: "icmp/3/2"},
		{s: "icmp/echo reply"},
		{s: "icmp/redirect/5"},
		// Bad cases
		{s: "", errMsg: "Protocol must be a valid L3 or L4 <protocol>/<port>"},
		{s: "SCP/1234", errMsg: "Protocol must be a valid L3 or L4 <protocol>/<port>"},
		{s: "Udp/1/2", errMsg: "Value had invalid format for udp, unexpected second '/' "},
		{s: "udp/-123", errMsg: "Value had invalid format for udp, protocol number must be a 16 bit value"},
		{s: "foo", errMsg: "Protocol must be a valid L3 or L4 <protocol>/<port>"},
		{s: "ipprotocol", errMsg: "Value had invalid format for ipprotocol, expected ipprotocol/<port>"},
		{s: "ipprotocol/17/1/2", errMsg: "Value had invalid format for ipprotocol, unexpected second '/' "},
		{s: "ethertype/0x806/10", errMsg: "Value had invalid format for ethertype, unexpected second '/' "},
		{s: "ethertype", errMsg: "Value had invalid format for ethertype, expected ethertype/<protocol number>"},
		{s: "ethertype/abc", errMsg: "Value had invalid format for ethertype, protocol number must be a 16 bit value"},
		{s: "ipprotocol/xyz", errMsg: "Value had invalid format for ipprotocol, protocol number must be an 8 bit value"},
		{s: "ipprotocol/500", errMsg: "Value had invalid format for ipprotocol, protocol number must be an 8 bit value"},
		{s: "icmp/xyz", errMsg: "Value had invalid format for icmp, protocol number must be a 16 bit value"},
	}
	for i, c := range cases {
		err := ProtoPort(c.s)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestL3L4ProtoRange(t *testing.T) {

	cases := []struct {
		s      string
		errMsg string
	}{
		// Good cases
		{s: "TCP/1234-1235"},
		{s: "TCP/1234-1234"},
		{s: "TCP/1234,1235"},
		{s: "TCP/1234,1235-1236,1238"},
		{s: "TCP/1234"},
		{s: "Udp"},
		{s: "udp/65535"},
		{s: "IcMp"},
		{s: "aRP"},
		{s: "ipprotocol/17"},
		{s: "ethertype/0x806"},
		{s: "icmp/1"},
		{s: "icmp/3/2"},
		{s: "icmp/echo reply"},
		{s: "icmp/redirect/5"},
		// Bad cases
		{s: "TCP/1234-1294", errMsg: "too large port range, limit 50"},
		{s: "TCP/1234-1224", errMsg: "Invalid port range 1234-1224. first number bigger than second"},
		{s: "", errMsg: "Protocol must be a valid L3 or L4 <protocol>/<port>"},
		{s: "SCP/1234", errMsg: "Protocol must be a valid L3 or L4 <protocol>/<port>"},
		{s: "Udp/1/2", errMsg: "Value had invalid format for udp, unexpected second '/' "},
		{s: "udp/-123", errMsg: "port  must be an integer value"},
		{s: "foo", errMsg: "Protocol must be a valid L3 or L4 <protocol>/<port>"},
		{s: "ipprotocol", errMsg: "Value had invalid format for ipprotocol, expected ipprotocol/<port>"},
		{s: "ipprotocol/17/1/2", errMsg: "Value had invalid format for ipprotocol, unexpected second '/' "},
		{s: "ethertype/0x806/10", errMsg: "Value had invalid format for ethertype, unexpected second '/' "},
		{s: "ethertype", errMsg: "Value had invalid format for ethertype, expected ethertype/<protocol number>"},
		{s: "ethertype/abc", errMsg: "Value had invalid format for ethertype, protocol number must be a 16 bit value"},
		{s: "ipprotocol/xyz", errMsg: "Value had invalid format for ipprotocol, protocol number must be an 8 bit value"},
		{s: "ipprotocol/500", errMsg: "Value had invalid format for ipprotocol, protocol number must be an 8 bit value"},
		{s: "icmp/xyz", errMsg: "port xyz must be an integer value"},
	}
	for i, c := range cases {
		err := ProtoPortRange(c.s)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d:%v Validation should have failed", i, c.s)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d:%v Validation should have failed with %s but got %s", i, c.s, c.errMsg, err.Error())
		}
	}
}

func TestDuration(t *testing.T) {
	cases := []struct {
		input  string
		args   []string
		errMsg string
	}{
		{
			input: "1h",
			args:  []string{"5s", "1h"},
		},
		{
			input: "2m",
			args:  []string{"5s", "1h"},
		},
		{
			input: "10s",
			args:  []string{"5s", "1h"},
		},
		// bad cases
		{
			input:  "1:20:10",
			args:   []string{"5s", "1h"},
			errMsg: "Value must be a valid duration",
		},
		{
			input:  "mytime",
			args:   []string{"5s", "1h"},
			errMsg: "Value must be a valid duration",
		},
		{
			input:  "3 s",
			args:   []string{"5s", "1h"},
			errMsg: "Value must be a valid duration",
		},
		{
			input:  "",
			args:   []string{"5s", "1h"},
			errMsg: "Value cannot be empty",
		},
		{
			input:  "2h",
			args:   []string{"5s", "1h"},
			errMsg: "Value duration must be at most 1h",
		},
		{
			input:  "65m",
			args:   []string{"5s", "1h"},
			errMsg: "Value duration must be at most 1h",
		},
		{
			input:  "2s",
			args:   []string{"5s", "1h"},
			errMsg: "Value duration must be at least 5s",
		},
	}
	for i, c := range cases {
		err := Duration(c.input, c.args)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestEmptyOr(t *testing.T) {
	casesDuration := []struct {
		input  string
		args   []string
		errMsg string
	}{
		{
			input: "10h",
			args:  []string{"0", "0"},
		},
		{
			input: "2m",
			args:  []string{"0", "0"},
		},
		{
			input: "3s",
			args:  []string{"0", "0"},
		},
		{
			input: "",
			args:  []string{"0", "0"},
		},
		// bad cases
		{
			input:  "1:20:10",
			args:   []string{"0", "0"},
			errMsg: "Value must be a valid duration",
		},
		{
			input:  "mytime",
			args:   []string{"0", "0"},
			errMsg: "Value must be a valid duration",
		},
		{
			input:  "3 s",
			args:   []string{"0", "0"},
			errMsg: "Value must be a valid duration",
		},
	}
	for i, c := range casesDuration {
		err := EmptyOr(Duration, c.input, c.args)
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
	nameErrMsg := "Value must start and end with alpha numeric and can have alphanumeric, -, _, ."
	alphanumErrMsg := "Value must be alpha-numerics"
	alphaErrMsg := "Value must be only alphabets"
	numErrMsg := "Value must be only numerics"
	emailErrMsg := "Value must be a valid email"

	casesRegex := []struct {
		exp    string
		val    string
		errMsg string
	}{
		{exp: "name", val: "", errMsg: ""},
		{exp: "name", val: "Andadaa_98", errMsg: ""},
		{exp: "name", val: "Andadaa_:.98", errMsg: nameErrMsg},
		{exp: "name", val: "_Andadaa_98", errMsg: nameErrMsg},
		{exp: "name", val: "Andadaa_ 98", errMsg: nameErrMsg},
		{exp: "name", val: "Andadaa%_98", errMsg: nameErrMsg},
		{exp: "name", val: "Andadaa%_98", errMsg: nameErrMsg},
		{exp: "alphanum", val: "", errMsg: ""},
		{exp: "alphanum", val: "Andadaa98", errMsg: ""},
		{exp: "alphanum", val: "Andadaa_98", errMsg: alphanumErrMsg},
		{exp: "alpha", val: "", errMsg: ""},
		{exp: "alpha", val: "Andadaa", errMsg: ""},
		{exp: "alpha", val: "Andadaa1", errMsg: alphaErrMsg},
		{exp: "num", val: "", errMsg: ""},
		{exp: "num", val: "1123131122", errMsg: ""},
		{exp: "num", val: "112313112a", errMsg: numErrMsg},
		{exp: "email", val: "notAnEmail", errMsg: emailErrMsg},
		{exp: "email", val: "", errMsg: ""},
		{exp: "email", val: "test@pensando.io", errMsg: ""},
		{exp: "email", val: "test+email@bbc.co.uk", errMsg: ""},
	}
	for i, c := range casesRegex {
		err := EmptyOr(RegExp, c.val, []string{c.exp})
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestRegExp(t *testing.T) {
	nameErrMsg := "Value must start and end with alpha numeric and can have alphanumeric, -, _, ."
	alphanumErrMsg := "Value must be alpha-numerics"
	alphaErrMsg := "Value must be only alphabets"
	numErrMsg := "Value must be only numerics"
	emailErrMsg := "Value must be a valid email"
	alphanumdashErrMsg := "Value must be alpha numeric and can have -"

	cases := []struct {
		exp    string
		val    string
		errMsg string
	}{
		{exp: "name", val: "", errMsg: nameErrMsg},
		{exp: "name", val: "Andadaa_98", errMsg: ""},
		{exp: "name", val: "Andadaa_:.98", errMsg: nameErrMsg},
		{exp: "name", val: "_Andadaa_98", errMsg: nameErrMsg},
		{exp: "name", val: "Andadaa_ 98", errMsg: nameErrMsg},
		{exp: "name", val: "Andadaa%_98", errMsg: nameErrMsg},
		{exp: "name", val: "Andadaa%_98", errMsg: nameErrMsg},
		{exp: "alphanum", val: "", errMsg: alphanumErrMsg},
		{exp: "alphanum", val: "Andadaa98", errMsg: ""},
		{exp: "alphanum", val: "Andadaa_98", errMsg: alphanumErrMsg},
		{exp: "alpha", val: "", errMsg: alphaErrMsg},
		{exp: "alpha", val: "Andadaa", errMsg: ""},
		{exp: "alpha", val: "Andadaa1", errMsg: alphaErrMsg},
		{exp: "num", val: "", errMsg: numErrMsg},
		{exp: "num", val: "1123131122", errMsg: ""},
		{exp: "num", val: "112313112a", errMsg: numErrMsg},
		{exp: "email", val: "notAnEmail", errMsg: emailErrMsg},
		{exp: "email", val: "", errMsg: emailErrMsg},
		{exp: "email", val: "test@pensando.io", errMsg: ""},
		{exp: "email", val: "test+email@bbc.co.uk", errMsg: ""},
		{exp: "alphanumdash", val: "Anda-daa98", errMsg: ""},
		{exp: "alphanumdash", val: "12Anda-daa98-", errMsg: ""},
		{exp: "alphanumdash", val: "Anda_daa98", errMsg: alphanumdashErrMsg},
		{exp: "alphanumdash", val: "Anda#daa98", errMsg: alphanumdashErrMsg},
	}
	for i, c := range cases {
		err := RegExp(c.val, []string{c.exp})
		if err == nil && c.errMsg != "" {
			t.Errorf("Test case %d: Validation should have failed", i)
		} else if err != nil && err.Error() != c.errMsg {
			t.Errorf("Test case %d: Validation should have failed with %s but got %s", i, c.errMsg, err.Error())
		}
	}
}

func TestValidKind(t *testing.T) {
	types := map[string]*api.Struct{
		"test.Type1": {
			Kind:     "TestKind1",
			APIGroup: "TestGroup1",
			Fields: map[string]api.Field{
				"Fld1": {Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "test.Type2"},
				"Fld2": {Name: "Fld2", JSONTag: "fld2", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"test.Type2": {
			Kind:     "TestKind2",
			APIGroup: "TestGroup1",
			Fields: map[string]api.Field{
				"Fld1": {Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"test.Type3": {
			Kind:     "TestKind3",
			APIGroup: "TestGroup2",
			Fields: map[string]api.Field{
				"Fld1": {Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
	}
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(types)
	cases := map[string]string{
		"TestKind1": "", "TestKind2": "", "TestKind3": "", "testKind": "Value must be a valid kind", "Dummy": "Value must be a valid kind",
	}
	for k, v := range cases {
		err := ValidKind(k)
		if err == nil && v != "" {
			t.Errorf("Test case %s: Validation should have failed", k)
		} else if err != nil && err.Error() != v {
			t.Errorf("Test case %s: Validation should have failed with %s but got %s", k, v, err.Error())
		}
	}

	cases = map[string]string{
		"TestGroup1": "", "TestGroup2": "", "UnknownGroup": "Value must be a valid API group", "testGroup1": "Value must be a valid API group",
	}

	for k, v := range cases {
		err := ValidGroup(k)
		if err == nil && v != "" {
			t.Errorf("Test case %s: Validation should have failed", k)
		} else if err != nil && err.Error() != v {
			t.Errorf("Test case %s: Validation should have failed with %s but got %s", k, v, err.Error())
		}
	}
}
