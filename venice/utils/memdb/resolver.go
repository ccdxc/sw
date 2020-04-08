package memdb

import (
	"github.com/pensando/sw/api/graph"
	"github.com/pensando/sw/venice/utils/log"
)

type resolver interface {
	//Check object resolved
	resolvedCheck(key string, obj Object) bool
	//trigger recursive dep check once resolved
	trigger(key string, obj Object) error
}

type resolverBase struct {
	md *Memdb
	ev EventType
}

type addResolver struct {
	resolverBase
}

type deleteResolver struct {
	resolverBase
}

func newAddResolver(md *Memdb) resolver {
	return &addResolver{resolverBase{md: md, ev: CreateEvent}}
}

func (r *addResolver) getReferences(key string) *graph.Node {
	return r.md.objGraph.References(key)
}

func (r *addResolver) getReferenceKind(key string) string {
	_, dKind, _ := getSKindDKindFieldKey(key)
	return dKind
}

func (r *addResolver) resolvedCheck(key string, obj Object) bool {
	node := r.md.objGraph.References(key)
	if node == nil {
		//This node had no references, hence resolved.
		return true
	}
	refObjsResolved := true
	for key, refs := range node.Refs {
		_, dKind, _ := getSKindDKindFieldKey(key)
		objDB := r.md.getObjectDB(memDbKind(dKind))
		objDB.Lock()
		for _, ref := range refs {
			refObj := objDB.getObject(getRefKey(ref))
			//Referred Object not present or still not resolved.
			if refObj == nil {
				refObjsResolved = false
				log.Infof("found unresolved reference while for [%v][%v][%v]", key, dKind, ref)
			} else {
				refObj.Lock()
				refObjsResolved = refObj.isResolved()
				refObj.Unlock()
			}
			if !refObjsResolved {
				break
			}
		}
		objDB.Unlock()
		if !refObjsResolved {
			break
		}
	}
	return refObjsResolved
}

func (r *addResolver) getEvent() EventType {
	return r.ev
}

func newDeleteResolver(md *Memdb) resolver {
	return &deleteResolver{resolverBase{md: md, ev: DeleteEvent}}
}

func getRefKey(ref string) string {
	return ref
}

func (r *deleteResolver) resolvedCheck(key string, obj Object) bool {

	node := r.md.objGraph.Referrers(key)
	if node == nil {
		//This node had no referrers, hence can be deleted.
		return true
	}
	refObjsPresent := false
	for key, refs := range node.Refs {
		sKind, _, _ := getSKindDKindFieldKey(key)
		objDB := r.md.getObjectDB(sKind)
		objDB.Lock()
		for _, ref := range refs {
			refObj := objDB.getObject(getRefKey(ref))
			//Referring Object still present
			if refObj != nil {
				refObjsPresent = true
				break
			}

		}
		objDB.Unlock()
		if refObjsPresent {
			break
		}
	}
	return !refObjsPresent
}

type transitQueue struct {
	key string
	obj Object
}

