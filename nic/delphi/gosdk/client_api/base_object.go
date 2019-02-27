package clientApi

import (
	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/nic/delphi/proto/delphi"
)

// Factory is a callback to be called to create objects of a specific type
type Factory func(client Client, data []byte) (BaseObject, error)

// Factories stores the factory methods for each kind
var Factories = make(map[string]Factory)

// ForeignKeyExtractor is a function that returns the foreign key value of an
// object
type ForeignKeyExtractor func(obj BaseObject) string

// Indexes are the list of all indexes. The first string is the From Kind,
// the second string is the field name in the From Kind, and the last
// string is the To Kind
var Indexes = make(map[string]map[string]string)

// ForeignKeyExtractors are just helper functions that given an object of
// From Kind (the first string) and the field name (second string) it gives
// us a function that we can call with the object type to get the value of
// the remote key. Sounds complicated, but it isn't. It's just a helper
// function to help us get a vlue of a field from an object.
var ForeignKeyExtractors = make(map[string]map[string]ForeignKeyExtractor)

// RegisterFactory register the `Factory` callback for a given `kind`
func RegisterFactory(kind string, factory Factory) {
	Factories[kind] = factory
}

// CreateIndex creates an index from one object to another
func CreateIndex(fromKind string, fromKindFieldName string, toKind string,
	extractor ForeignKeyExtractor) {

	i := Indexes[fromKind]
	if i == nil {
		i = make(map[string]string)
		Indexes[fromKind] = i
	}
	i[fromKindFieldName] = toKind

	fkv := ForeignKeyExtractors[fromKind]
	if fkv == nil {
		fkv = make(map[string]ForeignKeyExtractor)
		ForeignKeyExtractors[fromKind] = fkv
	}
	fkv[fromKindFieldName] = extractor
}

// BaseObject is the interfact that has to be implemented by Delphi objects
type BaseObject interface {
	GetDelphiMessage() proto.Message
	GetDelphiMeta() *delphi.ObjectMeta
	SetDelphiMeta(meta *delphi.ObjectMeta)
	GetDelphiKey() string
	GetDelphiPath() string
	GetDelphiKind() string
	DelphiClone() BaseObject
	TriggerEvent(sdkClient Client, oldObj BaseObject, op delphi.ObjectOperation,
		rl []BaseReactor)
}

// BaseReactor is the reactor interfact that has to be implemented by Delphi
// objects
type BaseReactor interface {
}
