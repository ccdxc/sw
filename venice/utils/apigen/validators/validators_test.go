package impl

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

func TestStrLen(t *testing.T) {
	str := "Abcdefghijklmnopqrstuvwzyz"
	var args []string
	args = append(args, "10", "25")

	if StrLen(str, args) {
		t.Errorf("Validation should have failed")
	}
	args[1] = "30"
	if !StrLen(str, args) {
		t.Errorf("Validation should have passed")
	}
}

func TestIntRange(t *testing.T) {
	var val int32 = -20
	var args []string
	args = append(args, "10", "25")
	if IntRange(val, args) {
		t.Errorf("validation expected to fail")
	}

	val = 20
	if !IntRange(val, args) {
		t.Errorf("validation expected to pass")
	}
	val = 23
	if !IntRange(val, args) {
		t.Errorf("validation expected to pass")
	}
	val = 10
	if !IntRange(val, args) {
		t.Errorf("validation expected to pass")
	}
	args[0] = "-11"
	val = 0
	if !IntRange(val, args) {
		t.Errorf("validation expected to pass")
	}

	val = -12
	if IntRange(val, args) {
		t.Errorf("validation expected to fail")
	}
	var v1 uint32 = 10
	if !IntRange(v1, args) {
		t.Errorf("validation expected to pass")
	}
}

func TestCIDR(t *testing.T) {
	cases := []struct {
		s   string
		exp bool
	}{
		{s: "10.1.1.1/16", exp: true},
		{s: "10.1.1.1", exp: false},
		{s: "2001:db8::/32", exp: true},
		{s: "2001:0db8:85a3:0000:0000:8a2e:0370:7334/64", exp: true},
		{s: "193.138.3.20/60", exp: false},
		{s: "500.323.2.23/43", exp: false},
		{s: "", exp: false},
	}
	for _, c := range cases {
		if CIDR(c.s) != c.exp {
			t.Errorf("Expected [%v] got [%v] for [%s]", c.exp, CIDR(c.s), c.s)
		}
	}
}
func TestIpAddr(t *testing.T) {
	cases := []struct {
		s   string
		exp bool
	}{
		{s: "10.1.1.1", exp: true},
		{s: "10.1.1.", exp: false},
		{s: "10.1.1", exp: false},
		{s: "10.01.001.4", exp: true},
		{s: ".10.1.1.100", exp: false},
		{s: "localhost", exp: false},
		{s: "169.1.1.11111", exp: false},
		{s: "169.1.1,11", exp: false},
		{s: "10.1.1.0/24", exp: false},
		{s: "A.1.C.0/24", exp: false},
		{s: "ff02::5", exp: true},
		{s: "0012:3dae:2341:1:0:4:c:d:a", exp: false},
		{s: "0012:3dae:2341:1:0:4:c:d", exp: true},
	}
	for _, c := range cases {
		if IPAddr(c.s) != c.exp {
			t.Errorf("Expected [%v] got [%v] for [%s]", c.exp, IPAddr(c.s), c.s)
		}
	}
}

func TestIpV4(t *testing.T) {
	cases := []struct {
		s   string
		exp bool
	}{
		{s: "10.1.1.1", exp: true},
		{s: "10.1.1.", exp: false},
		{s: "10.1.1", exp: false},
		{s: ".10.1.1.100", exp: false},
		{s: "localhost", exp: false},
		{s: "169.1.1.11111", exp: false},
		{s: "169.1.1,11", exp: false},
		{s: "0012:3dae:2341:1:0:4:c:d", exp: false},
	}
	for _, c := range cases {
		if IPv4(c.s) != c.exp {
			t.Errorf("Expected [%v] got [%v] for [%s]", c.exp, IPv4(c.s), c.s)
		}
	}
}

func TestHostAddr(t *testing.T) {
	cases := []struct {
		s   string
		exp bool
	}{
		{s: "10.1.1.1", exp: true},
		{s: "10.1.1.", exp: true},
		{s: "10.1.1", exp: true},
		{s: "10.01.001.4", exp: true},
		{s: "localhost", exp: true},
		{s: "A.1.C.0", exp: true},
		{s: "ff02::5", exp: true},
		{s: "0012:3dae:2341:1:0:4:c:d", exp: true},
		{s: "testhost.local", exp: true},
		{s: "testhost.local.", exp: true},
		{s: ".10.1.1.100", exp: false},
		{s: "169.1.1,11", exp: false},
		{s: "10.1.1.0/24", exp: false},
		{s: "0012:3dae:2341:1:0:4:c:d:a", exp: false},
		{s: ".testhost.local", exp: false},
	}
	for _, c := range cases {
		if HostAddr(c.s) != c.exp {
			t.Errorf("Expected [%v] got [%v] for [%s]", c.exp, HostAddr(c.s), c.s)
		}
	}
}

func TestMacAddr(t *testing.T) {
	cases := []struct {
		s   string
		exp bool
	}{
		{s: "aa:BB:cc:DD:00:00", exp: true},
		{s: "aaBB.ccDD.0000", exp: true},
		{s: "aa-BB-cc-DD-00-00", exp: true},
		{s: "aaBB.ccDD.0000", exp: true},
		{s: "aa.BB.cc.DD.00.00", exp: false},
		{s: "aaBB.ccDD.00.00", exp: false},
		{s: "aa:BB:cc:DD:00:00", exp: true},
		{s: "aaXY.ccDD.0000", exp: false},
	}
	for _, c := range cases {
		if MacAddr(c.s) != c.exp {
			t.Errorf("Expected [%v] got [%v] for [%s]", c.exp, MacAddr(c.s), c.s)
		}
	}
}

