package impl

import (
	"context"

	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/utils/log"
)

type authHooks struct {
	logger log.Logger
}

func (a *authHooks) preAuthNHook(ctx context.Context, i interface{}) (context.Context, interface{}, bool, error) {
	a.logger.InfoLog("msg", "received PreAuthNHook callback", "obj", i)
	//TODO: Add a check to determine if API GW is in bootstrap mode
	return ctx, i, true, nil
}

func registerAuthHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := authHooks{logger: l}
	// register pre auth hooks to skip auth for bootstrapping
	prof, err := svc.GetCrudServiceProfile("AuthenticationPolicy", "create")
	if err != nil {
		return err
	}
	prof.AddPreAuthNHook(r.preAuthNHook)
	prof, err = svc.GetCrudServiceProfile("User", "create")
	if err != nil {
		return err
	}
	prof.AddPreAuthNHook(r.preAuthNHook)

	//TODO: Add hooks for authz objects once it is wired in
	return nil
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("auth.AuthV1", registerAuthHooks)
}
