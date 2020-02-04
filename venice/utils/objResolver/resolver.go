package objResolver

import (
	"errors"
	"strings"
	"sync"

	"github.com/pensando/sw/api/graph"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

//ObjectResolver is interface to do resolution on the controller DB
type opResolver interface {
	//Check object resolved
	resolvedCheck(key string) bool
	//trigger recursive dep check once resolved
	trigger(obj apiintf.CtkitObject) ([]apiintf.CtkitEvent, error)
}

//ObjectResolver is interface to do resolution on the controller DB
type ObjectResolver struct {
	sync.Mutex
	objGraph       graph.Interface
	md             apiintf.ObjectDB
	addResolver    opResolver
	deleteResolver opResolver
}

type resolverBase struct {
	objGraph    graph.Interface
	md          apiintf.ObjectDB
	objResolver *ObjectResolver
}

type addResolver struct {
	resolverBase
}

type deleteResolver struct {
	resolverBase
}

func (r *resolverBase) clearReferences(key string) {
	node := graph.Node{
		This: key,
		Refs: make(map[string][]string),
	}
	r.objGraph.UpdateNode(&node)
}

//NewObjectResolver create a new interface for object resolver
func NewObjectResolver(md apiintf.ObjectDB) *ObjectResolver {
	resolver := &ObjectResolver{md: md}
	resolver.objGraph, _ = graph.NewCayleyStore()
	resolver.addResolver = &addResolver{resolverBase{md: md, objGraph: resolver.objGraph,
		objResolver: resolver}}
	resolver.deleteResolver = &deleteResolver{resolverBase{md: md, objGraph: resolver.objGraph,
		objResolver: resolver}}
	return resolver
}

//get the key for relation in graphdb
//constructed for source kind , desitnation kind and actual field
func getKeyForGraphDB(skind, dkind, key string) string {
	return skind + "-" + dkind + "-" + key
}

func getSKindDKindFieldKey(key string) (string, string, string) {
	parts := strings.Split(key, "-")
	return parts[0], parts[1], parts[2]
}

func (r *addResolver) resolvedCheck(key string) bool {
	node := r.objGraph.References(key)
	if node == nil {
		//This node had no references, hence resolved.
		return true
	}
	refObjsResolved := true
	for key, refs := range node.Refs {
		_, dKind, _ := getSKindDKindFieldKey(key)
		objDB := r.md.GetObjectStore(dKind)
		for _, ref := range refs {
			refObj, err := objDB.GetObject(getRefKey(ref))
			//Referred Object not present or still not resolved.
			if err != nil {
				refObjsResolved = false
			} else {
				refObj.Lock()
				refObjsResolved = refObj.IsResolved()
				refObj.Unlock()
			}
			if !refObjsResolved {
				break
			}
		}
		if !refObjsResolved {
			break
		}
	}
	return refObjsResolved
}

func getRefKey(ref string) string {
	return ref
}

func (r *deleteResolver) resolvedCheck(key string) bool {

	node := r.objGraph.Referrers(key)
	if node == nil {
		//This node had no referrers, hence can be deleted.
		return true
	}
	refObjsPresent := false
	for key, refs := range node.Refs {
		sKind, _, _ := getSKindDKindFieldKey(key)
		objDB := r.md.GetObjectStore(sKind)
		for _, ref := range refs {
			_, err := objDB.GetObject(getRefKey(ref))
			//Referring Object still present
			if err == nil {
				refObjsPresent = true
				break
			}
		}
		if refObjsPresent {
			break
		}
	}
	return !refObjsPresent
}

type transitQueue struct {
	//key string
	apiintf.CtkitObject
}

func (r *addResolver) trigger(obj apiintf.CtkitObject) ([]apiintf.CtkitEvent, error) {

	inFlightObjects := []apiintf.CtkitObject{obj}
	pendingObjects := []apiintf.CtkitEvent{}

	//Get all the Pendoing Events of this object first
	for _, pobj := range obj.PendingEvents() {
		pendingObjects = append(pendingObjects, pobj)
	}
	obj.ClearPendingEvents()

	processReferencesInternal := func(obj apiintf.CtkitObject,
		pendingObjs []apiintf.CtkitEvent) ([]apiintf.CtkitEvent, error) {
		node := r.objGraph.Referrers(obj.GetKey())
		if node == nil {
			//This node had no referrers, so nothing to process.
			return pendingObjs, nil
		}
		for key, referrers := range node.Refs {
			skind, _, _ := getSKindDKindFieldKey(key)
			objDB := r.md.GetObjectStore(skind)
		L:
			for _, referrer := range referrers {
				referrerObj, err := objDB.GetObject(getRefKey(referrer))
				//referrerObj, ok := objDB.objects[referrer]
				if err == nil {
					referrerObj.Lock()
					if !referrerObj.IsInProgress() && !referrerObj.IsResolved() {
						inFlightObjects = append(inFlightObjects, referrerObj)
						if r.resolvedCheck(referrerObj.GetKey()) {
							log.Infof("Object key %v resolved\n", referrerObj.GetKey())
							if referrerObj.IsUpdateUnResolved() {
								log.Infof("Update object %v %p\n", referrerObj.GetKey(), referrerObj)
								referrerObj.SetEvent(kvstore.Updated)
							} else if referrerObj.IsAddUnResolved() {
								referrerObj.SetEvent(kvstore.Created)
							} else {
								log.Infof("Object key %v resolved, but ignoreing\n", referrerObj.GetKey())
								//Contiue as this is not done yet.
								referrerObj.Unlock()
								break L
							}
							//Mark object as in progress to queue any event behind this
							referrerObj.SetInProgress()
							log.Infof("Add/update Object key %v unresolved by %v", referrerObj.GetKey(), obj.GetKey())
							r.md.ResolvedRun(referrerObj)
							//Copy all pending events associated to this object to run again
							for _, pobj := range referrerObj.PendingEvents() {
								pendingObjs = append(pendingObjs, pobj)
							}
							referrerObj.ClearPendingEvents()
						} else {
							//Break out as this is still not resolved.
							referrerObj.Unlock()
							break L
						}
					}
					referrerObj.Unlock()
				}
			}
		}

		return pendingObjs, nil
	}

	for true {
		currLen := len(inFlightObjects)
		if currLen == 0 {
			break
		}
		var err error
		for _, inFlightObj := range inFlightObjects {
			pendingObjects, err = processReferencesInternal(inFlightObj, pendingObjects)
			if err != nil {
				log.Errorf("Error in processsing references %v", err)
			}
		}
		inFlightObjects = inFlightObjects[currLen:]
	}

	return pendingObjects, nil
}

func (r *deleteResolver) trigger(obj apiintf.CtkitObject) ([]apiintf.CtkitEvent, error) {

	inFlightObjects := []apiintf.CtkitObject{obj}
	pendingObjects := []apiintf.CtkitEvent{}

	defer r.clearReferences(obj.GetKey())
	//Get all the Pendoing Events of this object first
	//for _, pobj := range obj.PendingEvents() {
	//	pendingObjects = append(pendingObjects, pobj)
	//}

	processReferencesInternal := func(obj apiintf.CtkitObject,
		pendingObjs []apiintf.CtkitEvent) ([]apiintf.CtkitEvent, error) {
		node := r.objGraph.References(obj.GetKey())
		thisObjPendingEvents := []apiintf.CtkitEvent{}
		for _, pobj := range obj.PendingEvents() {
			thisObjPendingEvents = append(thisObjPendingEvents, pobj)
		}
		obj.ClearPendingEvents()
		if node == nil {
			//This node had no referrers, so nothing to process.
			log.Infof("Object %v has no referrers", obj.GetKey())
			for _, pobj := range thisObjPendingEvents {
				pendingObjs = append(pendingObjs, pobj)
			}
			return pendingObjs, nil
		}
		for key, references := range node.Refs {
			_, dKind, _ := getSKindDKindFieldKey(key)
			objDB := r.md.GetObjectStore(dKind)
		L:
			for _, reference := range references {
				referenceObj, err := objDB.GetObject(getRefKey(reference))
				if err == nil {
					referenceObj.Lock()
					if !referenceObj.IsInProgress() && referenceObj.IsDelUnResolved() {
						if r.resolvedCheck(referenceObj.GetKey()) {
							//Put the deleted node to run as next loop
							inFlightObjects = append(inFlightObjects, referenceObj)
							referenceObj.SetEvent(kvstore.Deleted)
							referenceObj.SetInProgress()
							log.Infof("Delete Object key %v resolved by %v", referenceObj.GetKey(), obj.GetKey())
							r.md.ResolvedRun(referenceObj)
							for _, pobj := range referenceObj.PendingEvents() {
								pendingObjs = append(pendingObjs, pobj)
							}
							referenceObj.ClearPendingEvents()
						} else {
							log.Infof("Object key %v unresolved (delete) by %v, refs %v",
								referenceObj.GetKey(), obj.GetKey(), r.objGraph.Referrers(referenceObj.GetKey()))
							//Break out as this is still not resolved.
							//Add pending events  as we can't create until dep is done
							for _, pobj := range thisObjPendingEvents {
								log.Infof("Adding second pendong %v %v behind %v ", pobj.Event, pobj.Obj.GetKey(), referenceObj.GetKey())
								referenceObj.AddToPending(pobj)
							}
							thisObjPendingEvents = []apiintf.CtkitEvent{}
							referenceObj.Unlock()
							break L
						}
					}
					referenceObj.Unlock()
				}
			}
		}
		//If pending events were not added, add now
		for _, pobj := range thisObjPendingEvents {
			pendingObjs = append(pendingObjs, pobj)
		}
		return pendingObjs, nil
	}

	for true {
		currLen := len(inFlightObjects)
		if currLen == 0 {
			break
		}
		var err error
		for _, inFlight := range inFlightObjects {
			pendingObjects, err = processReferencesInternal(inFlight, pendingObjects)
			if err != nil {
				log.Errorf("Error in processsing references %v", err)
			}
		}
		inFlightObjects = inFlightObjects[currLen:]
	}

	return pendingObjects, nil
}

func (resolver *ObjectResolver) updateReferences(obj apiintf.CtkitObject) {

	if obj.References() == nil {
		return
	}
	node := graph.Node{
		This: obj.GetKey(),
		Refs: make(map[string][]string),
	}

	for field, refs := range obj.References() {
		objKey := getKeyForGraphDB(obj.GetKind(), refs.RefKind, field)
		node.Refs[objKey] = []string{}
		for _, ref := range refs.Refs {
			node.Refs[objKey] = append(node.Refs[objKey], ref)
		}
	}

	resolver.objGraph.UpdateNode(&node)
}

//ProcessAdd process add for the resolver
func (resolver *ObjectResolver) ProcessAdd(obj apiintf.CtkitObject) error {

	resolver.Lock()

	err := resolver.processAddInternal(obj)
	if err != nil {
		log.Errorf("Error running Process Add %v\n", err)
	}

	resolver.Unlock()

	return nil
}

//ProcessAdd process add for the resolver
func (resolver *ObjectResolver) processAddInternal(obj apiintf.CtkitObject) error {

	objectDB := resolver.md.GetObjectStore(obj.GetKind())
	if objectDB == nil {
		return errors.New("Object DB not found")
	}

	// if we have the object, make this an update
	_, err := objectDB.GetObject(obj.GetKey())
	if err == nil {
		return resolver.processUpdateInternal(obj)
	}

	objectDB.AddObject(obj)
	obj.Lock()

	obj.SetInProgress()
	resolver.updateReferences(obj)

	if resolver.addResolver.resolvedCheck(obj.GetKey()) {
		obj.SetEvent(kvstore.Created)
		obj.Unlock()
		//Send the object to consumer
		log.Infof("Add Object key %v resolved", obj.GetKey())
		resolver.md.ResolvedRun(obj)
	} else {
		//Don't Send the object to consumer as it is still not resolved
		obj.SetAddUnResolved()
		obj.Unlock()
		log.Infof("Add Object key %v unresolved, refs %v", obj.GetKey(), obj.References())
	}

	return nil
}

//ProcessUpdate process update for the resolver
func (resolver *ObjectResolver) ProcessUpdate(obj apiintf.CtkitObject) error {

	resolver.Lock()

	err := resolver.processUpdateInternal(obj)
	if err != nil {
		log.Errorf("Error running Process Add %v\n", err)
	}

	resolver.Unlock()

	return nil
}

//ProcessUpdate process update for the resolver
func (resolver *ObjectResolver) processUpdateInternal(obj apiintf.CtkitObject) error {

	event := kvstore.Updated
	objectDB := resolver.md.GetObjectStore(obj.GetKind())
	if objectDB == nil {
		return errors.New("Object DB not found")
	}

recheck:
	// if we have the object, make this an update
	existingObj, err := objectDB.GetObject(obj.GetKey())
	if err != nil {
		objectDB.AddObject(obj)
		obj.Lock()
		existingObj = obj
		event = kvstore.Created
	} else {
		existingObj.Lock()
		//Check if it is deleted.
		if !existingObj.IsDeleted() {
			if existingObj.IsOperationPending() {
				log.Infof("Adding %v to pending", existingObj.GetKey())
				existingObj.AddToPending(apiintf.CtkitEvent{Obj: obj, Event: kvstore.Updated})
				existingObj.Unlock()
				return nil
			}
		} else {
			//recheck as new object could be added
			goto recheck
		}
	}

	if existingObj.IsAddUnResolved() {
		//It was not resolved before, hence set it to create now
		//change even to create event as we never sent the object
		event = kvstore.Created
	}
	existingObj.SetInProgress()
	resolver.updateReferences(obj)

	if resolver.addResolver.resolvedCheck(existingObj.GetKey()) {
		if event == kvstore.Updated {
			existingObj.SetNewObj(obj)
		}
		existingObj.SetEvent(event)
		existingObj.Unlock()
		//Send the object to the consumer
		log.Infof("Update Object key %v resolved", existingObj.GetKey())
		resolver.md.ResolvedRun(existingObj)
	} else {
		//Don't Send the object to the consumer
		log.Infof("Update Object key %v unresolved", existingObj.GetKey())
		if event == kvstore.Updated {
			existingObj.SetUpdateUnResolved()
		} else {
			existingObj.SetAddUnResolved()
		}
		if event == kvstore.Updated {
			existingObj.SetNewObj(obj)
		}
		existingObj.Unlock()
	}
	return nil
}

//ProcessDelete process delete for the resolver
func (resolver *ObjectResolver) ProcessDelete(obj apiintf.CtkitObject) error {

	resolver.Lock()

	err := resolver.processDeleteInternal(obj)
	if err != nil {
		log.Errorf("Error running Process Add %v\n", err)
	}

	resolver.Unlock()

	return nil
}

//ProcessDelete process delete for the resolver
func (resolver *ObjectResolver) processDeleteInternal(obj apiintf.CtkitObject) error {

	objectDB := resolver.md.GetObjectStore(obj.GetKind())
	if objectDB == nil {
		return errors.New("Object DB not found")
	}

	// if we have the object, make this an update
	existingObj, err := objectDB.GetObject(obj.GetKey())
	if err != nil {
		log.Errorf("Object {%+v} not found", obj.GetKey())
		return errors.New("Object not found")
	}

	existingObj.Lock()

	// add it to db and send out watch notification
	if !existingObj.IsResolved() {
		existingObj.AddToPending(apiintf.CtkitEvent{Obj: obj, Event: kvstore.Deleted})
		existingObj.Unlock()
		return nil
	}
	existingObj.SetMarkedForDelete()

	if resolver.deleteResolver.resolvedCheck(obj.GetKey()) {
		// add it to db and send out watch notification
		existingObj.SetEvent(kvstore.Deleted)
		existingObj.SetMarkedForDelete()
		existingObj.Unlock()
		log.Infof("Delete Object key %v resolved", obj.GetKey())
		defer resolver.md.ResolvedRun(existingObj)
	} else {
		log.Infof("Delete Object key %v unresolved", obj.GetKey())
		existingObj.SetDeleteUnResolved()
		existingObj.Unlock()
	}

	return nil
}

//Resolve the object once the event has been processed downstream.
func (resolver *ObjectResolver) Resolve(event kvstore.WatchEventType, obj apiintf.CtkitObject) error {
	resolver.Lock()
	pendingObjects := []apiintf.CtkitEvent{}

	log.Infof("Resolving object %v %v\n", event, obj.GetKey())
	objectDB := resolver.md.GetObjectStore(obj.GetKind())
	if objectDB == nil {
		resolver.Unlock()
		return errors.New("Object DB not found")
	}

	// if we have the object, make this an update
	existingObj, err := objectDB.GetObject(obj.GetKey())
	if err != nil {
		resolver.Unlock()
		log.Errorf("Object {%+v} not found", obj.GetKey())
		return errors.New("Object not found")
	}

	existingObj.Lock()

	switch event {
	case kvstore.Created:
		existingObj.SetResolved()
		existingObj.Unlock()
		pendingObjects, _ = resolver.addResolver.trigger(existingObj)
	case kvstore.Updated:
		newObj := existingObj.GetNewObj()
		if newObj != nil {
			existingObj.Copy(newObj)
		}
		existingObj.SetResolved()
		existingObj.Unlock()
		pendingObjects, _ = resolver.addResolver.trigger(existingObj)
	case kvstore.Deleted:
		objectDB.DeleteObject(existingObj.GetKey())
		existingObj.SetDeleted()
		existingObj.Unlock()
		pendingObjects, _ = resolver.deleteResolver.trigger(existingObj)
	}
	log.Infof("Resolved object %v %v\n", event, obj.GetKey())

	//Process all pending objects withr resolver lock
	if pendingObjects != nil {
		for _, pobj := range pendingObjects {
			//When creates is resolved, only delete events will be queued
			if pobj.Event == kvstore.Deleted {
				log.Infof("Process update of key %v", pobj.Obj.GetKey())
				resolver.processDeleteInternal(pobj.Obj)
			} else if pobj.Event == kvstore.Updated {
				log.Infof("Process update of key %v", pobj.Obj.GetKey())
				resolver.processUpdateInternal(pobj.Obj)
			} else if pobj.Event == kvstore.Created {
				log.Infof("Process create of key %v", pobj.Obj.GetKey())
				resolver.processAddInternal(pobj.Obj)
			}
		}
	}

	resolver.Unlock()

	return nil
}
