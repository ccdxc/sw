package trace

import (
	"fmt"
	"net/http"

	opentracing "github.com/opentracing/opentracing-go"
	zipkin "github.com/openzipkin/zipkin-go-opentracing"
)

// TODO: read from config file eventually
const zipkinHTTPEndpoint = "http://192.168.30.11:9411/api/v1/spans"

var srvName string

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
	opentracing.SetGlobalTracer(opentracing.NoopTracer{})
}

// EnableOpenTraceHdlr enables opentracing compatible tracer for the caller service
func EnableOpenTraceHdlr(w http.ResponseWriter, r *http.Request) {
	EnableOpenTrace()
}

// EnableOpenTrace enables opentracing compatible tracer for the caller service
func EnableOpenTrace() {
	var tr opentracing.Tracer
	{
		collector, err := zipkin.NewHTTPCollector(zipkinHTTPEndpoint)
		if err != nil {
			fmt.Printf("unable to create Zipkin HTTP collector: %v", err)
			return
		}
		// create recorder.
		recorder := zipkin.NewRecorder(collector, true, "", srvName, zipkin.WithJSONMaterializer())
		// create tracer.
		tr, err = zipkin.NewTracer(recorder, zipkin.ClientServerSameSpan(false), zipkin.TraceID128Bit(true))
		if err != nil {
			fmt.Printf("unable to create Zipkin tracer: %v", err)
			return
		}
	}
	opentracing.SetGlobalTracer(tr)
}
