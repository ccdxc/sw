// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package ctkit is a auto generated package.
Input file: svc_network.proto
*/
package ctkit

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Network is a wrapper object that implements additional functionality
type Network struct {
	sync.Mutex
	network.Network
	HandlerCtx interface{} // additional state handlers can store
	ctrler     *ctrlerCtx  // reference back to the controller instance
}

func (obj *Network) Write() error {
	// if there is no API server to connect to, we are done
	if (obj.ctrler == nil) || (obj.ctrler.resolver == nil) || obj.ctrler.apisrvURL == "" {
		return nil
	}

	apicl, err := obj.ctrler.apiClient()
	if err != nil {
		log.Errorf("Error creating API server clent. Err: %v", err)
		return err
	}

	// write to api server
	if obj.ObjectMeta.ResourceVersion != "" {
		nobj := *obj
		// FIXME: clear the resource version till we figure out CAS semantics
		nobj.ObjectMeta.ResourceVersion = ""

		// update it
		_, err = apicl.NetworkV1().Network().Update(context.Background(), &nobj.Network)
	} else {
		//  create
		_, err = apicl.NetworkV1().Network().Create(context.Background(), &obj.Network)
	}

	return nil
}

// NetworkHandler is the event handler for Network object
type NetworkHandler interface {
	OnNetworkCreate(obj *Network) error
	OnNetworkUpdate(obj *Network) error
	OnNetworkDelete(obj *Network) error
}

// handleNetworkEvent handles Network events from watcher
func (ct *ctrlerCtx) handleNetworkEvent(evt *kvstore.WatchEvent) error {
	switch tp := evt.Object.(type) {
	case *network.Network:
		eobj := evt.Object.(*network.Network)
		kind := "Network"

		log.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)

		handler, ok := ct.handlers[kind]
		if !ok {
			log.Fatalf("Cant find the handler for %s", kind)
		}
		networkHandler := handler.(NetworkHandler)
		// handle based on event type
		switch evt.Type {
		case kvstore.Created:
			fallthrough
		case kvstore.Updated:
			fobj, err := ct.findObject(kind, eobj.GetKey())
			if err != nil {
				obj := &Network{
					Network:    *eobj,
					HandlerCtx: nil,
					ctrler:     ct,
				}
				ct.addObject(kind, obj.GetKey(), obj)
				// call the event handler
				obj.Lock()
				err = networkHandler.OnNetworkCreate(obj)
				obj.Unlock()
				if err != nil {
					log.Errorf("Error creating %s %+v. Err: %v", kind, obj, err)
					ct.delObject(kind, eobj.GetKey())
					return err
				}
			} else {
				obj := fobj.(*Network)
				obj.ObjectMeta = eobj.ObjectMeta

				// see if it changed
				if _, ok := ref.ObjDiff(obj.Spec, eobj.Spec); ok {
					obj.Spec = eobj.Spec
					// call the event handler
					obj.Lock()
					err = networkHandler.OnNetworkUpdate(obj)
					obj.Unlock()
					if err != nil {
						log.Errorf("Error creating %s %+v. Err: %v", kind, obj, err)
						return err
					}
				}
			}
		case kvstore.Deleted:
			fobj, err := ct.findObject(kind, eobj.GetKey())
			if err != nil {
				log.Errorf("Object %s/%s not found durng delete. Err: %v", kind, eobj.GetKey(), err)
				return err
			}

			obj := fobj.(*Network)

			// Call the event reactor
			obj.Lock()
			err = networkHandler.OnNetworkDelete(obj)
			obj.Unlock()
			if err != nil {
				log.Errorf("Error deleting %s: %+v. Err: %v", kind, obj, err)
				return err
			}

			ct.delObject(kind, eobj.GetKey())
		}
	default:
		log.Fatalf("API watcher Found object of invalid type: %v on Network watch channel", tp)
	}

	return nil
}

