package strconv

import (
	"strconv"
	"time"

	"github.com/pensando/sw/api"
)

// ParseFloat64 converts given string to float64
func ParseFloat64(val string) (float64, error) {
	return strconv.ParseFloat(val, 64)
}

// ParseTime converts given string to time.Time
func ParseTime(val string) (time.Time, error) {
	ts := &api.Timestamp{}
	if err := ts.Parse(val); err != nil {
		return time.Time{}, err
	}

	return ts.Time()
}

// MustInt64 converts a string, int64 or int to int64.
// panics if it cannot convert.
func MustInt64(val interface{}) int64 {
	if v, ok := val.(string); ok {
		i, err := strconv.ParseInt(v, 10, 64)
		if err != nil {
			panic("Bad integer")
		}
		return i
	}
	if v, ok := val.(int64); ok {
		return v
	}
	if v, ok := val.(int); ok {
		return int64(v)
	}
	panic("Bad compare value")
}
