package impl

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/archive"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type archiveHooks struct {
	logger        log.Logger
	apiServer     string
	rslvr         resolver.Interface
	b             balancer.Balancer
	archiveGetter archive.Getter
	clientGetter  archive.ClientGetter // to support unit testing
}

// CancelPreCallHook implements CancelArchiveRequest action
func (a *archiveHooks) CancelPreCallHook(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	obj, ok := in.(*monitoring.CancelArchiveRequest)
	if !ok {
		return ctx, nil, true, errors.New("invalid input type")
	}
	archiveGetter := a.archiveGetter
	if archiveGetter == nil {
		archiveGetter = archive.NewGetter(a.apiServer, a.b, a.rslvr, a.logger)
	}
	req, err := archiveGetter.GetArchiveRequest(&obj.ObjectMeta)
	if err != nil {
		return ctx, nil, true, err
	}
	a.logger.DebugLog("method", "CancelPreCallHook", "msg", fmt.Sprintf("***request UUID %s", req.UUID))
	clGetter := a.clientGetter
	if clGetter == nil { // will be not nil for unit testing, will set a mock
		clGetter, err = archive.NewClientGetter(globals.APIGw, req.Spec.Type, a.b, a.rslvr, a.logger)
		if err != nil {
			a.logger.ErrorLog("method", "CancelPreCallHook", "msg", fmt.Sprintf("unable to instantiate ClientGetter to cancel archiverequest [%#v]", *obj), "error", err)
			return ctx, nil, true, err
		}
	}
	archiveCl, err := clGetter.GetClient()
	if err != nil {
		a.logger.ErrorLog("method", "CancelPreCallHook", "msg", "failed to get archive client", "error", err)
		return ctx, nil, true, err
	}
	defer archiveCl.Close()
	resp, err := archiveCl.CancelRequest(ctx, req)
	if err != nil {
		a.logger.ErrorLog("method", "CancelPreCallHook", "msg", fmt.Sprintf("rpc call CancelRequest failed for archive request [%#v] with name [%s]", *req, req.Name), "error", err)
		return ctx, nil, true, err
	}
	return ctx, resp, true, nil
}

func (a *archiveHooks) registerCancelPreCallHook(svc apigw.APIGatewayService) error {
	prof, err := svc.GetServiceProfile("Cancel")
	if err != nil {
		return err
	}
	prof.AddPreCallHook(a.CancelPreCallHook)
	return nil
}

func registerArchiveHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	d := &archiveHooks{
		logger:    l,
		apiServer: gw.GetAPIServerAddr(globals.APIServer),
		rslvr:     gw.GetResolver(),
		b:         balancer.New(gw.GetResolver()),
	}
	d.archiveGetter = archive.NewGetter(globals.APIServer, d.b, d.rslvr, d.logger)
	return d.registerCancelPreCallHook(svc)
}
