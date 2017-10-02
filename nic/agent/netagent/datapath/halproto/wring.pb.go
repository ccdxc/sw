// Code generated by protoc-gen-go. DO NOT EDIT.
// source: nic/proto/hal/wring.proto

package halproto

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

import (
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// WRingKeyHandle is used to operate either by its key or handle
type WRingKeyHandle struct {
	// Types that are valid to be assigned to KeyOrHandle:
	//	*WRingKeyHandle_WringId
	//	*WRingKeyHandle_WringHandle
	KeyOrHandle isWRingKeyHandle_KeyOrHandle `protobuf_oneof:"key_or_handle"`
}

func (m *WRingKeyHandle) Reset()                    { *m = WRingKeyHandle{} }
func (m *WRingKeyHandle) String() string            { return proto.CompactTextString(m) }
func (*WRingKeyHandle) ProtoMessage()               {}
func (*WRingKeyHandle) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{0} }

type isWRingKeyHandle_KeyOrHandle interface {
	isWRingKeyHandle_KeyOrHandle()
}

type WRingKeyHandle_WringId struct {
	WringId uint32 `protobuf:"varint,1,opt,name=wring_id,json=wringId,oneof"`
}
type WRingKeyHandle_WringHandle struct {
	WringHandle uint64 `protobuf:"fixed64,2,opt,name=wring_handle,json=wringHandle,oneof"`
}

func (*WRingKeyHandle_WringId) isWRingKeyHandle_KeyOrHandle()     {}
func (*WRingKeyHandle_WringHandle) isWRingKeyHandle_KeyOrHandle() {}

func (m *WRingKeyHandle) GetKeyOrHandle() isWRingKeyHandle_KeyOrHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

func (m *WRingKeyHandle) GetWringId() uint32 {
	if x, ok := m.GetKeyOrHandle().(*WRingKeyHandle_WringId); ok {
		return x.WringId
	}
	return 0
}

func (m *WRingKeyHandle) GetWringHandle() uint64 {
	if x, ok := m.GetKeyOrHandle().(*WRingKeyHandle_WringHandle); ok {
		return x.WringHandle
	}
	return 0
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*WRingKeyHandle) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _WRingKeyHandle_OneofMarshaler, _WRingKeyHandle_OneofUnmarshaler, _WRingKeyHandle_OneofSizer, []interface{}{
		(*WRingKeyHandle_WringId)(nil),
		(*WRingKeyHandle_WringHandle)(nil),
	}
}

func _WRingKeyHandle_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*WRingKeyHandle)
	// key_or_handle
	switch x := m.KeyOrHandle.(type) {
	case *WRingKeyHandle_WringId:
		b.EncodeVarint(1<<3 | proto.WireVarint)
		b.EncodeVarint(uint64(x.WringId))
	case *WRingKeyHandle_WringHandle:
		b.EncodeVarint(2<<3 | proto.WireFixed64)
		b.EncodeFixed64(uint64(x.WringHandle))
	case nil:
	default:
		return fmt.Errorf("WRingKeyHandle.KeyOrHandle has unexpected type %T", x)
	}
	return nil
}

func _WRingKeyHandle_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*WRingKeyHandle)
	switch tag {
	case 1: // key_or_handle.wring_id
		if wire != proto.WireVarint {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeVarint()
		m.KeyOrHandle = &WRingKeyHandle_WringId{uint32(x)}
		return true, err
	case 2: // key_or_handle.wring_handle
		if wire != proto.WireFixed64 {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeFixed64()
		m.KeyOrHandle = &WRingKeyHandle_WringHandle{x}
		return true, err
	default:
		return false, nil
	}
}

func _WRingKeyHandle_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*WRingKeyHandle)
	// key_or_handle
	switch x := m.KeyOrHandle.(type) {
	case *WRingKeyHandle_WringId:
		n += proto.SizeVarint(1<<3 | proto.WireVarint)
		n += proto.SizeVarint(uint64(x.WringId))
	case *WRingKeyHandle_WringHandle:
		n += proto.SizeVarint(2<<3 | proto.WireFixed64)
		n += 8
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

// WRingSpec captures all the WRing level configuration
type WRingSpec struct {
	Meta        *ObjectMeta     `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle *WRingKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
	Type        WRingType       `protobuf:"varint,3,opt,name=type,enum=types.WRingType" json:"type,omitempty"`
}

func (m *WRingSpec) Reset()                    { *m = WRingSpec{} }
func (m *WRingSpec) String() string            { return proto.CompactTextString(m) }
func (*WRingSpec) ProtoMessage()               {}
func (*WRingSpec) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{1} }

func (m *WRingSpec) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *WRingSpec) GetKeyOrHandle() *WRingKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

func (m *WRingSpec) GetType() WRingType {
	if m != nil {
		return m.Type
	}
	return WRingType_WRING_TYPE_NONE
}

// WRingRequestMsg is batched add or modify wring request
type WRingRequestMsg struct {
	Request []*WRingSpec `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *WRingRequestMsg) Reset()                    { *m = WRingRequestMsg{} }
func (m *WRingRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*WRingRequestMsg) ProtoMessage()               {}
func (*WRingRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{2} }

func (m *WRingRequestMsg) GetRequest() []*WRingSpec {
	if m != nil {
		return m.Request
	}
	return nil
}

// WRingStatus is the operational status of a given wring
type WRingStatus struct {
	WringHandle uint64 `protobuf:"fixed64,1,opt,name=wring_handle,json=wringHandle" json:"wring_handle,omitempty"`
}

func (m *WRingStatus) Reset()                    { *m = WRingStatus{} }
func (m *WRingStatus) String() string            { return proto.CompactTextString(m) }
func (*WRingStatus) ProtoMessage()               {}
func (*WRingStatus) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{3} }

