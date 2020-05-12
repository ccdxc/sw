package api

import (
	"encoding/binary"
	"encoding/json"
	"errors"
	"math"
	"net"
	"strconv"
	"strings"
)

const (
	// IPFormatRD specifies that RD admin value is in IP format
	IPFormatRD = "IP"
	// ASNFormatRD specifies that RD admin value is in ASN Format
	ASNFormatRD = "ASN"
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

// UnmarshalJSON implements the json.Unmarshaller interface.
func (t *RDAdminValue) UnmarshalJSON(b []byte) error {
	/* Route Distinguisher Admin Value by type
	Type 0: 2 byte ASN
	Type 1: 4 byte IP Address
	Type 2: 4 byte ASN
	*/
	var ret uint32
	var format string

	var asn BgpAsn
	err := json.Unmarshal(b, &asn)
	if err == nil {
		format = ASNFormatRD
		ret = asn.ASNumber
	} else {
		var asStr string
		err = json.Unmarshal(b, &asStr)
		if err != nil {
			return errors.New("Route Distinguisher Admin Value should be either an ASN or an ip address")
		}
		ip := net.ParseIP(asStr)
		if ip != nil {
			format = IPFormatRD
			ret = ipToInt(ip)
		} else {
			return errors.New("Route Distinguisher Admin Value should be either an ASN or an ip address")
		}
	}

	t.Format = format
	t.Value = ret
	return nil
}

// MarshalJSON implements the json.Marshaler interface.
func (t *RDAdminValue) MarshalJSON() ([]byte, error) {
	if t.Format == IPFormatRD {
		ip := intToIP(t.Value)
		return json.Marshal(ip.String())
	}
	// Else the format is ASN
	return json.Marshal(&BgpAsn{ASNumber: t.Value})
}

func ipToInt(ip net.IP) uint32 {
	if len(ip) == 16 {
		return binary.BigEndian.Uint32(ip[12:16])
	}
	return binary.BigEndian.Uint32(ip)
}

func intToIP(nn uint32) net.IP {
	ip := make(net.IP, 4)
	binary.BigEndian.PutUint32(ip, nn)
	return ip
}
