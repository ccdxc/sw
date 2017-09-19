package errors

import (
	"encoding/json"
	"net/http"

	"k8s.io/apimachinery/pkg/util/validation/field"
)

// SendNotFound sends a status error with http.StatusNotFound.
func SendNotFound(w http.ResponseWriter, kind, name string) error {
	return json.NewEncoder(w).Encode(NewNotFound(kind, name))
}

// SendAlreadyExists sends a status error with http.StatusConflict.
func SendAlreadyExists(w http.ResponseWriter, kind, name string) error {
	return json.NewEncoder(w).Encode(NewAlreadyExists(kind, name))
}

// SendUnauthorized sends a status error with http.StatusUnauthorized.
func SendUnauthorized(w http.ResponseWriter, reason string) error {
	return json.NewEncoder(w).Encode(NewUnauthorized(reason))
}

// SendForbidden sends a status error with http.StatusForbidden.
func SendForbidden(w http.ResponseWriter, kind, name string, err error) error {
	return json.NewEncoder(w).Encode(NewForbidden(kind, name, err))
}

// SendConflict sends a status error with http.StatusConflict.
func SendConflict(w http.ResponseWriter, kind, name string, err error) error {
	return json.NewEncoder(w).Encode(NewConflict(kind, name, err))
}

// SendGone sends a status error with http.StatusGone.
func SendGone(w http.ResponseWriter, reason string) error {
	return json.NewEncoder(w).Encode(NewGone(reason))
}

// SendInvalid sends a status error with StatusUnprocessableEntity code.
func SendInvalid(w http.ResponseWriter, kind, name string, errs field.ErrorList) error {
	return json.NewEncoder(w).Encode(NewInvalid(kind, name, errs))
}

// SendBadRequest sends a status error with http.StatusBadRequest.
func SendBadRequest(w http.ResponseWriter, reason string) error {
	return json.NewEncoder(w).Encode(NewBadRequest(reason))
}

// SendServiceUnavailable sends a status error with http.StatusServiceUnavailable.
func SendServiceUnavailable(w http.ResponseWriter, reason string) error {
	return json.NewEncoder(w).Encode(NewServiceUnavailable(reason))
}

// SendMethodNotSupported sends a status error with http.MethodNotAllowed.
func SendMethodNotSupported(w http.ResponseWriter, kind, action string) error {
	return json.NewEncoder(w).Encode(NewMethodNotSupported(kind, action))
}

// SendServerTimeout sends a status error with http.StatusInternalServerError indicating the requested action
// could not be completed due to a transient error, and the client should try again.
func SendServerTimeout(w http.ResponseWriter, kind, operation string, retryAfterSeconds int) error {
	return json.NewEncoder(w).Encode(NewServerTimeout(kind, operation, retryAfterSeconds))
}

// SendInternalError sends a status error with http.StatusInternalServerError.
func SendInternalError(w http.ResponseWriter, err error) error {
	return json.NewEncoder(w).Encode(NewInternalError(err))
}

// SendTimeoutError sends a status error with StatusServerTimeout indicating that a timeout occurred before
// the request could be completed.  Clients may retry, but the operation may still complete.
func SendTimeoutError(w http.ResponseWriter, message string, retryAfterSeconds int) error {
	return json.NewEncoder(w).Encode(NewTimeoutError(message, retryAfterSeconds))
}

// SendGenericServerResponse returns a new error for server responses that are not in a recognizable form.
func SendGenericServerResponse(w http.ResponseWriter, code int, verb, kind, name, serverMessage string, retryAfterSeconds int, isUnexpectedResponse bool) error {
	return json.NewEncoder(w).Encode(NewGenericServerResponse(code, verb, kind, name, serverMessage, retryAfterSeconds, isUnexpectedResponse))
}
