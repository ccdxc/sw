// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package ctkit is a auto generated package.
Input file: svc_objstore.proto
*/
package ctkit

import (
	"context"
	"errors"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Bucket is a wrapper object that implements additional functionality
type Bucket struct {
	sync.Mutex
	objstore.Bucket
	HandlerCtx interface{} // additional state handlers can store
	ctrler     *ctrlerCtx  // reference back to the controller instance
}

func (obj *Bucket) Write() error {
	// if there is no API server to connect to, we are done
	if (obj.ctrler == nil) || (obj.ctrler.resolver == nil) || obj.ctrler.apisrvURL == "" {
		return nil
	}

	apicl, err := obj.ctrler.apiClient()
	if err != nil {
		obj.ctrler.logger.Errorf("Error creating API server clent. Err: %v", err)
		return err
	}

	obj.ctrler.stats.Counter("Bucket_Writes").Inc()

	// write to api server
	if obj.ObjectMeta.ResourceVersion != "" {
		// update it
		for i := 0; i < maxApisrvWriteRetry; i++ {
			_, err = apicl.ObjstoreV1().Bucket().Update(context.Background(), &obj.Bucket)
			if err == nil {
				break
			}
			time.Sleep(time.Millisecond * 100)
		}
	} else {
		//  create
		_, err = apicl.ObjstoreV1().Bucket().Create(context.Background(), &obj.Bucket)
	}

	return nil
}

// BucketHandler is the event handler for Bucket object
type BucketHandler interface {
	OnBucketCreate(obj *Bucket) error
	OnBucketUpdate(oldObj *Bucket, newObj *objstore.Bucket) error
	OnBucketDelete(obj *Bucket) error
}

// handleBucketEvent handles Bucket events from watcher
func (ct *ctrlerCtx) handleBucketEvent(evt *kvstore.WatchEvent) error {
	switch tp := evt.Object.(type) {
	case *objstore.Bucket:
		eobj := evt.Object.(*objstore.Bucket)
		kind := "Bucket"

		ct.logger.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)

		handler, ok := ct.handlers[kind]
		if !ok {
			ct.logger.Fatalf("Cant find the handler for %s", kind)
		}
		bucketHandler := handler.(BucketHandler)
		// handle based on event type
		switch evt.Type {
		case kvstore.Created:
			fallthrough
		case kvstore.Updated:
			fobj, err := ct.findObject(kind, eobj.GetKey())
			if err != nil {
				obj := &Bucket{
					Bucket:     *eobj,
					HandlerCtx: nil,
					ctrler:     ct,
				}
				ct.addObject(kind, obj.GetKey(), obj)
				ct.stats.Counter("Bucket_Created_Events").Inc()

				// call the event handler
				obj.Lock()
				err = bucketHandler.OnBucketCreate(obj)
				obj.Unlock()
				if err != nil {
					ct.logger.Errorf("Error creating %s %+v. Err: %v", kind, obj, err)
					ct.delObject(kind, eobj.GetKey())
					return err
				}
			} else {
				obj := fobj.(*Bucket)

				ct.stats.Counter("Bucket_Updated_Events").Inc()

				// call the event handler
				obj.Lock()
				err = bucketHandler.OnBucketUpdate(obj, eobj)
				obj.Unlock()
				if err != nil {
					ct.logger.Errorf("Error creating %s %+v. Err: %v", kind, obj, err)
					return err
				}
			}
		case kvstore.Deleted:
			fobj, err := ct.findObject(kind, eobj.GetKey())
			if err != nil {
				ct.logger.Errorf("Object %s/%s not found durng delete. Err: %v", kind, eobj.GetKey(), err)
				return err
			}

			obj := fobj.(*Bucket)

			ct.stats.Counter("Bucket_Deleted_Events").Inc()

			// Call the event reactor
			obj.Lock()
			err = bucketHandler.OnBucketDelete(obj)
			obj.Unlock()
			if err != nil {
				ct.logger.Errorf("Error deleting %s: %+v. Err: %v", kind, obj, err)
			}

			ct.delObject(kind, eobj.GetKey())
		}
	default:
		ct.logger.Fatalf("API watcher Found object of invalid type: %v on Bucket watch channel", tp)
	}

	return nil
}

