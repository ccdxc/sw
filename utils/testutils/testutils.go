// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package testutils

import (
	"path/filepath"
	"reflect"
	"runtime"
	"time"

	"github.com/Sirupsen/logrus"
)

// Evaluator prototype for eveluator function
type Evaluator func() bool

// TB is common T,B interface
type TB interface {
	Error(args ...interface{})
	Errorf(format string, args ...interface{})
	Fail()
	FailNow()
	Failed() bool
	Fatal(args ...interface{})
	Fatalf(format string, args ...interface{})
	Log(args ...interface{})
	Logf(format string, args ...interface{})
}

// Assert fails the test if the condition is false.
func Assert(tb TB, condition bool, msg string, v ...interface{}) {
	if !condition {
		_, file, line, _ := runtime.Caller(1)
		tb.Fatalf("\033[31m%s:%d: "+msg+"\033[39m\n\n", append([]interface{}{filepath.Base(file), line}, v...)...)
	}
}

// AssertOk fails the test if an err is not nil.
func AssertOk(tb TB, err error, msg string) {
	if err != nil {
		_, file, line, _ := runtime.Caller(1)
		tb.Fatalf("\033[31m%s:%d: %s. unexpected error: %s\033[39m\n\n", filepath.Base(file), line, msg, err.Error())
		tb.FailNow()
	}
}

// AssertEquals fails the test if exp is not equal to act.
func AssertEquals(tb TB, exp, act interface{}, msg string) {
	if !reflect.DeepEqual(exp, act) {
		_, file, line, _ := runtime.Caller(1)
		tb.Fatalf("\033[31m%s:%d:\n\n\texp: %#v\n\n\tgot: %#v\033[39m\n\n", filepath.Base(file), line, exp, act)
	}
}

// AssertEventually polls evaluator periodically and checks if it reached desired condition
// intervals are pollInterval followed by timeoutInterval in time.ParseDuration() format
func AssertEventually(tb TB, eval Evaluator, msg string, intervals ...string) {
	var err error
	pollInterval := time.Millisecond
	timeoutInterval := time.Second

	// parse intervals
	if len(intervals) > 0 {
		pollInterval, err = time.ParseDuration(intervals[0])
		if err != nil {
			tb.Fatalf("%#v is not a valid parsable duration string.", intervals[0])
		}
	}
	if len(intervals) > 1 {
		timeoutInterval, err = time.ParseDuration(intervals[1])
		if err != nil {
			tb.Fatalf("%#v is not a valid parsable duration string.", intervals[1])
		}
	}

	timer := time.Now()
	timeout := time.After(timeoutInterval)

	// loop till we reach timeout interval
	for {
		select {
		case <-timeout:
			// evaluate one last time
			if eval() {
				tb.Logf("Evaluator suceeded after %v", time.Since(timer))
				return
			}
			logrus.Errorf("Evaluator timed out after %v", time.Since(timer))
			_, file, line, _ := runtime.Caller(1)
			tb.Fatalf("\033[31m%s:%d: "+msg+"\033[39m\n\n", append([]interface{}{filepath.Base(file), line})...)
		case <-time.After(pollInterval):
			if eval() {
				tb.Logf("Evaluator suceeded after %v", time.Since(timer))
				return
			}
		}
	}
}

// AssertConsistently polls evaluator periodically and checks that the condition
// specified continuously matches until the timeout. intervals are pollInterval
// followed by timeoutInterval in time.ParseDuration() format.
func AssertConsistently(tb TB, eval Evaluator, msg string, intervals ...string) {
	var err error
	pollInterval := time.Millisecond
	timeoutInterval := time.Second

	// parse intervals
	if len(intervals) > 0 {
		pollInterval, err = time.ParseDuration(intervals[0])
		if err != nil {
			tb.Fatalf("%#v is not a valid parsable duration string.", intervals[0])
		}
	}
	if len(intervals) > 1 {
		timeoutInterval, err = time.ParseDuration(intervals[1])
		if err != nil {
			tb.Fatalf("%#v is not a valid parsable duration string.", intervals[1])
		}
	}

	_, file, line, _ := runtime.Caller(1)
	timer := time.Now()
	timeout := time.After(timeoutInterval)

	// loop till we reach timeout interval
	for {
		select {
		case <-timeout:
			// evaluate one last time
			if eval() {
				tb.Logf("Evaluator passed after %v", time.Since(timer))
				return
			}
			logrus.Errorf("Evaluator failed after %v", time.Since(timer))
			tb.Fatalf("\033[31m%s:%d: "+msg+"\033[39m\n\n", append([]interface{}{filepath.Base(file), line})...)
		case <-time.After(pollInterval):
			if !eval() {
				logrus.Errorf("Evaluator failed after %v", time.Since(timer))
				tb.Fatalf("\033[31m%s:%d: "+msg+"\033[39m\n\n", append([]interface{}{filepath.Base(file), line})...)
				return
			}
		}

	}
}

// CheckEventually polls eveluator periodically and checks if it reached desired condition
// intervals are pollInterval followed by timeoutInterval in time.ParseDuration() format
// returns true if desired state is reached
func CheckEventually(eval Evaluator, intervals ...string) bool {
	var err error
	pollInterval := time.Millisecond
	timeoutInterval := time.Second

	// parse intervals
	if len(intervals) > 0 {
		pollInterval, err = time.ParseDuration(intervals[0])
		if err != nil {
			logrus.Fatalf("%#v is not a valid parsable duration string.", intervals[0])
		}
	}
	if len(intervals) > 1 {
		timeoutInterval, err = time.ParseDuration(intervals[1])
		if err != nil {
			logrus.Fatalf("%#v is not a valid parsable duration string.", intervals[1])
		}
	}

	timer := time.Now()
	timeout := time.After(timeoutInterval)

	// loop till we reach timeout interval
	for {
		select {
		case <-time.After(pollInterval):
			if eval() {
				return true
			}
		case <-timeout:
			// eveluate one last time
			if !eval() {
				_, file, line, _ := runtime.Caller(1)
				logrus.Errorf("%s:%d: Evaluator timed out after %v", filepath.Base(file), line, time.Since(timer))
				return false
			}

			return true
		}
	}
}
