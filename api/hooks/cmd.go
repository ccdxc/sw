package impl

import (
	"context"

	opentracing "github.com/opentracing/opentracing-go"
	"github.com/pborman/uuid"
	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type cmdHooks struct {
	logger log.Logger
	tracer opentracing.Tracer
}

func (r *cmdHooks) clusterCreate(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, i interface{}) (interface{}, bool, error) {
	n := i.(cmd.Cluster)
	n.UUID = uuid.New()
	return n, true, nil
}

func (r *cmdHooks) nodeCreate(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, i interface{}) (interface{}, bool, error) {
	n := i.(cmd.Node)
	n.UUID = uuid.New()
	return n, true, nil
}

func registerCmdHooks(svc apiserver.Service, logger log.Logger) {
	r := cmdHooks{}
	r.logger = logger.WithContext("Service", "CMD")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("Node", apiserver.CreateOper).WithPreCommitHook(r.nodeCreate)
	svc.GetCrudService("Cluster", apiserver.CreateOper).WithPreCommitHook(r.clusterCreate)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("cmd.CmdV1", registerCmdHooks)
}
