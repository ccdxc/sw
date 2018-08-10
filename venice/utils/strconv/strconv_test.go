package strconv

import (
	"fmt"
	"math"
	"sync"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"
)

func TestStrToFloat(t *testing.T) {
	tests := []struct {
		input  string
		expOut float64
	}{
		{input: "54", expOut: 54},
		{input: "100000000", expOut: 100000000},
		{input: "5.4", expOut: 5.4},
		{input: "0.0004", expOut: 0.0004},
		{input: "-45", expOut: -45},
		{input: "-1.2", expOut: -1.2},
		{input: "-0.00008", expOut: -0.00008},
		{input: "+45", expOut: 45},
		{input: "+1.2", expOut: 1.2},
		{input: "+0.00008", expOut: 0.00008},
		{input: fmt.Sprintf("%v", math.MaxFloat64), expOut: math.MaxFloat64},
		{input: fmt.Sprintf("%v", math.MaxInt64), expOut: math.MaxInt64},
		{input: fmt.Sprintf("%v", uint32(math.MaxUint32)), expOut: math.MaxUint32},
		{input: fmt.Sprintf("%v", uint64(math.MaxUint64)), expOut: math.MaxUint64},
		{input: "invalid", expOut: 0},
	}

	for _, test := range tests {
		if out, _ := ParseFloat64(test.input); out != test.expOut {
			t.Fatalf("failed to convert value(%v) to float64, expected: %v, obtained: %v", test.input, test.expOut, out)
		}
	}
}

func TestStrToTime(t *testing.T) {
	timestampBefore2m, _ := types.TimestampProto(time.Now().Add(-2 * time.Minute))
	timestampNow, _ := types.TimestampProto(time.Now())
	timestampAfter2m, _ := types.TimestampProto(time.Now().Add(2 * time.Minute))

	// compute time.Time from timestamp
	timeBefore2m, _ := types.TimestampFromProto(timestampBefore2m)
	timeNow, _ := types.TimestampFromProto(timestampNow)
	timeAfter2m, _ := types.TimestampFromProto(timestampAfter2m)

	tests := []struct {
		input  string
		expOut time.Time
	}{
		{input: timestampBefore2m.String(), expOut: timeBefore2m},
		{input: timestampNow.String(), expOut: timeNow},
		{input: timestampAfter2m.String(), expOut: timeAfter2m},
		{input: "invalid", expOut: *(new(time.Time))},
		{input: "12345:567:89", expOut: *(new(time.Time))},
	}

	for _, test := range tests {
		if out, _ := ParseTime(test.input); !out.Equal(test.expOut) {
			t.Fatalf("failed to convert value(%v) to time.Time, expected: %v, obtained: %v", test.input, test.expOut, out)
		}
	}
}

func TestMustInt32(t *testing.T) {
	tests := []struct {
		input  interface{}
		expOut int64
	}{
		{input: "67", expOut: 67},
		{input: fmt.Sprintf("%v", math.MaxInt64), expOut: math.MaxInt64},
		{input: 567, expOut: 567},
		{input: int64(54), expOut: 54},
		{input: int(54), expOut: 54},
		{input: math.MinInt64, expOut: math.MinInt64},
		{input: math.MaxInt64, expOut: math.MaxInt64},
	}

	for _, test := range tests {
		if out := MustInt64(test.input); out != test.expOut {
			t.Fatalf("failed to convert value(%v) to int64, expected: %v, obtained: %v", test.input, test.expOut, out)
		}
	}

	// test -ve cases
	panicCases := []struct {
		input interface{}
	}{
		{input: "test"},
		{input: 56.9},
		{input: int32(54)},
		{input: int16(54)},
		{input: math.MaxFloat64},
	}

	var wg sync.WaitGroup
	wg.Add(len(panicCases))

	for _, test := range panicCases {
		go func(input interface{}) {
			defer func() {
				if r := recover(); r != nil {
					wg.Done()
				}
			}()

			MustInt64(input)
		}(test.input)
	}

	wg.Wait()
}
