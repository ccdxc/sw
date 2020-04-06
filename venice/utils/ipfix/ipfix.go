package ipfix

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"time"

	"github.com/calmh/ipfix"
)

// PensandoEnterpriseID is the enterprise id assigned https://www.iana.org/assignments/enterprise-numbers/enterprise-numbers
const PensandoEnterpriseID = 51886

// GenerateEnterpriseElements generates help to decode elements,
// based on https://github.com/pensando/sw/blob/master/dol/infra/penscapy/penscapy.py
func GenerateEnterpriseElements() []ipfix.DictionaryEntry {
	// element id scheme
	// ip		:	0x02xx
	// non-ip	:	0x03xx
	// ipv4		:	0x04xx
	// ip-common:	0x05xx
	// ipv6		:	0x06xx

	return []ipfix.DictionaryEntry{
		// ip
		{

			Name:         "flow-state-index",
			FieldID:      0x0201,
			EnterpriseID: PensandoEnterpriseID,
			Type:         ipfix.OctetArray,
		},
		{
			Name:         "role", //todo: check
			FieldID:      0x0202,
			EnterpriseID: PensandoEnterpriseID,
			Type:         ipfix.Uint8,
		},
		{
			Name:         "tcp-state",
			FieldID:      0x0203,
			EnterpriseID: PensandoEnterpriseID,
			Type:         ipfix.Uint8,
		},
		{
			Name:         "tcp-win-mss", // todo: check
			FieldID:      0x0204,
			EnterpriseID: PensandoEnterpriseID,
			Type:         ipfix.Uint16,
		},
		{
			Name:         "tcp-exceptions",
			FieldID:      0x0205,
			EnterpriseID: PensandoEnterpriseID,
			Type:         ipfix.Uint32,
		},
		{
			Name:         "tcp-rtt",
			FieldID:      0x0206,
			EnterpriseID: PensandoEnterpriseID,
			Type:         ipfix.Uint32,
		},

		// ip common
		{
			Name:         "egress-interface-type",
			FieldID:      0x0501,
			EnterpriseID: PensandoEnterpriseID,
			Type:         ipfix.Uint16,
		},
		{
			Name:         "egress-logical-interface",
			FieldID:      0x0502,
			EnterpriseID: PensandoEnterpriseID,
			Type:         ipfix.Uint16,
		},
		{
			Name:         "drop-vector",
			FieldID:      0x0506,
			EnterpriseID: PensandoEnterpriseID,
			Type:         ipfix.Uint64,
		},
	}
}

const setHdrLen = 2 + 2
const ipfixHdrLen = 2 + 2 + 4 + 4 + 4

// setHeader ipfix set header
type setHeader struct {
	SetID  uint16
	Length uint16
}

// templateHeader ipfix template header
type templateHeader struct {
	TemplateID uint16
	FieldCount uint16
}

// CreateTemplateMsg creates a ipfix template message
func CreateTemplateMsg() ([]byte, error) {
	tmplt, err := encodeTemplateRecord()
	if err != nil {
		return nil, err
	}

	setHdr, err := encodeSetHeader(len(tmplt))
	if err != nil {
		return nil, err
	}

	ipfixHdr, err := encodeIpfixHeader(len(setHdr) + len(tmplt))
	if err != nil {
		return nil, err
	}

	msg := append(ipfixHdr, setHdr...)
	msg = append(msg, tmplt...)
	return msg, nil
}

// encode ipfix header
func encodeIpfixHeader(l int) ([]byte, error) {
	buff := &bytes.Buffer{}

	hdr := &ipfix.MessageHeader{
		DomainID:       0, // todo: check the unique id here
		Length:         uint16(l + ipfixHdrLen),
		ExportTime:     uint32(time.Now().Unix()),
		SequenceNumber: 0, // no sequence
		Version:        0x0a,
	}

	if err := binary.Write(buff, binary.BigEndian, hdr); err != nil {
		return nil, fmt.Errorf("failed to encode set header, %v", err)
	}
	return buff.Bytes(), nil
}

