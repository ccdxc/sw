package impl

import (
	"context"
	"errors"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
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

// removePassword is a post-call hook to remove password from the user object
func (a *authHooks) removePassword(ctx context.Context, out interface{}) (retCtx context.Context, retOut interface{}, err error) {
	a.logger.InfoLog("msg", "apigw removePassword hook called")
	switch obj := out.(type) {
	case *auth.User:
		obj.Spec.Password = ""
		return ctx, obj, nil
	case *auth.UserList:
		for _, user := range obj.GetItems() {
			user.Spec.Password = ""
		}
		return ctx, obj, nil
	default:
		return ctx, out, errors.New("invalid input type")
	}
}

// removeSecret is a post-call hook to remove secret from AuthenticationPolicy
func (a *authHooks) removeSecret(ctx context.Context, out interface{}) (retCtx context.Context, retOut interface{}, err error) {
	a.logger.InfoLog("msg", "apigw removeSecret hook called")
	obj, ok := out.(*auth.AuthenticationPolicy)
	if !ok {
		return ctx, out, errors.New("invalid input type")
	}
	obj.Spec.Secret = nil
	return ctx, obj, nil
}

func registerRemovePasswordHook(svc apigw.APIGatewayService, r authHooks) error {
	opers := []apiserver.APIOperType{apiserver.CreateOper, apiserver.UpdateOper, apiserver.DeleteOper, apiserver.GetOper, apiserver.ListOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("User", oper)
		if err != nil {
			return err
		}
		prof.AddPostCallHook(r.removePassword)
	}
	return nil
}

func registerRemoveSecretHook(svc apigw.APIGatewayService, r authHooks) error {
	opers := []apiserver.APIOperType{apiserver.CreateOper, apiserver.UpdateOper, apiserver.GetOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("AuthenticationPolicy", oper)
		if err != nil {
			return err
		}
		prof.AddPostCallHook(r.removeSecret)
	}
	return nil
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
	// register post call hook to remove password from user object
	if err := registerRemovePasswordHook(svc, r); err != nil {
		return err
	}
	// register post call hook to remove secret from AuthenticationPolicy
	if err := registerRemoveSecretHook(svc, r); err != nil {
		return err
	}

	//TODO: Add hooks for authz objects once it is wired in
	return nil
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("auth.AuthV1", registerAuthHooks)
}
