// Code generated by protoc-gen-go.
// source: rsrc.proto
// DO NOT EDIT!

/*
Package rproto is a generated protocol buffer package.

It is generated from these files:
	rsrc.proto

It has these top-level messages:
	ResourceAttributes
	ScalarResource
	RangeResource
	SetResource
	Resource
	ResourceConstraints
	ResourceRequest
	ResourceProvide
	ResourceConsumer
	ResourceProvider
*/
package rproto

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

// resource kinds
type ResourceKind int32

const (
	ResourceKind_UnknownKind ResourceKind = 0
	ResourceKind_Scalar      ResourceKind = 1
	ResourceKind_Range       ResourceKind = 2
	ResourceKind_Set         ResourceKind = 3
)

var ResourceKind_name = map[int32]string{
	0: "UnknownKind",
	1: "Scalar",
	2: "Range",
	3: "Set",
}
var ResourceKind_value = map[string]int32{
	"UnknownKind": 0,
	"Scalar":      1,
	"Range":       2,
	"Set":         3,
}

func (x ResourceKind) String() string {
	return proto.EnumName(ResourceKind_name, int32(x))
}
func (ResourceKind) EnumDescriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

// request types
type AllocType int32

const (
	AllocType_Any      AllocType = 0
	AllocType_Specific AllocType = 1
)

var AllocType_name = map[int32]string{
	0: "Any",
	1: "Specific",
}
var AllocType_value = map[string]int32{
	"Any":      0,
	"Specific": 1,
}

func (x AllocType) String() string {
	return proto.EnumName(AllocType_name, int32(x))
}
func (AllocType) EnumDescriptor() ([]byte, []int) { return fileDescriptor0, []int{1} }

