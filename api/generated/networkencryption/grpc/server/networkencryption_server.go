// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package networkencryptionApiServer is a auto generated package.
Input file: protos/networkencryption.proto
*/
package networkencryptionApiServer

import (
	"context"
	"fmt"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/pensando/sw/api"
	networkencryption "github.com/pensando/sw/api/generated/networkencryption"
	"github.com/pensando/sw/api/listerwatcher"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pkg/errors"
	"github.com/satori/go.uuid"
	"google.golang.org/grpc"
)

// dummy vars to suppress unused errors
var _ api.ObjectMeta
var _ listerwatcher.WatcherClient
var _ fmt.Stringer

type snetworkencryptionNetworkencryptionBackend struct {
	Services map[string]apiserver.Service
	Messages map[string]apiserver.Message

	endpointsTrafficEncryptionPolicyV1 *eTrafficEncryptionPolicyV1Endpoints
}

type eTrafficEncryptionPolicyV1Endpoints struct {
	Svc snetworkencryptionNetworkencryptionBackend

	fnAutoAddTrafficEncryptionPolicy    func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoDeleteTrafficEncryptionPolicy func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoGetTrafficEncryptionPolicy    func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoListTrafficEncryptionPolicy   func(ctx context.Context, t interface{}) (interface{}, error)
	fnAutoUpdateTrafficEncryptionPolicy func(ctx context.Context, t interface{}) (interface{}, error)

	fnAutoWatchTrafficEncryptionPolicy func(in *api.ListWatchOptions, stream grpc.ServerStream, svcprefix string) error
}