// encode ipfix set header
func encodeSetHeader(l int) ([]byte, error) {
	buff := &bytes.Buffer{}

	sh := &setHeader{
		SetID:  2,
		Length: uint16(l + setHdrLen),
	}
	if err := binary.Write(buff, binary.BigEndian, sh); err != nil {
		return nil, fmt.Errorf("failed to encode set header, %v", err)
	}
	return buff.Bytes(), nil
}

// encode ipfix template header
func encodeTemplateRecord() ([]byte, error) {
	buff := &bytes.Buffer{}
	trList := GeneratePensandoTemplates()

	for _, tr := range trList {
		tmpltHdr := &templateHeader{
			TemplateID: tr.TemplateID,
			FieldCount: uint16(len(tr.FieldSpecifiers)),
		}
		if err := binary.Write(buff, binary.BigEndian, tmpltHdr); err != nil {
			return nil, fmt.Errorf("failed to encode template header, %v", err)
		}

		for _, f := range tr.FieldSpecifiers {
			switch f.EnterpriseID {
			// IANA field
			case 0:
				data := &struct {
					elementID uint16
					length    uint16
				}{
					elementID: f.FieldID & 0x7ff,
					length:    f.Length,
				}

				if err := binary.Write(buff, binary.BigEndian, data); err != nil {
					return nil, fmt.Errorf("failed to encode %+v, %v", f, err)
				}

			// enterprise field
			default:
				data := &struct {
					elementID    uint16
					length       uint16
					enterpriseID uint32
				}{
					elementID:    f.FieldID | 0x8000,
					length:       f.Length,
					enterpriseID: PensandoEnterpriseID,
				}
				if err := binary.Write(buff, binary.BigEndian, data); err != nil {
					return nil, fmt.Errorf("failed to encode %+v, %v", f, err)
				}
			}
		}
	}

	return buff.Bytes(), nil
}

