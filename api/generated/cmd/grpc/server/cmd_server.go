// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package cmdApiServer is a auto generated package.
Input file: protos/cmd.proto
*/
package cmdApiServer

import (
	"context"
	"fmt"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/pkg/errors"
	"github.com/satori/go.uuid"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/api/listerwatcher"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

// dummy vars to suppress unused errors
var _ api.ObjectMeta
var _ listerwatcher.WatcherClient
var _ fmt.Stringer

type scmdCmdBackend struct {
	Services map[string]apiserver.Service
	Messages map[string]apiserver.Message

	endpointsCmdV1 *eCmdV1Endpoints
}

type eCmdV1Endpoints struct {
	Svc scmdCmdBackend

	fnAutoAddCluster     func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoAddNode        func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoAddSmartNIC    func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoDeleteCluster  func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoDeleteNode     func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoDeleteSmartNIC func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoGetCluster     func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoGetNode        func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoGetSmartNIC    func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoListCluster    func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoListNode       func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoListSmartNIC   func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoUpdateCluster  func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoUpdateNode     func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoUpdateSmartNIC func(ctx context.Context, t interface{}) (interface{}, error)

	fnAutoWatchCluster  func(in *api.ListWatchOptions, stream grpc.ServerStream, svcprefix string) error
	fnAutoWatchNode     func(in *api.ListWatchOptions, stream grpc.ServerStream, svcprefix string) error
	fnAutoWatchSmartNIC func(in *api.ListWatchOptions, stream grpc.ServerStream, svcprefix string) error
}

func (s *scmdCmdBackend) CompleteRegistration(ctx context.Context, logger log.Logger,
	grpcserver *rpckit.RPCServer, scheme *runtime.Scheme) error {
	s.Messages = map[string]apiserver.Message{

		"cmd.AutoMsgClusterWatchHelper":  apisrvpkg.NewMessage("cmd.AutoMsgClusterWatchHelper"),
		"cmd.AutoMsgNodeWatchHelper":     apisrvpkg.NewMessage("cmd.AutoMsgNodeWatchHelper"),
		"cmd.AutoMsgSmartNICWatchHelper": apisrvpkg.NewMessage("cmd.AutoMsgSmartNICWatchHelper"),
		"cmd.Cluster": apisrvpkg.NewMessage("cmd.Cluster").WithKeyGenerator(func(i interface{}, prefix string) string {
			if i == nil {
				r := cmd.Cluster{}
				return r.MakeKey(prefix)
			}
			r := i.(cmd.Cluster)
			return r.MakeKey(prefix)
		}).WithObjectVersionWriter(func(i interface{}, version string) interface{} {
			r := i.(cmd.Cluster)
			r.APIVersion = version
			return r
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create, ignoreStatus bool) (interface{}, error) {
			r := i.(cmd.Cluster)
			key := r.MakeKey(prefix)
			r.Kind = "Cluster"
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				if ignoreStatus {
					updateFunc := func(obj runtime.Object) (runtime.Object, error) {
						saved := obj.(*cmd.Cluster)
						if r.ResourceVersion != "" && r.ResourceVersion != saved.ResourceVersion {
							return nil, fmt.Errorf("Resource Version specified does not match Object version")
						}
						r.Status = saved.Status
						return &r, nil
					}
					into := &cmd.Cluster{}
					err = kvs.ConsistentUpdate(ctx, key, into, updateFunc)
				} else {
					if r.ResourceVersion != "" {
						logger.Infof("resource version is specified %s\n", r.ResourceVersion)
						err = kvs.Update(ctx, key, &r, kvstore.Compare(kvstore.WithVersion(key), "=", r.ResourceVersion))
					} else {
						err = kvs.Update(ctx, key, &r)
					}
					err = errors.Wrap(err, "KV update failed")
				}
			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, txn kvstore.Txn, i interface{}, prefix string, create bool) error {
			r := i.(cmd.Cluster)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = txn.Create(key, &r)
				err = errors.Wrap(err, "KV transaction create failed")
			} else {
				err = txn.Update(key, &r)
				err = errors.Wrap(err, "KV transaction update failed")
			}
			return err
		}).WithUUIDWriter(func(i interface{}) (interface{}, error) {
			r := i.(cmd.Cluster)
			r.UUID = uuid.NewV4().String()
			return r, nil
		}).WithCreationTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(cmd.Cluster)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.CreationTime.Timestamp = *ts
			}
			return r, err
		}).WithModTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(cmd.Cluster)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.ModTime.Timestamp = *ts
			}
			return r, err
		}).WithSelfLinkWriter(func(path string, i interface{}) (interface{}, error) {
			r := i.(cmd.Cluster)
			r.SelfLink = path
			return r, nil
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := cmd.Cluster{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := cmd.Cluster{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			return txn.Delete(key)
		}).WithValidate(func(i interface{}, ver string, ignoreStatus bool) error {
			r := i.(cmd.Cluster)
			if !r.Validate(ver, ignoreStatus) {
				return fmt.Errorf("Default Validation failed")
			}
			return nil
		}),
		"cmd.ClusterList": apisrvpkg.NewMessage("cmd.ClusterList").WithKvListFunc(func(ctx context.Context, kvs kvstore.Interface, options *api.ListWatchOptions, prefix string) (interface{}, error) {

			into := cmd.ClusterList{}
			r := cmd.Cluster{}
			key := r.MakeKey(prefix)
			err := kvs.List(ctx, key, &into)
			if err != nil {
				return nil, err
			}
			return into, nil
		}),
		"cmd.ClusterSpec":   apisrvpkg.NewMessage("cmd.ClusterSpec"),
		"cmd.ClusterStatus": apisrvpkg.NewMessage("cmd.ClusterStatus"),
		"cmd.Node": apisrvpkg.NewMessage("cmd.Node").WithKeyGenerator(func(i interface{}, prefix string) string {
			if i == nil {
				r := cmd.Node{}
				return r.MakeKey(prefix)
			}
			r := i.(cmd.Node)
			return r.MakeKey(prefix)
		}).WithObjectVersionWriter(func(i interface{}, version string) interface{} {
			r := i.(cmd.Node)
			r.APIVersion = version
			return r
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create, ignoreStatus bool) (interface{}, error) {
			r := i.(cmd.Node)
			key := r.MakeKey(prefix)
			r.Kind = "Node"
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				if ignoreStatus {
					updateFunc := func(obj runtime.Object) (runtime.Object, error) {
						saved := obj.(*cmd.Node)
						if r.ResourceVersion != "" && r.ResourceVersion != saved.ResourceVersion {
							return nil, fmt.Errorf("Resource Version specified does not match Object version")
						}
						r.Status = saved.Status
						return &r, nil
					}
					into := &cmd.Node{}
					err = kvs.ConsistentUpdate(ctx, key, into, updateFunc)
				} else {
					if r.ResourceVersion != "" {
						logger.Infof("resource version is specified %s\n", r.ResourceVersion)
						err = kvs.Update(ctx, key, &r, kvstore.Compare(kvstore.WithVersion(key), "=", r.ResourceVersion))
					} else {
						err = kvs.Update(ctx, key, &r)
					}
					err = errors.Wrap(err, "KV update failed")
				}
			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, txn kvstore.Txn, i interface{}, prefix string, create bool) error {
			r := i.(cmd.Node)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = txn.Create(key, &r)
				err = errors.Wrap(err, "KV transaction create failed")
			} else {
				err = txn.Update(key, &r)
				err = errors.Wrap(err, "KV transaction update failed")
			}
			return err
		}).WithUUIDWriter(func(i interface{}) (interface{}, error) {
			r := i.(cmd.Node)
			r.UUID = uuid.NewV4().String()
			return r, nil
		}).WithCreationTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(cmd.Node)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.CreationTime.Timestamp = *ts
			}
			return r, err
		}).WithModTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(cmd.Node)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.ModTime.Timestamp = *ts
			}
			return r, err
		}).WithSelfLinkWriter(func(path string, i interface{}) (interface{}, error) {
			r := i.(cmd.Node)
			r.SelfLink = path
			return r, nil
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := cmd.Node{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := cmd.Node{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			return txn.Delete(key)
		}).WithValidate(func(i interface{}, ver string, ignoreStatus bool) error {
			r := i.(cmd.Node)
			if !r.Validate(ver, ignoreStatus) {
				return fmt.Errorf("Default Validation failed")
			}
			return nil
		}),
		"cmd.NodeCondition": apisrvpkg.NewMessage("cmd.NodeCondition"),
		"cmd.NodeList": apisrvpkg.NewMessage("cmd.NodeList").WithKvListFunc(func(ctx context.Context, kvs kvstore.Interface, options *api.ListWatchOptions, prefix string) (interface{}, error) {

			into := cmd.NodeList{}
			r := cmd.Node{}
			key := r.MakeKey(prefix)
			err := kvs.List(ctx, key, &into)
			if err != nil {
				return nil, err
			}
			return into, nil
		}),
		"cmd.NodeSpec":      apisrvpkg.NewMessage("cmd.NodeSpec"),
		"cmd.NodeStatus":    apisrvpkg.NewMessage("cmd.NodeStatus"),
		"cmd.PortCondition": apisrvpkg.NewMessage("cmd.PortCondition"),
		"cmd.PortSpec":      apisrvpkg.NewMessage("cmd.PortSpec"),
		"cmd.PortStatus":    apisrvpkg.NewMessage("cmd.PortStatus"),
		"cmd.SmartNIC": apisrvpkg.NewMessage("cmd.SmartNIC").WithKeyGenerator(func(i interface{}, prefix string) string {
			if i == nil {
				r := cmd.SmartNIC{}
				return r.MakeKey(prefix)
			}
			r := i.(cmd.SmartNIC)
			return r.MakeKey(prefix)
		}).WithObjectVersionWriter(func(i interface{}, version string) interface{} {
			r := i.(cmd.SmartNIC)
			r.APIVersion = version
			return r
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create, ignoreStatus bool) (interface{}, error) {
			r := i.(cmd.SmartNIC)
			key := r.MakeKey(prefix)
			r.Kind = "SmartNIC"
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				if ignoreStatus {
					updateFunc := func(obj runtime.Object) (runtime.Object, error) {
						saved := obj.(*cmd.SmartNIC)
						if r.ResourceVersion != "" && r.ResourceVersion != saved.ResourceVersion {
							return nil, fmt.Errorf("Resource Version specified does not match Object version")
						}
						r.Status = saved.Status
						return &r, nil
					}
					into := &cmd.SmartNIC{}
					err = kvs.ConsistentUpdate(ctx, key, into, updateFunc)
				} else {
					if r.ResourceVersion != "" {
						logger.Infof("resource version is specified %s\n", r.ResourceVersion)
						err = kvs.Update(ctx, key, &r, kvstore.Compare(kvstore.WithVersion(key), "=", r.ResourceVersion))
					} else {
						err = kvs.Update(ctx, key, &r)
					}
					err = errors.Wrap(err, "KV update failed")
				}
			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, txn kvstore.Txn, i interface{}, prefix string, create bool) error {
			r := i.(cmd.SmartNIC)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = txn.Create(key, &r)
				err = errors.Wrap(err, "KV transaction create failed")
			} else {
				err = txn.Update(key, &r)
				err = errors.Wrap(err, "KV transaction update failed")
			}
			return err
		}).WithUUIDWriter(func(i interface{}) (interface{}, error) {
			r := i.(cmd.SmartNIC)
			r.UUID = uuid.NewV4().String()
			return r, nil
		}).WithCreationTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(cmd.SmartNIC)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.CreationTime.Timestamp = *ts
			}
			return r, err
		}).WithModTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(cmd.SmartNIC)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.ModTime.Timestamp = *ts
			}
			return r, err
		}).WithSelfLinkWriter(func(path string, i interface{}) (interface{}, error) {
			r := i.(cmd.SmartNIC)
			r.SelfLink = path
			return r, nil
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := cmd.SmartNIC{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := cmd.SmartNIC{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			return txn.Delete(key)
		}).WithValidate(func(i interface{}, ver string, ignoreStatus bool) error {
			r := i.(cmd.SmartNIC)
			if !r.Validate(ver, ignoreStatus) {
				return fmt.Errorf("Default Validation failed")
			}
			return nil
		}),
		"cmd.SmartNICCondition": apisrvpkg.NewMessage("cmd.SmartNICCondition"),
		"cmd.SmartNICList": apisrvpkg.NewMessage("cmd.SmartNICList").WithKvListFunc(func(ctx context.Context, kvs kvstore.Interface, options *api.ListWatchOptions, prefix string) (interface{}, error) {

			into := cmd.SmartNICList{}
			r := cmd.SmartNIC{}
			key := r.MakeKey(prefix)
			err := kvs.List(ctx, key, &into)
			if err != nil {
				return nil, err
			}
			return into, nil
		}),
		"cmd.SmartNICSpec":   apisrvpkg.NewMessage("cmd.SmartNICSpec"),
		"cmd.SmartNICStatus": apisrvpkg.NewMessage("cmd.SmartNICStatus"),
		// Add a message handler for ListWatch options
		"api.ListWatchOptions": apisrvpkg.NewMessage("api.ListWatchOptions"),
	}

	scheme.AddKnownTypes(
		&cmd.Cluster{},
		&cmd.Node{},
		&cmd.SmartNIC{},
	)

	apisrv.RegisterMessages("cmd", s.Messages)

	{
		srv := apisrvpkg.NewService("CmdV1")

		s.endpointsCmdV1.fnAutoAddCluster = srv.AddMethod("AutoAddCluster",
			apisrvpkg.NewMethod(s.Messages["cmd.Cluster"], s.Messages["cmd.Cluster"], "cmd", "AutoAddCluster")).WithOper(apiserver.CreateOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			return "", fmt.Errorf("not rest endpoint")
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoAddNode = srv.AddMethod("AutoAddNode",
			apisrvpkg.NewMethod(s.Messages["cmd.Node"], s.Messages["cmd.Node"], "cmd", "AutoAddNode")).WithOper(apiserver.CreateOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(cmd.Node)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/nodes/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoAddSmartNIC = srv.AddMethod("AutoAddSmartNIC",
			apisrvpkg.NewMethod(s.Messages["cmd.SmartNIC"], s.Messages["cmd.SmartNIC"], "cmd", "AutoAddSmartNIC")).WithOper(apiserver.CreateOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(cmd.SmartNIC)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/smartnics/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoDeleteCluster = srv.AddMethod("AutoDeleteCluster",
			apisrvpkg.NewMethod(s.Messages["cmd.Cluster"], s.Messages["cmd.Cluster"], "cmd", "AutoDeleteCluster")).WithOper(apiserver.DeleteOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(cmd.Cluster)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/cluster/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoDeleteNode = srv.AddMethod("AutoDeleteNode",
			apisrvpkg.NewMethod(s.Messages["cmd.Node"], s.Messages["cmd.Node"], "cmd", "AutoDeleteNode")).WithOper(apiserver.DeleteOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(cmd.Node)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/nodes/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoDeleteSmartNIC = srv.AddMethod("AutoDeleteSmartNIC",
			apisrvpkg.NewMethod(s.Messages["cmd.SmartNIC"], s.Messages["cmd.SmartNIC"], "cmd", "AutoDeleteSmartNIC")).WithOper(apiserver.DeleteOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(cmd.SmartNIC)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/smartnics/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoGetCluster = srv.AddMethod("AutoGetCluster",
			apisrvpkg.NewMethod(s.Messages["cmd.Cluster"], s.Messages["cmd.Cluster"], "cmd", "AutoGetCluster")).WithOper(apiserver.GetOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(cmd.Cluster)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/cluster/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoGetNode = srv.AddMethod("AutoGetNode",
			apisrvpkg.NewMethod(s.Messages["cmd.Node"], s.Messages["cmd.Node"], "cmd", "AutoGetNode")).WithOper(apiserver.GetOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(cmd.Node)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/nodes/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoGetSmartNIC = srv.AddMethod("AutoGetSmartNIC",
			apisrvpkg.NewMethod(s.Messages["cmd.SmartNIC"], s.Messages["cmd.SmartNIC"], "cmd", "AutoGetSmartNIC")).WithOper(apiserver.GetOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(cmd.SmartNIC)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/smartnics/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoListCluster = srv.AddMethod("AutoListCluster",
			apisrvpkg.NewMethod(s.Messages["api.ListWatchOptions"], s.Messages["cmd.ClusterList"], "cmd", "AutoListCluster")).WithOper(apiserver.ListOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(api.ListWatchOptions)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/cluster/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoListNode = srv.AddMethod("AutoListNode",
			apisrvpkg.NewMethod(s.Messages["api.ListWatchOptions"], s.Messages["cmd.NodeList"], "cmd", "AutoListNode")).WithOper(apiserver.ListOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(api.ListWatchOptions)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/nodes/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoListSmartNIC = srv.AddMethod("AutoListSmartNIC",
			apisrvpkg.NewMethod(s.Messages["api.ListWatchOptions"], s.Messages["cmd.SmartNICList"], "cmd", "AutoListSmartNIC")).WithOper(apiserver.ListOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(api.ListWatchOptions)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/smartnics/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoUpdateCluster = srv.AddMethod("AutoUpdateCluster",
			apisrvpkg.NewMethod(s.Messages["cmd.Cluster"], s.Messages["cmd.Cluster"], "cmd", "AutoUpdateCluster")).WithOper(apiserver.UpdateOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(cmd.Cluster)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/cluster/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoUpdateNode = srv.AddMethod("AutoUpdateNode",
			apisrvpkg.NewMethod(s.Messages["cmd.Node"], s.Messages["cmd.Node"], "cmd", "AutoUpdateNode")).WithOper(apiserver.UpdateOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(cmd.Node)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/nodes/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoUpdateSmartNIC = srv.AddMethod("AutoUpdateSmartNIC",
			apisrvpkg.NewMethod(s.Messages["cmd.SmartNIC"], s.Messages["cmd.SmartNIC"], "cmd", "AutoUpdateSmartNIC")).WithOper(apiserver.UpdateOper).WithVersion("v1").WithMakeURI(func(i interface{}) (string, error) {
			in, ok := i.(cmd.SmartNIC)
			if !ok {
				return "", fmt.Errorf("wrong type")
			}
			return fmt.Sprint("/v1/", "cmd/smartnics/", in.Name), nil
		}).HandleInvocation

		s.endpointsCmdV1.fnAutoWatchCluster = s.Messages["cmd.Cluster"].WatchFromKv

		s.endpointsCmdV1.fnAutoWatchNode = s.Messages["cmd.Node"].WatchFromKv

		s.endpointsCmdV1.fnAutoWatchSmartNIC = s.Messages["cmd.SmartNIC"].WatchFromKv

		s.Services = map[string]apiserver.Service{
			"cmd.CmdV1": srv,
		}
		apisrv.RegisterService("cmd.CmdV1", srv)
		endpoints := cmd.MakeCmdV1ServerEndpoints(s.endpointsCmdV1, logger)
		server := cmd.MakeGRPCServerCmdV1(ctx, endpoints, logger)
		cmd.RegisterCmdV1Server(grpcserver.GrpcServer, server)
	}
	// Add Watchers
	{

		s.Messages["cmd.Cluster"].WithKvWatchFunc(func(l log.Logger, options *api.ListWatchOptions, kvs kvstore.Interface, stream interface{}, txfn func(from, to string, i interface{}) (interface{}, error), version, svcprefix string) error {
			o := cmd.Cluster{}
			key := o.MakeKey(svcprefix)
			if strings.HasSuffix(key, "//") {
				key = strings.TrimSuffix(key, "/")
			}
			wstream := stream.(cmd.CmdV1_AutoWatchClusterServer)
			nctx, cancel := context.WithCancel(wstream.Context())
			defer cancel()
			if kvs == nil {
				return fmt.Errorf("Nil KVS")
			}
			watcher, err := kvs.PrefixWatch(nctx, key, options.ResourceVersion)
			if err != nil {
				l.ErrorLog("msg", "error starting Watch on KV", "error", err, "object", "Cluster")
				return err
			}
			for {
				select {
				case ev, ok := <-watcher.EventChan():
					if !ok {
						l.DebugLog("Channel closed for Cluster Watcher")
						return nil
					}
					in, ok := ev.Object.(*cmd.Cluster)
					if !ok {
						status, ok := ev.Object.(*api.Status)
						if !ok {
							return errors.New("unknown error")
						}
						return fmt.Errorf("%v:(%s) %s", status.Code, status.Result, status.Message)
					}
					strEvent := cmd.AutoMsgClusterWatchHelper{
						Type:   string(ev.Type),
						Object: in,
					}
					l.DebugLog("msg", "received Cluster watch event from KV", "type", ev.Type)
					if version != in.APIVersion {
						i, err := txfn(in.APIVersion, version, in)
						if err != nil {
							l.ErrorLog("msg", "Failed to transform message", "type", "Cluster", "fromver", in.APIVersion, "tover", version)
							break
						}
						strEvent.Object = i.(*cmd.Cluster)
					}
					l.DebugLog("msg", "writing to stream")
					if err := wstream.Send(&strEvent); err != nil {
						l.DebugLog("msg", "Stream send error'ed for Cluster", "error", err)
						return err
					}
				case <-nctx.Done():
					l.DebugLog("msg", "Context cancelled for Cluster Watcher")
					return wstream.Context().Err()
				}
			}
		})

		s.Messages["cmd.Node"].WithKvWatchFunc(func(l log.Logger, options *api.ListWatchOptions, kvs kvstore.Interface, stream interface{}, txfn func(from, to string, i interface{}) (interface{}, error), version, svcprefix string) error {
			o := cmd.Node{}
			key := o.MakeKey(svcprefix)
			if strings.HasSuffix(key, "//") {
				key = strings.TrimSuffix(key, "/")
			}
			wstream := stream.(cmd.CmdV1_AutoWatchNodeServer)
			nctx, cancel := context.WithCancel(wstream.Context())
			defer cancel()
			if kvs == nil {
				return fmt.Errorf("Nil KVS")
			}
			watcher, err := kvs.PrefixWatch(nctx, key, options.ResourceVersion)
			if err != nil {
				l.ErrorLog("msg", "error starting Watch on KV", "error", err, "object", "Node")
				return err
			}
			for {
				select {
				case ev, ok := <-watcher.EventChan():
					if !ok {
						l.DebugLog("Channel closed for Node Watcher")
						return nil
					}
					in, ok := ev.Object.(*cmd.Node)
					if !ok {
						status, ok := ev.Object.(*api.Status)
						if !ok {
							return errors.New("unknown error")
						}
						return fmt.Errorf("%v:(%s) %s", status.Code, status.Result, status.Message)
					}
					strEvent := cmd.AutoMsgNodeWatchHelper{
						Type:   string(ev.Type),
						Object: in,
					}
					l.DebugLog("msg", "received Node watch event from KV", "type", ev.Type)
					if version != in.APIVersion {
						i, err := txfn(in.APIVersion, version, in)
						if err != nil {
							l.ErrorLog("msg", "Failed to transform message", "type", "Node", "fromver", in.APIVersion, "tover", version)
							break
						}
						strEvent.Object = i.(*cmd.Node)
					}
					l.DebugLog("msg", "writing to stream")
					if err := wstream.Send(&strEvent); err != nil {
						l.DebugLog("msg", "Stream send error'ed for Node", "error", err)
						return err
					}
				case <-nctx.Done():
					l.DebugLog("msg", "Context cancelled for Node Watcher")
					return wstream.Context().Err()
				}
			}
		})

		s.Messages["cmd.SmartNIC"].WithKvWatchFunc(func(l log.Logger, options *api.ListWatchOptions, kvs kvstore.Interface, stream interface{}, txfn func(from, to string, i interface{}) (interface{}, error), version, svcprefix string) error {
			o := cmd.SmartNIC{}
			key := o.MakeKey(svcprefix)
			if strings.HasSuffix(key, "//") {
				key = strings.TrimSuffix(key, "/")
			}
			wstream := stream.(cmd.CmdV1_AutoWatchSmartNICServer)
			nctx, cancel := context.WithCancel(wstream.Context())
			defer cancel()
			if kvs == nil {
				return fmt.Errorf("Nil KVS")
			}
			watcher, err := kvs.PrefixWatch(nctx, key, options.ResourceVersion)
			if err != nil {
				l.ErrorLog("msg", "error starting Watch on KV", "error", err, "object", "SmartNIC")
				return err
			}
			for {
				select {
				case ev, ok := <-watcher.EventChan():
					if !ok {
						l.DebugLog("Channel closed for SmartNIC Watcher")
						return nil
					}
					in, ok := ev.Object.(*cmd.SmartNIC)
					if !ok {
						status, ok := ev.Object.(*api.Status)
						if !ok {
							return errors.New("unknown error")
						}
						return fmt.Errorf("%v:(%s) %s", status.Code, status.Result, status.Message)
					}
					strEvent := cmd.AutoMsgSmartNICWatchHelper{
						Type:   string(ev.Type),
						Object: in,
					}
					l.DebugLog("msg", "received SmartNIC watch event from KV", "type", ev.Type)
					if version != in.APIVersion {
						i, err := txfn(in.APIVersion, version, in)
						if err != nil {
							l.ErrorLog("msg", "Failed to transform message", "type", "SmartNIC", "fromver", in.APIVersion, "tover", version)
							break
						}
						strEvent.Object = i.(*cmd.SmartNIC)
					}
					l.DebugLog("msg", "writing to stream")
					if err := wstream.Send(&strEvent); err != nil {
						l.DebugLog("msg", "Stream send error'ed for SmartNIC", "error", err)
						return err
					}
				case <-nctx.Done():
					l.DebugLog("msg", "Context cancelled for SmartNIC Watcher")
					return wstream.Context().Err()
				}
			}
		})

	}

	return nil
}

func (e *eCmdV1Endpoints) AutoAddCluster(ctx context.Context, t cmd.Cluster) (cmd.Cluster, error) {
	r, err := e.fnAutoAddCluster(ctx, t)
	if err == nil {
		return r.(cmd.Cluster), err
	}
	return cmd.Cluster{}, err

}
func (e *eCmdV1Endpoints) AutoAddNode(ctx context.Context, t cmd.Node) (cmd.Node, error) {
	r, err := e.fnAutoAddNode(ctx, t)
	if err == nil {
		return r.(cmd.Node), err
	}
	return cmd.Node{}, err

}
func (e *eCmdV1Endpoints) AutoAddSmartNIC(ctx context.Context, t cmd.SmartNIC) (cmd.SmartNIC, error) {
	r, err := e.fnAutoAddSmartNIC(ctx, t)
	if err == nil {
		return r.(cmd.SmartNIC), err
	}
	return cmd.SmartNIC{}, err

}
func (e *eCmdV1Endpoints) AutoDeleteCluster(ctx context.Context, t cmd.Cluster) (cmd.Cluster, error) {
	r, err := e.fnAutoDeleteCluster(ctx, t)
	if err == nil {
		return r.(cmd.Cluster), err
	}
	return cmd.Cluster{}, err

}
func (e *eCmdV1Endpoints) AutoDeleteNode(ctx context.Context, t cmd.Node) (cmd.Node, error) {
	r, err := e.fnAutoDeleteNode(ctx, t)
	if err == nil {
		return r.(cmd.Node), err
	}
	return cmd.Node{}, err

}
func (e *eCmdV1Endpoints) AutoDeleteSmartNIC(ctx context.Context, t cmd.SmartNIC) (cmd.SmartNIC, error) {
	r, err := e.fnAutoDeleteSmartNIC(ctx, t)
	if err == nil {
		return r.(cmd.SmartNIC), err
	}
	return cmd.SmartNIC{}, err

}
func (e *eCmdV1Endpoints) AutoGetCluster(ctx context.Context, t cmd.Cluster) (cmd.Cluster, error) {
	r, err := e.fnAutoGetCluster(ctx, t)
	if err == nil {
		return r.(cmd.Cluster), err
	}
	return cmd.Cluster{}, err

}
func (e *eCmdV1Endpoints) AutoGetNode(ctx context.Context, t cmd.Node) (cmd.Node, error) {
	r, err := e.fnAutoGetNode(ctx, t)
	if err == nil {
		return r.(cmd.Node), err
	}
	return cmd.Node{}, err

}
func (e *eCmdV1Endpoints) AutoGetSmartNIC(ctx context.Context, t cmd.SmartNIC) (cmd.SmartNIC, error) {
	r, err := e.fnAutoGetSmartNIC(ctx, t)
	if err == nil {
		return r.(cmd.SmartNIC), err
	}
	return cmd.SmartNIC{}, err

}
func (e *eCmdV1Endpoints) AutoListCluster(ctx context.Context, t api.ListWatchOptions) (cmd.ClusterList, error) {
	r, err := e.fnAutoListCluster(ctx, t)
	if err == nil {
		return r.(cmd.ClusterList), err
	}
	return cmd.ClusterList{}, err

}
func (e *eCmdV1Endpoints) AutoListNode(ctx context.Context, t api.ListWatchOptions) (cmd.NodeList, error) {
	r, err := e.fnAutoListNode(ctx, t)
	if err == nil {
		return r.(cmd.NodeList), err
	}
	return cmd.NodeList{}, err

}
func (e *eCmdV1Endpoints) AutoListSmartNIC(ctx context.Context, t api.ListWatchOptions) (cmd.SmartNICList, error) {
	r, err := e.fnAutoListSmartNIC(ctx, t)
	if err == nil {
		return r.(cmd.SmartNICList), err
	}
	return cmd.SmartNICList{}, err

}
func (e *eCmdV1Endpoints) AutoUpdateCluster(ctx context.Context, t cmd.Cluster) (cmd.Cluster, error) {
	r, err := e.fnAutoUpdateCluster(ctx, t)
	if err == nil {
		return r.(cmd.Cluster), err
	}
	return cmd.Cluster{}, err

}
func (e *eCmdV1Endpoints) AutoUpdateNode(ctx context.Context, t cmd.Node) (cmd.Node, error) {
	r, err := e.fnAutoUpdateNode(ctx, t)
	if err == nil {
		return r.(cmd.Node), err
	}
	return cmd.Node{}, err

}
func (e *eCmdV1Endpoints) AutoUpdateSmartNIC(ctx context.Context, t cmd.SmartNIC) (cmd.SmartNIC, error) {
	r, err := e.fnAutoUpdateSmartNIC(ctx, t)
	if err == nil {
		return r.(cmd.SmartNIC), err
	}
	return cmd.SmartNIC{}, err

}

func (e *eCmdV1Endpoints) AutoWatchCluster(in *api.ListWatchOptions, stream cmd.CmdV1_AutoWatchClusterServer) error {
	return e.fnAutoWatchCluster(in, stream, "cmd")
}
func (e *eCmdV1Endpoints) AutoWatchNode(in *api.ListWatchOptions, stream cmd.CmdV1_AutoWatchNodeServer) error {
	return e.fnAutoWatchNode(in, stream, "cmd")
}
func (e *eCmdV1Endpoints) AutoWatchSmartNIC(in *api.ListWatchOptions, stream cmd.CmdV1_AutoWatchSmartNICServer) error {
	return e.fnAutoWatchSmartNIC(in, stream, "cmd")
}

func init() {
	apisrv = apisrvpkg.MustGetAPIServer()

	svc := scmdCmdBackend{}

	{
		e := eCmdV1Endpoints{Svc: svc}
		svc.endpointsCmdV1 = &e
	}
	apisrv.Register("cmd.protos/cmd.proto", &svc)
}
