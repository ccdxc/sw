package ref

import (
	"fmt"
	"reflect"
	"strconv"
	"strings"

	"github.com/pensando/sw/utils/log"
)

// FInfo represents a field within a nested struct/array
// a list of fields represent a flattened recursive struct
type FInfo struct {
	ValueStr []string
	TypeStr  string
	SSkip    bool
	Key      bool
}

// NewFInfo gets a new Field Info
func NewFInfo(strs []string) FInfo {
	return FInfo{ValueStr: strs, TypeStr: ""}
}

// GetSubObjFn returns an empty struct object by struct's name
// struct's name is either an object's name within caller's package
// or referred object in another package
// The function is called by Get/Write/Walk routines to fetch empty embeded structs
type GetSubObjFn func(string) interface{}

// RfCtx is context used by the package to pass on intermediate context or user context
type RfCtx struct {
	GetSubObj GetSubObjFn
}

type kvContext struct {
	mapKey    string
	prefixKey string
	fieldName string
	inSlice   bool
}

// GetKvs recursively reads an object and extract all fields as (key, value) pairs
// in the map passed as 'kvs'
func GetKvs(obj interface{}, refCtx *RfCtx, kvs map[string]FInfo) {
	rv := reflect.ValueOf(obj)
	kvCtx := &kvContext{}
	getKvs(rv, kvCtx, refCtx, kvs)
}

// WriteKvs recursively fills a given structure from supplied (key, value) pairs
// in the map. Upon update it returns the struct that is passed into it
// If the input object is non-null, this function performs an update to existing fields
// Input object is left unchanged, while a copy is made with updates
func WriteKvs(obj interface{}, refCtx *RfCtx, kvs map[string]FInfo) interface{} {
	orig := reflect.ValueOf(obj)
	new := reflect.New(orig.Type()).Elem()
	kvCtx := &kvContext{}
	writeKvs(new, orig, kvCtx, refCtx, kvs)

	return new.Interface()
}

// WalkStruct recursively walks an object and pretty prints the structure output
// It is different from go fmt printf in a way that it walks pointers to various
// fields as well
func WalkStruct(v interface{}, refCtx *RfCtx) string {
	rv := reflect.Indirect(reflect.ValueOf(v))
	return walkObj(rv, refCtx, 0)
}

// GetIndent returns spaces to adjust to the depth level
func GetIndent(level int) string {
	return strings.Repeat("  ", level)
}

