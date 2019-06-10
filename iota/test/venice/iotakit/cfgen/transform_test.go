package cfgen

import (
	"strings"
	"testing"
)

func TestStringSub(t *testing.T) {

	testSamples := []struct {
		input    string
		expected string
	}{
		{input: "{{var=var11}}name-{{iter}}", expected: "name-0"},
		{input: "tcp/{{rand}}", expected: "tcp/"},
		{input: "udp/{{rand:1000-4000}}", expected: "udp/"},
		{input: "{{iter1:tcp|udp}}/{{rand:5-50}}", expected: "tcp/"},
		{input: "{{iter1:tcp|udp}}/{{rand:5-50}}", expected: "udp/"},
		{input: "tcp/{{iter3:32-500}}", expected: "tcp/32"},
		{input: "tcp/{{iter3:32-500}}", expected: "tcp/33"},
		{input: "name-{{iter}}", expected: "name-1"},
		{input: "{{$var11}}", expected: "name-0"},
		{input: "{{ipv4-srcip:11.2.x.x}}", expected: "11.2.0.1"},
		{input: "{{ipv4-srcip:11.2.x.x}}", expected: "11.2.0.2"},
		{input: "{{ipv4-dstip:11.2.x.x}}", expected: "11.2.0.1"},
		{input: "{{mac}}", expected: "00ae.dd00.0001"},
		{input: "{{mac-primary}}", expected: "00ae.dd00.0001"},
		{input: "{{mac-primary}}", expected: "00ae.dd00.0002"},
		{input: "{{mac-newoui:00.44.55}}", expected: "0044.5500.0001"},
	}
	iterCtx := newIterContext()
	for idx, testSample := range testSamples {
		tString := iterCtx.transformString(testSample.input)
		if !strings.Contains(tString, testSample.expected) {
			t.Fatalf("[%d] template --%s-- transformed --%s-- expected --%s--\n",
				idx, testSample.input, tString, testSample.expected)
		}
	}
}

func TestIntSub(t *testing.T) {
	testSamples := []struct {
		input  uint64
		expMax uint64
		expMin uint64
	}{
		{input: 9999, expMin: 1001, expMax: 9999},
		{input: 99, expMin: 11, expMax: 99},
		{input: 9, expMin: 1, expMax: 9},
		{input: 8, expMin: 1, expMax: 1},
		{input: 8, expMin: 2, expMax: 2},
		{input: 8, expMin: 3, expMax: 3},
	}

	iterCtx := newIterContext()

	for idx, testSample := range testSamples {
		tInt := iterCtx.transformUint(testSample.input)
		if tInt > testSample.expMax || tInt < testSample.expMin {
			t.Fatalf("[%d] template %d transformed %d expected %d-%d\n",
				idx, testSample.input, tInt, testSample.expMin, testSample.expMax)
		}
	}

}