func (m *WRingStatus) GetWringHandle() uint64 {
	if m != nil {
		return m.WringHandle
	}
	return 0
}

// WRingResponse is response to WRingSpec
type WRingResponse struct {
	ApiStatus   ApiStatus    `protobuf:"varint,1,opt,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
	WringStatus *WRingStatus `protobuf:"bytes,2,opt,name=wring_status,json=wringStatus" json:"wring_status,omitempty"`
}

func (m *WRingResponse) Reset()                    { *m = WRingResponse{} }
func (m *WRingResponse) String() string            { return proto.CompactTextString(m) }
func (*WRingResponse) ProtoMessage()               {}
func (*WRingResponse) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{4} }

func (m *WRingResponse) GetApiStatus() ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return ApiStatus_API_STATUS_OK
}

func (m *WRingResponse) GetWringStatus() *WRingStatus {
	if m != nil {
		return m.WringStatus
	}
	return nil
}

// WRingResponseMsg is batched response to WRingRequestMsg
type WRingResponseMsg struct {
	Response []*WRingResponse `protobuf:"bytes,1,rep,name=response" json:"response,omitempty"`
}

func (m *WRingResponseMsg) Reset()                    { *m = WRingResponseMsg{} }
func (m *WRingResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*WRingResponseMsg) ProtoMessage()               {}
func (*WRingResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{5} }

func (m *WRingResponseMsg) GetResponse() []*WRingResponse {
	if m != nil {
		return m.Response
	}
	return nil
}

// WRingDeleteRequest is used to delete a wring
type WRingDeleteRequest struct {
	Meta        *ObjectMeta     `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle *WRingKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
}

func (m *WRingDeleteRequest) Reset()                    { *m = WRingDeleteRequest{} }
func (m *WRingDeleteRequest) String() string            { return proto.CompactTextString(m) }
func (*WRingDeleteRequest) ProtoMessage()               {}
func (*WRingDeleteRequest) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{6} }

func (m *WRingDeleteRequest) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *WRingDeleteRequest) GetKeyOrHandle() *WRingKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

// WRingDeleteRequestMsg is used to delete a batch of wrings
type WRingDeleteRequestMsg struct {
	Request []*WRingDeleteRequest `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *WRingDeleteRequestMsg) Reset()                    { *m = WRingDeleteRequestMsg{} }
func (m *WRingDeleteRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*WRingDeleteRequestMsg) ProtoMessage()               {}
func (*WRingDeleteRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{7} }

func (m *WRingDeleteRequestMsg) GetRequest() []*WRingDeleteRequest {
	if m != nil {
		return m.Request
	}
	return nil
}

// WRingDeleteResponseMsg is batched response to WRingDeleteRequestMsg
type WRingDeleteResponseMsg struct {
	ApiStatus []ApiStatus `protobuf:"varint,1,rep,packed,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
}

func (m *WRingDeleteResponseMsg) Reset()                    { *m = WRingDeleteResponseMsg{} }
func (m *WRingDeleteResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*WRingDeleteResponseMsg) ProtoMessage()               {}
func (*WRingDeleteResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{8} }

func (m *WRingDeleteResponseMsg) GetApiStatus() []ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return nil
}

// WRingGetRequest is used to get information about a wring
type WRingGetEntriesRequest struct {
	Meta        *ObjectMeta     `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle *WRingKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
	Type        WRingType       `protobuf:"varint,3,opt,name=type,enum=types.WRingType" json:"type,omitempty"`
	Index       uint32          `protobuf:"varint,4,opt,name=index" json:"index,omitempty"`
}

func (m *WRingGetEntriesRequest) Reset()                    { *m = WRingGetEntriesRequest{} }
func (m *WRingGetEntriesRequest) String() string            { return proto.CompactTextString(m) }
func (*WRingGetEntriesRequest) ProtoMessage()               {}
func (*WRingGetEntriesRequest) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{9} }

func (m *WRingGetEntriesRequest) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *WRingGetEntriesRequest) GetKeyOrHandle() *WRingKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

func (m *WRingGetEntriesRequest) GetType() WRingType {
	if m != nil {
		return m.Type
	}
	return WRingType_WRING_TYPE_NONE
}

func (m *WRingGetEntriesRequest) GetIndex() uint32 {
	if m != nil {
		return m.Index
	}
	return 0
}

// WRingGetEntriesRequestMsg is batched GET requests for wrings
type WRingGetEntriesRequestMsg struct {
	Request []*WRingGetEntriesRequest `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *WRingGetEntriesRequestMsg) Reset()                    { *m = WRingGetEntriesRequestMsg{} }
func (m *WRingGetEntriesRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*WRingGetEntriesRequestMsg) ProtoMessage()               {}
func (*WRingGetEntriesRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{10} }

func (m *WRingGetEntriesRequestMsg) GetRequest() []*WRingGetEntriesRequest {
	if m != nil {
		return m.Request
	}
	return nil
}

