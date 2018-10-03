package api

import (
	"encoding/json"
	"errors"
)

// MarshalJSON implements the json.Marshaler interface.
func (t *InterfaceSlice) MarshalJSON() ([]byte, error) {
	var ret []interface{}
	for i := range t.Values {
		switch t.Values[i].GetValue().(type) {
		case *Interface_Str:
			ret = append(ret, t.Values[i].GetStr())
		case *Interface_Int64:
			ret = append(ret, t.Values[i].GetInt64())
		case *Interface_Bool:
			ret = append(ret, t.Values[i].GetBool())
		case *Interface_Float:
			ret = append(ret, t.Values[i].GetFloat())
		case *Interface_Interfaces:
			ret = append(ret, t.Values[i].GetInterfaces())
		case nil:
			ret = append(ret, nil)
		}
	}
	return json.Marshal(ret)
}

// MarshalJSON implements the json.Marshaler interface.
func (t *Interface) MarshalJSON() ([]byte, error) {
	switch t.Value.(type) {
	case *Interface_Str:
		return json.Marshal(t.GetStr())
	case *Interface_Int64:
		return json.Marshal(t.GetInt64())
	case *Interface_Bool:
		return json.Marshal(t.GetBool())
	case *Interface_Float:
		return json.Marshal(t.GetFloat())
	case *Interface_Interfaces:
		return json.Marshal(t.GetInterfaces())
	}
	return nil, errors.New("invalid type")
}
