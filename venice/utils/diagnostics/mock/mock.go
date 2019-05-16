package mock

import (
	"context"
	"errors"

	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
)

type client struct {
	simulateError bool
}

func (c *client) Close() {}

func (c *client) Debug(ctx context.Context, in *diagapi.DiagnosticsRequest, opts ...grpc.CallOption) (*diagapi.DiagnosticsResponse, error) {
	if c.simulateError {
		return nil, errors.New("simulated Debug error")
	}
	return &diagapi.DiagnosticsResponse{Object: &api.Any{}}, nil
}

// GetClientGetter returns mock client getter
func GetClientGetter(simulateDebugError, simulateClientError bool) diagnostics.ClientGetter {
	var clgetter diagnostics.ClientGetterFunc
	clgetter = func() (diagnostics.Client, error) {
		if simulateClientError {
			return nil, errors.New("simulated client error")
		}
		return &client{simulateError: simulateDebugError}, nil
	}
	return clgetter
}

type moduleGetter struct {
	simulateError bool
	modObj        *diagapi.Module
}

func (m *moduleGetter) GetModule(name string) (*diagapi.Module, error) {
	if m.simulateError {
		return nil, errors.New("simulated module error")
	}
	return m.modObj, nil
}

// GetModuleGetter returns mock module getter
func GetModuleGetter(modObj *diagapi.Module, simulateError bool) module.Getter {
	return &moduleGetter{
		modObj:        modObj,
		simulateError: simulateError,
	}
}

type moduleUpdater struct {
	simulateError bool
}

func (m *moduleUpdater) Enqueue(module *diagapi.Module, oper module.Oper) error {
	if m.simulateError {
		return errors.New("simulated module updated error")
	}
	return nil
}

func (m *moduleUpdater) Start() {}

func (m *moduleUpdater) Stop() {}

// GetModuleUpdater return mock module updater
func GetModuleUpdater() module.Updater {
	return &moduleUpdater{}
}

type moduleWatcher struct{}

func (m *moduleWatcher) Module() diagapi.Module {
	modObj := diagapi.Module{}
	modObj.Defaults("all")
	return modObj
}

func (m *moduleWatcher) RegisterOnChangeCb(cb module.OnChangeCb) {}

func (m *moduleWatcher) Start() {}

func (m *moduleWatcher) Stop() {}

// GetModuleWatcher returns mock module watcher
func GetModuleWatcher() module.Watcher {
	return &moduleWatcher{}
}

type diagnosticsService struct{}

func (d *diagnosticsService) Debug(context.Context, *diagapi.DiagnosticsRequest) (*diagapi.DiagnosticsResponse, error) {
	return nil, nil
}

func (d *diagnosticsService) RegisterHandler(rpcMethod, query string, handler diagnostics.Handler) error {
	return nil
}

func (d *diagnosticsService) UnregisterHandler(rpcMethod, query string) (diagnostics.Handler, bool) {
	return nil, false
}

func (d *diagnosticsService) GetHandlers() []diagnostics.Handler {
	return nil
}

func (d *diagnosticsService) GetHandler(rpcMethod, query string) (diagnostics.Handler, bool) {
	return nil, false
}

func (d *diagnosticsService) Start() {}

func (d *diagnosticsService) Stop() {}

func (d *diagnosticsService) GetModule() string {
	return ""
}

func (d *diagnosticsService) GetNode() string {
	return ""
}

func (d *diagnosticsService) GetCategory() diagapi.ModuleStatus_CategoryType {
	return diagapi.ModuleStatus_Venice
}

// GetDiagnosticsService returns mock diagnostics service
func GetDiagnosticsService() diagnostics.Service {
	return &diagnosticsService{}
}
