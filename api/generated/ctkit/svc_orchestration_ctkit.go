// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package ctkit is a auto generated package.
Input file: svc_orchestration.proto
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
	"github.com/pensando/sw/api/generated/orchestration"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/shardworkers"
)

// Orchestrator is a wrapper object that implements additional functionality
type Orchestrator struct {
	sync.Mutex
	orchestration.Orchestrator
	HandlerCtx interface{} // additional state handlers can store
	ctrler     *ctrlerCtx  // reference back to the controller instance
}

func (obj *Orchestrator) Write() error {
	// if there is no API server to connect to, we are done
	if (obj.ctrler == nil) || (obj.ctrler.resolver == nil) || obj.ctrler.apisrvURL == "" {
		return nil
	}

	apicl, err := obj.ctrler.apiClient()
	if err != nil {
		obj.ctrler.logger.Errorf("Error creating API server clent. Err: %v", err)
		return err
	}

	obj.ctrler.stats.Counter("Orchestrator_Writes").Inc()

	// write to api server
	if obj.ObjectMeta.ResourceVersion != "" {
		// update it
		for i := 0; i < maxApisrvWriteRetry; i++ {
			_, err = apicl.OrchestratorV1().Orchestrator().UpdateStatus(context.Background(), &obj.Orchestrator)
			if err == nil {
				break
			}
			time.Sleep(time.Millisecond * 100)
		}
	} else {
		//  create
		_, err = apicl.OrchestratorV1().Orchestrator().Create(context.Background(), &obj.Orchestrator)
	}

	return nil
}

// OrchestratorHandler is the event handler for Orchestrator object
type OrchestratorHandler interface {
	OnOrchestratorCreate(obj *Orchestrator) error
	OnOrchestratorUpdate(oldObj *Orchestrator, newObj *orchestration.Orchestrator) error
	OnOrchestratorDelete(obj *Orchestrator) error
	GetOrchestratorWatchOptions() *api.ListWatchOptions
}

// OnOrchestratorCreate is a dummy handler used in init if no one registers the handler
func (ctrler CtrlDefReactor) OnOrchestratorCreate(obj *Orchestrator) error {
	log.Info("OnOrchestratorCreate is not implemented")
	return nil
}

// OnOrchestratorUpdate is a dummy handler used in init if no one registers the handler
func (ctrler CtrlDefReactor) OnOrchestratorUpdate(oldObj *Orchestrator, newObj *orchestration.Orchestrator) error {
	log.Info("OnOrchestratorUpdate is not implemented")
	return nil
}

// OnOrchestratorDelete is a dummy handler used in init if no one registers the handler
func (ctrler CtrlDefReactor) OnOrchestratorDelete(obj *Orchestrator) error {
	log.Info("OnOrchestratorDelete is not implemented")
	return nil
}

// GetOrchestratorWatchOptions is a dummy handler used in init if no one registers the handler
func (ctrler CtrlDefReactor) GetOrchestratorWatchOptions() *api.ListWatchOptions {
	log.Info("GetOrchestratorWatchOptions is not implemented")
	opts := &api.ListWatchOptions{}
	return opts
}

// handleOrchestratorEvent handles Orchestrator events from watcher
func (ct *ctrlerCtx) handleOrchestratorEvent(evt *kvstore.WatchEvent) error {

	if ct.objResolver == nil {
		return ct.handleOrchestratorEventNoResolver(evt)
	}

	switch tp := evt.Object.(type) {
	case *orchestration.Orchestrator:
		eobj := evt.Object.(*orchestration.Orchestrator)
		kind := "Orchestrator"

		//ct.logger.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)
		log.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)

		ctx := &orchestratorCtx{event: evt.Type,
			obj: &Orchestrator{Orchestrator: *eobj, ctrler: ct}}

		var err error
		switch evt.Type {
		case kvstore.Created:
			err = ct.processAdd(ctx)
		case kvstore.Updated:
			err = ct.processUpdate(ctx)
		case kvstore.Deleted:
			err = ct.processDelete(ctx)
		}
		return err
	default:
		ct.logger.Fatalf("API watcher Found object of invalid type: %v on Orchestrator watch channel", tp)
	}

	return nil
}

