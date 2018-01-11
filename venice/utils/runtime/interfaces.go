package runtime

import (
	"github.com/pensando/sw/api"
)

// Object is an interface that need to be implemented by all API objects.
type Object interface {
	// Returns the kind of the object.
	GetObjectKind() string

	// Returns the API version of the object.
	GetObjectAPIVersion() string

	// Clones the Object
	Clone(into interface{}) error
}

// ObjectMetaAccessor allows for ObjectMeta access from Object.
type ObjectMetaAccessor interface {
	GetObjectMeta() *api.ObjectMeta
}

// ListMetaAccessor allows for ListMeta access from a List Object.
type ListMetaAccessor interface {
	GetListMeta() *api.ListMeta
}

// Codec defines the interface for serialization, deserialization of objects.
type Codec interface {
	// Encode serializes the object.
	Encode(obj Object) ([]byte, error)

	// Decode deserializes into the provided object. If no object is provided,
	// a new Object is created.
	Decode(data []byte, into Object) (Object, error)
}

// Versioner allows the kvstore to set and retrieve resource versions in the object.
type Versioner interface {
	// SetVersion sets the provided resource version in the object if its an
	// API object, errors out otherwise.
	SetVersion(obj Object, resourceVersion uint64) error

	// GetVersion returns the resource version in the object if its an API
	// object, error otherwise.
	GetVersion(obj Object) (uint64, error)
}

// ObjectCreater defines the interface for creating an Object, given a kind.
type ObjectCreater interface {
	New(kind string) (Object, error)
}
