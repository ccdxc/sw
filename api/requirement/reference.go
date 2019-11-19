package requirement

import (
	"context"
	"fmt"
	"reflect"

	"github.com/davecgh/go-spew/spew"

	"github.com/pensando/sw/api/graph"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

type referenceReq struct {
	store graph.Interface
	cache apiintf.CacheInterface
	oper  apiintf.APIOperType
	key   string
	// reference objects keyed by the field that is causing the reference
	reqs map[string]apiintf.ReferenceObj
	// Flattenned list of reqferences derived from reqs map. Computed for efficiency.
	flatReqs []string
	// Flattenned list of weak references derived from reqs map. Computed for efficiency.
	weakReqs []string
}

// NewReferenceReq creates a new reference requirement
func NewReferenceReq(oper apiintf.APIOperType, key string, reqs map[string]apiintf.ReferenceObj, g graph.Interface, kvs apiintf.CacheInterface) apiintf.Requirement {
	ret := referenceReq{
		store: g,
		cache: kvs,
		oper:  oper,
		key:   key,
		reqs:  reqs,
	}
	flatReqs := make(map[string]bool)
	weakReqs := make(map[string]bool)
	for _, v := range reqs {
		switch v.RefType {
		case apiintf.NamedReference:
			for i := range v.Refs {
				flatReqs[v.Refs[i]] = true
			}
		case apiintf.WeakReference:
			for i := range v.Refs {
				weakReqs[v.Refs[i]] = true
			}
		}
	}
	for k := range flatReqs {
		ret.flatReqs = append(ret.flatReqs, k)
	}
	for k := range weakReqs {
		ret.weakReqs = append(ret.weakReqs, k)
	}
	return &ret
}

func (r *referenceReq) getReferersFromOverlay(ctx context.Context, key string) (count int, objs string) {
	refs := r.store.Referrers(key)
	var flat []string
	for _, v := range refs.Refs {
		flat = append(flat, v...)
	}
	stat := r.cache.Stat(ctx, flat)
	objs = ""
	count = 0
	for i, v := range stat {
		if v.Valid {
			if !v.InOverlay {
				if count < 5 {
					objs = objs + "[" + v.Key + "]"
				}
				count++
			} else {
				key1 := flat[i]
				sch := runtime.GetDefaultScheme()
				into, err := sch.New(v.TypeMeta.Kind)
				if err != nil {
					panic(fmt.Sprintf("could not get new obj for [%+v]", v.TypeMeta))
				}
				err = r.cache.Get(ctx, key1, into)
				if err != nil {
					panic(fmt.Sprintf("could not get obj for [%+v](%s)", v.TypeMeta, err))
				}
				m := reflect.ValueOf(into).MethodByName("References")
				if m.IsValid() {
					objm, err := runtime.GetObjectMeta(into)
					if err != nil {
						panic(fmt.Sprintf("could not get objectmeta. Unexpected (%s)", err))
					}
					refs1 := make(map[string]apiintf.ReferenceObj)
					args := []reflect.Value{reflect.ValueOf(objm.Tenant), reflect.ValueOf(""), reflect.ValueOf(refs1)}
					m.Call(args)
					for _, v1 := range refs1 {
						if v1.RefType == apiintf.NamedReference {
							for _, v2 := range v1.Refs {
								if v2 == key {
									if count < 5 {
										objs = objs + "[" + v.Key + "]"
									}
									count++
								}
							}
						}
					}
				}
			}
		}
	}
	return
}

// Check is implementation of the Requirement interface.
func (r *referenceReq) Check(ctx context.Context) []error {
	var ret []error
	// Get requirements
	switch r.oper {
	case apiintf.CreateOper, apiintf.UpdateOper:
		if len(r.flatReqs) == 0 {
			return ret
		}
		stat := r.cache.Stat(ctx, r.flatReqs)
		if stat == nil || len(stat) != len(r.flatReqs) {
			ret = append(ret, fmt.Errorf("number of object stat did not match %d/%d", len(stat), len(r.flatReqs)))
		}
		for i := range stat {
			if !stat[i].Valid {
				ret = append(ret, fmt.Errorf("required object not found [%s]", stat[i].Key))
			}
		}
	case apiintf.DeleteOper:
		if !r.store.IsIsolated(r.key) {
			count, objs := r.getReferersFromOverlay(ctx, r.key)
			if count > 0 {
				ret = append(ret, fmt.Errorf("Object [%s] has references from other objects[%s]", r.key, objs))
			}
		}
	}
	return ret
}

// Apply is implementation of the Requirement interface.
func (r *referenceReq) Apply(ctx context.Context, txn kvstore.Txn, cache apiintf.CacheInterface) error {
	switch r.oper {
	case apiintf.CreateOper, apiintf.UpdateOper:
		for i := range r.flatReqs {
			txn.AddComparator(kvstore.Compare(kvstore.WithVersion(r.flatReqs[i]), ">", "0"))
			txn.Touch(r.flatReqs[i])
		}
	case apiintf.DeleteOper:
		if !r.store.IsIsolated(r.key) {
			// it is possible that the referrers are being deleted in the overlay. Check that the object will be
			//  isolated once the overlay is committed
			count, objs := r.getReferersFromOverlay(ctx, r.key)
			if count > 0 {
				return fmt.Errorf("Object [%v] has references from other objects[ %s]", r.key, objs)
			}
		}
		// XXX-TBD(sanjayt): there is potentially a timing issue: between checking the graph DB and cache stat a reference might have been added.
		//   encapsulate the IsIsolated check in the cache interface so it can be done atomically?
		stat := cache.Stat(ctx, []string{r.key})
		if stat == nil || stat[0].Valid == false {
			log.Errorf("got stat as [%+v]", stat)
			return fmt.Errorf("object not found during prep [%s]", r.key)
		}
		txn.AddComparator(kvstore.Compare(kvstore.WithVersion(r.key), "=", fmt.Sprintf("%d", stat[0].Revision)))
	}
	return nil
}

// Finalize is implementation of the Requirement interface.
func (r *referenceReq) Finalize(ctx context.Context) error {
	log.Infof("call requirement Finalize for oper [%v] key [%v] reqs [%v]", r.oper, r.key, r.reqs)
	switch r.oper {
	case apiintf.CreateOper, apiintf.UpdateOper:
		node := graph.Node{
			Refs:         make(map[string][]string),
			WeakRefs:     make(map[string][]string),
			SelectorRefs: make(map[string][]string),
		}
		node.This = r.key
		node.Dir = graph.RefOut
		for k, v := range r.reqs {
			if v.RefType == apiintf.WeakReference {
				node.WeakRefs[k] = append(node.WeakRefs[k], v.Refs...)
			}
			if v.RefType == apiintf.NamedReference {
				node.Refs[k] = append(node.Refs[k], v.Refs...)
			}
			if v.RefType == apiintf.SelectorReference {
				node.SelectorRefs[k] = append(node.SelectorRefs[k], v.Refs...)
			}
		}
		return r.store.UpdateNode(&node)
	case apiintf.DeleteOper:
		err := r.store.DeleteNode(r.key)
		// Errors are expected during delete when node does not exist.
		log.DebugLog("msg", "Finalize failed", "error", err)
		return nil
	}
	return nil
}

// String gives out a string representation of the requirements for debug/display purposes
func (r *referenceReq) String() string {
	c := spew.ConfigState{DisableCapacities: false, Indent: "  "}
	return fmt.Sprintf("\n Reference Requirement: Oper - %v key - %v Requirements:\n %v\n", r.oper, r.key, c.Sdump(r.reqs))
}
