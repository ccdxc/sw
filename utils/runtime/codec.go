package runtime

import (
	"encoding/json"
)

// JSONCodec implements a JSON codec for an object.
type JSONCodec struct {
}

// NewJSONCodec creates a new JsonCodec.
func NewJSONCodec() *JSONCodec {
	return &JSONCodec{}
}

// Encode implements the json serialization of an object.
func (j *JSONCodec) Encode(obj Object) ([]byte, error) {
	return json.Marshal(obj)
}

// Decode implements the json deserialization in to an object.
func (j *JSONCodec) Decode(data []byte, into Object) error {
	return json.Unmarshal(data, into)
}
