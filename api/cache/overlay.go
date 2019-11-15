package cache

import (
	"context"
	"fmt"
	"math/rand"
	"reflect"
	"strings"
	"sync"
	"time"

	"google.golang.org/grpc/codes"

	"github.com/deckarep/golang-set"
	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/types"
	"github.com/pkg/errors"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache/ovpb"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/requirement"
	"github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/helper"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

const maxConsistentUpdateRetries = 10
const maxOverlayOps = 1024

type dryRunMarker struct {
	verVer int64
}

type preCommmitApply struct{}

// setStagingBufferInGrpcMD sets the GRPC metadata with buffer ID
func setStagingBufferInGrpcMD(ctx context.Context, tenant, id string) context.Context {
	pair := metadata.Pairs(apiserver.RequestParamStagingBufferID, id, apiserver.RequestParamTenant, tenant)
	inMd, ok := metadata.FromIncomingContext(ctx)
	if ok {
		nMd := metadata.Join(inMd, pair)
		return metadata.NewIncomingContext(ctx, nMd)
	}

	return metadata.NewIncomingContext(ctx, pair)
}

func setReplaceStatusInGrpcMD(ctx context.Context) context.Context {
	pair := metadata.Pairs(apiserver.RequestParamReplaceStatusField, "true")
	inMd, ok := metadata.FromIncomingContext(ctx)
	if ok {
		nMd := metadata.Join(inMd, pair)
		return metadata.NewIncomingContext(ctx, nMd)
	}

	return metadata.NewIncomingContext(ctx, pair)

}

// SetDruRun sets the dry run flag in the the context
func setDryRun(ctx context.Context, val int64) context.Context {
	dval := &dryRunMarker{val}
	return context.WithValue(ctx, dryRunMarker{}, dval)
}

// GetDryRun retrieves the dry run flag from the context
func getDryRun(ctx context.Context) *dryRunMarker {
	ret := ctx.Value(dryRunMarker{})
	if ret == nil {
		return nil
	}
	if dr, ok := ret.(*dryRunMarker); ok {
		return dr
	}
	return nil
}

// IsDryRun returns true if the context is marked for dry run
func IsDryRun(ctx context.Context) bool {
	r := getDryRun(ctx)
	if r == nil {
		return false
	}
	return true
}

func setPreCommitApply(ctx context.Context) context.Context {
	dval := &preCommmitApply{}
	return context.WithValue(ctx, preCommmitApply{}, dval)
}

func isPreCommitApply(ctx context.Context) bool {
	r := ctx.Value(preCommmitApply{})
	return r != nil
}

type overlayObj struct {
	revision    int64
	verVer      int64
	oper        apiOper
	primary     bool
	serviceName string
	methodName  string
	key         string
	URI         string
	touch       bool
	// orig is the original object as seen from the user. This is used for
	//  - Persisting the staged object to kvstore
	//  - call to HandleInvocation during Verification
	// the APIVersion of the object is that of the original call
	orig runtime.Object
	// val is the object after all transformers have been applied. This is the view
	//  of the object as it would have been when saved as kvstore object. This is used
	//  - for all reads on the overlay
	//  - when object is finalized in a Commit() operation
	val      runtime.Object
	updateFn kvstore.UpdateFunc
	resVer   string
}

type overlay struct {
	sync.RWMutex
	// Exposes all methods of the cache with some frills
	apiintf.CacheInterface
	server   apiserver.Server
	revision int64
	// verVer is verification version
	verVer         int64
	tenant         string
	id             string
	baseKey        string
	ephemeral      bool
	overlay        map[string]*overlayObj
	comparators    []kvstore.Cmp
	preCommitComps []kvstore.Cmp
	reqs           apiintf.RequirementSet
	versioner      runtime.Versioner
	primaryCount   int
	maxOvEntries   int
}

type overlayMap struct {
	sync.Mutex
	ovMap map[string]apiintf.OverlayInterface
}

var (
	overlaysSingleton overlayMap
	once              sync.Once
)

func initOverlayMap() {
	overlaysSingleton.ovMap = make(map[string]apiintf.OverlayInterface)
}

// GetOverlay retrieves an existing overlay with id
func GetOverlay(tenant, id string) (apiintf.OverlayInterface, error) {
	defer overlaysSingleton.Unlock()
	overlaysSingleton.Lock()
	once.Do(initOverlayMap)
	name := tenant + "/" + id
	if o, ok := overlaysSingleton.ovMap[name]; ok {
		return o, nil
	}
	return nil, errors.New("not found")
}

// DelOverlay deletes an existing overlay with id
func DelOverlay(tenant, id string) error {
	overlaysSingleton.Lock()
	once.Do(initOverlayMap)
	name := tenant + "/" + id
	o, ok := overlaysSingleton.ovMap[name]
	if !ok {
		overlaysSingleton.Unlock()
		return errors.New("not found")
	}
	log.Infof("deleting overlay %s", name)
	delete(overlaysSingleton.ovMap, name)
	overlaysSingleton.Unlock()
	ov := o.(*overlay)
	defer ov.Unlock()
	ov.Lock()
	// delete all persisted data
	ov.cleanupKvObjs(context.TODO())

	for k := range ov.overlay {
		delete(ov.overlay, k)
	}
	return nil
}

// NewOverlay creates a new overlay over the passed cache.Interface
func NewOverlay(tenant, id, baseKey string, c apiintf.CacheInterface, apisrv apiserver.Server, local bool) (apiintf.OverlayInterface, error) {
	defer overlaysSingleton.Unlock()
	overlaysSingleton.Lock()
	once.Do(initOverlayMap)
	name := tenant + "/" + id
	if _, ok := overlaysSingleton.ovMap[name]; ok {
		return nil, errors.New("already exists")
	}
	o := &overlay{
		CacheInterface: c,
		tenant:         tenant,
		id:             id,
		baseKey:        strings.TrimSuffix(baseKey, "/"),
		overlay:        make(map[string]*overlayObj),
		server:         apisrv,
		ephemeral:      local,
		revision:       1,
		versioner:      runtime.NewObjectVersioner(),
		maxOvEntries:   maxOverlayOps,
	}
	o.reqs = requirement.NewRequirementSet(apisrv.GetGraphDB(), o, apisrv)
	log.Infof("creating new overlay %s, base path %s", id, o.baseKey)
	overlaysSingleton.ovMap[name] = o
	return o, nil
}

// NewLocalOverlay creates a new non-user created overlay
func NewLocalOverlay(tenant, id, baseKey string, c apiintf.CacheInterface, apisrv apiserver.Server) (apiintf.OverlayInterface, error) {
	ret, err := NewOverlay(tenant, id, baseKey, c, apisrv, true)
	return ret, err
}

// SetOverlay force sets the overlay.
//  ONLY USED FOR TESTING
func SetOverlay(tenant, id string, ov apiintf.OverlayInterface) {
	defer overlaysSingleton.Unlock()
	overlaysSingleton.Lock()
	once.Do(initOverlayMap)
	name := tenant + "/" + id
	overlaysSingleton.ovMap[name] = ov
}

// GetOverlays returns all the overlays
//  ONLY USED FOR TESTING
func GetOverlays() map[string]apiintf.OverlayInterface {
	ret := make(map[string]apiintf.OverlayInterface)
	defer overlaysSingleton.Unlock()
	overlaysSingleton.Lock()
	for k, v := range overlaysSingleton.ovMap {
		ret[k] = v
	}
	return ret
}

func restoreOverlays(ctx context.Context, base string, c apiintf.CacheInterface, server apiserver.Server) error {
	// XXX-TODO(sanjayt): need to handle issue where we are recovering from a prefix watcher failure
	//   and the overlay has been deleted behind our back. Very unlikely case when there is only one
	//   API server in the cluster.
	overlaysSingleton.Lock()
	for _, v := range overlaysSingleton.ovMap {
		ov := v.(*overlay)
		ov.Lock()
		ov.overlay = make(map[string]*overlayObj)
		ov.comparators = nil
		ov.Unlock()
	}
	initOverlayMap()
	overlaysSingleton.Unlock()
	// restore from the kvstore backend for each overlay.
	kv := c.GetKvConn()
	into := &overlaypb.BufferItemList{}
	err := kv.List(ctx, base, into)
	if err != nil {
		log.ErrorLog("msg", "failed to list buffer items", "error", err)
		return err
	}
	log.Infof("Restoring [%d] Overlays", len(into.Items))
	// This is not a dry run, but staging is supported from APIGw only so set replace status
	nctx := apiutils.SetVar(setReplaceStatusInGrpcMD(ctx), apiutils.CtxKeyAPISrvInitRestore, true)
	for _, v := range into.Items {
		o, err := GetOverlay(v.Tenant, v.Name)
		if err != nil {
			o, err = NewOverlay(v.Tenant, v.Name, base, c, server, false)
			if err != nil {
				log.Errorf("could not create overlay [%v/%v] to restore (%s)", v.Tenant, v.Name, err)
				continue
			}
		}
		ov := o.(*overlay)
		if err = ov.restoreKvObj(nctx, v); err != nil {
			log.Errorf("failed to restore overlay object (%v)", apierrors.FromError(err))
		}
	}
	// XXX-TODO(sanjayt): This will miss any empty Overlays in the kvstore. This should be restored seperately
	//  for each tenant in the cache.
	return nil
}

func makeBufferKey(base, tenant, id, key string) string {
	return fmt.Sprintf("%s/tenant/%v/buffer/%v/%s", base, tenant, id, strings.TrimPrefix(key, "/"))
}

func parseKey(base, key string) (string, string, error) {
	s := strings.TrimPrefix(key, base)
	p := strings.SplitN(s, "/", 6)
	if len(p) < 6 {
		return "", "", errors.New("could not parse")
	}
	return p[2], p[4], nil
}

func (c *overlay) findObjects(ctx context.Context, key string, into runtime.Object) (*overlayObj, error) {
	err := c.CacheInterface.Get(ctx, key, into)
	if err != nil {
		into = nil
	}
	ovObj := c.overlay[key]
	if ovObj != nil && !ovObj.primary {
		// ignore the object if it is not a primary object.
		ovObj = nil
	}
	return ovObj, err
}

func (c *overlay) makeBufferKey(key string) string {
	return makeBufferKey(c.baseKey, c.tenant, c.id, key)
}

func (c *overlay) parseKey(key string) (string, string, error) {
	return parseKey(c.baseKey, key)
}

func (c *overlay) createKvObj(ctx context.Context, uri, key string, obj *overlayObj) error {
	if c.ephemeral {
		return nil
	}
	if _, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPISrvInitRestore); ok {
		return nil
	}
	if !obj.primary {
		return nil
	}
	if obj.orig == nil && obj.oper != operDelete {
		return errors.New("primary without original set")
	}
	kv := c.GetKvConn()
	if kv == nil {
		return errors.New("no connection to KV store to persist")
	}
	ovKey := c.makeBufferKey(key)
	kind := ""
	if obj.val != nil {
		kind = obj.val.GetObjectKind()
	}
	ovObj := &overlaypb.BufferItem{
		TypeMeta:   api.TypeMeta{Kind: "BufferItem"},
		ObjectMeta: api.ObjectMeta{Tenant: c.tenant, Name: c.id},
		ItemId: &overlaypb.BufferItem_Id{
			Kind:      kind,
			URI:       uri,
			Operation: string(obj.oper),
			Method:    obj.methodName,
			Service:   obj.serviceName,
		},
	}
	var err error
	if obj.orig != nil {
		ovObj.Object, err = types.MarshalAny(obj.orig.(proto.Message))
	}
	if err != nil {
		return err
	}
	err = kv.Create(ctx, ovKey, ovObj)
	if err != nil {
		log.Errorf("[%v]Create overlay Buffer Item failed (%s)\n", c.id, err)
		return err
	}
	return nil
}

