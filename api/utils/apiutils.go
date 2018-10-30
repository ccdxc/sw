package apiutils

import "context"

type dryRunMarker struct {
	verVer int64
}

// SetDruRun sets the dry run flag in the the context
func setDryRun(ctx context.Context, val int64) context.Context {
	dval := &dryRunMarker{val}
	return context.WithValue(ctx, dryRunMarker{}, dval)
}

// GetDryRun retrieves the dry run flag from the context
func getDryRun(ctx context.Context) *dryRunMarker {
	ret := ctx.Value(dryRunMarker{})
	if ret == nil {
		return nil
	}
	if dr, ok := ret.(*dryRunMarker); ok {
		return dr
	}
	return nil
}

// IsDryRun returns true if the context is marked for dry run
func IsDryRun(ctx context.Context) bool {
	r := getDryRun(ctx)
	if r == nil {
		return false
	}
	return true
}

// SetVar and GetVar allow for setting arbitrary values in the context
//  - does not create new context for each value added (after the first one)
//  - without having to create a new type to be used as key for new values
// Should be used on only ephemeral contexts. Using it on long standing contexts
//  should be considered carefully.
// NOT CONCURRENCY SAFE

type ctxVal struct{}

// SetVar sets a arbitrary key Value in the context
func SetVar(ctx context.Context, key string, val interface{}) context.Context {
	v := ctx.Value(ctxVal{})
	if v == nil {
		m := make(map[string]interface{})
		m[key] = val
		return context.WithValue(ctx, ctxVal{}, m)
	}
	m := v.(map[string]interface{})
	m[key] = val
	return ctx
}

// GetVar gets the value for key if set in ctxVal
func GetVar(ctx context.Context, key string) (interface{}, bool) {
	v := ctx.Value(ctxVal{})
	if v != nil {
		m := v.(map[string]interface{})
		val, ok := m[key]
		return val, ok
	}
	return nil, false
}
