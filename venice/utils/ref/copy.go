package ref

import (
	"reflect"
)

// DeepCopy returns a copy of the input object.
func DeepCopy(obj interface{}) interface{} {
	old := reflect.ValueOf(obj)
	new := reflect.New(old.Type()).Elem()
	deepCopy(old, new)
	return new.Interface()
}

// deepCopy is a helper function to copy from old to new Values.
func deepCopy(old, new reflect.Value) {
	switch old.Kind() {
	case reflect.Struct:
		for ii := 0; ii < old.NumField(); ii++ {
			// Skip unexported fields
			if old.Type().Field(ii).PkgPath != "" {
				continue
			}
			deepCopy(old.Field(ii), new.Field(ii))
		}
	case reflect.Ptr:
		if old.IsNil() {
			return
		}
		new.Set(reflect.New(old.Elem().Type()))
		deepCopy(old.Elem(), new.Elem())
	case reflect.Map:
		if old.IsNil() {
			return
		}
		new.Set(reflect.MakeMap(old.Type()))
		for _, key := range old.MapKeys() {
			oValue := old.MapIndex(key)
			nValue := reflect.New(oValue.Type()).Elem()
			deepCopy(oValue, nValue)
			newKey := DeepCopy(key.Interface())
			new.SetMapIndex(reflect.ValueOf(newKey), nValue)
		}
	case reflect.Slice:
		if old.IsNil() {
			return
		}
		new.Set(reflect.MakeSlice(old.Type(), old.Len(), old.Cap()))
		for ii := 0; ii < old.Len(); ii++ {
			deepCopy(old.Index(ii), new.Index(ii))
		}
	default:
		new.Set(old)
	}
}
