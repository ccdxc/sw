package errors

import (
	"k8s.io/apimachinery/pkg/api/errors"
	"k8s.io/apimachinery/pkg/runtime/schema"
	"k8s.io/apimachinery/pkg/util/validation/field"
)

// NewNotFound creates a status error with http.StatusNotFound.
func NewNotFound(kind, name string) error {
	return errors.NewNotFound(schema.GroupResource{
		Resource: kind,
	}, name)
}

// NewAlreadyExists creates a status error with http.StatusConflict.
func NewAlreadyExists(kind, name string) error {
	return errors.NewAlreadyExists(schema.GroupResource{
		Resource: kind,
	}, name)
}

// NewUnauthorized creates a status error with http.StatusUnauthorized.
func NewUnauthorized(reason string) error {
	return errors.NewUnauthorized(reason)
}

// NewForbidden creates a status error with http.StatusForbidden.
func NewForbidden(kind, name string, err error) error {
	return errors.NewForbidden(schema.GroupResource{
		Resource: kind,
	}, name, err)
}

// NewConflict creates a status error with http.StatusConflict.
func NewConflict(kind, name string, err error) error {
	return errors.NewConflict(schema.GroupResource{
		Resource: kind,
	}, name, err)
}

// NewGone creates a status error with http.StatusGone.
func NewGone(reason string) error {
	return errors.NewGone(reason)
}

// NewInvalid creates a status error with StatusUnprocessableEntity code.
func NewInvalid(kind, name string, errs field.ErrorList) error {
	return errors.NewInvalid(schema.GroupKind{
		Kind: kind,
	}, name, errs)
}

// NewBadRequest creates a status error with http.StatusBadRequest.
func NewBadRequest(reason string) error {
	return errors.NewBadRequest(reason)
}

// NewServiceUnavailable creates a status error with http.StatusServiceUnavailable.
func NewServiceUnavailable(reason string) error {
	return errors.NewServiceUnavailable(reason)
}

// NewMethodNotSupported creates a status error with http.MethodNotAllowed.
func NewMethodNotSupported(kind, action string) error {
	return errors.NewMethodNotSupported(schema.GroupResource{
		Resource: kind,
	}, action)
}

// NewServerTimeout creates a status error with http.StatusInternalServerError indicating the requested action
// could not be completed due to a transient error, and the client should try again.
func NewServerTimeout(kind, operation string, retryAfterSeconds int) error {
	return errors.NewServerTimeout(schema.GroupResource{
		Resource: kind,
	}, operation, retryAfterSeconds)
}

// NewInternalError creates a status error with http.StatusInternalServerError.
func NewInternalError(err error) error {
	return errors.NewInternalError(err)
}

// NewTimeoutError creates a status error with StatusServerTimeout indicating that a timeout occurred before
// the request could be completed.  Clients may retry, but the operation may still complete.
func NewTimeoutError(message string, retryAfterSeconds int) error {
	return errors.NewTimeoutError(message, retryAfterSeconds)
}

// NewGenericServerResponse returns a new error for server responses that are not in a recognizable form.
func NewGenericServerResponse(code int, verb, kind, name, serverMessage string, retryAfterSeconds int, isUnexpectedResponse bool) error {
	return errors.NewGenericServerResponse(code, verb, schema.GroupResource{
		Resource: kind,
	}, name, serverMessage, retryAfterSeconds, isUnexpectedResponse)
}
