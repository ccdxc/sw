// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package utils

import (
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

// IsEmpty checks if the given string is empty
func IsEmpty(str string) bool {
	return len(strings.TrimSpace(str)) == 0
}