func (c *overlay) updateKvObj(ctx context.Context, key string, obj *overlayObj) error {
	if c.ephemeral {
		return nil
	}
	if _, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPISrvInitRestore); ok {
		return nil
	}
	if !obj.primary {
		return nil
	}
	if obj.orig == nil && obj.oper != operDelete {
		return errors.New("primary without original set")
	}
	kv := c.GetKvConn()
	if kv == nil {
		return errors.New("no connection to KV store to persist")
	}
	kind := ""
	if obj.orig == nil && obj.oper != operDelete {
		return errors.New("invalid operation")
	}
	if obj.orig != nil {
		kind = obj.val.GetObjectKind()
	}
	ovKey := c.makeBufferKey(key)
	ovObj := &overlaypb.BufferItem{
		TypeMeta:   api.TypeMeta{Kind: "BufferItem"},
		ObjectMeta: api.ObjectMeta{Tenant: c.tenant, Name: c.id},
		ItemId: &overlaypb.BufferItem_Id{
			Kind:      kind,
			URI:       key,
			Operation: string(obj.oper),
			Method:    obj.methodName,
			Service:   obj.serviceName,
		},
	}
	var err error
	if obj.orig != nil {
		ovObj.Object, err = types.MarshalAny(obj.orig.(proto.Message))
	}
	if err != nil {
		return err
	}
	err = kv.Update(ctx, ovKey, ovObj)
	if err != nil {
		log.Errorf("[%v]Update overlay Buffer Item failed (%s)\n", c.id, err)
		return err
	}
	return nil
}

func (c *overlay) deleteKvObj(ctx context.Context, key string, obj *overlayObj) error {
	if c.ephemeral {
		return nil
	}
	if _, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPISrvInitRestore); ok {
		// If we are in restore path, no need to persist to kvstore.
		return nil
	}
	if !obj.primary {
		return nil
	}
	kv := c.GetKvConn()
	if kv == nil {
		return errors.New("no connection to KV store to persist")
	}
	ovKey := c.makeBufferKey(key)

	err := kv.Delete(ctx, ovKey, nil)
	if err != nil {
		log.Errorf("[%v]Delete overlay Buffer Item failed (%s)\n", c.id, err)
		return err
	}
	return nil
}

