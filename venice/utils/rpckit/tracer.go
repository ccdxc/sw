// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpckit

import (
	"strings"

	"golang.org/x/net/context"
	"google.golang.org/grpc/metadata"

	opentracing "github.com/opentracing/opentracing-go"
	"github.com/opentracing/opentracing-go/ext"
	"github.com/opentracing/opentracing-go/log"

	logger "github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/trace"
)

// open tracing tag for gRPC
var gRPCComponentTag = opentracing.Tag{Key: string(ext.Component), Value: "gRPC"}

// metadataReaderWriter satisfies both the opentracing.TextMapReader and
// opentracing.TextMapWriter interfaces.
type metadataReaderWriter struct {
	metadata.MD
}

// Set wirte HTTP2 fields
func (w metadataReaderWriter) Set(key, val string) {
	// The GRPC HPACK implementation rejects any uppercase keys here.
	//
	// As such, since the HTTP_HEADERS format is case-insensitive anyway, we
	// blindly lowercase the key (which is guaranteed to work in the
	// Inject/Extract sense per the OpenTracing spec).
	key = strings.ToLower(key)
	w.MD[key] = append(w.MD[key], val)
}

// ForeachKey parses HTTP2 header fields
func (w metadataReaderWriter) ForeachKey(handler func(key, val string) error) error {
	for k, vals := range w.MD {
		for _, v := range vals {
			if dk, dv, err := metadata.DecodeKeyValue(k, v); err == nil {
				if err = handler(dk, dv); err != nil {
					return err
				}
			} else {
				return err
			}
		}
	}

	return nil
}

// tracerMiddleware is a tracer middleware instance
type tracerMiddleware struct {
	tracer opentracing.Tracer // tracer instance
}

// per rpc tracer context
type tracerCtx struct {
	span opentracing.Span // open tracing span
}

// ReqInterceptor implements request interception
func (t *tracerMiddleware) ReqInterceptor(ctx context.Context, role, mysvcName, method string, req interface{}) context.Context {
	if !trace.IsEnabled() {
		return ctx
	}
	tracer := opentracing.GlobalTracer()

	switch role {
	case RoleClient:
		var parentCtx opentracing.SpanContext

		// get parent span if exists
		if parent := opentracing.SpanFromContext(ctx); parent != nil {
			parentCtx = parent.Context()
		}

		// create child span
		clientSpan := tracer.StartSpan(method, opentracing.ChildOf(parentCtx),
			ext.SpanKindRPCClient, gRPCComponentTag)

		// get existing metadata
		md, ok := metadata.FromContext(ctx)
		if !ok {
			md = metadata.New(nil)
		} else {
			md = md.Copy()
		}
		mdWriter := metadataReaderWriter{md}

		// Add custom HTTP2 headers with trace info
		err := tracer.Inject(clientSpan.Context(), opentracing.HTTPHeaders, mdWriter)
		if err != nil {
			clientSpan.LogFields(log.String("event", "Tracer.Inject() failed"), log.Error(err))
			logger.Errorf("Tracer.Inject() failed. Err: %v", err)
		}

		// log request event
		clientSpan.LogFields(log.Object("gRPC request sent", req))

		// save trace context
		ctx = metadata.NewContext(ctx, md)
		trCtx := &tracerCtx{span: clientSpan}
		ctx = context.WithValue(ctx, tracerCtx{}, trCtx)

	case RoleServer:
		// get grpc metadata
		md, ok := metadata.FromContext(ctx)
		if !ok {
			md = metadata.New(nil)
		}

		// get parent span
		spanContext, err := tracer.Extract(opentracing.HTTPHeaders, metadataReaderWriter{md})
		if err != nil && err != opentracing.ErrSpanContextNotFound {
			logger.Errorf("Error getting span context")
			return ctx
		}

		// create new child span
		serverSpan := tracer.StartSpan(method, ext.RPCServerOption(spanContext), gRPCComponentTag)

		// log server request event
		serverSpan.LogFields(log.Object("gRPC request received", req))

		// save tracer context
		ctx = opentracing.ContextWithSpan(ctx, serverSpan)
		trCtx := &tracerCtx{span: serverSpan}
		ctx = context.WithValue(ctx, tracerCtx{}, trCtx)
	}

	return ctx
}

// RespInterceptor implements response interception
func (t *tracerMiddleware) RespInterceptor(ctx context.Context, role, mysvcName, method string, req, reply interface{}, err error) context.Context {
	var msg string

	switch role {
	case RoleClient:
		msg = "gRPC response received"
	case RoleServer:
		msg = "gRPC response sent"
	}

	// get the tracer context
	trCtx, ok := ctx.Value(tracerCtx{}).(*tracerCtx)
	if !ok {
		logger.Errorf("Error getting tracer context.")
		return ctx
	}

	// log response and finish client span
	span := trCtx.span
	defer span.Finish()
	if err == nil {
		span.LogFields(log.Object(msg, reply))
	} else {
		span.LogFields(log.String("event", "gRPC error"), log.Error(err))
		ext.Error.Set(span, true)
	}

	return ctx
}

// create a new tracer middleware
func newTracerMiddleware(svcName string) *tracerMiddleware {
	return &tracerMiddleware{}
}
