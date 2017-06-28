// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package testutils

import (
	"path/filepath"
	"reflect"
	"runtime"
	"testing"
)

// Assert fails the test if the condition is false.
func Assert(tb testing.TB, condition bool, msg string, v ...interface{}) {
	if !condition {
		_, file, line, _ := runtime.Caller(1)
		tb.Fatalf("\033[31m%s:%d: "+msg+"\033[39m\n\n", append([]interface{}{filepath.Base(file), line}, v...)...)
	}
}

// AssertOk fails the test if an err is not nil.
func AssertOk(tb testing.TB, err error, msg string) {
	if err != nil {
		_, file, line, _ := runtime.Caller(1)
		tb.Fatalf("\033[31m%s:%d: %s. unexpected error: %s\033[39m\n\n", filepath.Base(file), line, msg, err.Error())
		tb.FailNow()
	}
}

// AssertEquals fails the test if exp is not equal to act.
func AssertEquals(tb testing.TB, exp, act interface{}, msg string) {
	if !reflect.DeepEqual(exp, act) {
		_, file, line, _ := runtime.Caller(1)
		tb.Fatalf("\033[31m%s:%d:\n\n\texp: %#v\n\n\tgot: %#v\033[39m\n\n", filepath.Base(file), line, exp, act)
	}
}