// restoreKvObj
func (c *overlay) restoreKvObj(ctx context.Context, in *overlaypb.BufferItem) error {
	nctx := setStagingBufferInGrpcMD(ctx, in.Tenant, in.Name)
	svc := c.server.GetService(in.ItemId.Service)
	if svc == nil {
		return fmt.Errorf("could not find service during restore [%s][%s/%s]", in.ItemId.Service, in.Tenant, in.Name)
	}
	m := svc.GetMethod(in.ItemId.Method)
	if m == nil {
		return fmt.Errorf("could not find method during restore [%s/%s][%s/%s]", in.ItemId.Service, in.ItemId.Method, in.Tenant, in.Name)
	}

	robj := &types.DynamicAny{}
	err := types.UnmarshalAny(in.Object, robj)
	if err != nil {
		return errors.Wrap(err, "unable to unmarshal object during restore")
	}
	v, ok := robj.Message.(runtime.Object)
	if !ok {
		return fmt.Errorf("unmarshalled object not correct type")
	}
	tm := reflect.ValueOf(v).MethodByName("ApplyStorageTransformer")
	if tm.IsValid() {
		args := []reflect.Value{reflect.ValueOf(ctx), reflect.ValueOf(false)}
		ev := tm.Call(args)
		if !ev[0].IsNil() {
			err := ev[0].Interface().(error)
			log.ErrorLog("msg", "failed to tranform from storage", "error", err, "key")
		}
	}
	var obj interface{}
	obj = v
	if obj != nil && reflect.TypeOf(obj).Kind() == reflect.Ptr {
		obj = reflect.Indirect(reflect.ValueOf(obj)).Interface()
	}
	_, err = m.HandleInvocation(nctx, obj)
	if err != nil {
		return errors.Wrap(err, "failed to restore object")
	}
	return nil
}

// cleanupKvObjs cleans up all persisted data for the overlay
func (c *overlay) cleanupKvObjs(ctx context.Context) error {
	if c.ephemeral {
		return nil
	}
	kv := c.GetKvConn()
	if kv == nil {
		return errors.New("no connection to KV store to persist")
	}
	ovKey := c.makeBufferKey("")
	return c.PrefixDelete(ctx, ovKey)
}

// create - with following rules for non-DryRun cases
// NotInCache, NotInOverlay - Pass
// NotInCache, InOverlay - Fail
// InCache, NotInOverlay - Fail
// InCache, InOverlay - Fail
// For DryRun cases
//  - If primary then object is expected to be already existing in overlay
//  - object cannot exist in cache.
func (c *overlay) create(ctx context.Context, service, method, uri, key string, orig, obj runtime.Object, dryRun int64, primary bool) error {
	log.InfoLog("msg", "creating overlay object", "primary", primary, "service", service, "method", method, "key", key, "uri", uri, "dryrun", dryRun)
	if obj == nil {
		return errors.New("invalid parameters")
	}
	ovObj, err := c.findObjects(ctx, key, nil)
	if err == nil {
		// do a special check to see if the object is a secondary in the overlay
		ovObj, ok := c.overlay[key]
		if ok && ovObj != nil {
			if ovObj.oper == operDelete {
				ovObj.oper = operUpdate
				ovObj.val = obj
				ovObj.orig = orig
				ovObj.primary = primary
				if primary {
					if err := c.updateKvObj(ctx, key, ovObj); err != nil {
						log.Errorf("failed to persist for key %v(%s)", key, err)
					}
				}
				return nil
			}
		}
		return fmt.Errorf("already exists in cache")
	}

	if primary && dryRun == 0 {
		if ovObj != nil {
			return errors.New("already exists")
		}
	}

	if primary && dryRun > 0 {
		// This is a dry run, the primary object should exist in the overlay,
		//  dont create a new object if it does not exist.
		if ovObj == nil {
			log.ErrorLog("msg", "did not find overlay object in dryRun", "dryRun", dryRun, "key", key, "overlay", c.id)
			return errors.New("object not found")
		}
		ovObj.verVer = dryRun
		ovObj.val = obj
		return nil
	}

	objm, err := runtime.GetObjectMeta(obj)
	if err != nil {
		log.ErrorLog("msg", "failed to retrieve object meta", "operation", "create")
		return err
	}
	objm.GenerationID = "1"

	ovObj = &overlayObj{
		oper:        operCreate,
		key:         key,
		orig:        orig,
		val:         obj,
		serviceName: service,
		methodName:  method,
		primary:     primary,
		URI:         uri,
		verVer:      dryRun,
		touch:       false,
	}
	if err := c.createKvObj(ctx, uri, key, ovObj); err != nil {
		log.Errorf("failed to persist for key %v(%s)", key, err)
	}
	log.DebugLog("msg", "successful creating object", "key", key, "overlay", c.id)
	refs := requirement.GetRefRequirements(ctx, key, apiintf.CreateOper, obj, c.server, c)
	if refs != nil {
		c.reqs.AddRequirement(refs)
	}
	// Overwrite any existing overlay object with a create Oper.
	c.overlay[key] = ovObj
	return nil
}

// Create creates a object in overlay cache. If the object already exists in the
//  API Cache the create fails, else the object is added to the overlay cache. No
//  notifications are generated till the commit of the buffer.
func (c *overlay) Create(ctx context.Context, key string, obj runtime.Object) error {
	// if forced persist, persist and return
	if fpi, ok := apiutils.GetVar(ctx, apiutils.CtxKeyPersistDirectKV); ok && fpi.(bool) {
		return c.CacheInterface.Create(ctx, key, obj)
	}

	var verVer int64
	dm := getDryRun(ctx)
	if dm == nil {
		defer c.Unlock()
		c.Lock()
	} else {
		verVer = dm.verVer
	}
	return c.create(ctx, "", "", "", key, nil, obj, verVer, false)
}

