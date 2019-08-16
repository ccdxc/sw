package service

import (
	"context"
	"fmt"
	"sync"

	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// validMethods are rpc methods defined in diagnostics.proto. It is used to validate rpc method name while registering query handlers
var validMethods = map[string]bool{"Debug": true}

// diagnosticsService is a singleton that implements Service interface
var gService *diagnosticsService
var serviceOnce sync.Once

type diagnosticsService struct {
	sync.RWMutex
	handlers map[string]diagnostics.Handler
	module   string
	node     string
	category diagapi.ModuleStatus_CategoryType
	logger   log.Logger
	stopped  bool
}

func (d *diagnosticsService) RegisterHandler(rpcMethod, query string, handler diagnostics.Handler) error {
	if err := validateRPCMethod(rpcMethod); err != nil {
		return err
	}
	key := getHandlerKey(rpcMethod, query)
	defer d.Unlock()
	d.Lock()
	h, ok := d.handlers[key]
	if ok {
		// Stop the current handler and register the new one provided
		h.Stop()
		d.logger.InfoLog("method", "RegisterHandler", "msg", "closing existing handler due to duplicate re-registration", "method", rpcMethod, "query", query)
	}
	if err := handler.Start(); err != nil {
		return err
	}
	d.handlers[key] = handler
	return nil
}

func (d *diagnosticsService) UnregisterHandler(rpcMethod, query string) (diagnostics.Handler, bool) {
	defer d.Unlock()
	d.Lock()
	key := getHandlerKey(rpcMethod, query)
	handler, ok := d.handlers[key]
	if !ok {
		return nil, false
	}
	handler.Stop()
	delete(d.handlers, key)
	return handler, true
}

// RegisterCustomAction registers service specific custom actions with a handler.
func (d *diagnosticsService) RegisterCustomAction(action string, handler diagnostics.CustomHandler) error {
	key := getHandlerKey("Debug", diagapi.DiagnosticsRequest_Action.String())
	defer d.Unlock()
	d.Lock()
	h, ok := d.handlers[key]
	if !ok {
		h = &customActionHandler{actions: make(map[string]diagnostics.CustomHandler)}
		d.handlers[key] = h
	}
	ch := h.(*customActionHandler)
	defer ch.Unlock()
	ch.Lock()
	if _, ok := ch.actions[action]; ok {
		return fmt.Errorf("action is already registered")
	}
	ch.actions[action] = handler
	ch.knownActions = append(ch.knownActions, action)
	return nil
}

func (d *diagnosticsService) GetHandlers() []diagnostics.Handler {
	defer d.RUnlock()
	d.RLock()
	var svcs []diagnostics.Handler
	for _, svc := range d.handlers {
		svcs = append(svcs, svc)
	}
	return svcs
}

func (d *diagnosticsService) GetHandler(rpcMethod, query string) (diagnostics.Handler, bool) {
	defer d.RUnlock()
	d.RLock()
	key := getHandlerKey(rpcMethod, query)
	svc, ok := d.handlers[key]
	return svc, ok
}

func (d *diagnosticsService) Debug(ctx context.Context, req *diagapi.DiagnosticsRequest) (*diagapi.DiagnosticsResponse, error) {
	defer d.RUnlock()
	d.RLock()
	key := getHandlerKey("Debug", req.Query)
	svc, ok := d.handlers[key]
	if !ok {
		return nil, fmt.Errorf("unknown query: %s", req.Query)
	}
	any, err := svc.HandleRequest(ctx, req)
	if err != nil {
		return nil, err
	}
	return &diagapi.DiagnosticsResponse{Object: any}, nil
}

func (d *diagnosticsService) Start() {
	defer d.Unlock()
	d.Lock()
	if d.stopped {
		for query, svc := range d.handlers {
			if err := svc.Start(); err != nil {
				d.logger.ErrorLog("method", "Start", "msg", fmt.Sprintf("failed to start service for query %s", query))
				// TODO throw an event
			}
		}
		d.stopped = false
	}
}

func (d *diagnosticsService) Stop() {
	defer d.Unlock()
	d.Lock()
	for _, handler := range d.handlers {
		handler.Stop()
	}
	d.stopped = true
}

func (d *diagnosticsService) GetModule() string {
	return d.module
}

func (d *diagnosticsService) GetNode() string {
	return d.node
}

func (d *diagnosticsService) GetCategory() diagapi.ModuleStatus_CategoryType {
	return d.category
}

// GetDiagnosticsService returns a singleton implementation of debug Service
func GetDiagnosticsService(module, node string, category diagapi.ModuleStatus_CategoryType, logger log.Logger) diagnostics.Service {
	serviceOnce.Do(func() {
		gService = &diagnosticsService{
			handlers: make(map[string]diagnostics.Handler),
			module:   module,
			node:     node,
			category: category,
			logger:   logger,
			stopped:  true,
		}
	})
	return gService
}

// GetDiagnosticsServiceWithDefaults returns debug Service with log service registered
func GetDiagnosticsServiceWithDefaults(module, node string, category diagapi.ModuleStatus_CategoryType, rslvr resolver.Interface, logger log.Logger) diagnostics.Service {
	server := GetDiagnosticsService(module, node, category, logger)
	if err := server.RegisterHandler("Debug", diagapi.DiagnosticsRequest_Log.String(), NewElasticLogsHandler(module, node, category, rslvr, logger)); err != nil {
		logger.ErrorLog("method", "GetDiagnosticsServiceWithDefaults", "msg", "failed to register elastic logs handler", "err", err)
		// TODO throw an event
	}
	if err := server.RegisterHandler("Debug", diagapi.DiagnosticsRequest_Stats.String(), NewExpVarHandler(module, node, category, logger)); err != nil {
		logger.ErrorLog("method", "GetDiagnosticsServiceWithDefaults", "msg", "failed to register expvar handler", "err", err)
		// TODO throw an event
	}
	return server
}

func getHandlerKey(rpcMethod, query string) string {
	return fmt.Sprintf("%s|%s", rpcMethod, query)
}

func validateRPCMethod(rpcMethod string) error {
	if ok := validMethods[rpcMethod]; !ok {
		return fmt.Errorf("invalid rpc method: %s", rpcMethod)
	}
	return nil
}
