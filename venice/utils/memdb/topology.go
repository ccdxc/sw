// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package memdb

import (
	"fmt"
	"sort"
	"strings"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	createEvent      = "create"
	deleteEvent      = "delete"
	updateEvent      = "update"
	defaultNamespace = "default"
)

const (
	_ = 1 << iota
	updateVrf
	updateIPAM
	updateSgPolicy
)

var order = []string{"SecurityProfile", "IPAMPolicy", "RouteTable", "Vrf", "NetworkSecurityPolicy", "Network"}

type topoFunc func(old, new Object)
type kindWatchOptions map[string]api.ListWatchOptions

type topoInterface interface {
	addNode(obj Object)
	deleteNode(obj Object, evalOpts bool)
	updateNode(old, new Object)
	addBackRef(key, ref, refKind string)
	delBackRef(key, ref, refKind string)
	getRefs(key string) *topoRefs
	dump() string
}

type refCntInterface interface {
	addRefCnt(dsc, kind, tenant, name string)
	delRefCnt(dsc, kind, tenant, name string)
	getWatchOptions(dsc, kind string) api.ListWatchOptions
	dump()
}

type topoRefCnt struct {
	// map of per DSC+kind, tenant+ipamname/sgpolicyname reference count
	// used to generate the watchoptions for filtering
	refs map[string]map[string]int
}

type topoMgr struct {
	sync.Mutex
	md             *Memdb
	topoKinds      []string
	topoTriggerMap map[string]map[string]topoFunc
	topology       map[string]topoInterface
	refCounts      refCntInterface
}

type topoRefs struct {
	refs     map[string][]string
	backRefs map[string][]string
}

type topoNode struct {
	topo map[string]*topoRefs
	tm   *topoMgr
	md   *Memdb
}

func newTopoRefCnt() refCntInterface {
	return &topoRefCnt{
		refs: make(map[string]map[string]int),
	}
}

func (tr *topoRefCnt) addRefCnt(dsc, kind, tenant, name string) {
	key1 := dsc + "%" + kind
	key2 := tenant + "%" + name

	if a, ok := tr.refs[key1]; ok {
		a[key2]++
	} else {
		b := map[string]int{}
		b[key2]++
		tr.refs[key1] = b
	}
}

func (tr *topoRefCnt) delRefCnt(dsc, kind, tenant, name string) {
	key1 := dsc + "%" + kind
	key2 := tenant + "%" + name

	if a, ok := tr.refs[key1]; ok {
		if _, ok := a[key2]; ok {
			a[key2]--
			if a[key2] == 0 {
				delete(a, key2)
			}
		}
	}
}

func (tr *topoRefCnt) getWatchOptions(dsc, kind string) api.ListWatchOptions {
	ret := api.ListWatchOptions{}

	key1 := dsc + "%" + kind
	str := ""
	tenant := ""
	if a, ok := tr.refs[key1]; ok {
		first := true
		cnt := len(a)
		for key := range a {

			keys := strings.Split(key, "%")
			if first == true {
				tenant = keys[0]
				first = false
			}
			str += keys[1]
			cnt--
			if cnt != 0 {
				str += ","
			}
		}
	}

	if str != "" {
		fieldSelStr := fmt.Sprintf("tenant=%s,name in (%s)", tenant, str)
		ret.FieldSelector = fieldSelStr
	}

	return ret
}

func (tr *topoRefCnt) dump() {
	for k, r := range tr.refs {
		log.Infof("Refcnts for: %s", k)
		for k1, r1 := range r {
			log.Infof("   %s | %d", k1, r1)
		}
	}
}

func (tm *topoMgr) newTopoNode() topoInterface {
	node := &topoNode{
		topo: make(map[string]*topoRefs),
		tm:   tm,
		md:   tm.md,
	}
	return node
}

func newTopoRefs() *topoRefs {
	return &topoRefs{
		refs:     make(map[string][]string),
		backRefs: make(map[string][]string),
	}
}

func (tm *topoMgr) addObjBackref(objKey, objKind, ref, refKind string) {
	if node, ok := tm.topology[objKind]; ok {
		node.addBackRef(objKey, ref, refKind)
	} else {
		node := tm.newTopoNode()
		node.addBackRef(objKey, ref, refKind)
	}
}

func (tm *topoMgr) delObjBackref(objKey, objKind, ref, refKind string) {
	if node, ok := tm.topology[objKind]; ok {
		node.delBackRef(objKey, ref, refKind)
	}
}

func (tn *topoNode) getRefs(key string) *topoRefs {
	if a, ok := tn.topo[key]; ok {
		return a
	}
	return nil
}

func (tn *topoNode) updateBackRef(key, ref, refKind string) {
	if topos, ok := tn.topo[key]; ok {
		if ref != "" {
			topos.backRefs[refKind] = append(topos.backRefs[refKind], ref)
		} else {
			delete(topos.backRefs, refKind)
		}
	} else {
		if ref == "" {
			return
		}
		topos := newTopoRefs()
		topos.backRefs[refKind] = []string{ref}
		tn.topo[key] = topos
	}
}

func (tn *topoNode) addBackRef(key, ref, refKind string) {
	if topos, ok := tn.topo[key]; ok {
		topos.backRefs[refKind] = append(topos.backRefs[refKind], ref)
	} else {
		topos := newTopoRefs()
		topos.backRefs[refKind] = append(topos.backRefs[refKind], ref)
		tn.topo[key] = topos
	}
}

