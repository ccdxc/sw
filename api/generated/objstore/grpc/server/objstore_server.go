// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package objstoreApiServer is a auto generated package.
Input file: objstore.proto
*/
package objstoreApiServer

import (
	"context"
	"fmt"
	"strconv"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/pkg/errors"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache"
	objstore "github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/api/interfaces"
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

type sobjstoreObjstoreBackend struct {
	Services map[string]apiserver.Service
	Messages map[string]apiserver.Message
	logger   log.Logger
	scheme   *runtime.Scheme
}

func (s *sobjstoreObjstoreBackend) regMsgsFunc(l log.Logger, scheme *runtime.Scheme) {
	l.Infof("registering message for sobjstoreObjstoreBackend")
	s.Messages = map[string]apiserver.Message{

		"objstore.Bucket": apisrvpkg.NewMessage("objstore.Bucket").WithKeyGenerator(func(i interface{}, prefix string) string {
			if i == nil {
				r := objstore.Bucket{}
				return r.MakeKey(prefix)
			}
			r := i.(objstore.Bucket)
			return r.MakeKey(prefix)
		}).WithObjectVersionWriter(func(i interface{}, version string) interface{} {
			r := i.(objstore.Bucket)
			r.Kind = "Bucket"
			r.APIVersion = version
			return r
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool, updateFn kvstore.UpdateFunc) (interface{}, error) {
			r := i.(objstore.Bucket)
			key := r.MakeKey(prefix)
			r.Kind = "Bucket"
			var err error
			if create {
				if updateFn != nil {
					upd := &objstore.Bucket{}
					n, err := updateFn(upd)
					if err != nil {
						l.ErrorLog("msg", "could not create new object", "error", err)
						return nil, err
					}
					new := n.(*objstore.Bucket)
					new.TypeMeta = r.TypeMeta
					new.GenerationID = "1"
					new.UUID = r.UUID
					new.CreationTime = r.CreationTime
					new.SelfLink = r.SelfLink
					r = *new
				} else {
					r.GenerationID = "1"
				}
				err = kvs.Create(ctx, key, &r)
				if err != nil {
					l.ErrorLog("msg", "KV create failed", "key", key, "error", err)
				}
			} else {
				if updateFn != nil {
					into := &objstore.Bucket{}
					err = kvs.ConsistentUpdate(ctx, key, into, updateFn)
					if err != nil {
						l.ErrorLog("msg", "Consistent update failed", "error", err)
					}
					r = *into
				} else {
					var cur objstore.Bucket
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "error", err)
						return nil, err
					}
					r.UUID = cur.UUID
					r.CreationTime = cur.CreationTime
					if r.ResourceVersion != "" {
						l.Infof("resource version is specified %s\n", r.ResourceVersion)
						err = kvs.Update(ctx, key, &r, kvstore.Compare(kvstore.WithVersion(key), "=", r.ResourceVersion))
					} else {
						err = kvs.Update(ctx, key, &r)
					}
					if err != nil {
						l.ErrorLog("msg", "KV update failed", "key", key, "error", err)
					}
				}

			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, i interface{}, prefix string, create bool, updatefn kvstore.UpdateFunc) error {
			r := i.(objstore.Bucket)
			key := r.MakeKey(prefix)
			var err error
			if create {
				if updatefn != nil {
					upd := &objstore.Bucket{}
					n, err := updatefn(upd)
					if err != nil {
						l.ErrorLog("msg", "could not create new object", "error", err)
						return err
					}
					new := n.(*objstore.Bucket)
					new.TypeMeta = r.TypeMeta
					new.GenerationID = "1"
					new.UUID = r.UUID
					new.CreationTime = r.CreationTime
					new.SelfLink = r.SelfLink
					r = *new
				} else {
					r.GenerationID = "1"
				}
				err = txn.Create(key, &r)
				if err != nil {
					l.ErrorLog("msg", "KV transaction create failed", "key", key, "error", err)
				}
			} else {
				if updatefn != nil {
					var cur objstore.Bucket
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "error", err)
						return err
					}
					robj, err := updatefn(&cur)
					if err != nil {
						l.ErrorLog("msg", "unable to update current object", "key", key, "error", err)
						return err
					}
					r = *robj.(*objstore.Bucket)
					txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", r.ResourceVersion))
				} else {
					var cur objstore.Bucket
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "error", err)
						return err
					}
					r.UUID = cur.UUID
					r.CreationTime = cur.CreationTime
					if _, err := strconv.ParseUint(r.GenerationID, 10, 64); err != nil {
						r.GenerationID = cur.GenerationID
						_, err := strconv.ParseUint(cur.GenerationID, 10, 64)
						if err != nil {
							// Cant recover ID!!, reset ID
							r.GenerationID = "2"
						}
					}
				}
				err = txn.Update(key, &r)
				if err != nil {
					l.ErrorLog("msg", "KV transaction update failed", "key", key, "error", err)
				}
			}
			return err
		}).WithUUIDWriter(func(i interface{}) (interface{}, error) {
			r := i.(objstore.Bucket)
			r.UUID = uuid.NewV4().String()
			return r, nil
		}).WithCreationTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(objstore.Bucket)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.CreationTime.Timestamp = *ts
			}
			return r, err
		}).WithModTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(objstore.Bucket)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.ModTime.Timestamp = *ts
			}
			return r, err
		}).WithSelfLinkWriter(func(path, ver, prefix string, i interface{}) (interface{}, error) {
			r := i.(objstore.Bucket)
			r.SelfLink = path
			return r, nil
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := objstore.Bucket{}
			err := kvs.Get(ctx, key, &r)
			if err != nil {
				l.ErrorLog("msg", "Object get failed", "key", key, "error", err)
			}
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := objstore.Bucket{}
			err := kvs.Delete(ctx, key, &r)
			if err != nil {
				l.ErrorLog("msg", "Object delete failed", "key", key, "error", err)
			}
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			err := txn.Delete(key)
			if err != nil {
				l.ErrorLog("msg", "Object Txn delete failed", "key", key, "error", err)
			}
			return err
		}).WithGetRuntimeObject(func(i interface{}) runtime.Object {
			r := i.(objstore.Bucket)
			return &r
		}).WithValidate(func(i interface{}, ver string, ignoreStatus bool) []error {
			r := i.(objstore.Bucket)
			return r.Validate(ver, "", ignoreStatus)
		}).WithReferencesGetter(func(i interface{}) (map[string]apiintf.ReferenceObj, error) {
			ret := make(map[string]apiintf.ReferenceObj)
			r := i.(objstore.Bucket)

			tenant := ""
			r.References(tenant, "", ret)
			return ret, nil
		}).WithUpdateMetaFunction(func(ctx context.Context, i interface{}, create bool) kvstore.UpdateFunc {
			var n *objstore.Bucket
			if v, ok := i.(objstore.Bucket); ok {
				n = &v
			} else if v, ok := i.(*objstore.Bucket); ok {
				n = v
			} else {
				return nil
			}
			return func(oldObj runtime.Object) (runtime.Object, error) {
				if create {
					n.UUID = uuid.NewV4().String()
					ts, err := types.TimestampProto(time.Now())
					if err != nil {
						return nil, err
					}
					n.CreationTime.Timestamp = *ts
					n.ModTime.Timestamp = *ts
					n.GenerationID = "1"
					return n, nil
				}
				if oldObj == nil {
					return nil, errors.New("nil object")
				}
				o := oldObj.(*objstore.Bucket)
				n.UUID, n.CreationTime, n.Namespace, n.GenerationID = o.UUID, o.CreationTime, o.Namespace, o.GenerationID
				ts, err := types.TimestampProto(time.Now())
				if err != nil {
					return nil, err
				}
				n.ModTime.Timestamp = *ts
				return n, nil
			}
		}).WithReplaceSpecFunction(func(ctx context.Context, i interface{}) kvstore.UpdateFunc {
			var n *objstore.Bucket
			if v, ok := i.(objstore.Bucket); ok {
				n = &v
			} else if v, ok := i.(*objstore.Bucket); ok {
				n = v
			} else {
				return nil
			}
			dryRun := cache.IsDryRun(ctx)
			return func(oldObj runtime.Object) (runtime.Object, error) {
				if oldObj == nil {
					rete := &objstore.Bucket{}
					rete.TypeMeta, rete.ObjectMeta, rete.Spec = n.TypeMeta, n.ObjectMeta, n.Spec
					rete.GenerationID = "1"
					return rete, nil
				}
				if ret, ok := oldObj.(*objstore.Bucket); ok {
					ret.Name, ret.Tenant, ret.Namespace, ret.Labels, ret.ModTime, ret.SelfLink = n.Name, n.Tenant, n.Namespace, n.Labels, n.ModTime, n.SelfLink
					if !dryRun {
						gen, err := strconv.ParseUint(ret.GenerationID, 10, 64)
						if err != nil {
							l.ErrorLog("msg", "invalid GenerationID, reset gen ID", "generation", ret.GenerationID, "error", err)
							ret.GenerationID = "2"
						} else {
							ret.GenerationID = fmt.Sprintf("%d", gen+1)
						}
					}
					ret.Spec = n.Spec
					return ret, nil
				}
				return nil, errors.New("invalid object")
			}
		}).WithReplaceStatusFunction(func(i interface{}) kvstore.UpdateFunc {
			var n *objstore.Bucket
			if v, ok := i.(objstore.Bucket); ok {
				n = &v
			} else if v, ok := i.(*objstore.Bucket); ok {
				n = v
			} else {
				return nil
			}
			return func(oldObj runtime.Object) (runtime.Object, error) {
				if ret, ok := oldObj.(*objstore.Bucket); ok {
					ret.Status = n.Status
					return ret, nil
				}
				return nil, errors.New("invalid object")
			}
		}),

		"objstore.BucketSpec":   apisrvpkg.NewMessage("objstore.BucketSpec"),
		"objstore.BucketStatus": apisrvpkg.NewMessage("objstore.BucketStatus"),
		"objstore.Object": apisrvpkg.NewMessage("objstore.Object").WithKeyGenerator(func(i interface{}, prefix string) string {
			if i == nil {
				r := objstore.Object{}
				return r.MakeKey(prefix)
			}
			r := i.(objstore.Object)
			return r.MakeKey(prefix)
		}).WithObjectVersionWriter(func(i interface{}, version string) interface{} {
			r := i.(objstore.Object)
			r.Kind = "Object"
			r.APIVersion = version
			return r
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool, updateFn kvstore.UpdateFunc) (interface{}, error) {
			r := i.(objstore.Object)
			key := r.MakeKey(prefix)
			r.Kind = "Object"
			var err error
			if create {
				if updateFn != nil {
					upd := &objstore.Object{}
					n, err := updateFn(upd)
					if err != nil {
						l.ErrorLog("msg", "could not create new object", "error", err)
						return nil, err
					}
					new := n.(*objstore.Object)
					new.TypeMeta = r.TypeMeta
					new.GenerationID = "1"
					new.UUID = r.UUID
					new.CreationTime = r.CreationTime
					new.SelfLink = r.SelfLink
					r = *new
				} else {
					r.GenerationID = "1"
				}
				err = kvs.Create(ctx, key, &r)
				if err != nil {
					l.ErrorLog("msg", "KV create failed", "key", key, "error", err)
				}
			} else {
				if updateFn != nil {
					into := &objstore.Object{}
					err = kvs.ConsistentUpdate(ctx, key, into, updateFn)
					if err != nil {
						l.ErrorLog("msg", "Consistent update failed", "error", err)
					}
					r = *into
				} else {
					var cur objstore.Object
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "error", err)
						return nil, err
					}
					r.UUID = cur.UUID
					r.CreationTime = cur.CreationTime
					if r.ResourceVersion != "" {
						l.Infof("resource version is specified %s\n", r.ResourceVersion)
						err = kvs.Update(ctx, key, &r, kvstore.Compare(kvstore.WithVersion(key), "=", r.ResourceVersion))
					} else {
						err = kvs.Update(ctx, key, &r)
					}
					if err != nil {
						l.ErrorLog("msg", "KV update failed", "key", key, "error", err)
					}
				}

			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, i interface{}, prefix string, create bool, updatefn kvstore.UpdateFunc) error {
			r := i.(objstore.Object)
			key := r.MakeKey(prefix)
			var err error
			if create {
				if updatefn != nil {
					upd := &objstore.Object{}
					n, err := updatefn(upd)
					if err != nil {
						l.ErrorLog("msg", "could not create new object", "error", err)
						return err
					}
					new := n.(*objstore.Object)
					new.TypeMeta = r.TypeMeta
					new.GenerationID = "1"
					new.UUID = r.UUID
					new.CreationTime = r.CreationTime
					new.SelfLink = r.SelfLink
					r = *new
				} else {
					r.GenerationID = "1"
				}
				err = txn.Create(key, &r)
				if err != nil {
					l.ErrorLog("msg", "KV transaction create failed", "key", key, "error", err)
				}
			} else {
				if updatefn != nil {
					var cur objstore.Object
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "error", err)
						return err
					}
					robj, err := updatefn(&cur)
					if err != nil {
						l.ErrorLog("msg", "unable to update current object", "key", key, "error", err)
						return err
					}
					r = *robj.(*objstore.Object)
					txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", r.ResourceVersion))
				} else {
					var cur objstore.Object
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "error", err)
						return err
					}
					r.UUID = cur.UUID
					r.CreationTime = cur.CreationTime
					if _, err := strconv.ParseUint(r.GenerationID, 10, 64); err != nil {
						r.GenerationID = cur.GenerationID
						_, err := strconv.ParseUint(cur.GenerationID, 10, 64)
						if err != nil {
							// Cant recover ID!!, reset ID
							r.GenerationID = "2"
						}
					}
				}
				err = txn.Update(key, &r)
				if err != nil {
					l.ErrorLog("msg", "KV transaction update failed", "key", key, "error", err)
				}
			}
			return err
		}).WithUUIDWriter(func(i interface{}) (interface{}, error) {
			r := i.(objstore.Object)
			r.UUID = uuid.NewV4().String()
			return r, nil
		}).WithCreationTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(objstore.Object)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.CreationTime.Timestamp = *ts
			}
			return r, err
		}).WithModTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(objstore.Object)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.ModTime.Timestamp = *ts
			}
			return r, err
		}).WithSelfLinkWriter(func(path, ver, prefix string, i interface{}) (interface{}, error) {
			r := i.(objstore.Object)
			r.SelfLink = path
			return r, nil
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := objstore.Object{}
			err := kvs.Get(ctx, key, &r)
			if err != nil {
				l.ErrorLog("msg", "Object get failed", "key", key, "error", err)
			}
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := objstore.Object{}
			err := kvs.Delete(ctx, key, &r)
			if err != nil {
				l.ErrorLog("msg", "Object delete failed", "key", key, "error", err)
			}
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			err := txn.Delete(key)
			if err != nil {
				l.ErrorLog("msg", "Object Txn delete failed", "key", key, "error", err)
			}
			return err
		}).WithGetRuntimeObject(func(i interface{}) runtime.Object {
			r := i.(objstore.Object)
			return &r
		}).WithValidate(func(i interface{}, ver string, ignoreStatus bool) []error {
			r := i.(objstore.Object)
			return r.Validate(ver, "", ignoreStatus)
		}).WithReferencesGetter(func(i interface{}) (map[string]apiintf.ReferenceObj, error) {
			ret := make(map[string]apiintf.ReferenceObj)
			r := i.(objstore.Object)

			tenant := r.Tenant
			r.References(tenant, "", ret)
			return ret, nil
		}).WithUpdateMetaFunction(func(ctx context.Context, i interface{}, create bool) kvstore.UpdateFunc {
			var n *objstore.Object
			if v, ok := i.(objstore.Object); ok {
				n = &v
			} else if v, ok := i.(*objstore.Object); ok {
				n = v
			} else {
				return nil
			}
			return func(oldObj runtime.Object) (runtime.Object, error) {
				if create {
					n.UUID = uuid.NewV4().String()
					ts, err := types.TimestampProto(time.Now())
					if err != nil {
						return nil, err
					}
					n.CreationTime.Timestamp = *ts
					n.ModTime.Timestamp = *ts
					n.GenerationID = "1"
					return n, nil
				}
				if oldObj == nil {
					return nil, errors.New("nil object")
				}
				o := oldObj.(*objstore.Object)
				n.UUID, n.CreationTime, n.Namespace, n.GenerationID = o.UUID, o.CreationTime, o.Namespace, o.GenerationID
				ts, err := types.TimestampProto(time.Now())
				if err != nil {
					return nil, err
				}
				n.ModTime.Timestamp = *ts
				return n, nil
			}
		}).WithReplaceSpecFunction(func(ctx context.Context, i interface{}) kvstore.UpdateFunc {
			var n *objstore.Object
			if v, ok := i.(objstore.Object); ok {
				n = &v
			} else if v, ok := i.(*objstore.Object); ok {
				n = v
			} else {
				return nil
			}
			dryRun := cache.IsDryRun(ctx)
			return func(oldObj runtime.Object) (runtime.Object, error) {
				if oldObj == nil {
					rete := &objstore.Object{}
					rete.TypeMeta, rete.ObjectMeta, rete.Spec = n.TypeMeta, n.ObjectMeta, n.Spec
					rete.GenerationID = "1"
					return rete, nil
				}
				if ret, ok := oldObj.(*objstore.Object); ok {
					ret.Name, ret.Tenant, ret.Namespace, ret.Labels, ret.ModTime, ret.SelfLink = n.Name, n.Tenant, n.Namespace, n.Labels, n.ModTime, n.SelfLink
					if !dryRun {
						gen, err := strconv.ParseUint(ret.GenerationID, 10, 64)
						if err != nil {
							l.ErrorLog("msg", "invalid GenerationID, reset gen ID", "generation", ret.GenerationID, "error", err)
							ret.GenerationID = "2"
						} else {
							ret.GenerationID = fmt.Sprintf("%d", gen+1)
						}
					}
					ret.Spec = n.Spec
					return ret, nil
				}
				return nil, errors.New("invalid object")
			}
		}).WithReplaceStatusFunction(func(i interface{}) kvstore.UpdateFunc {
			var n *objstore.Object
			if v, ok := i.(objstore.Object); ok {
				n = &v
			} else if v, ok := i.(*objstore.Object); ok {
				n = v
			} else {
				return nil
			}
			return func(oldObj runtime.Object) (runtime.Object, error) {
				if ret, ok := oldObj.(*objstore.Object); ok {
					ret.Status = n.Status
					return ret, nil
				}
				return nil, errors.New("invalid object")
			}
		}),

		"objstore.ObjectSpec":   apisrvpkg.NewMessage("objstore.ObjectSpec"),
		"objstore.ObjectStatus": apisrvpkg.NewMessage("objstore.ObjectStatus"),
		// Add a message handler for ListWatch options
		"api.ListWatchOptions": apisrvpkg.NewMessage("api.ListWatchOptions"),
	}

	apisrv.RegisterMessages("objstore", s.Messages)
	// add messages to package.
	if pkgMessages == nil {
		pkgMessages = make(map[string]apiserver.Message)
	}
	for k, v := range s.Messages {
		pkgMessages[k] = v
	}
}

