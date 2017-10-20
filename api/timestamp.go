package api

import (
	"encoding/json"
	"time"

	"github.com/gogo/protobuf/types"
)

// UnmarshalJSON implements the json.Unmarshaller interface.
func (t *Timestamp) UnmarshalJSON(b []byte) error {
	var str string
	json.Unmarshal(b, &str)

	pt, err := time.Parse(time.RFC3339, str)
	if err != nil {
		return err
	}
	ts, err := types.TimestampProto(pt)
	if err != nil {
		return err
	}
	t.Timestamp = *ts
	return nil
}

// MarshalJSON implements the json.Marshaler interface.
func (t Timestamp) MarshalJSON() ([]byte, error) {
	stdTime, err := types.TimestampFromProto(&t.Timestamp)
	if err != nil {
		return []byte("null"), err
	}
	return json.Marshal(stdTime.Format(time.RFC3339))
}

// Time returns go Time from api.Timestamp. This enables us to change the implementation later
func (t Timestamp) Time() (time.Time, error) {
	return types.TimestampFromProto(&t.Timestamp)
}
