package strconv

import (
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestMacString(t *testing.T) {
	good := []struct {
		in  string
		out string
	}{
		{"01:23:45:67:89:AB", "0123.4567.89ab"},
		{"ab:cd:ef:AB:CD:EF", "abcd.efab.cdef"},
		{"01-23-45-67-89-AB", "0123.4567.89ab"},
		{"01-23-45-67-89-aB", "0123.4567.89ab"},
		{"0123.4567.ABCD", "0123.4567.abcd"},
		{"0123.4567.ABcd", "0123.4567.abcd"},
	}
	bad := []string{
		"01.02.03.04.05.06",
		"01:02:03:04:05:06:",
		"x1:02:03:04:05:06",
		"01002:03:04:05:06",
		"01:02003:04:05:06",
		"01:02:03004:05:06",
		"01:02:03:04005:06",
		"01:02:03:04:05006",
		"01-02:03:04:05:06",
		"01:02-03-04-05-06",
		"0123:4567:89AF",
		"0123-4567-89AF",
	}
	for _, c := range good {
		out, err := ParseMacAddr(c.in)
		AssertOk(t, err, "Failed to ParseMacAddr [%v]", c.in)
		Assert(t, out == c.out, "output does not match got[%v] want[%v]", out, c.out)
	}
	for _, c := range bad {
		_, err := ParseMacAddr(c)
		Assert(t, err != nil, "Succeeded but should have failed [%v]", c)
	}
}
