package apisrvpkg

import (
	"fmt"
	"sync"

	opentracing "github.com/opentracing/opentracing-go"
	"github.com/opentracing/opentracing-go/log"
	"google.golang.org/grpc"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/apiserver"
)

// ServiceHdlr is a represantaion of a service for API server
type ServiceHdlr struct {
	sync.Mutex
	// Name is the name of the service
	Name string
	// Methods is a container for the Methods defined in the service.
	Methods map[string]apiserver.Method
	// enabled is set to true when the service is enabled.
	enabled bool
	// watchFn is registered by auto generated code to handle watches for this service
	watchFn apiserver.WatchSvcKvFunc
	// crudMsgMap is a map of kind to API server message
	crudMsgMap map[string]apiserver.Message
	// prepMsgMap has a map of kind to message transform functions
	prepMsgMap map[string]func(from, to string, i interface{}) (interface{}, error)
}

// Disable disables all futures requests on the service to be forbidden
func (s *ServiceHdlr) Disable() {
	s.Lock()
	defer s.Unlock()
	for _, m := range s.Methods {
		m.Disable()
	}
	s.enabled = false
}

// Enable sets the API invocations on the Service to be allowed.
func (s *ServiceHdlr) Enable() {
	s.Lock()
	defer s.Unlock()
	for _, m := range s.Methods {
		m.Enable()
	}
	s.enabled = true
}

// GetMethod retrieves a method in the service given the method name.
func (s *ServiceHdlr) GetMethod(n string) apiserver.Method {
	if v, ok := s.Methods[n]; ok {
		return v
	}
	return nil
}

// GetCrudService is a helper function to retrieve a auto generated CRUD method
func (s *ServiceHdlr) GetCrudService(in string, oper apiserver.APIOperType) apiserver.Method {
	mname := fmt.Sprintf(apiserver.GetCrudServiceName(in, oper))
	return s.GetMethod(mname)
}

// AddMethod is invoked by the generated code to populate the Service with Methods.
func (s *ServiceHdlr) AddMethod(n string, m apiserver.Method) apiserver.Method {
	s.Methods[n] = m
	return s.Methods[n]
}

// WithKvWatchFunc retisters a Watch Handler for the service
func (s *ServiceHdlr) WithKvWatchFunc(fn apiserver.WatchSvcKvFunc) apiserver.Service {
	s.watchFn = fn
	return s
}

// WatchFromKv implements the watch function from KV store and bridges it to the grpc stream
func (s *ServiceHdlr) WatchFromKv(options *api.ListWatchOptions, stream grpc.ServerStream, svcprefix string) error {
	if !singletonAPISrv.getRunState() {
		return errShuttingDown
	}
	if s.watchFn != nil {
		var ver string
		l := singletonAPISrv.Logger
		ctx := stream.Context()

		md, ok := metadata.FromIncomingContext(ctx)
		if !ok {
			l.ErrorLog("msg", "unable to get metadata from context")
			return errRequestInfo.makeError(nil, []string{"metadata"}, "")
		}

		if v, ok := md[apiserver.RequestParamVersion]; ok {
			ver = v[0]
		} else {
			l.ErrorLog("msg", "unable to get request version from context")
			return errRequestInfo.makeError(nil, []string{"Unable to determine version"}, "")
		}
		var span opentracing.Span
		span = opentracing.SpanFromContext(ctx)
		if span != nil {
			span.SetTag("version", ver)
			span.SetTag("operation", "watch")
			if v, ok := md[apiserver.RequestParamMethod]; ok {
				span.SetTag(apiserver.RequestParamMethod, v[0])
			}
			span.LogFields(log.String("event", "calling watch"))
		}
		kv := singletonAPISrv.getKvConn()
		handle := singletonAPISrv.insertWatcher(stream.Context())
		defer singletonAPISrv.removeWatcher(handle)
		return s.watchFn(l, options, kv, stream, s.prepMsgMap, ver, svcprefix)
	}
	return errUnknownOperation.makeError(nil, []string{}, "")
}

// WithCrudServices registers all the crud services for this service
func (s *ServiceHdlr) WithCrudServices(msgs []apiserver.Message) apiserver.Service {
	for _, msg := range msgs {
		s.crudMsgMap[msg.GetKind()] = msg
		s.prepMsgMap[msg.GetKind()] = msg.PrepareMsg
	}
	return s
}

// NewService initializes and returns a new service object.
func NewService(n string) apiserver.Service {
	return &ServiceHdlr{
		Name:       n,
		Methods:    make(map[string]apiserver.Method),
		crudMsgMap: make(map[string]apiserver.Message),
		prepMsgMap: make(map[string]func(from, to string, i interface{}) (interface{}, error))}
}