// handleOrchestratorEventNoResolver handles Orchestrator events from watcher
func (ct *ctrlerCtx) handleOrchestratorEventNoResolver(evt *kvstore.WatchEvent) error {
	switch tp := evt.Object.(type) {
	case *orchestration.Orchestrator:
		eobj := evt.Object.(*orchestration.Orchestrator)
		kind := "Orchestrator"

		//ct.logger.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)
		log.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)

		ct.Lock()
		handler, ok := ct.handlers[kind]
		ct.Unlock()
		if !ok {
			ct.logger.Fatalf("Cant find the handler for %s", kind)
		}
		orchestratorHandler := handler.(OrchestratorHandler)
		// handle based on event type
		ctrlCtx := &orchestratorCtx{event: evt.Type, obj: &Orchestrator{Orchestrator: *eobj, ctrler: ct}}
		switch evt.Type {
		case kvstore.Created:
			fallthrough
		case kvstore.Updated:
			fobj, err := ct.getObject(kind, ctrlCtx.GetKey())
			if err != nil {
				ct.addObject(ctrlCtx)
				ct.stats.Counter("Orchestrator_Created_Events").Inc()

				// call the event handler
				ctrlCtx.Lock()
				err = orchestratorHandler.OnOrchestratorCreate(ctrlCtx.obj)
				ctrlCtx.Unlock()
				if err != nil {
					ct.logger.Errorf("Error creating %s %+v. Err: %v", kind, ctrlCtx.obj, err)
					ct.delObject(kind, ctrlCtx.GetKey())
					return err
				}
			} else {
				if ct.resolver != nil && fobj.GetResourceVersion() >= eobj.GetResourceVersion() {
					// Event already processed.
					ct.logger.Infof("Skipping update due to old resource version")
					return nil
				}
				ctrlCtx := fobj.(*orchestratorCtx)
				ct.stats.Counter("Orchestrator_Updated_Events").Inc()
				ctrlCtx.Lock()
				p := orchestration.Orchestrator{Spec: eobj.Spec,
					ObjectMeta: eobj.ObjectMeta,
					TypeMeta:   eobj.TypeMeta,
					Status:     eobj.Status}

				err = orchestratorHandler.OnOrchestratorUpdate(ctrlCtx.obj, &p)
				ctrlCtx.obj.Orchestrator = *eobj
				ctrlCtx.Unlock()
				if err != nil {
					ct.logger.Errorf("Error creating %s %+v. Err: %v", kind, ctrlCtx.obj, err)
					return err
				}

			}
		case kvstore.Deleted:
			ctrlCtx := &orchestratorCtx{event: evt.Type, obj: &Orchestrator{Orchestrator: *eobj, ctrler: ct}}
			fobj, err := ct.findObject(kind, ctrlCtx.GetKey())
			if err != nil {
				ct.logger.Errorf("Object %s/%s not found durng delete. Err: %v", kind, eobj.GetKey(), err)
				return err
			}

			obj := fobj.(*Orchestrator)
			ct.stats.Counter("Orchestrator_Deleted_Events").Inc()
			obj.Lock()
			err = orchestratorHandler.OnOrchestratorDelete(obj)
			obj.Unlock()
			if err != nil {
				ct.logger.Errorf("Error deleting %s: %+v. Err: %v", kind, obj, err)
			}
			ct.delObject(kind, ctrlCtx.GetKey())
			return nil

		}
	default:
		ct.logger.Fatalf("API watcher Found object of invalid type: %v on Orchestrator watch channel", tp)
	}

	return nil
}

type orchestratorCtx struct {
	ctkitBaseCtx
	event kvstore.WatchEventType
	obj   *Orchestrator //
	//   newObj     *orchestration.Orchestrator //update
	newObj *orchestratorCtx //update
}

func (ctx *orchestratorCtx) References() map[string]apiintf.ReferenceObj {
	resp := make(map[string]apiintf.ReferenceObj)
	ctx.obj.References(ctx.obj.GetObjectMeta().Name, ctx.obj.GetObjectMeta().Namespace, resp)
	return resp
}

