// Code generated by protoc-gen-gogo. DO NOT EDIT.
// source: pdsa.proto

/*
	Package venice is a generated protocol buffer package.

	It is generated from these files:
		pdsa.proto

	It has these top-level messages:
		GlobalOpts
		PdsaFields
*/
package venice

import proto "github.com/gogo/protobuf/proto"
import fmt "fmt"
import math "math"
import google_protobuf "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"

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

type GlobalOpts struct {
	OidLen string `protobuf:"bytes,1,opt,name=OidLen,proto3" json:"OidLen,omitempty"`
	Mib    string `protobuf:"bytes,2,opt,name=Mib,proto3" json:"Mib,omitempty"`
	FillFn string `protobuf:"bytes,3,opt,name=FillFn,proto3" json:"FillFn,omitempty"`
}

func (m *GlobalOpts) Reset()                    { *m = GlobalOpts{} }
func (m *GlobalOpts) String() string            { return proto.CompactTextString(m) }
func (*GlobalOpts) ProtoMessage()               {}
func (*GlobalOpts) Descriptor() ([]byte, []int) { return fileDescriptorPdsa, []int{0} }

func (m *GlobalOpts) GetOidLen() string {
	if m != nil {
		return m.OidLen
	}
	return ""
}

func (m *GlobalOpts) GetMib() string {
	if m != nil {
		return m.Mib
	}
	return ""
}

func (m *GlobalOpts) GetFillFn() string {
	if m != nil {
		return m.FillFn
	}
	return ""
}

type PdsaFields struct {
	Field             string `protobuf:"bytes,1,opt,name=Field,proto3" json:"Field,omitempty"`
	SetFieldFn        string `protobuf:"bytes,2,opt,name=SetFieldFn,proto3" json:"SetFieldFn,omitempty"`
	GetFieldFn        string `protobuf:"bytes,3,opt,name=GetFieldFn,proto3" json:"GetFieldFn,omitempty"`
	SetKeyOidIndex    string `protobuf:"bytes,4,opt,name=SetKeyOidIndex,proto3" json:"SetKeyOidIndex,omitempty"`
	GetKeyOidIndex    string `protobuf:"bytes,5,opt,name=GetKeyOidIndex,proto3" json:"GetKeyOidIndex,omitempty"`
	SetKeyOidFn       string `protobuf:"bytes,6,opt,name=SetKeyOidFn,proto3" json:"SetKeyOidFn,omitempty"`
	GetKeyOidFn       string `protobuf:"bytes,7,opt,name=GetKeyOidFn,proto3" json:"GetKeyOidFn,omitempty"`
	SetKeyOidLenIndex string `protobuf:"bytes,8,opt,name=SetKeyOidLenIndex,proto3" json:"SetKeyOidLenIndex,omitempty"`
	GetKeyOidLenIndex string `protobuf:"bytes,9,opt,name=GetKeyOidLenIndex,proto3" json:"GetKeyOidLenIndex,omitempty"`
}

func (m *PdsaFields) Reset()                    { *m = PdsaFields{} }
func (m *PdsaFields) String() string            { return proto.CompactTextString(m) }
func (*PdsaFields) ProtoMessage()               {}
func (*PdsaFields) Descriptor() ([]byte, []int) { return fileDescriptorPdsa, []int{1} }

func (m *PdsaFields) GetField() string {
	if m != nil {
		return m.Field
	}
	return ""
}

func (m *PdsaFields) GetSetFieldFn() string {
	if m != nil {
		return m.SetFieldFn
	}
	return ""
}

func (m *PdsaFields) GetGetFieldFn() string {
	if m != nil {
		return m.GetFieldFn
	}
	return ""
}

func (m *PdsaFields) GetSetKeyOidIndex() string {
	if m != nil {
		return m.SetKeyOidIndex
	}
	return ""
}

func (m *PdsaFields) GetGetKeyOidIndex() string {
	if m != nil {
		return m.GetKeyOidIndex
	}
	return ""
}

func (m *PdsaFields) GetSetKeyOidFn() string {
	if m != nil {
		return m.SetKeyOidFn
	}
	return ""
}

func (m *PdsaFields) GetGetKeyOidFn() string {
	if m != nil {
		return m.GetKeyOidFn
	}
	return ""
}

