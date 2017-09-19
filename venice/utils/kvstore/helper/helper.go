package helper

import (
	"fmt"
	"reflect"
)

// ValidObjForDecode checks if an object is valid for decoding in to.
func ValidObjForDecode(v interface{}) error {
	if v == nil {
		return fmt.Errorf("Decode error: nil")
	}

	rv := reflect.ValueOf(v)
	// Check if its a pointer object.
	if rv.Kind() != reflect.Ptr {
		return fmt.Errorf("Decode error: non-pointer %v", reflect.TypeOf(v).String())
	}

	// Check if the pointer is non nil.
	if rv.IsNil() {
		return fmt.Errorf("Decode error: nil %v", reflect.TypeOf(v).String())
	}
	return nil
}

// ValidListObjForDecode checks if an object is valid list object for decoding in to.
// It looks for a pointer object that has an "Items" field which is a slice.
//
// Valid example 1:
// type FooList struct {
//      api.TypeMeta
//      api.ListMeta
//      Items []Foo
// }
//
// Valid example 2:
// type FooList struct {
//      api.TypeMeta
//      api.ListMeta
//      Items []*Foo
// }
//
// Valid example 3 (valid only if Items pointer is non nil):
// type FooList struct {
//      api.TypeMeta
//      api.ListMeta
//      Items *[]Foo
// }
func ValidListObjForDecode(l interface{}) (reflect.Value, error) {
	// It should be a valid Object first (non-nil ptr Object).
	if err := ValidObjForDecode(l); err != nil {
		return reflect.Value{}, err
	}

	// Check if Items field exists.
	f := reflect.ValueOf(l).Elem().FieldByName("Items")
	if !f.IsValid() {
		return reflect.Value{}, fmt.Errorf("Decode error: missing Items field")
	}

	var intf interface{}
	switch f.Kind() {
	case reflect.Slice:
		// Example 1/2 above.
		intf = f.Addr().Interface()
	case reflect.Ptr:
		// Example 3 above. Check if its a pointer to a slice.
		kind := reflect.TypeOf(f.Interface()).Elem().Kind()
		if kind != reflect.Slice {
			return reflect.Value{}, fmt.Errorf("Decode error: expecting slice, got %v", kind)
		}
		intf = f.Interface()
	default:
		return reflect.Value{}, fmt.Errorf("Decode error: expecting slice, got %v", f.Kind())
	}

	// Check if the pointer is non nil.
	v := reflect.ValueOf(intf)
	if v.IsNil() {
		return reflect.Value{}, fmt.Errorf("Decode error: expecting pointer, got nil")
	}
	return v.Elem(), nil
}