func (ctx *orchestratorCtx) GetKey() string {
	return ctx.obj.MakeKey("orchestration")
}

func (ctx *orchestratorCtx) GetKind() string {
	return ctx.obj.GetKind()
}

func (ctx *orchestratorCtx) GetResourceVersion() string {
	return ctx.obj.GetResourceVersion()
}

func (ctx *orchestratorCtx) SetEvent(event kvstore.WatchEventType) {
	ctx.event = event
}

func (ctx *orchestratorCtx) SetNewObj(newObj apiintf.CtkitObject) {
	if newObj == nil {
		ctx.newObj = nil
	} else {
		ctx.newObj = newObj.(*orchestratorCtx)
		ctx.newObj.obj.HandlerCtx = ctx.obj.HandlerCtx
	}
}

func (ctx *orchestratorCtx) GetNewObj() apiintf.CtkitObject {
	return ctx.newObj
}

func (ctx *orchestratorCtx) Copy(obj apiintf.CtkitObject) {
	ctx.obj.Orchestrator = obj.(*orchestratorCtx).obj.Orchestrator
}

func (ctx *orchestratorCtx) Lock() {
	ctx.obj.Lock()
}

func (ctx *orchestratorCtx) Unlock() {
	ctx.obj.Unlock()
}

func (ctx *orchestratorCtx) GetObjectMeta() *api.ObjectMeta {
	return ctx.obj.GetObjectMeta()
}

func (ctx *orchestratorCtx) RuntimeObject() runtime.Object {
	var v interface{}
	v = ctx.obj
	return v.(runtime.Object)
}

func (ctx *orchestratorCtx) WorkFunc(context context.Context) error {
	var err error
	evt := ctx.event
	ct := ctx.obj.ctrler
	kind := "Orchestrator"
	ct.Lock()
	handler, ok := ct.handlers[kind]
	ct.Unlock()
	if !ok {
		ct.logger.Fatalf("Cant find the handler for %s", kind)
	}
	orchestratorHandler := handler.(OrchestratorHandler)
	switch evt {
	case kvstore.Created:
		ctx.obj.Lock()
		err = orchestratorHandler.OnOrchestratorCreate(ctx.obj)
		ctx.obj.Unlock()
		if err != nil {
			ct.logger.Errorf("Error creating %s %+v. Err: %v", kind, ctx.obj, err)
			ctx.SetEvent(kvstore.Deleted)
		}
	case kvstore.Updated:
		ct.stats.Counter("Orchestrator_Updated_Events").Inc()
		ctx.obj.Lock()
		p := orchestration.Orchestrator{Spec: ctx.newObj.obj.Spec,
			ObjectMeta: ctx.newObj.obj.ObjectMeta,
			TypeMeta:   ctx.newObj.obj.TypeMeta,
			Status:     ctx.newObj.obj.Status}
		err = orchestratorHandler.OnOrchestratorUpdate(ctx.obj, &p)
		ctx.obj.Unlock()
		if err != nil {
			ct.logger.Errorf("Error creating %s %+v. Err: %v", kind, ctx.obj, err)
			ctx.SetEvent(kvstore.Deleted)
		}
	case kvstore.Deleted:
		ctx.obj.Lock()
		err = orchestratorHandler.OnOrchestratorDelete(ctx.obj)
		ctx.obj.Unlock()
		if err != nil {
			ct.logger.Errorf("Error deleting %s %+v. Err: %v", kind, ctx.obj, err)
		}
	}
	ct.resolveObject(ctx.event, ctx)
	return nil
}

// handleOrchestratorEventParallel handles Orchestrator events from watcher
func (ct *ctrlerCtx) handleOrchestratorEventParallel(evt *kvstore.WatchEvent) error {

	if ct.objResolver == nil {
		return ct.handleOrchestratorEventParallelWithNoResolver(evt)
	}

	switch tp := evt.Object.(type) {
	case *orchestration.Orchestrator:
		eobj := evt.Object.(*orchestration.Orchestrator)
		kind := "Orchestrator"

		//ct.logger.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)
		log.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)

		ctx := &orchestratorCtx{event: evt.Type, obj: &Orchestrator{Orchestrator: *eobj, ctrler: ct}}

		var err error
		switch evt.Type {
		case kvstore.Created:
			err = ct.processAdd(ctx)
		case kvstore.Updated:
			err = ct.processUpdate(ctx)
		case kvstore.Deleted:
			err = ct.processDelete(ctx)
		}
		return err
	default:
		ct.logger.Fatalf("API watcher Found object of invalid type: %v on Orchestrator watch channel", tp)
	}

	return nil
}

