package cache

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"strings"
	"sync"

	mapset "github.com/deckarep/golang-set"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/helper"
	"github.com/pensando/sw/venice/utils/runtime"
)

type overlayObj struct {
	oper        apiOper
	key         string
	val         runtime.Object
	comparators []kvstore.Cmp
	updateFn    kvstore.UpdateFunc
}

type overlay struct {
	sync.Mutex
	// Exposes all methods of the cache with some frills
	Interface
	overlay map[string]*overlayObj
}

type overlayMap struct {
	sync.Mutex
	ovMap map[string]*overlay
}

var (
	overlaysSingleton overlayMap
	once              sync.Once
)

func initOverlayMap() {
	overlaysSingleton.ovMap = make(map[string]*overlay)
}

// GetOverlay retrieves an existing overlay with id
func GetOverlay(id string) (Interface, error) {
	defer overlaysSingleton.Unlock()
	overlaysSingleton.Lock()
	once.Do(initOverlayMap)
	if o, ok := overlaysSingleton.ovMap[id]; ok {
		return o, nil
	}
	return nil, errors.New("not found")
}

// DelOverlay deletes an existing overlay with id
func DelOverlay(id string) error {
	overlaysSingleton.Lock()
	once.Do(initOverlayMap)
	o, ok := overlaysSingleton.ovMap[id]
	if !ok {
		overlaysSingleton.Unlock()
		return errors.New("not found")
	}
	delete(overlaysSingleton.ovMap, id)
	overlaysSingleton.Unlock()
	defer o.Unlock()
	o.Lock()
	for k := range o.overlay {
		delete(o.overlay, k)
	}
	return nil
}

// NewOverlay creates a new overlay over the passed cache.Interface
func NewOverlay(id string, c Interface) (Interface, error) {
	defer overlaysSingleton.Unlock()
	overlaysSingleton.Lock()
	once.Do(initOverlayMap)
	if _, ok := overlaysSingleton.ovMap[id]; ok {
		return nil, errors.New("already exists")
	}
	o := &overlay{
		Interface: c,
		overlay:   make(map[string]*overlayObj),
	}
	overlaysSingleton.ovMap[id] = o
	return o, nil
}

func (c *overlay) findObjects(ctx context.Context, key string, into runtime.Object) (*overlayObj, error) {
	err := c.Interface.Get(ctx, key, into)
	if err != nil {
		into = nil
	}
	ovObj := c.overlay[key]
	return ovObj, err
}

// Create creates a object in overlay cache. If the object already exists in the
//  API Cache the create fails, else the object is added to the overlay cache. No
//  notifications are generated till the commit of the buffer.
func (c *overlay) Create(ctx context.Context, key string, obj runtime.Object) error {
	defer c.Unlock()
	c.Lock()
	if obj == nil {
		return errors.New("invalid parameters")
	}
	_, err := c.findObjects(ctx, key, nil)
	if err == nil {
		return fmt.Errorf("already exists")
	}
	// Overwrite any exising overlay object with a create Oper.
	c.overlay[key] = &overlayObj{
		oper: operCreate,
		key:  key,
		val:  obj,
	}
	return nil
}
func (c *overlay) update(ctx context.Context, key string, obj runtime.Object, updateFn kvstore.UpdateFunc) error {
	// First check if the object exists
	if obj == nil {
		return errors.New("invalid parameters")
	}
	ovObj := c.overlay[key]
	var cacheObj runtime.Object
	ovObj, err := c.findObjects(ctx, key, cacheObj)
	// Not in cache, not in overlay
	if err != nil && ovObj == nil {
		return err
	}

	o, _ := obj.Clone(nil)
	cacheObj = o.(runtime.Object)
	// In cache, not in overlay
	if err == nil && ovObj == nil {
		ovObj = &overlayObj{
			key:  key,
			oper: operUpdate,
			val:  cacheObj,
		}
	}
	// Not in Cache, in overlay
	if err != nil && ovObj != nil {
		ovObj.oper = operCreate

	}

	// In cache and in overlay
	if err == nil && ovObj != nil {
		ovObj.oper = operUpdate
	}
	ovObj.updateFn = updateFn
	ovObj.val = cacheObj
	c.overlay[key] = ovObj
	return nil
}

// Update stages update of an object in overlay cache. The object should exist either in the API cache or in the
//   overlay for an update to succeed. If neither exist then the operation fails.  If an operations is
//   already in the overlay the then following table defines the resultant operation
//   Existing entry in Overlay    -> State of Overlay after Update operation
//   Create                       =>  Create[New contents]
//   Update                       =>  Update[New Contents]
//   Delete                       =>  Update[New Contents] IF API cache has object or fail
func (c *overlay) Update(ctx context.Context, key string, obj runtime.Object, cs ...kvstore.Cmp) error {
	defer c.Unlock()
	c.Lock()
	// Cmps are not used in API path, so ignore.
	return c.update(ctx, key, obj, nil)
}

// Delete stages a delete in the overlay Cache. The object should exist in the overlay or the API cache.
func (c *overlay) Delete(ctx context.Context, key string, into runtime.Object, cs ...kvstore.Cmp) error {
	defer c.Unlock()
	c.Lock()

	ovObj, err := c.findObjects(ctx, key, into)
	// Not in Overlay, not in Cache
	if ovObj == nil && err != nil {
		into = nil
		return fmt.Errorf("does not exist")
	}
	// in overlay, not in cache
	if ovObj != nil && err != nil {
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
		key:  key,
		oper: operDelete,
		val:  cacheObj,
	}
	c.overlay[key] = ovObj
	return nil
}

// ConsistentUpdate updates an object in the backend KVStore if it already exists. If the operation is successful
//  the cache is updated and watch notifications are generated for established watches.
func (c *overlay) ConsistentUpdate(ctx context.Context, key string, into runtime.Object, updateFunc kvstore.UpdateFunc) error {
	defer c.Unlock()
	c.Lock()
	return c.update(ctx, key, into, updateFunc)
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

	return c.Interface.Get(ctx, key, into)
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
	c.Interface.ListFiltered(ctx, prefix, cacheObjs, opts)
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
