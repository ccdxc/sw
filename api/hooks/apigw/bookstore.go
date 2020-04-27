package impl

import (
	"context"
	"errors"

	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/utils/log"
)

type bookstoreHooks struct {
	logger log.Logger
}

func (b *bookstoreHooks) preCallHook(ctx context.Context, i, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	b.logger.InfoLog("msg", "received PreCallHook callback", "obj", i)
	in := i.(*bookstore.Order)
	if in.Spec.Id == "order-reject" {
		return ctx, in, out, false, errors.New("order not allowed")
	}
	return ctx, in, out, false, nil
}

func (b *bookstoreHooks) postCallHook(ctx context.Context, i interface{}) (context.Context, interface{}, error) {
	b.logger.InfoLog("msg", "received PostCallHook callback", "obj", i)
	in := i.(*bookstore.Order)
	in.Status.Message = "Message filled by hook"
	return ctx, in, nil
}

func (b *bookstoreHooks) skipAuthN(ctx context.Context, in interface{}) (retCtx context.Context, retIn interface{}, skipAuth bool, err error) {
	return ctx, in, true, nil
}

func registerBookstoreHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := bookstoreHooks{logger: l}
	prof, err := svc.GetCrudServiceProfile("Order", "create")
	if err != nil {
		return err
	}
	prof.AddPreCallHook(r.preCallHook)
	prof.AddPostCallHook(r.postCallHook)
	prof, err = svc.GetCrudServiceProfile("Order", "get")
	if err != nil {
		return err
	}
	prof.AddPostCallHook(r.postCallHook)

	prof, err = svc.GetProxyServiceProfile("/uploads")
	if err != nil {
		return err
	}
	if prof == nil {
		log.Fatalf("Got profile to be Nil")
	}
	prof.AddPreAuthNHook(r.skipAuthN)
	return nil
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("bookstore.BookstoreV1", registerBookstoreHooks)
}
