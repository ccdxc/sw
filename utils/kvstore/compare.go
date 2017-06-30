package kvstore

import (
	"strconv"
)

// CmpTarget is used to identify the target for comparison.
type CmpTarget int

// exported constants
const (
	Version CmpTarget = iota
	Value
)

// Cmp defines a single comparison. Multiple Cmps can be used in a single txn.
type Cmp struct {
	// Key contains the key to use for comparison.
	Key string

	// Target specifies what to compare.
	Target CmpTarget

	// Operator specifies the way to compare, "=", "!=", "<" and ">" are supported.
	Operator string

	// Version is used to compare if CmpTarget is Version.
	Version int64

	// Value is used to compare if CmpTarget is Value.
	Value []byte
}

// WithValue creates a Cmp with target type Value.
func WithValue(key string) Cmp {
	return Cmp{Key: key, Target: Value}
}

// WithVersion creates a Cmp with target type Version.
func WithVersion(key string) Cmp {
	return Cmp{Key: key, Target: Version}
}

// Compare updates a Cmp with operator and version or value.
func Compare(cmp Cmp, operator string, v interface{}) Cmp {
	switch operator {
	case "=":
	case "!=":
	case "<":
	case ">":
	default:
		panic("Unknown op")
	}

	cmp.Operator = operator

	switch cmp.Target {
	case Version:
		cmp.Version = mustInt64(v)
	case Value:
		val, ok := v.(string)
		if !ok {
			panic("Bad compare value")
		}
		cmp.Value = []byte(val)
	default:
		panic("Unknown compare target")
	}
	return cmp
}

// mustInt64 converts a string, int64 or int to int64.
func mustInt64(val interface{}) int64 {
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
