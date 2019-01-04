package ntsdb

import (
	"fmt"
	"reflect"
	"sort"

	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	keySeparator = "/"
)

// getKeys gets the keys for a Venice object
// an object is considered unique based on 4 tuple (tenant, namespace, kind, name)
func getKeys(o interface{}, keys map[string]string) (string, error) {
	obj, ok := o.(runtime.Object)
	if !ok {
		return "", fmt.Errorf("Not a runtime object")
	}

	m, err := runtime.GetObjectMeta(obj)
	if err != nil {
		return "", err
	}
	objKind := obj.GetObjectKind()
	keys["Tenant"] = m.Tenant
	keys["Namespace"] = m.Namespace
	keys["Name"] = m.Name
	keys["Kind"] = objKind

	tableName := objKind

	return tableName, nil
}

// getObjName creates a string based on the keys that can help identify
// the object uniquely
func getObjName(keys map[string]string) string {

	sortedKeys := func(m map[string]string) []string {
		mk := make([]string, len(m))
		i := 0
		for k := range m {
			mk[i] = k
			i++
		}
		sort.Strings(mk)
		return mk
	}(keys)

	objName := ""
	for _, key := range sortedKeys {
		objName += key + keySeparator + keys[key] + keySeparator
	}
	return objName
}

// fillFields takes a structure consisting of tsdb metric types and instantiate
// the values to those objects i.e. it does New() for all metric types in a structure
func fillFields(obj *iObj, m interface{}) error {
	v := reflect.ValueOf(m)
	if v.Kind() == reflect.Ptr {
		v = reflect.Indirect(v)
	}

	if v.Kind() == reflect.Struct {
		for i := 0; i < v.NumField(); i++ {
			field := v.Type().Field(i)
			fieldType := field.Type.Name()
			switch fieldType {
			case "Counter":
				c := obj.Counter(field.Name)
				cv := reflect.ValueOf(c)
				v.Field(i).Set(cv)
			case "Gauge":
				g := obj.Gauge(field.Name)
				gv := reflect.ValueOf(g)
				v.Field(i).Set(gv)
			case "PrecisionGauge":
				g := obj.PrecisionGauge(field.Name)
				gv := reflect.ValueOf(g)
				v.Field(i).Set(gv)
			case "Bool":
				b := obj.Bool(field.Name)
				bv := reflect.ValueOf(b)
				v.Field(i).Set(bv)
			case "String":
				s := obj.String(field.Name)
				sv := reflect.ValueOf(s)
				v.Field(i).Set(sv)
			case "Histogram":
				h := obj.Histogram(field.Name)
				hv := reflect.ValueOf(h)
				v.Field(i).Set(hv)
			case "Summary":
				s := obj.Summary(field.Name)
				sv := reflect.ValueOf(s)
				v.Field(i).Set(sv)
			default:
				return fmt.Errorf("Invalid metric type: %s", fieldType)
			}
		}
	}

	return nil
}