// handleOrchestratorEventParallel handles Orchestrator events from watcher
func (ct *ctrlerCtx) handleOrchestratorEventParallelWithNoResolver(evt *kvstore.WatchEvent) error {
	switch tp := evt.Object.(type) {
	case *orchestration.Orchestrator:
		eobj := evt.Object.(*orchestration.Orchestrator)
		kind := "Orchestrator"

		//ct.logger.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)
		log.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)

		ct.Lock()
		handler, ok := ct.handlers[kind]
		ct.Unlock()
		if !ok {
			ct.logger.Fatalf("Cant find the handler for %s", kind)
		}
		orchestratorHandler := handler.(OrchestratorHandler)
		// handle based on event type
		switch evt.Type {
		case kvstore.Created:
			fallthrough
		case kvstore.Updated:
			workFunc := func(ctx context.Context, ctrlCtx shardworkers.WorkObj) error {
				var err error
				workCtx := ctrlCtx.(*orchestratorCtx)
				eobj := workCtx.obj
				fobj, err := ct.getObject(kind, workCtx.GetKey())
				if err != nil {
					ct.addObject(workCtx)
					ct.stats.Counter("Orchestrator_Created_Events").Inc()
					eobj.Lock()
					err = orchestratorHandler.OnOrchestratorCreate(eobj)
					eobj.Unlock()
					if err != nil {
						ct.logger.Errorf("Error creating %s %+v. Err: %v", kind, eobj, err)
						ct.delObject(kind, workCtx.GetKey())
					}
				} else {
					workCtx := fobj.(*orchestratorCtx)
					obj := workCtx.obj
					ct.stats.Counter("Orchestrator_Updated_Events").Inc()
					obj.Lock()
					p := orchestration.Orchestrator{Spec: eobj.Spec,
						ObjectMeta: eobj.ObjectMeta,
						TypeMeta:   eobj.TypeMeta,
						Status:     eobj.Status}

					err = orchestratorHandler.OnOrchestratorUpdate(obj, &p)
					if err != nil {
						ct.logger.Errorf("Error creating %s %+v. Err: %v", kind, obj, err)
					} else {
						workCtx.obj.Orchestrator = p
					}
					obj.Unlock()
				}
				return err
			}
			ctrlCtx := &orchestratorCtx{event: evt.Type, obj: &Orchestrator{Orchestrator: *eobj, ctrler: ct}}
			ct.runFunction("Orchestrator", ctrlCtx, workFunc)
		case kvstore.Deleted:
			workFunc := func(ctx context.Context, ctrlCtx shardworkers.WorkObj) error {
				var err error
				workCtx := ctrlCtx.(*orchestratorCtx)
				eobj := workCtx.obj
				fobj, err := ct.findObject(kind, workCtx.GetKey())
				if err != nil {
					ct.logger.Errorf("Object %s/%s not found durng delete. Err: %v", kind, eobj.GetKey(), err)
					return err
				}
				obj := fobj.(*Orchestrator)
				ct.stats.Counter("Orchestrator_Deleted_Events").Inc()
				obj.Lock()
				err = orchestratorHandler.OnOrchestratorDelete(obj)
				obj.Unlock()
				if err != nil {
					ct.logger.Errorf("Error deleting %s: %+v. Err: %v", kind, obj, err)
				}
				ct.delObject(kind, workCtx.GetKey())
				return nil
			}
			ctrlCtx := &orchestratorCtx{event: evt.Type, obj: &Orchestrator{Orchestrator: *eobj, ctrler: ct}}
			ct.runFunction("Orchestrator", ctrlCtx, workFunc)
		}
	default:
		ct.logger.Fatalf("API watcher Found object of invalid type: %v on Orchestrator watch channel", tp)
	}

	return nil
}

