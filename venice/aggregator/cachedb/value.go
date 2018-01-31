package cachedb

import (
	"fmt"
	"time"

	"github.com/influxdata/influxdb/tsdb"
	"github.com/influxdata/influxql"
)

const (
	// BlockFloat64 designates a block encodes float64 values.
	BlockFloat64 = byte(0)

	// BlockInteger designates a block encodes int64 values.
	BlockInteger = byte(1)

	// BlockBoolean designates a block encodes boolean values.
	BlockBoolean = byte(2)

	// BlockString designates a block encodes string values.
	BlockString = byte(3)

	// BlockUnsigned designates a block encodes uint64 values.
	BlockUnsigned = byte(4)

	// encodedBlockHeaderSize is the size of the header for an encoded block.  There is one
	// byte encoding the type of the block.
	encodedBlockHeaderSize = 1
)

// Value represents a TSM-encoded value.
type Value interface {
	// UnixNano returns the timestamp of the value in nanoseconds since unix epoch.
	UnixNano() int64

	// Value returns the underlying value.
	Value() interface{}

	// Size returns the number of bytes necessary to represent the value and its timestamp.
	Size() int

	// String returns the string representation of the value and its timestamp.
	String() string

	// internalOnly is unexported to ensure implementations of Value
	// can only originate in this package.
	internalOnly()
}

// NewValue returns a new Value with the underlying type dependent on value.
func NewValue(t int64, value interface{}) Value {
	switch v := value.(type) {
	case int64:
		return IntegerValue{unixnano: t, value: v}
	case uint64:
		return UnsignedValue{unixnano: t, value: v}
	case float64:
		return FloatValue{unixnano: t, value: v}
	case bool:
		return BooleanValue{unixnano: t, value: v}
	case string:
		return StringValue{unixnano: t, value: v}
	}
	return EmptyValue{}
}

// NewIntegerValue returns a new integer value.
func NewIntegerValue(t int64, v int64) Value {
	return IntegerValue{unixnano: t, value: v}
}

// NewUnsignedValue returns a new unsigned integer value.
func NewUnsignedValue(t int64, v uint64) Value {
	return UnsignedValue{unixnano: t, value: v}
}

// NewFloatValue returns a new float value.
func NewFloatValue(t int64, v float64) Value {
	return FloatValue{unixnano: t, value: v}
}

// NewBooleanValue returns a new boolean value.
func NewBooleanValue(t int64, v bool) Value {
	return BooleanValue{unixnano: t, value: v}
}

// NewStringValue returns a new string value.
func NewStringValue(t int64, v string) Value {
	return StringValue{unixnano: t, value: v}
}

// EmptyValue is used when there is no appropriate other value.
type EmptyValue struct{}

// UnixNano returns tsdb.EOF.
func (e EmptyValue) UnixNano() int64 { return tsdb.EOF }

// Value returns nil.
func (e EmptyValue) Value() interface{} { return nil }

// Size returns 0.
func (e EmptyValue) Size() int { return 0 }

// String returns the empty string.
func (e EmptyValue) String() string { return "" }

func (e EmptyValue) internalOnly()    {}
func (v StringValue) internalOnly()   {}
func (v IntegerValue) internalOnly()  {}
func (v UnsignedValue) internalOnly() {}
func (v BooleanValue) internalOnly()  {}
func (v FloatValue) internalOnly()    {}

// Encode converts the values to a byte slice.  If there are no values,
// this function panics.
func (a Values) Encode(buf []byte) ([]byte, error) {
	return nil, fmt.Errorf("unsupported value type %T", a[0])
}

// InfluxQLType returns the influxql.DataType the values map to.
func (a Values) InfluxQLType() (influxql.DataType, error) {
	if len(a) == 0 {
		return influxql.Unknown, fmt.Errorf("no values to infer type")
	}

	switch a[0].(type) {
	case FloatValue:
		return influxql.Float, nil
	case IntegerValue:
		return influxql.Integer, nil
	case UnsignedValue:
		return influxql.Unsigned, nil
	case BooleanValue:
		return influxql.Boolean, nil
	case StringValue:
		return influxql.String, nil
	}

	return influxql.Unknown, fmt.Errorf("unsupported value type %T", a[0])
}

// FloatValue represents a float64 value.
type FloatValue struct {
	unixnano int64
	value    float64
}

// UnixNano returns the timestamp of the value.
func (v FloatValue) UnixNano() int64 {
	return v.unixnano
}

// Value returns the underlying float64 value.
func (v FloatValue) Value() interface{} {
	return v.value
}

// Size returns the number of bytes necessary to represent the value and its timestamp.
func (v FloatValue) Size() int {
	return 16
}

// String returns the string representation of the value and its timestamp.
func (v FloatValue) String() string {
	return fmt.Sprintf("%v %v", time.Unix(0, v.unixnano), v.value)
}

// BooleanValue represents a boolean value.
type BooleanValue struct {
	unixnano int64
	value    bool
}

// Size returns the number of bytes necessary to represent the value and its timestamp.
func (v BooleanValue) Size() int {
	return 9
}

// UnixNano returns the timestamp of the value in nanoseconds since unix epoch.
func (v BooleanValue) UnixNano() int64 {
	return v.unixnano
}

// Value returns the underlying boolean value.
func (v BooleanValue) Value() interface{} {
	return v.value
}

// String returns the string representation of the value and its timestamp.
func (v BooleanValue) String() string {
	return fmt.Sprintf("%v %v", time.Unix(0, v.unixnano), v.Value())
}

// IntegerValue represents an int64 value.
type IntegerValue struct {
	unixnano int64
	value    int64
}

// Value returns the underlying int64 value.
func (v IntegerValue) Value() interface{} {
	return v.value
}

// UnixNano returns the timestamp of the value.
func (v IntegerValue) UnixNano() int64 {
	return v.unixnano
}

// Size returns the number of bytes necessary to represent the value and its timestamp.
func (v IntegerValue) Size() int {
	return 16
}

// String returns the string representation of the value and its timestamp.
func (v IntegerValue) String() string {
	return fmt.Sprintf("%v %v", time.Unix(0, v.unixnano), v.Value())
}

// UnsignedValue represents an int64 value.
type UnsignedValue struct {
	unixnano int64
	value    uint64
}

// Value returns the underlying int64 value.
func (v UnsignedValue) Value() interface{} {
	return v.value
}

// UnixNano returns the timestamp of the value.
func (v UnsignedValue) UnixNano() int64 {
	return v.unixnano
}

// Size returns the number of bytes necessary to represent the value and its timestamp.
func (v UnsignedValue) Size() int {
	return 16
}

// String returns the string representation of the value and its timestamp.
func (v UnsignedValue) String() string {
	return fmt.Sprintf("%v %v", time.Unix(0, v.unixnano), v.Value())
}

// StringValue represents a string value.
type StringValue struct {
	unixnano int64
	value    string
}

// Value returns the underlying string value.
func (v StringValue) Value() interface{} {
	return v.value
}

// UnixNano returns the timestamp of the value.
func (v StringValue) UnixNano() int64 {
	return v.unixnano
}

// Size returns the number of bytes necessary to represent the value and its timestamp.
func (v StringValue) Size() int {
	return 8 + len(v.value)
}

// String returns the string representation of the value and its timestamp.
func (v StringValue) String() string {
	return fmt.Sprintf("%v %v", time.Unix(0, v.unixnano), v.Value())
}
