package vcli

import (
	"fmt"
	"reflect"
	"strconv"
	"strings"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	kvSplitter = "="
)

// get all fields of an object in kvstore
func getKvsValue(path, mapKeyPath string, obj reflect.Value, kvs map[string]cliField) {
	switch obj.Kind() {
	case reflect.Ptr:
		getKvsValue(path, mapKeyPath, reflect.Indirect(obj), kvs)
	case reflect.Struct:
		for i := 0; i < obj.NumField(); i++ {
			typeField := obj.Type().Field(i)
			elem := obj.Field(i)
			getKvsValue(path+typeField.Name+".", mapKeyPath, elem, kvs)
		}
	case reflect.Map:
		path = strings.TrimSuffix(path, ".") + "[]."
		for _, mapKey := range obj.MapKeys() {
			mapValue := obj.MapIndex(mapKey)
			getKvsValue(path, fmt.Sprintf("%s%s%s", mapKeyPath, mapKey, kvSplitter), mapValue, kvs)
		}
	case reflect.Slice:
		path = strings.TrimSuffix(path, ".") + "[]."
		path = path + "[]."
		for i := 0; i < obj.Len(); i++ {
			getKvsValue(path, mapKeyPath, obj.Index(i), kvs)
		}
	case reflect.Bool, reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64, reflect.Uint, reflect.Uint8,
		reflect.Uint16, reflect.Uint32, reflect.Uint64, reflect.Float32, reflect.Float64, reflect.String:
		path = strings.TrimSuffix(path, ".")
		objString := fmt.Sprintf("%v", obj)
		if objString == "" || objString == "0" {
			return
		}
		newString := mapKeyPath + objString
		if _, ok := kvs[path]; ok {
			newValues := append(kvs[path].values, newString)
			kvs[path] = cliField{values: newValues}
		} else {
			kvs[path] = cliField{values: []string{newString}}
		}
	default:
		panic(fmt.Sprintf("unsupported kind %s", obj.Kind()))

	}
}

// printgs json style struct definition of a command
func walkStruct(si *api.Struct, level int) string {
	levelSpaces := func(level int) string {
		levelSpaces := ""
		for i := 0; i <= level; i++ {
			levelSpaces += "    "
		}
		return levelSpaces
	}

	getPrintTag := func(af api.Field) string {
		if af.JSONTag != "" {
			return af.JSONTag
		}
		return af.Name
	}
	getPrintType := func(afType string) string {
		return strings.ToLower(strings.TrimPrefix(afType, "TYPE_"))
	}

	if si == nil {
		return ""
	}

	out := "{\n"
	for _, af := range si.Fields {
		if af.Inline {
			continue
		}
		if runtime.IsScalar(af.Type) {
			out += fmt.Sprintf("%s%s %s", levelSpaces(level), getPrintTag(af), getPrintType(af.Type))
		} else {
			out += fmt.Sprintf("%s%s ", levelSpaces(level), getPrintTag(af))
			if af.Slice {
				out += "[]"
			} else if af.Map {
				out += fmt.Sprintf("map[%s]", getPrintType(af.KeyType))
			}

			rs := runtime.GetDefaultScheme().GetSchema(af.Type)
			out += walkStruct(rs, level+1)
		}
		out += "\n"
	}
	out += fmt.Sprintf("%s}", levelSpaces(level-1))
	return out
}

