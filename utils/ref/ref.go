package ref

import (
	"fmt"
	log "github.com/Sirupsen/logrus"
	"reflect"
	"strings"
)

type FInfo struct {
	ValueStr []string
	TypeStr  string
}

func NewFInfo(strs []string) FInfo {
	return FInfo{ValueStr: strs, TypeStr: ""}
}

func veniceTagBool(allTags, lookupTag string) bool {
	vals := strings.Split(allTags, ",")
	for _, val := range vals {
		if val == lookupTag {
			return true
		}
	}
	return false
}

func veniceTagString(allTags, lookupTag string) string {
	vals := strings.Split(allTags, ",")
	for _, val := range vals {
		kvs := strings.Split(val, "=")
		if len(kvs) == 2 && kvs[0] == lookupTag {
			return kvs[1]
		}
	}
	return ""
}

// GetKvs reads a structure recursively and extract all fields as key-values in the supplied map
func GetKvs(v reflect.Value, getSubObj GetSubObj, kvs map[string]FInfo) {
	switch v.Kind() {
	case reflect.Ptr:
		elem := v.Elem()
		if elem.IsValid() {
			GetKvs(elem, getSubObj, kvs)
		}
	case reflect.Struct:
		for i := 0; i < v.NumField(); i++ {
			val := v.Field(i)
			typeField := v.Type().Field(i)
			tag := typeField.Tag
			if veniceTagBool(tag.Get("venice"), "sskip") {
				continue
			}

			t, v, isLeaf := getKv(val)
			if isLeaf {
				if newVal, ok := kvs[typeField.Name]; ok {
					newVal.ValueStr = append(newVal.ValueStr, v)
					kvs[typeField.Name] = newVal
				} else {
					kvs[typeField.Name] = FInfo{TypeStr: t, ValueStr: []string{v}}
				}
			} else {
				GetKvs(val, getSubObj, kvs)
			}
		}
	case reflect.Slice:
		elem := v.Type().Elem()
		if v.Len() == 0 && !isPrimitive(elem.Kind()) {
			if subObjPtr := getSubObj(elem.Name()); subObjPtr != nil {
				rv := reflect.Indirect(reflect.ValueOf(subObjPtr))
				GetKvs(rv, getSubObj, kvs)
			}
		}
		for i := 0; i < v.Len(); i++ {
			GetKvs(v.Index(i), getSubObj, kvs)
		}
	case reflect.Map:
		for _, key := range v.MapKeys() {
			val := v.MapIndex(key)
			GetKvs(val, getSubObj, kvs)
		}
	}
}

func getKv(v reflect.Value) (string, string, bool) {
	typeStr := "other"
	valueStr := ""
	isLeaf := false
	switch v.Kind() {
	case reflect.Ptr:
		return typeStr, valueStr, false
	case reflect.Struct:
		return typeStr, valueStr, false
	case reflect.Slice:
		typeStr = "slice"
		elem := v.Type().Elem()
		if isPrimitive(elem.Kind()) {
			retStr := ""
			for i := 0; i < v.Len(); i++ {
				retStr += fmt.Sprintf("%s,", v.Index(i))
			}
			valueStr = strings.TrimSuffix(retStr, ",")
			isLeaf = true
		}
	case reflect.Map:
		elem := v.Type().Elem()
		typeStr = "map"
		if isPrimitive(elem.Kind()) {
			retStr := ""
			for _, k := range v.MapKeys() {
				retStr += fmt.Sprintf("%s:%s,", k, v.MapIndex(k))
			}
			valueStr = strings.TrimSuffix(retStr, ",")
			isLeaf = true
		}
	case reflect.Bool:
		typeStr = "bool"
		valueStr = fmt.Sprintf("%v", v)
		isLeaf = true
	case reflect.Int:
		typeStr = "int"
		valueStr = fmt.Sprintf("%d", v)
		isLeaf = true
	case reflect.String:
		typeStr = "string"
		valueStr = fmt.Sprintf("%s", v)
		isLeaf = true
	default:
		valueStr = fmt.Sprintf("%s", v)
		isLeaf = true
	}
	return typeStr, valueStr, isLeaf
}

func isPrimitive(v reflect.Kind) bool {
	if v != reflect.Struct &&
		v != reflect.Map &&
		v != reflect.Slice &&
		v != reflect.Interface &&
		v != reflect.Array &&
		v != reflect.Ptr {
		return true
	}
	return false
}

