package trace

import (
	"context"
	"fmt"
	"net/http"
	"sync"

	"github.com/go-kit/kit/endpoint"
	gokittracing "github.com/go-kit/kit/tracing/opentracing"
	opentracing "github.com/opentracing/opentracing-go"
	zipkin "github.com/openzipkin/zipkin-go-opentracing"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/venice/utils/log"
)

// TODO: read from config file eventually
const zipkinHTTPEndpoint = "http://192.168.30.11:9411/api/v1/spans"

var srvName string
var enable bool
var collector zipkin.Collector
var recorder zipkin.SpanRecorder
var tracer opentracing.Tracer
var lock sync.Mutex

// Init tracing with the caller (service) name
func Init(serviceName string) {
	srvName = serviceName
	EnableOpenTrace()
}

// DisableOpenTraceHdlr disables opentracing compatible tracer for the caller service
func DisableOpenTraceHdlr(w http.ResponseWriter, r *http.Request) {
	DisableOpenTrace()
}

// DisableOpenTrace disables opentracing compatible tracer for the caller service
func DisableOpenTrace() {
	lock.Lock()
	defer lock.Unlock()
	opentracing.SetGlobalTracer(opentracing.NoopTracer{})
	enable = false
	if collector != nil {
		collector.Close()
		collector = nil
	}
}

// EnableOpenTraceHdlr enables opentracing compatible tracer for the caller service
func EnableOpenTraceHdlr(w http.ResponseWriter, r *http.Request) {
	EnableOpenTrace()
}

// EnableOpenTrace enables opentracing compatible tracer for the caller service
func EnableOpenTrace() {
	lock.Lock()
	defer lock.Unlock()
	var err error
	{
		collector, err = zipkin.NewHTTPCollector(zipkinHTTPEndpoint)
		if err != nil {
			fmt.Printf("unable to create Zipkin HTTP collector: %v", err)
			return
		}
		// create recorder.
		recorder = zipkin.NewRecorder(collector, true, "", srvName, zipkin.WithJSONMaterializer())
		// create tracer.
		tracer, err = zipkin.NewTracer(recorder, zipkin.ClientServerSameSpan(false), zipkin.TraceID128Bit(true))
		if err != nil {
			fmt.Printf("unable to create Zipkin tracer: %v", err)
			return
		}
	}
	enable = true
	opentracing.SetGlobalTracer(tracer)
}

// SetGlobalTracer sets custom global tracer
func SetGlobalTracer(tracer opentracing.Tracer) {
	lock.Lock()
	defer lock.Unlock()
	enable = true
	opentracing.SetGlobalTracer(tracer)
	if collector != nil {
		collector.Close()
		collector = nil
	}
}

// IsEnabled returns true if opentracing is enabled, false otherwise
func IsEnabled() bool {
	return enable
}

// GlobalTracer returns global instance of Tracer
func GlobalTracer() opentracing.Tracer {
	return opentracing.GlobalTracer()
}

// These are wrappers over go-kit implementations.

// ClientEndPoint is go-kit Middleware to trace RPC Clients
func ClientEndPoint(operationName string) endpoint.Middleware {
	if IsEnabled() {
		return gokittracing.TraceClient(GlobalTracer(), operationName)
	}
	return func(next endpoint.Endpoint) endpoint.Endpoint {
		return next
	}
}

// ServerEndpoint is go-kit middleware to trace RPC Servers
func ServerEndpoint(operationName string) endpoint.Middleware {
	if IsEnabled() {
		return gokittracing.TraceServer(GlobalTracer(), operationName)
	}
	return func(next endpoint.Endpoint) endpoint.Endpoint {
		return next
	}
}

// ToGRPCRequest is go-kit helper for logging GRPC Requests at client
// if enabled, this injects an OpenTracing Span found , if any,
// in `ctx` into the grpc Metadata.
func ToGRPCRequest(logger log.Logger) func(ctx context.Context, md *metadata.MD) context.Context {
	if IsEnabled() {
		return gokittracing.ToGRPCRequest(GlobalTracer(), logger)
	}
	return func(ctx context.Context, md *metadata.MD) context.Context {
		return ctx
	}
}

// FromGRPCRequest is go-kit helper on grpc server side
// if enabled, this creates a new span with metadata from grpc request injected on
//  client side with ToGRPCRequest() call above
func FromGRPCRequest(operationName string, logger log.Logger) func(ctx context.Context, md metadata.MD) context.Context {
	if IsEnabled() {
		return gokittracing.FromGRPCRequest(GlobalTracer(), operationName, logger)
	}
	return func(ctx context.Context, md metadata.MD) context.Context {
		return ctx
	}
}
