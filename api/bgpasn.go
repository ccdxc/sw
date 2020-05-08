package api

import (
	"encoding/json"
	"errors"
	"math"
	"strconv"
	"strings"
)

// UnmarshalJSON implements the json.Unmarshaller interface.
func (t *BgpAsn) UnmarshalJSON(b []byte) error {
	var ret uint32

	err := json.Unmarshal(b, &ret)
	if err != nil {
		var asStr string
		err = json.Unmarshal(b, &asStr)
		if err != nil {
			return errors.New("ASN should either be specified as a integer or in dotted notation as a string")
		}

		dottedParts := strings.Split(asStr, ".")
		if len(dottedParts) > 2 {
			return errors.New("ASN should either be specified as a integer or in dotted notation as a string")
		}

		// Handles 2 byte ASN specified as a string
		if len(dottedParts) == 1 {
			num, err := strconv.Atoi(dottedParts[0])
			if err != nil {
				return errors.New("ASN should either be specified as a integer or in dotted notation as a string")
			}

			if uint64(num) > uint64(math.MaxUint32) || num < 0 {
				return errors.New("ASN should either be specified as a integer or in dotted notation as a string")
			}
			ret = uint32(num)
		}

		// Handles 4 byte ASN in dotted notation
		if len(dottedParts) == 2 {
			higherBytesAsInt, err := strconv.Atoi(dottedParts[0])
			if err != nil {
				return errors.New("ASN should either be specified as a integer or in dotted notation as a string")
			}
			lowerBytesAsInt, err := strconv.Atoi(dottedParts[1])
			if err != nil {
				return errors.New("ASN should either be specified as a integer or in dotted notation as a string")
			}

			// Add uint range check
			if higherBytesAsInt > math.MaxUint16 || higherBytesAsInt < 0 || lowerBytesAsInt > math.MaxUint16 || lowerBytesAsInt < 0 {
				return errors.New("ASN should either be specified as a integer or in dotted notation as a string")
			}

			ret = uint32(higherBytesAsInt)*65535 + uint32(lowerBytesAsInt)
		}
	}

	t.ASNumber = ret
	return nil
}

// MarshalJSON implements the json.Marshaler interface.
func (t *BgpAsn) MarshalJSON() ([]byte, error) {
	if t.ASNumber <= math.MaxUint16 {
		return json.Marshal(t.ASNumber)
	}

	higherBytes := t.ASNumber / 65535
	lowerBytes := t.ASNumber % 65535
	dottedASN := strconv.Itoa(int(higherBytes)) + "." + strconv.Itoa(int(lowerBytes))

	return json.Marshal(dottedASN)
}