// FieldByName find the values of a field name in a struct (recursively)
// Fielname is represented with dotted notation to parse embedded structs or maps/arrays
// Algorithm: fieldName could be a '.' separted struct fields, we compare the first field with
//    current field's name, upon a match the current field is removed and recursion can
//    on sub hierarchy of the field (which could be a struct)
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
				if isPrimitive(val.Kind(), getKindString(val.Kind())) {
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
	case reflect.Slice, reflect.Array:
		for i := 0; i < v.Len(); i++ {
			retSubStrs := FieldByName(v.Index(i), fieldName)
			if len(retSubStrs) > 0 {
				retStrs = append(retStrs, retSubStrs...)
			}
		}
	case reflect.Ptr:
		if !v.IsNil() {
			retStrs = FieldByName(reflect.Indirect(v), fieldName)
		}

	case reflect.Map:
		for _, key := range v.MapKeys() {
			val := v.MapIndex(key)
			retSubStrs := FieldByName(val, fieldName)
			if len(retSubStrs) > 0 {
				for ii := range retSubStrs {
					retSubStrs[ii] = fmt.Sprintf("%s", key) + ":" + retSubStrs[ii]
				}
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

// Venice Tags on a go struct indicate special meaning to the fields.
// Tags are comma separated and can be either a single string or 'key=value' style string
// Single string tags are expected to be boolean indications of the field where as
// 'key=value' tags indicate specific value of the field. Following tags are defined:
// "sskip" - skip this field in the summary output
// "ins=<val>" - indicates that insert this string to qualify the embedded fields
//   this is useful when a struct is embedded multiple times within a struct and meaning
//   of each occurence is different e.g. including Rule struct for 'in' rule and 'out rule
// "id=<val>" - indicates that the field be identified in output/cli-input/summary as this field
//   this field is similar to how field name can be different in json format
// "key" indicates that this field is the key for CRUD operations
// "wkt=<wkt_val>" indicates that this field is a known types (e.g. ipv4 address, or macaddress)
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

func getFieldName(kvCtx *kvContext, typeFieldName, veniceTag string) string {
	fieldName := kvCtx.prefixKey
	if fieldName != "" {
		fieldName += "_"
	}

	if stringTag := veniceTagString(veniceTag, "id"); stringTag != "" {
		fieldName += stringTag
	} else {
		fieldName += typeFieldName
	}
	return fieldName
}

func insertTag(kvCtx *kvContext, veniceTag string, fieldName string) string {
	insTag := veniceTagString(veniceTag, "ins")
	kvCtx.fieldName = fieldName

	if insTag != "" {
		if kvCtx.prefixKey != "" {
			kvCtx.prefixKey += "_"
		}
		kvCtx.prefixKey += insTag
	}

	return insTag
}

func removeTag(kvCtx *kvContext, insTag string) {
	if insTag != "" {
		kvCtx.prefixKey = strings.TrimSuffix(kvCtx.prefixKey, insTag)
		kvCtx.prefixKey = strings.TrimSuffix(kvCtx.prefixKey, "_")
	}
	kvCtx.fieldName = ""
}

// getKvs is shadow routine that works recursively for GetKvs
func getKvs(v reflect.Value, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) {
	switch v.Kind() {
	case reflect.Ptr:
		elem := v.Elem()
		if elem.IsValid() {
			getKvs(elem, kvCtx, refCtx, kvs)
		}
	case reflect.Struct:
		getKvsStruct(v, kvCtx, refCtx, kvs)

	case reflect.Slice, reflect.Array:
		getKvsSlice(v, kvCtx, refCtx, kvs)
	case reflect.Map:
		getKvsMap(v, kvCtx, refCtx, kvs)
	}
}

func getKvsSliceOne(kind reflect.Kind, v reflect.Value, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) {
	fieldName := kvCtx.fieldName
	if isPrimitive(kind, getKindString(kind)) {
		veniceTag := kvCtx.prefixKey
		t, val, isLeaf := getKv(kvCtx, v)
		if isLeaf {
			if newVal, ok := kvs[fieldName]; ok {
				newVal.ValueStr = append(newVal.ValueStr, val)
				kvs[fieldName] = newVal
			} else {
				kvs[fieldName] = FInfo{
					TypeStr:  t,
					ValueStr: []string{val},
					SSkip:    veniceTagBool(veniceTag, "sskip"),
					Key:      veniceTagBool(veniceTag, "key")}
			}
		}
	} else {
		getKvs(v, kvCtx, refCtx, kvs)
	}
}

func getKvsSlice(v reflect.Value, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) {
	elem := v.Type().Elem()
	elemKind := elem.Kind()
	if elemKind == reflect.Ptr {
		elem = elem.Elem()
	}
	oldInSlice := kvCtx.inSlice
	kvCtx.inSlice = true
	if v.Len() == 0 {
		vi := getSubObj(v, elem.Name(), elem.Kind(), refCtx)
		vi = reflect.Indirect(vi)
		getKvsSliceOne(elem.Kind(), vi, kvCtx, refCtx, kvs)
	} else {
		for i := 0; i < v.Len(); i++ {
			vi := v.Index(i)
			if elemKind == reflect.Ptr {
				vi = reflect.Indirect(vi)
			}
			getKvsSliceOne(elem.Kind(), vi, kvCtx, refCtx, kvs)
		}
	}
	kvCtx.inSlice = oldInSlice
}

func getKvsMap(v reflect.Value, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) {
	elem := v.Type().Elem()
	if v.Len() == 0 {
		rv := getSubObj(v, elem.Name(), elem.Kind(), refCtx)
		if isPrimitive(elem.Kind(), getKindString(elem.Kind())) {
			typeStr := getKindString(elem.Kind())
			fieldName := kvCtx.fieldName
			veniceTag := kvCtx.prefixKey
			kvs[fieldName] = FInfo{TypeStr: typeStr, SSkip: veniceTagBool(veniceTag, "sskip"), Key: veniceTagBool(veniceTag, "key")}
		} else {
			getKvs(rv, kvCtx, refCtx, kvs)
		}
	}
	for _, key := range v.MapKeys() {
		val := v.MapIndex(key)
		if isPrimitive(elem.Kind(), getKindString(elem.Kind())) {
			typeStr := getKindString(elem.Kind())
			fieldName := kvCtx.fieldName
			veniceTag := kvCtx.prefixKey
			valueStr := fmt.Sprintf("%s:%s", key, val)
			if newVal, ok := kvs[fieldName]; ok {
				newVal.ValueStr = append(newVal.ValueStr, valueStr)
				kvs[fieldName] = newVal
			} else {
				kvs[fieldName] = FInfo{
					TypeStr:  typeStr,
					ValueStr: []string{valueStr},
					SSkip:    veniceTagBool(veniceTag, "sskip"),
					Key:      veniceTagBool(veniceTag, "key")}
			}
		} else {
			kvCtx.mapKey = fmt.Sprintf("%s", key)
			getKvs(val, kvCtx, refCtx, kvs)
			kvCtx.mapKey = ""
		}
	}
}

func getKvsStruct(v reflect.Value, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) {
	for i := 0; i < v.NumField(); i++ {
		val := v.Field(i)
		typeField := v.Type().Field(i)
		veniceTag := typeField.Tag.Get("venice")

		fieldName := getFieldName(kvCtx, typeField.Name, veniceTag)

		if val.Kind() == reflect.Ptr {
			elem := typeField.Type.Elem()
			val = getSubObj(val, elem.Name(), elem.Kind(), refCtx)
			val = reflect.Indirect(val)
		}

		t, v, isLeaf := getKv(kvCtx, val)
		if isLeaf {
			if newVal, ok := kvs[fieldName]; ok {
				newVal.ValueStr = append(newVal.ValueStr, v)
				kvs[fieldName] = newVal
			} else {
				kvs[fieldName] = FInfo{
					TypeStr:  t,
					ValueStr: []string{v},
					SSkip:    veniceTagBool(veniceTag, "sskip"),
					Key:      veniceTagBool(veniceTag, "key")}
			}
		} else {
			insTag := insertTag(kvCtx, veniceTag, fieldName)
			getKvs(val, kvCtx, refCtx, kvs)
			removeTag(kvCtx, insTag)
		}
	}
}

func getKv(kvCtx *kvContext, v reflect.Value) (string, string, bool) {
	typeStr := getKindString(v.Kind())
	if kvCtx.inSlice {
		typeStr = "slice"
	}
	valueStr := ""
	if kvCtx.mapKey != "" {
		valueStr = kvCtx.mapKey + ":"
	}
	isLeaf := false
	switch v.Kind() {
	case reflect.Ptr, reflect.Struct, reflect.Slice, reflect.Array, reflect.Map:
		return typeStr, valueStr, false
	case reflect.Bool, reflect.Int, reflect.Int32, reflect.Int64, reflect.Uint32, reflect.Uint64, reflect.String:
		valueStr += fmt.Sprintf("%v", v)
		isLeaf = true
	default:
		valueStr = fmt.Sprintf("unknown-type: %s", v)
		isLeaf = true
	}
	return typeStr, valueStr, isLeaf
}

// writeKvs is shadow routine that works recursively for WriteKvs
func writeKvs(new, orig reflect.Value, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) reflect.Value {
	mapKey := reflect.Value{}
	switch orig.Kind() {
	case reflect.Struct:
		mapKey = writeKvsStruct(new, orig, kvCtx, refCtx, kvs)

	case reflect.Slice, reflect.Array:
		writeKvsSlice(new, orig, kvCtx, refCtx, kvs)
	case reflect.Map:
		writeKvsMap(new, orig, kvCtx, refCtx, kvs)

	default:
		new.Set(orig)
	}
	return mapKey
}

func writeKvsMap(new, orig reflect.Value, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) {
	elem := orig.Type().Elem()
	fieldName := kvCtx.fieldName
	elemKind := elem.Kind()
	if elemKind == reflect.Ptr {
		elem = elem.Elem()
	}
	replace := isStructFieldFound(elem.Kind(), elem.Name(), kvCtx, refCtx, kvs)

	new.Set(reflect.MakeMap(orig.Type()))
	if replace || len(orig.MapKeys()) == 0 {
		oldMapKey := kvCtx.mapKey
		kvCtx.mapKey = "withinMap"
		numRecs, subStructs, mapKeys := getSubStructs(fieldName, elem.Name(), elemKind, elem.Kind(), kvCtx, refCtx, kvs)
		kvCtx.mapKey = oldMapKey

		for idx := 0; idx < numRecs; idx++ {
			new.SetMapIndex(mapKeys[idx], subStructs[idx])
		}
	} else {
		for _, key := range orig.MapKeys() {
			origValue := orig.MapIndex(key)
			copyValue := reflect.New(origValue.Type()).Elem()
			oldMapKey := kvCtx.mapKey
			kvCtx.mapKey = fmt.Sprintf("%s", key)
			writeKvs(copyValue, origValue, kvCtx, refCtx, kvs)
			kvCtx.mapKey = oldMapKey
			new.SetMapIndex(key, copyValue)
		}
	}
}

func writeKvsOne(fieldName, currValue string, new, orig reflect.Value, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) reflect.Value {
	if fi, ok := kvs[fieldName]; ok {
		if len(fi.ValueStr) > 1 {
			newfi := FInfo{TypeStr: fi.TypeStr, SSkip: fi.SSkip, Key: fi.Key, ValueStr: fi.ValueStr[1:]}
			kvs[fieldName] = newfi
		} else {
			delete(kvs, fieldName)
		}
		if fi.ValueStr[0] != "" {
			return writeKv(new, orig, fi.ValueStr[0], kvCtx)
		}
	} else {
		return writeKv(new, orig, currValue, kvCtx)
	}
	return reflect.Value{}
}

func isStructFieldFound(elemKind reflect.Kind, elemName string, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) bool {
	replace := false
	fieldName := kvCtx.fieldName
	if isKindPrimitive(elemKind) {
		if kvCtx.prefixKey != "" {
			fieldName = kvCtx.prefixKey + "_" + fieldName
		}
		if _, ok := kvs[fieldName]; ok {
			replace = true
		}
	} else {
		subObjPtr := refCtx.GetSubObj(elemName)
		if subObjPtr == nil {
			panic(fmt.Sprintf("invalid sub object: kind %s name %s\n", elemKind, elemName))
		}
		subObjVal := reflect.ValueOf(subObjPtr)
		subObjVal = reflect.Indirect(subObjVal)
		subObj := subObjVal.Interface()
		subKvs := make(map[string]FInfo)
		GetKvs(subObj, refCtx, subKvs)

		for fieldName := range subKvs {
			if kvCtx.prefixKey != "" {
				fieldName = kvCtx.prefixKey + "_" + fieldName
			}
			if _, ok := kvs[fieldName]; ok {
				replace = true
				break
			}
		}
	}

	return replace
}

func getSubStructs(fieldName, elemName string, parentElemKind, elemKind reflect.Kind, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) (int, []reflect.Value, []reflect.Value) {
	numRecs := 0
	mapKeys := []reflect.Value{}
	subStructs := []reflect.Value{}
	for len(kvs) > 0 {
		var subObjPtr interface{}

		prevKvs := map[string]FInfo{}
		for k, v := range kvs {
			prevKvs[k] = v
		}

		if isPrimitive(elemKind, elemName) {
			subObjPtr = getBaseObj(getKindString(elemKind))
		} else {
			subObjPtr = refCtx.GetSubObj(elemName)
		}
		if subObjPtr == nil {
			panic(fmt.Sprintf("invalid sub object: kind %s name %s\n", elemKind, elemName))
		}
		rvOrig := reflect.Indirect(reflect.ValueOf(subObjPtr))
		rvNew := reflect.Indirect(reflect.ValueOf(subObjPtr))

		mapKey := reflect.Value{}
		if isKindPrimitive(elemKind) {
			mapKey = writeKvsOne(fieldName, "", rvNew, rvOrig, kvCtx, refCtx, kvs)
		} else {
			mapKey = writeKvs(rvNew, rvOrig, kvCtx, refCtx, kvs)
		}

		if reflect.DeepEqual(prevKvs, kvs) {
			break
		}
		if parentElemKind == reflect.Ptr {
			subStructs = append(subStructs, reflect.ValueOf(subObjPtr))
		} else {
			subStructs = append(subStructs, rvNew)
		}
		mapKeys = append(mapKeys, mapKey)
		numRecs++
	}
	return numRecs, subStructs, mapKeys
}

func writeKvsSlice(new, orig reflect.Value, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) {
	elem := orig.Type().Elem()
	fieldName := kvCtx.fieldName
	elemKind := elem.Kind()
	if elemKind == reflect.Ptr {
		elem = elem.Elem()
	}
	replace := isStructFieldFound(elem.Kind(), elem.Name(), kvCtx, refCtx, kvs)

	if replace || orig.Len() == 0 {
		numRecs, subStructs, _ := getSubStructs(fieldName, elem.Name(), elemKind, elem.Kind(), kvCtx, refCtx, kvs)

		if numRecs > 0 {
			if orig.Kind() == reflect.Slice {
				new.Set(reflect.MakeSlice(orig.Type(), numRecs, numRecs))
			} else {
				new.Set(reflect.Indirect(reflect.New(orig.Type())))
			}
			for idx := 0; idx < numRecs; idx++ {
				new.Index(idx).Set(subStructs[idx])
			}
		}
	} else {
		if orig.Kind() == reflect.Slice {
			new.Set(reflect.MakeSlice(orig.Type(), orig.Len(), orig.Cap()))
		} else {
			new.Set(reflect.Indirect(reflect.New(orig.Type())))
		}
		for i := 0; i < orig.Len(); i++ {
			writeKvs(new.Index(i), orig.Index(i), kvCtx, refCtx, kvs)
		}
	}
}

func writeKvsStruct(new, orig reflect.Value, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) reflect.Value {
	mapKey := reflect.Value{}

	for i := 0; i < orig.NumField(); i++ {
		typeField := orig.Type().Field(i)
		val := orig.Field(i)
		veniceTag := typeField.Tag.Get("venice")
		fieldName := getFieldName(kvCtx, typeField.Name, veniceTag)

		newField := new.Field(i)
		origField := orig.Field(i)
		if val.Kind() == reflect.Ptr {
			replace := false
			if _, ok := kvs[fieldName]; ok {
				replace = true
			}
			if replace || orig.Field(i).IsNil() {
				elem := typeField.Type.Elem()
				val = getSubObj(val, elem.Name(), elem.Kind(), refCtx)
				new.Field(i).Set(val)

				newField = new.Field(i).Elem()
				origField = val.Elem()
				val = reflect.Indirect(val)
			}
		}

		_, v, isLeaf := getKv(kvCtx, val)
		if isLeaf {
			mapKey = writeKvsOne(fieldName, v, newField, origField, kvCtx, refCtx, kvs)
		} else {
			insTag := insertTag(kvCtx, veniceTag, fieldName)
			mapKey = writeKvs(newField, origField, kvCtx, refCtx, kvs)
			removeTag(kvCtx, insTag)
		}
	}

	return mapKey
}

func writeKv(new, orig reflect.Value, kvString string, kvCtx *kvContext) reflect.Value {
	key := ""
	valueString := kvString
	kvMap := strings.Split(kvString, ":")
	if kvCtx.mapKey == "withinMap" && len(kvMap) > 1 {
		key = kvMap[0]
		valueString = strings.Join(kvMap[1:], ":")
	}

	switch orig.Kind() {
	case reflect.Int, reflect.Int32, reflect.Int64:
		intVal, _ := strconv.ParseInt(valueString, 10, 64)
		new.SetInt(intVal)
	case reflect.Uint64, reflect.Uint32:
		var uintVal uint64
		if strings.HasPrefix(valueString, "0x") {
			uintVal, _ = strconv.ParseUint(strings.TrimPrefix(valueString, "0x"), 16, 64)
		} else {
			uintVal, _ = strconv.ParseUint(valueString, 10, 64)
		}
		new.SetUint(uintVal)
	case reflect.String:
		new.SetString(valueString)
	default:
		log.Errorf("Invalid kind %s %+v\n", orig.Kind(), orig)
	}

	return reflect.ValueOf(key)
}

// walkObj is shadow routine that works recursively for WalkStruct
func walkObj(v reflect.Value, refCtx *RfCtx, level int) string {
	outStr := ""
	switch v.Kind() {
	case reflect.Struct:
		outStr = walkObjStruct(v, refCtx, level)

	case reflect.Array, reflect.Slice:
		outStr = walkObjSlice(v, refCtx, level)
	case reflect.Map:
		outStr = walkObjMap(v, refCtx, level)
	default:
		log.Errorf("unsuspected kind %s", v.Kind())
	}
	return outStr
}

func walkObjMap(v reflect.Value, refCtx *RfCtx, level int) string {
	elem := v.Type().Elem()
	outStr := fmt.Sprintf("map[%s]%s\n", v.Type().Key().Kind(), elem.Kind())
	level++
	if v.Len() == 0 && !isKindPrimitive(elem.Kind()) {
		subObjPtr := refCtx.GetSubObj(elem.Name())
		if subObjPtr == nil {
			panic(fmt.Sprintf("invalid sub object: name %s\n", elem.Name()))
		}
		val := reflect.Indirect(reflect.ValueOf(subObjPtr))
		outStr += walkObj(val, refCtx, level)
	} else {
		for _, key := range v.MapKeys() {
			val := v.MapIndex(key)
			if isPrimitive(val.Kind(), getKindString(val.Kind())) {
				valueStr := fmt.Sprintf("%s", val)
				if valueStr == "" {
					valueStr = fmt.Sprintf("%s", getKindString(val.Kind()))
				}
				outStr += fmt.Sprintf("%s%s:%s\n", GetIndent(level), key, valueStr)
			} else {
				outStr += walkObj(val, refCtx, level)
			}
		}
	}
	return outStr
}

func walkObjSlice(v reflect.Value, refCtx *RfCtx, level int) string {
	elem := v.Type().Elem()
	outStr := fmt.Sprintf("[]")
	if !isKindPrimitive(elem.Kind()) {
		if v.Len() == 0 {
			if elem.Kind() == reflect.Ptr {
				elem = elem.Elem()
				outStr += "*"
			}
			nv := getSubObj(v, elem.Name(), elem.Kind(), refCtx)
			nv = reflect.Indirect(nv)
			if isPrimitive(nv.Kind(), elem.Name()) {
				outStr += fmt.Sprintf("%s\n", elem.Name())
			} else {
				outStr += walkObj(nv, refCtx, level+1)
			}
		} else {
			for i := 0; i < v.Len(); i++ {
				outStr += walkObj(v.Index(i), refCtx, level+1)
			}
		}
	} else {
		outStr += fmt.Sprintf("%s\n", elem.Kind())
	}
	return outStr
}

func walkObjStruct(v reflect.Value, refCtx *RfCtx, level int) string {
	outStr := fmt.Sprintf("%s{\n", GetIndent(level))
	level++
	for i := 0; i < v.NumField(); i++ {
		val := v.Field(i)
		typeField := v.Type().Field(i)

		ptrStr := ""
		if val.Kind() == reflect.Ptr {
			elem := typeField.Type.Elem()
			val = getSubObj(val, elem.Name(), elem.Kind(), refCtx)
			val = reflect.Indirect(val)
			ptrStr = "*"
		}

		if isKindPrimitive(val.Kind()) {
			valueStr := fmt.Sprintf("%s", val.Kind())
			outStr += fmt.Sprintf("%s%s: %s\n", GetIndent(level), ptrStr+typeField.Name, valueStr)
		} else {
			outStr += fmt.Sprintf("%s%s: ", GetIndent(level), ptrStr+typeField.Name)
			outStr += walkObj(val, refCtx, level)
		}
	}
	outStr += fmt.Sprintf("%s}\n", GetIndent(level))
	return outStr
}

// Utilities functions are defined to assist with other routines
func isKindPrimitive(v reflect.Kind) bool {
	if v == reflect.Bool || v == reflect.String || v == reflect.Int || v == reflect.Int8 ||
		v == reflect.Int16 || v == reflect.Int32 || v == reflect.Int64 || v == reflect.Float32 ||
		v == reflect.Float64 || v == reflect.Uint || v == reflect.Uint8 || v == reflect.Uint16 ||
		v == reflect.Uint32 || v == reflect.Uint64 || v == reflect.Uintptr ||
		v == reflect.Complex64 || v == reflect.Complex128 {
		return true
	}
	return false
}

func isPrimitive(v reflect.Kind, elemName string) bool {
	if isKindPrimitive(v) && getKindString(v) == elemName {
		return true
	}

	return false
}

func getKindString(kind reflect.Kind) string {
	return kind.String()
	/*
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
		case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64,
			reflect.Float32, reflect.Float64, reflect.Complex64, reflect.Complex128:
			return "int"

		case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32,
			reflect.Uint64, reflect.Uintptr:
			return "uint"
		default:
			log.Errorf("invalid kind: %s", kind)
			return "unknown"
		}
	*/
}

func getSubObj(val reflect.Value, name string, kind reflect.Kind, refCtx *RfCtx) reflect.Value {
	if !val.IsValid() || val.IsNil() {
		if isPrimitive(kind, name) {
			subObjPtr := getBaseObj(name)
			if subObjPtr == nil {
				panic(fmt.Sprintf("invalid primitive object: name %s kind %s\n", name, kind))
			}
			val = reflect.ValueOf(subObjPtr)
		} else {
			subObjPtr := refCtx.GetSubObj(name)
			if subObjPtr == nil {
				panic(fmt.Sprintf("invalid sub object: name %s kind %s\n", name, kind))
			}
			val = reflect.ValueOf(subObjPtr)
		}
	}
	return val
}

func getBaseObj(name string) interface{} {
	switch name {
	case "bool":
		v := false
		return &v
	case "string":
		v := ""
		return &v

	case "int":
		v := int(0)
		return &v
	case "int8":
		v := int8(0)
		return &v
	case "int16":
		v := int16(0)
		return &v
	case "int32":
		v := int32(0)
		return &v
	case "int64":
		v := int64(0)
		return &v

	case "uint":
		v := uint(0)
		return &v
	case "uint8":
		v := uint8(0)
		return &v
	case "uint16":
		v := uint16(0)
		return &v
	case "uint32":
		v := uint32(0)
		return &v
	case "uint64":
		v := uint64(0)
		return &v

	}
	log.Errorf("Invalid base obj type '%s'", name)
	return nil
}
