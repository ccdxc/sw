package runtime

import (
	"fmt"

	"github.com/pensando/sw/api"
)

var (
	errNotAPIObject  = fmt.Errorf("Object is not an API Object")
	errNotListObject = fmt.Errorf("Object is not a List Object")
)

// GetObjectMeta returns the ObjectMeta if its an API object, error otherwise
func GetObjectMeta(obj interface{}) (*api.ObjectMeta, error) {
	switch t := obj.(type) {
	case ObjectMetaAccessor:
		if meta := t.GetObjectMeta(); meta != nil {
			return meta, nil
		}
	}
	return nil, errNotAPIObject
}

// GetListMeta returns the ListMeta if its a list object, error otherwise
func GetListMeta(obj interface{}) (*api.ListMeta, error) {
	switch t := obj.(type) {
	case ListMetaAccessor:
		if meta := t.GetListMeta(); meta != nil {
			return meta, nil
		}
	}
	return nil, errNotListObject
}