// update operation on the overlay, rules:
//  - object has to exist in either the cache or the overlay for the operation to succeed
//  - if object exists in the cache, the resultant operation in overlay is update
//  - if object exists in the overlay then the following rules apply
//  - Primary always takes precedence. If there is already a primary object, then a secondary update is not allowed.
//  - only one secondary update operation is allowed. Multiple updates would make the overlay unpredictable since secondary
//  	updates are because of updates during verification/Commit process and order cannot be guaranteed. It is hard to
//      to catch this during regular secondary update. We catch this during dryRun by making sure each secondary object is
//      updated only once during dry run.
//  - Primary on top of secondary always turns the object to primary
//  - update with existing create in overlay -> create
//  - update with exiting update in overlay -> update
//  - update with existing delete in overlay -> fail if no cache entry exists, update operation if cache entry exists
func (c *overlay) update(ctx context.Context, service, method, uri, key string, orig, obj runtime.Object, updateFn kvstore.UpdateFunc, resVer string, dryRun int64, primary bool) error {
	if obj == nil {
		return errors.New("invalid parameters")
	}
	log.InfoLog("msg", "updating overlay object", "primary", primary, "service", service, "method", method, "key", key, "uri", uri, "dryrun", dryRun, "ResVersion", resVer)
	var cacheObj runtime.Object
	ovObj, err := c.findObjects(ctx, key, cacheObj)

	if dryRun == 0 {
		// Not in cache, not in overlay, update not allowed
		// findObjects only returns primary Overlay objects, But in this case it is
		// valid to fail in non-dryrun cases, because
		//  - 2 secondary ops on the same object is not allowed.
		//  - there is no cache object so update is invalid.
		if err != nil && ovObj == nil {
			log.Errorf("cannot update object [%v] not found in cache", key)
			return errors.Wrap(err, "cannot update without cache object")
		}
	}
	pcApply := isPreCommitApply(ctx)

	var inOv bool
	if ovObj != nil {
		inOv = true
	}
	ovObj = c.overlay[key]

	if ovObj != nil && !primary && dryRun > 0 && ovObj.verVer == dryRun && !pcApply {
		// more than one secondary operation only allowed during dryRun.
		return errors.New("operation would conflict")
	}

	// If dry run and primary, the object should exist in the overlay
	if ovObj == nil && primary && dryRun > 0 {
		return errors.New("object not found")
	}

	if ovObj != nil {
		if ovObj.primary && !primary && !pcApply {
			// Secondary update on a primary would conflict
			return errors.New("primary operation would conflict")
		}
		if pcApply {
			primary = ovObj.primary
		}
	}

	if err != nil && ovObj.oper == operDelete {
		// current operation cannot be delete if there is no cache object
		return errors.New("object not found")
	}

	o, e1 := obj.Clone(nil)
	if e1 != nil {
		return e1
	}
	cacheObj = o.(runtime.Object)
	// In cache, not in overlay
	oper := operUpdate
	if err == nil && ovObj == nil {
		ovObj = &overlayObj{
			key:      key,
			val:      cacheObj,
			updateFn: updateFn,
			resVer:   resVer,
		}
	}

	if err != nil {
		// Not in Cache, in overlay
		oper = operCreate
		ovObj.val = obj
		objm, err := runtime.GetObjectMeta(obj)
		if err != nil {
			log.ErrorLog("msg", "failed to retrieve object meta", "operation", "create", "overlay", c.id)
			return err
		}
		objm.GenerationID = "1"
	} else {
		// In cache and in overlay
		ovObj.val = obj
	}
	ovObj.oper = oper
	ovObj.primary = primary
	if !pcApply {
		// If this is coming from an apply then retain the updateFn, else overwrite.
		ovObj.updateFn = updateFn
		ovObj.resVer = resVer
	}
	if oper == operUpdate && primary && updateFn != nil {
		c.reqs.NewConsUpdateRequirement([]apiintf.ConstUpdateItem{{Key: key, Func: updateFn, ResourceVersion: resVer, Into: obj}})
	}
	ovObj.val = cacheObj
	ovObj.URI = uri
	ovObj.orig = orig
	ovObj.serviceName = service
	ovObj.methodName = method
	ovObj.verVer = dryRun
	ovObj.touch = false

	if !inOv {
		err = c.createKvObj(ctx, uri, key, ovObj)
		if err != nil {
			log.Errorf("Create overlay Buffer Item failed [%s](%s)\n", key, err)
			return err
		}
	} else {
		err = c.updateKvObj(ctx, key, ovObj)
		if err != nil {
			log.Errorf("update overlay Buffer Item failed [%s](%s)\n", key, err)
			return err
		}
	}
	// in case of consistent update, late resolve the requirements. No use resolving here.
	if oper == operCreate || (oper == operUpdate && updateFn == nil) {
		refs := requirement.GetRefRequirements(ctx, key, apiintf.APIOperType(ovObj.oper), obj, c.server, c)
		if refs != nil {
			c.reqs.AddRequirement(refs)
		}
	}
	c.overlay[key] = ovObj
	return nil
}

// Update stages update of an object in overlay cache. The object should exist either in the API cache or in the
//   overlay for an update to succeed. If neither exist then the operation fails.  If an operations is
//   already in the overlay the then following table defines the resultant operation
//   Existing entry in Overlay    -> State of Overlay after Update operation
//   Any primary object			  => fail due to conflict
//   Create                       =>  Create[New contents]
//   Update                       =>  Update[New Contents]
//   Delete                       =>  Update[New Contents] IF API cache has object or fail
func (c *overlay) Update(ctx context.Context, key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	if fpi, ok := apiutils.GetVar(ctx, apiutils.CtxKeyPersistDirectKV); ok && fpi.(bool) {
		return c.CacheInterface.Update(ctx, key, obj)
	}
	var verVer int64
	dm := getDryRun(ctx)
	if dm == nil {
		defer c.Unlock()
		c.Lock()
	} else {
		verVer = dm.verVer
	}
	// Cmps are not used in API path, so ignore.
	return c.update(ctx, "", "", "", key, nil, obj, nil, "", verVer, false)
}

// delete an object from the cache - rules
//  - primary supercedes secondary objects.
//  - secondary delete if there is an existing primary object is not allowed.
//  - multiple secondary operations on the same object not allowed, like in update case this is caught during dryRun
//  - if there is no object in the cache, delete removes the object from overlay
func (c *overlay) delete(ctx context.Context, service, method, uri, key string, orig, into runtime.Object, dryRun int64, primary bool) error {
	log.Infof("processing delete operation for key [%v]", key)
	ovObj, err := c.findObjects(ctx, key, into)
	// Not in Overlay, not in Cache
	if ovObj == nil && err != nil {
		into = nil
		if err != nil {
			return err
		}
		return fmt.Errorf("does not exist")
	}
	ovObj = c.overlay[key]
	// if primary and dryrun, the primary object should exist in the overlay
	if ovObj == nil && primary && dryRun > 0 {
		return errors.New("object not found")
	}

	// secondary operation can repeat only in dry run
	if ovObj != nil && !primary && dryRun > 0 && ovObj.verVer == dryRun {
		// more than one secondary operation only allowed during dryRun.
		return errors.New("operation would conflict")
	}

	// if secondary operation and there is a existing primary, disallow
	if ovObj != nil && ovObj.primary && !primary {
		return errors.New("operation would conflict")
	}

	// in overlay, not in cache
	if ovObj != nil && err != nil {
		if err := c.deleteKvObj(ctx, key, ovObj); err != nil {
			log.Errorf("Failed to delete buffer item fromm kv store [%s](%s)", key, err)
			return err
		}
		delete(c.overlay, key)
		if ovObj.val != nil {
			ovObj.val.Clone(into)
		}
		return nil
	}

	// in Overlay, in cache
	if ovObj != nil {
		if ovObj.val != nil {
			ovObj.val.Clone(into)
		}
		ovObj.oper = operDelete
		if err := c.updateKvObj(ctx, key, ovObj); err != nil {
			log.Errorf("Failed to update buffer item fromm kv store [%s](%s)", key, err)
			return err
		}
		ovObj.methodName = method
		ovObj.serviceName = service
		ovObj.primary = primary
		ovObj.URI = uri
		ovObj.verVer = dryRun
		ovObj.orig = orig
		ovObj.touch = false
		return nil
	}

	// not in Overlay, in cache
	var cacheObj runtime.Object
	if into != nil {
		o, e := into.Clone(nil)
		if e != nil {
			return e
		}
		cacheObj = o.(runtime.Object)
	}
	ovObj = &overlayObj{
		key:         key,
		oper:        operDelete,
		val:         cacheObj,
		verVer:      dryRun,
		methodName:  method,
		serviceName: service,
		URI:         uri,
		primary:     primary,
		orig:        orig,
		touch:       false,
	}
	if err := c.createKvObj(ctx, "", key, ovObj); err != nil {
		log.Errorf("Failed to update buffer item fromm kv store [%s](%s)", key, err)
		return err
	}
	refs := requirement.GetRefRequirements(ctx, key, apiintf.DeleteOper, nil, c.server, c)
	if refs != nil {
		c.reqs.AddRequirement(refs)
	}
	c.overlay[key] = ovObj
	log.Infof("processed delete operation for key [%v]", key)
	return nil
}

