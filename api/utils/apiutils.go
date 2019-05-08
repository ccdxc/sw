package apiutils

import (
	"context"
	"errors"
	"fmt"
	"strconv"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/runtime"
)

type dryRunMarker struct {
	verVer int64
}

// Constants for use with SetVar() and GetVar()
const (
	CtxKeyObjKind             = "ObjKind"
	CtxKeyAPIGwHTTPReq        = "ApiGwHttpReq"
	CtxKeyAPIGwHTTPWriter     = "ApiGwHttpWriter"
	CtxKeyAPIGwWebSocketWatch = "ApiGwWebSocketWatch"
	CtxKeyAPIGwWebSocketConn  = "ApiGwWebSocketConn"
	CtxKeyAPIGwBinStreamReq   = "CtxKeyAPIGwBinStreamReq"
	CtxKeyAPISrvInitRestore   = "InitRestorePath"
)

type requirementSetMarker struct {
	val interface{}
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

// GetQueryStringFromListWatchOptions returns a query string given ListWatchOptions
func GetQueryStringFromListWatchOptions(in *api.ListWatchOptions) string {
	var params []string
	if in.Name != "" {
		params = append(params, fmt.Sprintf("name=%s", in.Name))
	}
	if in.Tenant != "" {
		params = append(params, fmt.Sprintf("tenant=%s", in.Tenant))
	}
	if in.Namespace != "" {
		params = append(params, fmt.Sprintf("namespace=%s", in.Namespace))
	}
	if in.LabelSelector != "" {
		params = append(params, fmt.Sprintf("label-selector=%s", in.LabelSelector))
	}
	if in.FieldSelector != "" {
		params = append(params, fmt.Sprintf("field-selector=%s", in.FieldSelector))
	}
	if len(in.FieldChangeSelector) != 0 {
		for _, v := range in.FieldChangeSelector {
			if v != "" {
				params = append(params, fmt.Sprintf("field-change-selector=%s", v))
			}
		}
	}
	if in.From != 0 {
		params = append(params, fmt.Sprintf("from=%d", in.From))
	}
	if in.MaxResults != 0 {
		params = append(params, fmt.Sprintf("max-results=%d", in.MaxResults))
	}
	if len(params) > 0 {
		return strings.Join(params, "&")
	}
	return ""
}

// SetRequirements annotates the context with a requirement set
func SetRequirements(ctx context.Context, in interface{}) context.Context {
	return context.WithValue(ctx, requirementSetMarker{}, &requirementSetMarker{in})
}

// GetRequirements retrieves the requirements set if present
func GetRequirements(ctx context.Context) (interface{}, error) {
	ret := ctx.Value(requirementSetMarker{})
	if ret == nil {
		return nil, errors.New("not found")
	}
	if r, ok := ret.(*requirementSetMarker); ok {
		return r.val, nil
	}
	return nil, errors.New("invalid type")
}

// MustGetObjectMetaVersion retrieves the resource version from object meta. Panics on failure.
func MustGetObjectMetaVersion(obj runtime.Object) (*api.ObjectMeta, uint64) {
	objm := obj.(runtime.ObjectMetaAccessor)
	meta := objm.GetObjectMeta()
	v, err := strconv.ParseUint(meta.ResourceVersion, 10, 64)
	if err != nil {
		panic(fmt.Sprintf("unable to parse version string [%s](%s)", meta.ResourceVersion, err))
	}
	return meta, v
}

// IsSecretsReaderCtx returns true if the client context is allowed to access secrets
func IsSecretsReaderCtx(ctx context.Context) bool {
	return !ctxutils.IsAPIGwCtx(ctx)
}

// IsEventsReaderCtx returns true if the client context is allowed to access events
func IsEventsReaderCtx(ctx context.Context) bool {
	return ctxutils.IsAPIGwCtx(ctx)
}

// IsAuditsReaderCtx returns true if the client context is allowed to access audits
func IsAuditsReaderCtx(ctx context.Context) bool {
	return ctxutils.IsAPIGwCtx(ctx)
}