// Resource attributes
type ResourceAttributes struct {
	Attributes map[string]string `protobuf:"bytes,1,rep,name=Attributes" json:"Attributes,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
}

func (m *ResourceAttributes) Reset()                    { *m = ResourceAttributes{} }
func (m *ResourceAttributes) String() string            { return proto.CompactTextString(m) }
func (*ResourceAttributes) ProtoMessage()               {}
func (*ResourceAttributes) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

func (m *ResourceAttributes) GetAttributes() map[string]string {
	if m != nil {
		return m.Attributes
	}
	return nil
}

// Scalar resource
type ScalarResource struct {
	TotalResource     uint64 `protobuf:"varint,1,opt,name=TotalResource" json:"TotalResource,omitempty"`
	AvailableResource uint64 `protobuf:"varint,2,opt,name=AvailableResource" json:"AvailableResource,omitempty"`
}

func (m *ScalarResource) Reset()                    { *m = ScalarResource{} }
func (m *ScalarResource) String() string            { return proto.CompactTextString(m) }
func (*ScalarResource) ProtoMessage()               {}
func (*ScalarResource) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{1} }

func (m *ScalarResource) GetTotalResource() uint64 {
	if m != nil {
		return m.TotalResource
	}
	return 0
}

func (m *ScalarResource) GetAvailableResource() uint64 {
	if m != nil {
		return m.AvailableResource
	}
	return 0
}

// Range resource
type RangeResource struct {
	Begin        uint64 `protobuf:"varint,1,opt,name=Begin" json:"Begin,omitempty"`
	End          uint64 `protobuf:"varint,2,opt,name=End" json:"End,omitempty"`
	NumAvailable uint64 `protobuf:"varint,3,opt,name=NumAvailable" json:"NumAvailable,omitempty"`
	Allocated    []byte `protobuf:"bytes,4,opt,name=Allocated,proto3" json:"Allocated,omitempty"`
}

func (m *RangeResource) Reset()                    { *m = RangeResource{} }
func (m *RangeResource) String() string            { return proto.CompactTextString(m) }
func (*RangeResource) ProtoMessage()               {}
func (*RangeResource) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{2} }

func (m *RangeResource) GetBegin() uint64 {
	if m != nil {
		return m.Begin
	}
	return 0
}

func (m *RangeResource) GetEnd() uint64 {
	if m != nil {
		return m.End
	}
	return 0
}

func (m *RangeResource) GetNumAvailable() uint64 {
	if m != nil {
		return m.NumAvailable
	}
	return 0
}

func (m *RangeResource) GetAllocated() []byte {
	if m != nil {
		return m.Allocated
	}
	return nil
}

// Set resource
type SetResource struct {
	NumTotal       uint64            `protobuf:"varint,1,opt,name=NumTotal" json:"NumTotal,omitempty"`
	NumAvailable   uint64            `protobuf:"varint,2,opt,name=NumAvailable" json:"NumAvailable,omitempty"`
	Items          []uint64          `protobuf:"varint,3,rep,packed,name=Items" json:"Items,omitempty"`
	AllocatedItems map[uint64]uint64 `protobuf:"bytes,4,rep,name=AllocatedItems" json:"AllocatedItems,omitempty" protobuf_key:"varint,1,opt,name=key" protobuf_val:"varint,2,opt,name=value"`
}

func (m *SetResource) Reset()                    { *m = SetResource{} }
func (m *SetResource) String() string            { return proto.CompactTextString(m) }
func (*SetResource) ProtoMessage()               {}
func (*SetResource) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{3} }

func (m *SetResource) GetNumTotal() uint64 {
	if m != nil {
		return m.NumTotal
	}
	return 0
}

func (m *SetResource) GetNumAvailable() uint64 {
	if m != nil {
		return m.NumAvailable
	}
	return 0
}

func (m *SetResource) GetItems() []uint64 {
	if m != nil {
		return m.Items
	}
	return nil
}

func (m *SetResource) GetAllocatedItems() map[uint64]uint64 {
	if m != nil {
		return m.AllocatedItems
	}
	return nil
}

// Resource definition
type Resource struct {
	ResourceType string              `protobuf:"bytes,1,opt,name=ResourceType" json:"ResourceType,omitempty"`
	ResourceKind ResourceKind        `protobuf:"varint,2,opt,name=ResourceKind,enum=rproto.ResourceKind" json:"ResourceKind,omitempty"`
	Scalar       *ScalarResource     `protobuf:"bytes,3,opt,name=Scalar" json:"Scalar,omitempty"`
	Range        *RangeResource      `protobuf:"bytes,4,opt,name=Range" json:"Range,omitempty"`
	Set          *SetResource        `protobuf:"bytes,5,opt,name=Set" json:"Set,omitempty"`
	Attributes   *ResourceAttributes `protobuf:"bytes,6,opt,name=Attributes" json:"Attributes,omitempty"`
}

func (m *Resource) Reset()                    { *m = Resource{} }
func (m *Resource) String() string            { return proto.CompactTextString(m) }
func (*Resource) ProtoMessage()               {}
func (*Resource) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{4} }

func (m *Resource) GetResourceType() string {
	if m != nil {
		return m.ResourceType
	}
	return ""
}

func (m *Resource) GetResourceKind() ResourceKind {
	if m != nil {
		return m.ResourceKind
	}
	return ResourceKind_UnknownKind
}

func (m *Resource) GetScalar() *ScalarResource {
	if m != nil {
		return m.Scalar
	}
	return nil
}

func (m *Resource) GetRange() *RangeResource {
	if m != nil {
		return m.Range
	}
	return nil
}

func (m *Resource) GetSet() *SetResource {
	if m != nil {
		return m.Set
	}
	return nil
}

func (m *Resource) GetAttributes() *ResourceAttributes {
	if m != nil {
		return m.Attributes
	}
	return nil
}

// resource request Constraints
type ResourceConstraints struct {
	ProviderID string              `protobuf:"bytes,1,opt,name=ProviderID" json:"ProviderID,omitempty"`
	Attributes *ResourceAttributes `protobuf:"bytes,2,opt,name=Attributes" json:"Attributes,omitempty"`
}

func (m *ResourceConstraints) Reset()                    { *m = ResourceConstraints{} }
func (m *ResourceConstraints) String() string            { return proto.CompactTextString(m) }
func (*ResourceConstraints) ProtoMessage()               {}
func (*ResourceConstraints) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{5} }

func (m *ResourceConstraints) GetProviderID() string {
	if m != nil {
		return m.ProviderID
	}
	return ""
}

func (m *ResourceConstraints) GetAttributes() *ResourceAttributes {
	if m != nil {
		return m.Attributes
	}
	return nil
}

// resource request
type ResourceRequest struct {
	ResourceType string               `protobuf:"bytes,1,opt,name=ResourceType" json:"ResourceType,omitempty"`
	AllocType    AllocType            `protobuf:"varint,2,opt,name=AllocType,enum=rproto.AllocType" json:"AllocType,omitempty"`
	Scheduler    string               `protobuf:"bytes,3,opt,name=Scheduler" json:"Scheduler,omitempty"`
	Quantity     uint64               `protobuf:"varint,4,opt,name=Quantity" json:"Quantity,omitempty"`
	ConsumerID   string               `protobuf:"bytes,5,opt,name=ConsumerID" json:"ConsumerID,omitempty"`
	Values       []uint64             `protobuf:"varint,6,rep,packed,name=Values" json:"Values,omitempty"`
	Constraints  *ResourceConstraints `protobuf:"bytes,7,opt,name=Constraints" json:"Constraints,omitempty"`
}

func (m *ResourceRequest) Reset()                    { *m = ResourceRequest{} }
func (m *ResourceRequest) String() string            { return proto.CompactTextString(m) }
func (*ResourceRequest) ProtoMessage()               {}
func (*ResourceRequest) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{6} }

func (m *ResourceRequest) GetResourceType() string {
	if m != nil {
		return m.ResourceType
	}
	return ""
}

func (m *ResourceRequest) GetAllocType() AllocType {
	if m != nil {
		return m.AllocType
	}
	return AllocType_Any
}

func (m *ResourceRequest) GetScheduler() string {
	if m != nil {
		return m.Scheduler
	}
	return ""
}

func (m *ResourceRequest) GetQuantity() uint64 {
	if m != nil {
		return m.Quantity
	}
	return 0
}

func (m *ResourceRequest) GetConsumerID() string {
	if m != nil {
		return m.ConsumerID
	}
	return ""
}

func (m *ResourceRequest) GetValues() []uint64 {
	if m != nil {
		return m.Values
	}
	return nil
}

func (m *ResourceRequest) GetConstraints() *ResourceConstraints {
	if m != nil {
		return m.Constraints
	}
	return nil
}

// ResourceProvide is the resource provider
type ResourceProvide struct {
	Resource   *Resource `protobuf:"bytes,1,opt,name=Resource" json:"Resource,omitempty"`
	ProviderID string    `protobuf:"bytes,2,opt,name=ProviderID" json:"ProviderID,omitempty"`
}

func (m *ResourceProvide) Reset()                    { *m = ResourceProvide{} }
func (m *ResourceProvide) String() string            { return proto.CompactTextString(m) }
func (*ResourceProvide) ProtoMessage()               {}
func (*ResourceProvide) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{7} }

func (m *ResourceProvide) GetResource() *Resource {
	if m != nil {
		return m.Resource
	}
	return nil
}

func (m *ResourceProvide) GetProviderID() string {
	if m != nil {
		return m.ProviderID
	}
	return ""
}

// ResourceConsumer is a resource consumer instance
type ResourceConsumer struct {
	ConsumerID   string   `protobuf:"bytes,1,opt,name=ConsumerID" json:"ConsumerID,omitempty"`
	ProviderID   string   `protobuf:"bytes,2,opt,name=ProviderID" json:"ProviderID,omitempty"`
	ResourceType string   `protobuf:"bytes,3,opt,name=ResourceType" json:"ResourceType,omitempty"`
	Values       []uint64 `protobuf:"varint,4,rep,packed,name=Values" json:"Values,omitempty"`
}

func (m *ResourceConsumer) Reset()                    { *m = ResourceConsumer{} }
func (m *ResourceConsumer) String() string            { return proto.CompactTextString(m) }
func (*ResourceConsumer) ProtoMessage()               {}
func (*ResourceConsumer) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{8} }

func (m *ResourceConsumer) GetConsumerID() string {
	if m != nil {
		return m.ConsumerID
	}
	return ""
}

func (m *ResourceConsumer) GetProviderID() string {
	if m != nil {
		return m.ProviderID
	}
	return ""
}

func (m *ResourceConsumer) GetResourceType() string {
	if m != nil {
		return m.ResourceType
	}
	return ""
}

func (m *ResourceConsumer) GetValues() []uint64 {
	if m != nil {
		return m.Values
	}
	return nil
}

// ResourceProvider provides a resource
type ResourceProvider struct {
	Resource   *Resource                    `protobuf:"bytes,1,opt,name=Resource" json:"Resource,omitempty"`
	ProviderID string                       `protobuf:"bytes,2,opt,name=ProviderID" json:"ProviderID,omitempty"`
	Consumers  map[string]*ResourceConsumer `protobuf:"bytes,3,rep,name=Consumers" json:"Consumers,omitempty" protobuf_key:"bytes,1,opt,name=key" protobuf_val:"bytes,2,opt,name=value"`
}

func (m *ResourceProvider) Reset()                    { *m = ResourceProvider{} }
func (m *ResourceProvider) String() string            { return proto.CompactTextString(m) }
func (*ResourceProvider) ProtoMessage()               {}
func (*ResourceProvider) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{9} }

func (m *ResourceProvider) GetResource() *Resource {
	if m != nil {
		return m.Resource
	}
	return nil
}

func (m *ResourceProvider) GetProviderID() string {
	if m != nil {
		return m.ProviderID
	}
	return ""
}

func (m *ResourceProvider) GetConsumers() map[string]*ResourceConsumer {
	if m != nil {
		return m.Consumers
	}
	return nil
}

func init() {
	proto.RegisterType((*ResourceAttributes)(nil), "rproto.ResourceAttributes")
	proto.RegisterType((*ScalarResource)(nil), "rproto.ScalarResource")
	proto.RegisterType((*RangeResource)(nil), "rproto.RangeResource")
	proto.RegisterType((*SetResource)(nil), "rproto.SetResource")
	proto.RegisterType((*Resource)(nil), "rproto.Resource")
	proto.RegisterType((*ResourceConstraints)(nil), "rproto.ResourceConstraints")
	proto.RegisterType((*ResourceRequest)(nil), "rproto.ResourceRequest")
	proto.RegisterType((*ResourceProvide)(nil), "rproto.ResourceProvide")
	proto.RegisterType((*ResourceConsumer)(nil), "rproto.ResourceConsumer")
	proto.RegisterType((*ResourceProvider)(nil), "rproto.ResourceProvider")
	proto.RegisterEnum("rproto.ResourceKind", ResourceKind_name, ResourceKind_value)
	proto.RegisterEnum("rproto.AllocType", AllocType_name, AllocType_value)
}

func init() { proto.RegisterFile("rsrc.proto", fileDescriptor0) }

var fileDescriptor0 = []byte{
	// 715 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x09, 0x6e, 0x88, 0x02, 0xff, 0xac, 0x55, 0x4b, 0x6f, 0xd3, 0x40,
	0x10, 0xae, 0x1f, 0x49, 0x9b, 0x71, 0x9a, 0xba, 0xd3, 0x52, 0x59, 0x01, 0xa1, 0xc8, 0x02, 0x51,
	0x95, 0xca, 0x48, 0xe1, 0x52, 0x55, 0xaa, 0x50, 0x80, 0x1e, 0x0a, 0x52, 0x81, 0x4d, 0xe9, 0x15,
	0xb9, 0xce, 0x52, 0xac, 0x3a, 0x76, 0x6b, 0xaf, 0x8b, 0xf2, 0x27, 0xb8, 0x72, 0xe4, 0xca, 0x3f,
	0xe4, 0x08, 0xda, 0xf5, 0x6b, 0xed, 0x18, 0x01, 0x12, 0xa7, 0x78, 0x66, 0xbe, 0x9d, 0xd7, 0x37,
	0x33, 0x01, 0x88, 0x93, 0xd8, 0x73, 0xae, 0xe3, 0x88, 0x45, 0xd8, 0x8d, 0xc5, 0xaf, 0xfd, 0x4d,
	0x01, 0x24, 0x34, 0x89, 0xd2, 0xd8, 0xa3, 0x13, 0xc6, 0x62, 0xff, 0x22, 0x65, 0x34, 0xc1, 0x57,
	0x00, 0x95, 0x64, 0x29, 0x23, 0x6d, 0xd7, 0x18, 0xef, 0x39, 0xd9, 0x1b, 0x67, 0x19, 0xef, 0x54,
	0x9f, 0xc7, 0x21, 0x8b, 0x17, 0x44, 0x7a, 0x3d, 0x3c, 0x82, 0x8d, 0x86, 0x19, 0x4d, 0xd0, 0xae,
	0xe8, 0xc2, 0x52, 0x46, 0xca, 0x6e, 0x8f, 0xf0, 0x4f, 0xdc, 0x86, 0xce, 0xad, 0x1b, 0xa4, 0xd4,
	0x52, 0x85, 0x2e, 0x13, 0x0e, 0xd5, 0x03, 0xc5, 0x9e, 0xc1, 0x60, 0xea, 0xb9, 0x81, 0x1b, 0x17,
	0x61, 0xf1, 0x01, 0xac, 0x9f, 0x45, 0xcc, 0x0d, 0x0a, 0x85, 0xf0, 0xa3, 0x93, 0xba, 0x12, 0xf7,
	0x61, 0x73, 0x72, 0xeb, 0xfa, 0x81, 0x7b, 0x11, 0xd0, 0x12, 0xa9, 0x0a, 0xe4, 0xb2, 0xc1, 0x5e,
	0xc0, 0x3a, 0x71, 0xc3, 0xcb, 0x52, 0xc1, 0x13, 0x7a, 0x4e, 0x2f, 0xfd, 0x30, 0x77, 0x9e, 0x09,
	0x3c, 0xf1, 0xe3, 0x70, 0x96, 0xbb, 0xe1, 0x9f, 0x68, 0x43, 0xff, 0x34, 0x9d, 0x97, 0x0e, 0x2d,
	0x4d, 0x98, 0x6a, 0x3a, 0xbc, 0x07, 0xbd, 0x49, 0x10, 0x44, 0x9e, 0xcb, 0xe8, 0xcc, 0xd2, 0x47,
	0xca, 0x6e, 0x9f, 0x54, 0x0a, 0xfb, 0x87, 0x02, 0xc6, 0x94, 0xb2, 0x32, 0xf2, 0x10, 0xd6, 0x4e,
	0xd3, 0xb9, 0x28, 0x26, 0x0f, 0x5e, 0xca, 0x4b, 0xd1, 0xd4, 0x96, 0x68, 0xdb, 0xd0, 0x39, 0x61,
	0x74, 0x9e, 0x58, 0xda, 0x48, 0xe3, 0x99, 0x0b, 0x01, 0xdf, 0xc0, 0xa0, 0x0c, 0x99, 0x99, 0x75,
	0xc1, 0xea, 0xa3, 0x82, 0x55, 0x29, 0x05, 0xa7, 0x8e, 0xcc, 0x28, 0x6d, 0x3c, 0x1f, 0x4e, 0x60,
	0xab, 0x05, 0x26, 0x53, 0xab, 0xb7, 0x50, 0xab, 0xcb, 0xd4, 0x7e, 0x57, 0x61, 0xad, 0x2c, 0xdb,
	0x86, 0x7e, 0xf1, 0x7d, 0xb6, 0xb8, 0xa6, 0xf9, 0x70, 0xd4, 0x74, 0x78, 0x50, 0x61, 0x5e, 0xfb,
	0x39, 0x0f, 0x83, 0xf1, 0x76, 0x73, 0x30, 0xb9, 0x8d, 0xd4, 0x90, 0xe8, 0x40, 0x37, 0x9b, 0x22,
	0x41, 0x90, 0x31, 0xde, 0x29, 0xcb, 0xae, 0xcd, 0x16, 0xc9, 0x51, 0xf8, 0x18, 0x3a, 0x62, 0x1e,
	0x04, 0x5d, 0xc6, 0xf8, 0x4e, 0x19, 0x42, 0x1e, 0x12, 0x92, 0x61, 0xf0, 0x21, 0x68, 0x53, 0xca,
	0xac, 0x8e, 0x80, 0x6e, 0xb5, 0x34, 0x94, 0x70, 0x3b, 0x1e, 0xd6, 0x96, 0xaa, 0x2b, 0xd0, 0xc3,
	0xdf, 0x2f, 0x95, 0xbc, 0x44, 0xf6, 0x0d, 0x6c, 0x15, 0x88, 0x17, 0x51, 0x98, 0xb0, 0xd8, 0xf5,
	0x43, 0x96, 0xe0, 0x7d, 0x80, 0xb7, 0x71, 0x74, 0xeb, 0xcf, 0x68, 0x7c, 0xf2, 0x32, 0x6f, 0x99,
	0xa4, 0x69, 0x84, 0x54, 0xff, 0x29, 0xe4, 0x57, 0x15, 0x36, 0xca, 0x02, 0xe8, 0x4d, 0x4a, 0x13,
	0xf6, 0x57, 0x24, 0x3d, 0xc9, 0xa7, 0x5d, 0x00, 0x32, 0x86, 0x36, 0x8b, 0x90, 0xa5, 0x81, 0x54,
	0x18, 0xbe, 0x1e, 0x53, 0xef, 0x13, 0x9d, 0xa5, 0x01, 0xcd, 0xe8, 0xe9, 0x91, 0x4a, 0xc1, 0xd7,
	0xe1, 0x5d, 0xea, 0x86, 0xcc, 0x67, 0x0b, 0x41, 0x86, 0x4e, 0x4a, 0x99, 0x97, 0xcf, 0xbb, 0x91,
	0xce, 0x45, 0xf9, 0x9d, 0xac, 0xfc, 0x4a, 0x83, 0x3b, 0xd0, 0x3d, 0xe7, 0xd3, 0xc6, 0xbb, 0xcd,
	0x77, 0x21, 0x97, 0xf0, 0x08, 0x0c, 0xa9, 0x8b, 0xd6, 0xaa, 0xe8, 0xcb, 0xdd, 0x66, 0x5f, 0x24,
	0x08, 0x91, 0xf1, 0xf6, 0x87, 0xaa, 0x31, 0x79, 0xaf, 0x71, 0xbf, 0x9a, 0x64, 0xd1, 0x14, 0x63,
	0x6c, 0x36, 0xdd, 0x91, 0x6a, 0xd6, 0xeb, 0xb4, 0xa9, 0x4d, 0xda, 0xec, 0x2f, 0x0a, 0x98, 0x72,
	0x16, 0xbc, 0x9c, 0x46, 0xb1, 0xca, 0x52, 0xb1, 0x7f, 0x70, 0xba, 0xc4, 0x9d, 0xd6, 0xc2, 0x5d,
	0xd5, 0x30, 0x5d, 0x6e, 0x98, 0xfd, 0x53, 0x4a, 0xa8, 0x70, 0xf9, 0x7f, 0x6b, 0xc6, 0x63, 0xe8,
	0x15, 0xc5, 0x64, 0xa7, 0x4b, 0xba, 0x4d, 0xcd, 0xd0, 0x4e, 0x89, 0xcc, 0x6e, 0x53, 0xf5, 0x72,
	0x78, 0x0e, 0x83, 0xba, 0xb1, 0xe5, 0xcf, 0xc6, 0x91, 0x2f, 0x92, 0x31, 0xb6, 0xda, 0x88, 0xe7,
	0x0e, 0xa4, 0x5b, 0xb5, 0xf7, 0xac, 0x7e, 0x7a, 0x70, 0x03, 0x8c, 0xf7, 0xe1, 0x55, 0x18, 0x7d,
	0x0e, 0xb9, 0x68, 0xae, 0x20, 0x14, 0x17, 0xc6, 0x54, 0xb0, 0x97, 0x5f, 0x0f, 0x53, 0xc5, 0x55,
	0x71, 0x1b, 0x4c, 0x6d, 0xcf, 0x96, 0xd6, 0x82, 0x6b, 0x27, 0xe1, 0xc2, 0x5c, 0xc1, 0x3e, 0xac,
	0x4d, 0xaf, 0xa9, 0xe7, 0x7f, 0xf4, 0x3d, 0x53, 0xb9, 0xe8, 0x8a, 0x3c, 0x9e, 0xfe, 0x0a, 0x00,
	0x00, 0xff, 0xff, 0x60, 0xaa, 0x3f, 0x22, 0xaa, 0x07, 0x00, 0x00,
}
