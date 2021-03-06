// Code generated by protoc-gen-gogo. DO NOT EDIT.
// source: selector.proto

/*
	Package labels is a generated protocol buffer package.

	It is generated from these files:
		selector.proto

	It has these top-level messages:
		Selector
		Requirement
*/
package labels

import proto "github.com/gogo/protobuf/proto"
import fmt "fmt"
import math "math"
import _ "github.com/gogo/protobuf/gogoproto"
import _ "github.com/pensando/sw/venice/utils/apigen/annotations"

import io "io"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.GoGoProtoPackageIsVersion2 // please upgrade the proto package

// Operator defines the supported operators for each Requirement.
type Operator int32

const (
	Operator_equals Operator = 0
	// ui-hint: not equals
	Operator_notEquals Operator = 1
	Operator_in        Operator = 2
	// ui-hint: not in
	Operator_notIn Operator = 3
)

var Operator_name = map[int32]string{
	0: "equals",
	1: "notEquals",
	2: "in",
	3: "notIn",
}
var Operator_value = map[string]int32{
	"equals":    0,
	"notEquals": 1,
	"in":        2,
	"notIn":     3,
}

func (x Operator) String() string {
	return proto.EnumName(Operator_name, int32(x))
}
func (Operator) EnumDescriptor() ([]byte, []int) { return fileDescriptorSelector, []int{0} }

// Selector is used to select objects by labels. Requirements in the selector are ANDed.
// A selector with no Requirements does not select anything.
type Selector struct {
	// Requirements are ANDed.
	Requirements []*Requirement `protobuf:"bytes,1,rep,name=Requirements" json:"requirements,omitempty"`
}

func (m *Selector) Reset()                    { *m = Selector{} }
func (m *Selector) String() string            { return proto.CompactTextString(m) }
func (*Selector) ProtoMessage()               {}
func (*Selector) Descriptor() ([]byte, []int) { return fileDescriptorSelector, []int{0} }

func (m *Selector) GetRequirements() []*Requirement {
	if m != nil {
		return m.Requirements
	}
	return nil
}

// Requirement defines a single matching condition for a selector.
type Requirement struct {
	// The label key that the condition applies to.
	Key string `protobuf:"bytes,1,opt,name=Key,proto3" json:"key"`
	// Condition checked for the key.
	Operator string `protobuf:"bytes,2,opt,name=Operator,proto3" json:"operator"`
	// Values contains one or more values corresponding to the label key. "equals" and
	// "notEquals" operators need a single Value. "in" and "notIn" operators can have
	// one or more values.
	Values []string `protobuf:"bytes,3,rep,name=Values" json:"values"`
}

func (m *Requirement) Reset()                    { *m = Requirement{} }
func (m *Requirement) String() string            { return proto.CompactTextString(m) }
func (*Requirement) ProtoMessage()               {}
func (*Requirement) Descriptor() ([]byte, []int) { return fileDescriptorSelector, []int{1} }

func (m *Requirement) GetKey() string {
	if m != nil {
		return m.Key
	}
	return ""
}

func (m *Requirement) GetOperator() string {
	if m != nil {
		return m.Operator
	}
	return ""
}

func (m *Requirement) GetValues() []string {
	if m != nil {
		return m.Values
	}
	return nil
}

func init() {
	proto.RegisterType((*Selector)(nil), "labels.Selector")
	proto.RegisterType((*Requirement)(nil), "labels.Requirement")
	proto.RegisterEnum("labels.Operator", Operator_name, Operator_value)
}
func (m *Selector) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *Selector) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.Requirements) > 0 {
		for _, msg := range m.Requirements {
			dAtA[i] = 0xa
			i++
			i = encodeVarintSelector(dAtA, i, uint64(msg.Size()))
			n, err := msg.MarshalTo(dAtA[i:])
			if err != nil {
				return 0, err
			}
			i += n
		}
	}
	return i, nil
}

