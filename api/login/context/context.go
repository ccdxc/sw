package context

import "context"

type authzHeaderKey struct{}

// NewContextWithAuthzHeader creates a new context with given Authorization header value (Bearer <token>)
func NewContextWithAuthzHeader(ctx context.Context, token string) context.Context {
	return context.WithValue(ctx, authzHeaderKey{}, token)
}

// AuthzHeaderFromContext returns  Authorization header value from the context
func AuthzHeaderFromContext(ctx context.Context) (string, bool) {
	tok, ok := ctx.Value(authzHeaderKey{}).(string)
	return tok, ok
}

type extRequestIDHeaderKey struct{}

// NewContextWithExtRequestIDHeader creates a new context with given external request ID header value
func NewContextWithExtRequestIDHeader(ctx context.Context, token string) context.Context {
	return context.WithValue(ctx, extRequestIDHeaderKey{}, token)
}

// ExtRequestIDHeaderFromContext returns external request ID header value from the context
func ExtRequestIDHeaderFromContext(ctx context.Context) (string, bool) {
	tok, ok := ctx.Value(extRequestIDHeaderKey{}).(string)
	return tok, ok
}