func deleteElement(refs []string, elm string) []string {
	for a, b := range refs {
		if b == elm {
			refs = append(refs[:a], refs[a+1:]...)
			return refs
		}
	}
	return refs
}

func (tn *topoNode) delBackRef(key, ref, refKind string) {
	if topos, ok := tn.topo[key]; ok {
		if len(topos.backRefs[refKind]) == 1 {
			delete(topos.backRefs, refKind)
			return
		}
		topos.backRefs[refKind] = deleteElement(topos.backRefs[refKind], ref)
	}
}

func (tn *topoNode) evalWatchOptions(dsc, kind, key, tenant, ns string, mod uint) kindWatchOptions {
	ret := kindWatchOptions{}
	log.Infof("Received for dsc: %s | kind: %s | key: %s | mod %dƒ", dsc, kind, key, mod)
	switch kind {
	case "Interface":
		if r, ok := tn.topo[key]; ok {
			// update the watchoptions for ipampolicy, Vrf, network
			if nw, ok := r.refs["Network"]; ok && len(nw) != 0 {
				k1 := getKey(tenant, ns, nw[0])
				node := tn.tm.topology["Network"]
				rr := node.getRefs(k1)
				if rr == nil {
					log.Errorf("topo references not found for key: %s", k1)
					tn.tm.dump()
					return ret
				}
				fwRefs := rr.refs
				if fwRefs == nil {
					log.Errorf("forward topo references not found for key: %s", k1)
					tn.tm.dump()
					return ret
				}

				if _, ok := fwRefs["Vrf"]; ok {
					opts := api.ListWatchOptions{}
					opts.Tenant = tenant
					ret["Vrf"] = opts
				}

				if ipamRef, ok := fwRefs["IPAMPolicy"]; ok && len(ipamRef) != 0 {
					opts := tn.tm.refCounts.getWatchOptions(dsc, "IPAMPolicy")
					ret["IPAMPolicy"] = opts
				} else {
					// if an IPAM policy is not configured at the network level, pick the default
					// configured for the vrf
					if vrfRef, ok := fwRefs["Vrf"]; ok && len(vrfRef) != 0 {
						k2 := getKey(tenant, ns, vrfRef[0])
						node1 := tn.tm.topology["Vrf"]
						rr1 := node1.getRefs(k2)
						if rr1 != nil {
							fwRefs1 := rr1.refs
							if fwRefs1 != nil {
								if ipamDefRef, ok := fwRefs1["IPAMPolicy"]; ok && len(ipamDefRef) != 0 {
									opts := tn.tm.refCounts.getWatchOptions(dsc, "IPAMPolicy")
									ret["IPAMPolicy"] = opts
								}
							}
						}

					}
				}

				if sgRef, ok := fwRefs["NetworkSecurityPolicy"]; ok && len(sgRef) != 0 {
					opts := tn.tm.refCounts.getWatchOptions(dsc, "NetworkSecurityPolicy")
					ret["NetworkSecurityPolicy"] = opts
				}

				opts := api.ListWatchOptions{}
				opts.Tenant = tenant
				ret["Network"] = opts
				ret["RouteTable"] = opts
				ret["SecurityProfile"] = opts
			}
		}
	case "Network":
		if r, ok := tn.topo[key]; ok {
			strs := strings.Split(key, "/")
			if mod&updateSgPolicy == updateSgPolicy {
				opts := tn.tm.refCounts.getWatchOptions(dsc, "NetworkSecurityPolicy")
				ret["NetworkSecurityPolicy"] = opts
			}

			if mod&updateIPAM == updateIPAM {
				fwRef := r.refs
				if fwRef != nil {
					if ipam, ok := fwRef["IPAMPolicy"]; ok && len(ipam) != 0 {
						opts := tn.tm.refCounts.getWatchOptions(dsc, "IPAMPolicy")
						ret["IPAMPolicy"] = opts
					} else {
						// if an IPAM policy is not configured at the network level, pick the default
						// configured for the vrf
						if vrfRef, ok := fwRef["Vrf"]; ok && len(vrfRef) != 0 {
							k2 := getKey(strs[0], strs[1], vrfRef[0])
							node1 := tn.tm.topology["Vrf"]
							rr1 := node1.getRefs(k2)
							if rr1 != nil {
								fwRefs1 := rr1.refs
								if fwRefs1 != nil {
									if ipamDefRef, ok := fwRefs1["IPAMPolicy"]; ok && len(ipamDefRef) != 0 {
										tn.tm.refCounts.addRefCnt(dsc, "IPAMPolicy", strs[0], ipamDefRef[0])
										opts := tn.tm.refCounts.getWatchOptions(dsc, "IPAMPolicy")
										ret["IPAMPolicy"] = opts
									}
								}
							}
						}
					}
				}
			}

			if mod&updateVrf == updateVrf {
				opts := api.ListWatchOptions{}
				opts.Tenant = strs[0]
				ret["Vrf"] = opts
				ret["RouteTable"] = opts
				ret["SecurityProfile"] = opts
			}
		}
	case "Vrf":
		opts := tn.tm.refCounts.getWatchOptions(dsc, "IPAMPolicy")
		ret["IPAMPolicy"] = opts
	}
	return ret
}

