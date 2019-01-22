package impl

import (
	"context"

	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/log"
)

type objstoreHooks struct {
	logger log.Logger
}

func (b *objstoreHooks) skipAuthN(ctx context.Context, in interface{}) (retCtx context.Context, retIn interface{}, skipAuth bool, err error) {
	return ctx, in, true, nil
}

func registerObjstoreHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := objstoreHooks{logger: l}

	prof, err := svc.GetProxyServiceProfile("/uploads")
	if err != nil {
		return err
	}
	prof.AddPreAuthNHook(r.skipAuthN)
	prof, err = svc.GetCrudServiceProfile("Object", apiserver.ListOper)
	prof.AddPreAuthNHook(r.skipAuthN)
	prof, err = svc.GetCrudServiceProfile("Object", apiserver.DeleteOper)
	prof.AddPreAuthNHook(r.skipAuthN)
	prof, err = svc.GetCrudServiceProfile("Object", apiserver.GetOper)
	prof.AddPreAuthNHook(r.skipAuthN)
	return nil
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("objstore.ObjstoreV1", registerObjstoreHooks)
}