func TestURI(t *testing.T) {
	cases := []struct {
		s   string
		exp bool
	}{
		{s: "abc.x.y", exp: false},
		{s: "https://abc.x.i", exp: true},
		{s: "https://abc.x.in", exp: true},
		{s: "https://10.1.1.1", exp: true},
		{s: "https://10.1.1.1:8000", exp: true},
		{s: "ldap://10.1.1.1:800", exp: true},
		{s: "sd://testservice/xyz", exp: true},
		{s: ".http:testservice/xyz", exp: false},
		{s: "ref:testservice/xyz", exp: true},
		{s: "10.1.1.1", exp: false},
		{s: "/path/to/x", exp: true},
		{s: "path/to/x", exp: false},
	}
	for _, c := range cases {
		if URI(c.s) != c.exp {
			t.Errorf("Expected [%v] got [%v] for [%s]", c.exp, URI(c.s), c.s)
		}
	}
}

func TestUUID(t *testing.T) {
	cases := []struct {
		s   string
		exp bool
	}{
		{s: "49943a2c-9d76-11e7-abc4-cec278b6b50a", exp: true},
		{s: "970fd5c3-ebde-4a42-8212-d4bac833ddac", exp: true},
		{s: "00000000-0000-0000-0000-000000000000", exp: true},
		{s: "970fd5c3ebde4a428212d4bac833ddac", exp: false},
		{s: "uuid:970fd5c3ebde4a428212d4bac833ddac", exp: false},
	}
	for _, c := range cases {
		if UUID(c.s) != c.exp {
			t.Errorf("Expected [%v] got [%v] for [%s]", c.exp, UUID(c.s), c.s)
		}
	}
}

func TestL3L4Proto(t *testing.T) {
	goodCases := []string{
		"TCP/1234", "Udp", "udp/65535", "IcMp", "aRP", "ipprotocol/17", "ethertype/0x806",
		"icmp/1", "icmp/3/2", "icmp/echo reply", "icmp/redirect/5",
	}
	badCases := []string{
		"SCP/1234", "Udp/1/2", "udp/-123", "foo", "ipprotocol", "ipprotocol/17/1/2", "ethertype/0x806/10",
		"ethertype", "ethertype/abc", "ipprotocol/xyz", "ipprotocol/500", "icmp/xyz",
	}
	for _, c := range goodCases {
		if ok := ProtoPort(c); !ok {
			t.Errorf("Incorrect Error detection in %v", c)
		}
	}
	for _, c := range badCases {
		if ok := ProtoPort(c); ok {
			t.Errorf("Undetected error in %v", c)
		}
	}
}

func TestDuration(t *testing.T) {
	goodCases := []string{
		"10h", "2m", "3s",
	}
	badCases := []string{
		"1:20:10", "mytime", "3 s",
	}
	for _, c := range goodCases {
		if ok := Duration(c); !ok {
			t.Errorf("Incorrect Error detection in %v", c)
		}
	}
	for _, c := range badCases {
		if ok := Duration(c); ok {
			t.Errorf("Undetected error in %v", c)
		}
	}
}

func TestRegExp(t *testing.T) {
	cases := []struct {
		exp    string
		val    string
		result bool
	}{
		{exp: "name", val: "Andadaa_98", result: true},
		{exp: "name", val: "Andadaa_:.98", result: true},
		{exp: "name", val: "_Andadaa_98", result: false},
		{exp: "name", val: "Andadaa_ 98", result: false},
		{exp: "name", val: "Andadaa%_98", result: false},
		{exp: "name", val: "Andadaa%_98", result: false},
		{exp: "alphanum", val: "Andadaa98", result: true},
		{exp: "alphanum", val: "Andadaa_98", result: false},
		{exp: "alpha", val: "Andadaa", result: true},
		{exp: "alpha", val: "Andadaa1", result: false},
		{exp: "num", val: "1123131122", result: true},
		{exp: "num", val: "112313112a", result: false},
	}
	for _, c := range cases {
		args := []string{c.exp}
		r := RegExp(c.val, args)
		if r != c.result {
			t.Errorf("[%v/[%v] expecting [%v] got [%v]", c.exp, c.val, c.result, r)
		}
	}
}
func TestValidKind(t *testing.T) {
	types := map[string]*api.Struct{
		"test.Type1": &api.Struct{
			Kind:     "TestKind1",
			APIGroup: "TestGroup1",
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "test.Type2"},
				"Fld2": api.Field{Name: "Fld2", JSONTag: "fld2", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"test.Type2": &api.Struct{
			Kind:     "TestKind2",
			APIGroup: "TestGroup1",
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"test.Type3": &api.Struct{
			Kind:     "TestKind3",
			APIGroup: "TestGroup2",
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
	}
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(types)
	cases := map[string]bool{
		"TestKind1": true, "TestKind2": true, "TestKind3": true, "testKind": false, "Dummy": false,
	}
	for k, v := range cases {
		r := ValidKind(k)
		if r != v {
			t.Errorf("ValidKind(%v) did not match want[%v] got [%v]", k, v, r)
		}
	}

	cases = map[string]bool{
		"TestGroup1": true, "TestGroup2": true, "UnknownGroup": false, "testGroup1": false,
	}
	for k, v := range cases {
		r := ValidGroup(k)
		if r != v {
			t.Errorf("ValidKind(%v) did not match want[%v] got [%v]", k, v, r)
		}
	}
}