// Delete stages a delete in the overlay Cache. The object should exist in the overlay or the API cache.
func (c *overlay) Delete(ctx context.Context, key string, into runtime.Object, cs ...kvstore.Cmp) error {
	if fpi, ok := apiutils.GetVar(ctx, apiutils.CtxKeyPersistDirectKV); ok && fpi.(bool) {
		return c.CacheInterface.Delete(ctx, key, into, cs...)
	}
	var verVer int64
	dm := getDryRun(ctx)
	if dm == nil {
		defer c.Unlock()
		c.Lock()
	} else {
		verVer = dm.verVer
	}
	return c.delete(ctx, "", "", "", key, nil, into, verVer, false)
}

// ConsistentUpdate updates an object in the backend KVStore if it already exists. If the operation is successful
//  the cache is updated and watch notifications are generated for established watches.
func (c *overlay) ConsistentUpdate(ctx context.Context, key string, into runtime.Object, updateFunc kvstore.UpdateFunc) error {
	var verVer int64
	dm := getDryRun(ctx)
	if dm == nil {
		defer c.Unlock()
		c.Lock()
	} else {
		verVer = dm.verVer
	}
	if updateFunc != nil {
		c.reqs.NewConsUpdateRequirement([]apiintf.ConstUpdateItem{{Key: key, Func: updateFunc, Into: into}})
	}
	return c.update(ctx, "", "", "", key, nil, into, updateFunc, "", verVer, false)
}

// Get retrieves object from the cache if it exists.
func (c *overlay) Get(ctx context.Context, key string, into runtime.Object) error {
	dm := getDryRun(ctx)
	if dm == nil {
		defer c.RUnlock()
		c.RLock()
	}
	cl, err := into.Clone(nil)
	if err != nil {
		return err
	}

	// If request is to ignore the overlay, do so.
	if v, ok := apiutils.GetVar(ctx, apiutils.CtxKeyGetPersistedKV); ok {
		if b, ok := v.(bool); ok && b {
			return c.CacheInterface.Get(ctx, key, into)
		}
	}

	err = c.CacheInterface.Get(ctx, key, cl.(runtime.Object))
	ovObj := c.overlay[key]
	if ovObj != nil {
		if ovObj.oper == operDelete {
			return errors.New("not found")
		}
		ovObj.val.Clone(into)
		if err == nil {
			ver, err := c.versioner.GetVersion(cl.(runtime.Object))
			if err == nil {
				c.versioner.SetVersion(into, ver)
			}
		}
		return nil
	}

	return c.CacheInterface.Get(ctx, key, into)
}

func (c *overlay) list(ctx context.Context, prefix string, opts api.ListWatchOptions) []*overlayObj {
	ret := []*overlayObj{}
	kind := ""
	k, ok := apiutils.GetVar(ctx, apiutils.CtxKeyObjKind)
	if ok {
		kind = k.(string)
	}
	filters, err := getFilters(opts, kind)
	if err != nil {
		return nil
	}
	for key, obj := range c.overlay {
		skip := false
		if strings.HasPrefix(key, prefix) {
			for _, fn := range filters {
				if !fn(obj.val, nil) {
					skip = true
					break
				}
			}
			if !skip {
				ret = append(ret, c.overlay[key])
			}
		}
	}
	return ret
}

