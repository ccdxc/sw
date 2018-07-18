package runtime

import (
	"errors"
	"fmt"
	"reflect"
	"strings"
	"sync"

	"github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
)

var (
	defaultScheme *Scheme
	once          sync.Once
)

// Scheme contains methods to help with serialization/deserialization of API
// objects.
type Scheme struct {
	kindToTypes map[string]reflect.Type
	Types       map[string]*Struct
}

// NewScheme returns a new Scheme.
func NewScheme() *Scheme {
	return &Scheme{
		kindToTypes: make(map[string]reflect.Type),
		Types:       make(map[string]*Struct),
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
		s.kindToTypes[strings.ToLower(t.Name())] = t
	}
}

// New creates a new object given a kind, if the kind was registered using
// AddKnownTypes above.
func (s *Scheme) New(kind string) (Object, error) {
	kind = strings.ToLower(kind)
	t, exists := s.kindToTypes[kind]
	if !exists {
		return nil, fmt.Errorf("Kind %s not registered with this scheme", kind)
	}
	return reflect.New(t).Interface().(Object), nil
}

// AddSchema adds a set of schema nodes. This is typically added by
//  the generated code during init
func (s *Scheme) AddSchema(in map[string]*Struct) {
	for k, v := range in {
		if _, ok := s.Types[k]; ok {
			panic(fmt.Sprintf("type %v already registered", k))
		}
		s.Types[k] = v
		if v.Tags == nil {
			v.Tags = make(map[string]string)
		}
		for _, f := range v.Fields {
			if f.JSONTag != "" {
				v.Tags[f.JSONTag] = f.Name
			}
		}
	}
}

// GetSchema returns the type schema for kind specified in in
func (s *Scheme) GetSchema(in string) *Struct {
	return s.Types[in]
}

//NewEmpty returns a new object of the same kind as the input
func NewEmpty(in Object) (Object, error) {
	val := reflect.ValueOf(in)
	if !val.IsValid() {
		return nil, fmt.Errorf("invalid input")
	}
	if val.Kind() == reflect.Ptr {
		if val.IsNil() {
			return nil, fmt.Errorf("invalid input")
		}
		tpe := reflect.Indirect(val).Type()
		return reflect.New(tpe).Interface().(Object), nil
	}
	tpe := val.Type()
	return reflect.Indirect(reflect.New(tpe)).Interface().(Object), nil
}

// GetDefaultScheme retrieves the default scheme if there is already one or creates one.
func GetDefaultScheme() *Scheme {
	once.Do(func() {
		defaultScheme = NewScheme()
	})
	return defaultScheme
}

// CLIInfo is a container for all CLI Related tags and info
type CLIInfo struct {
	Path   string
	Skip   bool
	Insert string
	Help   string
}

// Field represents the schema details of a field
type Field struct {
	Name    string
	CLITag  CLIInfo
	JSONTag string
	Pointer bool
	Slice   bool
	Map     bool
	// KeyType is valid only when Map is true
	KeyType string
	// Type specifies either the path for the type (cluster.ClusterSpec) or
	//  if the field is a scalar the type of scalar (TYPE_STRING, TYPE_INT64, etc)
	Type string
}

// Struct represents the schema details of a field
type Struct struct {
	Fields    map[string]Field // Refers to to Field Object in Schema
	Tags      map[string]string
	GetTypeFn func() reflect.Type
	CLITags   map[string]CLIInfo
}

// FindField finds a field schema in the Struct by golang name.
func (n *Struct) FindField(in string) (Field, bool) {
	f, ok := n.Fields[in]
	return f, ok
}

// FindFieldByJSONTag finds a field schema in the Struct by jsontag specified.
func (n *Struct) FindFieldByJSONTag(in string) (Field, bool) {
	name, ok := n.Tags[in]
	if !ok {
		return Field{}, false
	}
	f, ok := n.Fields[name]
	return f, ok
}

// GetType returns the reflect.Type for this type
func (n *Struct) GetType() reflect.Type {
	if n.GetTypeFn != nil {
		n.GetTypeFn()
	}
	return nil
}

// GetCLIFieldPath returns the fully qualified field path for the CLI tag
func (n *Struct) GetCLIFieldPath(in string) (string, error) {
	if v, ok := n.CLITags[in]; ok {
		return v.Path, nil
	}
	return "", errors.New("not found")
}

// IsScalar returns if the provided type is a scalar.
func IsScalar(in string) bool {
	if in == "TYPE_GROUP" || in == "TYPE_MESSAGE" {
		return false
	}
	_, ok := descriptor.FieldDescriptorProto_Type_value[in]
	return ok
}
