package api

import (
	"encoding/json"

	"github.com/gogo/protobuf/types"
)

// MarshalJSON implements the json.Marshaler interface.
func (t *Any) MarshalJSON() ([]byte, error) {
	robj := &types.DynamicAny{}
	err := types.UnmarshalAny(&t.Any, robj)
	if err != nil {
		return nil, err
	}
	return json.Marshal(robj.Message)
}