func (s *snetworkencryptionNetworkencryptionBackend) CompleteRegistration(ctx context.Context, logger log.Logger,
	grpcserver *rpckit.RPCServer, scheme *runtime.Scheme) error {
	s.Messages = map[string]apiserver.Message{

		"networkencryption.AutoMsgTrafficEncryptionPolicyWatchHelper": apisrvpkg.NewMessage("networkencryption.AutoMsgTrafficEncryptionPolicyWatchHelper"),
		"networkencryption.IPsecProtocolSpec":                         apisrvpkg.NewMessage("networkencryption.IPsecProtocolSpec"),
		"networkencryption.TLSProtocolSpec":                           apisrvpkg.NewMessage("networkencryption.TLSProtocolSpec"),
		"networkencryption.TrafficEncryptionPolicy": apisrvpkg.NewMessage("networkencryption.TrafficEncryptionPolicy").WithKeyGenerator(func(i interface{}, prefix string) string {
			if i == nil {
				r := networkencryption.TrafficEncryptionPolicy{}
				return r.MakeKey(prefix)
			}
			r := i.(networkencryption.TrafficEncryptionPolicy)
			return r.MakeKey(prefix)
		}).WithObjectVersionWriter(func(i interface{}, version string) interface{} {
			r := i.(networkencryption.TrafficEncryptionPolicy)
			r.APIVersion = version
			return r
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create, ignoreStatus bool) (interface{}, error) {
			r := i.(networkencryption.TrafficEncryptionPolicy)
			key := r.MakeKey(prefix)
			r.Kind = "TrafficEncryptionPolicy"
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				if ignoreStatus {
					updateFunc := func(obj runtime.Object) (runtime.Object, error) {
						saved := obj.(*networkencryption.TrafficEncryptionPolicy)
						if r.ResourceVersion != "" && r.ResourceVersion != saved.ResourceVersion {
							return nil, fmt.Errorf("Resource Version specified does not match Object version")
						}
						r.Status = saved.Status
						return &r, nil
					}
					into := &networkencryption.TrafficEncryptionPolicy{}
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
			r := i.(networkencryption.TrafficEncryptionPolicy)
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
			r := i.(networkencryption.TrafficEncryptionPolicy)
			r.UUID = uuid.NewV4().String()
			return r, nil
		}).WithCreationTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(networkencryption.TrafficEncryptionPolicy)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.CreationTime.Timestamp = *ts
			}
			return r, err
		}).WithModTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(networkencryption.TrafficEncryptionPolicy)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.ModTime.Timestamp = *ts
			}
			return r, err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := networkencryption.TrafficEncryptionPolicy{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := networkencryption.TrafficEncryptionPolicy{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			return txn.Delete(key)
		}).WithValidate(func(i interface{}, ver string, ignoreStatus bool) error {
			r := i.(networkencryption.TrafficEncryptionPolicy)
			if !r.Validate(ver, ignoreStatus) {
				return fmt.Errorf("Default Validation failed")
			}
			return nil
		}),
		"networkencryption.TrafficEncryptionPolicyList": apisrvpkg.NewMessage("networkencryption.TrafficEncryptionPolicyList").WithKvListFunc(func(ctx context.Context, kvs kvstore.Interface, options *api.ListWatchOptions, prefix string) (interface{}, error) {

			into := networkencryption.TrafficEncryptionPolicyList{}
			r := networkencryption.TrafficEncryptionPolicy{}
			key := r.MakeKey(prefix)
			err := kvs.List(ctx, key, &into)
			if err != nil {
				return nil, err
			}
			return into, nil
		}),
		"networkencryption.TrafficEncryptionPolicySpec":   apisrvpkg.NewMessage("networkencryption.TrafficEncryptionPolicySpec"),
		"networkencryption.TrafficEncryptionPolicyStatus": apisrvpkg.NewMessage("networkencryption.TrafficEncryptionPolicyStatus"),
		// Add a message handler for ListWatch options
		"api.ListWatchOptions": apisrvpkg.NewMessage("api.ListWatchOptions"),
	}

	scheme.AddKnownTypes(
		&networkencryption.TrafficEncryptionPolicy{},
	)

	apisrv.RegisterMessages("networkencryption", s.Messages)

	{
		srv := apisrvpkg.NewService("TrafficEncryptionPolicyV1")

		s.endpointsTrafficEncryptionPolicyV1.fnAutoAddTrafficEncryptionPolicy = srv.AddMethod("AutoAddTrafficEncryptionPolicy",
			apisrvpkg.NewMethod(s.Messages["networkencryption.TrafficEncryptionPolicy"], s.Messages["networkencryption.TrafficEncryptionPolicy"], "trafficEncryptionPolicy", "AutoAddTrafficEncryptionPolicy")).WithOper(apiserver.CreateOper).WithVersion("v1").HandleInvocation

		s.endpointsTrafficEncryptionPolicyV1.fnAutoDeleteTrafficEncryptionPolicy = srv.AddMethod("AutoDeleteTrafficEncryptionPolicy",
			apisrvpkg.NewMethod(s.Messages["networkencryption.TrafficEncryptionPolicy"], s.Messages["networkencryption.TrafficEncryptionPolicy"], "trafficEncryptionPolicy", "AutoDeleteTrafficEncryptionPolicy")).WithOper(apiserver.DeleteOper).WithVersion("v1").HandleInvocation

		s.endpointsTrafficEncryptionPolicyV1.fnAutoGetTrafficEncryptionPolicy = srv.AddMethod("AutoGetTrafficEncryptionPolicy",
			apisrvpkg.NewMethod(s.Messages["networkencryption.TrafficEncryptionPolicy"], s.Messages["networkencryption.TrafficEncryptionPolicy"], "trafficEncryptionPolicy", "AutoGetTrafficEncryptionPolicy")).WithOper(apiserver.GetOper).WithVersion("v1").HandleInvocation

		s.endpointsTrafficEncryptionPolicyV1.fnAutoListTrafficEncryptionPolicy = srv.AddMethod("AutoListTrafficEncryptionPolicy",
			apisrvpkg.NewMethod(s.Messages["api.ListWatchOptions"], s.Messages["networkencryption.TrafficEncryptionPolicyList"], "trafficEncryptionPolicy", "AutoListTrafficEncryptionPolicy")).WithOper(apiserver.ListOper).WithVersion("v1").HandleInvocation

		s.endpointsTrafficEncryptionPolicyV1.fnAutoUpdateTrafficEncryptionPolicy = srv.AddMethod("AutoUpdateTrafficEncryptionPolicy",
			apisrvpkg.NewMethod(s.Messages["networkencryption.TrafficEncryptionPolicy"], s.Messages["networkencryption.TrafficEncryptionPolicy"], "trafficEncryptionPolicy", "AutoUpdateTrafficEncryptionPolicy")).WithOper(apiserver.UpdateOper).WithVersion("v1").HandleInvocation

		s.endpointsTrafficEncryptionPolicyV1.fnAutoWatchTrafficEncryptionPolicy = s.Messages["networkencryption.TrafficEncryptionPolicy"].WatchFromKv

		s.Services = map[string]apiserver.Service{
			"networkencryption.TrafficEncryptionPolicyV1": srv,
		}
		apisrv.RegisterService("networkencryption.TrafficEncryptionPolicyV1", srv)
		endpoints := networkencryption.MakeTrafficEncryptionPolicyV1ServerEndpoints(s.endpointsTrafficEncryptionPolicyV1, logger)
		server := networkencryption.MakeGRPCServerTrafficEncryptionPolicyV1(ctx, endpoints, logger)
		networkencryption.RegisterTrafficEncryptionPolicyV1Server(grpcserver.GrpcServer, server)
	}
	// Add Watchers
	{

		s.Messages["networkencryption.TrafficEncryptionPolicy"].WithKvWatchFunc(func(l log.Logger, options *api.ListWatchOptions, kvs kvstore.Interface, stream interface{}, txfn func(from, to string, i interface{}) (interface{}, error), version, svcprefix string) error {
			o := networkencryption.TrafficEncryptionPolicy{}
			key := o.MakeKey(svcprefix)
			wstream := stream.(networkencryption.TrafficEncryptionPolicyV1_AutoWatchTrafficEncryptionPolicyServer)
			nctx, cancel := context.WithCancel(wstream.Context())
			defer cancel()
			watcher, err := kvs.PrefixWatch(nctx, key, options.ResourceVersion)
			if err != nil {
				l.ErrorLog("msg", "error starting Watch on KV", "error", err, "object", "TrafficEncryptionPolicy")
				return err
			}
			for {
				select {
				case ev, ok := <-watcher.EventChan():
					if !ok {
						l.DebugLog("Channel closed for TrafficEncryptionPolicy Watcher")
						return nil
					}
					in, ok := ev.Object.(*networkencryption.TrafficEncryptionPolicy)
					if !ok {
						status, ok := ev.Object.(*api.Status)
						if !ok {
							return errors.New("unknown error")
						}
						return fmt.Errorf("%v:(%s) %s", status.Code, status.Result, status.Message)
					}
					strEvent := networkencryption.AutoMsgTrafficEncryptionPolicyWatchHelper{
						Type:   string(ev.Type),
						Object: in,
					}
					l.DebugLog("msg", "recieved TrafficEncryptionPolicy watch event from KV", "type", ev.Type)
					if version != in.APIVersion {
						i, err := txfn(in.APIVersion, version, in)
						if err != nil {
							l.ErrorLog("msg", "Failed to transform message", "type", "TrafficEncryptionPolicy", "fromver", in.APIVersion, "tover", version)
							break
						}
						strEvent.Object = i.(*networkencryption.TrafficEncryptionPolicy)
					}
					l.DebugLog("msg", "writing to stream")
					if err := wstream.Send(&strEvent); err != nil {
						l.DebugLog("msg", "Stream send error'ed for TrafficEncryptionPolicy", "error", err)
						return err
					}
				case <-nctx.Done():
					l.DebugLog("msg", "Context cancelled for TrafficEncryptionPolicy Watcher")
					return wstream.Context().Err()
				}
			}
		})

	}

	return nil
}