// ListFiltered returns a list in into filtered as per the opts passed in
func (c *overlay) ListFiltered(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error {
	dm := getDryRun(ctx)
	if dm == nil {
		defer c.RUnlock()
		c.RLock()
	}
	// Collect from the API cache
	intoItems, err := helper.ValidListObjForDecode(into)
	if err != nil {
		return err
	}
	intoItems.Set(reflect.MakeSlice(intoItems.Type(), 0, intoItems.Cap()))
	ptr := false
	elem := intoItems.Type().Elem()
	if elem.Kind() == reflect.Ptr {
		ptr = true
	}
	// If request is to ignore the overlay, do so.
	if v, ok := apiutils.GetVar(ctx, apiutils.CtxKeyGetPersistedKV); ok {
		if b, ok := v.(bool); ok && b {
			return c.CacheInterface.ListFiltered(ctx, prefix, into, opts)
		}
	}

	o, err := into.Clone(nil)
	if err != nil {
		return err
	}
	cacheObjs := o.(runtime.Object)
	cacheSet := mapset.NewSet()
	c.CacheInterface.ListFiltered(ctx, prefix, cacheObjs, opts)
	cItems, err := helper.ValidListObjForDecode(cacheObjs)
	if err != nil {
		return err
	}
	cacheItems := make(map[string]int)
	for i := 0; i < cItems.Len(); i++ {
		v := cItems.Index(i)
		// Objects in the cache are assumed to be always API objects so blindly call MakeKey()
		if v.IsNil() {
			panic(fmt.Sprintf("index %d is nil, len %d", i, cItems.Len()))
		}
		m := v.MethodByName("MakeKey")
		k := m.Call([]reflect.Value{reflect.ValueOf("overlaytemp")})
		key := k[0].Interface().(string)
		cacheItems[key] = i
		cacheSet.Add(key)
	}

	// Get list from overlay
	ovObjs := c.list(ctx, prefix, opts)
	ovSet := mapset.NewSet()
	ovItems := make(map[string]int)
	for i, v := range ovObjs {
		// Objects in the overlay are assumed to be always API objects so blindly call MakeKey()
		k := reflect.ValueOf(v.val).MethodByName("MakeKey").Call([]reflect.Value{reflect.ValueOf("overlaytemp")})
		key := k[0].Interface().(string)
		ovItems[key] = i
		ovSet.Add(key)
	}
	cache := true
	ops := []apiOper{operCreate, operUpdate}
	// Merge the results from overlay and cache
	cacheOnly := cacheSet.Difference(ovSet)
	ovOnly := ovSet.Difference(cacheSet)
	common := cacheSet.Intersect(ovSet)

	addCacheFn := func(in interface{}) bool {
		k := in.(string)
		var v runtime.Object
		if cache {
			v = cItems.Index(cacheItems[k]).Interface().(runtime.Object)
		} else {
			o := ovObjs[ovItems[k]]
			found := false
			for _, x := range ops {
				if o.oper == x {
					found = true
					break
				}
			}
			if found {
				v = ovObjs[ovItems[k]].val
			} else {
				return false
			}
		}
		if ptr {
			intoItems.Set(reflect.Append(intoItems, reflect.ValueOf(v)))
		} else {
			intoItems.Set(reflect.Append(intoItems, reflect.ValueOf(v).Elem()))
		}
		return false
	}
	cacheOnly.Each(addCacheFn)
	cache = false
	ovOnly.Each(addCacheFn)
	ops = []apiOper{operCreate, operUpdate}
	common.Each(addCacheFn)
	return nil
}

// List lists all the object of prefix. it is expected that all objects are of the same type and into is a List object.
func (c *overlay) List(ctx context.Context, prefix string, into runtime.Object) error {
	return c.ListFiltered(ctx, prefix, into, api.ListWatchOptions{})
}

func (c *overlay) CreatePrimary(ctx context.Context, service, method, uri, key string, orig, obj runtime.Object) error {
	var verVer int64
	dm := getDryRun(ctx)
	if dm == nil {
		defer c.Unlock()
		c.Lock()
	} else {
		verVer = dm.verVer
	}
	if c.primaryCount > c.maxOvEntries {
		return fmt.Errorf("too many operations in the buffer")
	}
	c.primaryCount++
	if !c.ephemeral && (service == "" || method == "") {
		panic("primary with no service or method")
	}
	return c.create(ctx, service, method, uri, key, orig, obj, verVer, true)
}
func (c *overlay) UpdatePrimary(ctx context.Context, service, method, uri, key, resVer string, orig, obj runtime.Object, updateFn kvstore.UpdateFunc) error {
	var verVer int64
	dm := getDryRun(ctx)
	if dm == nil {
		defer c.Unlock()
		c.Lock()
	} else {
		verVer = dm.verVer
	}
	if c.primaryCount > c.maxOvEntries {
		return fmt.Errorf("too many operations in the buffer")
	}
	c.primaryCount++
	if !c.ephemeral && (service == "" || method == "") {
		panic("primary with no service or method")
	}

	return c.update(ctx, service, method, uri, key, orig, obj, updateFn, resVer, verVer, true)
}

func (c *overlay) DeletePrimary(ctx context.Context, service, method, uri, key string, orig, into runtime.Object) error {
	var verVer int64
	dm := getDryRun(ctx)
	if dm == nil {
		defer c.Unlock()
		c.Lock()
	} else {
		verVer = dm.verVer
	}
	if c.primaryCount > c.maxOvEntries {
		return fmt.Errorf("too many operations in the buffer")
	}
	c.primaryCount++
	if !c.ephemeral && (service == "" || method == "") {
		panic("primary with no service or method")
	}
	return c.delete(ctx, service, method, uri, key, orig, into, verVer, true)
}

// verify verifies the current Overlay contents. Expects the caller holds the lock on the overlay.
func (c *overlay) verify(ctx context.Context, server apiserver.Server) (apiintf.OverlayStatus, int64) {
	var ret apiintf.OverlayStatus
	c.revision++
	c.verVer = c.revision

	// Staging currently only from API gateway, set replace status.
	nctx := setReplaceStatusInGrpcMD(ctx)
	updCtx := setDryRun(nctx, c.verVer)

	// empty all ephemeral parts of the overlay. This includes the comparators and the secondary entries in the overlay.
	//  all these will be repopulated as we verify the primary entries.
	c.comparators = nil
	for k, v := range c.overlay {
		if !v.primary {
			delete(c.overlay, k)
		}
	}
	// clear up requirements
	c.reqs.Clear(ctx)

	for _, v := range c.overlay {
		if v.primary {
			log.DebugLog("msg", "Verifying", "key", v.key, "verificationVersion", c.verVer, "overlay", c.id)
			ret.Items = append(ret.Items, apiintf.OverlayItem{Key: apiintf.OverlayKey{Oper: string(v.oper), URI: v.URI}, Object: v.orig})
			svc := c.server.GetService(v.serviceName)
			if svc == nil {
				log.Errorf("[%v]could not find service %v", c.id, v.serviceName)
				ret.Failed = append(ret.Failed, apiintf.FailedVerification{Key: apiintf.OverlayKey{Oper: string(v.oper), URI: v.URI}, Errors: []error{errors.New("unknown service")}})
				continue
			}
			m := svc.GetMethod(v.methodName)
			if m == nil {
				log.Errorf("[%v]could not find method %v.%v", c.id, v.serviceName, v.methodName)
				ret.Failed = append(ret.Failed, apiintf.FailedVerification{Key: apiintf.OverlayKey{Oper: string(v.oper), URI: v.URI}, Errors: []error{errors.New("unknown method")}})
				continue
			}
			var obj interface{}
			if v.oper == operDelete {
				obj = v.orig.(runtime.ObjectMetaAccessor).GetObjectMeta()
			} else {
				obj = v.orig
			}
			if obj != nil && reflect.TypeOf(obj).Kind() == reflect.Ptr {
				obj = reflect.Indirect(reflect.ValueOf(v.orig)).Interface()
			}
			_, err := m.HandleInvocation(updCtx, obj)
			if err != nil {
				apiStatus := apierrors.FromError(err)
				ret.Failed = append(ret.Failed, apiintf.FailedVerification{Key: apiintf.OverlayKey{Oper: string(v.oper), URI: v.URI}, Errors: []error{&apiStatus}})
			}
		}
	}
	return ret, c.verVer
}

func (c *overlay) Verify(ctx context.Context) (apiintf.OverlayStatus, error) {
	defer c.Unlock()
	c.Lock()
	ret, _ := c.verify(ctx, c.server)
	return ret, nil
}

func (c *overlay) ClearBuffer(ctx context.Context, action []apiintf.OverlayKey) error {
	defer c.Unlock()
	c.Lock()
	uriSet := mapset.NewSet()
	if len(action) > 0 {
		for _, v := range action {
			uriSet.Add(v.URI)
		}
		for k, v := range c.overlay {
			if v.primary {
				c.primaryCount--
				if uriSet.Contains(v.URI) {
					c.deleteKvObj(ctx, k, v)
					delete(c.overlay, k)
				}
			}
		}
		return nil
	}
	for k, v := range c.overlay {
		c.deleteKvObj(ctx, k, v)
		delete(c.overlay, k)
	}
	c.comparators = nil
	c.preCommitComps = nil
	c.primaryCount = 0
	return nil
}

// commitDirect is used to commit a largeBuffer without retries.
func (c *overlay) commitDirect(ctx context.Context, retries, maxEntries int, verVer int64, otxn kvstore.Txn) (resp kvstore.TxnResponse, retry bool, pCount, sCount int, err error) {
	// The Staging buffer could have more number of objects that are supported in a trasaction.
	//  breakup the buffer into multiple trasaction if necessary. this API is only called if the cache
	//  is locked so it is safe to break it into multiple trasactions.
	kv := c.GetKvConn()
	if kv == nil {
		return kvstore.TxnResponse{}, false, pCount, sCount, errors.New("no backend KV store connection")
	}
	ctxn := &cacheTxn{
		Txn:    kv.NewTxn(),
		parent: c.CacheInterface.(*cache),
	}
	applyComps := func(ctxn *cacheTxn) {
		// Add all comparators
		for _, cp := range c.comparators {
			// Filter comparators before ading them
			//  - comparator for version > 0 and overlay is creating the object.
			if ovobj, ok := c.overlay[cp.Key]; ok {
				if ovobj.oper == operCreate && cp.Target == kvstore.Version && cp.Operator == ">" && cp.Version == int64(0) {
					continue
				}
			}
			log.Infof("adding comparator [%+v]", cp)
			ctxn.AddComparator(cp)
		}
		for _, cp := range c.preCommitComps {
			// Filter comparators before ading them
			//  - comparator for version > 0 and overlay is creating the object.
			if ovobj, ok := c.overlay[cp.Key]; ok {
				if ovobj.oper == operCreate && cp.Target == kvstore.Version && cp.Operator == ">" && cp.Version == int64(0) {
					continue
				}
			}
			log.Infof("adding comparator [%+v]", cp)
			ctxn.AddComparator(cp)
		}
	}

	commitTxn := func(ctxn *cacheTxn) (kvstore.TxnResponse, bool, error) {
		if ctxn.IsEmpty() {
			resp.Succeeded = true
		} else {
			resp, err = ctxn.Commit(ctx)
			if err != nil {
				if kvstore.IsVersionConflictError(err) && retry {
					log.Infof("[%v]Retrying failed transaction - retry %d (%s)", c.id, retries, err)
					return resp, true, errors.Wrap(err, "transaction commit failed")
				}
				if kvstore.IsTxnFailedError(err) && c.ephemeral {
					// This is a ephemeral overlay for a non-staged operation from user. Retry of txn failed.
					log.Infof("[%v]Retrying failed transaction - retry %d (%s)", c.id, retries, err)
					return resp, true, errors.Wrap(err, "transaction commit failed on ephemeral buffer")
				}
				log.Errorf("[%v]error on Txn.Commit (%s)", c.id, err)
				return resp, false, errors.Wrap(err, "Commit to backend failed")
			}
		}

		if !resp.Succeeded {
			log.Errorf("[%v]commit did not succeed", c.id)
			// If all errors are related to consistent update keys then we can retry this commit
			for _, r := range resp.Responses {
				ovobj, ok := c.overlay[r.Key]
				if r.Oper != kvstore.OperUpdate || !ok || ovobj.updateFn == nil {
					// Not recoverable
					return resp, false, errors.New("commit failed")
				}
			}
			log.Infof("[%v]Retrying failed transaction - retry %d (%s)", c.id, retries, err)
			return resp, true, errors.Wrap(err, "transaction commit did not succeed")
		}
		return resp, false, nil
	}
	numTxnEntries := 0
	otxn = c.NewTxn()
	kv = c.GetKvConn()
	if kv == nil {
		return kvstore.TxnResponse{}, false, pCount, sCount, errors.New("no backend KV store connection")
	}
	ctx = setDryRun(ctx, verVer)
	log.Infof("[%v]Comitting overlay (tenant:%s) with %d items", c.id, c.tenant, len(c.overlay))
	nctx := setPreCommitApply(ctx)
	otxn, _ = wrapOverlayTxn(nctx, otxn)
	// Clear all the pre-commit Apply comparators
	c.preCommitComps = nil
	errs := c.reqs.Apply(ctx, otxn, c.CacheInterface)
	if errs != nil {
		return kvstore.TxnResponse{}, false, pCount, sCount, fmt.Errorf("%v", errs)
	}
	retry = false

	for k, v := range c.overlay {
		// All objects that get added to the transaction should have the right verVer. Panic for now.
		if verVer != 0 && v.verVer != verVer {
			panic(fmt.Sprintf("found wrong version during commit[%s][%v][%s](%d/%d)", v.key, v.oper, v.URI, v.verVer, verVer))
		}
		numTxnEntries++
		if v.primary {
			pCount++
		} else {
			sCount++
		}
		log.Infof("[%v]Adding key [%v] oper [%v] to txn", c.id, k, v.oper)
		switch v.oper {
		case operCreate:
			ctxn.Create(k, v.val)
		case operUpdate:
			if v.touch {
				ctxn.Touch(k)
				continue
			}
			if v.updateFn != nil && v.resVer == "" {
				// txn is already updated, no action needed
				log.Infof("Consistent updated obj skipped [%v][%v][%+v]", v.resVer, v.key, v.val)
				retry = true
			}
			err = ctxn.Update(k, v.val)
			if err != nil {
				return kvstore.TxnResponse{}, false, pCount, sCount, errors.Wrap(err, "adding to txn failed")
			}
		case operDelete:
			err = ctxn.Delete(k)
			if err != nil {
				return kvstore.TxnResponse{}, false, pCount, sCount, errors.Wrap(err, "failed to add delete action to txn")
			}
			// operation could fail to delete because the underlying object changed between check and commmit. Retry
			retry = true
		}
		// Delete the persisted key
		if v.primary && !c.ephemeral {
			ctxn.Delete(c.makeBufferKey(k))
		}
		if numTxnEntries >= maxEntries {
			applyComps(ctxn)
			resp, retry, err := commitTxn(ctxn)
			if err != nil {
				return resp, retry, pCount, sCount, err
			}
			// Finalize the requirements
			errs := c.reqs.Finalize(ctx)
			if errs != nil && len(errs) > 0 {
				log.Errorf("[%v]Finalizing the references failed! (%v)", c.id, errs)
			}
			numTxnEntries = 0
			ctxn = &cacheTxn{
				Txn:    kv.NewTxn(),
				parent: c.CacheInterface.(*cache),
			}
		}
	}

	applyComps(ctxn)
	resp, retry, err = commitTxn(ctxn)
	if err != nil {
		return resp, retry, pCount, sCount, err
	}

	// Finalize the requirements
	errs = c.reqs.Finalize(ctx)
	if errs != nil && len(errs) > 0 {
		log.Errorf("[%v]Finalizing the references failed! (%v)", c.id, errs)
	}
	// Delete all the keys that were committed from the buffer
	for k := range c.overlay {
		delete(c.overlay, k)
	}

	return resp, false, pCount, sCount, nil
}

func (c *overlay) commit(ctx context.Context, verVer int64, otxn kvstore.Txn) (kvstore.TxnResponse, error) {
	// Range over all the items in the overlay and add to commit buffer
	var (
		err            error
		pCount, sCount int
	)
	var resp kvstore.TxnResponse
	retries, maxRetries, maxEntries := 0, 0, c.maxOvEntries*4
	lb := false
	// Large Buffer handling takes care of usage of buffer during config restore operations and such where we still want use
	//  a staging buffer to accumulate operations, but the number of operations may not fit into a KVStore trasaction.
	//  When large buffer is enabled, it is assumed that apiserver is locked and txn boundaries can be ignored safely.
	//  The staing buffer commit then splits the buffer into multiple operations
	lbi, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPISrvLargeBuffer)
	if ok {
		log.Infof("Large buffer is set")
		lb = lbi.(bool)
	}
	maxRetries = maxConsistentUpdateRetries
	if lb {
		maxRetries = 1
		maxEntries = 1024 * c.maxOvEntries
	}
	retry := false

	for retries < maxRetries {
		resp, retry, pCount, sCount, err = c.commitDirect(ctx, retries, maxEntries, verVer, otxn)
		retries++
		if err != nil {
			if retry {
				if retries > 0 {
					// give it a random sleep between 0 - 10ms
					jitter := rand.Intn(500000)
					time.Sleep(time.Duration(int(time.Microsecond) * jitter))
				}
				log.Infof("[%v] commit failed, retrying, retry %d", c.id, retries)
				continue
			}
			return resp, err
		}
		break
	}

	if retries == maxConsistentUpdateRetries {
		// failed after max retries
		log.Errorf("[%v/%v] failed commit after max retries", c.tenant, c.id)
		return resp, errors.New("commit failed")
	}

	c.reqs = requirement.NewRequirementSet(c.server.GetGraphDB(), c, c.server)
	c.primaryCount = 0
	log.Infof("Completing commit for buffer [%s/%s] with %d primary and %d secondary objects %d objects left in overlay", c.tenant, c.id, pCount, sCount, len(c.overlay))
	return resp, nil
}