// WRingStats is the statistics object for each wring
type WRingStats struct {
}

func (m *WRingStats) Reset()                    { *m = WRingStats{} }
func (m *WRingStats) String() string            { return proto.CompactTextString(m) }
func (*WRingStats) ProtoMessage()               {}
func (*WRingStats) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{11} }

type WRingBarcoGCMDescMsg struct {
	IlistAddr    uint64 `protobuf:"fixed64,1,opt,name=ilist_addr,json=ilistAddr" json:"ilist_addr,omitempty"`
	OlistAddr    uint64 `protobuf:"fixed64,2,opt,name=olist_addr,json=olistAddr" json:"olist_addr,omitempty"`
	Command      uint32 `protobuf:"varint,3,opt,name=command" json:"command,omitempty"`
	KeyDescIndex uint32 `protobuf:"varint,4,opt,name=key_desc_index,json=keyDescIndex" json:"key_desc_index,omitempty"`
	IvAddr       uint64 `protobuf:"fixed64,5,opt,name=iv_addr,json=ivAddr" json:"iv_addr,omitempty"`
	StatusAddr   uint64 `protobuf:"fixed64,6,opt,name=status_addr,json=statusAddr" json:"status_addr,omitempty"`
	DoorbellAddr uint64 `protobuf:"fixed64,7,opt,name=doorbell_addr,json=doorbellAddr" json:"doorbell_addr,omitempty"`
	DoorbellData uint64 `protobuf:"fixed64,8,opt,name=doorbell_data,json=doorbellData" json:"doorbell_data,omitempty"`
}

func (m *WRingBarcoGCMDescMsg) Reset()                    { *m = WRingBarcoGCMDescMsg{} }
func (m *WRingBarcoGCMDescMsg) String() string            { return proto.CompactTextString(m) }
func (*WRingBarcoGCMDescMsg) ProtoMessage()               {}
func (*WRingBarcoGCMDescMsg) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{12} }

func (m *WRingBarcoGCMDescMsg) GetIlistAddr() uint64 {
	if m != nil {
		return m.IlistAddr
	}
	return 0
}

func (m *WRingBarcoGCMDescMsg) GetOlistAddr() uint64 {
	if m != nil {
		return m.OlistAddr
	}
	return 0
}

func (m *WRingBarcoGCMDescMsg) GetCommand() uint32 {
	if m != nil {
		return m.Command
	}
	return 0
}

func (m *WRingBarcoGCMDescMsg) GetKeyDescIndex() uint32 {
	if m != nil {
		return m.KeyDescIndex
	}
	return 0
}

func (m *WRingBarcoGCMDescMsg) GetIvAddr() uint64 {
	if m != nil {
		return m.IvAddr
	}
	return 0
}

func (m *WRingBarcoGCMDescMsg) GetStatusAddr() uint64 {
	if m != nil {
		return m.StatusAddr
	}
	return 0
}

func (m *WRingBarcoGCMDescMsg) GetDoorbellAddr() uint64 {
	if m != nil {
		return m.DoorbellAddr
	}
	return 0
}

func (m *WRingBarcoGCMDescMsg) GetDoorbellData() uint64 {
	if m != nil {
		return m.DoorbellData
	}
	return 0
}