func (m *Requirement) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *Requirement) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.Key) > 0 {
		dAtA[i] = 0xa
		i++
		i = encodeVarintSelector(dAtA, i, uint64(len(m.Key)))
		i += copy(dAtA[i:], m.Key)
	}
	if len(m.Operator) > 0 {
		dAtA[i] = 0x12
		i++
		i = encodeVarintSelector(dAtA, i, uint64(len(m.Operator)))
		i += copy(dAtA[i:], m.Operator)
	}
	if len(m.Values) > 0 {
		for _, s := range m.Values {
			dAtA[i] = 0x1a
			i++
			l = len(s)
			for l >= 1<<7 {
				dAtA[i] = uint8(uint64(l)&0x7f | 0x80)
				l >>= 7
				i++
			}
			dAtA[i] = uint8(l)
			i++
			i += copy(dAtA[i:], s)
		}
	}
	return i, nil
}

func encodeVarintSelector(dAtA []byte, offset int, v uint64) int {
	for v >= 1<<7 {
		dAtA[offset] = uint8(v&0x7f | 0x80)
		v >>= 7
		offset++
	}
	dAtA[offset] = uint8(v)
	return offset + 1
}
func (m *Selector) Size() (n int) {
	var l int
	_ = l
	if len(m.Requirements) > 0 {
		for _, e := range m.Requirements {
			l = e.Size()
			n += 1 + l + sovSelector(uint64(l))
		}
	}
	return n
}

func (m *Requirement) Size() (n int) {
	var l int
	_ = l
	l = len(m.Key)
	if l > 0 {
		n += 1 + l + sovSelector(uint64(l))
	}
	l = len(m.Operator)
	if l > 0 {
		n += 1 + l + sovSelector(uint64(l))
	}
	if len(m.Values) > 0 {
		for _, s := range m.Values {
			l = len(s)
			n += 1 + l + sovSelector(uint64(l))
		}
	}
	return n
}