func (c *overlay) Commit(ctx context.Context, action []apiintf.OverlayKey) error {
	txn := c.NewTxn()
	defer c.Unlock()
	c.Lock()
	c.reqs = requirement.NewRequirementSet(c.server.GetGraphDB(), c, c.server)
	ret, verVer := c.verify(ctx, c.server)
	log.Infof("Calling Commit on overlay [%v/%v][%p][%p]", c.tenant, c.id, c, c.reqs)
	if len(ret.Failed) != 0 {
		retErr := &api.Status{}
		retErr.Code = int32(codes.FailedPrecondition)
		retErr.Result.Str = "Buffer verification failed"
		for _, e := range ret.Failed {
			retErr.Message = append(retErr.Message, fmt.Sprintf("Key: %v Error: %v", e.Key, e.Errors))
		}
		return retErr
	}
	// If this is for a subset of items build a commit buffer with subset of overlay and
	//  related objects.  XXX-TBD(sanjayt): Not supported in this PR.
	if len(action) > 0 {
		return errors.New("Commit of subset not supported yet")
	}
	kv := c.GetKvConn()
	if kv == nil {
		return errors.New("no backend KV store connection")
	}
	_, err := c.commit(ctx, verVer, txn)
	return err
}

func (c *overlay) touch(ctx context.Context, key string, verVer int64) error {
	var cacheObj runtime.Object
	ovObj, err := c.findObjects(context.TODO(), key, cacheObj)
	if ovObj != nil {
		if ovObj.oper == operDelete {
			return fmt.Errorf("[%v]operation would conflict [delete+touch] for [%v]", c.id, key)
		}
		// if there is already an create or update operation in the overlay - nothing to do.
		return nil
	}
	if err != nil {
		return fmt.Errorf("[%v]touch operation on a non-existent key [%v]", c.id, key)
	}
	ovObj = &overlayObj{
		oper:   operUpdate,
		key:    key,
		touch:  true,
		verVer: verVer,
	}
	c.overlay[key] = ovObj
	return nil
}