// WRingGetResponse captures all the information about a wring
// only if api_status indicates success, other fields are valid
type WRingGetEntriesResponse struct {
	ApiStatus ApiStatus    `protobuf:"varint,1,opt,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
	Spec      *WRingSpec   `protobuf:"bytes,2,opt,name=spec" json:"spec,omitempty"`
	Index     uint32       `protobuf:"varint,3,opt,name=index" json:"index,omitempty"`
	Status    *WRingStatus `protobuf:"bytes,4,opt,name=status" json:"status,omitempty"`
	Stats     *WRingStats  `protobuf:"bytes,5,opt,name=stats" json:"stats,omitempty"`
	// Types that are valid to be assigned to WRingSlotInfo:
	//	*WRingGetEntriesResponse_Value
	//	*WRingGetEntriesResponse_BarcoGcmDesc
	//	*WRingGetEntriesResponse_Raw
	WRingSlotInfo isWRingGetEntriesResponse_WRingSlotInfo `protobuf_oneof:"WRingSlotInfo"`
}

func (m *WRingGetEntriesResponse) Reset()                    { *m = WRingGetEntriesResponse{} }
func (m *WRingGetEntriesResponse) String() string            { return proto.CompactTextString(m) }
func (*WRingGetEntriesResponse) ProtoMessage()               {}
func (*WRingGetEntriesResponse) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{13} }

type isWRingGetEntriesResponse_WRingSlotInfo interface {
	isWRingGetEntriesResponse_WRingSlotInfo()
}

type WRingGetEntriesResponse_Value struct {
	Value uint64 `protobuf:"fixed64,6,opt,name=value,oneof"`
}
type WRingGetEntriesResponse_BarcoGcmDesc struct {
	BarcoGcmDesc *WRingBarcoGCMDescMsg `protobuf:"bytes,7,opt,name=barco_gcm_desc,json=barcoGcmDesc,oneof"`
}
type WRingGetEntriesResponse_Raw struct {
	Raw []byte `protobuf:"bytes,8,opt,name=raw,proto3,oneof"`
}

func (*WRingGetEntriesResponse_Value) isWRingGetEntriesResponse_WRingSlotInfo()        {}
func (*WRingGetEntriesResponse_BarcoGcmDesc) isWRingGetEntriesResponse_WRingSlotInfo() {}
func (*WRingGetEntriesResponse_Raw) isWRingGetEntriesResponse_WRingSlotInfo()          {}

func (m *WRingGetEntriesResponse) GetWRingSlotInfo() isWRingGetEntriesResponse_WRingSlotInfo {
	if m != nil {
		return m.WRingSlotInfo
	}
	return nil
}

func (m *WRingGetEntriesResponse) GetApiStatus() ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return ApiStatus_API_STATUS_OK
}

func (m *WRingGetEntriesResponse) GetSpec() *WRingSpec {
	if m != nil {
		return m.Spec
	}
	return nil
}

func (m *WRingGetEntriesResponse) GetIndex() uint32 {
	if m != nil {
		return m.Index
	}
	return 0
}

func (m *WRingGetEntriesResponse) GetStatus() *WRingStatus {
	if m != nil {
		return m.Status
	}
	return nil
}

func (m *WRingGetEntriesResponse) GetStats() *WRingStats {
	if m != nil {
		return m.Stats
	}
	return nil
}

func (m *WRingGetEntriesResponse) GetValue() uint64 {
	if x, ok := m.GetWRingSlotInfo().(*WRingGetEntriesResponse_Value); ok {
		return x.Value
	}
	return 0
}

func (m *WRingGetEntriesResponse) GetBarcoGcmDesc() *WRingBarcoGCMDescMsg {
	if x, ok := m.GetWRingSlotInfo().(*WRingGetEntriesResponse_BarcoGcmDesc); ok {
		return x.BarcoGcmDesc
	}
	return nil
}

func (m *WRingGetEntriesResponse) GetRaw() []byte {
	if x, ok := m.GetWRingSlotInfo().(*WRingGetEntriesResponse_Raw); ok {
		return x.Raw
	}
	return nil
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*WRingGetEntriesResponse) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _WRingGetEntriesResponse_OneofMarshaler, _WRingGetEntriesResponse_OneofUnmarshaler, _WRingGetEntriesResponse_OneofSizer, []interface{}{
		(*WRingGetEntriesResponse_Value)(nil),
		(*WRingGetEntriesResponse_BarcoGcmDesc)(nil),
		(*WRingGetEntriesResponse_Raw)(nil),
	}
}

func _WRingGetEntriesResponse_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*WRingGetEntriesResponse)
	// WRingSlotInfo
	switch x := m.WRingSlotInfo.(type) {
	case *WRingGetEntriesResponse_Value:
		b.EncodeVarint(6<<3 | proto.WireFixed64)
		b.EncodeFixed64(uint64(x.Value))
	case *WRingGetEntriesResponse_BarcoGcmDesc:
		b.EncodeVarint(7<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.BarcoGcmDesc); err != nil {
			return err
		}
	case *WRingGetEntriesResponse_Raw:
		b.EncodeVarint(8<<3 | proto.WireBytes)
		b.EncodeRawBytes(x.Raw)
	case nil:
	default:
		return fmt.Errorf("WRingGetEntriesResponse.WRingSlotInfo has unexpected type %T", x)
	}
	return nil
}

func _WRingGetEntriesResponse_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*WRingGetEntriesResponse)
	switch tag {
	case 6: // WRingSlotInfo.value
		if wire != proto.WireFixed64 {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeFixed64()
		m.WRingSlotInfo = &WRingGetEntriesResponse_Value{x}
		return true, err
	case 7: // WRingSlotInfo.barco_gcm_desc
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(WRingBarcoGCMDescMsg)
		err := b.DecodeMessage(msg)
		m.WRingSlotInfo = &WRingGetEntriesResponse_BarcoGcmDesc{msg}
		return true, err
	case 8: // WRingSlotInfo.raw
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeRawBytes(true)
		m.WRingSlotInfo = &WRingGetEntriesResponse_Raw{x}
		return true, err
	default:
		return false, nil
	}
}

func _WRingGetEntriesResponse_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*WRingGetEntriesResponse)
	// WRingSlotInfo
	switch x := m.WRingSlotInfo.(type) {
	case *WRingGetEntriesResponse_Value:
		n += proto.SizeVarint(6<<3 | proto.WireFixed64)
		n += 8
	case *WRingGetEntriesResponse_BarcoGcmDesc:
		s := proto.Size(x.BarcoGcmDesc)
		n += proto.SizeVarint(7<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *WRingGetEntriesResponse_Raw:
		n += proto.SizeVarint(8<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(len(x.Raw)))
		n += len(x.Raw)
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

// WRingGetEntriesResponseMsg is batched response to WRingGetEntriesRequestMsg
type WRingGetEntriesResponseMsg struct {
	Response []*WRingGetEntriesResponse `protobuf:"bytes,1,rep,name=response" json:"response,omitempty"`
}

func (m *WRingGetEntriesResponseMsg) Reset()                    { *m = WRingGetEntriesResponseMsg{} }
func (m *WRingGetEntriesResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*WRingGetEntriesResponseMsg) ProtoMessage()               {}
func (*WRingGetEntriesResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{14} }

func (m *WRingGetEntriesResponseMsg) GetResponse() []*WRingGetEntriesResponse {
	if m != nil {
		return m.Response
	}
	return nil
}

// WRingGetMetaResponseMsg captures all the information about a wring metadata
// only if api_status indicates success, other fields are valid
type WRingGetMetaResponse struct {
	ApiStatus ApiStatus  `protobuf:"varint,1,opt,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
	Spec      *WRingSpec `protobuf:"bytes,2,opt,name=spec" json:"spec,omitempty"`
	Pi        uint32     `protobuf:"varint,3,opt,name=pi" json:"pi,omitempty"`
	Ci        uint32     `protobuf:"varint,4,opt,name=ci" json:"ci,omitempty"`
}