func sovSelector(x uint64) (n int) {
	for {
		n++
		x >>= 7
		if x == 0 {
			break
		}
	}
	return n
}
func sozSelector(x uint64) (n int) {
	return sovSelector(uint64((x << 1) ^ uint64((int64(x) >> 63))))
}
func (m *Selector) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowSelector
			}
			if iNdEx >= l {
				return io.ErrUnexpectedEOF
			}
			b := dAtA[iNdEx]
			iNdEx++
			wire |= (uint64(b) & 0x7F) << shift
			if b < 0x80 {
				break
			}
		}
		fieldNum := int32(wire >> 3)
		wireType := int(wire & 0x7)
		if wireType == 4 {
			return fmt.Errorf("proto: Selector: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: Selector: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Requirements", wireType)
			}
			var msglen int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowSelector
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				msglen |= (int(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			if msglen < 0 {
				return ErrInvalidLengthSelector
			}
			postIndex := iNdEx + msglen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Requirements = append(m.Requirements, &Requirement{})
			if err := m.Requirements[len(m.Requirements)-1].Unmarshal(dAtA[iNdEx:postIndex]); err != nil {
				return err
			}
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipSelector(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthSelector
			}
			if (iNdEx + skippy) > l {
				return io.ErrUnexpectedEOF
			}
			iNdEx += skippy
		}
	}

	if iNdEx > l {
		return io.ErrUnexpectedEOF
	}
	return nil
}
func (m *Requirement) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowSelector
			}
			if iNdEx >= l {
				return io.ErrUnexpectedEOF
			}
			b := dAtA[iNdEx]
			iNdEx++
			wire |= (uint64(b) & 0x7F) << shift
			if b < 0x80 {
				break
			}
		}
		fieldNum := int32(wire >> 3)
		wireType := int(wire & 0x7)
		if wireType == 4 {
			return fmt.Errorf("proto: Requirement: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: Requirement: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Key", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowSelector
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				stringLen |= (uint64(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			intStringLen := int(stringLen)
			if intStringLen < 0 {
				return ErrInvalidLengthSelector
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Key = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 2:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Operator", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowSelector
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				stringLen |= (uint64(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			intStringLen := int(stringLen)
			if intStringLen < 0 {
				return ErrInvalidLengthSelector
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Operator = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 3:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Values", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowSelector
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				stringLen |= (uint64(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			intStringLen := int(stringLen)
			if intStringLen < 0 {
				return ErrInvalidLengthSelector
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Values = append(m.Values, string(dAtA[iNdEx:postIndex]))
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipSelector(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthSelector
			}
			if (iNdEx + skippy) > l {
				return io.ErrUnexpectedEOF
			}
			iNdEx += skippy
		}
	}

	if iNdEx > l {
		return io.ErrUnexpectedEOF
	}
	return nil
}
func skipSelector(dAtA []byte) (n int, err error) {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return 0, ErrIntOverflowSelector
			}
			if iNdEx >= l {
				return 0, io.ErrUnexpectedEOF
			}
			b := dAtA[iNdEx]
			iNdEx++
			wire |= (uint64(b) & 0x7F) << shift
			if b < 0x80 {
				break
			}
		}
		wireType := int(wire & 0x7)
		switch wireType {
		case 0:
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return 0, ErrIntOverflowSelector
				}
				if iNdEx >= l {
					return 0, io.ErrUnexpectedEOF
				}
				iNdEx++
				if dAtA[iNdEx-1] < 0x80 {
					break
				}
			}
			return iNdEx, nil
		case 1:
			iNdEx += 8
			return iNdEx, nil
		case 2:
			var length int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return 0, ErrIntOverflowSelector
				}
				if iNdEx >= l {
					return 0, io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				length |= (int(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			iNdEx += length
			if length < 0 {
				return 0, ErrInvalidLengthSelector
			}
			return iNdEx, nil
		case 3:
			for {
				var innerWire uint64
				var start int = iNdEx
				for shift := uint(0); ; shift += 7 {
					if shift >= 64 {
						return 0, ErrIntOverflowSelector
					}
					if iNdEx >= l {
						return 0, io.ErrUnexpectedEOF
					}
					b := dAtA[iNdEx]
					iNdEx++
					innerWire |= (uint64(b) & 0x7F) << shift
					if b < 0x80 {
						break
					}
				}
				innerWireType := int(innerWire & 0x7)
				if innerWireType == 4 {
					break
				}
				next, err := skipSelector(dAtA[start:])
				if err != nil {
					return 0, err
				}
				iNdEx = start + next
			}
			return iNdEx, nil
		case 4:
			return iNdEx, nil
		case 5:
			iNdEx += 4
			return iNdEx, nil
		default:
			return 0, fmt.Errorf("proto: illegal wireType %d", wireType)
		}
	}
	panic("unreachable")
}

var (
	ErrInvalidLengthSelector = fmt.Errorf("proto: negative length found during unmarshaling")
	ErrIntOverflowSelector   = fmt.Errorf("proto: integer overflow")
)

func init() { proto.RegisterFile("selector.proto", fileDescriptorSelector) }

var fileDescriptorSelector = []byte{
	// 353 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x54, 0x90, 0xc1, 0x4e, 0xdb, 0x40,
	0x10, 0x86, 0xbb, 0xb1, 0xea, 0xc6, 0x9b, 0xb4, 0x4a, 0xb7, 0x52, 0xe5, 0xe4, 0x10, 0xa7, 0x39,
	0xb9, 0x55, 0xeb, 0x95, 0xca, 0x85, 0x0b, 0x17, 0x4b, 0x39, 0x20, 0x0e, 0x20, 0x47, 0xe2, 0x8a,
	0xd6, 0xce, 0x60, 0x56, 0xd8, 0xbb, 0x8e, 0x77, 0x37, 0x28, 0x2f, 0xc0, 0x85, 0x37, 0xe1, 0x49,
	0x38, 0xf2, 0x04, 0x16, 0xca, 0xd1, 0x4f, 0x81, 0xb0, 0x03, 0x98, 0xdb, 0x7e, 0xff, 0x37, 0x33,
	0x3b, 0x1a, 0xfc, 0x4d, 0x41, 0x06, 0x89, 0x96, 0x65, 0x50, 0x94, 0x52, 0x4b, 0x62, 0x67, 0x2c,
	0x86, 0x4c, 0x4d, 0xfe, 0xa5, 0x5c, 0x5f, 0x99, 0x38, 0x48, 0x64, 0x4e, 0x53, 0x99, 0x4a, 0xda,
	0xe8, 0xd8, 0x5c, 0x36, 0xd4, 0x40, 0xf3, 0x6a, 0xdb, 0x26, 0x8b, 0x4e, 0x79, 0x01, 0x42, 0x31,
	0xb1, 0x92, 0x54, 0xdd, 0xd0, 0x0d, 0x08, 0x9e, 0x00, 0x35, 0x9a, 0x67, 0x8a, 0xb2, 0x82, 0xa7,
	0x20, 0x28, 0x13, 0x42, 0x6a, 0xa6, 0xb9, 0x14, 0x8a, 0x72, 0x91, 0x64, 0x66, 0x05, 0xaa, 0x1d,
	0x33, 0xbf, 0xc0, 0xfd, 0xe5, 0x7e, 0x1f, 0xb2, 0xc4, 0xc3, 0x08, 0xd6, 0x86, 0x97, 0x90, 0x83,
	0xd0, 0xca, 0x45, 0x33, 0xcb, 0x1f, 0xfc, 0xff, 0x11, 0xb4, 0x0b, 0x06, 0x1d, 0x17, 0x4e, 0xea,
	0xca, 0xfb, 0x59, 0x76, 0x8a, 0xff, 0xca, 0x9c, 0x6b, 0xc8, 0x0b, 0xbd, 0x8d, 0x3e, 0x0c, 0x99,
	0xdf, 0x21, 0x3c, 0xe8, 0x04, 0x64, 0x8c, 0xad, 0x13, 0xd8, 0xba, 0x68, 0x86, 0x7c, 0x27, 0xfc,
	0x52, 0x57, 0x9e, 0x75, 0x0d, 0xdb, 0xe8, 0x25, 0x23, 0x47, 0xb8, 0x7f, 0x5a, 0x40, 0xc9, 0xb4,
	0x2c, 0xdd, 0x5e, 0xe3, 0x7f, 0xdd, 0xdf, 0x8e, 0xbf, 0x2f, 0x75, 0xb9, 0x10, 0x26, 0xf7, 0x5f,
	0xdd, 0xef, 0xba, 0xf2, 0xfa, 0x72, 0x0f, 0xd1, 0x5b, 0x0b, 0x99, 0x63, 0xfb, 0x9c, 0x65, 0x06,
	0x94, 0x6b, 0xcd, 0x2c, 0xdf, 0x09, 0x71, 0x5d, 0x79, 0xf6, 0xa6, 0x49, 0xa2, 0xbd, 0xf9, 0x73,
	0xf8, 0xfe, 0x05, 0xc1, 0xd8, 0x86, 0xb5, 0x61, 0x99, 0x1a, 0x7d, 0x22, 0x5f, 0xb1, 0x23, 0xa4,
	0x5e, 0xb4, 0x88, 0x88, 0x8d, 0x7b, 0x5c, 0x8c, 0x7a, 0xc4, 0xc1, 0x9f, 0x85, 0xd4, 0xc7, 0x62,
	0x64, 0x85, 0xc3, 0x87, 0xdd, 0x14, 0x3d, 0xee, 0xa6, 0xe8, 0x69, 0x37, 0x45, 0x67, 0x28, 0xb6,
	0x9b, 0xfb, 0x1d, 0x3c, 0x07, 0x00, 0x00, 0xff, 0xff, 0x43, 0x66, 0xe7, 0x3f, 0xcf, 0x01, 0x00,
	0x00,
}
