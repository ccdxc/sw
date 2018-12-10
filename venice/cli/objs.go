package vcli

import (
	"encoding/json"
	"fmt"
	"reflect"
	"sort"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cli/gen"
	"github.com/pensando/sw/venice/utils/runtime"
)

type cliField struct {
	typeStr string
	values  []string
	sskip   bool
}

// populateGenCtx pre-populates context structure with functions names that are called
// through out command processing. service is workload.Workload that is derived from the subcommand
func populateGenCtx(ctx *context) error {
	ctx.genInfo = gen.GetInfo()
	svcName, svcListName, err := ctx.genInfo.FindSvcName(ctx.subcmd)
	if err != nil {
		return err
	}

	scheme := runtime.GetDefaultScheme()

	ctx.structInfo = scheme.GetSchema(svcName)
	if ctx.structInfo == nil || ctx.structInfo.GetTypeFn == nil {
		return fmt.Errorf("unable to find schema: :svcName %s structInfo = %+v", svcName, ctx)
	}

	ctx.listStructInfo = scheme.GetSchema(svcListName)
	if ctx.listStructInfo == nil || ctx.listStructInfo.GetTypeFn == nil {
		return fmt.Errorf("unable to find schema: svcListName %s, structInfo = %+v", svcListName, ctx.listStructInfo)
	}

	ctx.removeObjOperFunc, err = ctx.genInfo.FindRemoveObjOperFunc(ctx.subcmd)
	if err != nil {
		return err
	}

	ctx.restGetFunc, err = ctx.genInfo.FindRestGetFunc(ctx.subcmd, "v1")
	if err != nil {
		return err
	}

	ctx.restDeleteFunc, err = ctx.genInfo.FindRestDeleteFunc(ctx.subcmd, "v1")
	if err != nil {
		return err
	}

	ctx.restPutFunc, err = ctx.genInfo.FindRestPutFunc(ctx.subcmd, "v1")
	if err != nil {
		return err
	}

	ctx.restPostFunc, err = ctx.genInfo.FindRestPostFunc(ctx.subcmd, "v1")
	if err != nil {
		return err
	}

	return nil
}

// getNewObj returns an empty object; type of an object is returned based on ctx populatation
func getNewObj(ctx *context) interface{} {
	objValue := reflect.New(ctx.structInfo.GetTypeFn())
	return objValue.Interface()
}

// getNewObjList returns an empty object list; type of an object is returned based on ctx populatation
func getNewObjList(ctx *context) interface{} {
	objValue := reflect.New(ctx.listStructInfo.GetTypeFn())
	return objValue.Interface()
}

// removeObjOper empties out the operational fields of an object;
// useful when saved configuration is replayed that may contain some read-only fields
func removeObjOper(ctx *context, obj interface{}) error {
	return ctx.removeObjOperFunc(obj)
}

// createObjFromBytes creates an object from json/yml bytes of data
func createObjFromBytes(ctx *context, inp string) error {
	obj := getNewObj(ctx)

	if err := json.Unmarshal([]byte(inp), obj); err != nil {
		return fmt.Errorf("unmarshaling error: %s", err)
	}

	objm, err := runtime.GetObjectMeta(obj)
	if err != nil {
		return fmt.Errorf("unable to fetch object meta from the object: %+v", obj)
	}

	objm.Tenant = ctx.tenant

	switch ctx.cmd {
	case "update", "edit":
		if err = ctx.restPutFunc(ctx.server, ctx.token, obj); err != nil {
			fmt.Printf("put error %s", err)
		}
	default:
		if err := ctx.restPostFunc(ctx.server, ctx.token, obj); err != nil {
			fmt.Printf("post error %s", err)
		}
	}

	return nil
}

// updateLabel updates the label of an exsting object
func updateLabel(obj interface{}, newLabels map[string]string) error {
	objm, err := runtime.GetObjectMeta(obj)
	if err != nil {
		return fmt.Errorf("unable to fetch object meta from the object: %+v", obj)
	}

	return updateMetaLabel(objm, newLabels)
}

// writeObj populates the provided structure with cli fields
func writeObj(ctx *context, obj interface{}, specKvs map[string]cliField) error {
	objValue := reflect.ValueOf(obj)
	if objValue.Kind() == reflect.Ptr {
		objValue = reflect.Indirect(objValue)
	}

	if objValue.Kind() != reflect.Struct {
		return fmt.Errorf("invalid kind %s", objValue.Kind())
	}

	for i := 0; i < objValue.NumField(); i++ {
		switch objValue.Type().Field(i).Name {
		case "TypeMeta":
			typeMeta := api.TypeMeta{Kind: ctx.subcmd}
			objValue.Field(i).Set(reflect.ValueOf(typeMeta))
		case "ObjectMeta":
			objectMeta := api.ObjectMeta{Tenant: ctx.tenant, Name: ctx.names[0], Labels: ctx.labels}
			objValue.Field(i).Set(reflect.ValueOf(objectMeta))
		}
	}

	return writeObjValue(ctx, objValue, specKvs)
}

