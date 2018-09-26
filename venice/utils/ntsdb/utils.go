package ntsdb

import (
	"fmt"
	"reflect"

	"github.com/pensando/sw/venice/utils/runtime"
)

func getKeys(obj interface{}, keys map[string]string) (string, error) {
	o, ok := obj.(runtime.Object)
	if !ok {
		return "", fmt.Errorf("Not a runtime object")
	}

	m, err := runtime.GetObjectMeta(o)
	if err != nil {
		return "", err
	}
	k := o.GetObjectKind()
	keys["Tenant"] = m.Tenant
	keys["Namespace"] = m.Namespace
	keys["Name"] = m.Name
	keys["Kind"] = k

	return k, nil
}

func fillFields(t *iTable, m interface{}) error {
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
				c := t.Counter(field.Name)
				cv := reflect.ValueOf(c)
				v.Field(i).Set(cv)
			case "Gauge":
				g := t.Gauge(field.Name)
				gv := reflect.ValueOf(g)
				v.Field(i).Set(gv)
			case "Bool":
				b := t.Bool(field.Name)
				bv := reflect.ValueOf(b)
				v.Field(i).Set(bv)
			case "String":
				s := t.String(field.Name)
				sv := reflect.ValueOf(s)
				v.Field(i).Set(sv)
			case "Histogram":
				h := t.Histogram(field.Name)
				hv := reflect.ValueOf(h)
				v.Field(i).Set(hv)
			case "Summary":
				s := t.Summary(field.Name)
				sv := reflect.ValueOf(s)
				v.Field(i).Set(sv)
			default:
				return fmt.Errorf("Invalid metric type: %s", fieldType)
			}
		}
	}

	return nil
}
