package runtime

import (
	"encoding/json"
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
		Kind string `json:"kind,omitempty"`
	}{}
	if err := json.Unmarshal(data, &kind); err != nil {
		return "", err
	}
	return kind.Kind, nil
}
