// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package memdb

import (
	"encoding/json"
	"errors"
	"fmt"
	"strconv"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
)

type dscWatcherDB struct {
	md          *Memdb
	watcherInfo map[string]*DSCWatcherInfo
	sync.RWMutex
}

// DSCWatcherInfo stores the per DSC per kind watcher list
type DSCWatcherInfo struct {
	watchers     []chan Event
	watchOptions api.ListWatchOptions
	filterGroup  *WFilterGroup
}

type watcherFilterSet struct {
	md        *Memdb
	filterGrp map[string]*WFilterGroup
	lock      sync.RWMutex
}

// WFilterGroup contains all the configured watch filters for an object kind
type WFilterGroup struct {
	filters      []FilterFn
	watchers     []chan Event
	watchOptions api.ListWatchOptions
}

// flags used to determine watch filter behavior
const (
	_ = 1 << iota
	// indicates controller owns the watch filters
	ControllerWatchFilter
	// indicates the default watch filter is to block the obj from being sent to naples
	DefWatchFilterBlock
)

func fromVersionFilterFn(fromVer uint64) FilterFn {
	return func(obj, prev Object) bool {
		meta := obj.GetObjectMeta()
		ver, err := strconv.ParseUint(meta.ResourceVersion, 10, 64)
		if err != nil {
			log.Fatalf("unable to parse version string [%s](%s)", meta.ResourceVersion, err)
		}
		return ver >= fromVer
	}
}

func nameFilterFn(name string) FilterFn {
	return func(obj, prev Object) bool {
		meta := obj.GetObjectMeta()
		return meta.Name == name
	}
}

func tenantFilterFn(tenant string) FilterFn {
	return func(obj, prev Object) bool {
		meta := obj.GetObjectMeta()
		return meta.Tenant == tenant
	}
}

func namespaceFilterFn(namespace string) FilterFn {
	return func(obj, prev Object) bool {
		meta := obj.GetObjectMeta()
		return meta.Namespace == namespace
	}
}

func labelSelectorFilterFn(selector *labels.Selector) FilterFn {
	return func(obj, prev Object) bool {
		meta := obj.GetObjectMeta()
		labels := labels.Set(meta.Labels)
		return selector.Matches(labels)
	}
}

func fieldSelectorFilterFn(selector *fields.Selector) FilterFn {
	return func(obj, prev Object) bool {
		return selector.MatchesObj(obj)
	}
}

func fieldChangeSelectorFilterFn(selectors []string) FilterFn {
	return func(obj, prev Object) bool {
		diffs, ok := ref.ObjDiff(obj, prev)
		if !ok {
			return false
		}
		for _, f := range selectors {
			if diffs.Lookup(f) {
				return true
			}
		}
		return false
	}
}

// GetWatchFilter returns a filter function to filter Watch Events
func (md *Memdb) GetWatchFilter(kind string, opts *api.ListWatchOptions) []FilterFn {
	var filters []FilterFn

	if opts.ResourceVersion != "" {
		ver, err := strconv.ParseUint(opts.ResourceVersion, 10, 64)
		if err != nil {
			log.Fatalf("unable to parse version string [%s](%s)", opts.ResourceVersion, err)
		}
		filters = append(filters, fromVersionFilterFn(ver))
	}

	if opts.Name != "" {
		// FIX for VS-1305, with Naples running release A code and Venice running newer code, Naples will send the
		// node ID as the name for endpoints to filter.
		if kind == "netproto.Endpoint" {
			newOpts := &api.ListWatchOptions{}
			str := fmt.Sprintf("spec.node-uuid=%s", opts.Name)
			newOpts.FieldSelector = str
			return md.GetWatchFilter(kind, newOpts)
		}
		filters = append(filters, nameFilterFn(opts.Name))
	}

	if opts.Tenant != "" {
		filters = append(filters, tenantFilterFn(opts.Tenant))
	}

	if opts.Namespace != "" {
		filters = append(filters, namespaceFilterFn(opts.Namespace))
	}

	if opts.LabelSelector != "" {
		selector, err := labels.Parse(opts.LabelSelector)
		if err != nil {
			log.Errorf("invalid label selector specification(%s)", err)
			return nil
		}
		filters = append(filters, labelSelectorFilterFn(selector))
	}

	if opts.FieldSelector != "" {
		var selector *fields.Selector
		var err error
		if kind != "" {
			selector, err = fields.ParseWithValidation(kind, opts.FieldSelector)
			if err != nil {
				log.Errorf("invalid field selector specification(%s)", err)
				return nil
			}
		} else {
			selector, err = fields.Parse(opts.FieldSelector)
			if err != nil {
				log.Errorf("invalid field selector specification(%s)", err)
				return nil
			}
		}
		filters = append(filters, fieldSelectorFilterFn(selector))
	}
	if len(opts.FieldChangeSelector) != 0 {
		filters = append(filters, fieldChangeSelectorFilterFn(opts.FieldChangeSelector))
	}

	return filters
}

