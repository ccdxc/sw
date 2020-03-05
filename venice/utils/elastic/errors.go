// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"fmt"
	"strings"

	es "github.com/olivere/elastic"
)

// error codes
const (
	ErrIndexExists int = iota + 1
	ErrRespNotAcknowledged
	ErrIndexNotExist
	ErrEmptyDocument
	ErrBulkRequestFailed
	ErrInvalidSearchQuery
	ErrInvalidSearchAggregation
	ErrInvalidIndex
)

// error code map with the description
var errors = map[int]string{
	ErrIndexExists:              "Index exists already",
	ErrRespNotAcknowledged:      "Request not acknowledged",
	ErrIndexNotExist:            "Index does not exist",
	ErrEmptyDocument:            "Empty document",
	ErrBulkRequestFailed:        "Bulk request failed",
	ErrInvalidSearchQuery:       "Invalid search query",
	ErrInvalidSearchAggregation: "Invalid search aggregation",
	ErrInvalidIndex:             "Invalid search index",
}

// Error represents the elastic error.
// cause can be an error by itself or anything specific.
type Error struct {
	Code  int
	Msg   string
	Cause string
}

// Error is required for the Error interface
func (err *Error) Error() string {
	return fmt.Sprintf("[%d] %s: %s", err.Code, err.Msg, err.Cause)
}

// NewError creates a new error with the given code and cause.
func NewError(errorCode int, cause string) *Error {
	return &Error{
		Code:  errorCode,
		Msg:   errors[errorCode],
		Cause: cause,
	}
}

// IsIndexExists returns true if the error code is ErrIndexExists.
func IsIndexExists(err error) bool {
	if cErr, ok := err.(*Error); ok {
		return cErr.Code == ErrIndexExists
	}

	return false
}

// IsIndexNotExists returns true if the error code is IsIndexNotExists.
func IsIndexNotExists(err error) bool {
	if cErr, ok := err.(*es.Error); ok {
		return cErr.Status == 404
	}
	return false
}

// IsBulkRequestFailed returns true if the error code is ErrBulkRequestFailed.
func IsBulkRequestFailed(err error) bool {
	if cErr, ok := err.(*Error); ok {
		return cErr.Code == ErrBulkRequestFailed
	}

	return false
}

// IsInvalidSearchQuery returns true if the error code is ErrInvalidSearchQuery.
func IsInvalidSearchQuery(err error) bool {
	if cErr, ok := err.(*Error); ok {
		return cErr.Code == ErrInvalidSearchQuery
	}

	return false
}

// IsConnRefused returns true if the error contains "connection refused" or "EOF" or any error strings
func IsConnRefused(err error) bool {
	errStr := err.Error()
	if strings.Contains(errStr, "connection refused") ||
		strings.Contains(errStr, "EOF") ||
		strings.Contains(errStr, "connection reset") ||
		strings.Contains(errStr, "server closed") ||
		strings.Contains(errStr, "broken pipe") ||
		strings.Contains(errStr, "no available connection") ||
		strings.Contains(errStr, "Service Unavailable") {
		return true
	}

	return false
}
