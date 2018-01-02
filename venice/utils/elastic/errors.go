// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package elastic

import "fmt"

// error codes
const (
	ErrIndexExists int = iota + 1
	ErrRespNotAcknowledged
	ErrIndexNotExist
	ErrEmptyDocument
	ErrBulkRequestFailed
	ErrInvalidSearchQuery
)

// error code map with the description
var errors = map[int]string{
	ErrIndexExists:         "Index exists already",
	ErrRespNotAcknowledged: "Request not acknowledged",
	ErrIndexNotExist:       "Index does not exist",
	ErrEmptyDocument:       "Empty document",
	ErrBulkRequestFailed:   "Bulk request failed",
	ErrInvalidSearchQuery:  "Invalid search query",
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
