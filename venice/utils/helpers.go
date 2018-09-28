// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package utils

import (
	"context"
	"os"
	"strings"
	"time"

	"github.com/pensando/sw/venice/utils/log"
)

// Evaluator function
type Evaluator func() (interface{}, error)

// ExecuteWithRetry call the Evaluator until success or maxRetries
// is met with the specified retry interval
func ExecuteWithRetry(eval Evaluator, retryInterval time.Duration, maxRetries int) (interface{}, error) {

	var result interface{}
	var err error
	retryCount := 0

retryloop:
	for {
		select {
		case <-time.After(retryInterval):
			retryCount++
			result, err = eval()
			if err != nil {
				if retryCount > maxRetries {
					log.Errorf("Retry exhausted, evaluator failed err: %v", err)
					return nil, err
				}
				log.Warnf("Retrying, evaluator failed err: %v", err)
			} else {
				break retryloop
			}
		}
	}
	return result, nil
}

// ExecuteWithContext calls the evaluator (in a separate goroutine) and returns when it completes or
// when the supplied context is cancelled
func ExecuteWithContext(ctx context.Context, eval Evaluator) (interface{}, error) {
	type Result struct {
		I interface{}
		E error
	}
	ch := make(chan Result, 1)
	go func() {
		i, e := eval()
		ch <- Result{I: i, E: e}
		close(ch)
	}()
	select {
	case r := <-ch:
		return r.I, r.E
	case <-ctx.Done():
		return nil, ctx.Err()
	}
}

// IsEmpty checks if the given string is empty
func IsEmpty(str string) bool {
	return len(strings.TrimSpace(str)) == 0
}

// GetHostname helper function to return the hostname
func GetHostname() string {
	hostname, err := os.Hostname()
	if err != nil {
		log.Errorf("failed to get hostname, err: %v", err)
		return ""
	}

	return hostname
}

// CompareTime compares the given time values
// returns
//	0 val1 == val2
//	-1 val1 < val2
//	1 val1 > val2
//
func CompareTime(val1, val2 time.Time) int {
	if val1.Equal(val2) {
		return 0
	} else if val1.Before(val2) {
		return -1
	} else {
		return 1
	}
}

// CompareFloat compares the given CompareFloat values
// returns
//	0 val1 == val2
//	-1 val1 < val2
//	1 val1 > val2
//
func CompareFloat(val1, val2 float64) int {
	if val1 == val2 {
		return 0
	} else if val1 < val2 {
		return -1
	} else {
		return 1
	}
}

// AppendStringIfNotPresent appends a string to a slice if it is not already there
// This is ok for small slices. For big slices, maintain an auxiliary map.
func AppendStringIfNotPresent(s string, ss []string) []string {
	for _, e := range ss {
		if e == s {
			return ss
		}
	}
	return append(ss, s)
}