// writes a given set of values into a fields alongside specified path, recurse if this is not terminating field
func writeAny(obj reflect.Value, af *api.Field, path string, values []string) error {
	switch obj.Kind() {
	case reflect.Ptr:
		return writeAny(reflect.Indirect(obj), af, path, values)
	case reflect.Struct:
		rs := runtime.GetDefaultScheme().GetSchema(af.Type)
		return writeStruct(obj, rs, path, values)
	case reflect.Map:
		return writeMap(obj, af, path, values)
	case reflect.Slice:
		return writeSlice(obj, af, path, values)
	default:
		if !runtime.IsScalar(af.Type) {
			return fmt.Errorf("unsupported kind: %s", obj.Kind())
		}

		if len(values) > 1 {
			return fmt.Errorf("invalid number of values for primitive type %d", len(values))
		}

		fieldValue := getPrimitive(af.Type, values[0])
		if !obj.CanSet() {
			return fmt.Errorf("can't set obj %+v, af %+v", obj, af)
		}
		obj.Set(fieldValue)
	}

	return nil
}

// parses the values to look for map fields and tries to set mapped value
// values are expected to be of the form key:value (nested maps are specified using separators)
func writeMap(obj reflect.Value, af *api.Field, path string, values []string) error {
	if !af.Map {
		return fmt.Errorf("obj map but rs not map: obj %+v rf %+v", obj, af)
	}

	for _, value := range values {
		kv := strings.Split(value, kvSplitter)
		if len(kv) < 2 {
			return fmt.Errorf("invalid map '%s', should be 'key%s%s'", value, kvSplitter, value)
		}
		keyValue := getPrimitive(af.KeyType, kv[0])
		if obj.IsNil() {
			newMap := reflect.MakeMap(obj.Type())
			obj.Set(newMap)
		}
		oldValue := obj.MapIndex(keyValue)
		newValue := reflect.Indirect(reflect.New(getType(af.Type)))
		if oldValue.IsValid() {
			copyValue(newValue, oldValue)
		}
		if err := writeAny(newValue, af, path, []string{kv[1]}); err != nil {
			return fmt.Errorf("error filling map: key %+v value %+v err '%s'", keyValue, newValue, err)
		}
		obj.SetMapIndex(keyValue, newValue)
	}

	return nil
}

// parses a slice of any type (including basic types) and write the values into the supplied object's value
func writeSlice(obj reflect.Value, af *api.Field, path string, values []string) error {
	if !af.Slice {
		return fmt.Errorf("obj slice but rs not slice: obj %+v rf %+v", obj, af)
	}

	if obj.IsNil() {
		newSlice := reflect.MakeSlice(obj.Type(), 0, 0)
		obj.Set(newSlice)
	}

	// special handling for nested arrays, or maps in arrays, or arrays in maps
	leafSlice := strings.Count(path, "[]") > 0
	for idx, value := range values {
		var newValue reflect.Value

		objCreated := false
		if obj.Len() <= idx {
			objCreated = true

			newValue = reflect.New(getType(af.Type))
			if runtime.IsScalar(af.Type) || !af.Pointer {
				newValue = reflect.Indirect(newValue)
			}
		} else {
			newValue = obj.Index(idx)
		}

		newValues := []string{value}
		if leafSlice {
			newValues = values
		}
		if err := writeAny(newValue, af, path, newValues); err != nil {
			return fmt.Errorf("error filling slice: value %+v err '%s'", newValue, err)
		}

		if objCreated {
			obj.Set(reflect.Append(obj, newValue))
		}

		if leafSlice {
			break
		}

	}

	return nil
}

// parses struct field in the path to look for a match and writes values into supplied object's value)
func writeStruct(obj reflect.Value, rs *api.Struct, path string, values []string) error {
	if obj.Kind() == reflect.Ptr {
		obj = reflect.Indirect(obj)
	}

	fieldNames := strings.Split(path, ".")
	fieldName := strings.Trim(fieldNames[0], "*[]")
	af, ok := rs.Fields[fieldName]
	if !ok {
		return fmt.Errorf("unable to find field '%s' in fields %+v", fieldName, rs.Fields)
	}

	for i := 0; i < obj.NumField(); i++ {
		typeField := obj.Type().Field(i)
		if typeField.Name != fieldName {
			continue
		}

		elem := obj.Field(i)
		subPath := strings.Join(fieldNames[1:], ".")
		if err := writeAny(elem, &af, subPath, values); err != nil {
			return err
		}
	}

	return nil
}