// GeneratePensandoTemplates generates ipfix template
// genarted from https://github.com/pensando/sw/blob/master/dol/infra/penscapy/penscapy.py, 05/23/2019
func GeneratePensandoTemplates() []ipfix.TemplateRecord {
	ipfixIpv4Fields := []ipfix.TemplateFieldSpecifier{
		// ipv4 - 17 bytes
		{
			EnterpriseID: 0,
			FieldID:      234,
			Length:       4,
		},
		{
			EnterpriseID: 0,
			FieldID:      8,
			Length:       4,
		},
		{
			EnterpriseID: 0,
			FieldID:      12,
			Length:       4,
		},
		{
			EnterpriseID: 0,
			FieldID:      4,
			Length:       1,
		},

		{
			EnterpriseID: 0,
			FieldID:      7,
			Length:       2,
		},
		{
			EnterpriseID: 0,
			FieldID:      11,
			Length:       2,
		},
	}

	ipfixIpv6Fields := []ipfix.TemplateFieldSpecifier{
		// 42 bytes
		{
			EnterpriseID: 0,
			FieldID:      234,
			Length:       4,
		},
		{
			EnterpriseID: 0,
			FieldID:      27,
			Length:       16,
		},
		{
			EnterpriseID: 0,
			FieldID:      28,
			Length:       16,
		},
		{
			EnterpriseID: 0,
			FieldID:      4,
			Length:       1,
		},
		{
			EnterpriseID: 0,
			FieldID:      7,
			Length:       2,
		},
		{
			EnterpriseID: 0,
			FieldID:      11,
			Length:       2,
		},
		{
			EnterpriseID: 0,
			FieldID:      210,
			Length:       1,
		},
	}

	ipfixIPFields := []ipfix.TemplateFieldSpecifier{
		// ip - 31 bytes
		{
			EnterpriseID: PensandoEnterpriseID,
			FieldID:      0x0201,
			Length:       3,
		},

		{
			EnterpriseID: PensandoEnterpriseID,
			FieldID:      0x0202,
			Length:       1,
		},
		{
			EnterpriseID: 0,
			FieldID:      192,
			Length:       1,
		},
		{
			EnterpriseID: 0,
			FieldID:      32,
			Length:       2,
		},

		{
			EnterpriseID: 0,
			FieldID:      184,
			Length:       4,
		},
		{
			EnterpriseID: 0,
			FieldID:      185,
			Length:       4,
		},

		{
			EnterpriseID: 0,
			FieldID:      186,
			Length:       2,
		},

		{
			EnterpriseID: 0,
			FieldID:      238,
			Length:       1,
		},

		{
			EnterpriseID: PensandoEnterpriseID,
			FieldID:      0x0203,
			Length:       1,
		},

		{
			EnterpriseID: PensandoEnterpriseID,
			FieldID:      0x0204,
			Length:       2,
		},
		{
			EnterpriseID: PensandoEnterpriseID,
			FieldID:      0x0205,
			Length:       4,
		},
		{
			EnterpriseID: 0,
			FieldID:      210,
			Length:       2,
		},
		{
			EnterpriseID: PensandoEnterpriseID,
			FieldID:      0x0206,
			Length:       4,
		},
	}

	ipfixIPCommonFields := []ipfix.TemplateFieldSpecifier{
		// ip common 84 bytes
		{
			EnterpriseID: 0,
			FieldID:      148,
			Length:       4,
		},

		{
			EnterpriseID: PensandoEnterpriseID,
			FieldID:      0x0502,
			Length:       2,
		},
		{
			EnterpriseID: PensandoEnterpriseID,
			FieldID:      0x0501,
			Length:       2,
		},

		{
			EnterpriseID: 0,
			FieldID:      86,
			Length:       8,
		},
		{
			EnterpriseID: 0,
			FieldID:      85,
			Length:       8,
		},
		{
			EnterpriseID: 0,
			FieldID:      2,
			Length:       4,
		},
		{
			EnterpriseID: 0,
			FieldID:      1,
			Length:       4,
		},

		{
			EnterpriseID: 0,
			FieldID:      135,
			Length:       8,
		},

		{
			EnterpriseID: 0,
			FieldID:      134,
			Length:       8,
		},
		{
			EnterpriseID: 0,
			FieldID:      133,
			Length:       4,
		},
		{
			EnterpriseID: 0,
			FieldID:      132,
			Length:       4,
		},
		{
			EnterpriseID: 0,
			FieldID:      210,
			Length:       4,
		},

		{
			EnterpriseID: 0,
			FieldID:      152,
			Length:       8,
		},
		{
			EnterpriseID: 0,
			FieldID:      153,
			Length:       8,
		},
		{
			EnterpriseID: PensandoEnterpriseID,
			FieldID:      0x0506,
			Length:       8,
		},
	}

	ipfixNonIPFields := []ipfix.TemplateFieldSpecifier{
		// 18 bytes
		{
			EnterpriseID: 0,
			FieldID:      234,
			Length:       4,
		},
		{
			EnterpriseID: 0,
			FieldID:      56,
			Length:       6,
		},
		{
			EnterpriseID: 0,
			FieldID:      80,
			Length:       6,
		},
		{
			EnterpriseID: 0,
			FieldID:      256,
			Length:       2,
		},
	}

	// ipv4 template
	ipv4Template := ipfixIpv4Fields
	ipv4Template = append(ipv4Template, ipfixIPFields...)
	ipv4Template = append(ipv4Template, ipfixIPCommonFields...)

	// ipv6 template
	ipv6Template := ipfixIpv6Fields
	ipv6Template = append(ipv6Template, ipfixIPFields...)
	ipv6Template = append(ipv6Template, ipfixIPCommonFields...)

	// non-ip templte
	nonIPTemplate := ipfixNonIPFields
	nonIPTemplate = append(nonIPTemplate, ipfixIPCommonFields...)

	return []ipfix.TemplateRecord{
		{
			TemplateID:      257,
			FieldSpecifiers: ipv4Template,
		},
		{
			TemplateID:      258,
			FieldSpecifiers: ipv6Template,
		},
		{
			TemplateID:      259,
			FieldSpecifiers: nonIPTemplate,
		},
	}
}