func getKey(tenant, ns, name string) string {
	ometa := api.ObjectMeta{}
	ometa.Tenant = tenant
	ometa.Namespace = ns
	ometa.Name = name
	return memdbKey(&ometa)
}

func (tn *topoNode) updateRefCounts(key, dsc, tenant, ns string, add bool) {
	log.Infof("Received for key: %s | dsc: %s | tenat: %s | ns: %s | add: %v", key, dsc, tenant, ns, add)
	node := tn.tm.topology["Network"]
	rr := node.getRefs(key)
	if rr == nil {
		log.Errorf("topo references not found for key: %s", key)
		tn.tm.dump()
		return
	}
	fwRefs := rr.refs
	if fwRefs == nil {
		log.Errorf("forward topo references not found for key: %s", key)
		tn.tm.dump()
		return
	}

	if ipamRef, ok := fwRefs["IPAMPolicy"]; ok {
		if add {
			tn.tm.refCounts.addRefCnt(dsc, "IPAMPolicy", tenant, ipamRef[0])
		} else {
			tn.tm.refCounts.delRefCnt(dsc, "IPAMPolicy", tenant, ipamRef[0])
		}
	} else {
		if vrfRef, ok := fwRefs["Vrf"]; ok {
			k2 := getKey(tenant, ns, vrfRef[0])
			node1 := tn.tm.topology["Vrf"]
			rr1 := node1.getRefs(k2)
			if rr1 != nil {
				fwRefs1 := rr1.refs
				if fwRefs1 != nil {
					if ipamDefRef, ok := fwRefs1["IPAMPolicy"]; ok {
						if add {
							tn.tm.refCounts.addRefCnt(dsc, "IPAMPolicy", tenant, ipamDefRef[0])
						} else {
							tn.tm.refCounts.delRefCnt(dsc, "IPAMPolicy", tenant, ipamDefRef[0])
						}
					}
				}
			}

		}
	}

	if sgRefs, ok := fwRefs["NetworkSecurityPolicy"]; ok {
		for _, sgRef := range sgRefs {
			if add {
				tn.tm.refCounts.addRefCnt(dsc, "NetworkSecurityPolicy", tenant, sgRef)
			} else {
				tn.tm.refCounts.delRefCnt(dsc, "NetworkSecurityPolicy", tenant, sgRef)
			}
		}
	}
	tn.tm.refCounts.dump()
}

func (tn *topoNode) updateWatchOptions(dsc string, opts kindWatchOptions) {
	for _, kind := range order {
		if opt, ok := opts[kind]; ok {
			oldFilters, newFilters, err := tn.md.addDscWatchOptions(dsc, kind, opt)
			if err == nil {
				log.Infof("Sending reconcile event for dsc: %s | kind: %s", dsc, kind)
				tn.md.sendReconcileEvent(dsc, kind, oldFilters, newFilters)
			} else {
				log.Errorf("addDscWatchOptions returned error: %s", err)
			}
		}
	}
}

func (tn *topoNode) addNode(obj Object) {
	kind := obj.GetObjectKind()
	switch kind {
	case "IPAMPolicy":
		key := memdbKey(obj.GetObjectMeta())
		if _, ok := tn.topo[key]; ok {
			// already exists
			return
		}
		tn.topo[key] = newTopoRefs()
	case "NetworkSecurityPolicy":
		key := memdbKey(obj.GetObjectMeta())
		if _, ok := tn.topo[key]; ok {
			// already exists
			return
		}
		tn.topo[key] = newTopoRefs()
	case "Vrf":
		key := memdbKey(obj.GetObjectMeta())
		if _, ok := tn.topo[key]; ok {
			// already exists
			return
		}
		vr := obj.(*netproto.Vrf)
		topoRefs := newTopoRefs()
		if vr.Spec.IPAMPolicy != "" {
			topoRefs.refs["IPAMPolicy"] = []string{vr.Spec.IPAMPolicy}
		}
		tn.topo[key] = topoRefs
	case "Interface":
		nwIf := obj.(*netproto.Interface)
		log.Infof("Interface add for: %v | dsc : %v", nwIf.Spec, nwIf.Status.DSC)
		tenant := ""
		if nwIf.Spec.VrfName != "" {
			tenant = nwIf.Spec.VrfName
		} else {
			return
		}

		key := getKey(obj.GetObjectMeta().Tenant, obj.GetObjectMeta().Namespace, obj.GetObjectMeta().Name)
		var topoRefs *topoRefs
		if refs, ok := tn.topo[key]; ok {
			topoRefs = refs
		} else {
			topoRefs = newTopoRefs()
		}

		if tenant != "" && nwIf.Spec.Network != "" {
			topoRefs.refs["Network"] = []string{nwIf.Spec.Network}
			k1 := getKey(tenant, obj.GetObjectMeta().Namespace, nwIf.Spec.Network)
			tn.tm.addObjBackref(k1, "Network", key, kind)
			tn.topo[key] = topoRefs

			// trigger an update to watchoptions of the effected kinds
			dsc := nwIf.Status.DSC
			tn.updateRefCounts(k1, dsc, tenant, obj.GetObjectMeta().Namespace, true)
			opts := tn.evalWatchOptions(dsc, kind, key, tenant, obj.GetObjectMeta().Namespace, updateVrf|updateIPAM|updateSgPolicy)
			log.Infof("New watchoptions after topo update: %v", opts)
			tn.updateWatchOptions(dsc, opts)
		}
	case "Network":
		key := memdbKey(obj.GetObjectMeta())
		if _, ok := tn.topo[key]; ok {
			return
		}
		nw := obj.(*netproto.Network)
		topoRefs := newTopoRefs()
		if nw.Spec.IPAMPolicy != "" {
			topoRefs.refs["IPAMPolicy"] = []string{nw.Spec.IPAMPolicy}
		}

		if nw.Spec.VrfName != "" {
			topoRefs.refs["Vrf"] = []string{nw.Spec.VrfName}
			k1 := getKey(obj.GetObjectMeta().Tenant, obj.GetObjectMeta().Namespace, nw.Spec.VrfName)
			tn.tm.addObjBackref(k1, "Vrf", key, kind)
		}
		if len(nw.Spec.IngV4SecurityPolicyID) != 0 {
			topoRefs.refs["NetworkSecurityPolicy"] = append(topoRefs.refs["NetworkSecurityPolicy"], nw.Spec.IngV4SecurityPolicyID...)
		}
		if len(nw.Spec.IngV6SecurityPolicyID) != 0 {
			topoRefs.refs["NetworkSecurityPolicy"] = append(topoRefs.refs["NetworkSecurityPolicy"], nw.Spec.IngV6SecurityPolicyID...)
		}
		if len(nw.Spec.EgV6SecurityPolicyID) != 0 {
			topoRefs.refs["NetworkSecurityPolicy"] = append(topoRefs.refs["NetworkSecurityPolicy"], nw.Spec.EgV6SecurityPolicyID...)
		}
		if len(nw.Spec.EgV4SecurityPolicyID) != 0 {
			topoRefs.refs["NetworkSecurityPolicy"] = append(topoRefs.refs["NetworkSecurityPolicy"], nw.Spec.EgV4SecurityPolicyID...)
		}

		tn.topo[key] = topoRefs
	}
}

