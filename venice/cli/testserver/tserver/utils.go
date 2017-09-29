// (c) Pensando Systems, Inc.
// This is a generated file, please do not hand edit !!

package tserver

import (
	"context"
	"encoding/json"
	"fmt"
	"path"
	"strings"

	"github.com/pensando/sw/venice/cli/api"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
	uuid "github.com/satori/go.uuid"
)

// NameToUUID is
var NameToUUID = map[string]string{}

// UUIDToName is
var UUIDToName = map[string]string{}

func findNameByUUID(uuidStr string) (string, error) {
	if _, err := uuid.FromString(uuidStr); err != nil {
		return "", fmt.Errorf("UUID '%s' - invalid format", uuidStr)
	}

	if v, ok := UUIDToName[uuidStr]; ok {
		return v, nil
	}
	return "", fmt.Errorf("Unable to find uuid %s ", uuidStr)
}

func findUUIDByName(kind, name string) (string, error) {
	fullName := kind + "/" + name
	if v, ok := NameToUUID[fullName]; ok {
		return v, nil
	}

	return "", fmt.Errorf("Unable to find object %s/%s ", kind, name)
}

func saveNameUUID(kind, name, uuidStr string) {
	if v, ok := UUIDToName[uuidStr]; ok && v != name {
		delete(NameToUUID, kind+"/"+v)
	}
	fullName := kind + "/" + name
	NameToUUID[fullName] = uuidStr
	UUIDToName[uuidStr] = name
}

func clearNameUUID(kind, name, uuidStr string) {
	delete(NameToUUID, kind+"/"+name)
	delete(UUIDToName, uuidStr)
}

// FindObj is
func FindObj(kvs kvstore.Interface, kind, value string) error {
	if _, ok := api.Objs[kind]; !ok {
		return fmt.Errorf("Unrecognized kind '%s'", kind)
	}
	names := strings.Split(value, ",")

	for _, name := range names {
		uuidStr, err := findUUIDByName(kind, name)
		if err != nil {
			return err
		}
		key := path.Join(api.Objs[kind].URL, uuidStr)
		obj, _ := getEmptyObj(kind)

		runObj, ok := obj.(runtime.Object)
		if !ok {
			return fmt.Errorf("invalid kind %s", kind)
		}
		if err = kvs.Get(context.Background(), key, runObj); err != nil {
			if kvstore.IsKeyNotFoundError(err) {
				return fmt.Errorf("%s %q not found", kind, name)
			}
			return fmt.Errorf("%s '%s' get error: %v", kind, name, err)
		}
	}

	return nil
}

// FindObjs is
func FindObjs(kvs kvstore.Interface, kind string, names []string) error {
	for _, name := range names {
		if err := FindObj(kvs, kind, name); err != nil {
			return err
		}
	}
	return nil
}

// GetObjByName is
func GetObjByName(kind, objName string) (interface{}, error) {
	uuidStr, err := findUUIDByName(kind, objName)
	if err != nil {
		return nil, err
	}
	if _, ok := api.Objs[kind]; !ok {
		return nil, fmt.Errorf("Unrecognized kind '%s'", kind)
	}
	key := path.Join(api.Objs[kind].URL, uuidStr)
	obj, _ := getEmptyObj(kind)
	if obj == nil {
		return nil, fmt.Errorf("unknown kind/name '%s/%s'", kind, objName)
	}
	ro := obj.(runtime.Object)

	if err = kvStore.Get(context.Background(), key, ro); err != nil {
		return nil, err
	}
	return ro, nil
}

// GetObjsByLabel is
func GetObjsByLabel(kind, label string) ([]interface{}, error) {
	kvs := strings.Split(label, ":")
	if len(kvs) != 2 {
		return nil, fmt.Errorf("unable to parse label '%s'", label)
	}
	key := kvs[0]
	value := kvs[1]

	retList := []interface{}{}
	_, objList := getEmptyObj(kind)
	if retList == nil || objList == nil {
		return nil, fmt.Errorf("unknown kind '%s'", kind)
	}
	if err := kvStore.List(context.Background(), api.Objs[kind].URL, objList.(runtime.Object)); err != nil {
		return nil, fmt.Errorf("unable to fetch list for %ss: %s", kind, err)
	}

	hdrBytes, err := json.Marshal(objList)
	if err != nil {
		return nil, fmt.Errorf("Unable to find header in object (marshal): %+v", objList)
	}
	listHdr := api.ListHeader{}
	if err := json.Unmarshal(hdrBytes, &listHdr); err != nil {
		return nil, fmt.Errorf("Unable to find header in object (unmarshal): '%s'", string(hdrBytes))
	}

	if len(listHdr.Items) <= 0 {
		return retList, nil
	}

	for idx, hdrObj := range listHdr.Items {
		for k, v := range hdrObj.ObjectMeta.Labels {
			if key == k && value == v {
				obj := getObjFromList(objList, idx)
				retList = append(retList, obj)
			}
		}
	}

	return retList, nil
}