func (r *addResolver) trigger(key string, obj Object) error {

	inFlightObjects := []transitQueue{transitQueue{key: key, obj: obj}}
	pendingObjects := []Event{}

	processReferencesInternal := func(key string, obj Object, pendingObjs []Event) ([]Event, error) {
		node := r.md.objGraph.Referrers(key)
		if node == nil {
			//This node had no referrers, so nothing to process.
			return pendingObjs, nil
		}
		for key, referrers := range node.Refs {
			skind, _, _ := getSKindDKindFieldKey(key)
			objDB := r.md.getObjectDB(skind)
			objDB.Lock()
		L:
			for _, referrer := range referrers {
				referrerObj := objDB.getObject(getRefKey(referrer))
				//referrerObj, ok := objDB.objects[referrer]
				if referrerObj != nil {
					referrerObj.Lock()
					if !referrerObj.isResolved() {
						inFlightObjects = append(inFlightObjects, transitQueue{key: referrerObj.Key(),
							obj: referrerObj.Object()})
						if r.resolvedCheck(referrerObj.Key(), referrerObj.Object()) {
							log.Infof("Object key %v resolved\n", referrerObj.Key())
							if referrerObj.isUpdateUnResolved() {
								r.md.topoHandler.handleUpdateEvent(nil, referrerObj.Object())
								objDB.watchEvent(r.md, referrerObj, UpdateEvent)
							} else {
								r.md.topoHandler.handleAddEvent(referrerObj.Object())
								objDB.watchEvent(r.md, referrerObj, CreateEvent)
							}
							referrerObj.resolved()
							for _, pobj := range referrerObj.getAndClearPending() {
								pendingObjs = append(pendingObjs, pobj)
							}
						} else {
							//Break out as this is still not resolved.
							referrerObj.Unlock()
							break L
						}
					}
					referrerObj.Unlock()
				}
			}
			objDB.Unlock()
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
			pendingObjects, err = processReferencesInternal(inFlight.key, inFlight.obj, pendingObjects)
			if err != nil {
				log.Errorf("Error in processsing references %v", err)
			}
		}
		inFlightObjects = inFlightObjects[currLen:]
	}
	for _, pobj := range pendingObjects {
		//When creates is resolved, only delete events will be queued
		if pobj.EventType == DeleteEvent {
			r.md.DeleteObjectWithReferences(pobj.key, pobj.Obj, pobj.refs)
		}
	}

	return nil
}

func (r *deleteResolver) trigger(key string, obj Object) error {

	inFlightObjects := []transitQueue{transitQueue{key: key, obj: obj}}
	pendingObjects := []Event{}

	processReferencesInternal := func(key string, obj Object, pendingObjs []Event) ([]Event, error) {
		node := r.md.objGraph.References(key)
		if node == nil {
			//This node had no referrers, so nothing to process.
			return pendingObjs, nil
		}
		for key, references := range node.Refs {
			_, dKind, _ := getSKindDKindFieldKey(key)
			objDB := r.md.getObjectDB(dKind)
			objDB.Lock()
		L:
			for _, reference := range references {
				referenceObj := objDB.getObject(getRefKey(reference))
				if referenceObj != nil {
					referenceObj.Lock()
					if referenceObj.isDelUnResolved() {
						if r.resolvedCheck(referenceObj.Key(), referenceObj.Object()) {
							//Put the deleted node to run as next loop
							inFlightObjects = append(inFlightObjects, transitQueue{key: referenceObj.Key(),
								obj: referenceObj.Object()})
							log.Infof("Object key %v resolved\n", referenceObj.Key())
							r.md.topoHandler.handleDeleteEvent(referenceObj.Object())
							objDB.deleteObject(getRefKey(reference))
							objDB.watchEvent(r.md, referenceObj, DeleteEvent)
							for _, pobj := range referenceObj.getAndClearPending() {
								pendingObjs = append(pendingObjs, pobj)
							}
						} else {
							//Break out as this is still not resolved.
							referenceObj.Unlock()
							break L
						}
					}
					referenceObj.Unlock()
				}
			}
			objDB.Unlock()
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
			pendingObjects, err = processReferencesInternal(inFlight.key, inFlight.obj, pendingObjects)
			if err != nil {
				log.Errorf("Error in processsing references %v", err)
			}
			//Remove all references of that node.
			r.md.clearReferences(inFlight.key)
		}
		inFlightObjects = inFlightObjects[currLen:]
	}

	//Process all pending ones which were waiting for object
	for _, pobj := range pendingObjects {
		//When objected is deleted, only create/update events will be queued
		if pobj.EventType == CreateEvent {
			r.md.AddObjectWithReferences(pobj.key, pobj.Obj, pobj.refs)
		} else if pobj.EventType == UpdateEvent {
			err := r.md.UpdateObjectWithReferences(pobj.key, pobj.Obj, pobj.refs)
			if err == errObjNotFound {
				log.Errorf("Ignore object update as it i not found")
			}
		}
	}

	return nil
}