// diffBucket does a diff of Bucket objects between local cache and API server
func (ct *ctrlerCtx) diffBucket(apicl apiclient.Services) {
	opts := api.ListWatchOptions{}

	// get a list of all objects from API server
	objlist, err := apicl.ObjstoreV1().Bucket().List(context.Background(), &opts)
	if err != nil {
		ct.logger.Errorf("Error getting a list of objects. Err: %v", err)
		return
	}

	ct.logger.Infof("diffBucket(): BucketList returned %d objects", len(objlist))

	// build an object map
	objmap := make(map[string]*objstore.Bucket)
	for _, obj := range objlist {
		objmap[obj.GetKey()] = obj
	}

	// if an object is in our local cache and not in API server, trigger delete for it
	for _, obj := range ct.Bucket().List() {
		_, ok := objmap[obj.GetKey()]
		if !ok {
			ct.logger.Infof("diffBucket(): Deleting existing object %#v since its not in apiserver", obj.GetKey())
			evt := kvstore.WatchEvent{
				Type:   kvstore.Deleted,
				Key:    obj.GetKey(),
				Object: &obj.Bucket,
			}
			ct.handleBucketEvent(&evt)
		}
	}

	// trigger create event for all others
	for _, obj := range objlist {
		ct.logger.Infof("diffBucket(): Adding object %#v", obj.GetKey())
		evt := kvstore.WatchEvent{
			Type:   kvstore.Created,
			Key:    obj.GetKey(),
			Object: obj,
		}
		ct.handleBucketEvent(&evt)
	}
}

func (ct *ctrlerCtx) runBucketWatcher() {
	kind := "Bucket"

	// if there is no API server to connect to, we are done
	if (ct.resolver == nil) || ct.apisrvURL == "" {
		return
	}

	// create context
	ctx, cancel := context.WithCancel(context.Background())
	ct.Lock()
	ct.watchCancel[kind] = cancel
	ct.Unlock()
	opts := api.ListWatchOptions{}

	// setup wait group
	ct.waitGrp.Add(1)
	defer ct.waitGrp.Done()
	logger := ct.logger.WithContext("submodule", "BucketWatcher")

	ct.stats.Counter("Bucket_Watch").Inc()
	defer ct.stats.Counter("Bucket_Watch").Dec()

	// loop forever
	for {
		// create a grpc client
		apicl, err := apiclient.NewGrpcAPIClient(ct.name, ct.apisrvURL, logger, rpckit.WithBalancer(balancer.New(ct.resolver)))
		if err != nil {
			logger.Warnf("Failed to connect to gRPC server [%s]\n", ct.apisrvURL)
			ct.stats.Counter("Bucket_ApiClientErr").Inc()
		} else {
			logger.Infof("API client connected {%+v}", apicl)

			// Bucket object watcher
			wt, werr := apicl.ObjstoreV1().Bucket().Watch(ctx, &opts)
			if werr != nil {
				logger.Errorf("Failed to start %s watch (%s)\n", kind, werr)
				// wait for a second and retry connecting to api server
				apicl.Close()
				time.Sleep(time.Second)
				continue
			}
			ct.Lock()
			ct.watchers[kind] = wt
			ct.Unlock()

			// perform a diff with API server and local cache
			time.Sleep(time.Millisecond * 100)
			ct.diffBucket(apicl)

			// handle api server watch events
		innerLoop:
			for {
				// wait for events
				select {
				case evt, ok := <-wt.EventChan():
					if !ok {
						logger.Error("Error receiving from apisrv watcher")
						ct.stats.Counter("Bucket_WatchErrors").Inc()
						break innerLoop
					}

					// handle event
					ct.handleBucketEvent(evt)
				}
			}
			apicl.Close()
		}

		// if stop flag is set, we are done
		if ct.stoped {
			logger.Infof("Exiting API server watcher")
			return
		}

		// wait for a second and retry connecting to api server
		time.Sleep(time.Second)
	}
}

