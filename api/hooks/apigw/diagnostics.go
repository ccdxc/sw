package impl

import (
	"context"
	"errors"
	"fmt"

	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type diagnosticsHooks struct {
	apiServer    string
	rslvr        resolver.Interface
	b            balancer.Balancer
	logger       log.Logger
	moduleGetter module.Getter
	clientGetter diagnostics.ClientGetter // to support unit testing
	diagSvc      diagnostics.Service
}

// DebugPreCallHook implements Debug action for module object
func (d *diagnosticsHooks) DebugPreCallHook(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	obj, ok := in.(*diagapi.DiagnosticsRequest)
	if !ok {
		return ctx, nil, true, errors.New("invalid input type")
	}
	modObj, err := d.moduleGetter.GetModule(obj.Name)
	if err != nil {
		d.logger.ErrorLog("method", "DebugPreCallHook", "msg", fmt.Sprintf("failed to get module object [%s]", obj.Name), "error", err)
		return ctx, nil, true, err
	}
	// find service instance URL to route for Venice grpc services TODO: Abstract out routing
	var svcURL string
	if diagnostics.IsSupported(modObj) {
		svcInstanceList := d.rslvr.Lookup(modObj.Status.Module)
		if svcInstanceList != nil {
			for _, svcInstance := range svcInstanceList.Items {
				if svcInstance.Node == modObj.Status.Node {
					svcURL = svcInstance.URL
					break
				}
			}
		}
	} else {
		return ctx, nil, true, fmt.Errorf("diagnostics not supported for module [%s]", modObj.Name)
	}
	if svcURL == "" {
		d.logger.ErrorLog("method", "DebugPreCallHook", "msg", fmt.Sprintf("unable to locate service instance for module [%s]", modObj.Name))
		return ctx, nil, true, fmt.Errorf("unable to locate service instance for module [%s]", modObj.Name)
	}
	clGetter := d.clientGetter
	if clGetter == nil { // will be not nil for unit testing. will set a mock
		clGetter = diagnostics.GetClientGetter(globals.APIGw, svcURL, modObj.Status.Module, d.diagSvc)
	}
	diagCl, err := clGetter.GetClient()
	if err != nil {
		d.logger.ErrorLog("method", "DebugPreCallHook", "msg", fmt.Sprintf("failed to get diagnostics client for service URL [%s]", svcURL), "error", err)
		return ctx, nil, true, err
	}
	defer diagCl.Close()
	resp, err := diagCl.Debug(ctx, &diagapi.DiagnosticsRequest{
		ObjectMeta: modObj.ObjectMeta,
		Query:      obj.Query,
		Parameters: obj.Parameters,
	})
	if err != nil {
		d.logger.ErrorLog("method", "DebugPreCallHook", "msg", fmt.Sprintf("rpc call Debug failed to service instance [%s] for module obj [%s]", svcURL, modObj.Name), "error", err)
		return ctx, nil, true, err
	}
	return ctx, resp, true, nil
}

func (d *diagnosticsHooks) registerDebugPreCallHook(svc apigw.APIGatewayService) error {
	prof, err := svc.GetServiceProfile("Debug")
	if err != nil {
		return err
	}
	prof.AddPreCallHook(d.DebugPreCallHook)
	return nil
}

func registerDiagnosticsHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	d := &diagnosticsHooks{
		apiServer: gw.GetAPIServerAddr(globals.APIServer),
		logger:    l,
		rslvr:     gw.GetResolver(),
		b:         balancer.New(gw.GetResolver()),
		diagSvc:   gw.GetDiagnosticsService(),
	}
	d.moduleGetter = module.NewGetter(d.apiServer, d.rslvr, d.b, d.logger)
	return d.registerDebugPreCallHook(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("diagnostics.DiagnosticsV1", registerDiagnosticsHooks)
}