func (tn *topoNode) deleteNode(obj Object, evalOpts bool) {
	kind := obj.GetObjectKind()
	switch kind {
	case "Interface":
		nwIf := obj.(*netproto.Interface)
		tenant := ""
		if nwIf.Spec.VrfName != "" {
			tenant = nwIf.Spec.VrfName
		} else {
			return
		}

		key := getKey(obj.GetObjectMeta().Tenant, obj.GetObjectMeta().Namespace, obj.GetObjectMeta().Name)
		if _, ok := tn.topo[key]; !ok {
			// doesn't exist
			log.Errorf("topo doesn't exist for: %v", obj.GetObjectMeta())
			return
		}
		topoRefs := tn.topo[key]
		fwRefs := topoRefs.refs
		if tenant != "" && nwIf.Spec.Network != "" {
			delete(fwRefs, "Network")

			k1 := getKey(tenant, obj.GetObjectMeta().Namespace, nwIf.Spec.Network)
			// clear the back reference
			tn.tm.delObjBackref(k1, "Network", "", kind)

			dsc := nwIf.Status.DSC
			// decrement the refcnt
			tn.updateRefCounts(k1, dsc, tenant, obj.GetObjectMeta().Namespace, false)
			if evalOpts {
				// trigger an update to watchoptions of the effected kinds
				opts := tn.evalWatchOptions(dsc, kind, key, tenant, obj.GetObjectMeta().Namespace, updateVrf|updateIPAM|updateSgPolicy)
				log.Infof("New watchoptions after topo update: %v", opts)
				for _, kind := range order {
					if opt, ok := opts[kind]; ok {
						oldFilters, newFilters, err := tn.md.addDscWatchOptions(dsc, kind, opt)
						if err == nil {
							tn.md.sendReconcileEvent(dsc, kind, oldFilters, newFilters)
						}
					} else {
						oldFilters := tn.md.clearWatchOptions(dsc, kind)
						if len(oldFilters) != 0 {
							tn.md.sendReconcileEvent(dsc, kind, oldFilters, nil)
						}
					}
				}
			}
		}
	case "Network":
		// with dependency check logic, network delete will not happen until interface is detached from it
		// on delete, delete the topo node remove the back references
		key := getKey(obj.GetObjectMeta().Tenant, obj.GetObjectMeta().Namespace, obj.GetObjectMeta().Name)

		topoRefs := tn.topo[key]
		if topoRefs != nil {
			fwRefs := topoRefs.refs
			if len(fwRefs) != 0 {
				if vrf, ok := fwRefs["Vrf"]; ok {
					k1 := getKey(obj.GetObjectMeta().Tenant, obj.GetObjectMeta().Namespace, vrf[0])
					tn.tm.delObjBackref(k1, "Vrf", key, kind)
				}

				if ipam, ok := fwRefs["IPAMPolicy"]; ok {
					k1 := getKey(obj.GetObjectMeta().Tenant, obj.GetObjectMeta().Namespace, ipam[0])
					tn.tm.delObjBackref(k1, "IPAMPolicy", key, kind)
				}
			}
		}
		delete(tn.topo, key)
	case "Vrf":
		key := getKey(obj.GetObjectMeta().Tenant, obj.GetObjectMeta().Namespace, obj.GetObjectMeta().Name)

		topoRefs := tn.topo[key]
		if topoRefs != nil {
			fwRefs := topoRefs.refs
			if len(fwRefs) != 0 {
				if ipam, ok := fwRefs["IPAMPolicy"]; ok {
					k1 := getKey(obj.GetObjectMeta().Tenant, obj.GetObjectMeta().Namespace, ipam[0])
					tn.tm.delObjBackref(k1, "IPAMPolicy", key, kind)
				}
			}
		}
		delete(tn.topo, key)
	}
}

