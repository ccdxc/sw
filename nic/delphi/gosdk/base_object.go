package gosdk

import (
	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/nic/delphi/proto/delphi"
)

var factories = make(map[string]Factory)

// Factory is a callback to be called to create objects of a specific type
type Factory func(client Client, data []byte) (BaseObject, error)

// RegisterFactory register the `Factory` callback for a given `kind`
func RegisterFactory(kind string, factory Factory) {
	factories[kind] = factory
}

// BaseObject is the interfact that has to be implemented by Delphi objects
type BaseObject interface {
	GetMeta() *delphi.ObjectMeta
	SetMeta(meta *delphi.ObjectMeta)
	GetMessage() proto.Message
	GetKeyString() string
	TriggerEvent(oldObj BaseObject, op delphi.ObjectOperation, rl []BaseReactor)
}

// BaseReactor is the reactor interfact that has to be implemented by Delphi
// objects
type BaseReactor interface {
}