func (m *PdsaFields) GetSetKeyOidLenIndex() string {
	if m != nil {
		return m.SetKeyOidLenIndex
	}
	return ""
}

func (m *PdsaFields) GetGetKeyOidLenIndex() string {
	if m != nil {
		return m.GetKeyOidLenIndex
	}
	return ""
}

var E_PdsaSetGlobOpts = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.MessageOptions)(nil),
	ExtensionType: (*GlobalOpts)(nil),
	Field:         82400,
	Name:          "venice.pdsaSetGlobOpts",
	Tag:           "bytes,82400,opt,name=pdsaSetGlobOpts",
	Filename:      "pdsa.proto",
}

var E_PdsaGetGlobOpts = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.MessageOptions)(nil),
	ExtensionType: (*GlobalOpts)(nil),
	Field:         82401,
	Name:          "venice.pdsaGetGlobOpts",
	Tag:           "bytes,82401,opt,name=pdsaGetGlobOpts",
	Filename:      "pdsa.proto",
}

var E_PdsaFields = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.FieldOptions)(nil),
	ExtensionType: (*PdsaFields)(nil),
	Field:         83400,
	Name:          "venice.pdsaFields",
	Tag:           "bytes,83400,opt,name=pdsaFields",
	Filename:      "pdsa.proto",
}

func init() {
	proto.RegisterType((*GlobalOpts)(nil), "venice.GlobalOpts")
	proto.RegisterType((*PdsaFields)(nil), "venice.PdsaFields")
	proto.RegisterExtension(E_PdsaSetGlobOpts)
	proto.RegisterExtension(E_PdsaGetGlobOpts)
	proto.RegisterExtension(E_PdsaFields)
}
func (m *GlobalOpts) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *GlobalOpts) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.OidLen) > 0 {
		dAtA[i] = 0xa
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.OidLen)))
		i += copy(dAtA[i:], m.OidLen)
	}
	if len(m.Mib) > 0 {
		dAtA[i] = 0x12
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.Mib)))
		i += copy(dAtA[i:], m.Mib)
	}
	if len(m.FillFn) > 0 {
		dAtA[i] = 0x1a
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.FillFn)))
		i += copy(dAtA[i:], m.FillFn)
	}
	return i, nil
}

func (m *PdsaFields) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *PdsaFields) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.Field) > 0 {
		dAtA[i] = 0xa
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.Field)))
		i += copy(dAtA[i:], m.Field)
	}
	if len(m.SetFieldFn) > 0 {
		dAtA[i] = 0x12
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.SetFieldFn)))
		i += copy(dAtA[i:], m.SetFieldFn)
	}
	if len(m.GetFieldFn) > 0 {
		dAtA[i] = 0x1a
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.GetFieldFn)))
		i += copy(dAtA[i:], m.GetFieldFn)
	}
	if len(m.SetKeyOidIndex) > 0 {
		dAtA[i] = 0x22
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.SetKeyOidIndex)))
		i += copy(dAtA[i:], m.SetKeyOidIndex)
	}
	if len(m.GetKeyOidIndex) > 0 {
		dAtA[i] = 0x2a
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.GetKeyOidIndex)))
		i += copy(dAtA[i:], m.GetKeyOidIndex)
	}
	if len(m.SetKeyOidFn) > 0 {
		dAtA[i] = 0x32
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.SetKeyOidFn)))
		i += copy(dAtA[i:], m.SetKeyOidFn)
	}
	if len(m.GetKeyOidFn) > 0 {
		dAtA[i] = 0x3a
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.GetKeyOidFn)))
		i += copy(dAtA[i:], m.GetKeyOidFn)
	}
	if len(m.SetKeyOidLenIndex) > 0 {
		dAtA[i] = 0x42
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.SetKeyOidLenIndex)))
		i += copy(dAtA[i:], m.SetKeyOidLenIndex)
	}
	if len(m.GetKeyOidLenIndex) > 0 {
		dAtA[i] = 0x4a
		i++
		i = encodeVarintPdsa(dAtA, i, uint64(len(m.GetKeyOidLenIndex)))
		i += copy(dAtA[i:], m.GetKeyOidLenIndex)
	}
	return i, nil
}

