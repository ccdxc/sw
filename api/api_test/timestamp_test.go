package apitest

import (
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	timingMargin = 2 * time.Second
)

var tZero time.Time
var errNeedErr = fmt.Errorf("Expected parse error")

var testCases = []struct {
	timeStr string
	refStr  string
	offset  time.Duration
	add     bool
	expErr  bool
}{
	{`now() - 1h`, "", time.Hour, false, false},
	{`now() + 2h`, "", 2 * time.Hour, true, false},
	{`now() - 50m`, "", 50 * time.Minute, false, false},
	{`now() + 15m`, "", 15 * time.Minute, true, false},
	{`now()`, "", 0, true, false},
	{`2006-01-02T15:04:05.999999999+03:00 - 15m`, `2006-01-02T15:04:05.999999999+03:00`, 15 * time.Minute, false, false},
	{`2006-01-02T15:04:05.999999999+03:00 + 5h`, `2006-01-02T15:04:05.999999999+03:00`, 5 * time.Hour, true, false},
	{`2006-01-02T15:04:05Z + 20s`, `2006-01-02T15:04:05Z`, 20 * time.Second, true, false},
	{`2006-01-02T15:04:05Z`, `2006-01-02T15:04:05Z`, 0, true, false},
	{`2006-01-02T15:04:05.999999999+03:00`, `2006-01-02T15:04:05.999999999+03:00`, 0, true, false},
	{`2006 01-02T15:04:05.999999999+03:00`, `2006-01-02T15:04:05.999999999+03:00`, 0, true, true},
	{`now() - 1h + 2h`, "", time.Hour, true, true},
	{`now() - 2*2h`, "", time.Hour, false, true},
	{`then() - 1h`, "", time.Hour, false, true},
	{`2006:01-02T15:04:05.999999999+03:00`, `2006-01-02T15:04:05.999999999+03:00`, 0, true, true},
	{`2006/01/02T15:04:05.999999999+03:00`, `2006-01-02T15:04:05.999999999+03:00`, 0, true, true},
	{`2006-01-02T15:04:05.999999999+03:00+00:10`, `2006-01-02T15:04:05.999999999+03:00`, 0, true, true},
}

func verifyRelative(ref, result time.Time, offset time.Duration, add bool) error {
	var delta time.Duration

	// value of zero for ref means now.
	if ref.IsZero() {
		ref = time.Now()
	}

	tRes := result.UTC()
	tRef := ref.UTC()
	if add {
		delta = tRes.Sub(tRef)
	} else {
		delta = tRef.Sub(tRes)
	}

	if delta.Round(timingMargin) == offset.Round(timingMargin) {
		return nil
	}

	return fmt.Errorf("Times did not match delta: %v offset: %v", delta.Round(timingMargin), offset.Round(timingMargin))
}
func TestTimestamp(t *testing.T) {
	var ref time.Time
	var err error

	for id, tc := range testCases {
		if tc.refStr == "" {
			ref = tZero
		} else {
			ref, err = time.Parse(time.RFC3339Nano, tc.refStr)
			AssertOk(t, err, "Test case definition error")
		}

		ts := &api.Timestamp{}
		err = ts.Parse(tc.timeStr)
		if tc.expErr && err == nil {
			AssertOk(t, errNeedErr, "tc %d:  %s Expected parsing error", id, tc.timeStr)

		}

		if tc.expErr {
			continue
		}

		AssertOk(t, err, "tc %d:  %s Expected no parsing error", id, tc.timeStr)
		tt, err := ts.Time()
		AssertOk(t, err, "timestamp parsing error")
		AssertOk(t, verifyRelative(ref, tt, tc.offset, tc.add), "Verify failed tc %d: %s", id, tc.timeStr)
	}
}