// WatchBucket starts watch on Bucket object
func (ct *ctrlerCtx) WatchBucket(handler BucketHandler) error {
	kind := "Bucket"

	ct.Lock()
	defer ct.Unlock()

	// see if we already have a watcher
	_, ok := ct.watchers[kind]
	if ok {
		return fmt.Errorf("Bucket watcher already exists")
	}

	// save handler
	ct.handlers[kind] = handler

	// run Bucket watcher in a go routine
	go ct.runBucketWatcher()

	return nil
}

// BucketAPI returns
type BucketAPI interface {
	Create(obj *objstore.Bucket) error
	Update(obj *objstore.Bucket) error
	Delete(obj *objstore.Bucket) error
	Find(meta *api.ObjectMeta) (*Bucket, error)
	List() []*Bucket
	Watch(handler BucketHandler) error
}

// dummy struct that implements BucketAPI
type bucketAPI struct {
	ct *ctrlerCtx
}

// Create creates Bucket object
func (api *bucketAPI) Create(obj *objstore.Bucket) error {
	if api.ct.resolver != nil {
		apicl, err := api.ct.apiClient()
		if err != nil {
			api.ct.logger.Errorf("Error creating API server clent. Err: %v", err)
			return err
		}

		_, err = apicl.ObjstoreV1().Bucket().Create(context.Background(), obj)
		if err != nil && strings.Contains(err.Error(), "AlreadyExists") {
			_, err = apicl.ObjstoreV1().Bucket().Update(context.Background(), obj)
		}
		if err != nil {
			return err
		}
	}

	return api.ct.handleBucketEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Created})
}

// Update triggers update on Bucket object
func (api *bucketAPI) Update(obj *objstore.Bucket) error {
	if api.ct.resolver != nil {
		apicl, err := api.ct.apiClient()
		if err != nil {
			api.ct.logger.Errorf("Error creating API server clent. Err: %v", err)
			return err
		}

		_, err = apicl.ObjstoreV1().Bucket().Update(context.Background(), obj)
		if err != nil {
			return err
		}
	}

	return api.ct.handleBucketEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Updated})
}

// Delete deletes Bucket object
func (api *bucketAPI) Delete(obj *objstore.Bucket) error {
	if api.ct.resolver != nil {
		apicl, err := api.ct.apiClient()
		if err != nil {
			api.ct.logger.Errorf("Error creating API server clent. Err: %v", err)
			return err
		}

		apicl.ObjstoreV1().Bucket().Delete(context.Background(), &obj.ObjectMeta)
	}

	return api.ct.handleBucketEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Deleted})
}

// Find returns an object by meta
func (api *bucketAPI) Find(meta *api.ObjectMeta) (*Bucket, error) {
	// find the object
	obj, err := api.ct.FindObject("Bucket", meta)
	if err != nil {
		return nil, err
	}

	// asset type
	switch obj.(type) {
	case *Bucket:
		hobj := obj.(*Bucket)
		return hobj, nil
	default:
		return nil, errors.New("incorrect object type")
	}
}

// List returns a list of all Bucket objects
func (api *bucketAPI) List() []*Bucket {
	var objlist []*Bucket

	objs := api.ct.ListObjects("Bucket")
	for _, obj := range objs {
		switch tp := obj.(type) {
		case *Bucket:
			eobj := obj.(*Bucket)
			objlist = append(objlist, eobj)
		default:
			log.Fatalf("Got invalid object type %v while looking for Bucket", tp)
		}
	}

	return objlist
}

