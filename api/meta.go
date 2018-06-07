package api

import (
	"fmt"

	golangproto "github.com/golang/protobuf/proto"
)

// GetObjectKind returns the kind of an object.
func (t *TypeMeta) GetObjectKind() string {
	return t.Kind
}

// GetObjectAPIVersion returns the version of an object.
func (t *TypeMeta) GetObjectAPIVersion() string {
	return t.APIVersion
}

// GetObjectMeta returns the ObjectMeta of an object.
func (o *ObjectMeta) GetObjectMeta() *ObjectMeta {
	return o
}

// Clone clones the object into into
func (o *ObjectMeta) Clone(into interface{}) (interface{}, error) {
	return nil, fmt.Errorf("not defined")
}

// GetListMeta returns the ListMeta of a list object.
func (l *ListMeta) GetListMeta() *ListMeta {
	return l
}

// Clone clones the object into into
func (l *ListMeta) Clone(into interface{}) (interface{}, error) {
	return nil, fmt.Errorf("not defined")
}

// Clone clones the object into into
func (m *Status) Clone(into interface{}) (interface{}, error) {
	if into == nil {
		into = &Status{}
	}
	out, ok := into.(*Status)
	if !ok {
		return nil, fmt.Errorf("mismatched types")
	}
	*out = *m
	return out, nil
}

// Defaults applies defaults to the object
func (m *ListWatchOptions) Defaults(ver string) bool {
	return false
}

func init() {
	// Register this with regular golang proto so it is accessible for grpc code
	golangproto.RegisterType((*StatusResult)(nil), "api.StatusResult")
	golangproto.RegisterType((*Status)(nil), "api.Status")
}