func (md *Memdb) isControllerWatchFilter(kind string) bool {
	if md.filterFlags[kind]&ControllerWatchFilter == ControllerWatchFilter {
		return true
	}
	return false
}

func (md *Memdb) newDSCWatchDB() dscWatcherDBInterface {
	return &dscWatcherDB{
		md:          md,
		watcherInfo: make(map[string]*DSCWatcherInfo),
	}
}

func (wdb *dscWatcherDB) dump() string {
	ret := ""
	for key, info := range wdb.watcherInfo {
		ret += fmt.Sprintf("    Kind: %s, watchers: %v, options: %v, fgrp: %v\n", key, info.watchers, info.watchOptions, info.filterGroup)
	}
	return ret
}

func (wdb *dscWatcherDB) addDSCWatcherInfo(kind string, watcher chan Event) {
	defer log.Infof("watcher %d | %s added", watcher, kind)
	if info, ok := wdb.watcherInfo[kind]; ok {
		// a watcher for this kind has already been added
		// update the watcher list, and if there is an associated filtergrp, update it too
		info.watchers = append(info.watchers, watcher)
		if info.filterGroup != nil {
			wdb.md.addWFltGrp(kind, info.watchOptions, []chan Event{watcher})
		}
		return
	}

	newInfo := &DSCWatcherInfo{}
	if watcher != nil {
		newInfo.watchers = []chan Event{watcher}
	}
	wdb.watcherInfo[kind] = newInfo
	return
}

func (wdb *dscWatcherDB) addDSCWatcherInfoWatchOptions(kind string, options api.ListWatchOptions) (old, new []FilterFn, err error) {
	if info, ok := wdb.watcherInfo[kind]; ok {
		// if a filter group already exisits, compare the watch options
		// if watchoptions haven't changed, just return
		if info.filterGroup != nil {
			if getWatchOptionsStr(info.watchOptions) == getWatchOptionsStr(options) {
				return info.filterGroup.filters, info.filterGroup.filters, nil
			}

			// watch options got updated, save the old filter group and then update
			oldFilterFuncs := info.filterGroup.filters
			newFilterGrp, err := wdb.md.addWFltGrp(kind, options, info.watchers)
			if err == nil {
				info.filterGroup = newFilterGrp
				wdb.md.delWFltGrp(kind, info.watchOptions, info.watchers)
				info.watchOptions = options
				return oldFilterFuncs, newFilterGrp.filters, nil
			}
			return nil, nil, errors.New("AddDSCWatcherInfoWatchOptions, updating watch options failed")
		}
		newFilterGrp, err := wdb.md.addWFltGrp(kind, options, info.watchers)
		if err == nil {
			info.filterGroup = newFilterGrp
			info.watchOptions = options
			return nil, newFilterGrp.filters, nil
		}
		return nil, nil, errors.New("AddDSCWatcherInfoWatchOptions, updating watch options failed")
	}

	newInfo := &DSCWatcherInfo{}
	newInfo.watchOptions = options
	wdb.watcherInfo[kind] = newInfo
	newFilterGrp, err := wdb.md.addWFltGrp(kind, options, nil)
	if err == nil {
		newInfo.filterGroup = newFilterGrp
		return nil, newFilterGrp.filters, nil
	}
	return nil, nil, errors.New("AddDSCWatcherInfoWatchOptions, updating watch options failed")
}

func (wdb *dscWatcherDB) delDSCWatcherInfo(kind string, watcher chan Event) {
	var err error

	defer func() {
		if err == nil {
			log.Infof("watcher %d | %s deleted", watcher, kind)
		} else {
			log.Errorf("watcher %d | %s delete failed err: %s", watcher, kind, err)
		}
	}()

	if info, ok := wdb.watcherInfo[kind]; ok {
		for i, w := range info.watchers {
			if w == watcher {
				if info.filterGroup != nil {

					fgrp, err := wdb.md.delWFltGrp(kind, info.watchOptions, []chan Event{w})
					if err != nil {
						log.Error("delDSCWatcherInfo, delWatcherFromWFilterGroup returned error: ", err)
					}
					err = nil
					info.filterGroup = fgrp
				}
				info.watchers = append(info.watchers[:i], info.watchers[i+1:]...)
				/*
					if len(info.watchers) == 0 {
						delete(wdb.watcherInfo, kind)
					}
				*/
				return
			}
		}
		err = errors.New("watcher not found in the list")
		return
	}

	err = errors.New("unknown kind")

}