// Stat returns information about the objects in list of keys
func (c *overlay) Stat(ctx context.Context, keys []string) []apiintf.ObjectStat {
	if getDryRun(ctx) == nil {
		defer c.Unlock()
		c.Lock()
	}
	// Get stat from cache
	ret := c.CacheInterface.Stat(ctx, keys)
	// Ammend as per overlay
	for i := range ret {
		ovObj := c.overlay[ret[i].Key]
		if ovObj != nil {
			if ret[i].Valid && ovObj.oper == operDelete {
				ret[i].Valid = false
				ret[i].InOverlay = true
			} else if !ret[i].Valid && ovObj.oper == operCreate || ovObj.oper == operUpdate {
				ret[i].Valid = true
				ret[i].InOverlay = true
			}
		}
	}
	return ret
}

// StatKind returns stat for all object of kind. Not implemented on the overlay yet.
func (c *overlay) StatKind(group string, kind string) ([]apiintf.ObjectStat, error) {
	return nil, fmt.Errorf("unimplemented")
}

// GetRequirements returns the RequirementSet used by the Overlay
func (c *overlay) GetRequirements() apiintf.RequirementSet {
	return c.reqs
}

// Txn creates a Txn on the overlay.
func (c *overlay) NewTxn() kvstore.Txn {
	return &overlayTxn{ov: c, context: context.TODO()}
}

// NewWrappedTxn creates a Txn on the overlay wrapped in a context
func (c *overlay) NewWrappedTxn(ctx context.Context) kvstore.Txn {
	return &overlayTxn{ov: c, context: ctx}
}

type overlayTxn struct {
	ov      *overlay
	used    bool
	context context.Context
}

// Create stages an object creation in a transaction.
func (t *overlayTxn) Create(key string, obj runtime.Object) error {
	var verVer int64
	if dm := getDryRun(t.context); dm == nil {
		defer t.ov.Unlock()
		t.ov.Lock()
	} else {
		verVer = dm.verVer
	}

	t.ov.create(t.context, "", "", "", key, nil, obj, verVer, false)
	t.used = true
	return nil
}

// Delete stages an object deletion in a transaction.
func (t *overlayTxn) Delete(key string, cs ...kvstore.Cmp) error {
	var verVer int64
	if dm := getDryRun(t.context); dm == nil {
		defer t.ov.Unlock()
		t.ov.Lock()
	} else {
		verVer = dm.verVer
	}
	t.ov.delete(t.context, "", "", "", key, nil, nil, verVer, false)
	t.ov.comparators = append(t.ov.comparators, cs...)
	t.used = true
	return nil
}

// Update stages an object update in a transaction.
func (t *overlayTxn) Update(key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	var verVer int64
	if dm := getDryRun(t.context); dm == nil {
		defer t.ov.Unlock()
		t.ov.Lock()
	} else {
		verVer = dm.verVer
	}
	// XXX-TBD(sanjayt): How do we get to know if this is a dry run??
	err := t.ov.update(t.context, "", "", "", key, nil, obj, nil, "", verVer, false)
	if err != nil {
		return err
	}
	t.ov.comparators = append(t.ov.comparators, cs...)
	t.used = true
	return nil
}

// Touch updates the version without changing the object. Not implemented on the overlay.
func (t *overlayTxn) Touch(key string) error {
	var verVer int64
	if dm := getDryRun(t.context); dm == nil {
		defer t.ov.Unlock()
		t.ov.Lock()
	} else {
		verVer = dm.verVer
	}
	t.used = true
	return t.ov.touch(t.context, key, verVer)
}

// Commit tries to commit the transaction.
func (t *overlayTxn) Commit(ctx context.Context) (kvstore.TxnResponse, error) {
	// perform a commit on the overlay
	log.Infof("CommitTxn called for the overlay [%v/%v]", t.ov.tenant, t.ov.id)
	kv := t.ov.GetKvConn()
	if kv == nil {
		return kvstore.TxnResponse{}, errors.New("no backend KV store connection")
	}
	t.context = setPreCommitApply(ctx)
	defer t.ov.Unlock()
	t.ov.Lock()
	ret, err := t.ov.commit(ctx, 0, t)
	return ret, err
}

// IsEmpty returns true if the Txn is empty
func (t *overlayTxn) IsEmpty() bool {
	defer t.ov.Unlock()
	t.ov.Lock()
	return !t.used
}

// AddComparator adds a condition for the transaction.
func (t *overlayTxn) AddComparator(cs ...kvstore.Cmp) {
	if getDryRun(t.context) == nil {
		defer t.ov.Unlock()
		t.ov.Lock()
	}
	if isPreCommitApply(t.context) {
		t.ov.preCommitComps = append(t.ov.preCommitComps, cs...)
	} else {
		t.ov.comparators = append(t.ov.comparators, cs...)
	}
}

// wrapOverlayTxn wraps the overlay in a context
func wrapOverlayTxn(ctx context.Context, txn kvstore.Txn) (kvstore.Txn, error) {
	otxn, ok := txn.(*overlayTxn)
	if !ok {
		return nil, errors.New("not a overlay txn")
	}
	otxn.context = ctx
	return otxn, nil
}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddKnownTypes(&overlaypb.BufferItem{}, &overlaypb.BufferItemList{})
}
