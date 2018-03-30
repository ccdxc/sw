package runtime

import (
	"fmt"
	"reflect"

	"github.com/pensando/sw/api"
)

var (
	errNotAPIObject  = fmt.Errorf("Object is not an API Object")
	errNotListObject = fmt.Errorf("Object is not a List Object")
)

// GetObjectMeta returns the ObjectMeta if its an API object, error otherwise
func GetObjectMeta(obj interface{}) (*api.ObjectMeta, error) {
	if obj == nil {
		return nil, errNotAPIObject
	}
	switch t := obj.(type) {
	case ObjectMetaAccessor:
		if reflect.ValueOf(t) == reflect.Zero(reflect.TypeOf(t)) {
			return nil, errNotAPIObject
		}
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