func (wdb *dscWatcherDB) getDSCWatchers(kind string) []chan Event {
	if w, ok := wdb.watcherInfo[kind]; ok {
		return w.watchers
	}
	return nil
}

func (wdb *dscWatcherDB) clearWatchOptions(kind string) []FilterFn {
	if info, ok := wdb.watcherInfo[kind]; ok {
		if info.filterGroup == nil {
			return nil
		}

		oldFilterFuncs := info.filterGroup.filters
		_, err := wdb.md.delWFltGrp(kind, info.watchOptions, info.watchers)
		if err != nil {
			log.Errorf("delWFltGrp for kind: %s returned err: %s", kind, err)
		}
		info.filterGroup = nil
		info.watchOptions = api.ListWatchOptions{}
		return oldFilterFuncs
	}
	return nil
}

func (wdb *dscWatcherDB) getFilterFns(kind string) []FilterFn {
	if info, ok := wdb.watcherInfo[kind]; ok {
		if info.filterGroup == nil {
			return nil
		}
		return info.filterGroup.filters
	}
	return nil
}

func (md *Memdb) addDscWInfo(dsc, kind string, watcher chan Event) {
	md.wFilterDSCLock.Lock()
	defer md.wFilterDSCLock.Unlock()
	if dscInfo, ok := md.dscWatcherInfo[dsc]; ok {
		// dsc already exists
		dscInfo.addDSCWatcherInfo(kind, watcher)
		return
	}

	dscDB := md.newDSCWatchDB()
	dscDB.addDSCWatcherInfo(kind, watcher)
	md.dscWatcherInfo[dsc] = dscDB
}

// AddDSCWatcherInfoWatchOptions is called to add/update watch options for a DSC+Kind, is called when a topo event generated new watch options
func (md *Memdb) addDscWatchOptions(dsc, kind string, options api.ListWatchOptions) (old, new []FilterFn, err error) {
	md.wFilterDSCLock.RLock()
	if dscDB, ok := md.dscWatcherInfo[dsc]; ok {

		o, n, e := dscDB.addDSCWatcherInfoWatchOptions(kind, options)
		md.wFilterDSCLock.RUnlock()
		return o, n, e
	}
	md.wFilterDSCLock.RUnlock()
	md.addDscWInfo(dsc, kind, nil)

	return md.addDscWatchOptions(dsc, kind, options)
}

func (md *Memdb) delDSCInfo(dsc, kind string, watcher chan Event) {
	md.wFilterDSCLock.RLock()
	defer md.wFilterDSCLock.RUnlock()
	if dscDB, ok := md.dscWatcherInfo[dsc]; ok {
		dscDB.delDSCWatcherInfo(kind, watcher)
	}
}

func (md *Memdb) clearWatchOptions(dsc, kind string) []FilterFn {
	md.wFilterDSCLock.RLock()
	defer md.wFilterDSCLock.RUnlock()
	if dscDB, ok := md.dscWatcherInfo[dsc]; ok {
		return dscDB.clearWatchOptions(kind)
	}

	return nil
}

// SetWatchFilterFlags sets the watch filter behavior based on the featureflags
func (md *Memdb) SetWatchFilterFlags(flags map[string]uint) {
	md.filterFlags = flags
}

func (md *Memdb) newFilterSet() watchFiltersetInterface {
	return &watcherFilterSet{
		md:        md,
		filterGrp: make(map[string]*WFilterGroup),
	}
}

func getWatchOptionsStr(options api.ListWatchOptions) string {
	ret, err := json.Marshal(options)

	if err != nil {
		log.Error("getWatchOptionsStr, json marshal failed err: ", err)
		return ""
	}

	return string(ret)
}

func (md *Memdb) addWFltGrp(kind string, watchOptions api.ListWatchOptions, watchers []chan Event) (*WFilterGroup, error) {
	if grp, ok := md.filterGroups[kind]; ok {
		return grp.addWFilterGroup(kind, watchOptions, watchers)
	}

	fgrp := md.newFilterSet()
	md.filterGroups[kind] = fgrp
	return fgrp.addWFilterGroup(kind, watchOptions, watchers)
}

func (md *Memdb) delWFltGrp(kind string, watchOptions api.ListWatchOptions, watchers []chan Event) (*WFilterGroup, error) {
	if grp, ok := md.filterGroups[kind]; ok {
		return grp.delWatcherFromWFilterGroup(watchOptions, watchers)
	}
	return nil, errors.New("kind not registered yet")
}

