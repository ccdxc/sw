package impl

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/audit"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/module"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type diagnosticsHooks struct {
	apiServer    string
	rslvr        resolver.Interface
	logger       log.Logger
	moduleGetter diagnostics.Getter
	clientGetter diagnostics.ClientGetter // to support unit testing
	diagSvc      diagnostics.Service
}

// DebugPreCallHook implements Debug action for module object
func (d *diagnosticsHooks) DebugPreCallHook(ctx context.Context, in, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	obj, ok := in.(*diagapi.DiagnosticsRequest)
	if !ok {
		return ctx, nil, out, true, errors.New("invalid input type")
	}
	var err error
	clGetter := d.clientGetter
	if clGetter == nil { // will be not nil for unit testing. will set a mock
		clGetter, err = diagnostics.NewClientGetter(globals.APIGw, obj, diagnostics.NewRouter(d.rslvr, d.moduleGetter, d.logger), d.diagSvc, d.rslvr)
		if err != nil {
			d.logger.ErrorLog("method", "DebugPreCallHook", "msg", fmt.Sprintf("unable to instantiate ClientGetter to process diagnostics request [%#v]", *obj), "error", err)
			return ctx, nil, out, true, err

		}
	}
	diagCl, err := clGetter.GetClient()
	if err != nil {
		d.logger.ErrorLog("method", "DebugPreCallHook", "msg", "failed to get diagnostics client", "error", err)
		return ctx, nil, out, true, err
	}
	defer diagCl.Close()
	resp, err := diagCl.Debug(ctx, obj)
	if err != nil {
		d.logger.ErrorLog("method", "DebugPreCallHook", "msg", fmt.Sprintf("rpc call Debug failed for diagnostics request [%#v] for module obj [%s]", *obj, obj.Name), "error", err)
		return ctx, nil, out, true, err
	}
	return ctx, resp, out, true, nil
}

func (d *diagnosticsHooks) registerDebugPreCallHook(svc apigw.APIGatewayService) error {
	prof, err := svc.GetServiceProfile("Debug")
	if err != nil {
		return err
	}
	prof.AddPreCallHook(d.DebugPreCallHook)
	prof.SetAuditLevel(audit.Level_Request.String())
	return nil
}

func registerDiagnosticsHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	d := &diagnosticsHooks{
		apiServer: gw.GetAPIServerAddr(globals.APIServer),
		logger:    l,
		rslvr:     gw.GetResolver(),
		diagSvc:   gw.GetDiagnosticsService(),
	}
	d.moduleGetter = module.NewGetter(d.apiServer, d.rslvr, d.logger)
	return d.registerDebugPreCallHook(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("diagnostics.DiagnosticsV1", registerDiagnosticsHooks)
}