// write cli fields from kvs map into the supplied object
func writeObjValue(ctx *context, obj reflect.Value, kvs map[string]cliField) error {

	svcName, _, err := ctx.genInfo.FindSvcName(ctx.subcmd)
	if err != nil {
		return err
	}

	rs := runtime.GetDefaultScheme().GetSchema(svcName)
	if rs == nil || rs.GetTypeFn == nil {
		return fmt.Errorf("unable to find schema: :svcName %s reflect type = %+v", svcName, rs.GetTypeFn())
	}

	for k, v := range kvs {
		path, err := ctx.structInfo.GetCLIFieldPath(k)
		if err != nil {
			return fmt.Errorf("unable to find key '%s' in the dictionary", k)
		}
		if len(path) == 0 {
			return fmt.Errorf("invalid path '%s' for key '%s'", path, k)
		}

		if err := writeStruct(obj, rs, path, v.values); err != nil {
			return err
		}
	}

	return nil
}

// getallKvs returns four set of maps
//  - spec/stats kvs for all objects in an array of maps, each map represents cli key-value
//  - object meta kvs for all objects in an arry of maps, each map represents cli key-value (key = field-name, value = <from object>)
//  - valid object meta map (key = field-name, value = bool which is true the field that is valid in any objects in the objList)
//  - valid object spec/status map (key = field-name, value = bool which is true the field that is valid in any objects in the objList)
//  valid object values are used to skip the display of empty fields for better width utilization
func getAllKvs(ctx *context, numItems int, objList interface{}) ([]map[string]cliField, []map[string]cliField, map[string]bool, map[string]bool) {
	objmKvs := []map[string]cliField{}
	specKvs := []map[string]cliField{}
	objmValidKvs := make(map[string]bool)
	specValidKvs := make(map[string]bool)

	for idx := 0; idx < numItems; idx++ {
		objmKvs = append(objmKvs, make(map[string]cliField))
		specKvs = append(specKvs, make(map[string]cliField))
		obj := getObjFromList(objList, idx)
		getObjMetaKvs(obj, objmKvs[idx])

		// get spec and status fields in specKvs
		getKvs(ctx, obj, "Spec", specKvs[idx])
		getKvs(ctx, obj, "Status", specKvs[idx])
		for key, fi := range objmKvs[idx] {
			if strings.Join(fi.values, "") != "" && !fi.sskip {
				objmValidKvs[key] = true
			}
		}
		for key, fi := range specKvs[idx] {
			if strings.Join(fi.values, "") != "" && !fi.sskip {
				specValidKvs[key] = true
			}
		}
	}

	return objmKvs, specKvs, objmValidKvs, specValidKvs
}

// getObjectMetaKv gets the cli key-value fields (in a map) from provided object's meta
func getObjMetaKvs(obj interface{}, kvs map[string]cliField) {
	objm, err := runtime.GetObjectMeta(obj)
	if err != nil {
		panic("object doesn't contain meta!")
	}

	kvs["name"] = cliField{values: []string{objm.Name}}
	if len(objm.Tenant) > 0 {
		kvs["tenant"] = cliField{values: []string{objm.Tenant}}
	}
	if len(objm.Namespace) > 0 {
		kvs["namespace"] = cliField{values: []string{objm.Namespace}}
	}
	if len(objm.Labels) > 0 {
		stringLabels := []string{}
		for labelK, labelV := range objm.Labels {
			stringLabels = append(stringLabels, fmt.Sprintf("%s%s%s", labelK, kvSplitter, labelV))
		}
		kvs["labels"] = cliField{values: stringLabels}
	}
}

// getKvs gets the cli key-value fields (in a map) from the provided object
func getKvs(ctx *context, obj interface{}, field string, kvs map[string]cliField) {
	// user specified to parse only a subfield of an object
	if field != "" {
		obj = getObjField(obj, field)
		if obj == nil {
			return
		}
	}

	objValue := reflect.ValueOf(obj)
	getKvsValue("", "", objValue, kvs)

	// iterate through all known cli tags
	for cliKey, cliValue := range ctx.structInfo.CLITags {
		relFieldPath := cliValue.Path
		if field != "" {
			// remove field prefix from absolute Path (e.g. remove 'Spec.' or 'Status.'
			relFieldPath = strings.TrimPrefix(relFieldPath, field+".")
		}
		// if there is a match for the absolute name path in kvs, then replace cli-id
		if _, ok := kvs[relFieldPath]; ok {
			kvs[cliKey] = kvs[relFieldPath]
			delete(kvs, relFieldPath)
		}
	}
}

// skipObj returns true if obj does not match the filter criteria
func skipObj(ctx *context, meta *api.ObjectMeta) bool {
	if matchRe(ctx.re, meta.Name) && matchLabel(ctx.labels, meta.Labels) && matchString(ctx.names, meta.Name) {
		return false
	}
	return true
}