func getKindString(kind reflect.Kind) string {
	switch kind {
	case reflect.Struct:
		return "struct"
	case reflect.Bool:
		return "bool"
	case reflect.String:
		return "string"
	case reflect.Ptr:
		return "ptr"
	case reflect.Array:
		return "array"
	case reflect.Map:
		return "map"
	case reflect.Slice:
		return "slice"
	case reflect.Interface:
		return "interface"
	case reflect.Int:
	case reflect.Int8:
	case reflect.Int16:
	case reflect.Int32:
	case reflect.Int64:
	case reflect.Uint:
	case reflect.Uint8:
	case reflect.Uint16:
	case reflect.Uint32:
	case reflect.Uint64:
	case reflect.Uintptr:
	case reflect.Float32:
	case reflect.Float64:
	case reflect.Complex64:
	case reflect.Complex128:
		return "int"
	default:
		return "unknown"
	}
	return "unknown"
}

// WriteKvs fills a given structure with the supplied key-value pairs in the supplied map
func WriteKvs(new, orig reflect.Value, getSubObj GetSubObj, kvs map[string]FInfo) {
	switch orig.Kind() {
	case reflect.Struct:
		for i := 0; i < orig.NumField(); i++ {
			typeField := orig.Type().Field(i)
			val := orig.Field(i)

			_, v, isLeaf := getKv(val)
			if isLeaf {
				if fi, ok := kvs[typeField.Name]; ok {
					if len(fi.ValueStr) > 1 {
						newfi := FInfo{TypeStr: fi.TypeStr, ValueStr: fi.ValueStr[1:]}
						kvs[typeField.Name] = newfi
					} else {
						delete(kvs, typeField.Name)
					}
					if fi.ValueStr[0] != "" {
						writeKv(new.Field(i), orig.Field(i), fi.ValueStr[0])
					}
				} else {
					writeKv(new.Field(i), orig.Field(i), v)
				}
			} else if val.Kind() == reflect.Ptr {
				elem := typeField.Type.Elem()
				subObj := orig.Field(i)
				if orig.Field(i).IsNil() {
					if subObjPtr := getSubObj(elem.Name()); subObjPtr != nil {
						subObj = reflect.ValueOf(subObjPtr)
					}
				}
				new.Field(i).Set(subObj)
				WriteKvs(new.Field(i).Elem(), subObj.Elem(), getSubObj, kvs)
			} else {
				WriteKvs(new.Field(i), orig.Field(i), getSubObj, kvs)
			}
		}
	case reflect.Slice:
		new.Set(reflect.MakeSlice(orig.Type(), orig.Len(), orig.Cap()))
		for i := 0; i < orig.Len(); i++ {
			WriteKvs(new.Index(i), orig.Index(i), getSubObj, kvs)
		}
	case reflect.Map:
		new.Set(reflect.MakeMap(orig.Type()))
		for _, key := range orig.MapKeys() {
			origValue := orig.MapIndex(key)
			copyValue := reflect.New(origValue.Type()).Elem()
			WriteKvs(copyValue, origValue, getSubObj, kvs)
			new.SetMapIndex(key, copyValue)
		}
	default:
		new.Set(orig)
	}
}

func writeKv(new, orig reflect.Value, kvString string) {

	switch orig.Kind() {
	case reflect.Slice:
		strs := strings.Split(kvString, ",")
		if len(strs) <= 0 {
			log.Errorf("error parsing kvstring: %s\n", kvString)
			return
		}
		new.Set(reflect.MakeSlice(orig.Type(), len(strs), len(strs)))
		for i := 0; i < len(strs); i++ {
			newSliceItem := new.Index(i)
			newSliceItem.SetString(strs[i])
		}
	case reflect.Map:
		strs := strings.Split(kvString, ",")
		if len(strs) <= 0 {
			log.Errorf("error parsing kvstring: %s\n", kvString)
		}
		new.Set(reflect.MakeMap(orig.Type()))

		for i := range strs {
			kvs := strings.Split(strs[i], ":")
			if len(kvs) != 2 {
				log.Errorf("error parsing map value from kvstring '%s', kv '%s'\n", kvString, strs[i])
				return
			}
			mapKey := reflect.New(reflect.TypeOf("")).Elem()
			mapKey.SetString(kvs[0])
			mapValue := reflect.New(reflect.TypeOf("")).Elem()
			mapValue.SetString(kvs[1])
			new.SetMapIndex(mapKey, mapValue)
		}
	case reflect.Int:
	case reflect.String:
		new.SetString(kvString)
	default:
		log.Errorf("Invalid kind %s \n", orig.Kind())
	}
}

