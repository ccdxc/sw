package impl

import "testing"

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