func (m *WRingGetMetaResponse) Reset()                    { *m = WRingGetMetaResponse{} }
func (m *WRingGetMetaResponse) String() string            { return proto.CompactTextString(m) }
func (*WRingGetMetaResponse) ProtoMessage()               {}
func (*WRingGetMetaResponse) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{15} }

func (m *WRingGetMetaResponse) GetApiStatus() ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return ApiStatus_API_STATUS_OK
}

func (m *WRingGetMetaResponse) GetSpec() *WRingSpec {
	if m != nil {
		return m.Spec
	}
	return nil
}

func (m *WRingGetMetaResponse) GetPi() uint32 {
	if m != nil {
		return m.Pi
	}
	return 0
}

func (m *WRingGetMetaResponse) GetCi() uint32 {
	if m != nil {
		return m.Ci
	}
	return 0
}

// WRingGetMetaResponse is batched response to WRingGetMetaResponseMsg
type WRingGetMetaResponseMsg struct {
	Response []*WRingGetMetaResponse `protobuf:"bytes,1,rep,name=response" json:"response,omitempty"`
}

func (m *WRingGetMetaResponseMsg) Reset()                    { *m = WRingGetMetaResponseMsg{} }
func (m *WRingGetMetaResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*WRingGetMetaResponseMsg) ProtoMessage()               {}
func (*WRingGetMetaResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor23, []int{16} }

func (m *WRingGetMetaResponseMsg) GetResponse() []*WRingGetMetaResponse {
	if m != nil {
		return m.Response
	}
	return nil
}

func init() {
	proto.RegisterType((*WRingKeyHandle)(nil), "wring.WRingKeyHandle")
	proto.RegisterType((*WRingSpec)(nil), "wring.WRingSpec")
	proto.RegisterType((*WRingRequestMsg)(nil), "wring.WRingRequestMsg")
	proto.RegisterType((*WRingStatus)(nil), "wring.WRingStatus")
	proto.RegisterType((*WRingResponse)(nil), "wring.WRingResponse")
	proto.RegisterType((*WRingResponseMsg)(nil), "wring.WRingResponseMsg")
	proto.RegisterType((*WRingDeleteRequest)(nil), "wring.WRingDeleteRequest")
	proto.RegisterType((*WRingDeleteRequestMsg)(nil), "wring.WRingDeleteRequestMsg")
	proto.RegisterType((*WRingDeleteResponseMsg)(nil), "wring.WRingDeleteResponseMsg")
	proto.RegisterType((*WRingGetEntriesRequest)(nil), "wring.WRingGetEntriesRequest")
	proto.RegisterType((*WRingGetEntriesRequestMsg)(nil), "wring.WRingGetEntriesRequestMsg")
	proto.RegisterType((*WRingStats)(nil), "wring.WRingStats")
	proto.RegisterType((*WRingBarcoGCMDescMsg)(nil), "wring.WRingBarcoGCMDescMsg")
	proto.RegisterType((*WRingGetEntriesResponse)(nil), "wring.WRingGetEntriesResponse")
	proto.RegisterType((*WRingGetEntriesResponseMsg)(nil), "wring.WRingGetEntriesResponseMsg")
	proto.RegisterType((*WRingGetMetaResponse)(nil), "wring.WRingGetMetaResponse")
	proto.RegisterType((*WRingGetMetaResponseMsg)(nil), "wring.WRingGetMetaResponseMsg")
}

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion4

// Client API for WRing service

type WRingClient interface {
	WRingCreate(ctx context.Context, in *WRingRequestMsg, opts ...grpc.CallOption) (*WRingResponseMsg, error)
	WRingUpdate(ctx context.Context, in *WRingRequestMsg, opts ...grpc.CallOption) (*WRingResponseMsg, error)
	WRingDelete(ctx context.Context, in *WRingDeleteRequestMsg, opts ...grpc.CallOption) (*WRingDeleteResponseMsg, error)
	WRingGetEntries(ctx context.Context, in *WRingGetEntriesRequestMsg, opts ...grpc.CallOption) (*WRingGetEntriesResponseMsg, error)
	WRingGetMeta(ctx context.Context, in *WRingRequestMsg, opts ...grpc.CallOption) (*WRingGetMetaResponseMsg, error)
}

type wRingClient struct {
	cc *grpc.ClientConn
}

func NewWRingClient(cc *grpc.ClientConn) WRingClient {
	return &wRingClient{cc}
}

