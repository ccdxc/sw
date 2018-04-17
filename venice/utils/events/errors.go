// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package events

import "fmt"

// error codes
const (
	ErrFlushFailed int = iota + 1
	ErrInvalidEventType
	ErrInvalidSeverity
	ErrMissingEventAttributes
)

// error code map with the description
var errors = map[int]string{
	ErrFlushFailed:            "Flush operation failed",
	ErrInvalidEventType:       "Invalid event type",
	ErrInvalidSeverity:        "Invalid event severity",
	ErrMissingEventAttributes: "Missing event attributes",
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

// IsMissingEventAttributes returns true if the error code is ErrMissingEventAttributes.
func IsMissingEventAttributes(err error) bool {
	if cErr, ok := err.(*Error); ok {
		return cErr.Code == ErrMissingEventAttributes
	}

	return false
}
