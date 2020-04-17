package bulkedit

import (
	"encoding/json"
	fmt "fmt"
	"reflect"
	"strings"

	proto "github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/types"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

// UnmarshalJSON implements the json.Unmarshaller interface.
func (p *BulkEditItem) UnmarshalJSON(b []byte) error {
	var err error

	var BulkEditItemResultMap map[string]interface{}

	err = json.Unmarshal(b, &BulkEditItemResultMap)
	if err != nil {
		return err
	}

	methodv, present := BulkEditItemResultMap["method"]
	if !present {
		return fmt.Errorf("item is missing method type")
	}

	method := methodv.(string)

	objectv, present := BulkEditItemResultMap["object"]
	if !present {
		return fmt.Errorf("item is missing object type")
	}

	objectResultMap := objectv.(map[string]interface{})

	kindv, present := objectResultMap["kind"]
	if !present {
		return fmt.Errorf("missing Kind type, unable to infer the object type")
	}

	kind := kindv.(string)
	if kind == "" {
		return fmt.Errorf("missing Kind type, unable to infer the object type, object %s", string(b))
	}
	schema := runtime.GetDefaultScheme()
	group := schema.Kind2APIGroup(kind)
	sch := schema.GetSchema(group + "." + kind)
	tpe := sch.GetType()
	obj1 := reflect.New(tpe).Interface()

	objBytes, err := json.Marshal(objectResultMap)
	if err != nil {
		return err
	}

	err = json.Unmarshal(objBytes, obj1)
	if err != nil {
		return err
	}

	makeKeyFn := reflect.ValueOf(obj1).MethodByName("MakeKey")

	if !makeKeyFn.IsValid() {
		return fmt.Errorf("Invalid Object kind:%s", kind)
	}
	pval := makeKeyFn.Call([]reflect.Value{reflect.ValueOf(group)})
	objKey := pval[0].Interface().(string)
	objURI := schema.GetURI(objKey, "v1")

	protoAny, err := types.MarshalAny(obj1.(proto.Message))
	if err != nil {
		return err
	}

	p.URI = objURI
	p.Method = method
	p.Object = &api.Any{Any: *protoAny}

	return nil
}

// FetchObjectFromBulkEditItem Performs the UnamrshalAny operation and returns the runtime.object and its kind
func (p *BulkEditItem) FetchObjectFromBulkEditItem() (string, string, runtime.Object, error) {

	var err error
	var kind, group string
	var obj runtime.Object
	schema := runtime.GetDefaultScheme()

	if p == nil {
		return kind, group, obj, err
	}

	if p.GetObject() == nil {
		err = fmt.Errorf("invalid object:%s", err.Error())
		return kind, group, obj, err
	}
	typeURL := p.GetObject().GetTypeUrl()
	typeStr := strings.Split(typeURL, "/")[1] // Remove the type.googleapis.com from the typeURL to get the obj Kind
	kind = strings.Split(typeStr, ".")[1]
	group = strings.Split(typeStr, ".")[0]
	obj, err = schema.New(kind)
	if err != nil {
		err = fmt.Errorf("kind field is invalid:%s", err.Error())
		return kind, group, obj, err
	}

	objp := obj.(proto.Message)
	err = types.UnmarshalAny(&(p.GetObject().Any), objp)
	if err != nil {
		err = fmt.Errorf("invalid BulkEditItem:%s", err.Error())
		return kind, group, obj, err
	}

	objR := objp.(runtime.Object)

	return kind, group, objR, err
}

// Validate validates the BulkEditActionSpec.
// Dummy function to help compile .ext.go files that include BulkEditActionSpec.
func (s *BulkEditActionSpec) Validate(ver, path string, ignoreStatus, ignoreSpec bool) []error {
	return nil
}

// Normalize normalizes the object if needed
func (s *BulkEditActionSpec) Normalize() {}