// diffNetwork does a diff of Network objects between local cache and API server
func (ct *ctrlerCtx) diffNetwork(apicl apiclient.Services) {
	opts := api.ListWatchOptions{}

	// get a list of all objects from API server
	objlist, err := apicl.NetworkV1().Network().List(context.Background(), &opts)
	if err != nil {
		log.Errorf("Error getting a list of objects. Err: %v", err)
		return
	}

	// build an object map
	objmap := make(map[string]*network.Network)
	for _, obj := range objlist {
		objmap[obj.GetKey()] = obj
	}

	// if an object is in our local cache and not in API server, trigger delete for it
	for _, obj := range ct.Network().List() {
		_, ok := objmap[obj.GetKey()]
		if !ok {
			evt := kvstore.WatchEvent{
				Type:   kvstore.Deleted,
				Key:    obj.GetKey(),
				Object: &obj.Network,
			}
			ct.handleNetworkEvent(&evt)
		}
	}

	// trigger create event for all others
	for _, obj := range objlist {
		evt := kvstore.WatchEvent{
			Type:   kvstore.Created,
			Key:    obj.GetKey(),
			Object: obj,
		}
		ct.handleNetworkEvent(&evt)
	}
}

func (ct *ctrlerCtx) runNetworkWatcher() {
	kind := "Network"

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

	// loop forever
	for {
		// create a grpc client
		apicl, err := apiclient.NewGrpcAPIClient(ct.name, ct.apisrvURL, ct.logger, rpckit.WithBalancer(ct.balancer))
		if err != nil {
			log.Warnf("Failed to connect to gRPC server [%s]\n", ct.apisrvURL)
		} else {
			log.Infof("API client connected {%+v}", apicl)

			// Network object watcher
			wt, werr := apicl.NetworkV1().Network().Watch(ctx, &opts)
			if werr != nil {
				log.Errorf("Failed to start %s watch (%s)\n", kind, werr)
				// wait for a second and retry connecting to api server
				time.Sleep(time.Second)
				continue
			}
			ct.Lock()
			ct.watchers[kind] = wt
			ct.Unlock()

			// perform a diff with API server and local cache
			time.Sleep(time.Millisecond * 100)
			ct.diffNetwork(apicl)

			// handle api server watch events
		innerLoop:
			for {
				// wait for events
				select {
				case evt, ok := <-wt.EventChan():
					if !ok {
						log.Error("Error receiving from apisrv watcher")
						break innerLoop
					}

					// handle event
					ct.handleNetworkEvent(evt)
				}
			}
			apicl.Close()
		}

		// if stop flag is set, we are done
		if ct.stoped {
			log.Infof("Exiting API server watcher")
			return
		}

		// wait for a second and retry connecting to api server
		time.Sleep(time.Second)
	}
}

// WatchNetwork starts watch on Network object
func (ct *ctrlerCtx) WatchNetwork(handler NetworkHandler) error {
	kind := "Network"

	ct.Lock()
	defer ct.Unlock()

	// see if we already have a watcher
	_, ok := ct.watchers[kind]
	if ok {
		return fmt.Errorf("Network watcher already exists")
	}

	// save handler
	ct.handlers[kind] = handler

	// run Network watcher in a go routine
	go ct.runNetworkWatcher()

	return nil
}

// NetworkAPI returns
type NetworkAPI interface {
	Create(obj *network.Network) error
	Update(obj *network.Network) error
	Delete(obj *network.Network) error
	List() []*Network
	Watch(handler NetworkHandler) error
}

// dummy struct that implements NetworkAPI
type networkAPI struct {
	ct *ctrlerCtx
}

// Create creates Network object
func (api *networkAPI) Create(obj *network.Network) error {
	return api.ct.handleNetworkEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Created})
}

// Update triggers update on Network object
func (api *networkAPI) Update(obj *network.Network) error {
	return api.ct.handleNetworkEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Updated})
}

// Delete deletes Network object
func (api *networkAPI) Delete(obj *network.Network) error {
	return api.ct.handleNetworkEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Deleted})
}

// List returns a list of all Network objects
func (api *networkAPI) List() []*Network {
	var objlist []*Network

	objs := api.ct.ListObjects("Network")
	for _, obj := range objs {
		switch tp := obj.(type) {
		case *Network:
			eobj := obj.(*Network)
			objlist = append(objlist, eobj)
		default:
			log.Fatalf("Got invalid object type %v while looking for Network", tp)
		}
	}

	return objlist
}

// Watch sets up a event handlers for Network object
func (api *networkAPI) Watch(handler NetworkHandler) error {
	return api.ct.WatchNetwork(handler)
}

// Network returns NetworkAPI
func (ct *ctrlerCtx) Network() NetworkAPI {
	return &networkAPI{ct: ct}
}

// Service is a wrapper object that implements additional functionality
type Service struct {
	sync.Mutex
	network.Service
	HandlerCtx interface{} // additional state handlers can store
	ctrler     *ctrlerCtx  // reference back to the controller instance
}

