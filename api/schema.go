package api

import (
	"errors"
	"reflect"
)

// CLIInfo is a container for all CLI Related tags and info
type CLIInfo struct {
	ID     string
	Path   string
	Skip   bool
	Insert string
	Help   string
}

// PathsMap maps URI and Key paths
type PathsMap struct {
	URI string
	Key string
}

// Field represents the schema details of a field
type Field struct {
	Name       string
	CLITag     CLIInfo
	JSONTag    string
	Pointer    bool
	Slice      bool
	Inline     bool
	FromInline bool
	Mutable    bool
	Map        bool
	// KeyType is valid only when Map is true
	KeyType string
	// Type specifies either the path for the type (cluster.ClusterSpec) or
	//  if the field is a scalar the type of scalar (TYPE_STRING, TYPE_INT64, etc)
	Type string
}

// Struct represents the schema details of a field
type Struct struct {
	Kind      string
	APIGroup  string
	Scopes    []string
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
		return n.GetTypeFn()
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
