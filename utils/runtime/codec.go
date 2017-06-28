package runtime

import (
	"encoding/json"
	"fmt"
	"reflect"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/api"
)

// JSONCodec implements a JSON codec for an object.
type JSONCodec struct {
	creater ObjectCreater
}

// NewJSONCodec creates a new JsonCodec.
func NewJSONCodec(creater ObjectCreater) *JSONCodec {
	return &JSONCodec{creater: creater}
}

// Encode implements the json serialization of an object.
func (j *JSONCodec) Encode(obj Object) ([]byte, error) {
	return json.Marshal(obj)
}

// Decode implements the json deserialization in to an object.
func (j *JSONCodec) Decode(data []byte, into Object) (Object, error) {
	if into == nil {
		kind, err := j.findKind(data)
		if err != nil {
			return nil, err
		}
		into, err = j.creater.New(kind)
		if err != nil {
			return nil, err
		}
	}
	if err := json.Unmarshal(data, into); err != nil {
		return nil, err
	}
	return into, nil
}

// findKind finds the kind from a serialized version of an API object.
func (j *JSONCodec) findKind(data []byte) (string, error) {
	kind := struct {
		Kind       string `json:"kind,omitempty"`
		ObjectKind string `json:"ObjectKind,omitempty"`
	}{}
	if err := json.Unmarshal(data, &kind); err != nil {
		return "", err
	}

	if kind.Kind != "" {
		return kind.Kind, nil
	}
	return kind.ObjectKind, nil
}

// findKind finds the kind from a serialized version of an API object.
func (j *ProtoCodec) findKind(data []byte) (string, error) {
	var meta api.TypeMeta
	if err := proto.Unmarshal(data, &meta); err != nil {
		return "", err
	}

	return meta.Kind, nil
}

// ProtoCodec implements a protobuf codec for an object.
type ProtoCodec struct {
	creater ObjectCreater
}

// NewProtoCodec creates a new JsonCodec.
func NewProtoCodec(creater ObjectCreater) *ProtoCodec {
	return &ProtoCodec{creater: creater}
}

// Encode implements the json serialization of an object.
func (p *ProtoCodec) Encode(obj Object) ([]byte, error) {
	protoIntf, ok := reflect.ValueOf(obj).Interface().(proto.Message)
	if !ok {
		return nil, fmt.Errorf("Object is not a proto")
	}
	return proto.Marshal(protoIntf)
}

// Decode implements the json deserialization in to an object.
func (j *ProtoCodec) Decode(data []byte, into Object) (Object, error) {
	if into == nil {
		kind, err := j.findKind(data)
		if err != nil {
			return nil, err
		}
		into, err = j.creater.New(kind)
		if err != nil {
			return nil, err
		}
	}
	protoIntf, ok := reflect.ValueOf(into).Interface().(proto.Message)
	if !ok {
		return nil, fmt.Errorf("Object is not a proto")
	}
	if err := proto.Unmarshal(data, protoIntf); err != nil {
		return nil, err
	}
	return into, nil
}
