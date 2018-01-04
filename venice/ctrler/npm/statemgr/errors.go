package statemgr

import "errors"

var (
	// ErrIncorrectObjectType is returned when type deferencing from memdb.Object is of invalid type
	ErrIncorrectObjectType = errors.New("incorrect object type")

	// ErrEndpointNotFound is returned when an endpoint is not found
	ErrEndpointNotFound = errors.New("could not find the endpoint")

	// ErrTenantNotFound is returned when a tenant is not found
	ErrTenantNotFound = errors.New("could not find the tenant")

	// ErrDefaultTenantDeleteNotPermitted is returned when default tenant deletion is attempted
	ErrDefaultTenantDeleteNotPermitted = errors.New("deleting default tenant is not allowed")
)