// copies the value `from` to `to - this is equivalen of `to := from`
func copyValue(to, from reflect.Value) {
	switch to.Kind() {
	case reflect.Struct:
		for i := 0; i < from.NumField(); i++ {
			to.Field(i).Set(from.Field(i))
		}
	default:
		to.Set(from)
	}
}

var typeInfo = map[string]struct {
	reflectType reflect.Type
	size        int
	unsigned    bool
}{
	"TYPE_BOOL":    {reflectType: reflect.TypeOf(true)},
	"TYPE_STRING":  {reflectType: reflect.TypeOf("")},
	"TYPE_FLOAT32": {reflectType: reflect.TypeOf(float32(0)), size: 32},
	"TYPE_FLOAT64": {reflectType: reflect.TypeOf(float64(0)), size: 64},
	"TYPE_INT":     {reflectType: reflect.TypeOf(0), size: 32},
	"TYPE_INT8":    {reflectType: reflect.TypeOf(int8(0)), size: 8},
	"TYPE_INT16":   {reflectType: reflect.TypeOf(int16(0)), size: 16},
	"TYPE_INT32":   {reflectType: reflect.TypeOf(int32(0)), size: 32},
	"TYPE_INT64":   {reflectType: reflect.TypeOf(int64(0)), size: 64},
	"TYPE_UINT":    {reflectType: reflect.TypeOf(uint(0)), size: 32, unsigned: true},
	"TYPE_UINT8":   {reflectType: reflect.TypeOf(uint8(0)), size: 8, unsigned: true},
	"TYPE_UINT16":  {reflectType: reflect.TypeOf(uint16(0)), size: 16, unsigned: true},
	"TYPE_UINT32":  {reflectType: reflect.TypeOf(uint32(0)), size: 32, unsigned: true},
	"TYPE_UINT64":  {reflectType: reflect.TypeOf(uint64(0)), size: 64, unsigned: true},
}

// gets reflect type corresponding to the schema type
func getType(kind string) reflect.Type {
	if _, ok := typeInfo[kind]; ok {
		return typeInfo[kind].reflectType
	}
	fieldSi := runtime.GetDefaultScheme().GetSchema(kind)
	if fieldSi == nil || fieldSi.GetTypeFn == nil {
		log.Fatalf("unable to find schema: kind %s structInfo = %+v\n", kind, fieldSi)
	}
	return fieldSi.GetTypeFn()
}

// getPrimitive returns a reflect.Value for a specified type set with the specified value
func getPrimitive(kind, value string) reflect.Value {
	v := reflect.Indirect(reflect.New(getType(kind)))
	switch kind {
	case "TYPE_BOOL":
		switch value {
		case "true":
			v.SetBool(true)
		case "false":
			v.SetBool(false)
		default:
			return reflect.Zero(getType(kind))
		}
		return v
	case "TYPE_STRING":
		v.SetString(value)
		return v
	case "TYPE_FLOAT32", "TYPE_FLOAT64":
		size := typeInfo[kind].size
		if key, err := strconv.ParseFloat(value, size); err == nil {
			v.SetFloat(key)
			return v
		}
	case "TYPE_INT", "TYPE_INT8", "TYPE_INT16", "TYPE_INT32", "TYPE_INT64", "TYPE_UINT", "TYPE_UINT8", "TYPE_UINT16", "TYPE_UINT32", "TYPE_UINT64":
		size := typeInfo[kind].size
		if typeInfo[kind].unsigned {
			if key, err := strconv.ParseUint(value, 10, size); err == nil {
				v.SetUint(key)
				return v
			}
		}
		if key, err := strconv.ParseInt(value, 10, size); err == nil {
			v.SetInt(key)
			return v
		}
	}
	return reflect.Zero(getType(kind))
}