// diffOrchestrator does a diff of Orchestrator objects between local cache and API server
func (ct *ctrlerCtx) diffOrchestrator(apicl apiclient.Services) {
	opts := api.ListWatchOptions{}

	// get a list of all objects from API server
	objlist, err := apicl.OrchestratorV1().Orchestrator().List(context.Background(), &opts)
	if err != nil {
		ct.logger.Errorf("Error getting a list of objects. Err: %v", err)
		return
	}

	ct.logger.Infof("diffOrchestrator(): OrchestratorList returned %d objects", len(objlist))

	// build an object map
	objmap := make(map[string]*orchestration.Orchestrator)
	for _, obj := range objlist {
		objmap[obj.GetKey()] = obj
	}

	list, err := ct.Orchestrator().List(context.Background(), &opts)
	if err != nil {
		ct.logger.Infof("Failed to get a list of objects. Err: %s", err)
		return
	}

	// if an object is in our local cache and not in API server, trigger delete for it
	for _, obj := range list {
		_, ok := objmap[obj.GetKey()]
		if !ok {
			ct.logger.Infof("diffOrchestrator(): Deleting existing object %#v since its not in apiserver", obj.GetKey())
			evt := kvstore.WatchEvent{
				Type:   kvstore.Deleted,
				Key:    obj.GetKey(),
				Object: &obj.Orchestrator,
			}
			ct.handleOrchestratorEvent(&evt)
		}
	}

	// trigger create event for all others
	for _, obj := range objlist {
		ct.logger.Infof("diffOrchestrator(): Adding object %#v", obj.GetKey())
		evt := kvstore.WatchEvent{
			Type:   kvstore.Created,
			Key:    obj.GetKey(),
			Object: obj,
		}
		ct.handleOrchestratorEvent(&evt)
	}
}