// getFilteredNames, fetches all names and filter out the ones
// that do not meet the specified match criteria; this is used by
// BashCompleter functions for dynamic command completion
func getFilteredNames(ctx *context) []string {
	names := []string{}
	objList := getNewObjList(ctx)
	err := ctx.restGetFunc(ctx.server, ctx.tenant, ctx.token, objList)
	if err != nil {
		fmt.Printf("Error getting %ss: %v", ctx.subcmd, err)
		return names
	}

	for idx := 0; idx < getListNumItems(objList); idx++ {
		obj := getObjFromList(objList, idx)
		objm, err := runtime.GetObjectMeta(obj)
		if err != nil {
			fmt.Printf("Unable to fetch object meta from the object: %+v", objList)
			return names
		}
		if skipObj(ctx, objm) {
			continue
		}
		names = append(names, objm.Name)
	}

	return names
}

// get underline string of specified length
// TODO: replace with equivalent predefined strings function
func getHdrUL(len int) string {
	lineStr := ""
	for ii := 0; ii < len; ii++ {
		lineStr = lineStr + "-"
	}
	return lineStr
}

// getLineHeader returns one line of the header (field names with underlines) for multi-line read output
func getLineHeader(ctx *context, objmValidKvs, specValidKvs map[string]bool) []byte {
	hdrNames := ""
	hdrUL := ""
	if _, ok := objmValidKvs["name"]; ok {
		hdrNames += "name" + "\t"
		hdrUL += getHdrUL(len("name"))
	}
	hdrUL += "\t"
	if _, ok := objmValidKvs["labels"]; ok {
		hdrNames += "labels" + "\t"
		hdrUL += getHdrUL(len("labels"))
		hdrUL += "\t"
	}

	for _, key := range sortKeys(specValidKvs) {
		if !specValidKvs[key] {
			continue
		}
		hdrNames += key + "\t"
		hdrUL += getHdrUL(len(key)) + "\t"
	}
	return ([]byte(hdrNames + "\n" + hdrUL + "\n"))
}

// getLineData returns one line of the summary line in multi-line read output
func getLineData(ctx *context, specIdx *int, objmKvs, specKvs map[string]cliField, objmValidKvs, specValidKvs map[string]bool) ([]byte, bool) {
	more := false
	objLine := ""
	if fi, ok := objmKvs["name"]; ok && objmValidKvs["name"] {
		if *specIdx == 0 {
			objLine += fi.values[0]
		}
	}
	objLine += "\t"
	if fi, ok := objmKvs["labels"]; ok && objmValidKvs["labels"] {
		vals := fi.values
		sort.Strings(vals)
		if len(vals) >= *specIdx+1 {
			objLine += vals[*specIdx]
			if len(vals) >= *specIdx+2 {
				more = true
			}
		}
		objLine += "\t"
	}

	for _, key := range sortKeys(specValidKvs) {
		if !specValidKvs[key] {
			continue
		}

		fi := specKvs[key]
		if len(fi.values) >= *specIdx+1 {
			objLine += fi.values[*specIdx]
			if len(fi.values) >= *specIdx+2 {
				more = true
			}
		}
		objLine += "\t"
	}
	*specIdx++
	return []byte(objLine + "\n"), more
}

// getListNumItems returns total number of elements in the list
func getListNumItems(objList interface{}) int {
	return getFuzFromList(objList, 0, func(elem reflect.Value, idx int) interface{} {
		return elem.Len()
	}).(int)
}

// getObjFromList returns an object at specified index within an array of objList
func getObjFromList(objList interface{}, idx int) interface{} {
	return getFuzFromList(objList, idx, func(elem reflect.Value, idx int) interface{} {
		return elem.Index(idx).Interface()
	})
}

// getFuzFromList is a generic function to do something for 'idx' in an object list
// fuzFunc is called for the specified idx on the list
// the function uses reflect to parse through the generic structure of List objects
func getFuzFromList(objList interface{}, idx int, fuzFunc func(elem reflect.Value, idx int) interface{}) interface{} {
	objValue := reflect.ValueOf(objList)
	if objValue.Kind() == reflect.Ptr {
		objValue = reflect.Indirect(objValue)
	}

	if objValue.Kind() != reflect.Struct {
		return 0
	}

	for i := 0; i < objValue.NumField(); i++ {
		if objValue.Type().Field(i).Name != "Items" {
			continue
		}
		elem := objValue.Field(i)
		if elem.Kind() != reflect.Slice {
			return 0
		}
		return fuzFunc(elem, idx)
	}

	return 0
}

func getObjField(obj interface{}, field string) interface{} {
	objValue := reflect.ValueOf(obj)
	if objValue.Kind() == reflect.Ptr {
		objValue = reflect.Indirect(objValue)
	}

	if objValue.Kind() != reflect.Struct {
		return nil
	}

	for i := 0; i < objValue.NumField(); i++ {
		if objValue.Type().Field(i).Name == field {
			return objValue.Field(i).Interface()
		}
	}

	return nil
}
