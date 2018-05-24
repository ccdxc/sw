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