// Watch sets up a event handlers for Bucket object
func (api *bucketAPI) Watch(handler BucketHandler) error {
	return api.ct.WatchBucket(handler)
}

// Bucket returns BucketAPI
func (ct *ctrlerCtx) Bucket() BucketAPI {
	return &bucketAPI{ct: ct}
}

// Object is a wrapper object that implements additional functionality
type Object struct {
	sync.Mutex
	objstore.Object
	HandlerCtx interface{} // additional state handlers can store
	ctrler     *ctrlerCtx  // reference back to the controller instance
}

func (obj *Object) Write() error {
	// if there is no API server to connect to, we are done
	if (obj.ctrler == nil) || (obj.ctrler.resolver == nil) || obj.ctrler.apisrvURL == "" {
		return nil
	}

	apicl, err := obj.ctrler.apiClient()
	if err != nil {
		obj.ctrler.logger.Errorf("Error creating API server clent. Err: %v", err)
		return err
	}

	obj.ctrler.stats.Counter("Object_Writes").Inc()

	// write to api server
	if obj.ObjectMeta.ResourceVersion != "" {
		// update it
		for i := 0; i < maxApisrvWriteRetry; i++ {
			_, err = apicl.ObjstoreV1().Object().Update(context.Background(), &obj.Object)
			if err == nil {
				break
			}
			time.Sleep(time.Millisecond * 100)
		}
	} else {
		//  create
		_, err = apicl.ObjstoreV1().Object().Create(context.Background(), &obj.Object)
	}

	return nil
}

// ObjectHandler is the event handler for Object object
type ObjectHandler interface {
	OnObjectCreate(obj *Object) error
	OnObjectUpdate(oldObj *Object, newObj *objstore.Object) error
	OnObjectDelete(obj *Object) error
}

// handleObjectEvent handles Object events from watcher
func (ct *ctrlerCtx) handleObjectEvent(evt *kvstore.WatchEvent) error {
	switch tp := evt.Object.(type) {
	case *objstore.Object:
		eobj := evt.Object.(*objstore.Object)
		kind := "Object"

		ct.logger.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)

		handler, ok := ct.handlers[kind]
		if !ok {
			ct.logger.Fatalf("Cant find the handler for %s", kind)
		}
		objectHandler := handler.(ObjectHandler)
		// handle based on event type
		switch evt.Type {
		case kvstore.Created:
			fallthrough
		case kvstore.Updated:
			fobj, err := ct.findObject(kind, eobj.GetKey())
			if err != nil {
				obj := &Object{
					Object:     *eobj,
					HandlerCtx: nil,
					ctrler:     ct,
				}
				ct.addObject(kind, obj.GetKey(), obj)
				ct.stats.Counter("Object_Created_Events").Inc()

				// call the event handler
				obj.Lock()
				err = objectHandler.OnObjectCreate(obj)
				obj.Unlock()
				if err != nil {
					ct.logger.Errorf("Error creating %s %+v. Err: %v", kind, obj, err)
					ct.delObject(kind, eobj.GetKey())
					return err
				}
			} else {
				obj := fobj.(*Object)

				ct.stats.Counter("Object_Updated_Events").Inc()

				// call the event handler
				obj.Lock()
				err = objectHandler.OnObjectUpdate(obj, eobj)
				obj.Unlock()
				if err != nil {
					ct.logger.Errorf("Error creating %s %+v. Err: %v", kind, obj, err)
					return err
				}
			}
		case kvstore.Deleted:
			fobj, err := ct.findObject(kind, eobj.GetKey())
			if err != nil {
				ct.logger.Errorf("Object %s/%s not found durng delete. Err: %v", kind, eobj.GetKey(), err)
				return err
			}

			obj := fobj.(*Object)

			ct.stats.Counter("Object_Deleted_Events").Inc()

			// Call the event reactor
			obj.Lock()
			err = objectHandler.OnObjectDelete(obj)
			obj.Unlock()
			if err != nil {
				ct.logger.Errorf("Error deleting %s: %+v. Err: %v", kind, obj, err)
			}

			ct.delObject(kind, eobj.GetKey())
		}
	default:
		ct.logger.Fatalf("API watcher Found object of invalid type: %v on Object watch channel", tp)
	}

	return nil
}