func (obj *Service) Write() error {
	// if there is no API server to connect to, we are done
	if (obj.ctrler == nil) || (obj.ctrler.resolver == nil) || obj.ctrler.apisrvURL == "" {
		return nil
	}

	apicl, err := obj.ctrler.apiClient()
	if err != nil {
		log.Errorf("Error creating API server clent. Err: %v", err)
		return err
	}

	// write to api server
	if obj.ObjectMeta.ResourceVersion != "" {
		nobj := *obj
		// FIXME: clear the resource version till we figure out CAS semantics
		nobj.ObjectMeta.ResourceVersion = ""

		// update it
		_, err = apicl.NetworkV1().Service().Update(context.Background(), &nobj.Service)
	} else {
		//  create
		_, err = apicl.NetworkV1().Service().Create(context.Background(), &obj.Service)
	}

	return nil
}

// ServiceHandler is the event handler for Service object
type ServiceHandler interface {
	OnServiceCreate(obj *Service) error
	OnServiceUpdate(obj *Service) error
	OnServiceDelete(obj *Service) error
}

// handleServiceEvent handles Service events from watcher
func (ct *ctrlerCtx) handleServiceEvent(evt *kvstore.WatchEvent) error {
	switch tp := evt.Object.(type) {
	case *network.Service:
		eobj := evt.Object.(*network.Service)
		kind := "Service"

		log.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)

		handler, ok := ct.handlers[kind]
		if !ok {
			log.Fatalf("Cant find the handler for %s", kind)
		}
		serviceHandler := handler.(ServiceHandler)
		// handle based on event type
		switch evt.Type {
		case kvstore.Created:
			fallthrough
		case kvstore.Updated:
			fobj, err := ct.findObject(kind, eobj.GetKey())
			if err != nil {
				obj := &Service{
					Service:    *eobj,
					HandlerCtx: nil,
					ctrler:     ct,
				}
				ct.addObject(kind, obj.GetKey(), obj)
				// call the event handler
				obj.Lock()
				err = serviceHandler.OnServiceCreate(obj)
				obj.Unlock()
				if err != nil {
					log.Errorf("Error creating %s %+v. Err: %v", kind, obj, err)
					ct.delObject(kind, eobj.GetKey())
					return err
				}
			} else {
				obj := fobj.(*Service)
				obj.ObjectMeta = eobj.ObjectMeta

				// see if it changed
				if _, ok := ref.ObjDiff(obj.Spec, eobj.Spec); ok {
					obj.Spec = eobj.Spec
					// call the event handler
					obj.Lock()
					err = serviceHandler.OnServiceUpdate(obj)
					obj.Unlock()
					if err != nil {
						log.Errorf("Error creating %s %+v. Err: %v", kind, obj, err)
						return err
					}
				}
			}
		case kvstore.Deleted:
			fobj, err := ct.findObject(kind, eobj.GetKey())
			if err != nil {
				log.Errorf("Object %s/%s not found durng delete. Err: %v", kind, eobj.GetKey(), err)
				return err
			}

			obj := fobj.(*Service)

			// Call the event reactor
			obj.Lock()
			err = serviceHandler.OnServiceDelete(obj)
			obj.Unlock()
			if err != nil {
				log.Errorf("Error deleting %s: %+v. Err: %v", kind, obj, err)
				return err
			}

			ct.delObject(kind, eobj.GetKey())
		}
	default:
		log.Fatalf("API watcher Found object of invalid type: %v on Service watch channel", tp)
	}

	return nil
}

// diffService does a diff of Service objects between local cache and API server
func (ct *ctrlerCtx) diffService(apicl apiclient.Services) {
	opts := api.ListWatchOptions{}

	// get a list of all objects from API server
	objlist, err := apicl.NetworkV1().Service().List(context.Background(), &opts)
	if err != nil {
		log.Errorf("Error getting a list of objects. Err: %v", err)
		return
	}

	// build an object map
	objmap := make(map[string]*network.Service)
	for _, obj := range objlist {
		objmap[obj.GetKey()] = obj
	}

	// if an object is in our local cache and not in API server, trigger delete for it
	for _, obj := range ct.Service().List() {
		_, ok := objmap[obj.GetKey()]
		if !ok {
			evt := kvstore.WatchEvent{
				Type:   kvstore.Deleted,
				Key:    obj.GetKey(),
				Object: &obj.Service,
			}
			ct.handleServiceEvent(&evt)
		}
	}

	// trigger create event for all others
	for _, obj := range objlist {
		evt := kvstore.WatchEvent{
			Type:   kvstore.Created,
			Key:    obj.GetKey(),
			Object: obj,
		}
		ct.handleServiceEvent(&evt)
	}
}

