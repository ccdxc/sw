package jsonlogger

import (
	"bytes"
	"encoding"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"reflect"

	log "github.com/go-kit/kit/log"
)

type jsonLogger struct {
	io.Writer
}

var desiredOrder = []string{"level", "ts", "module", "caller", "pid"}

type orderedMap map[string]interface{}

// MarshalJSON is tge custom marshaller
func (om orderedMap) MarshalJSON() ([]byte, error) {
	buf := &bytes.Buffer{}
	buf.Write([]byte{'{'})

	// print the keys which  are specified in desiredOrder first
	for _, k := range desiredOrder {
		if om[k] == "" {
			continue
		}
		fmt.Fprintf(buf, " \"%s\": \"%v\"", k, om[k])
		delete(om, k)
		if len(om) > 0 {
			buf.WriteByte(',')
		}
		buf.WriteByte(' ')
	}
	// print all remaining data
	len := len(om)
	index := 0
	for k, v := range om {
		fmt.Fprintf(buf, " \"%s\": \"%v\"", k, v)
		index++
		if index < len {
			buf.WriteByte(',')
		}
		buf.WriteByte(' ')
	}
	buf.Write([]byte{'}', '\n'})
	return buf.Bytes(), nil

}

var errMissingValue = errors.New("(MISSING)")

// NewJSONLogger returns a Logger that encodes keyvals to the Writer as a
// single JSON object. Each log event produces no more than one call to
// w.Write. The passed Writer must be safe for concurrent use by multiple
// goroutines if the returned Logger will be used concurrently.
func NewJSONLogger(w io.Writer) log.Logger {
	return &jsonLogger{w}
}

func (l *jsonLogger) Log(keyvals ...interface{}) error {
	n := (len(keyvals) + 1) / 2 // +1 to handle case when len is odd
	m := make(map[string]interface{}, n)
	for i := 0; i < len(keyvals); i += 2 {
		k := keyvals[i]
		var v interface{} = errMissingValue
		if i+1 < len(keyvals) {
			v = keyvals[i+1]
		}
		merge(m, k, v)
	}
	return json.NewEncoder(l.Writer).Encode(orderedMap(m))
}

func merge(dst map[string]interface{}, k, v interface{}) {
	var key string
	switch x := k.(type) {
	case string:
		key = x
	case fmt.Stringer:
		key = safeString(x)
	default:
		key = fmt.Sprint(x)
	}

	// We want json.Marshaler and encoding.TextMarshaller to take priority over
	// err.Error() and v.String(). But json.Marshall (called later) does that by
	// default so we force a no-op if it's one of those 2 case.
	switch x := v.(type) {
	case json.Marshaler:
	case encoding.TextMarshaler:
	case error:
		v = safeError(x)
	case fmt.Stringer:
		v = safeString(x)
	}

	dst[key] = v
}

func safeString(str fmt.Stringer) (s string) {
	defer func() {
		if panicVal := recover(); panicVal != nil {
			if v := reflect.ValueOf(str); v.Kind() == reflect.Ptr && v.IsNil() {
				s = "NULL"
			} else {
				panic(panicVal)
			}
		}
	}()
	s = str.String()
	return
}

func safeError(err error) (s interface{}) {
	defer func() {
		if panicVal := recover(); panicVal != nil {
			if v := reflect.ValueOf(err); v.Kind() == reflect.Ptr && v.IsNil() {
				s = nil
			} else {
				panic(panicVal)
			}
		}
	}()
	s = err.Error()
	return
}
