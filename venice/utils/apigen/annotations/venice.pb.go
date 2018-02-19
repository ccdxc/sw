// Code generated by protoc-gen-gogo. DO NOT EDIT.
// source: venice.proto

/*
	Package venice is a generated protocol buffer package.

	It is generated from these files:
		venice.proto

	It has these top-level messages:
		ObjectRln
		RestEndpoint
		ObjectPrefix
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

type ObjectRln struct {
	// Type of reference one of [ NamedRef, SelectorRef, BackRef ]
	Type string `protobuf:"bytes,1,opt,name=Type,proto3" json:"Type,omitempty"`
	// Object being referred to.
	To string `protobuf:"bytes,2,opt,name=To,proto3" json:"To,omitempty"`
}

func (m *ObjectRln) Reset()                    { *m = ObjectRln{} }
func (m *ObjectRln) String() string            { return proto.CompactTextString(m) }
func (*ObjectRln) ProtoMessage()               {}
func (*ObjectRln) Descriptor() ([]byte, []int) { return fileDescriptorVenice, []int{0} }

func (m *ObjectRln) GetType() string {
	if m != nil {
		return m.Type
	}
	return ""
}

func (m *ObjectRln) GetTo() string {
	if m != nil {
		return m.To
	}
	return ""
}

// RestEndpoint is used by the venice.apiRestservice option to specify REST resources to be exposed.
type RestEndpoint struct {
	// Object for which the endpoint is being defined
	Object string `protobuf:"bytes,1,opt,name=Object,proto3" json:"Object,omitempty"`
	// Methods allowed on the resource
	Method []string `protobuf:"bytes,2,rep,name=Method" json:"Method,omitempty"`
	// Pattern for the URI
	Pattern string `protobuf:"bytes,3,opt,name=Pattern,proto3" json:"Pattern,omitempty"`
}

func (m *RestEndpoint) Reset()                    { *m = RestEndpoint{} }
func (m *RestEndpoint) String() string            { return proto.CompactTextString(m) }
func (*RestEndpoint) ProtoMessage()               {}
func (*RestEndpoint) Descriptor() ([]byte, []int) { return fileDescriptorVenice, []int{1} }

func (m *RestEndpoint) GetObject() string {
	if m != nil {
		return m.Object
	}
	return ""
}

func (m *RestEndpoint) GetMethod() []string {
	if m != nil {
		return m.Method
	}
	return nil
}

func (m *RestEndpoint) GetPattern() string {
	if m != nil {
		return m.Pattern
	}
	return ""
}

// ObjectPrefix defines the way the object is identified in the kvstore and REST api.
type ObjectPrefix struct {
	// Path is the path component of the prefix - like tenant etc.
	Path string `protobuf:"bytes,1,opt,name=Path,proto3" json:"Path,omitempty"`
	// Target is the prefix for the object. Which is usually the containers name or singleton name
	//
	// Types that are valid to be assigned to Target:
	//	*ObjectPrefix_Collection
	//	*ObjectPrefix_Singleton
	Target isObjectPrefix_Target `protobuf_oneof:"Target"`
}

func (m *ObjectPrefix) Reset()                    { *m = ObjectPrefix{} }
func (m *ObjectPrefix) String() string            { return proto.CompactTextString(m) }
func (*ObjectPrefix) ProtoMessage()               {}
func (*ObjectPrefix) Descriptor() ([]byte, []int) { return fileDescriptorVenice, []int{2} }

type isObjectPrefix_Target interface {
	isObjectPrefix_Target()
	MarshalTo([]byte) (int, error)
	Size() int
}

type ObjectPrefix_Collection struct {
	Collection string `protobuf:"bytes,2,opt,name=Collection,proto3,oneof"`
}
type ObjectPrefix_Singleton struct {
	Singleton string `protobuf:"bytes,3,opt,name=Singleton,proto3,oneof"`
}

func (*ObjectPrefix_Collection) isObjectPrefix_Target() {}
func (*ObjectPrefix_Singleton) isObjectPrefix_Target()  {}

func (m *ObjectPrefix) GetTarget() isObjectPrefix_Target {
	if m != nil {
		return m.Target
	}
	return nil
}

func (m *ObjectPrefix) GetPath() string {
	if m != nil {
		return m.Path
	}
	return ""
}

func (m *ObjectPrefix) GetCollection() string {
	if x, ok := m.GetTarget().(*ObjectPrefix_Collection); ok {
		return x.Collection
	}
	return ""
}

func (m *ObjectPrefix) GetSingleton() string {
	if x, ok := m.GetTarget().(*ObjectPrefix_Singleton); ok {
		return x.Singleton
	}
	return ""
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*ObjectPrefix) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _ObjectPrefix_OneofMarshaler, _ObjectPrefix_OneofUnmarshaler, _ObjectPrefix_OneofSizer, []interface{}{
		(*ObjectPrefix_Collection)(nil),
		(*ObjectPrefix_Singleton)(nil),
	}
}

func _ObjectPrefix_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*ObjectPrefix)
	// Target
	switch x := m.Target.(type) {
	case *ObjectPrefix_Collection:
		_ = b.EncodeVarint(2<<3 | proto.WireBytes)
		_ = b.EncodeStringBytes(x.Collection)
	case *ObjectPrefix_Singleton:
		_ = b.EncodeVarint(3<<3 | proto.WireBytes)
		_ = b.EncodeStringBytes(x.Singleton)
	case nil:
	default:
		return fmt.Errorf("ObjectPrefix.Target has unexpected type %T", x)
	}
	return nil
}

func _ObjectPrefix_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*ObjectPrefix)
	switch tag {
	case 2: // Target.Collection
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeStringBytes()
		m.Target = &ObjectPrefix_Collection{x}
		return true, err
	case 3: // Target.Singleton
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeStringBytes()
		m.Target = &ObjectPrefix_Singleton{x}
		return true, err
	default:
		return false, nil
	}
}

func _ObjectPrefix_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*ObjectPrefix)
	// Target
	switch x := m.Target.(type) {
	case *ObjectPrefix_Collection:
		n += proto.SizeVarint(2<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(len(x.Collection)))
		n += len(x.Collection)
	case *ObjectPrefix_Singleton:
		n += proto.SizeVarint(3<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(len(x.Singleton)))
		n += len(x.Singleton)
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

var E_FileGrpcDest = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.FileOptions)(nil),
	ExtensionType: (*string)(nil),
	Field:         51235,
	Name:          "venice.fileGrpcDest",
	Tag:           "bytes,51235,opt,name=fileGrpcDest",
	Filename:      "venice.proto",
}

var E_FileApiServerBacked = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.FileOptions)(nil),
	ExtensionType: (*bool)(nil),
	Field:         51236,
	Name:          "venice.fileApiServerBacked",
	Tag:           "varint,51236,opt,name=fileApiServerBacked",
	Filename:      "venice.proto",
}

var E_ApiVersion = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.ServiceOptions)(nil),
	ExtensionType: (*string)(nil),
	Field:         51250,
	Name:          "venice.apiVersion",
	Tag:           "bytes,51250,opt,name=apiVersion",
	Filename:      "venice.proto",
}

var E_ApiPrefix = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.ServiceOptions)(nil),
	ExtensionType: (*string)(nil),
	Field:         51251,
	Name:          "venice.apiPrefix",
	Tag:           "bytes,51251,opt,name=apiPrefix",
	Filename:      "venice.proto",
}

var E_ApiGrpcCrudService = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.ServiceOptions)(nil),
	ExtensionType: ([]string)(nil),
	Field:         51252,
	Name:          "venice.apiGrpcCrudService",
	Tag:           "bytes,51252,rep,name=apiGrpcCrudService",
	Filename:      "venice.proto",
}

var E_ApiRestService = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.ServiceOptions)(nil),
	ExtensionType: ([]*RestEndpoint)(nil),
	Field:         51253,
	Name:          "venice.apiRestService",
	Tag:           "bytes,51253,rep,name=apiRestService",
	Filename:      "venice.proto",
}

var E_NaplesRestService = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.ServiceOptions)(nil),
	ExtensionType: ([]*RestEndpoint)(nil),
	Field:         51254,
	Name:          "venice.naplesRestService",
	Tag:           "bytes,51254,rep,name=naplesRestService",
	Filename:      "venice.proto",
}

var E_MethodOper = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.MethodOptions)(nil),
	ExtensionType: (*string)(nil),
	Field:         51270,
	Name:          "venice.methodOper",
	Tag:           "bytes,51270,opt,name=methodOper",
	Filename:      "venice.proto",
}

var E_MethodAutoGen = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.MethodOptions)(nil),
	ExtensionType: (*bool)(nil),
	Field:         41291,
	Name:          "venice.methodAutoGen",
	Tag:           "varint,41291,opt,name=methodAutoGen",
	Filename:      "venice.proto",
}

var E_ObjectPrefix = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.MessageOptions)(nil),
	ExtensionType: (*ObjectPrefix)(nil),
	Field:         51290,
	Name:          "venice.objectPrefix",
	Tag:           "bytes,51290,opt,name=objectPrefix",
	Filename:      "venice.proto",
}

var E_ObjectAutoGen = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.MessageOptions)(nil),
	ExtensionType: (*string)(nil),
	Field:         41291,
	Name:          "venice.objectAutoGen",
	Tag:           "bytes,41291,opt,name=objectAutoGen",
	Filename:      "venice.proto",
}

var E_ObjRelation = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.FieldOptions)(nil),
	ExtensionType: (*ObjectRln)(nil),
	Field:         51300,
	Name:          "venice.objRelation",
	Tag:           "bytes,51300,opt,name=objRelation",
	Filename:      "venice.proto",
}

var E_Check = &proto.ExtensionDesc{
	ExtendedType:  (*google_protobuf.FieldOptions)(nil),
	ExtensionType: ([]string)(nil),
	Field:         51301,
	Name:          "venice.check",
	Tag:           "bytes,51301,rep,name=check",
	Filename:      "venice.proto",
}

func init() {
	proto.RegisterType((*ObjectRln)(nil), "venice.ObjectRln")
	proto.RegisterType((*RestEndpoint)(nil), "venice.RestEndpoint")
	proto.RegisterType((*ObjectPrefix)(nil), "venice.ObjectPrefix")
	proto.RegisterExtension(E_FileGrpcDest)
	proto.RegisterExtension(E_FileApiServerBacked)
	proto.RegisterExtension(E_ApiVersion)
	proto.RegisterExtension(E_ApiPrefix)
	proto.RegisterExtension(E_ApiGrpcCrudService)
	proto.RegisterExtension(E_ApiRestService)
	proto.RegisterExtension(E_NaplesRestService)
	proto.RegisterExtension(E_MethodOper)
	proto.RegisterExtension(E_MethodAutoGen)
	proto.RegisterExtension(E_ObjectPrefix)
	proto.RegisterExtension(E_ObjectAutoGen)
	proto.RegisterExtension(E_ObjRelation)
	proto.RegisterExtension(E_Check)
}
func (m *ObjectRln) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *ObjectRln) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.Type) > 0 {
		dAtA[i] = 0xa
		i++
		i = encodeVarintVenice(dAtA, i, uint64(len(m.Type)))
		i += copy(dAtA[i:], m.Type)
	}
	if len(m.To) > 0 {
		dAtA[i] = 0x12
		i++
		i = encodeVarintVenice(dAtA, i, uint64(len(m.To)))
		i += copy(dAtA[i:], m.To)
	}
	return i, nil
}

func (m *RestEndpoint) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *RestEndpoint) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.Object) > 0 {
		dAtA[i] = 0xa
		i++
		i = encodeVarintVenice(dAtA, i, uint64(len(m.Object)))
		i += copy(dAtA[i:], m.Object)
	}
	if len(m.Method) > 0 {
		for _, s := range m.Method {
			dAtA[i] = 0x12
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
	if len(m.Pattern) > 0 {
		dAtA[i] = 0x1a
		i++
		i = encodeVarintVenice(dAtA, i, uint64(len(m.Pattern)))
		i += copy(dAtA[i:], m.Pattern)
	}
	return i, nil
}

func (m *ObjectPrefix) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *ObjectPrefix) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.Path) > 0 {
		dAtA[i] = 0xa
		i++
		i = encodeVarintVenice(dAtA, i, uint64(len(m.Path)))
		i += copy(dAtA[i:], m.Path)
	}
	if m.Target != nil {
		nn1, err := m.Target.MarshalTo(dAtA[i:])
		if err != nil {
			return 0, err
		}
		i += nn1
	}
	return i, nil
}

func (m *ObjectPrefix_Collection) MarshalTo(dAtA []byte) (int, error) {
	i := 0
	dAtA[i] = 0x12
	i++
	i = encodeVarintVenice(dAtA, i, uint64(len(m.Collection)))
	i += copy(dAtA[i:], m.Collection)
	return i, nil
}
func (m *ObjectPrefix_Singleton) MarshalTo(dAtA []byte) (int, error) {
	i := 0
	dAtA[i] = 0x1a
	i++
	i = encodeVarintVenice(dAtA, i, uint64(len(m.Singleton)))
	i += copy(dAtA[i:], m.Singleton)
	return i, nil
}
func encodeVarintVenice(dAtA []byte, offset int, v uint64) int {
	for v >= 1<<7 {
		dAtA[offset] = uint8(v&0x7f | 0x80)
		v >>= 7
		offset++
	}
	dAtA[offset] = uint8(v)
	return offset + 1
}
func (m *ObjectRln) Size() (n int) {
	var l int
	_ = l
	l = len(m.Type)
	if l > 0 {
		n += 1 + l + sovVenice(uint64(l))
	}
	l = len(m.To)
	if l > 0 {
		n += 1 + l + sovVenice(uint64(l))
	}
	return n
}

func (m *RestEndpoint) Size() (n int) {
	var l int
	_ = l
	l = len(m.Object)
	if l > 0 {
		n += 1 + l + sovVenice(uint64(l))
	}
	if len(m.Method) > 0 {
		for _, s := range m.Method {
			l = len(s)
			n += 1 + l + sovVenice(uint64(l))
		}
	}
	l = len(m.Pattern)
	if l > 0 {
		n += 1 + l + sovVenice(uint64(l))
	}
	return n
}

func (m *ObjectPrefix) Size() (n int) {
	var l int
	_ = l
	l = len(m.Path)
	if l > 0 {
		n += 1 + l + sovVenice(uint64(l))
	}
	if m.Target != nil {
		n += m.Target.Size()
	}
	return n
}

func (m *ObjectPrefix_Collection) Size() (n int) {
	var l int
	_ = l
	l = len(m.Collection)
	n += 1 + l + sovVenice(uint64(l))
	return n
}
func (m *ObjectPrefix_Singleton) Size() (n int) {
	var l int
	_ = l
	l = len(m.Singleton)
	n += 1 + l + sovVenice(uint64(l))
	return n
}

func sovVenice(x uint64) (n int) {
	for {
		n++
		x >>= 7
		if x == 0 {
			break
		}
	}
	return n
}
func sozVenice(x uint64) (n int) {
	return sovVenice(uint64((x << 1) ^ uint64((int64(x) >> 63))))
}
func (m *ObjectRln) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowVenice
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
			return fmt.Errorf("proto: ObjectRln: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: ObjectRln: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Type", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowVenice
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
				return ErrInvalidLengthVenice
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Type = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 2:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field To", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowVenice
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
				return ErrInvalidLengthVenice
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.To = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipVenice(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthVenice
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
func (m *RestEndpoint) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowVenice
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
			return fmt.Errorf("proto: RestEndpoint: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: RestEndpoint: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Object", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowVenice
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
				return ErrInvalidLengthVenice
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Object = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 2:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Method", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowVenice
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
				return ErrInvalidLengthVenice
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Method = append(m.Method, string(dAtA[iNdEx:postIndex]))
			iNdEx = postIndex
		case 3:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Pattern", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowVenice
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
				return ErrInvalidLengthVenice
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Pattern = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipVenice(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthVenice
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
func (m *ObjectPrefix) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowVenice
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
			return fmt.Errorf("proto: ObjectPrefix: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: ObjectPrefix: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Path", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowVenice
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
				return ErrInvalidLengthVenice
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Path = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 2:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Collection", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowVenice
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
				return ErrInvalidLengthVenice
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Target = &ObjectPrefix_Collection{string(dAtA[iNdEx:postIndex])}
			iNdEx = postIndex
		case 3:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Singleton", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowVenice
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
				return ErrInvalidLengthVenice
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Target = &ObjectPrefix_Singleton{string(dAtA[iNdEx:postIndex])}
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipVenice(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthVenice
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
func skipVenice(dAtA []byte) (n int, err error) {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return 0, ErrIntOverflowVenice
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
					return 0, ErrIntOverflowVenice
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
					return 0, ErrIntOverflowVenice
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
				return 0, ErrInvalidLengthVenice
			}
			return iNdEx, nil
		case 3:
			for {
				var innerWire uint64
				var start int = iNdEx
				for shift := uint(0); ; shift += 7 {
					if shift >= 64 {
						return 0, ErrIntOverflowVenice
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
				next, err := skipVenice(dAtA[start:])
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
	ErrInvalidLengthVenice = fmt.Errorf("proto: negative length found during unmarshaling")
	ErrIntOverflowVenice   = fmt.Errorf("proto: integer overflow")
)

func init() { proto.RegisterFile("venice.proto", fileDescriptorVenice) }

var fileDescriptorVenice = []byte{
	// 566 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x94, 0x93, 0xbd, 0x6e, 0x13, 0x41,
	0x10, 0xc7, 0xfd, 0x01, 0x26, 0x9e, 0x38, 0x11, 0x39, 0x10, 0x3a, 0x21, 0xb8, 0x58, 0xae, 0xa8,
	0x6c, 0x09, 0x44, 0xb3, 0x0d, 0xd8, 0x81, 0x38, 0x4d, 0x64, 0xb3, 0xb1, 0x10, 0x12, 0x05, 0x3a,
	0x9f, 0xc7, 0xf6, 0x26, 0xc7, 0xed, 0x6a, 0x6f, 0x1d, 0xc1, 0x5b, 0xa4, 0xa4, 0x87, 0x27, 0xe0,
	0xeb, 0x01, 0x28, 0x10, 0x12, 0x0d, 0x35, 0x15, 0x32, 0xf0, 0x1e, 0xe8, 0x76, 0xd7, 0xf6, 0x19,
	0x5b, 0x1c, 0xe9, 0x6e, 0xe6, 0xe6, 0xff, 0xdb, 0xff, 0xec, 0xce, 0x40, 0xe5, 0x14, 0x23, 0x16,
	0x60, 0x5d, 0x48, 0xae, 0xb8, 0x53, 0x32, 0xd1, 0xf5, 0xea, 0x88, 0xf3, 0x51, 0x88, 0x0d, 0x9d,
	0xed, 0x4f, 0x86, 0x8d, 0x01, 0xc6, 0x81, 0x64, 0x42, 0x71, 0x69, 0x2a, 0x6b, 0x0d, 0x28, 0x77,
	0xfa, 0xc7, 0x18, 0x28, 0x1a, 0x46, 0x8e, 0x03, 0x17, 0x7a, 0x2f, 0x05, 0xba, 0xf9, 0x6a, 0xfe,
	0x56, 0x99, 0xea, 0x6f, 0x67, 0x1b, 0x0a, 0x3d, 0xee, 0x16, 0x74, 0xa6, 0xd0, 0xe3, 0xb5, 0x27,
	0x50, 0xa1, 0x18, 0xab, 0x87, 0xd1, 0x40, 0x70, 0x16, 0x29, 0xe7, 0x1a, 0x94, 0x0c, 0xc0, 0xaa,
	0x6c, 0x94, 0xe4, 0x0f, 0x51, 0x8d, 0xf9, 0xc0, 0x2d, 0x54, 0x8b, 0x49, 0xde, 0x44, 0x8e, 0x0b,
	0x97, 0xba, 0xbe, 0x52, 0x28, 0x23, 0xb7, 0xa8, 0x05, 0xb3, 0xb0, 0x16, 0x41, 0xc5, 0x68, 0xbb,
	0x12, 0x87, 0xec, 0x45, 0xe2, 0xa6, 0xeb, 0xab, 0xf1, 0xcc, 0x4d, 0xf2, 0xed, 0x54, 0x01, 0xf6,
	0x78, 0x18, 0x62, 0xa0, 0x18, 0x8f, 0x8c, 0xab, 0x83, 0x1c, 0x4d, 0xe5, 0x1c, 0x0f, 0xca, 0x47,
	0x2c, 0x1a, 0x85, 0xa8, 0xb8, 0x3d, 0xe1, 0x20, 0x47, 0x17, 0xa9, 0xd6, 0x06, 0x94, 0x7a, 0xbe,
	0x1c, 0xa1, 0x22, 0x2d, 0xa8, 0x0c, 0x59, 0x88, 0x6d, 0x29, 0x82, 0x07, 0x18, 0x2b, 0xe7, 0x46,
	0xdd, 0xdc, 0x56, 0x7d, 0x76, 0x5b, 0xf5, 0x7d, 0x16, 0x62, 0x47, 0x24, 0xd8, 0xd8, 0x7d, 0x7d,
	0x66, 0xec, 0x2e, 0x69, 0x48, 0x17, 0xae, 0x24, 0x71, 0x53, 0xb0, 0x23, 0x94, 0xa7, 0x28, 0x5b,
	0x7e, 0x70, 0x82, 0x83, 0x0c, 0xd4, 0x1b, 0x8d, 0xda, 0xa0, 0xeb, 0xa4, 0xa4, 0x09, 0xe0, 0x0b,
	0xf6, 0x18, 0x65, 0x9c, 0x74, 0xb3, 0xbb, 0x02, 0x4a, 0x8a, 0x59, 0x30, 0x67, 0xbd, 0xb5, 0xb6,
	0x52, 0x22, 0x72, 0x0f, 0xca, 0xbe, 0x60, 0xf6, 0x16, 0x33, 0x09, 0xef, 0x2c, 0x61, 0xa1, 0x21,
	0x8f, 0xc0, 0xf1, 0x05, 0x4b, 0x9a, 0xdc, 0x93, 0x93, 0x81, 0xad, 0xcf, 0x26, 0xbd, 0x3f, 0x2b,
	0xea, 0xa7, 0x5e, 0x23, 0x26, 0xcf, 0x60, 0xdb, 0x17, 0x2c, 0x99, 0x9c, 0xff, 0xc6, 0x7d, 0xd0,
	0xb8, 0xcd, 0xdb, 0x57, 0xeb, 0x76, 0xb6, 0xd3, 0x73, 0x47, 0xff, 0xc2, 0x11, 0x84, 0x9d, 0xc8,
	0x17, 0x21, 0xc6, 0xe7, 0x3a, 0xe3, 0xe3, 0x3f, 0xcf, 0x58, 0x25, 0x92, 0xfb, 0x00, 0xcf, 0xf5,
	0x20, 0x77, 0x04, 0x4a, 0xc7, 0x5b, 0xe1, 0x1f, 0xda, 0x9f, 0x06, 0xff, 0x79, 0xf6, 0x3a, 0x0b,
	0x0d, 0xd9, 0x87, 0x2d, 0x13, 0x35, 0x27, 0x8a, 0xb7, 0x31, 0xca, 0x84, 0x7c, 0xfd, 0x54, 0xd0,
	0xe3, 0xb2, 0x2c, 0x23, 0x4f, 0xa1, 0xc2, 0xd3, 0xeb, 0xb2, 0xbb, 0x06, 0x13, 0xc7, 0xfe, 0x68,
	0xde, 0xeb, 0x77, 0x6d, 0x26, 0xd5, 0x6b, 0x7a, 0xdb, 0xe8, 0x12, 0x8c, 0xb4, 0x61, 0xcb, 0xc4,
	0x33, 0x93, 0x99, 0x74, 0xe3, 0xb2, 0x4c, 0x97, 0x75, 0xa4, 0x07, 0x9b, 0xbc, 0x7f, 0x4c, 0x31,
	0xf4, 0xf5, 0x76, 0xde, 0x5c, 0xb3, 0x18, 0x18, 0xce, 0x5b, 0xfd, 0x65, 0x2d, 0xee, 0x2c, 0x5b,
	0xa4, 0x61, 0x44, 0xd3, 0x18, 0x72, 0x17, 0x2e, 0x06, 0x63, 0x0c, 0x4e, 0xb2, 0x78, 0xbf, 0xed,
	0x44, 0x9a, 0xea, 0xd6, 0xe5, 0x2f, 0x53, 0x2f, 0xff, 0x6d, 0xea, 0xe5, 0x7f, 0x4c, 0xbd, 0xfc,
	0xab, 0x9f, 0x5e, 0xae, 0x5f, 0xd2, 0xba, 0x3b, 0x7f, 0x02, 0x00, 0x00, 0xff, 0xff, 0x4e, 0xca,
	0xf4, 0x53, 0x3f, 0x05, 0x00, 0x00,
}