func (ct *ctrlerCtx) runServiceWatcher() {
	kind := "Service"

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

	// loop forever
	for {
		// create a grpc client
		apicl, err := apiclient.NewGrpcAPIClient(ct.name, ct.apisrvURL, ct.logger, rpckit.WithBalancer(ct.balancer))
		if err != nil {
			log.Warnf("Failed to connect to gRPC server [%s]\n", ct.apisrvURL)
		} else {
			log.Infof("API client connected {%+v}", apicl)

			// Service object watcher
			wt, werr := apicl.NetworkV1().Service().Watch(ctx, &opts)
			if werr != nil {
				log.Errorf("Failed to start %s watch (%s)\n", kind, werr)
				// wait for a second and retry connecting to api server
				time.Sleep(time.Second)
				continue
			}
			ct.Lock()
			ct.watchers[kind] = wt
			ct.Unlock()

			// perform a diff with API server and local cache
			time.Sleep(time.Millisecond * 100)
			ct.diffService(apicl)

			// handle api server watch events
		innerLoop:
			for {
				// wait for events
				select {
				case evt, ok := <-wt.EventChan():
					if !ok {
						log.Error("Error receiving from apisrv watcher")
						break innerLoop
					}

					// handle event
					ct.handleServiceEvent(evt)
				}
			}
			apicl.Close()
		}

		// if stop flag is set, we are done
		if ct.stoped {
			log.Infof("Exiting API server watcher")
			return
		}

		// wait for a second and retry connecting to api server
		time.Sleep(time.Second)
	}
}

// WatchService starts watch on Service object
func (ct *ctrlerCtx) WatchService(handler ServiceHandler) error {
	kind := "Service"

	ct.Lock()
	defer ct.Unlock()

	// see if we already have a watcher
	_, ok := ct.watchers[kind]
	if ok {
		return fmt.Errorf("Service watcher already exists")
	}

	// save handler
	ct.handlers[kind] = handler

	// run Service watcher in a go routine
	go ct.runServiceWatcher()

	return nil
}

// ServiceAPI returns
type ServiceAPI interface {
	Create(obj *network.Service) error
	Update(obj *network.Service) error
	Delete(obj *network.Service) error
	List() []*Service
	Watch(handler ServiceHandler) error
}

// dummy struct that implements ServiceAPI
type serviceAPI struct {
	ct *ctrlerCtx
}

// Create creates Service object
func (api *serviceAPI) Create(obj *network.Service) error {
	return api.ct.handleServiceEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Created})
}

// Update triggers update on Service object
func (api *serviceAPI) Update(obj *network.Service) error {
	return api.ct.handleServiceEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Updated})
}

// Delete deletes Service object
func (api *serviceAPI) Delete(obj *network.Service) error {
	return api.ct.handleServiceEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Deleted})
}

// List returns a list of all Service objects
func (api *serviceAPI) List() []*Service {
	var objlist []*Service

	objs := api.ct.ListObjects("Service")
	for _, obj := range objs {
		switch tp := obj.(type) {
		case *Service:
			eobj := obj.(*Service)
			objlist = append(objlist, eobj)
		default:
			log.Fatalf("Got invalid object type %v while looking for Service", tp)
		}
	}

	return objlist
}

// Watch sets up a event handlers for Service object
func (api *serviceAPI) Watch(handler ServiceHandler) error {
	return api.ct.WatchService(handler)
}

// Service returns ServiceAPI
func (ct *ctrlerCtx) Service() ServiceAPI {
	return &serviceAPI{ct: ct}
}

// LbPolicy is a wrapper object that implements additional functionality
type LbPolicy struct {
	sync.Mutex
	network.LbPolicy
	HandlerCtx interface{} // additional state handlers can store
	ctrler     *ctrlerCtx  // reference back to the controller instance
}

