package runtime

import (
	"fmt"
	"reflect"
)

// Scheme contains methods to help with serialization/deserialization of API
// objects.
type Scheme struct {
	kindToTypes map[string]reflect.Type
}

// NewScheme returns a new Scheme.
func NewScheme() *Scheme {
	return &Scheme{
		kindToTypes: make(map[string]reflect.Type),
	}
}

// AddKnownTypes is used to register known API objects. All API objects need to
// register here.
// TODO: Version based differentiation.
func (s *Scheme) AddKnownTypes(types ...Object) {
	for _, obj := range types {
		t := reflect.TypeOf(obj)
		if t.Kind() != reflect.Ptr {
			panic("Must be a ptr")
		}
		t = t.Elem()
		if t.Kind() != reflect.Struct {
			panic("Must be a ptr to struct")
		}
		s.kindToTypes[t.Name()] = t
	}
}

// New creates a new object given a kind, if the kind was registered using
// AddKnownTypes above.
func (s *Scheme) New(kind string) (Object, error) {
	t, exists := s.kindToTypes[kind]
	if !exists {
		return nil, fmt.Errorf("Kind %s not registered with this scheme", kind)
	}
	return reflect.New(t).Interface().(Object), nil
}