func (tn *topoNode) updateNode(old, new Object) {
	kind := new.GetObjectKind()
	switch kind {
	case "Interface":
		oldObj := old.(*netproto.Interface)
		newObj := new.(*netproto.Interface)

		log.Infof("Old interface spec: %v | dsc: %v | new spec: %v | dsc: %v", oldObj.Spec, oldObj.Status.DSC, newObj.Spec, newObj.Status.DSC)
		newTenant := newObj.Spec.VrfName
		newNw := newObj.Spec.Network

		oldTenant := oldObj.Spec.VrfName
		oldNw := oldObj.Spec.Network

		evalOpts := true

		if newTenant != "" && newNw != "" {
			evalOpts = false
		}

		if oldTenant != "" && oldNw != "" {
			// remove the oldObj references
			tn.deleteNode(old, evalOpts)
		}

		if evalOpts == false {
			tn.addNode(new)
		}
	case "Network":
		oldObj := old.(*netproto.Network)
		newObj := new.(*netproto.Network)

		key := getKey(oldObj.Tenant, oldObj.Namespace, oldObj.Name)
		var mod uint

		oldVrf := oldObj.Spec.VrfName
		newVrf := newObj.Spec.VrfName

		topoRefs := tn.topo[key]

		if topoRefs == nil {
			return
		}
		fwRefs := topoRefs.refs

		backRefs := topoRefs.backRefs
		dsc := ""
		if backRefs != nil {
			if nwRef, ok := backRefs["Interface"]; ok && len(nwRef) != 0 {
				nwIf := nwRef[0]
				// there is an interface attached to this network
				od := tn.md.getObjectDBByType("Interface")
				od.Lock()
				nwIfObj := od.getObject(nwIf)
				od.Unlock()
				obj1 := nwIfObj.Object()
				obj2 := obj1.(*netproto.Interface)
				dsc = obj2.Status.DSC
			}
		}

		if oldVrf != newVrf {
			mod |= updateVrf
			if newVrf != "" {
				fwRefs["Vrf"] = []string{newVrf}
				k1 := getKey(oldObj.Tenant, oldObj.Namespace, newVrf)
				tn.tm.addObjBackref(k1, "Vrf", key, "Network")
				if oldVrf != "" {
					k1 := getKey(oldObj.Tenant, oldObj.Namespace, oldVrf)
					tn.tm.delObjBackref(k1, "Vrf", key, "Network")
				}
			} else {
				if fwRefs != nil {
					delete(fwRefs, "Vrf")
				}
				k1 := getKey(oldObj.Tenant, oldObj.Namespace, oldVrf)
				tn.tm.delObjBackref(k1, "Vrf", key, "Network")
			}
		}

		oldIPAM := oldObj.Spec.IPAMPolicy
		newIPAM := newObj.Spec.IPAMPolicy
		if oldIPAM != newIPAM {
			mod |= updateIPAM

			if newIPAM != "" {
				if fwRefs != nil {
					fwRefs["IPAMPolicy"] = []string{newIPAM}
				}
				if dsc != "" {
					if oldIPAM != "" {
						tn.tm.refCounts.delRefCnt(dsc, "IPAMPolicy", oldObj.Tenant, oldIPAM)
					}
					tn.tm.refCounts.addRefCnt(dsc, "IPAMPolicy", oldObj.Tenant, newIPAM)
					tn.tm.refCounts.dump()
				}
			} else {
				if fwRefs != nil {
					delete(fwRefs, "IPAMPolicy")
				}
				if dsc != "" {
					tn.tm.refCounts.delRefCnt(dsc, "IPAMPolicy", oldObj.Tenant, oldIPAM)
					tn.tm.refCounts.dump()
				}
			}
		}

		sgPolicies := []string{}
		sgPolicies = append(sgPolicies, oldObj.Spec.IngV4SecurityPolicyID...)
		sgPolicies = append(sgPolicies, oldObj.Spec.EgV4SecurityPolicyID...)
		sgPolicies = append(sgPolicies, oldObj.Spec.IngV6SecurityPolicyID...)
		sgPolicies = append(sgPolicies, oldObj.Spec.EgV6SecurityPolicyID...)

		nsgPolicies := []string{}
		nsgPolicies = append(nsgPolicies, newObj.Spec.IngV4SecurityPolicyID...)
		nsgPolicies = append(nsgPolicies, newObj.Spec.EgV4SecurityPolicyID...)
		nsgPolicies = append(nsgPolicies, newObj.Spec.IngV6SecurityPolicyID...)
		nsgPolicies = append(nsgPolicies, newObj.Spec.EgV6SecurityPolicyID...)

		delSg, addSg := getSgPolicyDiffs(sgPolicies, nsgPolicies)

		if len(delSg) != 0 || len(addSg) != 0 {
			mod |= updateSgPolicy

			for _, a := range delSg {
				// update references
				fwRefs["NetworkSecurityPolicy"] = deleteElement(fwRefs["NetworkSecurityPolicy"], a)
				if dsc != "" {
					// update refcnts
					tn.tm.refCounts.delRefCnt(dsc, "NetworkSecurityPolicy", oldObj.Tenant, a)
				}
			}

			if len(addSg) != 0 {
				fwRefs["NetworkSecurityPolicy"] = append(fwRefs["NetworkSecurityPolicy"], addSg...)
			}

			if dsc != "" {
				for _, b := range addSg {
					// update refcnts
					tn.tm.refCounts.addRefCnt(dsc, "NetworkSecurityPolicy", oldObj.Tenant, b)
					tn.tm.refCounts.dump()
				}
			}
		}

		if dsc != "" {
			opts := tn.evalWatchOptions(dsc, "Network", key, "", "", mod)
			log.Infof("New watchoptions after topo update: %v", opts)
			tn.updateWatchOptions(dsc, opts)
		}
	case "Vrf":
		oldObj := old.(*netproto.Vrf)
		newObj := new.(*netproto.Vrf)

		oldIPAM := oldObj.Spec.IPAMPolicy
		newIPAM := newObj.Spec.IPAMPolicy

		key := getKey(oldObj.Tenant, oldObj.Namespace, oldObj.Name)
		topoRefs := tn.topo[key]

		if topoRefs == nil {
			return
		}
		fwRefs := topoRefs.refs

		backRefs := topoRefs.backRefs
		dsc := ""
		oldIPAMUsed := false
		mod := false

		if backRefs != nil {
			if nwRef, ok := backRefs["Network"]; ok && len(nwRef) != 0 {
				nw := nwRef[0]
				node := tn.tm.topology["Network"]
				rr := node.getRefs(nw)
				if rr != nil {
					fr := rr.refs
					if fr != nil {
						if ipam, ok := fr["IPAMPolicy"]; !ok || len(ipam) == 0 {
							// network doesn't have an IPAM configured
							// the default configured in the vrf is used
							oldIPAMUsed = true
						}
					}
					br := rr.backRefs
					if br != nil {
						if nwIfRef, ok := br["Interface"]; ok && len(nwIfRef) != 0 {
							nwIf := nwIfRef[0]
							od := tn.md.getObjectDBByType("Interface")
							od.Lock()
							nwIfObj := od.getObject(nwIf)
							od.Unlock()
							obj1 := nwIfObj.Object()
							obj2 := obj1.(*netproto.Interface)
							dsc = obj2.Status.DSC
							mod = true
						}
					}
				}
			}
		}
		if oldIPAM != "" {
			if oldIPAMUsed && dsc != "" {
				tn.tm.refCounts.delRefCnt(dsc, "IPAMPolicy", oldObj.Tenant, oldIPAM)
			}
			if newIPAM == "" {
				if fwRefs != nil {
					delete(fwRefs, "IPAMPolicy")
				}
			} else {
				if fwRefs != nil {
					fwRefs["IPAMPolicy"] = []string{newIPAM}
				}
				if dsc != "" {
					tn.tm.refCounts.addRefCnt(dsc, "IPAMPolicy", oldObj.Tenant, newIPAM)
					tn.tm.refCounts.dump()
				}
			}
		} else {
			if fwRefs != nil {
				fwRefs["IPAMPolicy"] = []string{newIPAM}
			}
			if dsc != "" {
				tn.tm.refCounts.addRefCnt(dsc, "IPAMPolicy", oldObj.Tenant, newIPAM)
				tn.tm.refCounts.dump()
			}
		}

		if mod {
			opts := tn.evalWatchOptions(dsc, "Vrf", key, "", "", 0)
			log.Infof("New watchoptions after topo update: %v", opts)
			tn.updateWatchOptions(dsc, opts)
		}
	}
}