func (obj *LbPolicy) Write() error {
	// if there is no API server to connect to, we are done
	if (obj.ctrler == nil) || (obj.ctrler.resolver == nil) || obj.ctrler.apisrvURL == "" {
		return nil
	}

	apicl, err := obj.ctrler.apiClient()
	if err != nil {
		log.Errorf("Error creating API server clent. Err: %v", err)
		return err
	}

	// write to api server
	if obj.ObjectMeta.ResourceVersion != "" {
		nobj := *obj
		// FIXME: clear the resource version till we figure out CAS semantics
		nobj.ObjectMeta.ResourceVersion = ""

		// update it
		_, err = apicl.NetworkV1().LbPolicy().Update(context.Background(), &nobj.LbPolicy)
	} else {
		//  create
		_, err = apicl.NetworkV1().LbPolicy().Create(context.Background(), &obj.LbPolicy)
	}

	return nil
}

// LbPolicyHandler is the event handler for LbPolicy object
type LbPolicyHandler interface {
	OnLbPolicyCreate(obj *LbPolicy) error
	OnLbPolicyUpdate(obj *LbPolicy) error
	OnLbPolicyDelete(obj *LbPolicy) error
}

// handleLbPolicyEvent handles LbPolicy events from watcher
func (ct *ctrlerCtx) handleLbPolicyEvent(evt *kvstore.WatchEvent) error {
	switch tp := evt.Object.(type) {
	case *network.LbPolicy:
		eobj := evt.Object.(*network.LbPolicy)
		kind := "LbPolicy"

		log.Infof("Watcher: Got %s watch event(%s): {%+v}", kind, evt.Type, eobj)

		handler, ok := ct.handlers[kind]
		if !ok {
			log.Fatalf("Cant find the handler for %s", kind)
		}
		lbpolicyHandler := handler.(LbPolicyHandler)
		// handle based on event type
		switch evt.Type {
		case kvstore.Created:
			fallthrough
		case kvstore.Updated:
			fobj, err := ct.findObject(kind, eobj.GetKey())
			if err != nil {
				obj := &LbPolicy{
					LbPolicy:   *eobj,
					HandlerCtx: nil,
					ctrler:     ct,
				}
				ct.addObject(kind, obj.GetKey(), obj)
				// call the event handler
				obj.Lock()
				err = lbpolicyHandler.OnLbPolicyCreate(obj)
				obj.Unlock()
				if err != nil {
					log.Errorf("Error creating %s %+v. Err: %v", kind, obj, err)
					ct.delObject(kind, eobj.GetKey())
					return err
				}
			} else {
				obj := fobj.(*LbPolicy)
				obj.ObjectMeta = eobj.ObjectMeta

				// see if it changed
				if _, ok := ref.ObjDiff(obj.Spec, eobj.Spec); ok {
					obj.Spec = eobj.Spec
					// call the event handler
					obj.Lock()
					err = lbpolicyHandler.OnLbPolicyUpdate(obj)
					obj.Unlock()
					if err != nil {
						log.Errorf("Error creating %s %+v. Err: %v", kind, obj, err)
						return err
					}
				}
			}
		case kvstore.Deleted:
			fobj, err := ct.findObject(kind, eobj.GetKey())
			if err != nil {
				log.Errorf("Object %s/%s not found durng delete. Err: %v", kind, eobj.GetKey(), err)
				return err
			}

			obj := fobj.(*LbPolicy)

			// Call the event reactor
			obj.Lock()
			err = lbpolicyHandler.OnLbPolicyDelete(obj)
			obj.Unlock()
			if err != nil {
				log.Errorf("Error deleting %s: %+v. Err: %v", kind, obj, err)
				return err
			}

			ct.delObject(kind, eobj.GetKey())
		}
	default:
		log.Fatalf("API watcher Found object of invalid type: %v on LbPolicy watch channel", tp)
	}

	return nil
}

// diffLbPolicy does a diff of LbPolicy objects between local cache and API server
func (ct *ctrlerCtx) diffLbPolicy(apicl apiclient.Services) {
	opts := api.ListWatchOptions{}

	// get a list of all objects from API server
	objlist, err := apicl.NetworkV1().LbPolicy().List(context.Background(), &opts)
	if err != nil {
		log.Errorf("Error getting a list of objects. Err: %v", err)
		return
	}

	// build an object map
	objmap := make(map[string]*network.LbPolicy)
	for _, obj := range objlist {
		objmap[obj.GetKey()] = obj
	}

	// if an object is in our local cache and not in API server, trigger delete for it
	for _, obj := range ct.LbPolicy().List() {
		_, ok := objmap[obj.GetKey()]
		if !ok {
			evt := kvstore.WatchEvent{
				Type:   kvstore.Deleted,
				Key:    obj.GetKey(),
				Object: &obj.LbPolicy,
			}
			ct.handleLbPolicyEvent(&evt)
		}
	}

	// trigger create event for all others
	for _, obj := range objlist {
		evt := kvstore.WatchEvent{
			Type:   kvstore.Created,
			Key:    obj.GetKey(),
			Object: obj,
		}
		ct.handleLbPolicyEvent(&evt)
	}
}

