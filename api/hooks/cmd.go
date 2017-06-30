package impl

import (
	"context"
	"fmt"

	opentracing "github.com/opentracing/opentracing-go"
	"github.com/pborman/uuid"
	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/apiserver"
	apisrvpkg "github.com/pensando/sw/apiserver/pkg"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/log"
)

type cmdHooks struct {
	logger log.Logger
	tracer opentracing.Tracer
}

func (r *cmdHooks) getNodeList(ctx context.Context, kvs kvstore.Interface, prefix string, i interface{}, old interface{}, oper string) (interface{}, error) {

	n := i.(cmd.NodeList)
	node := cmd.Node{}

	key := node.MakeKey(prefix)
	err := kvs.List(context.TODO(), key, &n)
	if err != nil {
		return nil, err
	}
	return n, nil

}
func (r *cmdHooks) clusterOper(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key, oper string, i interface{}) (interface{}, bool, error) {
	if oper == "POST" {
		n := i.(cmd.Cluster)
		n.UUID = uuid.New()
		return n, true, nil
	}
	return i, true, nil
}

func (r *cmdHooks) nodeOper(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key, oper string, i interface{}) (interface{}, bool, error) {
	if oper == "POST" {
		n := i.(cmd.Node)
		n.UUID = uuid.New()
		return n, true, nil
	}
	return i, true, nil
}

func registerCmdHooks(svc apiserver.Service, logger log.Logger) {
	r := cmdHooks{}
	r.logger = logger.WithContext("Service", "CMD")
	logger.Log("msg", "registering Hooks")
	svc.GetMethod("GetNodeList").WithResponseWriter(r.getNodeList)
	svc.GetMethod("NodeOper").WithPreCommitHook(r.nodeOper)
	svc.GetMethod("ClusterOper").WithPreCommitHook(r.clusterOper)
}

func init() {
	fmt.Printf("registered Hooks")
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("cmd.CmdV1", registerCmdHooks)
}
