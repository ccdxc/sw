// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package browserApiServer is a auto generated package.
Input file: browser.proto
*/
package browserApiServer

import (
	"context"
	"fmt"
	"strconv"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	browser "github.com/pensando/sw/api/generated/browser"
	fieldhooks "github.com/pensando/sw/api/hooks/apiserver/fields"
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
var _ fieldhooks.Dummy

type sbrowserBrowserBackend struct {
	Services map[string]apiserver.Service
	Messages map[string]apiserver.Message
	logger   log.Logger
	scheme   *runtime.Scheme
}

func (s *sbrowserBrowserBackend) regMsgsFunc(l log.Logger, scheme *runtime.Scheme) {
	l.Infof("registering message for sbrowserBrowserBackend")
	s.Messages = map[string]apiserver.Message{

		"browser.BrowseRequest": apisrvpkg.NewMessage("browser.BrowseRequest").WithKeyGenerator(func(i interface{}, prefix string) string {
			if i == nil {
				r := browser.BrowseRequest{}
				return r.MakeKey(prefix)
			}
			r := i.(browser.BrowseRequest)
			return r.MakeKey(prefix)
		}).WithObjectVersionWriter(func(i interface{}, version string) interface{} {
			r := i.(browser.BrowseRequest)
			r.Kind = "BrowseRequest"
			r.APIVersion = version
			return r
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool, updateFn kvstore.UpdateFunc) (interface{}, error) {
			r := i.(browser.BrowseRequest)
			key := r.MakeKey(prefix)
			r.Kind = "BrowseRequest"
			var err error
			if create {
				if updateFn != nil {
					upd := &browser.BrowseRequest{}
					n, err := updateFn(upd)
					if err != nil {
						l.ErrorLog("msg", "could not create new object", "err", err)
						return nil, err
					}
					new := n.(*browser.BrowseRequest)
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
					l.ErrorLog("msg", "KV create failed", "key", key, "err", err)
				}
			} else {
				var cur browser.BrowseRequest
				err = kvs.Get(ctx, key, &cur)
				if err != nil {
					l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "err", err)
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
					l.ErrorLog("msg", "KV update failed", "key", key, "err", err)
				}

			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, i interface{}, prefix string, create bool, updatefn kvstore.UpdateFunc) error {
			r := i.(browser.BrowseRequest)
			key := r.MakeKey(prefix)
			var err error
			if create {
				if updatefn != nil {
					upd := &browser.BrowseRequest{}
					n, err := updatefn(upd)
					if err != nil {
						l.ErrorLog("msg", "could not create new object", "err", err)
						return err
					}
					new := n.(*browser.BrowseRequest)
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
					l.ErrorLog("msg", "KV transaction create failed", "key", key, "err", err)
				}
			} else {
				if updatefn != nil {
					var cur browser.BrowseRequest
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "err", err)
						return err
					}
					robj, err := updatefn(&cur)
					if err != nil {
						l.ErrorLog("msg", "unable to update current object", "key", key, "err", err)
						return err
					}
					r = *robj.(*browser.BrowseRequest)
					txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", r.ResourceVersion))
				} else {
					var cur browser.BrowseRequest
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "err", err)
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
					l.ErrorLog("msg", "KV transaction update failed", "key", key, "err", err)
				}
			}
			return err
		}).WithUUIDWriter(func(i interface{}) (interface{}, error) {
			r := i.(browser.BrowseRequest)
			r.UUID = uuid.NewV4().String()
			return r, nil
		}).WithCreationTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(browser.BrowseRequest)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.CreationTime.Timestamp = *ts
			}
			return r, err
		}).WithModTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(browser.BrowseRequest)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.ModTime.Timestamp = *ts
			}
			return r, err
		}).WithSelfLinkWriter(func(path, ver, prefix string, i interface{}) (interface{}, error) {
			r := i.(browser.BrowseRequest)
			r.SelfLink = path
			return r, nil
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := browser.BrowseRequest{}
			err := kvs.Get(ctx, key, &r)
			if err != nil {
				l.ErrorLog("msg", "Object get failed", "key", key, "err", err)
			}
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := browser.BrowseRequest{}
			err := kvs.Delete(ctx, key, &r)
			if err != nil {
				l.ErrorLog("msg", "Object delete failed", "key", key, "err", err)
			}
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			err := txn.Delete(key)
			if err != nil {
				l.ErrorLog("msg", "Object Txn delete failed", "key", key, "err", err)
			}
			return err
		}).WithGetRuntimeObject(func(i interface{}) runtime.Object {
			r := i.(browser.BrowseRequest)
			return &r
		}).WithValidate(func(i interface{}, ver string, ignoreStatus, ignoreSpec bool) []error {
			r := i.(browser.BrowseRequest)
			return r.Validate(ver, "", ignoreStatus, ignoreSpec)
		}).WithNormalizer(func(i interface{}) interface{} {
			r := i.(browser.BrowseRequest)
			r.Normalize()
			return r
		}).WithReferencesGetter(func(i interface{}) (map[string]apiintf.ReferenceObj, error) {
			ret := make(map[string]apiintf.ReferenceObj)
			r := i.(browser.BrowseRequest)

			tenant := ""
			r.References(tenant, "", ret)
			return ret, nil
		}),

		"browser.BrowseRequestList": apisrvpkg.NewMessage("browser.BrowseRequestList").WithKeyGenerator(func(i interface{}, prefix string) string {
			if i == nil {
				r := browser.BrowseRequestList{}
				return r.MakeKey(prefix)
			}
			r := i.(browser.BrowseRequestList)
			return r.MakeKey(prefix)
		}).WithObjectVersionWriter(func(i interface{}, version string) interface{} {
			r := i.(browser.BrowseRequestList)
			r.Kind = "BrowseRequestList"
			r.APIVersion = version
			return r
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool, updateFn kvstore.UpdateFunc) (interface{}, error) {
			r := i.(browser.BrowseRequestList)
			key := r.MakeKey(prefix)
			r.Kind = "BrowseRequestList"
			var err error
			if create {
				if updateFn != nil {
					upd := &browser.BrowseRequestList{}
					n, err := updateFn(upd)
					if err != nil {
						l.ErrorLog("msg", "could not create new object", "err", err)
						return nil, err
					}
					new := n.(*browser.BrowseRequestList)
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
					l.ErrorLog("msg", "KV create failed", "key", key, "err", err)
				}
			} else {
				var cur browser.BrowseRequestList
				err = kvs.Get(ctx, key, &cur)
				if err != nil {
					l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "err", err)
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
					l.ErrorLog("msg", "KV update failed", "key", key, "err", err)
				}

			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, i interface{}, prefix string, create bool, updatefn kvstore.UpdateFunc) error {
			r := i.(browser.BrowseRequestList)
			key := r.MakeKey(prefix)
			var err error
			if create {
				if updatefn != nil {
					upd := &browser.BrowseRequestList{}
					n, err := updatefn(upd)
					if err != nil {
						l.ErrorLog("msg", "could not create new object", "err", err)
						return err
					}
					new := n.(*browser.BrowseRequestList)
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
					l.ErrorLog("msg", "KV transaction create failed", "key", key, "err", err)
				}
			} else {
				if updatefn != nil {
					var cur browser.BrowseRequestList
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "err", err)
						return err
					}
					robj, err := updatefn(&cur)
					if err != nil {
						l.ErrorLog("msg", "unable to update current object", "key", key, "err", err)
						return err
					}
					r = *robj.(*browser.BrowseRequestList)
					txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", r.ResourceVersion))
				} else {
					var cur browser.BrowseRequestList
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "err", err)
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
					l.ErrorLog("msg", "KV transaction update failed", "key", key, "err", err)
				}
			}
			return err
		}).WithUUIDWriter(func(i interface{}) (interface{}, error) {
			r := i.(browser.BrowseRequestList)
			r.UUID = uuid.NewV4().String()
			return r, nil
		}).WithCreationTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(browser.BrowseRequestList)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.CreationTime.Timestamp = *ts
			}
			return r, err
		}).WithModTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(browser.BrowseRequestList)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.ModTime.Timestamp = *ts
			}
			return r, err
		}).WithSelfLinkWriter(func(path, ver, prefix string, i interface{}) (interface{}, error) {
			r := i.(browser.BrowseRequestList)
			r.SelfLink = path
			return r, nil
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := browser.BrowseRequestList{}
			err := kvs.Get(ctx, key, &r)
			if err != nil {
				l.ErrorLog("msg", "Object get failed", "key", key, "err", err)
			}
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := browser.BrowseRequestList{}
			err := kvs.Delete(ctx, key, &r)
			if err != nil {
				l.ErrorLog("msg", "Object delete failed", "key", key, "err", err)
			}
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			err := txn.Delete(key)
			if err != nil {
				l.ErrorLog("msg", "Object Txn delete failed", "key", key, "err", err)
			}
			return err
		}).WithGetRuntimeObject(func(i interface{}) runtime.Object {
			r := i.(browser.BrowseRequestList)
			return &r
		}).WithValidate(func(i interface{}, ver string, ignoreStatus, ignoreSpec bool) []error {
			r := i.(browser.BrowseRequestList)
			return r.Validate(ver, "", ignoreStatus, ignoreSpec)
		}).WithNormalizer(func(i interface{}) interface{} {
			r := i.(browser.BrowseRequestList)
			r.Normalize()
			return r
		}).WithReferencesGetter(func(i interface{}) (map[string]apiintf.ReferenceObj, error) {
			ret := make(map[string]apiintf.ReferenceObj)
			r := i.(browser.BrowseRequestList)

			tenant := ""
			r.References(tenant, "", ret)
			return ret, nil
		}),

		"browser.BrowseRequestObject": apisrvpkg.NewMessage("browser.BrowseRequestObject"),
		"browser.BrowseResponse": apisrvpkg.NewMessage("browser.BrowseResponse").WithKeyGenerator(func(i interface{}, prefix string) string {
			if i == nil {
				r := browser.BrowseResponse{}
				return r.MakeKey(prefix)
			}
			r := i.(browser.BrowseResponse)
			return r.MakeKey(prefix)
		}).WithObjectVersionWriter(func(i interface{}, version string) interface{} {
			r := i.(browser.BrowseResponse)
			r.Kind = "BrowseResponse"
			r.APIVersion = version
			return r
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool, updateFn kvstore.UpdateFunc) (interface{}, error) {
			r := i.(browser.BrowseResponse)
			key := r.MakeKey(prefix)
			r.Kind = "BrowseResponse"
			var err error
			if create {
				if updateFn != nil {
					upd := &browser.BrowseResponse{}
					n, err := updateFn(upd)
					if err != nil {
						l.ErrorLog("msg", "could not create new object", "err", err)
						return nil, err
					}
					new := n.(*browser.BrowseResponse)
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
					l.ErrorLog("msg", "KV create failed", "key", key, "err", err)
				}
			} else {
				var cur browser.BrowseResponse
				err = kvs.Get(ctx, key, &cur)
				if err != nil {
					l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "err", err)
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
					l.ErrorLog("msg", "KV update failed", "key", key, "err", err)
				}

			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, i interface{}, prefix string, create bool, updatefn kvstore.UpdateFunc) error {
			r := i.(browser.BrowseResponse)
			key := r.MakeKey(prefix)
			var err error
			if create {
				if updatefn != nil {
					upd := &browser.BrowseResponse{}
					n, err := updatefn(upd)
					if err != nil {
						l.ErrorLog("msg", "could not create new object", "err", err)
						return err
					}
					new := n.(*browser.BrowseResponse)
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
					l.ErrorLog("msg", "KV transaction create failed", "key", key, "err", err)
				}
			} else {
				if updatefn != nil {
					var cur browser.BrowseResponse
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "err", err)
						return err
					}
					robj, err := updatefn(&cur)
					if err != nil {
						l.ErrorLog("msg", "unable to update current object", "key", key, "err", err)
						return err
					}
					r = *robj.(*browser.BrowseResponse)
					txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", r.ResourceVersion))
				} else {
					var cur browser.BrowseResponse
					err = kvs.Get(ctx, key, &cur)
					if err != nil {
						l.ErrorLog("msg", "trying to update an object that does not exist", "key", key, "err", err)
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
					l.ErrorLog("msg", "KV transaction update failed", "key", key, "err", err)
				}
			}
			return err
		}).WithUUIDWriter(func(i interface{}) (interface{}, error) {
			r := i.(browser.BrowseResponse)
			r.UUID = uuid.NewV4().String()
			return r, nil
		}).WithCreationTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(browser.BrowseResponse)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.CreationTime.Timestamp = *ts
			}
			return r, err
		}).WithModTimeWriter(func(i interface{}) (interface{}, error) {
			r := i.(browser.BrowseResponse)
			var err error
			ts, err := types.TimestampProto(time.Now())
			if err == nil {
				r.ModTime.Timestamp = *ts
			}
			return r, err
		}).WithSelfLinkWriter(func(path, ver, prefix string, i interface{}) (interface{}, error) {
			r := i.(browser.BrowseResponse)
			r.SelfLink = path
			return r, nil
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := browser.BrowseResponse{}
			err := kvs.Get(ctx, key, &r)
			if err != nil {
				l.ErrorLog("msg", "Object get failed", "key", key, "err", err)
			}
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := browser.BrowseResponse{}
			err := kvs.Delete(ctx, key, &r)
			if err != nil {
				l.ErrorLog("msg", "Object delete failed", "key", key, "err", err)
			}
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			err := txn.Delete(key)
			if err != nil {
				l.ErrorLog("msg", "Object Txn delete failed", "key", key, "err", err)
			}
			return err
		}).WithGetRuntimeObject(func(i interface{}) runtime.Object {
			r := i.(browser.BrowseResponse)
			return &r
		}).WithValidate(func(i interface{}, ver string, ignoreStatus, ignoreSpec bool) []error {
			r := i.(browser.BrowseResponse)
			return r.Validate(ver, "", ignoreStatus, ignoreSpec)
		}).WithNormalizer(func(i interface{}) interface{} {
			r := i.(browser.BrowseResponse)
			r.Normalize()
			return r
		}).WithReferencesGetter(func(i interface{}) (map[string]apiintf.ReferenceObj, error) {
			ret := make(map[string]apiintf.ReferenceObj)
			r := i.(browser.BrowseResponse)

			tenant := ""
			r.References(tenant, "", ret)
			return ret, nil
		}),

		"browser.BrowseResponseList":   apisrvpkg.NewMessage("browser.BrowseResponseList"),
		"browser.BrowseResponseObject": apisrvpkg.NewMessage("browser.BrowseResponseObject"),
		"browser.Object":               apisrvpkg.NewMessage("browser.Object"),
		// Add a message handler for ListWatch options
		"api.ListWatchOptions": apisrvpkg.NewMessage("api.ListWatchOptions"),
		// Add a message handler for Label options
		"api.Label": apisrvpkg.NewMessage("api.Label").WithGetRuntimeObject(func(i interface{}) runtime.Object {
			r := i.(api.Label)
			return &r
		}).WithObjectVersionWriter(func(i interface{}, version string) interface{} {
			r := i.(api.Label)
			r.APIVersion = version
			return r
		}),
	}

	apisrv.RegisterMessages("browser", s.Messages)
	// add messages to package.
	if pkgMessages == nil {
		pkgMessages = make(map[string]apiserver.Message)
	}
	for k, v := range s.Messages {
		pkgMessages[k] = v
	}
}

