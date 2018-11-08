package clientApi

import (
	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/nic/delphi/proto/delphi"
)

// Factory is a callback to be called to create objects of a specific type
type Factory func(client Client, data []byte) (BaseObject, error)

// Factories stores the factory methods for each kind
var Factories = make(map[string]Factory)

// RegisterFactory register the `Factory` callback for a given `kind`
func RegisterFactory(kind string, factory Factory) {
	Factories[kind] = factory
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
