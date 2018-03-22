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

func (b *bookstoreHooks) preCallHook(ctx context.Context, i interface{}) (context.Context, interface{}, bool, error) {
	b.logger.InfoLog("msg", "received PreCallHook callback", "obj", i)
	in := i.(*bookstore.Order)
	if in.Spec.Id == "order-reject" {
		return ctx, in, false, errors.New("order not allowed")
	}
	return ctx, in, false, nil
}

func (b *bookstoreHooks) postCallHook(ctx context.Context, i interface{}) (context.Context, interface{}, error) {
	b.logger.InfoLog("msg", "received PostCallHook callback", "obj", i)
	in := i.(*bookstore.Order)
	in.Status.Message = "Message filled by hook"
	return ctx, in, nil
}

func registerBookstoreHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := bookstoreHooks{logger: l}
	prof, err := svc.GetCrudServiceProfile("Order", "create")
	if err != nil {
		return err
	}
	prof.AddPreCallHook(r.preCallHook)
	prof.AddPostCallHook(r.postCallHook)
	return nil
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("bookstore.BookstoreV1", registerBookstoreHooks)
}