func getSgPolicyDiffs(old, new []string) ([]string, []string) {
	dels, adds := []string{}, []string{}
	map1, map2 := map[string]struct{}{}, map[string]struct{}{}

	for _, s := range old {
		if _, ok := map1[s]; !ok {
			map1[s] = struct{}{}
		}
	}

	for _, s := range new {
		if _, ok := map2[s]; !ok {
			map2[s] = struct{}{}
		}
	}

	for k := range map1 {
		if _, ok := map2[k]; !ok {
			dels = append(dels, k)
		}
	}

	for k := range map2 {
		if _, ok := map1[k]; !ok {
			adds = append(adds, k)
		}
	}
	return dels, adds
}

/*
func (tn *topoNode) dump() string {
	ret := ""

	for kind, node := range tn.topo {
		ret += fmt.Sprintf("  Object key: %s\n", kind)
		for key, ref := range node.refs {
			ret += fmt.Sprintf("    Ref key: %s | refs %v\n", key, ref)
		}
		for k, bref := range node.backRefs {
			ret += fmt.Sprintf("    BackRef key: %s | refs %v\n", k, bref)
		}
	}
	return ret
}

func (tm *topoMgr) dump() string {
	ret := ""
	ret += "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"

	for kind, obj := range tm.topology {
		ret += fmt.Sprintf("Topology for kind: %s\n", kind)
		ret += obj.dump()
	}
	ret += "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"

	return ret
}
*/