// diffObject does a diff of Object objects between local cache and API server
func (ct *ctrlerCtx) diffObject(apicl apiclient.Services) {
	opts := api.ListWatchOptions{}

	// get a list of all objects from API server
	objlist, err := apicl.ObjstoreV1().Object().List(context.Background(), &opts)
	if err != nil {
		ct.logger.Errorf("Error getting a list of objects. Err: %v", err)
		return
	}

	ct.logger.Infof("diffObject(): ObjectList returned %d objects", len(objlist))

	// build an object map
	objmap := make(map[string]*objstore.Object)
	for _, obj := range objlist {
		objmap[obj.GetKey()] = obj
	}

	// if an object is in our local cache and not in API server, trigger delete for it
	for _, obj := range ct.Object().List() {
		_, ok := objmap[obj.GetKey()]
		if !ok {
			ct.logger.Infof("diffObject(): Deleting existing object %#v since its not in apiserver", obj.GetKey())
			evt := kvstore.WatchEvent{
				Type:   kvstore.Deleted,
				Key:    obj.GetKey(),
				Object: &obj.Object,
			}
			ct.handleObjectEvent(&evt)
		}
	}

	// trigger create event for all others
	for _, obj := range objlist {
		ct.logger.Infof("diffObject(): Adding object %#v", obj.GetKey())
		evt := kvstore.WatchEvent{
			Type:   kvstore.Created,
			Key:    obj.GetKey(),
			Object: obj,
		}
		ct.handleObjectEvent(&evt)
	}
}

func (ct *ctrlerCtx) runObjectWatcher() {
	kind := "Object"

	// if there is no API server to connect to, we are done
	if (ct.resolver == nil) || ct.apisrvURL == "" {
		return
	}

	// create context
	ctx, cancel := context.WithCancel(context.Background())
	ct.Lock()
	ct.watchCancel[kind] = cancel
	ct.Unlock()
	opts := api.ListWatchOptions{}

	// setup wait group
	ct.waitGrp.Add(1)
	defer ct.waitGrp.Done()
	logger := ct.logger.WithContext("submodule", "ObjectWatcher")

	ct.stats.Counter("Object_Watch").Inc()
	defer ct.stats.Counter("Object_Watch").Dec()

	// loop forever
	for {
		// create a grpc client
		apicl, err := apiclient.NewGrpcAPIClient(ct.name, ct.apisrvURL, logger, rpckit.WithBalancer(balancer.New(ct.resolver)))
		if err != nil {
			logger.Warnf("Failed to connect to gRPC server [%s]\n", ct.apisrvURL)
			ct.stats.Counter("Object_ApiClientErr").Inc()
		} else {
			logger.Infof("API client connected {%+v}", apicl)

			// Object object watcher
			wt, werr := apicl.ObjstoreV1().Object().Watch(ctx, &opts)
			if werr != nil {
				logger.Errorf("Failed to start %s watch (%s)\n", kind, werr)
				// wait for a second and retry connecting to api server
				apicl.Close()
				time.Sleep(time.Second)
				continue
			}
			ct.Lock()
			ct.watchers[kind] = wt
			ct.Unlock()

			// perform a diff with API server and local cache
			time.Sleep(time.Millisecond * 100)
			ct.diffObject(apicl)

			// handle api server watch events
		innerLoop:
			for {
				// wait for events
				select {
				case evt, ok := <-wt.EventChan():
					if !ok {
						logger.Error("Error receiving from apisrv watcher")
						ct.stats.Counter("Object_WatchErrors").Inc()
						break innerLoop
					}

					// handle event
					ct.handleObjectEvent(evt)
				}
			}
			apicl.Close()
		}

		// if stop flag is set, we are done
		if ct.stoped {
			logger.Infof("Exiting API server watcher")
			return
		}

		// wait for a second and retry connecting to api server
		time.Sleep(time.Second)
	}
}