// addWFilterGroup is used to add a new watch filter group for a kind, with watchOptions
// and an optional list of watcher channels. If a filter group for the kind already exists which matches
// the watchoptions, it updates the watcher list and return the existing group
// otherwise create a new group, generate the filter functions based on the group and add it to the mapa nd return
// the new group
func (fs *watcherFilterSet) addWFilterGroup(kind string, watchOptions api.ListWatchOptions, watchers []chan Event) (*WFilterGroup, error) {
	optsStr := getWatchOptionsStr(watchOptions)

	defer log.Infof("added filtergroup with options %s watchers %v", optsStr, watchers)
	fs.lock.Lock()
	defer fs.lock.Unlock()

	if grp, ok := fs.filterGrp[optsStr]; ok {
		// a group matching these watch options already exists
		if len(watchers) == 0 {
			//just return the existing group
			return grp, nil
		}

		// assuming this list of watchers doesn't exist in the list already
		grp.watchers = append(grp.watchers, watchers...)
		return grp, nil
	}

	grp := &WFilterGroup{
		watchOptions: watchOptions,
	}

	grp.watchers = append(grp.watchers, watchers...)
	grp.filters = fs.md.GetWatchFilter("netproto."+kind, &watchOptions)
	fs.filterGrp[optsStr] = grp
	return grp, nil
}

func (md *Memdb) removeWatcherFromList(fgrp *WFilterGroup, watcher chan Event) error {
	for i, w := range fgrp.watchers {
		if w == watcher {
			fgrp.watchers = append(fgrp.watchers[:i], fgrp.watchers[i+1:]...)
			return nil
		}
	}
	return errors.New("Watcher not found in the list")
}

// DelWatcherFromWFilterGroup deletes a watcher from the list of watchers of a kind with the given watchoptions
// if the list of watchers becomes 0, the group is deleted
func (fs *watcherFilterSet) delWatcherFromWFilterGroup(watchOptions api.ListWatchOptions, watchers []chan Event) (*WFilterGroup, error) {
	var err error
	fs.lock.Lock()
	defer fs.lock.Unlock()
	optsStr := getWatchOptionsStr(watchOptions)

	defer func() {
		if err == nil {
			log.Infof("Deleted watchers %v with options %s", watchers, optsStr)
		} else {
			log.Errorf("Delete watchers %v with options %s failed err: %s", watchers, optsStr, err)
		}
	}()

	if grp, ok := fs.filterGrp[optsStr]; ok {
		for _, w := range watchers {
			err = fs.md.removeWatcherFromList(grp, w)
			if err != nil {
				return grp, err
			}
		}
		/*
			if len(grp.watchers) == 0 {
				// no more watchers in the group, delete it
				delete(fs.filterGrp, optsStr)
				return nil, nil
			}
		*/
		return grp, nil
	}
	err = errors.New("Filter group doesn't exist")
	return nil, err
}

func (fs *watcherFilterSet) watchEvent(ev Event) {
	log.Infof("Received event: %+v | kind: %s", ev, ev.Obj.GetObjectKind())
	for _, grp := range fs.filterGrp {
		for _, flt := range grp.filters {
			if !flt(ev.Obj, nil) {
				break
			}

			for _, watcher := range grp.watchers {
				w := &Watcher{
					Channel: watcher,
				}
				log.Infof("Sending ev: %+v | kind: %s | watcher: %v", ev, ev.Obj.GetObjectKind(), watcher)
				sendToWatcher(ev, w)
			}
		}
	}
}

func (fs *watcherFilterSet) dump() string {
	ret := ""
	for key, grp := range fs.filterGrp {
		ret += fmt.Sprintf("    opts: %s, filters: %v, watchers: %v\n", key, grp.filters, grp.watchers)
	}
	return ret
}

func (md *Memdb) sendReconcileEvent(dsc, kind string, old, new []FilterFn) {
	type dummyObj struct {
		api.TypeMeta
		api.ObjectMeta
	}
	md.wFilterDSCLock.RLock()
	defer md.wFilterDSCLock.RUnlock()
	if dscDB, ok := md.dscWatcherInfo[dsc]; ok {
		watchers := dscDB.getDSCWatchers(kind)
		for _, w := range watchers {
			obj := &dummyObj{
				TypeMeta: api.TypeMeta{Kind: kind},
			}
			ev := Event{
				EventType: ReconcileEvent,
				Obj:       obj,
				OldFlts:   old,
				NewFlts:   new,
			}
			ww := &Watcher{
				Channel: w,
			}
			sendToWatcher(ev, ww)
		}
	}

}