func (ct *ctrlerCtx) runOrchestratorWatcher() {
	kind := "Orchestrator"

	ct.Lock()
	handler, ok := ct.handlers[kind]
	ct.Unlock()
	if !ok {
		ct.logger.Fatalf("Cant find the handler for %s", kind)
	}
	orchestratorHandler := handler.(OrchestratorHandler)

	opts := orchestratorHandler.GetOrchestratorWatchOptions()

	// if there is no API server to connect to, we are done
	if (ct.resolver == nil) || ct.apisrvURL == "" {
		return
	}

	// create context
	ctx, cancel := context.WithCancel(context.Background())
	ct.Lock()
	ct.watchCancel[kind] = cancel
	ct.Unlock()
	logger := ct.logger.WithContext("submodule", "OrchestratorWatcher")

	// create a grpc client
	apiclt, err := apiclient.NewGrpcAPIClient(ct.name, ct.apisrvURL, logger, rpckit.WithBalancer(balancer.New(ct.resolver)))
	if err == nil {
		ct.diffOrchestrator(apiclt)
	}

	// setup wait group
	ct.waitGrp.Add(1)

	// start a goroutine
	go func() {
		defer ct.waitGrp.Done()
		ct.stats.Counter("Orchestrator_Watch").Inc()
		defer ct.stats.Counter("Orchestrator_Watch").Dec()

		// loop forever
		for {
			// create a grpc client
			apicl, err := apiclient.NewGrpcAPIClient(ct.name, ct.apisrvURL, logger, rpckit.WithBalancer(balancer.New(ct.resolver)))
			if err != nil {
				logger.Warnf("Failed to connect to gRPC server [%s]\n", ct.apisrvURL)
				ct.stats.Counter("Orchestrator_ApiClientErr").Inc()
			} else {
				logger.Infof("API client connected {%+v}", apicl)

				// Orchestrator object watcher
				wt, werr := apicl.OrchestratorV1().Orchestrator().Watch(ctx, opts)
				if werr != nil {
					select {
					case <-ctx.Done():
						logger.Infof("watch %s cancelled", kind)
						return
					default:
					}
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
				ct.diffOrchestrator(apicl)

				// handle api server watch events
			innerLoop:
				for {
					// wait for events
					select {
					case evt, ok := <-wt.EventChan():
						if !ok {
							logger.Error("Error receiving from apisrv watcher")
							ct.stats.Counter("Orchestrator_WatchErrors").Inc()
							break innerLoop
						}

						// handle event in parallel
						ct.handleOrchestratorEventParallel(evt)
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
	}()
}

// WatchOrchestrator starts watch on Orchestrator object
func (ct *ctrlerCtx) WatchOrchestrator(handler OrchestratorHandler) error {
	kind := "Orchestrator"

	// see if we already have a watcher
	ct.Lock()
	_, ok := ct.watchers[kind]
	ct.Unlock()
	if ok {
		return fmt.Errorf("Orchestrator watcher already exists")
	}

	// save handler
	ct.Lock()
	ct.handlers[kind] = handler
	ct.Unlock()

	// run Orchestrator watcher in a go routine
	ct.runOrchestratorWatcher()

	return nil
}

// StopWatchOrchestrator stops watch on Orchestrator object
func (ct *ctrlerCtx) StopWatchOrchestrator(handler OrchestratorHandler) error {
	kind := "Orchestrator"

	// see if we already have a watcher
	ct.Lock()
	_, ok := ct.watchers[kind]
	ct.Unlock()
	if !ok {
		return fmt.Errorf("Orchestrator watcher does not exist")
	}

	ct.Lock()
	cancel, _ := ct.watchCancel[kind]
	cancel()
	delete(ct.watchers, kind)
	delete(ct.watchCancel, kind)
	ct.Unlock()

	time.Sleep(100 * time.Millisecond)

	return nil
}

// OrchestratorAPI returns
type OrchestratorAPI interface {
	Create(obj *orchestration.Orchestrator) error
	SyncCreate(obj *orchestration.Orchestrator) error
	Update(obj *orchestration.Orchestrator) error
	SyncUpdate(obj *orchestration.Orchestrator) error
	Delete(obj *orchestration.Orchestrator) error
	Find(meta *api.ObjectMeta) (*Orchestrator, error)
	List(ctx context.Context, opts *api.ListWatchOptions) ([]*Orchestrator, error)
	Watch(handler OrchestratorHandler) error
	StopWatch(handler OrchestratorHandler) error
}

// dummy struct that implements OrchestratorAPI
type orchestratorAPI struct {
	ct *ctrlerCtx
}

// Create creates Orchestrator object
func (api *orchestratorAPI) Create(obj *orchestration.Orchestrator) error {
	if api.ct.resolver != nil {
		apicl, err := api.ct.apiClient()
		if err != nil {
			api.ct.logger.Errorf("Error creating API server clent. Err: %v", err)
			return err
		}

		_, err = apicl.OrchestratorV1().Orchestrator().Create(context.Background(), obj)
		if err != nil && strings.Contains(err.Error(), "AlreadyExists") {
			_, err = apicl.OrchestratorV1().Orchestrator().Update(context.Background(), obj)
		}
		return err
	}

	api.ct.handleOrchestratorEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Created})
	return nil
}

// SyncCreate creates Orchestrator object and updates the cache
func (api *orchestratorAPI) SyncCreate(obj *orchestration.Orchestrator) error {
	newObj := obj
	evtType := kvstore.Created
	var writeErr error
	if api.ct.resolver != nil {
		apicl, err := api.ct.apiClient()
		if err != nil {
			api.ct.logger.Errorf("Error creating API server clent. Err: %v", err)
			return err
		}

		newObj, writeErr = apicl.OrchestratorV1().Orchestrator().Create(context.Background(), obj)
		if writeErr != nil && strings.Contains(err.Error(), "AlreadyExists") {
			newObj, writeErr = apicl.OrchestratorV1().Orchestrator().Update(context.Background(), obj)
			evtType = kvstore.Updated
		}
	}

	if writeErr == nil {
		api.ct.handleOrchestratorEvent(&kvstore.WatchEvent{Object: newObj, Type: evtType})
	}

	if writeErr == nil {
		api.ct.handleOrchestratorEvent(&kvstore.WatchEvent{Object: newObj, Type: evtType})
	}

	return writeErr
}

// Update triggers update on Orchestrator object
func (api *orchestratorAPI) Update(obj *orchestration.Orchestrator) error {
	if api.ct.resolver != nil {
		apicl, err := api.ct.apiClient()
		if err != nil {
			api.ct.logger.Errorf("Error creating API server clent. Err: %v", err)
			return err
		}

		_, err = apicl.OrchestratorV1().Orchestrator().Update(context.Background(), obj)
		return err
	}

	api.ct.handleOrchestratorEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Updated})
	return nil
}

// SyncUpdate triggers update on Orchestrator object and updates the cache
func (api *orchestratorAPI) SyncUpdate(obj *orchestration.Orchestrator) error {
	newObj := obj
	var writeErr error
	if api.ct.resolver != nil {
		apicl, err := api.ct.apiClient()
		if err != nil {
			api.ct.logger.Errorf("Error creating API server clent. Err: %v", err)
			return err
		}

		newObj, writeErr = apicl.OrchestratorV1().Orchestrator().Update(context.Background(), obj)
	}

	if writeErr == nil {
		api.ct.handleOrchestratorEvent(&kvstore.WatchEvent{Object: newObj, Type: kvstore.Updated})
	}

	return writeErr
}

// Delete deletes Orchestrator object
func (api *orchestratorAPI) Delete(obj *orchestration.Orchestrator) error {
	if api.ct.resolver != nil {
		apicl, err := api.ct.apiClient()
		if err != nil {
			api.ct.logger.Errorf("Error creating API server clent. Err: %v", err)
			return err
		}

		_, err = apicl.OrchestratorV1().Orchestrator().Delete(context.Background(), &obj.ObjectMeta)
		return err
	}

	api.ct.handleOrchestratorEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Deleted})
	return nil
}