func (s *sbrowserBrowserBackend) regSvcsFunc(ctx context.Context, logger log.Logger, grpcserver *rpckit.RPCServer, scheme *runtime.Scheme) {

}

func (s *sbrowserBrowserBackend) regWatchersFunc(ctx context.Context, logger log.Logger, grpcserver *rpckit.RPCServer, scheme *runtime.Scheme) {

}

func (s *sbrowserBrowserBackend) CompleteRegistration(ctx context.Context, logger log.Logger,
	grpcserver *rpckit.RPCServer, scheme *runtime.Scheme) error {
	// register all messages in the package if not done already
	s.logger = logger
	s.scheme = scheme
	registerMessages(logger, scheme)
	registerServices(ctx, logger, grpcserver, scheme)
	registerWatchers(ctx, logger, grpcserver, scheme)
	return nil
}

func (s *sbrowserBrowserBackend) Reset() {
	cleanupRegistration()
}

func init() {
	apisrv = apisrvpkg.MustGetAPIServer()

	svc := sbrowserBrowserBackend{}
	addMsgRegFunc(svc.regMsgsFunc)
	addSvcRegFunc(svc.regSvcsFunc)
	addWatcherRegFunc(svc.regWatchersFunc)

	apisrv.Register("browser.browser.proto", &svc)
}