func (e *eTrafficEncryptionPolicyV1Endpoints) AutoAddTrafficEncryptionPolicy(ctx context.Context, t networkencryption.TrafficEncryptionPolicy) (networkencryption.TrafficEncryptionPolicy, error) {
	r, err := e.fnAutoAddTrafficEncryptionPolicy(ctx, t)
	if err == nil {
		return r.(networkencryption.TrafficEncryptionPolicy), err
	}
	return networkencryption.TrafficEncryptionPolicy{}, err

}
func (e *eTrafficEncryptionPolicyV1Endpoints) AutoDeleteTrafficEncryptionPolicy(ctx context.Context, t networkencryption.TrafficEncryptionPolicy) (networkencryption.TrafficEncryptionPolicy, error) {
	r, err := e.fnAutoDeleteTrafficEncryptionPolicy(ctx, t)
	if err == nil {
		return r.(networkencryption.TrafficEncryptionPolicy), err
	}
	return networkencryption.TrafficEncryptionPolicy{}, err

}
func (e *eTrafficEncryptionPolicyV1Endpoints) AutoGetTrafficEncryptionPolicy(ctx context.Context, t networkencryption.TrafficEncryptionPolicy) (networkencryption.TrafficEncryptionPolicy, error) {
	r, err := e.fnAutoGetTrafficEncryptionPolicy(ctx, t)
	if err == nil {
		return r.(networkencryption.TrafficEncryptionPolicy), err
	}
	return networkencryption.TrafficEncryptionPolicy{}, err

}
func (e *eTrafficEncryptionPolicyV1Endpoints) AutoListTrafficEncryptionPolicy(ctx context.Context, t api.ListWatchOptions) (networkencryption.TrafficEncryptionPolicyList, error) {
	r, err := e.fnAutoListTrafficEncryptionPolicy(ctx, t)
	if err == nil {
		return r.(networkencryption.TrafficEncryptionPolicyList), err
	}
	return networkencryption.TrafficEncryptionPolicyList{}, err

}
func (e *eTrafficEncryptionPolicyV1Endpoints) AutoUpdateTrafficEncryptionPolicy(ctx context.Context, t networkencryption.TrafficEncryptionPolicy) (networkencryption.TrafficEncryptionPolicy, error) {
	r, err := e.fnAutoUpdateTrafficEncryptionPolicy(ctx, t)
	if err == nil {
		return r.(networkencryption.TrafficEncryptionPolicy), err
	}
	return networkencryption.TrafficEncryptionPolicy{}, err

}

func (e *eTrafficEncryptionPolicyV1Endpoints) AutoWatchTrafficEncryptionPolicy(in *api.ListWatchOptions, stream networkencryption.TrafficEncryptionPolicyV1_AutoWatchTrafficEncryptionPolicyServer) error {
	return e.fnAutoWatchTrafficEncryptionPolicy(in, stream, "trafficEncryptionPolicy")
}

func init() {
	apisrv = apisrvpkg.MustGetAPIServer()

	svc := snetworkencryptionNetworkencryptionBackend{}

	{
		e := eTrafficEncryptionPolicyV1Endpoints{Svc: svc}
		svc.endpointsTrafficEncryptionPolicyV1 = &e
	}
	apisrv.Register("networkencryption.protos/networkencryption.proto", &svc)
}
