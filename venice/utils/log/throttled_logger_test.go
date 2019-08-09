// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//
// This file contains an implementation of a logger paired with a per-message rate-limiter

package log

import (
	"bytes"
	"fmt"
	"math/rand"
	"strconv"
	"testing"
	"time"

	"github.com/go-logfmt/logfmt"
)

var (
	numTestRuns       = 20
	testFlushInterval = 200 * time.Millisecond
	testGetKeyFn      = func(kvs ...interface{}) (string, error) {
		k, err := logfmt.MarshalKeyvals(kvs...)
		return string(k), err
	}
)

func getThresholdLogger(t *testing.T, threshold int, buf *bytes.Buffer) Logger {
	config := GetDefaultConfig("TestLogger")
	config.Filter = AllowAllFilter
	config.ThrottleCfg = &ThrottleConfig{
		ThrottleThreshold: uint64(threshold),
		FlushInterval:     testFlushInterval,
		GetKeyFn:          testGetKeyFn,
	}
	tl := GetNewLogger(config).SetOutput(buf)
	if tl == nil {
		t.Fatalf("Error instantiating throttled logger")
	}
	return tl
}

func getMessageCount(t *testing.T, expLevel, expMsg string, buf *bytes.Buffer) (int, int) {
	var numLogEntries, throttleCount int
	decoder := logfmt.NewDecoder(bytes.NewReader(buf.Bytes()))

	for {
		found := decoder.ScanRecord()
		if found == false {
			// processed all records in the buffer
			break
		}

		var level, msg, throttleCountStr string
		for {
			more := decoder.ScanKeyval()
			if !more {
				// processed all KV pairs in the record
				if level == expLevel {
					if throttleCountStr == "" {
						numLogEntries++
					} else {
						// records that contain throttled message counts are not counted as regular log entries
						tc, err := strconv.Atoi(throttleCountStr)
						if err != nil {
							t.Fatalf("Error parsing throttle count %s, err: %v, buf: %s", throttleCountStr, err, buf.String())
						}
						throttleCount += tc
					}
				}
				break
			}

			switch string(decoder.Key()) {
			case "level":
				if level != "" {
					t.Fatalf("Found multiple level values, buf: %s", buf.String())
				}
				level = string(decoder.Value())
			case "msg":
				if msg != "" {
					t.Fatalf("Found multiple level values, buf: %s", buf.String())
				}
				msg = string(decoder.Value())
				if expMsg != "" && msg != expMsg {
					t.Fatalf("Error decoding log, buf: %s. Have msg: %s, want: %s", buf.String(), msg, expMsg)
				}
			case "throttle_hit_count":
				if throttleCountStr != "" {
					t.Fatalf("Found multiple throttle_hit_count values, buf: %s", buf.String())
				}
				throttleCountStr = string(decoder.Value())
			}
		}
	}

	return numLogEntries, throttleCount
}

// Test with messages that CAN be throttled
func TestThrottleableLogs(t *testing.T) {
	buf := &bytes.Buffer{}
	testMsg := "Hello World"

	// tt = 0 means that every occurence of a message, even the first one, gets throttled
	for _, threshold := range []int{0, 1, 10} {
		tl := getThresholdLogger(t, threshold, buf)

		for i := 0; i < numTestRuns; i++ {
			numLogMessages := rand.Intn(5 * (threshold + 1))
			for j := 0; j < numLogMessages; j++ {
				tl.Infof(testMsg)
			}
			time.Sleep(2 * testFlushInterval)

			numLogEntries, numThrottledMsgs := getMessageCount(t, "info", testMsg, buf)
			if numLogMessages != numLogEntries+numThrottledMsgs {
				t.Fatalf("Unexpected number of log messages. LogEntries: %d, NumThrottledMsgs: %d, Expected Total: %d, buf: %s",
					numLogEntries, numThrottledMsgs, numLogMessages, buf.String())
			}
			buf.Truncate(0)
		}
	}
}

// Test with messages that CANNOT be throttled because they are all different and tt > 0
func TestUnthrottleableLogs(t *testing.T) {
	buf := &bytes.Buffer{}

	for _, threshold := range []int{1, 10} {
		tl := getThresholdLogger(t, threshold, buf)

		for i := 0; i < numTestRuns; i++ {
			numLogMessages := rand.Intn(5 * (threshold + 1))
			for j := 0; j < numLogMessages; j++ {
				testMsg := fmt.Sprintf("HelloWorld%d", j)
				tl.Infof(testMsg)
			}
			time.Sleep(2 * testFlushInterval)

			numLogEntries, numThrottledMsgs := getMessageCount(t, "info", "", buf)
			if numLogEntries != numLogMessages {
				t.Fatalf("Unexpected number of log entries. Have: %d, want: %d", numLogEntries, numLogMessages)
			}
			if numThrottledMsgs != 0 {
				t.Fatalf("Unexpected number of throttled messages. Have: %d, want: %d", numThrottledMsgs, 0)
			}
			buf.Truncate(0)
		}
	}
}

// Check that entries with same message but different severity are throttled separately
func TestThrottleableLeveledLogs(t *testing.T) {
	buf := &bytes.Buffer{}
	testMsg := "Hello World"

	for _, threshold := range []int{0, 1, 10} {
		tl := getThresholdLogger(t, threshold, buf)

		for i := 0; i < numTestRuns; i++ {
			numLogMessages := rand.Intn(5 * (threshold + 1))
			for j := 0; j < numLogMessages; j++ {
				tl.Infof(testMsg)
				tl.Errorf(testMsg)
			}
			time.Sleep(2 * testFlushInterval)

			for _, level := range []string{"info", "error"} {
				numLogEntries, numThrottledMsgs := getMessageCount(t, level, testMsg, buf)
				if numLogMessages != numLogEntries+numThrottledMsgs {
					t.Fatalf("Unexpected number of level %s log messages. LogEntries: %d, NumThrottledMsgs: %d, Expected Total: %d, buf: %s", level, numLogEntries, numThrottledMsgs, numLogMessages, buf.String())
				}
			}
			buf.Truncate(0)
		}
	}
}

func TestConfig(t *testing.T) {
	// Config checks
	defaultLoggerConfig := GetDefaultConfig("test")
	l := GetNewLogger(defaultLoggerConfig)
	tl := newThrottledLogger(&ThrottleConfig{GetKeyFn: testGetKeyFn}, l)
	if tl != nil {
		t.Fatalf("Creation of a throttled logger with FlushInterval = 0 should not succeed")
	}
	tl = newThrottledLogger(&ThrottleConfig{FlushInterval: testFlushInterval}, l)
	if tl != nil {
		t.Fatalf("Creation of a throttled logger with nil GetKeyFn should not succeed")
	}

	// Defaults
	tlc := GetDefaultThrottledLoggerConfig(*defaultLoggerConfig)
	if tlc.ThrottleCfg == nil {
		t.Fatalf("GetDefaultThrottledLoggerConfig did not return a config with throttling")
	}
	if tlc.ThrottleCfg.FlushInterval < time.Second {
		t.Fatalf("GetDefaultThrottledLoggerConfig returned a config with sub-second flush interval")
	}
	if tlc.ThrottleCfg.GetKeyFn == nil {
		t.Fatalf("GetDefaultThrottledLoggerConfig returned a config with nil GetKeyFn")
	}
}
