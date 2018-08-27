package cache

import (
	"context"
	"fmt"
	"reflect"
	"strings"
	"sync"

	mapset "github.com/deckarep/golang-set"
	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/types"
	"github.com/pkg/errors"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache/ovpb"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/helper"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

type dryRunMarker struct {
	verVer int64
}

// setStagingBufferInGrpcMD sets the GRPC metadata with buffer ID
func setStagingBufferInGrpcMD(ctx context.Context, id string) context.Context {
	pair := metadata.Pairs(apiserver.RequestParamStagingBufferID, id)
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

type overlayObj struct {
	revision    int64
	verVer      int64
	oper        apiOper
	primary     bool
	serviceName string
	methodName  string
	key         string
	URI         string
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
}

type overlay struct {
	sync.Mutex
	// Exposes all methods of the cache with some frills
	apiintf.CacheInterface
	server   apiserver.Server
	revision int64
	// verVer is verification version
	verVer      int64
	tenant      string
	id          string
	baseKey     string
	overlay     map[string]*overlayObj
	comparators []kvstore.Cmp
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
func NewOverlay(tenant, id, baseKey string, c apiintf.CacheInterface, apisrv apiserver.Server) (apiintf.OverlayInterface, error) {
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
		revision:       1,
	}
	log.Infof("creating new overlay %s, base path %s", id, o.baseKey)
	overlaysSingleton.ovMap[name] = o
	return o, nil
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

func restoreOverlays(c apiintf.CacheInterface) {
	defer overlaysSingleton.Unlock()
	overlaysSingleton.Lock()

	// XXX-TODO(sanjayt): need to handle issue where we are recovering from a prefix watcher failure
	//   and the overlay has been deleted behind our back. Very unlikely case when there is only one
	//   API server in the cluster.
	for _, v := range overlaysSingleton.ovMap {
		ov := v.(*overlay)
		ov.Lock()
		// It is okay to release all locks on exit, no operations are possible on the staging buffer till
		// restore is done, So big hammer and nuke everything and restore instead of a mark and sweep.
		defer ov.Unlock()
		ov.overlay = make(map[string]*overlayObj)
	}

	// restore from the kvstore backend for each overlay.
	// XXX-TBD(sanjayt): Restore the overlay path from persisted staging object.
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
	return fmt.Sprintf("%s/tenant/%v/buffer/%v/%s", c.baseKey, c.tenant, c.id, strings.TrimPrefix(key, "/"))
}

func (c *overlay) parseKey(key string) (string, string, error) {
	s := strings.TrimPrefix(key, c.baseKey)
	p := strings.SplitN(s, "/", 6)
	if len(p) < 6 {
		return "", "", errors.New("could not parse")
	}
	return p[2], p[4], nil
}

func (c *overlay) createKvObj(ctx context.Context, uri, key string, obj *overlayObj) error {
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
		TypeMeta: api.TypeMeta{Kind: "BufferItem"},
		ItemId: &overlaypb.BufferItem_Id{
			Kind:   kind,
			URI:    uri,
			Method: string(obj.oper),
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
		log.Errorf("Create overlay Buffer Item failed (%s)\n", err)
		return err
	}
	return nil
}

func (c *overlay) updateKvObj(ctx context.Context, key string, obj *overlayObj) error {
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
		TypeMeta: api.TypeMeta{Kind: "BufferItem"},
		ItemId: &overlaypb.BufferItem_Id{
			Kind:   kind,
			URI:    key,
			Method: string(operCreate),
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
		log.Errorf("Update overlay Buffer Item failed (%s)\n", err)
		return err
	}
	return nil
}

func (c *overlay) deleteKvObj(ctx context.Context, key string, obj *overlayObj) error {
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
		log.Errorf("Delete overlay Buffer Item failed (%s)\n", err)
		return err
	}
	return nil
}

// cleanupKvObjs cleans up all persisted data for the overlay
func (c *overlay) cleanupKvObjs(ctx context.Context) error {
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
		return fmt.Errorf("already exists")
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
			log.ErrorLog("msg", "did not find overlay object in dryRun", "dryRun", dryRun, "key", key)
			return errors.New("object not found")
		}
		ovObj.verVer = dryRun
		ovObj.val = obj
		return nil
	}

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
	}

	if err := c.createKvObj(ctx, uri, key, ovObj); err != nil {
		log.Errorf("failed to persist for key %v(%s)", key, err)
	}
	log.DebugLog("msg", "successful creating object", "key", key)
	// Overwrite any existing overlay object with a create Oper.
	c.overlay[key] = ovObj
	return nil
}