func (c *wRingClient) WRingCreate(ctx context.Context, in *WRingRequestMsg, opts ...grpc.CallOption) (*WRingResponseMsg, error) {
	out := new(WRingResponseMsg)
	err := grpc.Invoke(ctx, "/wring.WRing/WRingCreate", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *wRingClient) WRingUpdate(ctx context.Context, in *WRingRequestMsg, opts ...grpc.CallOption) (*WRingResponseMsg, error) {
	out := new(WRingResponseMsg)
	err := grpc.Invoke(ctx, "/wring.WRing/WRingUpdate", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *wRingClient) WRingDelete(ctx context.Context, in *WRingDeleteRequestMsg, opts ...grpc.CallOption) (*WRingDeleteResponseMsg, error) {
	out := new(WRingDeleteResponseMsg)
	err := grpc.Invoke(ctx, "/wring.WRing/WRingDelete", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *wRingClient) WRingGetEntries(ctx context.Context, in *WRingGetEntriesRequestMsg, opts ...grpc.CallOption) (*WRingGetEntriesResponseMsg, error) {
	out := new(WRingGetEntriesResponseMsg)
	err := grpc.Invoke(ctx, "/wring.WRing/WRingGetEntries", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *wRingClient) WRingGetMeta(ctx context.Context, in *WRingRequestMsg, opts ...grpc.CallOption) (*WRingGetMetaResponseMsg, error) {
	out := new(WRingGetMetaResponseMsg)
	err := grpc.Invoke(ctx, "/wring.WRing/WRingGetMeta", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

// Server API for WRing service

type WRingServer interface {
	WRingCreate(context.Context, *WRingRequestMsg) (*WRingResponseMsg, error)
	WRingUpdate(context.Context, *WRingRequestMsg) (*WRingResponseMsg, error)
	WRingDelete(context.Context, *WRingDeleteRequestMsg) (*WRingDeleteResponseMsg, error)
	WRingGetEntries(context.Context, *WRingGetEntriesRequestMsg) (*WRingGetEntriesResponseMsg, error)
	WRingGetMeta(context.Context, *WRingRequestMsg) (*WRingGetMetaResponseMsg, error)
}

func RegisterWRingServer(s *grpc.Server, srv WRingServer) {
	s.RegisterService(&_WRing_serviceDesc, srv)
}

func _WRing_WRingCreate_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(WRingRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(WRingServer).WRingCreate(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/wring.WRing/WRingCreate",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(WRingServer).WRingCreate(ctx, req.(*WRingRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _WRing_WRingUpdate_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(WRingRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(WRingServer).WRingUpdate(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/wring.WRing/WRingUpdate",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(WRingServer).WRingUpdate(ctx, req.(*WRingRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _WRing_WRingDelete_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(WRingDeleteRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(WRingServer).WRingDelete(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/wring.WRing/WRingDelete",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(WRingServer).WRingDelete(ctx, req.(*WRingDeleteRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _WRing_WRingGetEntries_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(WRingGetEntriesRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(WRingServer).WRingGetEntries(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/wring.WRing/WRingGetEntries",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(WRingServer).WRingGetEntries(ctx, req.(*WRingGetEntriesRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _WRing_WRingGetMeta_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(WRingRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(WRingServer).WRingGetMeta(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/wring.WRing/WRingGetMeta",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(WRingServer).WRingGetMeta(ctx, req.(*WRingRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

var _WRing_serviceDesc = grpc.ServiceDesc{
	ServiceName: "wring.WRing",
	HandlerType: (*WRingServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "WRingCreate",
			Handler:    _WRing_WRingCreate_Handler,
		},
		{
			MethodName: "WRingUpdate",
			Handler:    _WRing_WRingUpdate_Handler,
		},
		{
			MethodName: "WRingDelete",
			Handler:    _WRing_WRingDelete_Handler,
		},
		{
			MethodName: "WRingGetEntries",
			Handler:    _WRing_WRingGetEntries_Handler,
		},
		{
			MethodName: "WRingGetMeta",
			Handler:    _WRing_WRingGetMeta_Handler,
		},
	},
	Streams:  []grpc.StreamDesc{},
	Metadata: "nic/proto/hal/wring.proto",
}

func init() { proto.RegisterFile("nic/proto/hal/wring.proto", fileDescriptor23) }

var fileDescriptor23 = []byte{
	// 876 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xbc, 0x56, 0x6d, 0x6f, 0x1b, 0x45,
	0x10, 0xb6, 0xcf, 0x6f, 0xc9, 0x9c, 0xed, 0x86, 0x55, 0x5e, 0x9c, 0x84, 0x14, 0xf7, 0x1a, 0x44,
	0xd4, 0x0f, 0x49, 0xe5, 0x0a, 0x21, 0x90, 0x90, 0x68, 0x12, 0x14, 0x47, 0x34, 0xaa, 0xb4, 0x14,
	0x51, 0xf1, 0xc5, 0xda, 0xdc, 0x2d, 0xce, 0x92, 0xf3, 0xdd, 0x71, 0xbb, 0x49, 0xf0, 0x8f, 0xe0,
	0x13, 0x5f, 0xf8, 0x25, 0xfc, 0x1b, 0x7e, 0x0a, 0x12, 0xda, 0xd9, 0xb5, 0xef, 0xc5, 0x71, 0x40,
	0x95, 0xda, 0x4f, 0xf1, 0xbc, 0x3d, 0x33, 0xf3, 0xcc, 0xdc, 0x6c, 0x60, 0x3b, 0x12, 0xfe, 0x51,
	0x92, 0xc6, 0x2a, 0x3e, 0xba, 0x62, 0xe1, 0xd1, 0x5d, 0x2a, 0xa2, 0xf1, 0x21, 0xca, 0xa4, 0x81,
	0xc2, 0xce, 0x46, 0xe6, 0xa1, 0xa6, 0x09, 0x97, 0xc6, 0xea, 0xf9, 0xd0, 0xfd, 0x91, 0x8a, 0x68,
	0xfc, 0x1d, 0x9f, 0x0e, 0x59, 0x14, 0x84, 0x9c, 0xec, 0xc2, 0x0a, 0x46, 0x8c, 0x44, 0xd0, 0xab,
	0xf6, 0xab, 0x07, 0x9d, 0x61, 0x85, 0xb6, 0x50, 0x73, 0x1e, 0x90, 0xa7, 0xd0, 0x36, 0xc6, 0x2b,
	0x74, 0xee, 0x39, 0xfd, 0xea, 0x41, 0x73, 0x58, 0xa1, 0x2e, 0x6a, 0x0d, 0xc2, 0xf1, 0x23, 0xe8,
	0x5c, 0xf3, 0xe9, 0x28, 0x4e, 0xad, 0x97, 0xf7, 0x47, 0x15, 0x56, 0x31, 0xcb, 0xf7, 0x09, 0xf7,
	0xc9, 0xa7, 0x50, 0x9f, 0x70, 0xc5, 0x10, 0xdc, 0x1d, 0x7c, 0x74, 0x68, 0xca, 0x79, 0x7d, 0xf9,
	0x0b, 0xf7, 0xd5, 0x05, 0x57, 0x8c, 0xa2, 0x99, 0x7c, 0x59, 0x42, 0xc1, 0x5c, 0xee, 0x60, 0xe3,
	0xd0, 0x34, 0x57, 0xac, 0x9a, 0xba, 0xd7, 0x7c, 0xfa, 0x3a, 0xb5, 0x2d, 0xec, 0x43, 0x5d, 0x83,
	0xf6, 0x6a, 0xfd, 0xea, 0x41, 0x77, 0xb0, 0x66, 0x33, 0x60, 0xc4, 0x9b, 0x69, 0xc2, 0x29, 0x5a,
	0xbd, 0xaf, 0xe1, 0x11, 0xaa, 0x28, 0xff, 0xf5, 0x86, 0x4b, 0x75, 0x21, 0xc7, 0xe4, 0x19, 0xb4,
	0x52, 0x23, 0xf5, 0xaa, 0xfd, 0xda, 0x81, 0x3b, 0x58, 0xcb, 0x67, 0xd3, 0xd5, 0xd3, 0x99, 0x83,
	0xf7, 0x1c, 0x5c, 0xa3, 0x55, 0x4c, 0xdd, 0x48, 0xf2, 0xa4, 0xc4, 0x8c, 0xee, 0xae, 0x59, 0xe0,
	0xc5, 0xbb, 0x83, 0x8e, 0x4d, 0x28, 0x93, 0x38, 0x92, 0x9c, 0x1c, 0x01, 0xb0, 0x44, 0x8c, 0x24,
	0x22, 0x60, 0x44, 0x56, 0xed, 0xcb, 0x44, 0x18, 0x64, 0xba, 0xca, 0x66, 0x3f, 0xc9, 0xe7, 0xb3,
	0x24, 0x36, 0xc4, 0x50, 0x42, 0x0a, 0x45, 0x9a, 0x20, 0x93, 0xd8, 0x08, 0xde, 0x29, 0xac, 0x15,
	0x12, 0xeb, 0x56, 0x9f, 0xc3, 0x4a, 0x6a, 0x45, 0xdb, 0xeb, 0x7a, 0x1e, 0x66, 0xe6, 0x4a, 0xe7,
	0x5e, 0xde, 0x2d, 0x10, 0x34, 0x9d, 0xf2, 0x90, 0x2b, 0x6e, 0x59, 0x7b, 0xff, 0xd3, 0xf4, 0x5e,
	0xc1, 0xc6, 0x62, 0x5e, 0xdd, 0xc2, 0x8b, 0xf2, 0xb4, 0xb6, 0xf3, 0x68, 0x05, 0xf7, 0x6c, 0x6c,
	0xe7, 0xb0, 0x59, 0x30, 0x67, 0x8c, 0x94, 0xa7, 0x51, 0xfb, 0x8f, 0x69, 0x78, 0x7f, 0x55, 0x2d,
	0xd6, 0x19, 0x57, 0xdf, 0x46, 0x2a, 0x15, 0x5c, 0x7e, 0x30, 0x56, 0xfe, 0xdf, 0x8e, 0x93, 0x75,
	0x68, 0x88, 0x28, 0xe0, 0xbf, 0xf5, 0xea, 0xfa, 0x4b, 0xa6, 0x46, 0xf0, 0xde, 0xc0, 0xf6, 0xfd,
	0x75, 0x6b, 0x1a, 0xbe, 0x28, 0xb3, 0xba, 0x97, 0xaf, 0x66, 0x21, 0x24, 0x63, 0xb6, 0x0d, 0x30,
	0xdf, 0x40, 0xe9, 0xfd, 0xe9, 0xc0, 0x3a, 0x8a, 0xc7, 0x2c, 0xf5, 0xe3, 0xb3, 0x93, 0x8b, 0x53,
	0x2e, 0x7d, 0x8d, 0xbf, 0x07, 0x20, 0x42, 0x21, 0xd5, 0x88, 0x05, 0x41, 0x6a, 0x3f, 0x93, 0x55,
	0xd4, 0xbc, 0x0c, 0x82, 0x54, 0x9b, 0xe3, 0xcc, 0xec, 0x18, 0x73, 0x3c, 0x37, 0xf7, 0xa0, 0xe5,
	0xc7, 0x93, 0x09, 0x8b, 0x02, 0xec, 0xbc, 0x43, 0x67, 0x22, 0xd9, 0x87, 0xae, 0xe6, 0x32, 0xe0,
	0xd2, 0x1f, 0xe5, 0x7b, 0x6e, 0x5f, 0xf3, 0xa9, 0xce, 0x7d, 0xae, 0x75, 0x64, 0x0b, 0x5a, 0xe2,
	0xd6, 0x60, 0x37, 0x10, 0xbb, 0x29, 0x6e, 0x11, 0xf8, 0x13, 0x70, 0xcd, 0xe4, 0x8d, 0xb1, 0x89,
	0x46, 0x30, 0x2a, 0x74, 0x78, 0x0a, 0x9d, 0x20, 0x8e, 0xd3, 0x4b, 0x1e, 0x86, 0xc6, 0xa5, 0x85,
	0x2e, 0xed, 0x99, 0x72, 0xc1, 0x29, 0x60, 0x8a, 0xf5, 0x56, 0x8a, 0x4e, 0xa7, 0x4c, 0x31, 0xef,
	0x6f, 0x07, 0xb6, 0x16, 0xc8, 0x7c, 0xd7, 0x93, 0xb0, 0x0f, 0x75, 0x99, 0x70, 0xdf, 0x6e, 0xce,
	0xe2, 0xbd, 0x42, 0x6b, 0xb6, 0x07, 0xb5, 0xdc, 0x1e, 0x90, 0x67, 0xd0, 0xb4, 0x89, 0xea, 0x4b,
	0x0f, 0x89, 0xf5, 0x20, 0x9f, 0x41, 0x43, 0xff, 0x92, 0x48, 0x9b, 0x5e, 0xe9, 0x92, 0xab, 0xa4,
	0xc6, 0x4e, 0x36, 0xa1, 0x71, 0xcb, 0xc2, 0x1b, 0x6e, 0x28, 0x1c, 0x56, 0xa8, 0x11, 0xc9, 0x09,
	0x74, 0x2f, 0xf5, 0x2a, 0x8c, 0xc6, 0xfe, 0x04, 0xa7, 0x84, 0x04, 0xba, 0x83, 0xdd, 0x3c, 0x52,
	0x69, 0x59, 0x86, 0x15, 0xda, 0xc6, 0xa0, 0x33, 0x7f, 0xa2, 0x55, 0x84, 0x40, 0x2d, 0x65, 0x77,
	0xc8, 0x6a, 0x7b, 0x58, 0xa1, 0x5a, 0xd0, 0xcf, 0x8d, 0xa9, 0x22, 0x8c, 0xd5, 0x79, 0xf4, 0x73,
	0xec, 0xbd, 0x85, 0x9d, 0x25, 0xf4, 0xea, 0xfd, 0xfb, 0x6a, 0xe1, 0xf0, 0x3d, 0x5e, 0xb6, 0xe0,
	0x0b, 0x27, 0xf0, 0xf7, 0xaa, 0x5d, 0xea, 0x33, 0x6e, 0x3e, 0xe3, 0xf7, 0x3c, 0xb6, 0x2e, 0x38,
	0x89, 0xb0, 0x33, 0x73, 0x12, 0xa1, 0x65, 0x5f, 0xd8, 0xbd, 0x76, 0x7c, 0xe1, 0xd1, 0x6c, 0x91,
	0xf2, 0xe5, 0x98, 0xcf, 0xb8, 0xdc, 0xe6, 0x6e, 0xa9, 0xcd, 0x7c, 0x44, 0xd6, 0xe3, 0xe0, 0x1f,
	0x07, 0x1a, 0xe8, 0x42, 0xbe, 0xb1, 0x2f, 0xdc, 0x49, 0xca, 0x99, 0xe2, 0x64, 0xb3, 0xf8, 0x3e,
	0xcc, 0x0e, 0xc6, 0xce, 0xd6, 0x7d, 0xef, 0xc6, 0x85, 0x1c, 0x7b, 0x95, 0x39, 0xc2, 0x0f, 0x49,
	0xf0, 0x8e, 0x08, 0xaf, 0x2c, 0x82, 0x39, 0xd7, 0xe4, 0xe3, 0xa5, 0x17, 0x5e, 0xe3, 0xec, 0xdd,
	0x67, 0xcd, 0xa3, 0xbd, 0xb5, 0x4f, 0x7e, 0x36, 0x64, 0xd2, 0x7f, 0xf0, 0xba, 0x69, 0xd4, 0x27,
	0x0f, 0xaf, 0x87, 0x41, 0x1e, 0x42, 0x3b, 0xcf, 0xeb, 0xd2, 0x56, 0x1f, 0x3f, 0x30, 0x04, 0x44,
	0x3a, 0x86, 0x9f, 0x56, 0xae, 0x58, 0x88, 0xff, 0x9d, 0x5d, 0x36, 0xf1, 0xcf, 0x8b, 0x7f, 0x03,
	0x00, 0x00, 0xff, 0xff, 0x3c, 0xd9, 0x80, 0xfc, 0xdf, 0x09, 0x00, 0x00,
}
