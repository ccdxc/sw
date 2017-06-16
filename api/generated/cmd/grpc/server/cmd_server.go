/*
Package cmd is a auto generated package.
Input file: protos/cmd.proto
*/
package cmdApiServer

import (
	"context"

	"github.com/pensando/sw/apiserver"
	"github.com/pensando/sw/apiserver/pkg"

	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/runtime"
	"github.com/pkg/errors"
	"google.golang.org/grpc"
)

var apisrv apiserver.Server

type s_cmdBackend struct {
	Services map[string]apiserver.Service
	Messages map[string]apiserver.Message

	endpoints_CmdV1 *e_CmdV1Endpoints
}

type e_CmdV1Endpoints struct {
	Svc s_cmdBackend

	fn_GetNodeList func(ctx context.Context, t interface{}) (interface{}, error)
	fn_NodeOper    func(ctx context.Context, t interface{}) (interface{}, error)
	fn_ClusterOper func(ctx context.Context, t interface{}) (interface{}, error)
}

func (s *s_cmdBackend) CompleteRegistration(ctx context.Context, logger log.Logger,
	grpcserver *grpc.Server, scheme *runtime.Scheme) error {
	s.Messages = map[string]apiserver.Message{

		"cmd.NodeSpec":   apisrvpkg.NewMessage("cmd.NodeSpec"),
		"cmd.NodeStatus": apisrvpkg.NewMessage("cmd.NodeStatus"),
		"cmd.Node": apisrvpkg.NewMessage("cmd.Node").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(cmd.Node)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(cmd.Node)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r)
				err = errors.Wrap(err, "KV update failed")
			}
			return r, err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := cmd.Node{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := cmd.Node{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}),
		"cmd.NodeList":      apisrvpkg.NewMessage("cmd.NodeList"),
		"cmd.ClusterSpec":   apisrvpkg.NewMessage("cmd.ClusterSpec"),
		"cmd.ClusterStatus": apisrvpkg.NewMessage("cmd.ClusterStatus"),
		"cmd.Cluster": apisrvpkg.NewMessage("cmd.Cluster").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(cmd.Cluster)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(cmd.Cluster)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r)
				err = errors.Wrap(err, "KV update failed")
			}
			return r, err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := cmd.Cluster{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := cmd.Cluster{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}),
	}

	scheme.AddKnownTypes(
		&cmd.Node{},
		&cmd.Cluster{},
	)

	apisrv.RegisterMessages("cmd", s.Messages)

	{
		srv := apisrvpkg.NewService("CmdV1")

		s.endpoints_CmdV1.fn_GetNodeList = srv.AddMethod("GetNodeList",
			apisrvpkg.NewMethod(s.Messages["cmd.NodeList"], s.Messages["cmd.NodeList"], "cmd", "GetNodeList")).WithPreCommitHook(func(ctx context.Context, oper string, i interface{}) (interface{}, bool) {
			return i, false
		}).WithVersion("v1").HandleInvocation

		s.endpoints_CmdV1.fn_NodeOper = srv.AddMethod("NodeOper",
			apisrvpkg.NewMethod(s.Messages["cmd.Node"], s.Messages["cmd.Node"], "cmd", "NodeOper")).WithVersion("v1").HandleInvocation

		s.endpoints_CmdV1.fn_ClusterOper = srv.AddMethod("ClusterOper",
			apisrvpkg.NewMethod(s.Messages["cmd.Cluster"], s.Messages["cmd.Cluster"], "cmd", "ClusterOper")).WithVersion("v1").HandleInvocation

		s.Services = map[string]apiserver.Service{
			"cmd.CmdV1": srv,
		}
		apisrv.RegisterService("cmd.CmdV1", srv)
		endpoints := cmd.MakeCmdV1ServerEndpoints(s.endpoints_CmdV1, logger)
		server := cmd.MakeGRPCServerCmdV1(ctx, endpoints, logger)
		cmd.RegisterCmdV1Server(grpcserver, server)
	}
	return nil
}

func (e *e_CmdV1Endpoints) GetNodeList(ctx context.Context, t cmd.NodeList) (cmd.NodeList, error) {
	r, err := e.fn_GetNodeList(ctx, t)
	if err == nil {
		return r.(cmd.NodeList), err
	} else {
		return cmd.NodeList{}, err
	}
}
func (e *e_CmdV1Endpoints) NodeOper(ctx context.Context, t cmd.Node) (cmd.Node, error) {
	r, err := e.fn_NodeOper(ctx, t)
	if err == nil {
		return r.(cmd.Node), err
	} else {
		return cmd.Node{}, err
	}
}
func (e *e_CmdV1Endpoints) ClusterOper(ctx context.Context, t cmd.Cluster) (cmd.Cluster, error) {
	r, err := e.fn_ClusterOper(ctx, t)
	if err == nil {
		return r.(cmd.Cluster), err
	} else {
		return cmd.Cluster{}, err
	}
}

func init() {
	apisrv = apisrvpkg.MustGetApiServer()

	svc := s_cmdBackend{}

	{
		e := e_CmdV1Endpoints{Svc: svc}
		svc.endpoints_CmdV1 = &e
	}
	apisrv.Register("cmd.protos/cmd.proto", &svc)
}
