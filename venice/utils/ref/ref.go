package ref

import (
	"fmt"
	"reflect"
	"strconv"
	"strings"

	"github.com/pensando/sw/venice/utils/log"
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
// The function is called by Get/Write/Walk routines to fetch empty embedded structs
type GetSubObjFn func(string) interface{}

// NilSubObj doesn't recognize any sub object name
func NilSubObj(string) interface{} {
	return nil
}

// CustomParser is an interface to be implemented by objects that have custom
// parsers. For example, label selectors are represented as a string in cli and
// parsed in to labels.Selector when passed to the backend.
type CustomParser interface {
	// Parse a string representation of the object
	Parse(string) (reflect.Value, error)
	// Print an object to its string representation
	Print(reflect.Value) string
}

// RfCtx is context used by the package to pass on intermediate context or user context
type RfCtx struct {
	GetSubObj     GetSubObjFn
	UseJSONTag    bool
	CustomParsers map[string]CustomParser
}

type kvContext struct {
	mapKey    string
	prefixKey string
	sskip     bool
	isKey     bool
	typeName  string
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
// Algorithm: fieldName could be a '.' separted struct fields, we compare the first sield with
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
				if isPrimitive(val.Kind(), val.Kind().String()) {
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
//   of each occurrence is different e.g. including Rule struct for 'in' rule and 'out rule
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

func getJSONFieldName(refCtx *RfCtx, typeField *reflect.StructField) string {
	typeFieldName := typeField.Name
	if refCtx.UseJSONTag {
		jsonTags := strings.Split(typeField.Tag.Get("json"), ",")
		if len(jsonTags) > 0 && jsonTags[0] != "" {
			typeFieldName = jsonTags[0]
		}
	}
	return typeFieldName
}

func getFieldName(kvCtx *kvContext, refCtx *RfCtx, typeField *reflect.StructField) string {
	veniceTag := typeField.Tag.Get("venice")
	fieldName := kvCtx.prefixKey
	typeFieldName := getJSONFieldName(refCtx, typeField)

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

func insertTag(kvCtx *kvContext, veniceTag, typeName, fieldName string) (string, bool, bool) {
	insTag := veniceTagString(veniceTag, "ins")
	sskip := kvCtx.sskip
	isKey := kvCtx.isKey
	kvCtx.typeName = typeName
	kvCtx.fieldName = fieldName
	kvCtx.sskip = veniceTagBool(veniceTag, "sskip")
	kvCtx.isKey = veniceTagBool(veniceTag, "key")

	if insTag != "" {
		if kvCtx.prefixKey != "" {
			kvCtx.prefixKey += "_"
		}
		kvCtx.prefixKey += insTag
	}

	return insTag, sskip, isKey
}

func removeTag(kvCtx *kvContext, insTag string, sskip, isKey bool) {
	if insTag != "" {
		kvCtx.prefixKey = strings.TrimSuffix(kvCtx.prefixKey, insTag)
		kvCtx.prefixKey = strings.TrimSuffix(kvCtx.prefixKey, "_")
	}
	kvCtx.sskip = sskip
	kvCtx.isKey = isKey
	kvCtx.fieldName = ""
	kvCtx.typeName = ""
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
		if cp, ok := refCtx.CustomParsers[kvCtx.typeName]; ok {
			kvs[kvCtx.fieldName] = FInfo{
				ValueStr: []string{cp.Print(v)},
				TypeStr:  reflect.String.String(),
				SSkip:    kvCtx.sskip,
				Key:      kvCtx.isKey,
			}
			return
		}
		getKvsStruct(v, kvCtx, refCtx, kvs)

	case reflect.Slice, reflect.Array:
		getKvsSlice(v, kvCtx, refCtx, kvs)
	case reflect.Map:
		getKvsMap(v, kvCtx, refCtx, kvs)
	}
}

func getKvsSliceOne(kind reflect.Kind, v reflect.Value, kvCtx *kvContext, refCtx *RfCtx, kvs map[string]FInfo) {
	fieldName := kvCtx.fieldName
	if isPrimitive(kind, kind.String()) {
		t, val, isLeaf := getKv(kvCtx, v)
		if isLeaf {
			if newVal, ok := kvs[fieldName]; ok {
				newVal.ValueStr = append(newVal.ValueStr, val)
				kvs[fieldName] = newVal
			} else {
				kvs[fieldName] = FInfo{
					TypeStr:  t,
					ValueStr: []string{val},
					SSkip:    kvCtx.sskip,
					Key:      kvCtx.isKey,
				}
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
		if isPrimitive(elem.Kind(), elem.Kind().String()) {
			typeStr := elem.Kind().String()
			fieldName := kvCtx.fieldName
			kvs[fieldName] = FInfo{TypeStr: typeStr, SSkip: kvCtx.sskip, Key: kvCtx.isKey}
		} else {
			getKvs(rv, kvCtx, refCtx, kvs)
		}
	}
	for _, key := range v.MapKeys() {
		val := v.MapIndex(key)
		if isPrimitive(elem.Kind(), elem.Kind().String()) {
			typeStr := elem.Kind().String()
			fieldName := kvCtx.fieldName
			valueStr := fmt.Sprintf("%s:%s", key, val)
			if newVal, ok := kvs[fieldName]; ok {
				newVal.ValueStr = append(newVal.ValueStr, valueStr)
				kvs[fieldName] = newVal
			} else {
				kvs[fieldName] = FInfo{
					TypeStr:  typeStr,
					ValueStr: []string{valueStr},
					SSkip:    kvCtx.sskip,
					Key:      kvCtx.isKey,
				}
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
		fieldName := getFieldName(kvCtx, refCtx, &typeField)

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
					Key:      veniceTagBool(veniceTag, "key"),
				}
			}
		} else {
			insTag, sskip, isKey := insertTag(kvCtx, veniceTag, fmt.Sprintf("%v", typeField.Type), fieldName)
			getKvs(val, kvCtx, refCtx, kvs)
			removeTag(kvCtx, insTag, sskip, isKey)
		}
	}
}

func getKv(kvCtx *kvContext, v reflect.Value) (string, string, bool) {
	typeStr := v.Kind().String()
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
	case reflect.Bool, reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64, reflect.Uint, reflect.Uint8,
		reflect.Uint16, reflect.Uint32, reflect.Uint64, reflect.Float32, reflect.Float64, reflect.String:
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
		if cp, ok := refCtx.CustomParsers[kvCtx.typeName]; ok {
			info, ok := kvs[kvCtx.fieldName]
			if ok {
				delete(kvs, kvCtx.fieldName)
				v, err := cp.Parse(info.ValueStr[0])
				if err == nil {
					new.Set(v)
					return reflect.ValueOf("")
				}
			}
		}
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
		if len(fi.ValueStr) > 0 && fi.ValueStr[0] != "" {
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
			subObjPtr = getBaseObj(elemKind.String())
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
		fieldName := getFieldName(kvCtx, refCtx, &typeField)

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
			insTag, sskip, key := insertTag(kvCtx, veniceTag, fmt.Sprintf("%v", typeField.Type), fieldName)
			mapKey = writeKvs(newField, origField, kvCtx, refCtx, kvs)
			removeTag(kvCtx, insTag, sskip, key)
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
	case reflect.Bool:
		boolVal := false
		if valueString == "true" {
			boolVal = true
		}
		new.SetBool(boolVal)
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		intVal, _ := strconv.ParseInt(valueString, 10, 64)
		new.SetInt(intVal)
	case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64:
		var uintVal uint64
		if strings.HasPrefix(valueString, "0x") {
			uintVal, _ = strconv.ParseUint(strings.TrimPrefix(valueString, "0x"), 16, 64)
		} else {
			uintVal, _ = strconv.ParseUint(valueString, 10, 64)
		}
		new.SetUint(uintVal)
	case reflect.Float32, reflect.Float64:
		floatVal, _ := strconv.ParseFloat(valueString, 64)
		new.SetFloat(floatVal)
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
	outStr := fmt.Sprintf("map[%s]%s", v.Type().Key().Kind(), elem.Kind())
	if v.Len() == 0 && !isKindPrimitive(elem.Kind()) {
		subObjPtr := refCtx.GetSubObj(elem.Name())
		if subObjPtr == nil {
			panic(fmt.Sprintf("invalid sub object: name %s\n", elem.Name()))
		}
		val := reflect.Indirect(reflect.ValueOf(subObjPtr))
		outStr += walkObj(val, refCtx, level)
	} else {
		outStr += "\n"
		for _, key := range v.MapKeys() {
			val := v.MapIndex(key)
			if isPrimitive(val.Kind(), val.Kind().String()) {
				valueStr := fmt.Sprintf("%s", val)
				if valueStr == "" {
					valueStr = fmt.Sprintf("%s", val.Kind().String())
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
	lenStr := "[]"
	if v.Kind() == reflect.Array {
		lenStr = fmt.Sprintf("[%d]", v.Len())
	}
	outStr := lenStr
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
				outStr += walkObj(nv, refCtx, level)
			}
		} else {
			for i := 0; i < v.Len(); i++ {
				if i > 0 {
					outStr += GetIndent(level)
				}
				outStr += walkObj(v.Index(i), refCtx, level)
			}
		}
	} else {
		outStr += fmt.Sprintf("%s\n", elem.Kind())
	}
	return outStr
}

func walkObjStruct(v reflect.Value, refCtx *RfCtx, level int) string {
	outStr := "{\n"
	level++
	for i := 0; i < v.NumField(); i++ {
		val := v.Field(i)
		typeField := v.Type().Field(i)
		fieldName := getJSONFieldName(refCtx, &typeField)

		ptrStr := ""
		if val.Kind() == reflect.Ptr {
			elem := typeField.Type.Elem()
			val = getSubObj(val, elem.Name(), elem.Kind(), refCtx)
			val = reflect.Indirect(val)
			ptrStr = "*"
		}

		if isKindPrimitive(val.Kind()) {
			valueStr := fmt.Sprintf("%s", val.Kind())
			outStr += fmt.Sprintf("%s%s: %s\n", GetIndent(level), ptrStr+fieldName, valueStr)
		} else {
			outStr += fmt.Sprintf("%s%s: ", GetIndent(level), ptrStr+fieldName)
			outStr += walkObj(val, refCtx, level)
		}
	}
	level--
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
	if isKindPrimitive(v) && v.String() == elemName {
		return true
	}

	return false
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

// fieldByJSONTag returns the field Name given a JSON tag. 't' must be a structure.
func fieldByJSONTag(t reflect.Type, json string) (*reflect.StructField, int) {
	if t.Kind() != reflect.Struct {
		return nil, 0
	}
	for ii := 0; ii < t.NumField(); ii++ {
		f := t.Field(ii)
		tag := f.Tag.Get("json")
		if tag == "" {
			continue
		}
		parts := strings.Split(tag, ",")
		if parts[0] == json {
			return &f, ii
		}
	}
	return nil, 0
}

// ParseVal is a utility function to parse a string to a value. It uses the provided kind
// to do the validation and parsing.
func ParseVal(kind reflect.Kind, value string) (reflect.Value, error) {
	size := 0
	unsigned := false
	var v reflect.Value
	switch kind {
	case reflect.Bool:
		v = reflect.Indirect(reflect.New(reflect.TypeOf(true)))
		switch value {
		case "true":
			v.SetBool(true)
		case "false":
			v.SetBool(false)
		default:
			return reflect.Value{}, fmt.Errorf("Error parsing %v as bool", value)
		}
		return v, nil
	case reflect.Float32:
		size = 32
		v = reflect.New(reflect.TypeOf(float32(0)))
		fallthrough
	case reflect.Float64:
		if size == 0 {
			size = 64
			v = reflect.New(reflect.TypeOf(float64(0)))
		}
		key, err := strconv.ParseFloat(value, size)
		if err != nil {
			return reflect.Value{}, fmt.Errorf("Error parsing %v as float%v: %v", value, size, err)
		}
		v = reflect.Indirect(v)
		v.SetFloat(key)
		return v, nil
	case reflect.Int:
		size = 32
		v = reflect.New(reflect.TypeOf(0))
	case reflect.Int8:
		size = 8
		v = reflect.New(reflect.TypeOf(int8(0)))
	case reflect.Int16:
		size = 16
		v = reflect.New(reflect.TypeOf(int16(0)))
	case reflect.Int32:
		size = 32
		v = reflect.New(reflect.TypeOf(int32(0)))
	case reflect.Int64:
		size = 64
		v = reflect.New(reflect.TypeOf(int64(0)))
	case reflect.Uint:
		size = 32
		unsigned = true
		v = reflect.New(reflect.TypeOf(uint(0)))
	case reflect.Uint8:
		size = 8
		unsigned = true
		v = reflect.New(reflect.TypeOf(uint8(0)))
	case reflect.Uint16:
		size = 16
		unsigned = true
		v = reflect.New(reflect.TypeOf(uint16(0)))
	case reflect.Uint32:
		size = 32
		unsigned = true
		v = reflect.New(reflect.TypeOf(uint32(0)))
	case reflect.Uint64:
		size = 64
		unsigned = true
		v = reflect.New(reflect.TypeOf(uint64(0)))
	case reflect.String:
		return reflect.ValueOf(value), nil
	default:
		return reflect.Value{}, fmt.Errorf("Unsupported kind %v for ParseVal", kind)
	}
	v = reflect.Indirect(v)
	if unsigned {
		key, err := strconv.ParseUint(value, 10, size)
		if err != nil {
			return reflect.Value{}, fmt.Errorf("Error parsing %v as uint%v: %v", value, size, err)
		}
		v.SetUint(key)
		return v, nil
	}
	key, err := strconv.ParseInt(value, 10, size)
	if err != nil {
		return reflect.Value{}, fmt.Errorf("Error parsing %v as int%v: %v", value, size, err)
	}
	v.SetInt(key)
	return v, nil
}

// FieldByJSONTag converts hierachical json tag based field to name based field.
// 'v' must be a dummy structure with pointers, maps, slices instantiated. In the
// future, this will be rewritten to use generated schema.
//
// It validates that indexing can only happen on maps. maps support two forms of
// indexing - "*" for any key OR a specific key. Slices skip indexing.
//
// Returns an error on failure.
//
// Valid examples:
//   spec.vlan                                    => Spec.Vlan
//   spec.networksSlice.vlan                      => Spec.NetworksSlice.Vlan
//   spec.networksMap[*].ipaddressesSlice.gateway => Spec.NetworksMap[*].IpAddressesSlice.Gateway
//   spec.networksMap[abc].vlan                   => Spec.NetworksMap[abc].Vlan
//
func FieldByJSONTag(v reflect.Value, f string) (string, error) {
	fList := strings.Split(f, ".")
	result := ""
	t := v.Type()
	for ii := range fList {
		jsonStr := fList[ii]
		indexStr := "" // string inside the []
		if t.Kind() == reflect.Ptr {
			v = reflect.Indirect(v)
			if !v.IsValid() {
				return "", fmt.Errorf("Invalid or nil pointer for %v", t)
			}
			t = v.Type()
		}
		// if string contains [], the element should be a slice or map
		jj := strings.Index(jsonStr, "[")
		// Strip off the indexed part
		if jj != -1 {
			jsonStr = jsonStr[:jj]
			indexStr = fList[ii][jj+1 : len(fList[ii])-1]
		}
		sf, index := fieldByJSONTag(t, jsonStr)
		if sf == nil {
			return "", fmt.Errorf("Did not find field %v in %v", jsonStr, t)
		}
		if result != "" {
			result += "."
		}
		result += sf.Name
		v = v.Field(index)
		t = v.Type()
		if jj != -1 {
			switch t.Kind() {
			case reflect.Slice:
				return "", fmt.Errorf("Indexing is not supported on slice %v, found %v", t, indexStr)
			case reflect.Map:
				// "*" is ok for indexing maps with int keys
				if indexStr != "*" {
					if _, err := ParseVal(t.Key().Kind(), indexStr); err != nil {
						return "", err
					}
				}

				keys := v.MapKeys()
				if len(keys) == 0 {
					return "", fmt.Errorf("Empty map %v", t)
				}
				t = t.Elem()
				v = v.MapIndex(keys[0])
			default:
				return "", fmt.Errorf("Found [] in non map kind %v in %v", t.Kind(), t)
			}
			// Add back the stripped off []
			result += fList[ii][jj:]
		} else if t.Kind() == reflect.Slice {
			t = t.Elem()
			v = v.Index(0)
		}
	}
	switch t.Kind() {
	case reflect.Bool, reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64, reflect.Uint, reflect.Uint8,
		reflect.Uint16, reflect.Uint32, reflect.Uint64, reflect.Float32, reflect.Float64, reflect.String:
	default:
		return "", fmt.Errorf("Leaf kind is %v, not scalar for %v", t.Kind(), t)
	}
	return result, nil
}

// sliceValues is a helper for fieldValues, see below.
func sliceValues(v reflect.Value, f string) ([]string, error) {
	result := []string{}
	if v.Kind() != reflect.Slice {
		return nil, fmt.Errorf("%v is not a slice", v)
	}
	for ii := 0; ii < v.Len(); ii++ {
		res, err := fieldValues(v.Index(ii), f)
		if err != nil {
			return nil, err
		}
		result = append(result, res...)
	}
	return result, nil
}

// mapValues is a helper for fieldValues, see below.
func mapValues(v reflect.Value, index, f string) ([]string, error) {
	if v.Kind() != reflect.Map {
		return nil, fmt.Errorf("%v is not a map", v)
	}
	if index == "*" {
		keys := v.MapKeys()
		result := []string{}
		for _, key := range keys {
			res, err := fieldValues(v.MapIndex(key), f)
			if err != nil {
				return nil, err
			}
			result = append(result, res...)
		}
		return result, nil
	}
	key, err := ParseVal(v.Type().Key().Kind(), index)
	if err != nil {
		return nil, err
	}
	newV := v.MapIndex(key)
	if !newV.IsValid() {
		return []string{}, nil
	}
	return fieldValues(newV, f)
}

// fieldValues is a helper for FieldValues, see below.
func fieldValues(v reflect.Value, f string) ([]string, error) {
	if v.Kind() == reflect.Ptr {
		v = reflect.Indirect(v)
		if !v.IsValid() {
			return nil, fmt.Errorf("Invalid or nil pointer for %v", v)
		}
	}
	if isKindPrimitive(v.Kind()) {
		if f != "" {
			return nil, fmt.Errorf("Non empty field at leaf: %v", f)
		}
		return []string{fmt.Sprintf("%v", v)}, nil
	}
	switch v.Kind() {
	case reflect.Struct:
		if f == "" {
			return nil, fmt.Errorf("Empty field at non-leaf: %v", v)
		}
		fList := strings.Split(f, ".")
		fName := fList[0]
		ii := strings.Index(fList[0], "[")
		indexStr := ""
		if ii != -1 {
			fName = fList[0][:ii]
			indexStr = fList[0][ii+1 : len(fList[0])-1]
		}
		newV := v.FieldByName(fName)
		if !newV.IsValid() {
			return nil, fmt.Errorf("Field %v is not valid", fName)
		}
		if newV.Kind() == reflect.Map && ii == -1 || newV.Kind() == reflect.Map && indexStr == "" ||
			newV.Kind() != reflect.Map && ii != -1 {
			return nil, fmt.Errorf("Non indexed map or indexed but not map, Val: %v, Field: %v", v, fList[0])
		}
		newF := strings.TrimPrefix(f, fList[0])
		newF = strings.TrimPrefix(newF, ".")
		switch newV.Kind() {
		case reflect.Slice:
			return sliceValues(newV, newF)
		case reflect.Map:
			return mapValues(newV, indexStr, newF)
		default:
			return fieldValues(newV, newF)
		}
		// Slice of Slice, Slice of Map, Map of Map, Map of Slice fields not supported.
		// Examples: [][]string, []map[string]string, map[string][]string
	}
	return nil, fmt.Errorf("Hit leaf on non-leaf field %v", f)
}

const (
	maxFieldLen = 1000
)

// FieldValues retrieves string values for a hierarchical field in a struct.
// 'v' must be a struct or a pointer to a struct and 'f' must be a valid
// hierarchical field within 'v'. If the heirarchy includes a map field,
// the traversal is based on a the specified index. '*' results in traversing
// the entire map. A specific key only traverses that key. If that key is not
// found, it returns an empty result with no error.
//
// Returns an error on failure.
//
//
func FieldValues(v reflect.Value, f string) ([]string, error) {
	if len(f) > maxFieldLen {
		return nil, fmt.Errorf("%v exceeds max field length of %v", f, maxFieldLen)
	}
	return fieldValues(v, f)
}