// GetSubObj returns an empty struct object by its name
// This function is called inline for the type of structs, other sub-objects refer to
type GetSubObj func(string) interface{}

// Walkstruct walks a structure nested and all types using reflect
func WalkStruct(v interface{}, getSubObj GetSubObj) string {
	rv := reflect.Indirect(reflect.ValueOf(v))
	return walkStruct(rv, getSubObj, 0)
}

func walkStruct(v reflect.Value, getSubObj GetSubObj, level int) string {
	outStr := ""
	switch v.Kind() {
	case reflect.Struct:
		outStr += fmt.Sprintf("%s{\n", PrintTabs(level))
		level++
		for i := 0; i < v.NumField(); i++ {
			val := v.Field(i)
			typeField := v.Type().Field(i)
			// tag := typeField.Tag
			// tag.Get("json")

			if isPrimitive(val.Kind()) {
				valueStr := fmt.Sprintf("%s", val.Kind())
				outStr += fmt.Sprintf("%s%s: %s,\n", PrintTabs(level), typeField.Name, valueStr)
			} else {
				outStr += fmt.Sprintf("%s%s: ", PrintTabs(level), typeField.Name)
				outStr += walkStruct(val, getSubObj, level)
			}
		}
		level--
		outStr += fmt.Sprintf("%s}\n", PrintTabs(level))
	case reflect.Slice:
		elem := v.Type().Elem()
		outStr += fmt.Sprintf("[]%s\n", elem.Kind())
		if !isPrimitive(elem.Kind()) {
			if v.Len() == 0 {
				if subObjPtr := getSubObj(elem.Name()); subObjPtr != nil {
					rv := reflect.Indirect(reflect.ValueOf(subObjPtr))
					outStr += walkStruct(rv, getSubObj, level+1)
				}
			}

			for i := 0; i < v.Len(); i += 1 {
				outStr += walkStruct(v.Index(i), getSubObj, level+1)
			}
		}
	case reflect.Map:
		elem := v.Type().Elem()
		outStr += fmt.Sprintf("map[%s]%s\n", v.Type().Key().Kind(), elem.Kind())
		level++
		for _, key := range v.MapKeys() {
			val := v.MapIndex(key)
			if isPrimitive(val.Kind()) {
				valueStr := fmt.Sprintf("%s", val)
				if valueStr == "" {
					valueStr = fmt.Sprintf("%s", getKindString(val.Kind()))
				}
				outStr += fmt.Sprintf("%s%s:%s,\n", PrintTabs(level), key, valueStr)
			} else {
				outStr += walkStruct(val, getSubObj, level+1)
			}
		}
		level--
	}
	return outStr
}

// PrintTabs prints spaces to adjust to the depth level
func PrintTabs(level int) string {
	retStr := ""
	for ii := 0; ii < level; ii++ {
		retStr += fmt.Sprintf("  ")
	}
	return retStr
}

// FieldByName find the values of a field name in a struct (recursively)
func FieldByName(v reflect.Value, fieldName string) []string {
	allFs := strings.Split(fieldName, ".")
	currF := ""
	rFs := ""
	if len(allFs) > 0 {
		currF = allFs[0]
		rFs = strings.Join(allFs[1:], ".")
	}
	retStrs := []string{}

	switch v.Kind() {
	case reflect.Struct:
		for i := 0; i < v.NumField(); i++ {
			val := v.Field(i)
			typeField := v.Type().Field(i)
			if typeField.Name == currF {
				if isPrimitive(val.Kind()) {
					if len(rFs) == 0 {
						retSubStr := fmt.Sprintf("%v", val)
						if retSubStr != "" {
							retStrs = append(retStrs, retSubStr)
						}
					}
				} else {
					retStrs = FieldByName(val, rFs)
				}
			}
		}
	case reflect.Slice:
		for i := 0; i < v.Len(); i += 1 {
			retSubStrs := FieldByName(v.Index(i), fieldName)
			if len(retSubStrs) > 0 {
				retStrs = append(retStrs, retSubStrs...)
			}
		}
	case reflect.Map:
		for _, key := range v.MapKeys() {
			val := v.MapIndex(key)
			retSubStrs := FieldByName(val, fieldName)
			if len(retSubStrs) > 0 {
				retStrs = append(retStrs, retSubStrs...)
			}
		}
	default:
		retSubStr := fmt.Sprintf("%v", v)
		if retSubStr != "" {
			retStrs = []string{retSubStr}
		}
	}
	return retStrs
}
