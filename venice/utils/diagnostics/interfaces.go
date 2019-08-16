package diagnostics

import (
	"context"

	"github.com/pensando/sw/api"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/protos"
)

// Handler interface is an abstraction for a handler to process a diagnostic query like get logs
type Handler interface {
	// HandleRequest processes diagnostic query specified in the request
	HandleRequest(ctx context.Context, req *diagapi.DiagnosticsRequest) (*api.Any, error)
	// Start initializes the handler. If the handler is already started it should be a no-op
	Start() error
	// Stop stops the handler. If the handler is already stopped it should be a no-op
	Stop()
}

// Service interface is an abstraction for Debug rpc method to process diagnostic queries
type Service interface {
	protos.DiagnosticsServer
	// RegisterHandler registers a handler to handle a diagnostic request query like get logs. It calls Start() on the handler being registered. If handler fails
	// to start it doesn't register it and returns error
	RegisterHandler(rpcMethod, query string, handler Handler) error
	// UnregisterHandler stops and un-registers handler. It returns false if handler is not found
	UnregisterHandler(rpcMethod, query string) (Handler, bool)
	// RegisterCustomAction registers service specific custom actions with a handler.
	RegisterCustomAction(action string, handler CustomHandler) error
	// GetHandlers returns all the registered handlers
	GetHandlers() []Handler
	// GetHandler returns the registered handler for the given rpc method and query
	GetHandler(rpcMethod, query string) (Handler, bool)
	// Start starts all registered handlers
	Start()
	// Stop stops all registered handlers
	Stop()
	// GetModule returns the name of the process or container
	GetModule() string
	// GetNode returns the node name on which the process is running
	GetNode() string
	// GetCategory returns the category of the process
	GetCategory() diagapi.ModuleStatus_CategoryType
}

// ClientGetter interface abstracts out diagnostics Client creation
type ClientGetter interface {
	// GetClient returns an implementation of diagnostics Client
	GetClient() (Client, error)
}

// ClientGetterFunc implements ClientGetter interface
type ClientGetterFunc func() (Client, error)

// GetClient implements method for ClientGetter interface
func (f ClientGetterFunc) GetClient() (Client, error) {
	return f()
}

// Client interface abstracts out Diagnostics rpc client
type Client interface {
	protos.DiagnosticsClient
	// Close closes the client
	Close()
}

// Router interface abstracts out determining routing coordinates of diagnostics request
type Router interface {
	// GetRoute returns service URL and remote service name
	GetRoute(*diagapi.DiagnosticsRequest) (string, string, error)
}

// CustomHandler handle service specific actions. Inputs are opaque actions and params (not
// validated by the diagnostics infra). The handler returns a interface and error. If the
// returned object is a proto.Message then the object is marshalled into a proto.Any,
// otherwise the object is json marshalled into a string and returned. If error is not nil,
// the error is returned as the body of the response.
type CustomHandler func(action string, params map[string]string) (interface{}, error)

// Getter is an interface to fetch module object
type Getter interface {
	// GetModule returns module object given the key
	GetModule(name string) (*diagapi.Module, error)
}