func (ct *ctrlerCtx) runLbPolicyWatcher() {
	kind := "LbPolicy"

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

	// loop forever
	for {
		// create a grpc client
		apicl, err := apiclient.NewGrpcAPIClient(ct.name, ct.apisrvURL, ct.logger, rpckit.WithBalancer(ct.balancer))
		if err != nil {
			log.Warnf("Failed to connect to gRPC server [%s]\n", ct.apisrvURL)
		} else {
			log.Infof("API client connected {%+v}", apicl)

			// LbPolicy object watcher
			wt, werr := apicl.NetworkV1().LbPolicy().Watch(ctx, &opts)
			if werr != nil {
				log.Errorf("Failed to start %s watch (%s)\n", kind, werr)
				// wait for a second and retry connecting to api server
				time.Sleep(time.Second)
				continue
			}
			ct.Lock()
			ct.watchers[kind] = wt
			ct.Unlock()

			// perform a diff with API server and local cache
			time.Sleep(time.Millisecond * 100)
			ct.diffLbPolicy(apicl)

			// handle api server watch events
		innerLoop:
			for {
				// wait for events
				select {
				case evt, ok := <-wt.EventChan():
					if !ok {
						log.Error("Error receiving from apisrv watcher")
						break innerLoop
					}

					// handle event
					ct.handleLbPolicyEvent(evt)
				}
			}
			apicl.Close()
		}

		// if stop flag is set, we are done
		if ct.stoped {
			log.Infof("Exiting API server watcher")
			return
		}

		// wait for a second and retry connecting to api server
		time.Sleep(time.Second)
	}
}

// WatchLbPolicy starts watch on LbPolicy object
func (ct *ctrlerCtx) WatchLbPolicy(handler LbPolicyHandler) error {
	kind := "LbPolicy"

	ct.Lock()
	defer ct.Unlock()

	// see if we already have a watcher
	_, ok := ct.watchers[kind]
	if ok {
		return fmt.Errorf("LbPolicy watcher already exists")
	}

	// save handler
	ct.handlers[kind] = handler

	// run LbPolicy watcher in a go routine
	go ct.runLbPolicyWatcher()

	return nil
}

// LbPolicyAPI returns
type LbPolicyAPI interface {
	Create(obj *network.LbPolicy) error
	Update(obj *network.LbPolicy) error
	Delete(obj *network.LbPolicy) error
	List() []*LbPolicy
	Watch(handler LbPolicyHandler) error
}

// dummy struct that implements LbPolicyAPI
type lbpolicyAPI struct {
	ct *ctrlerCtx
}

// Create creates LbPolicy object
func (api *lbpolicyAPI) Create(obj *network.LbPolicy) error {
	return api.ct.handleLbPolicyEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Created})
}

// Update triggers update on LbPolicy object
func (api *lbpolicyAPI) Update(obj *network.LbPolicy) error {
	return api.ct.handleLbPolicyEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Updated})
}

// Delete deletes LbPolicy object
func (api *lbpolicyAPI) Delete(obj *network.LbPolicy) error {
	return api.ct.handleLbPolicyEvent(&kvstore.WatchEvent{Object: obj, Type: kvstore.Deleted})
}

// List returns a list of all LbPolicy objects
func (api *lbpolicyAPI) List() []*LbPolicy {
	var objlist []*LbPolicy

	objs := api.ct.ListObjects("LbPolicy")
	for _, obj := range objs {
		switch tp := obj.(type) {
		case *LbPolicy:
			eobj := obj.(*LbPolicy)
			objlist = append(objlist, eobj)
		default:
			log.Fatalf("Got invalid object type %v while looking for LbPolicy", tp)
		}
	}

	return objlist
}

// Watch sets up a event handlers for LbPolicy object
func (api *lbpolicyAPI) Watch(handler LbPolicyHandler) error {
	return api.ct.WatchLbPolicy(handler)
}

// LbPolicy returns LbPolicyAPI
func (ct *ctrlerCtx) LbPolicy() LbPolicyAPI {
	return &lbpolicyAPI{ct: ct}
}