// WatchObject starts watch on Object object
func (ct *ctrlerCtx) WatchObject(handler ObjectHandler) error {
	kind := "Object"

	ct.Lock()
	defer ct.Unlock()

	// see if we already have a watcher
	_, ok := ct.watchers[kind]
	if ok {
		return fmt.Errorf("Object watcher already exists")
	}

	// save handler
	ct.handlers[kind] = handler

	// run Object watcher in a go routine
	go ct.runObjectWatcher()

	return nil
}

// ObjectAPI returns
type ObjectAPI interface {
	Create(obj *objstore.Object) error
	Update(obj *objstore.Object) error
	Delete(obj *objstore.Object) error
	Find(meta *api.ObjectMeta) (*Object, error)
	List() []*Object
	Watch(handler ObjectHandler) error
}

// dummy struct that implements ObjectAPI
type objectAPI struct {
	ct *ctrlerCtx
}

// Create creates Object object
func (api *objectAPI) Create(obj *objstore.Object) error {
	if api.ct.resolver != nil {
		apicl, err := api.ct.apiClient()
		if err != nil {
			api.ct.logger.Errorf("Error creating API server clent. Err: %v", err)
			return err
		}

		_, err = apicl.ObjstoreV1().Object().Create(context.Background(), obj)
		if err != nil && strings.Contains(err.Error(), "AlreadyExists") {
			_, err = apicl.ObjstoreV1().Object().Update(context.Background(), obj)
		}
		if err != nil {
			return err
		}
	}

	return api.ct.handleObjectEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Created})
}

// Update triggers update on Object object
func (api *objectAPI) Update(obj *objstore.Object) error {
	if api.ct.resolver != nil {
		apicl, err := api.ct.apiClient()
		if err != nil {
			api.ct.logger.Errorf("Error creating API server clent. Err: %v", err)
			return err
		}

		_, err = apicl.ObjstoreV1().Object().Update(context.Background(), obj)
		if err != nil {
			return err
		}
	}

	return api.ct.handleObjectEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Updated})
}

// Delete deletes Object object
func (api *objectAPI) Delete(obj *objstore.Object) error {
	if api.ct.resolver != nil {
		apicl, err := api.ct.apiClient()
		if err != nil {
			api.ct.logger.Errorf("Error creating API server clent. Err: %v", err)
			return err
		}

		apicl.ObjstoreV1().Object().Delete(context.Background(), &obj.ObjectMeta)
	}

	return api.ct.handleObjectEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Deleted})
}

// Find returns an object by meta
func (api *objectAPI) Find(meta *api.ObjectMeta) (*Object, error) {
	// find the object
	obj, err := api.ct.FindObject("Object", meta)
	if err != nil {
		return nil, err
	}

	// asset type
	switch obj.(type) {
	case *Object:
		hobj := obj.(*Object)
		return hobj, nil
	default:
		return nil, errors.New("incorrect object type")
	}
}

// List returns a list of all Object objects
func (api *objectAPI) List() []*Object {
	var objlist []*Object

	objs := api.ct.ListObjects("Object")
	for _, obj := range objs {
		switch tp := obj.(type) {
		case *Object:
			eobj := obj.(*Object)
			objlist = append(objlist, eobj)
		default:
			log.Fatalf("Got invalid object type %v while looking for Object", tp)
		}
	}

	return objlist
}

// Watch sets up a event handlers for Object object
func (api *objectAPI) Watch(handler ObjectHandler) error {
	return api.ct.WatchObject(handler)
}

// Object returns ObjectAPI
func (ct *ctrlerCtx) Object() ObjectAPI {
	return &objectAPI{ct: ct}
}