// Create creates a object in overlay cache. If the object already exists in the
//  API Cache the create fails, else the object is added to the overlay cache. No
//  notifications are generated till the commit of the buffer.
func (c *overlay) Create(ctx context.Context, key string, obj runtime.Object) error {
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
func (c *overlay) update(ctx context.Context, service, method, uri, key string, orig, obj runtime.Object, updateFn kvstore.UpdateFunc, dryRun int64, primary bool) error {
	if obj == nil {
		return errors.New("invalid parameters")
	}

	var cacheObj runtime.Object
	ovObj, err := c.findObjects(ctx, key, cacheObj)

	if dryRun == 0 {
		// Not in cache, not in overlay, update not allowed
		// findObjects only returns primary Overlay objects, But in this case it is
		// valid to fail in non-dryrun cases, because
		//  - 2 secondary ops on the same object is not allowed.
		//  - there is no cache object so update is invalid.
		if err != nil && ovObj == nil {
			return errors.Wrap(err, "cannot update without cache object")
		}
	}

	var inOv bool
	if ovObj != nil {
		inOv = true
	}
	ovObj = c.overlay[key]

	if ovObj != nil && !primary && dryRun > 0 && ovObj.verVer == dryRun {
		// more than one secondary operation only allowed during dryRun.
		return errors.New("operation would conflict")
	}

	// If dry run and primary, the object should exist in the overlay
	if ovObj == nil && primary && dryRun > 0 {
		return errors.New("object not found")
	}

	if ovObj != nil {
		if ovObj.primary && !primary {
			// Secondary update on a primary would conflict
			return errors.New("operation would conflict")
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
		}
	}

	if err != nil {
		// Not in Cache, in overlay
		oper = operCreate
		ovObj.val = obj
	} else {
		// In cache and in overlay
		ovObj.oper = operUpdate
		ovObj.val = obj
	}
	ovObj.oper = oper
	ovObj.primary = primary
	ovObj.updateFn = updateFn
	ovObj.val = cacheObj
	ovObj.URI = uri
	ovObj.orig = orig
	ovObj.serviceName = service
	ovObj.methodName = method
	ovObj.verVer = dryRun

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
	var verVer int64
	dm := getDryRun(ctx)
	if dm == nil {
		defer c.Unlock()
		c.Lock()
	} else {
		verVer = dm.verVer
	}
	// Cmps are not used in API path, so ignore.
	return c.update(ctx, "", "", "", key, nil, obj, nil, verVer, false)
}

// delete an object from the cache - rules
//  - primary supercedes secondary objects.
//  - secondary delete if there is an existing primary object is not allowed.
//  - multiple secondary operations on the same object not allowed, like in update case this is caught during dryRun
//  - if there is no object in the cache, delete removes the object from overlay
func (c *overlay) delete(ctx context.Context, service, method, uri, key string, orig, into runtime.Object, dryRun int64, primary bool) error {
	ovObj, err := c.findObjects(ctx, key, into)
	// Not in Overlay, not in Cache
	if ovObj == nil && err != nil {
		into = nil
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
	} else {
		panic("not in overlay , in cache, but into is nil")
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
	}
	if err := c.createKvObj(ctx, "", key, ovObj); err != nil {
		log.Errorf("Failed to update buffer item fromm kv store [%s](%s)", key, err)
		return err
	}
	c.overlay[key] = ovObj
	return nil
}

// Delete stages a delete in the overlay Cache. The object should exist in the overlay or the API cache.
func (c *overlay) Delete(ctx context.Context, key string, into runtime.Object, cs ...kvstore.Cmp) error {
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
	return c.update(ctx, "", "", "", key, nil, into, updateFunc, verVer, false)
}

// Get retrieves object from the cache if it exists.
func (c *overlay) Get(ctx context.Context, key string, into runtime.Object) error {
	defer c.Unlock()
	c.Lock()
	ovObj := c.overlay[key]
	if ovObj != nil {
		if ovObj.oper == operDelete {
			return errors.New("not found")
		}
		ovObj.val.Clone(into)
		return nil
	}

	return c.CacheInterface.Get(ctx, key, into)
}

func (c *overlay) list(ctx context.Context, prefix string, opts api.ListWatchOptions) []*overlayObj {
	ret := []*overlayObj{}
	filters, err := getFilters(opts)
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
	defer c.Unlock()
	c.Lock()
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

	if service == "" || method == "" {
		panic("primary with no service or method")
	}
	return c.create(ctx, service, method, uri, key, orig, obj, verVer, true)
}
func (c *overlay) UpdatePrimary(ctx context.Context, service, method, uri, key string, orig, obj runtime.Object, updateFn kvstore.UpdateFunc) error {
	var verVer int64
	dm := getDryRun(ctx)
	if dm == nil {
		defer c.Unlock()
		c.Lock()
	} else {
		verVer = dm.verVer
	}
	return c.update(ctx, service, method, uri, key, orig, obj, updateFn, verVer, true)
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
	for _, v := range c.overlay {
		if v.primary {
			log.DebugLog("msg", "Verifying", "key", v.key, "verificationVersion", c.verVer)
			ret.Items = append(ret.Items, apiintf.OverlayItem{Key: apiintf.OverlayKey{Oper: string(v.oper), URI: v.URI}, Object: v.orig})
			svc := c.server.GetService(v.serviceName)
			if svc == nil {
				log.Errorf("could not find service %v", v.serviceName)
				ret.Failed = append(ret.Failed, apiintf.FailedVerification{Key: apiintf.OverlayKey{Oper: string(v.oper), URI: v.URI}, Errors: []error{errors.New("unknown service")}})
				continue
			}
			m := svc.GetMethod(v.methodName)
			if m == nil {
				log.Errorf("could not find method %v.%v", v.serviceName, v.methodName)
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
				ret.Failed = append(ret.Failed, apiintf.FailedVerification{Key: apiintf.OverlayKey{Oper: string(v.oper), URI: v.URI}, Errors: []error{err}})
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
			if uriSet.Contains(v.URI) {
				c.deleteKvObj(ctx, k, v)
				delete(c.overlay, k)
			}
		}
		return nil
	}
	for k, v := range c.overlay {
		c.deleteKvObj(ctx, k, v)
		delete(c.overlay, k)
	}
	return nil
}

func (c *overlay) Commit(ctx context.Context, action []apiintf.OverlayKey) error {
	defer c.Unlock()
	c.Lock()
	ret, verVer := c.verify(ctx, c.server)

	if len(ret.Failed) != 0 {
		return errors.New("Buffer verification failed")
	}
	// If this is for a subset of items build a commit buffer with subset of overlay and
	//  related objects.  XXX-TBD(sanjayt): Not supported in this PR.
	if len(action) > 0 {
		return errors.New("Commit of subset not supported yet")
	}
	// Range over all the items in the overlay and add to commit buffer
	kv := c.GetKvConn()
	if kv == nil {
		return errors.New("no backend KV store connection")
	}
	ctxn := &cacheTxn{
		Txn:    kv.NewTxn(),
		parent: c.CacheInterface.(*cache),
	}
	if kv == nil {
		return errors.New("failed to create a new transaction")
	}
	var (
		err            error
		pCount, sCount int
	)

	log.Infof("Comitting overlay (%s/%s) with %d items", c.tenant, c.id, len(c.overlay))

	for k, v := range c.overlay {
		// All objects that get added to the transaction should have the right verVer. Panic for now.
		if v.verVer != verVer {
			panic(fmt.Sprintf("found wrong version during commit[%s][%s](%d/%d)", v.key, v.URI, v.verVer, verVer))
		}
		if v.primary {
			pCount++
		} else {
			sCount++
		}
		switch v.oper {
		case operCreate:
			ctxn.Create(k, v.val)
		case operUpdate:
			ctxn.Update(k, v.val)
			if v.updateFn != nil {
				newObj, err := runtime.NewEmpty(v.val)
				if err != nil {
					return errors.New("could not create new empty object")
				}
				err = kv.Get(ctx, k, newObj)
				if err != nil {
					return errors.New("Consistent update needed on object not in backend")
				}
				meta, _ := mustGetObjectMetaVersion(newObj)
				o, err := v.updateFn(newObj)
				if err != nil {
					return errors.Wrap(err, "consistent update failed")
				}
				err = ctxn.Update(k, o)
				ctxn.AddComparator(kvstore.Compare(kvstore.WithVersion(k), "=", meta.ResourceVersion))
			} else {
				err = ctxn.Update(k, v.val)
			}
			if err != nil {
				return errors.Wrap(err, "adding to txn failed")
			}
		case operDelete:
			err = ctxn.Delete(k)
			if err != nil {
				return errors.Wrap(err, "failed to add delete action to txn")
			}
		}
		// Delete the persisted key
		if v.primary {
			ctxn.Delete(c.makeBufferKey(k))
		}
	}
	// Add all comparators
	if len(c.comparators) > 0 {
		ctxn.AddComparator(c.comparators...)
	}
	resp, err := ctxn.commit(ctx)
	if err != nil {
		log.Errorf("error on Txn.Commit (%s)", err)
		return errors.Wrap(err, "Commit to backend failed")
	}
	if !resp.Succeeded {
		log.Errorf("commit did not succeed")
		// XXX-TODO(sanjayt): Check if the error was due to any consistent update keys. If so retry
		return errors.New("commit failed")
	}

	// Delete all the keys that were committed from the buffer
	for k := range c.overlay {
		delete(c.overlay, k)
	}
	log.Infof("Completing commit for buffer [%s/%s] with %d primary and %d secondary objects %d objects left in overlay", c.tenant, c.id, pCount, sCount, len(c.overlay))
	return nil
}

// Txn creates a Txn on the overlay.
func (c *overlay) NewTxn() kvstore.Txn {
	return &overlayTxn{ov: c}
}

type overlayTxn struct {
	ov   *overlay
	used bool
}

// Create stages an object creation in a transaction.
func (t *overlayTxn) Create(key string, obj runtime.Object) error {
	defer t.ov.Unlock()
	t.ov.Lock()

	ctx := context.TODO()
	t.ov.create(ctx, "", "", "", key, nil, obj, 0, false)
	t.used = true
	return nil
}

// Delete stages an object deletion in a transaction.
func (t *overlayTxn) Delete(key string, cs ...kvstore.Cmp) error {
	defer t.ov.Unlock()
	t.ov.Lock()

	ctx := context.TODO()
	t.ov.delete(ctx, "", "", "", key, nil, nil, 0, false)
	t.ov.comparators = append(t.ov.comparators, cs...)
	t.used = true
	return nil
}

// Update stages an object update in a transaction.
func (t *overlayTxn) Update(key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	defer t.ov.Unlock()
	t.ov.Lock()

	ctx := context.TODO()
	// XXX-TBD(sanjayt): How do we get to know if this is a dry run??
	t.ov.update(ctx, "", "", "", key, nil, obj, nil, 0, false)
	t.ov.comparators = append(t.ov.comparators, cs...)
	return nil
}

// Commit tries to commit the transaction.
func (t *overlayTxn) Commit(ctx context.Context) (kvstore.TxnResponse, error) {
	return kvstore.TxnResponse{}, errors.New("commit not allowed directly on the overlay")
}

// IsEmpty returns true if the Txn is empty
func (t *overlayTxn) IsEmpty() bool {
	defer t.ov.Unlock()
	t.ov.Lock()
	return t.used
}

// AddComparator adds a condition for the transaction.
func (t *overlayTxn) AddComparator(cs ...kvstore.Cmp) {
	defer t.ov.Unlock()
	t.ov.Lock()
	t.ov.comparators = append(t.ov.comparators, cs...)
}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddKnownTypes(&overlaypb.BufferItem{}, &overlaypb.BufferItemList{})
}
