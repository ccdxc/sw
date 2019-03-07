package plugins

import (
	"context"

	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/vos"
)

func getLoggerCbFunc(bucket string, stage vos.OperStage) vos.CallBackFunc {
	return func(ctx context.Context, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) error {
		log.InfoLog("bucket", bucket, "Stage", stage, "oper", oper, "msg", "logger plugin")
		return nil
	}
}

func loggerRegisterCallbacks(instance vos.Interface) {
	for _, n := range objstore.Buckets_name {
		instance.RegisterCb(n, vos.PreOp, vos.Upload, getLoggerCbFunc(n, vos.PreOp))
		instance.RegisterCb(n, vos.PreOp, vos.Download, getLoggerCbFunc(n, vos.PreOp))
		instance.RegisterCb(n, vos.PreOp, vos.Get, getLoggerCbFunc(n, vos.PreOp))
		instance.RegisterCb(n, vos.PreOp, vos.List, getLoggerCbFunc(n, vos.PreOp))
		instance.RegisterCb(n, vos.PreOp, vos.Delete, getLoggerCbFunc(n, vos.PreOp))
		instance.RegisterCb(n, vos.PreOp, vos.Update, getLoggerCbFunc(n, vos.PreOp))
		instance.RegisterCb(n, vos.PreOp, vos.Watch, getLoggerCbFunc(n, vos.PreOp))
		instance.RegisterCb(n, vos.PostOp, vos.Upload, getLoggerCbFunc(n, vos.PostOp))
		instance.RegisterCb(n, vos.PostOp, vos.Download, getLoggerCbFunc(n, vos.PostOp))
		instance.RegisterCb(n, vos.PostOp, vos.Get, getLoggerCbFunc(n, vos.PostOp))
		instance.RegisterCb(n, vos.PostOp, vos.List, getLoggerCbFunc(n, vos.PostOp))
		instance.RegisterCb(n, vos.PostOp, vos.Delete, getLoggerCbFunc(n, vos.PostOp))
		instance.RegisterCb(n, vos.PostOp, vos.Update, getLoggerCbFunc(n, vos.PostOp))
		instance.RegisterCb(n, vos.PostOp, vos.Watch, getLoggerCbFunc(n, vos.PostOp))
	}
}