func encodeVarintPdsa(dAtA []byte, offset int, v uint64) int {
	for v >= 1<<7 {
		dAtA[offset] = uint8(v&0x7f | 0x80)
		v >>= 7
		offset++
	}
	dAtA[offset] = uint8(v)
	return offset + 1
}
func (m *GlobalOpts) Size() (n int) {
	var l int
	_ = l
	l = len(m.OidLen)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	l = len(m.Mib)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	l = len(m.FillFn)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	return n
}

func (m *PdsaFields) Size() (n int) {
	var l int
	_ = l
	l = len(m.Field)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	l = len(m.SetFieldFn)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	l = len(m.GetFieldFn)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	l = len(m.SetKeyOidIndex)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	l = len(m.GetKeyOidIndex)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	l = len(m.SetKeyOidFn)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	l = len(m.GetKeyOidFn)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	l = len(m.SetKeyOidLenIndex)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	l = len(m.GetKeyOidLenIndex)
	if l > 0 {
		n += 1 + l + sovPdsa(uint64(l))
	}
	return n
}

func sovPdsa(x uint64) (n int) {
	for {
		n++
		x >>= 7
		if x == 0 {
			break
		}
	}
	return n
}
func sozPdsa(x uint64) (n int) {
	return sovPdsa(uint64((x << 1) ^ uint64((int64(x) >> 63))))
}
func (m *GlobalOpts) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowPdsa
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
			return fmt.Errorf("proto: GlobalOpts: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: GlobalOpts: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field OidLen", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.OidLen = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 2:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Mib", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Mib = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 3:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field FillFn", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.FillFn = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipPdsa(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthPdsa
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
func (m *PdsaFields) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowPdsa
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
			return fmt.Errorf("proto: PdsaFields: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: PdsaFields: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Field", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Field = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 2:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field SetFieldFn", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.SetFieldFn = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 3:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field GetFieldFn", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.GetFieldFn = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 4:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field SetKeyOidIndex", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.SetKeyOidIndex = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 5:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field GetKeyOidIndex", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.GetKeyOidIndex = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 6:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field SetKeyOidFn", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.SetKeyOidFn = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 7:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field GetKeyOidFn", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.GetKeyOidFn = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 8:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field SetKeyOidLenIndex", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.SetKeyOidLenIndex = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 9:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field GetKeyOidLenIndex", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowPdsa
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
				return ErrInvalidLengthPdsa
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.GetKeyOidLenIndex = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipPdsa(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthPdsa
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
func skipPdsa(dAtA []byte) (n int, err error) {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return 0, ErrIntOverflowPdsa
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
					return 0, ErrIntOverflowPdsa
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
					return 0, ErrIntOverflowPdsa
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
				return 0, ErrInvalidLengthPdsa
			}
			return iNdEx, nil
		case 3:
			for {
				var innerWire uint64
				var start int = iNdEx
				for shift := uint(0); ; shift += 7 {
					if shift >= 64 {
						return 0, ErrIntOverflowPdsa
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
				next, err := skipPdsa(dAtA[start:])
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
	ErrInvalidLengthPdsa = fmt.Errorf("proto: negative length found during unmarshaling")
	ErrIntOverflowPdsa   = fmt.Errorf("proto: integer overflow")
)

func init() { proto.RegisterFile("pdsa.proto", fileDescriptorPdsa) }

var fileDescriptorPdsa = []byte{
	// 364 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x94, 0x92, 0xc1, 0x4a, 0xeb, 0x40,
	0x14, 0x86, 0x49, 0x7b, 0x9b, 0x7b, 0x7b, 0x7a, 0xb9, 0x57, 0x07, 0x91, 0x20, 0x18, 0x4b, 0x17,
	0xe2, 0x42, 0x52, 0xd0, 0x5d, 0x97, 0x2e, 0x12, 0xc4, 0xd6, 0x48, 0xf3, 0x00, 0x92, 0x34, 0xc7,
	0x32, 0x10, 0x66, 0x42, 0x67, 0x14, 0x5d, 0xbb, 0xf4, 0xc5, 0xba, 0x14, 0x9f, 0xa0, 0xf6, 0x49,
	0x64, 0x66, 0xd2, 0x76, 0xda, 0x2c, 0xc4, 0xdd, 0x39, 0xff, 0xf9, 0xe6, 0x3f, 0x0c, 0xff, 0x01,
	0x28, 0x73, 0x91, 0x06, 0xe5, 0x8c, 0x4b, 0x4e, 0xdc, 0x27, 0x64, 0x74, 0x82, 0x47, 0xdd, 0x29,
	0xe7, 0xd3, 0x02, 0xfb, 0x5a, 0xcd, 0x1e, 0x1f, 0xfa, 0x39, 0x8a, 0xc9, 0x8c, 0x96, 0x92, 0xcf,
	0x0c, 0xd9, 0xbb, 0x05, 0x88, 0x0a, 0x9e, 0xa5, 0x45, 0x5c, 0x4a, 0x41, 0x0e, 0xc1, 0x8d, 0x69,
	0x3e, 0x44, 0xe6, 0x39, 0x5d, 0xe7, 0xac, 0x3d, 0xae, 0x3a, 0xb2, 0x07, 0xcd, 0x11, 0xcd, 0xbc,
	0x86, 0x16, 0x55, 0xa9, 0xc8, 0x90, 0x16, 0x45, 0xc8, 0xbc, 0xa6, 0x21, 0x4d, 0xd7, 0xfb, 0x68,
	0x00, 0xdc, 0xe5, 0x22, 0x0d, 0x29, 0x16, 0xb9, 0x20, 0x07, 0xd0, 0xd2, 0x55, 0xe5, 0x67, 0x1a,
	0xe2, 0x03, 0x24, 0x28, 0x75, 0x1d, 0xb2, 0xca, 0xd5, 0x52, 0xd4, 0x3c, 0xda, 0xcc, 0xcd, 0x02,
	0x4b, 0x21, 0xa7, 0xf0, 0x2f, 0x41, 0x79, 0x83, 0x2f, 0x31, 0xcd, 0xaf, 0x59, 0x8e, 0xcf, 0xde,
	0x2f, 0xcd, 0xec, 0xa8, 0x8a, 0x8b, 0xb6, 0xb9, 0x96, 0xe1, 0xb6, 0x55, 0xd2, 0x85, 0xce, 0xfa,
	0x65, 0xc8, 0x3c, 0x57, 0x43, 0xb6, 0xa4, 0x88, 0xc8, 0x22, 0x7e, 0x1b, 0xc2, 0x92, 0xc8, 0x39,
	0xec, 0xaf, 0x1f, 0x0c, 0x91, 0x99, 0x75, 0x7f, 0x34, 0x57, 0x1f, 0x28, 0x3a, 0xaa, 0xd1, 0x6d,
	0x43, 0xd7, 0x06, 0x83, 0x7b, 0xf8, 0xaf, 0xc2, 0x4d, 0x50, 0xaa, 0xac, 0x74, 0x52, 0x27, 0x81,
	0x89, 0x36, 0x58, 0x45, 0x1b, 0x8c, 0x50, 0x88, 0x74, 0x8a, 0x71, 0x29, 0x29, 0x67, 0xc2, 0x5b,
	0xbc, 0xaa, 0xbf, 0x76, 0x2e, 0x48, 0x60, 0x6e, 0x21, 0xd8, 0xc4, 0x3c, 0xde, 0x75, 0x5b, 0x2d,
	0x88, 0x7e, 0xb2, 0xe0, 0xf3, 0xbb, 0x05, 0x96, 0xdb, 0x20, 0x31, 0xe7, 0x59, 0x5d, 0xc5, 0x71,
	0xcd, 0x5b, 0x0f, 0x56, 0xce, 0xf3, 0xb7, 0x1d, 0xe7, 0xcd, 0x41, 0x8d, 0x2d, 0x9b, 0xab, 0xbf,
	0xf3, 0xa5, 0xef, 0xbc, 0x2f, 0x7d, 0x67, 0xb1, 0xf4, 0x9d, 0xcc, 0xd5, 0x66, 0x97, 0x5f, 0x01,
	0x00, 0x00, 0xff, 0xff, 0x21, 0x9c, 0x3d, 0x43, 0x08, 0x03, 0x00, 0x00,
}
