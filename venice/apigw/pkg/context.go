package apigwpkg

import (
	"context"
	"errors"

	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
)

var (
	// ErrNoMetadataInContext error is thrown when no metadata in context
	ErrNoMetadataInContext = errors.New("unable to get metadata from context")
	// ErrNoRequestMethodHeader error is thrown when req-method header is not present in metadata
	ErrNoRequestMethodHeader = errors.New("req-method header is not present in metadata")
	// ErrNoUserInContext error is thrown when there is no user present in context
	ErrNoUserInContext = errors.New("user is not present in context")
)

type authzOperationsKey struct{}

type bulkOperationsFlag struct{}

// NewContextWithOperations creates a new context with requested operations
func NewContextWithOperations(ctx context.Context, operations ...authz.Operation) context.Context {
	return context.WithValue(ctx, authzOperationsKey{}, operations)
}

// OperationsFromContext returns requested operations from context
func OperationsFromContext(ctx context.Context) ([]authz.Operation, bool) {
	ops, ok := ctx.Value(authzOperationsKey{}).([]authz.Operation)
	return ops, ok
}

// AddOperationsToContext creates a new context with requested operations added to existing operations in context
func AddOperationsToContext(ctx context.Context, operations ...authz.Operation) context.Context {
	ops, _ := OperationsFromContext(ctx)
	ops = append(ops, operations...)
	return context.WithValue(ctx, authzOperationsKey{}, ops)
}

// AddBulkOperationsFlagToContext creates a new context with the bulkOperationsFlag set in context
func AddBulkOperationsFlagToContext(ctx context.Context) context.Context {
	return context.WithValue(ctx, bulkOperationsFlag{}, true)
}

// BulkOperationsFlagFromContext returns requested bulkOperations flag from context
func BulkOperationsFlagFromContext(ctx context.Context) (bool, bool) {
	present, ok := ctx.Value(bulkOperationsFlag{}).(bool)
	return present, ok
}

// RequestMethodFromContext get request method from the metadata in context
func RequestMethodFromContext(ctx context.Context) (string, error) {
	// get metadata from context
	md, ok := metadata.FromOutgoingContext(ctx)
	if !ok {
		return "", ErrNoMetadataInContext
	}
	// get request method
	reqMethodHeader, ok := md["req-method"]
	if !ok || len(reqMethodHeader) == 0 {
		return "", ErrNoRequestMethodHeader
	}
	return reqMethodHeader[0], nil
}

type userKey struct{}

// NewContextWithUser creates a new context with user
func NewContextWithUser(ctx context.Context, user *auth.User) context.Context {
	return context.WithValue(ctx, userKey{}, user)
}

// UserFromContext returns user from context
func UserFromContext(ctx context.Context) (*auth.User, bool) {
	user, ok := ctx.Value(userKey{}).(*auth.User)
	return user, ok
}