func (s *sobjstoreObjstoreBackend) regSvcsFunc(ctx context.Context, logger log.Logger, grpcserver *rpckit.RPCServer, scheme *runtime.Scheme) {

}

func (s *sobjstoreObjstoreBackend) regWatchersFunc(ctx context.Context, logger log.Logger, grpcserver *rpckit.RPCServer, scheme *runtime.Scheme) {

}

func (s *sobjstoreObjstoreBackend) CompleteRegistration(ctx context.Context, logger log.Logger,
	grpcserver *rpckit.RPCServer, scheme *runtime.Scheme) error {
	// register all messages in the package if not done already
	s.logger = logger
	s.scheme = scheme
	registerMessages(logger, scheme)
	registerServices(ctx, logger, grpcserver, scheme)
	registerWatchers(ctx, logger, grpcserver, scheme)
	return nil
}

func (s *sobjstoreObjstoreBackend) Reset() {
	cleanupRegistration()
}

func init() {
	apisrv = apisrvpkg.MustGetAPIServer()

	svc := sobjstoreObjstoreBackend{}
	addMsgRegFunc(svc.regMsgsFunc)
	addSvcRegFunc(svc.regSvcsFunc)
	addWatcherRegFunc(svc.regWatchersFunc)

	apisrv.Register("objstore.objstore.proto", &svc)
}
