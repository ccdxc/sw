// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package gometrics

import (
	"encoding/binary"

	"github.com/pensando/sw/venice/utils/log"
)

// Metrics is the interface all meterics objects have to implement
type Metrics interface {
	GetKey() []byte                      // returns the key for the metrics
	String() string                      // return json string
	GetCounter(offset int) Counter       // get a counter attribute
	GetGauge(offset int) Gauge           // get gauge attribute
	SetCounter(cntr Counter, offset int) // set counter value
	SetGauge(val Gauge, offset int)      // set gauge value
}

// MetricsIterator can iterate over a kind of metrics objects
type MetricsIterator interface {
	HasNext() bool                      // does iterator has next value?
	Next() Metrics                      // get the next value
	Find(key []byte) (Metrics, error)   // get a specific metrics entry
	Create(key []byte, len int) Metrics // create a metrics entry
	Delete(key []byte) error            // delete a metrics entry
}

// type definitions
type Counter uint64
type Gauge float64

// Size returns len of counter
func (c Counter) Size() int {
	return 8
}

// Size returns byte length of gauge
func (g Gauge) Size() int {
	return 8
}

// NewMetricsIterator returns new metrics iterator for a kind
func NewMetricsIterator(kind string) (MetricsIterator, error) {
	return newMetricsIterator(kind)
}

// EncodeScalarKey encodes scalar key into bytes
func EncodeScalarKey(key interface{}) []byte {
	switch t := key.(type) {
	case uint8:
		buf := make([]byte, 1)
		buf[0] = key.(uint8)
		return buf
	case int8:
		buf := make([]byte, 1)
		buf[0] = key.(uint8)
		return buf
	case uint16:
		buf := make([]byte, 2)
		binary.LittleEndian.PutUint16(buf[0:], key.(uint16))
		return buf
	case int16:
		buf := make([]byte, 2)
		binary.LittleEndian.PutUint16(buf[0:], key.(uint16))
		return buf
	case uint32:
		buf := make([]byte, 4)
		binary.LittleEndian.PutUint32(buf[0:], key.(uint32))
		return buf
	case int32:
		buf := make([]byte, 4)
		binary.LittleEndian.PutUint32(buf[0:], key.(uint32))
		return buf
	case uint64:
		buf := make([]byte, 8)
		binary.LittleEndian.PutUint64(buf[0:], key.(uint64))
		return buf
	case int64:
		buf := make([]byte, 8)
		binary.LittleEndian.PutUint64(buf[0:], key.(uint64))
		return buf
	case string:
		return []byte(key.(string))
	default:
		log.Errorf("Unknown type %v", t)
		return nil
	}
}

// DecodeScalarKey decodes bytes into scalar type
func DecodeScalarKey(key interface{}, buf []byte) {
	switch t := key.(type) {
	case *uint8:
		*key.(*uint8) = buf[0]
	case *int8:
		*key.(*uint8) = buf[0]
	case *uint16:
		*key.(*uint16) = binary.LittleEndian.Uint16(buf[0:])
	case *int16:
		*key.(*int16) = int16(binary.LittleEndian.Uint16(buf[0:]))
	case *uint32:
		*key.(*uint32) = binary.LittleEndian.Uint32(buf[0:])
	case *int32:
		*key.(*int32) = int32(binary.LittleEndian.Uint32(buf[0:]))
	case *uint64:
		*key.(*uint64) = binary.LittleEndian.Uint64(buf[0:])
	case *int64:
		*key.(*int64) = int64(binary.LittleEndian.Uint64(buf[0:]))
	case *string:
		*key.(*string) = string(buf)
	default:
		log.Errorf("Unknown type %v", t)
	}
}
