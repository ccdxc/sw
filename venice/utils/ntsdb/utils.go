package ntsdb

import (
	"fmt"
	"reflect"

	"github.com/pensando/sw/venice/utils/ref"
)

func getKeys(obj interface{}, keys map[string]string) (string, error) {
	refCtx := &ref.RfCtx{GetSubObj: ref.NilSubObj}
	kvs := make(map[string]ref.FInfo)
	ref.GetKvs(obj, refCtx, kvs)

	keyNames := []string{"Tenant", "Namespace", "Kind", "Name"}
	for _, key := range keyNames {
		if v, ok := kvs[key]; ok {
			if v.ValueStr[0] != "" {
				keys[key] = v.ValueStr[0]
			}
		}
	}

	if len(keys) == 0 {
		return "", fmt.Errorf("keys not found")
	}
	if _, ok := keys["Kind"]; !ok {
		return "", fmt.Errorf("unable to find kind")
	}
	if _, ok := keys["Name"]; !ok {
		return "", fmt.Errorf("unable to find name")
	}

	tableName := keys["Kind"]
	return tableName, nil
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