// MakeKey generates a KV store key for the object
func (api *orchestratorAPI) getFullKey(tenant, name string) string {
	if tenant != "" {
		return fmt.Sprint(globals.ConfigRootPrefix, "/", "orchestration", "/", "orchestrator", "/", tenant, "/", name)
	}
	return fmt.Sprint(globals.ConfigRootPrefix, "/", "orchestration", "/", "orchestrator", "/", name)
}

// Find returns an object by meta
func (api *orchestratorAPI) Find(meta *api.ObjectMeta) (*Orchestrator, error) {
	// find the object
	obj, err := api.ct.FindObject("Orchestrator", meta)
	if err != nil {
		return nil, err
	}

	// asset type
	switch obj.(type) {
	case *Orchestrator:
		hobj := obj.(*Orchestrator)
		return hobj, nil
	default:
		return nil, errors.New("incorrect object type")
	}
}

// List returns a list of all Orchestrator objects
func (api *orchestratorAPI) List(ctx context.Context, opts *api.ListWatchOptions) ([]*Orchestrator, error) {
	var objlist []*Orchestrator
	objs, err := api.ct.List("Orchestrator", ctx, opts)

	if err != nil {
		return nil, err
	}

	for _, obj := range objs {
		switch tp := obj.(type) {
		case *Orchestrator:
			eobj := obj.(*Orchestrator)
			objlist = append(objlist, eobj)
		default:
			log.Fatalf("Got invalid object type %v while looking for Orchestrator", tp)
		}
	}

	return objlist, nil
}

// Watch sets up a event handlers for Orchestrator object
func (api *orchestratorAPI) Watch(handler OrchestratorHandler) error {
	api.ct.startWorkerPool("Orchestrator")
	return api.ct.WatchOrchestrator(handler)
}

// StopWatch stop watch for Tenant Orchestrator object
func (api *orchestratorAPI) StopWatch(handler OrchestratorHandler) error {
	api.ct.Lock()
	api.ct.workPools["Orchestrator"].Stop()
	api.ct.Unlock()
	return api.ct.StopWatchOrchestrator(handler)
}

// Orchestrator returns OrchestratorAPI
func (ct *ctrlerCtx) Orchestrator() OrchestratorAPI {
	return &orchestratorAPI{ct: ct}
}