func (tn *topoNode) dump() string {
	ret := ""

	for kind, node := range tn.topo {
		log.Infof("  Object key: %s", kind)
		for key, ref := range node.refs {
			log.Infof("    Ref key: %s | refs %v", key, ref)
		}
		for k, bref := range node.backRefs {
			log.Infof("    BackRef key: %s | refs %v", k, bref)
		}
	}
	return ret
}

func (tm *topoMgr) dump() string {
	ret := ""
	log.Info("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")

	for kind, obj := range tm.topology {
		log.Infof("Topology for kind: %s", kind)
		obj.dump()
	}
	log.Info("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")

	return ret
}

func newTopoMgr(md *Memdb) topoMgrInterface {
	mgr := &topoMgr{
		md:             md,
		topoKinds:      []string{"Interface", "Network", "Vrf", "IPAMPolicy", "NetworkSecurityPolicy"},
		topoTriggerMap: make(map[string]map[string]topoFunc),
		topology:       make(map[string]topoInterface),
	}
	mgr.refCounts = newTopoRefCnt()

	for _, kind := range mgr.topoKinds {
		switch kind {
		case "Network":
			funcs := map[string]topoFunc{}
			funcs[createEvent] = mgr.handleNetworkCreate
			funcs[deleteEvent] = mgr.handleNetworkDelete
			funcs[updateEvent] = mgr.handleNetworkUpdate
			mgr.topoTriggerMap[kind] = funcs
		case "Interface":
			funcs := map[string]topoFunc{}
			funcs[createEvent] = mgr.handleInterfaceCreate
			funcs[deleteEvent] = mgr.handleInterfaceDelete
			funcs[updateEvent] = mgr.handleInterfaceUpdate
			mgr.topoTriggerMap[kind] = funcs
		case "Vrf":
			funcs := map[string]topoFunc{}
			funcs[createEvent] = mgr.handleVrfCreate
			funcs[deleteEvent] = mgr.handleVrfDelete
			funcs[updateEvent] = mgr.handleVrfUpdate
			mgr.topoTriggerMap[kind] = funcs
		case "IPAMPolicy", "NetworkSecurityPolicy":
			funcs := map[string]topoFunc{}
			funcs[createEvent] = mgr.handleObjectCreate
			funcs[deleteEvent] = mgr.handleObjectDelete
			funcs[updateEvent] = mgr.handleObjectUpdate
			mgr.topoTriggerMap[kind] = funcs
		}
	}
	return mgr
}

func (tm *topoMgr) handleAddEvent(obj Object) {
	if tm.md.isControllerWatchFilter("IPAMPolicy") == false {
		// just return
		return
	}
	if handler, ok := tm.topoTriggerMap[obj.GetObjectKind()]; ok {
		tm.Lock()
		defer tm.Unlock()
		fn := handler[createEvent]
		fn(nil, obj)
		tm.dump()
	}
}

func (tm *topoMgr) handleUpdateEvent(old, new Object) {
	if tm.md.isControllerWatchFilter("IPAMPolicy") == false {
		// just return
		return
	}
	if handler, ok := tm.topoTriggerMap[new.GetObjectKind()]; ok {
		tm.Lock()
		defer tm.Unlock()
		fn := handler[updateEvent]
		fn(old, new)
	}
}

func (tm *topoMgr) handleDeleteEvent(obj Object) {
	if tm.md.isControllerWatchFilter("IPAMPolicy") == false {
		// just return
		return
	}
	if handler, ok := tm.topoTriggerMap[obj.GetObjectKind()]; ok {
		tm.Lock()
		defer tm.Unlock()
		fn := handler[deleteEvent]
		fn(nil, obj)
	}
}

func (tm *topoMgr) handleObjectCreate(old, new Object) {
	kind := new.GetObjectKind()
	if node, ok := tm.topology[kind]; ok {
		node.addNode(new)
	} else {
		node := tm.newTopoNode()
		node.addNode(new)
		tm.topology[kind] = node
	}
}

func (tm *topoMgr) handleObjectUpdate(old, new Object) {
	if node, ok := tm.topology[new.GetObjectKind()]; ok {
		node.updateNode(old, new)
	} else {
		log.Error("Object doesn't exist in the topo", new)
	}
}

func (tm *topoMgr) handleObjectDelete(old, new Object) {
	if node, ok := tm.topology[new.GetObjectKind()]; ok {
		node.deleteNode(new, true)
	} else {
		log.Error("Object doesn't exist in the topo", new)
	}
}

func (tm *topoMgr) handleVrfCreate(old, new Object) {
	kind := new.GetObjectKind()
	if node, ok := tm.topology[kind]; ok {
		node.addNode(new)
	} else {
		node := tm.newTopoNode()
		node.addNode(new)
		tm.topology[kind] = node
	}
}

func (tm *topoMgr) handleVrfUpdate(old, new Object) {
	oldObj := old.(*netproto.Vrf)
	newObj := new.(*netproto.Vrf)

	if oldObj.Spec.IPAMPolicy == newObj.Spec.IPAMPolicy {
		// not a topo trigger
		return
	}

	if node, ok := tm.topology[new.GetObjectKind()]; ok {
		node.updateNode(old, new)
	} else {
		log.Error("Object doesn't exist in the topo", new)
	}
}

