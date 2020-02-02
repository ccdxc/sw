package cfgen

import (
	"reflect"
)

// transform takes an object, goes over all the fields and performs substituitions as necessary
// the transformed structure of the same type is returned; it uses transformString to perform field level transformation
func (ctx *iterContext) transform(obj interface{}) interface{} {
	old := reflect.ValueOf(obj)
	new := reflect.New(old.Type()).Elem()
	ctx.transformRecursive(old, new)
	return new.Interface()
}

// transformRecursive is a helper function to copy from old to new Values.
func (ctx *iterContext) transformRecursive(old, new reflect.Value) {
	switch old.Kind() {
	case reflect.Struct:
		for ii := 0; ii < old.NumField(); ii++ {
			// Skip unexported fields
			if old.Type().Field(ii).PkgPath != "" {
				continue
			}
			ctx.transformRecursive(old.Field(ii), new.Field(ii))
		}
	case reflect.Ptr:
		if old.IsNil() {
			return
		}
		new.Set(reflect.New(old.Elem().Type()))
		ctx.transformRecursive(old.Elem(), new.Elem())
	case reflect.Map:
		if old.IsNil() {
			return
		}
		new.Set(reflect.MakeMapWithSize(old.Type(), old.Len()))
		for _, key := range old.MapKeys() {
			oValue := old.MapIndex(key)
			nValue := reflect.New(oValue.Type()).Elem()
			ctx.transformRecursive(oValue, nValue)
			newKey := ctx.transform(key.Interface())
			new.SetMapIndex(reflect.ValueOf(newKey), nValue)
		}
	case reflect.Slice:
		if old.IsNil() {
			return
		}
		new.Set(reflect.MakeSlice(old.Type(), old.Len(), old.Cap()))
		for ii := 0; ii < old.Len(); ii++ {
			ctx.transformRecursive(old.Index(ii), new.Index(ii))
		}
	case reflect.String:
		str := ctx.transformString(old.String())
		new.SetString(str)
	case reflect.Uint32, reflect.Uint64:
		newUint := ctx.transformUint(old.Uint())
		new.SetUint(newUint)
	default:
		new.Set(old)
	}
}