func (tm *topoMgr) handleVrfDelete(old, new Object) {
	if node, ok := tm.topology[new.GetObjectKind()]; ok {
		node.deleteNode(new, true)
	} else {
		log.Error("Object doesn't exist in the topo", new)
	}
}

func (tm *topoMgr) handleNetworkCreate(old, new Object) {
	kind := new.GetObjectKind()
	if node, ok := tm.topology[kind]; ok {
		node.addNode(new)
	} else {
		node := tm.newTopoNode()
		node.addNode(new)
		tm.topology[kind] = node
	}
}

func (tm *topoMgr) handleNetworkDelete(old, new Object) {
	if node, ok := tm.topology[new.GetObjectKind()]; ok {
		node.deleteNode(new, true)
	} else {
		log.Error("Object doesn't exist in the topo", new)
	}
}

func sgPoliciesUpdated(str1, str2 []string) bool {
	if len(str1) != len(str2) {
		return true
	}
	sort.Strings(str1)
	sort.Strings(str2)

	for x, y := range str1 {
		if y != str2[x] {
			return true
		}
	}
	return false
}

func (tm *topoMgr) handleNetworkUpdate(old, new Object) {
	oldObj := old.(*netproto.Network)
	newObj := old.(*netproto.Network)

	// TODO use objDiff??
	if oldObj.Spec.IPAMPolicy == newObj.Spec.IPAMPolicy && oldObj.Spec.VrfName == newObj.Spec.VrfName &&
		!sgPoliciesUpdated(oldObj.Spec.IngV4SecurityPolicyID, newObj.Spec.IngV4SecurityPolicyID) &&
		!sgPoliciesUpdated(oldObj.Spec.EgV4SecurityPolicyID, newObj.Spec.EgV4SecurityPolicyID) &&
		!sgPoliciesUpdated(oldObj.Spec.IngV6SecurityPolicyID, newObj.Spec.IngV6SecurityPolicyID) &&
		!sgPoliciesUpdated(oldObj.Spec.EgV4SecurityPolicyID, newObj.Spec.EgV6SecurityPolicyID) {
		// not a topology trigger
		return
	}
	if node, ok := tm.topology[new.GetObjectKind()]; ok {
		node.updateNode(old, new)
	} else {
		log.Error("Object doesn't exist in the topo", new)
	}
}

func (tm *topoMgr) handleInterfaceCreate(old, new Object) {
	log.Infof("Interface add received: %v", new.GetObjectMeta())
	kind := new.GetObjectKind()
	if node, ok := tm.topology[kind]; ok {
		node.addNode(new)
	} else {
		node := tm.newTopoNode()
		node.addNode(new)
		tm.topology[kind] = node
	}
}

func (tm *topoMgr) handleInterfaceDelete(old, new Object) {
	if node, ok := tm.topology[new.GetObjectKind()]; ok {
		node.deleteNode(new, true)
	} else {
		log.Error("Object doesn't exist in the topo", new)
	}
}

func (tm *topoMgr) handleInterfaceUpdate(old, new Object) {
	if old == nil || new == nil {
		log.Error("Invalid obect received ", old, new)
		return
	}
	oldObj := old.(*netproto.Interface)
	newObj := new.(*netproto.Interface)
	log.Infof("Interface update received for old: %v, new: %v", oldObj.Spec, newObj.Spec)
	// TODO use objDiff??
	if oldObj.Spec.Network == newObj.Spec.Network && oldObj.Spec.VrfName == newObj.Spec.VrfName {
		//not a topology trigger
		return
	}
	if node, ok := tm.topology[new.GetObjectKind()]; ok {
		node.updateNode(old, new)
	} else {
		log.Error("Object doesn't exist in the topo", new)
	}
}

// SendRoutingConfig handles routing config refered to by DSC config
func (md *Memdb) SendRoutingConfig(dsc, oldRtCfg, newRtCfg string) {
	if oldRtCfg == "" {
		opts := api.ListWatchOptions{}
		opts.Name = newRtCfg
		_, newFlt, err := md.addDscWatchOptions(dsc, "RoutingConfig", opts)
		if err != nil {
			log.Errorf("RoutingConfig addDscWatchOptions for dsc: %s | name: %s | err: %s", dsc, newRtCfg, err)
			return
		}
		log.Infof("Sending reconcile event for RoutingConfig | dsc: %s | opts: %v", dsc, opts)
		md.sendReconcileEvent(dsc, "RoutingConfig", nil, newFlt)
	} else if newRtCfg == "" {
		oldFlt := md.clearWatchOptions(dsc, "RoutingConfig")
		if len(oldFlt) != 0 {
			md.sendReconcileEvent(dsc, "RoutingConfig", oldFlt, nil)
		}
	} else {
		opts := api.ListWatchOptions{}
		opts.Name = newRtCfg
		oldFlt, newFlt, err := md.addDscWatchOptions(dsc, "RoutingConfig", opts)
		if err != nil {
			log.Errorf("RoutingConfig addDscWatchOptions for dsc: %s | name(old/new): %s/%s | err: %s", dsc, oldRtCfg, newRtCfg, err)
			return
		}
		log.Infof("Sending reconcile event for RoutingConfig | dsc: %s | opts: %v", dsc, opts)
		md.sendReconcileEvent(dsc, "RoutingConfig", oldFlt, newFlt)
	}
}
