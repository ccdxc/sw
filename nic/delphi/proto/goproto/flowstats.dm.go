// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

type L2FlowKey struct {
	svrf uint32 `protobuf:"varint,1,opt,name=svrf,json=svrf" json:"svrf,omitempty"`

	dvrf uint32 `protobuf:"varint,2,opt,name=dvrf,json=dvrf" json:"dvrf,omitempty"`

	l2seg_id uint32 `protobuf:"varint,3,opt,name=l2seg_id,json=l2seg_id" json:"l2seg_id,omitempty"`

	ether_type uint32 `protobuf:"varint,4,opt,name=ether_type,json=ether_type" json:"ether_type,omitempty"`

	smac uint64 `protobuf:"varint,5,opt,name=smac,json=smac" json:"smac,omitempty"`

	dmac uint64 `protobuf:"varint,6,opt,name=dmac,json=dmac" json:"dmac,omitempty"`
}

func (m *L2FlowKey) Reset()         { *m = L2FlowKey{} }
func (m *L2FlowKey) String() string { return proto.CompactTextString(m) }
func (*L2FlowKey) ProtoMessage()    {}

type IPv4FlowKey struct {
	svrf uint32 `protobuf:"varint,1,opt,name=svrf,json=svrf" json:"svrf,omitempty"`

	dvrf uint32 `protobuf:"varint,2,opt,name=dvrf,json=dvrf" json:"dvrf,omitempty"`

	sip uint32 `protobuf:"varint,3,opt,name=sip,json=sip" json:"sip,omitempty"`

	dip uint32 `protobuf:"varint,4,opt,name=dip,json=dip" json:"dip,omitempty"`

	sport uint32 `protobuf:"varint,5,opt,name=sport,json=sport" json:"sport,omitempty"`

	dport uint32 `protobuf:"varint,6,opt,name=dport,json=dport" json:"dport,omitempty"`

	ip_proto uint32 `protobuf:"varint,7,opt,name=ip_proto,json=ip_proto" json:"ip_proto,omitempty"`
}

func (m *IPv4FlowKey) Reset()         { *m = IPv4FlowKey{} }
func (m *IPv4FlowKey) String() string { return proto.CompactTextString(m) }
func (*IPv4FlowKey) ProtoMessage()    {}

type IPv6FlowKey struct {
	svrf uint32 `protobuf:"varint,1,opt,name=svrf,json=svrf" json:"svrf,omitempty"`

	dvrf uint32 `protobuf:"varint,2,opt,name=dvrf,json=dvrf" json:"dvrf,omitempty"`

	sip_hi uint64 `protobuf:"varint,3,opt,name=sip_hi,json=sip_hi" json:"sip_hi,omitempty"`

	sip_lo uint64 `protobuf:"varint,4,opt,name=sip_lo,json=sip_lo" json:"sip_lo,omitempty"`

	dip_hi uint64 `protobuf:"varint,5,opt,name=dip_hi,json=dip_hi" json:"dip_hi,omitempty"`

	dip_lo uint64 `protobuf:"varint,6,opt,name=dip_lo,json=dip_lo" json:"dip_lo,omitempty"`

	sport uint32 `protobuf:"varint,7,opt,name=sport,json=sport" json:"sport,omitempty"`

	dport uint32 `protobuf:"varint,8,opt,name=dport,json=dport" json:"dport,omitempty"`

	ip_proto uint32 `protobuf:"varint,9,opt,name=ip_proto,json=ip_proto" json:"ip_proto,omitempty"`
}

func (m *IPv6FlowKey) Reset()         { *m = IPv6FlowKey{} }
func (m *IPv6FlowKey) String() string { return proto.CompactTextString(m) }
func (*IPv6FlowKey) ProtoMessage()    {}

type L2FlowRawMetrics struct {
	ObjectMeta api.ObjectMeta

	key L2FlowKey

	Instances metrics.Counter

	Packets metrics.Counter

	Bytes metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *L2FlowRawMetrics) GetKey() L2FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *L2FlowRawMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.Packets.Size()

	sz += mtr.Bytes.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *L2FlowRawMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.Packets = mtr.metrics.GetCounter(offset)
	offset += mtr.Packets.Size()

	mtr.Bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Bytes.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *L2FlowRawMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "Packets" {
		return offset
	}
	offset += mtr.Packets.Size()

	if fldName == "Bytes" {
		return offset
	}
	offset += mtr.Bytes.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *L2FlowRawMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetPackets sets cunter in shared memory
func (mtr *L2FlowRawMetrics) SetPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Packets"))
	return nil
}

// SetBytes sets cunter in shared memory
func (mtr *L2FlowRawMetrics) SetBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bytes"))
	return nil
}

// L2FlowRawMetricsIterator is the iterator object
type L2FlowRawMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *L2FlowRawMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *L2FlowRawMetricsIterator) Next() *L2FlowRawMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &L2FlowRawMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *L2FlowRawMetricsIterator) Find(key L2FlowKey) (*L2FlowRawMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &L2FlowRawMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *L2FlowRawMetricsIterator) Create(key L2FlowKey) (*L2FlowRawMetrics, error) {
	tmtr := &L2FlowRawMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &L2FlowRawMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *L2FlowRawMetricsIterator) Delete(key L2FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *L2FlowRawMetricsIterator) Free() {
	it.iter.Free()
}

// NewL2FlowRawMetricsIterator returns an iterator
func NewL2FlowRawMetricsIterator() (*L2FlowRawMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("L2FlowRawMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &L2FlowRawMetricsIterator{iter: iter}, nil
}

type IPv4FlowRawMetrics struct {
	ObjectMeta api.ObjectMeta

	key IPv4FlowKey

	Instances metrics.Counter

	Packets metrics.Counter

	Bytes metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *IPv4FlowRawMetrics) GetKey() IPv4FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IPv4FlowRawMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.Packets.Size()

	sz += mtr.Bytes.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *IPv4FlowRawMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.Packets = mtr.metrics.GetCounter(offset)
	offset += mtr.Packets.Size()

	mtr.Bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Bytes.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *IPv4FlowRawMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "Packets" {
		return offset
	}
	offset += mtr.Packets.Size()

	if fldName == "Bytes" {
		return offset
	}
	offset += mtr.Bytes.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *IPv4FlowRawMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetPackets sets cunter in shared memory
func (mtr *IPv4FlowRawMetrics) SetPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Packets"))
	return nil
}

// SetBytes sets cunter in shared memory
func (mtr *IPv4FlowRawMetrics) SetBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bytes"))
	return nil
}

// IPv4FlowRawMetricsIterator is the iterator object
type IPv4FlowRawMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IPv4FlowRawMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IPv4FlowRawMetricsIterator) Next() *IPv4FlowRawMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &IPv4FlowRawMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IPv4FlowRawMetricsIterator) Find(key IPv4FlowKey) (*IPv4FlowRawMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &IPv4FlowRawMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IPv4FlowRawMetricsIterator) Create(key IPv4FlowKey) (*IPv4FlowRawMetrics, error) {
	tmtr := &IPv4FlowRawMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &IPv4FlowRawMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IPv4FlowRawMetricsIterator) Delete(key IPv4FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *IPv4FlowRawMetricsIterator) Free() {
	it.iter.Free()
}

// NewIPv4FlowRawMetricsIterator returns an iterator
func NewIPv4FlowRawMetricsIterator() (*IPv4FlowRawMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IPv4FlowRawMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IPv4FlowRawMetricsIterator{iter: iter}, nil
}

type IPv6FlowRawMetrics struct {
	ObjectMeta api.ObjectMeta

	key IPv6FlowKey

	Instances metrics.Counter

	Packets metrics.Counter

	Bytes metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *IPv6FlowRawMetrics) GetKey() IPv6FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IPv6FlowRawMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.Packets.Size()

	sz += mtr.Bytes.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *IPv6FlowRawMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.Packets = mtr.metrics.GetCounter(offset)
	offset += mtr.Packets.Size()

	mtr.Bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Bytes.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *IPv6FlowRawMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "Packets" {
		return offset
	}
	offset += mtr.Packets.Size()

	if fldName == "Bytes" {
		return offset
	}
	offset += mtr.Bytes.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *IPv6FlowRawMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetPackets sets cunter in shared memory
func (mtr *IPv6FlowRawMetrics) SetPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Packets"))
	return nil
}

// SetBytes sets cunter in shared memory
func (mtr *IPv6FlowRawMetrics) SetBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bytes"))
	return nil
}

// IPv6FlowRawMetricsIterator is the iterator object
type IPv6FlowRawMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IPv6FlowRawMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IPv6FlowRawMetricsIterator) Next() *IPv6FlowRawMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &IPv6FlowRawMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IPv6FlowRawMetricsIterator) Find(key IPv6FlowKey) (*IPv6FlowRawMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &IPv6FlowRawMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IPv6FlowRawMetricsIterator) Create(key IPv6FlowKey) (*IPv6FlowRawMetrics, error) {
	tmtr := &IPv6FlowRawMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &IPv6FlowRawMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IPv6FlowRawMetricsIterator) Delete(key IPv6FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *IPv6FlowRawMetricsIterator) Free() {
	it.iter.Free()
}

// NewIPv6FlowRawMetricsIterator returns an iterator
func NewIPv6FlowRawMetricsIterator() (*IPv6FlowRawMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IPv6FlowRawMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IPv6FlowRawMetricsIterator{iter: iter}, nil
}

type L2FlowDropMetrics struct {
	ObjectMeta api.ObjectMeta

	key L2FlowKey

	Instances metrics.Counter

	DropPackets metrics.Counter

	DropBytes metrics.Counter

	DropFirstTimestamp metrics.Gauge

	DropLastTimestamp metrics.Gauge

	DropReason metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *L2FlowDropMetrics) GetKey() L2FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *L2FlowDropMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.DropPackets.Size()

	sz += mtr.DropBytes.Size()

	sz += mtr.DropFirstTimestamp.Size()

	sz += mtr.DropLastTimestamp.Size()

	sz += mtr.DropReason.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *L2FlowDropMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.DropPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.DropPackets.Size()

	mtr.DropBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.DropBytes.Size()

	mtr.DropFirstTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.DropFirstTimestamp.Size()

	mtr.DropLastTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.DropLastTimestamp.Size()

	mtr.DropReason = mtr.metrics.GetCounter(offset)
	offset += mtr.DropReason.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *L2FlowDropMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "DropPackets" {
		return offset
	}
	offset += mtr.DropPackets.Size()

	if fldName == "DropBytes" {
		return offset
	}
	offset += mtr.DropBytes.Size()

	if fldName == "DropFirstTimestamp" {
		return offset
	}
	offset += mtr.DropFirstTimestamp.Size()

	if fldName == "DropLastTimestamp" {
		return offset
	}
	offset += mtr.DropLastTimestamp.Size()

	if fldName == "DropReason" {
		return offset
	}
	offset += mtr.DropReason.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *L2FlowDropMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetDropPackets sets cunter in shared memory
func (mtr *L2FlowDropMetrics) SetDropPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropPackets"))
	return nil
}

// SetDropBytes sets cunter in shared memory
func (mtr *L2FlowDropMetrics) SetDropBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropBytes"))
	return nil
}

// SetDropFirstTimestamp sets gauge in shared memory
func (mtr *L2FlowDropMetrics) SetDropFirstTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("DropFirstTimestamp"))
	return nil
}

// SetDropLastTimestamp sets gauge in shared memory
func (mtr *L2FlowDropMetrics) SetDropLastTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("DropLastTimestamp"))
	return nil
}

// SetDropReason sets cunter in shared memory
func (mtr *L2FlowDropMetrics) SetDropReason(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropReason"))
	return nil
}

// L2FlowDropMetricsIterator is the iterator object
type L2FlowDropMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *L2FlowDropMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *L2FlowDropMetricsIterator) Next() *L2FlowDropMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &L2FlowDropMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *L2FlowDropMetricsIterator) Find(key L2FlowKey) (*L2FlowDropMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &L2FlowDropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *L2FlowDropMetricsIterator) Create(key L2FlowKey) (*L2FlowDropMetrics, error) {
	tmtr := &L2FlowDropMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &L2FlowDropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *L2FlowDropMetricsIterator) Delete(key L2FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *L2FlowDropMetricsIterator) Free() {
	it.iter.Free()
}

// NewL2FlowDropMetricsIterator returns an iterator
func NewL2FlowDropMetricsIterator() (*L2FlowDropMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("L2FlowDropMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &L2FlowDropMetricsIterator{iter: iter}, nil
}

type IPv4FlowDropMetrics struct {
	ObjectMeta api.ObjectMeta

	key IPv4FlowKey

	Instances metrics.Counter

	DropPackets metrics.Counter

	DropBytes metrics.Counter

	DropFirstTimestamp metrics.Gauge

	DropLastTimestamp metrics.Gauge

	DropReason metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *IPv4FlowDropMetrics) GetKey() IPv4FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IPv4FlowDropMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.DropPackets.Size()

	sz += mtr.DropBytes.Size()

	sz += mtr.DropFirstTimestamp.Size()

	sz += mtr.DropLastTimestamp.Size()

	sz += mtr.DropReason.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *IPv4FlowDropMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.DropPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.DropPackets.Size()

	mtr.DropBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.DropBytes.Size()

	mtr.DropFirstTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.DropFirstTimestamp.Size()

	mtr.DropLastTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.DropLastTimestamp.Size()

	mtr.DropReason = mtr.metrics.GetCounter(offset)
	offset += mtr.DropReason.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *IPv4FlowDropMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "DropPackets" {
		return offset
	}
	offset += mtr.DropPackets.Size()

	if fldName == "DropBytes" {
		return offset
	}
	offset += mtr.DropBytes.Size()

	if fldName == "DropFirstTimestamp" {
		return offset
	}
	offset += mtr.DropFirstTimestamp.Size()

	if fldName == "DropLastTimestamp" {
		return offset
	}
	offset += mtr.DropLastTimestamp.Size()

	if fldName == "DropReason" {
		return offset
	}
	offset += mtr.DropReason.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *IPv4FlowDropMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetDropPackets sets cunter in shared memory
func (mtr *IPv4FlowDropMetrics) SetDropPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropPackets"))
	return nil
}

// SetDropBytes sets cunter in shared memory
func (mtr *IPv4FlowDropMetrics) SetDropBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropBytes"))
	return nil
}

// SetDropFirstTimestamp sets gauge in shared memory
func (mtr *IPv4FlowDropMetrics) SetDropFirstTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("DropFirstTimestamp"))
	return nil
}

// SetDropLastTimestamp sets gauge in shared memory
func (mtr *IPv4FlowDropMetrics) SetDropLastTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("DropLastTimestamp"))
	return nil
}

// SetDropReason sets cunter in shared memory
func (mtr *IPv4FlowDropMetrics) SetDropReason(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropReason"))
	return nil
}

// IPv4FlowDropMetricsIterator is the iterator object
type IPv4FlowDropMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IPv4FlowDropMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IPv4FlowDropMetricsIterator) Next() *IPv4FlowDropMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &IPv4FlowDropMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IPv4FlowDropMetricsIterator) Find(key IPv4FlowKey) (*IPv4FlowDropMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &IPv4FlowDropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IPv4FlowDropMetricsIterator) Create(key IPv4FlowKey) (*IPv4FlowDropMetrics, error) {
	tmtr := &IPv4FlowDropMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &IPv4FlowDropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IPv4FlowDropMetricsIterator) Delete(key IPv4FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *IPv4FlowDropMetricsIterator) Free() {
	it.iter.Free()
}

// NewIPv4FlowDropMetricsIterator returns an iterator
func NewIPv4FlowDropMetricsIterator() (*IPv4FlowDropMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IPv4FlowDropMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IPv4FlowDropMetricsIterator{iter: iter}, nil
}

type IPv6FlowDropMetrics struct {
	ObjectMeta api.ObjectMeta

	key IPv6FlowKey

	Instances metrics.Counter

	DropPackets metrics.Counter

	DropBytes metrics.Counter

	DropFirstTimestamp metrics.Gauge

	DropLastTimestamp metrics.Gauge

	DropReason metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *IPv6FlowDropMetrics) GetKey() IPv6FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IPv6FlowDropMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.DropPackets.Size()

	sz += mtr.DropBytes.Size()

	sz += mtr.DropFirstTimestamp.Size()

	sz += mtr.DropLastTimestamp.Size()

	sz += mtr.DropReason.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *IPv6FlowDropMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.DropPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.DropPackets.Size()

	mtr.DropBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.DropBytes.Size()

	mtr.DropFirstTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.DropFirstTimestamp.Size()

	mtr.DropLastTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.DropLastTimestamp.Size()

	mtr.DropReason = mtr.metrics.GetCounter(offset)
	offset += mtr.DropReason.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *IPv6FlowDropMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "DropPackets" {
		return offset
	}
	offset += mtr.DropPackets.Size()

	if fldName == "DropBytes" {
		return offset
	}
	offset += mtr.DropBytes.Size()

	if fldName == "DropFirstTimestamp" {
		return offset
	}
	offset += mtr.DropFirstTimestamp.Size()

	if fldName == "DropLastTimestamp" {
		return offset
	}
	offset += mtr.DropLastTimestamp.Size()

	if fldName == "DropReason" {
		return offset
	}
	offset += mtr.DropReason.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *IPv6FlowDropMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetDropPackets sets cunter in shared memory
func (mtr *IPv6FlowDropMetrics) SetDropPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropPackets"))
	return nil
}

// SetDropBytes sets cunter in shared memory
func (mtr *IPv6FlowDropMetrics) SetDropBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropBytes"))
	return nil
}

// SetDropFirstTimestamp sets gauge in shared memory
func (mtr *IPv6FlowDropMetrics) SetDropFirstTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("DropFirstTimestamp"))
	return nil
}

// SetDropLastTimestamp sets gauge in shared memory
func (mtr *IPv6FlowDropMetrics) SetDropLastTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("DropLastTimestamp"))
	return nil
}

// SetDropReason sets cunter in shared memory
func (mtr *IPv6FlowDropMetrics) SetDropReason(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropReason"))
	return nil
}

// IPv6FlowDropMetricsIterator is the iterator object
type IPv6FlowDropMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IPv6FlowDropMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IPv6FlowDropMetricsIterator) Next() *IPv6FlowDropMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &IPv6FlowDropMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IPv6FlowDropMetricsIterator) Find(key IPv6FlowKey) (*IPv6FlowDropMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &IPv6FlowDropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IPv6FlowDropMetricsIterator) Create(key IPv6FlowKey) (*IPv6FlowDropMetrics, error) {
	tmtr := &IPv6FlowDropMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &IPv6FlowDropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IPv6FlowDropMetricsIterator) Delete(key IPv6FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *IPv6FlowDropMetricsIterator) Free() {
	it.iter.Free()
}

// NewIPv6FlowDropMetricsIterator returns an iterator
func NewIPv6FlowDropMetricsIterator() (*IPv6FlowDropMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IPv6FlowDropMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IPv6FlowDropMetricsIterator{iter: iter}, nil
}

type L2FlowPerformanceMetrics struct {
	ObjectMeta api.ObjectMeta

	key L2FlowKey

	Instances metrics.Counter

	PeakPps metrics.Gauge

	PeakPpsTimestamp metrics.Gauge

	PeakBw metrics.Gauge

	PeakBwTimestamp metrics.Gauge

	// private state
	metrics gometrics.Metrics
}

func (mtr *L2FlowPerformanceMetrics) GetKey() L2FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *L2FlowPerformanceMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.PeakPps.Size()

	sz += mtr.PeakPpsTimestamp.Size()

	sz += mtr.PeakBw.Size()

	sz += mtr.PeakBwTimestamp.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *L2FlowPerformanceMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.PeakPps = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakPps.Size()

	mtr.PeakPpsTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakPpsTimestamp.Size()

	mtr.PeakBw = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakBw.Size()

	mtr.PeakBwTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakBwTimestamp.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *L2FlowPerformanceMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "PeakPps" {
		return offset
	}
	offset += mtr.PeakPps.Size()

	if fldName == "PeakPpsTimestamp" {
		return offset
	}
	offset += mtr.PeakPpsTimestamp.Size()

	if fldName == "PeakBw" {
		return offset
	}
	offset += mtr.PeakBw.Size()

	if fldName == "PeakBwTimestamp" {
		return offset
	}
	offset += mtr.PeakBwTimestamp.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *L2FlowPerformanceMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetPeakPps sets gauge in shared memory
func (mtr *L2FlowPerformanceMetrics) SetPeakPps(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakPps"))
	return nil
}

// SetPeakPpsTimestamp sets gauge in shared memory
func (mtr *L2FlowPerformanceMetrics) SetPeakPpsTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakPpsTimestamp"))
	return nil
}

// SetPeakBw sets gauge in shared memory
func (mtr *L2FlowPerformanceMetrics) SetPeakBw(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakBw"))
	return nil
}

// SetPeakBwTimestamp sets gauge in shared memory
func (mtr *L2FlowPerformanceMetrics) SetPeakBwTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakBwTimestamp"))
	return nil
}

// L2FlowPerformanceMetricsIterator is the iterator object
type L2FlowPerformanceMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *L2FlowPerformanceMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *L2FlowPerformanceMetricsIterator) Next() *L2FlowPerformanceMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &L2FlowPerformanceMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *L2FlowPerformanceMetricsIterator) Find(key L2FlowKey) (*L2FlowPerformanceMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &L2FlowPerformanceMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *L2FlowPerformanceMetricsIterator) Create(key L2FlowKey) (*L2FlowPerformanceMetrics, error) {
	tmtr := &L2FlowPerformanceMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &L2FlowPerformanceMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *L2FlowPerformanceMetricsIterator) Delete(key L2FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *L2FlowPerformanceMetricsIterator) Free() {
	it.iter.Free()
}

// NewL2FlowPerformanceMetricsIterator returns an iterator
func NewL2FlowPerformanceMetricsIterator() (*L2FlowPerformanceMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("L2FlowPerformanceMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &L2FlowPerformanceMetricsIterator{iter: iter}, nil
}

type IPv4FlowPerformanceMetrics struct {
	ObjectMeta api.ObjectMeta

	key IPv4FlowKey

	Instances metrics.Counter

	PeakPps metrics.Gauge

	PeakPpsTimestamp metrics.Gauge

	PeakBw metrics.Gauge

	PeakBwTimestamp metrics.Gauge

	// private state
	metrics gometrics.Metrics
}

func (mtr *IPv4FlowPerformanceMetrics) GetKey() IPv4FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IPv4FlowPerformanceMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.PeakPps.Size()

	sz += mtr.PeakPpsTimestamp.Size()

	sz += mtr.PeakBw.Size()

	sz += mtr.PeakBwTimestamp.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *IPv4FlowPerformanceMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.PeakPps = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakPps.Size()

	mtr.PeakPpsTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakPpsTimestamp.Size()

	mtr.PeakBw = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakBw.Size()

	mtr.PeakBwTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakBwTimestamp.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *IPv4FlowPerformanceMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "PeakPps" {
		return offset
	}
	offset += mtr.PeakPps.Size()

	if fldName == "PeakPpsTimestamp" {
		return offset
	}
	offset += mtr.PeakPpsTimestamp.Size()

	if fldName == "PeakBw" {
		return offset
	}
	offset += mtr.PeakBw.Size()

	if fldName == "PeakBwTimestamp" {
		return offset
	}
	offset += mtr.PeakBwTimestamp.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *IPv4FlowPerformanceMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetPeakPps sets gauge in shared memory
func (mtr *IPv4FlowPerformanceMetrics) SetPeakPps(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakPps"))
	return nil
}

// SetPeakPpsTimestamp sets gauge in shared memory
func (mtr *IPv4FlowPerformanceMetrics) SetPeakPpsTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakPpsTimestamp"))
	return nil
}

// SetPeakBw sets gauge in shared memory
func (mtr *IPv4FlowPerformanceMetrics) SetPeakBw(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakBw"))
	return nil
}

// SetPeakBwTimestamp sets gauge in shared memory
func (mtr *IPv4FlowPerformanceMetrics) SetPeakBwTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakBwTimestamp"))
	return nil
}

// IPv4FlowPerformanceMetricsIterator is the iterator object
type IPv4FlowPerformanceMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IPv4FlowPerformanceMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IPv4FlowPerformanceMetricsIterator) Next() *IPv4FlowPerformanceMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &IPv4FlowPerformanceMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IPv4FlowPerformanceMetricsIterator) Find(key IPv4FlowKey) (*IPv4FlowPerformanceMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &IPv4FlowPerformanceMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IPv4FlowPerformanceMetricsIterator) Create(key IPv4FlowKey) (*IPv4FlowPerformanceMetrics, error) {
	tmtr := &IPv4FlowPerformanceMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &IPv4FlowPerformanceMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IPv4FlowPerformanceMetricsIterator) Delete(key IPv4FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *IPv4FlowPerformanceMetricsIterator) Free() {
	it.iter.Free()
}

// NewIPv4FlowPerformanceMetricsIterator returns an iterator
func NewIPv4FlowPerformanceMetricsIterator() (*IPv4FlowPerformanceMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IPv4FlowPerformanceMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IPv4FlowPerformanceMetricsIterator{iter: iter}, nil
}

type IPv6FlowPerformanceMetrics struct {
	ObjectMeta api.ObjectMeta

	key IPv6FlowKey

	Instances metrics.Counter

	PeakPps metrics.Gauge

	PeakPpsTimestamp metrics.Gauge

	PeakBw metrics.Gauge

	PeakBwTimestamp metrics.Gauge

	// private state
	metrics gometrics.Metrics
}

func (mtr *IPv6FlowPerformanceMetrics) GetKey() IPv6FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IPv6FlowPerformanceMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.PeakPps.Size()

	sz += mtr.PeakPpsTimestamp.Size()

	sz += mtr.PeakBw.Size()

	sz += mtr.PeakBwTimestamp.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *IPv6FlowPerformanceMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.PeakPps = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakPps.Size()

	mtr.PeakPpsTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakPpsTimestamp.Size()

	mtr.PeakBw = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakBw.Size()

	mtr.PeakBwTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PeakBwTimestamp.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *IPv6FlowPerformanceMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "PeakPps" {
		return offset
	}
	offset += mtr.PeakPps.Size()

	if fldName == "PeakPpsTimestamp" {
		return offset
	}
	offset += mtr.PeakPpsTimestamp.Size()

	if fldName == "PeakBw" {
		return offset
	}
	offset += mtr.PeakBw.Size()

	if fldName == "PeakBwTimestamp" {
		return offset
	}
	offset += mtr.PeakBwTimestamp.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *IPv6FlowPerformanceMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetPeakPps sets gauge in shared memory
func (mtr *IPv6FlowPerformanceMetrics) SetPeakPps(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakPps"))
	return nil
}

// SetPeakPpsTimestamp sets gauge in shared memory
func (mtr *IPv6FlowPerformanceMetrics) SetPeakPpsTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakPpsTimestamp"))
	return nil
}

// SetPeakBw sets gauge in shared memory
func (mtr *IPv6FlowPerformanceMetrics) SetPeakBw(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakBw"))
	return nil
}

// SetPeakBwTimestamp sets gauge in shared memory
func (mtr *IPv6FlowPerformanceMetrics) SetPeakBwTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PeakBwTimestamp"))
	return nil
}

// IPv6FlowPerformanceMetricsIterator is the iterator object
type IPv6FlowPerformanceMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IPv6FlowPerformanceMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IPv6FlowPerformanceMetricsIterator) Next() *IPv6FlowPerformanceMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &IPv6FlowPerformanceMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IPv6FlowPerformanceMetricsIterator) Find(key IPv6FlowKey) (*IPv6FlowPerformanceMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &IPv6FlowPerformanceMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IPv6FlowPerformanceMetricsIterator) Create(key IPv6FlowKey) (*IPv6FlowPerformanceMetrics, error) {
	tmtr := &IPv6FlowPerformanceMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &IPv6FlowPerformanceMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IPv6FlowPerformanceMetricsIterator) Delete(key IPv6FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *IPv6FlowPerformanceMetricsIterator) Free() {
	it.iter.Free()
}

// NewIPv6FlowPerformanceMetricsIterator returns an iterator
func NewIPv6FlowPerformanceMetricsIterator() (*IPv6FlowPerformanceMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IPv6FlowPerformanceMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IPv6FlowPerformanceMetricsIterator{iter: iter}, nil
}

type IPv4FlowLatencyMetrics struct {
	ObjectMeta api.ObjectMeta

	key IPv4FlowKey

	Instances metrics.Counter

	MinSetupLatency metrics.Counter

	MinSetupLatencyTimestamp metrics.Gauge

	MaxSetupLatency metrics.Counter

	MaxSetupLatencyTimestamp metrics.Gauge

	MinRttLatency metrics.Counter

	MinRttLatencyTimestamp metrics.Gauge

	MaxRttLatency metrics.Counter

	MaxRttLatencyTimestamp metrics.Gauge

	// private state
	metrics gometrics.Metrics
}

func (mtr *IPv4FlowLatencyMetrics) GetKey() IPv4FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IPv4FlowLatencyMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.MinSetupLatency.Size()

	sz += mtr.MinSetupLatencyTimestamp.Size()

	sz += mtr.MaxSetupLatency.Size()

	sz += mtr.MaxSetupLatencyTimestamp.Size()

	sz += mtr.MinRttLatency.Size()

	sz += mtr.MinRttLatencyTimestamp.Size()

	sz += mtr.MaxRttLatency.Size()

	sz += mtr.MaxRttLatencyTimestamp.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *IPv4FlowLatencyMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.MinSetupLatency = mtr.metrics.GetCounter(offset)
	offset += mtr.MinSetupLatency.Size()

	mtr.MinSetupLatencyTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.MinSetupLatencyTimestamp.Size()

	mtr.MaxSetupLatency = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxSetupLatency.Size()

	mtr.MaxSetupLatencyTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.MaxSetupLatencyTimestamp.Size()

	mtr.MinRttLatency = mtr.metrics.GetCounter(offset)
	offset += mtr.MinRttLatency.Size()

	mtr.MinRttLatencyTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.MinRttLatencyTimestamp.Size()

	mtr.MaxRttLatency = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxRttLatency.Size()

	mtr.MaxRttLatencyTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.MaxRttLatencyTimestamp.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *IPv4FlowLatencyMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "MinSetupLatency" {
		return offset
	}
	offset += mtr.MinSetupLatency.Size()

	if fldName == "MinSetupLatencyTimestamp" {
		return offset
	}
	offset += mtr.MinSetupLatencyTimestamp.Size()

	if fldName == "MaxSetupLatency" {
		return offset
	}
	offset += mtr.MaxSetupLatency.Size()

	if fldName == "MaxSetupLatencyTimestamp" {
		return offset
	}
	offset += mtr.MaxSetupLatencyTimestamp.Size()

	if fldName == "MinRttLatency" {
		return offset
	}
	offset += mtr.MinRttLatency.Size()

	if fldName == "MinRttLatencyTimestamp" {
		return offset
	}
	offset += mtr.MinRttLatencyTimestamp.Size()

	if fldName == "MaxRttLatency" {
		return offset
	}
	offset += mtr.MaxRttLatency.Size()

	if fldName == "MaxRttLatencyTimestamp" {
		return offset
	}
	offset += mtr.MaxRttLatencyTimestamp.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *IPv4FlowLatencyMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetMinSetupLatency sets cunter in shared memory
func (mtr *IPv4FlowLatencyMetrics) SetMinSetupLatency(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MinSetupLatency"))
	return nil
}

// SetMinSetupLatencyTimestamp sets gauge in shared memory
func (mtr *IPv4FlowLatencyMetrics) SetMinSetupLatencyTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("MinSetupLatencyTimestamp"))
	return nil
}

// SetMaxSetupLatency sets cunter in shared memory
func (mtr *IPv4FlowLatencyMetrics) SetMaxSetupLatency(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxSetupLatency"))
	return nil
}

// SetMaxSetupLatencyTimestamp sets gauge in shared memory
func (mtr *IPv4FlowLatencyMetrics) SetMaxSetupLatencyTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("MaxSetupLatencyTimestamp"))
	return nil
}

// SetMinRttLatency sets cunter in shared memory
func (mtr *IPv4FlowLatencyMetrics) SetMinRttLatency(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MinRttLatency"))
	return nil
}

// SetMinRttLatencyTimestamp sets gauge in shared memory
func (mtr *IPv4FlowLatencyMetrics) SetMinRttLatencyTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("MinRttLatencyTimestamp"))
	return nil
}

// SetMaxRttLatency sets cunter in shared memory
func (mtr *IPv4FlowLatencyMetrics) SetMaxRttLatency(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxRttLatency"))
	return nil
}

// SetMaxRttLatencyTimestamp sets gauge in shared memory
func (mtr *IPv4FlowLatencyMetrics) SetMaxRttLatencyTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("MaxRttLatencyTimestamp"))
	return nil
}

// IPv4FlowLatencyMetricsIterator is the iterator object
type IPv4FlowLatencyMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IPv4FlowLatencyMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IPv4FlowLatencyMetricsIterator) Next() *IPv4FlowLatencyMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &IPv4FlowLatencyMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IPv4FlowLatencyMetricsIterator) Find(key IPv4FlowKey) (*IPv4FlowLatencyMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &IPv4FlowLatencyMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IPv4FlowLatencyMetricsIterator) Create(key IPv4FlowKey) (*IPv4FlowLatencyMetrics, error) {
	tmtr := &IPv4FlowLatencyMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &IPv4FlowLatencyMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IPv4FlowLatencyMetricsIterator) Delete(key IPv4FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *IPv4FlowLatencyMetricsIterator) Free() {
	it.iter.Free()
}

// NewIPv4FlowLatencyMetricsIterator returns an iterator
func NewIPv4FlowLatencyMetricsIterator() (*IPv4FlowLatencyMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IPv4FlowLatencyMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IPv4FlowLatencyMetricsIterator{iter: iter}, nil
}

type IPv6FlowLatencyMetrics struct {
	ObjectMeta api.ObjectMeta

	key IPv6FlowKey

	Instances metrics.Counter

	MinSetupLatency metrics.Counter

	MinSetupLatencyTimestamp metrics.Gauge

	MaxSetupLatency metrics.Counter

	MaxSetupLatencyTimestamp metrics.Gauge

	MinRttLatency metrics.Counter

	MinRttLatencyTimestamp metrics.Gauge

	MaxRttLatency metrics.Counter

	MaxRttLatencyTimestamp metrics.Gauge

	// private state
	metrics gometrics.Metrics
}

func (mtr *IPv6FlowLatencyMetrics) GetKey() IPv6FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IPv6FlowLatencyMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.MinSetupLatency.Size()

	sz += mtr.MinSetupLatencyTimestamp.Size()

	sz += mtr.MaxSetupLatency.Size()

	sz += mtr.MaxSetupLatencyTimestamp.Size()

	sz += mtr.MinRttLatency.Size()

	sz += mtr.MinRttLatencyTimestamp.Size()

	sz += mtr.MaxRttLatency.Size()

	sz += mtr.MaxRttLatencyTimestamp.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *IPv6FlowLatencyMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.MinSetupLatency = mtr.metrics.GetCounter(offset)
	offset += mtr.MinSetupLatency.Size()

	mtr.MinSetupLatencyTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.MinSetupLatencyTimestamp.Size()

	mtr.MaxSetupLatency = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxSetupLatency.Size()

	mtr.MaxSetupLatencyTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.MaxSetupLatencyTimestamp.Size()

	mtr.MinRttLatency = mtr.metrics.GetCounter(offset)
	offset += mtr.MinRttLatency.Size()

	mtr.MinRttLatencyTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.MinRttLatencyTimestamp.Size()

	mtr.MaxRttLatency = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxRttLatency.Size()

	mtr.MaxRttLatencyTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.MaxRttLatencyTimestamp.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *IPv6FlowLatencyMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "MinSetupLatency" {
		return offset
	}
	offset += mtr.MinSetupLatency.Size()

	if fldName == "MinSetupLatencyTimestamp" {
		return offset
	}
	offset += mtr.MinSetupLatencyTimestamp.Size()

	if fldName == "MaxSetupLatency" {
		return offset
	}
	offset += mtr.MaxSetupLatency.Size()

	if fldName == "MaxSetupLatencyTimestamp" {
		return offset
	}
	offset += mtr.MaxSetupLatencyTimestamp.Size()

	if fldName == "MinRttLatency" {
		return offset
	}
	offset += mtr.MinRttLatency.Size()

	if fldName == "MinRttLatencyTimestamp" {
		return offset
	}
	offset += mtr.MinRttLatencyTimestamp.Size()

	if fldName == "MaxRttLatency" {
		return offset
	}
	offset += mtr.MaxRttLatency.Size()

	if fldName == "MaxRttLatencyTimestamp" {
		return offset
	}
	offset += mtr.MaxRttLatencyTimestamp.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *IPv6FlowLatencyMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetMinSetupLatency sets cunter in shared memory
func (mtr *IPv6FlowLatencyMetrics) SetMinSetupLatency(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MinSetupLatency"))
	return nil
}

// SetMinSetupLatencyTimestamp sets gauge in shared memory
func (mtr *IPv6FlowLatencyMetrics) SetMinSetupLatencyTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("MinSetupLatencyTimestamp"))
	return nil
}

// SetMaxSetupLatency sets cunter in shared memory
func (mtr *IPv6FlowLatencyMetrics) SetMaxSetupLatency(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxSetupLatency"))
	return nil
}

// SetMaxSetupLatencyTimestamp sets gauge in shared memory
func (mtr *IPv6FlowLatencyMetrics) SetMaxSetupLatencyTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("MaxSetupLatencyTimestamp"))
	return nil
}

// SetMinRttLatency sets cunter in shared memory
func (mtr *IPv6FlowLatencyMetrics) SetMinRttLatency(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MinRttLatency"))
	return nil
}

// SetMinRttLatencyTimestamp sets gauge in shared memory
func (mtr *IPv6FlowLatencyMetrics) SetMinRttLatencyTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("MinRttLatencyTimestamp"))
	return nil
}

// SetMaxRttLatency sets cunter in shared memory
func (mtr *IPv6FlowLatencyMetrics) SetMaxRttLatency(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxRttLatency"))
	return nil
}

// SetMaxRttLatencyTimestamp sets gauge in shared memory
func (mtr *IPv6FlowLatencyMetrics) SetMaxRttLatencyTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("MaxRttLatencyTimestamp"))
	return nil
}

// IPv6FlowLatencyMetricsIterator is the iterator object
type IPv6FlowLatencyMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IPv6FlowLatencyMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IPv6FlowLatencyMetricsIterator) Next() *IPv6FlowLatencyMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &IPv6FlowLatencyMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IPv6FlowLatencyMetricsIterator) Find(key IPv6FlowKey) (*IPv6FlowLatencyMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &IPv6FlowLatencyMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IPv6FlowLatencyMetricsIterator) Create(key IPv6FlowKey) (*IPv6FlowLatencyMetrics, error) {
	tmtr := &IPv6FlowLatencyMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &IPv6FlowLatencyMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IPv6FlowLatencyMetricsIterator) Delete(key IPv6FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *IPv6FlowLatencyMetricsIterator) Free() {
	it.iter.Free()
}

// NewIPv6FlowLatencyMetricsIterator returns an iterator
func NewIPv6FlowLatencyMetricsIterator() (*IPv6FlowLatencyMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IPv6FlowLatencyMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IPv6FlowLatencyMetricsIterator{iter: iter}, nil
}

type L2FlowBehavioralMetrics struct {
	ObjectMeta api.ObjectMeta

	key L2FlowKey

	Instances metrics.Counter

	PpsThreshold metrics.Counter

	PpsThresholdExceedEvents metrics.Counter

	PpsThresholdExceedEventFirstTimestamp metrics.Gauge

	PpsThresholdExceedEventLastTimestamp metrics.Gauge

	BwThreshold metrics.Counter

	BwThresholdExceedEvents metrics.Counter

	BwThresholdExceedEventFirstTimestamp metrics.Gauge

	BwThresholdExceedEventLastTimestamp metrics.Gauge

	// private state
	metrics gometrics.Metrics
}

func (mtr *L2FlowBehavioralMetrics) GetKey() L2FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *L2FlowBehavioralMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.PpsThreshold.Size()

	sz += mtr.PpsThresholdExceedEvents.Size()

	sz += mtr.PpsThresholdExceedEventFirstTimestamp.Size()

	sz += mtr.PpsThresholdExceedEventLastTimestamp.Size()

	sz += mtr.BwThreshold.Size()

	sz += mtr.BwThresholdExceedEvents.Size()

	sz += mtr.BwThresholdExceedEventFirstTimestamp.Size()

	sz += mtr.BwThresholdExceedEventLastTimestamp.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *L2FlowBehavioralMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.PpsThreshold = mtr.metrics.GetCounter(offset)
	offset += mtr.PpsThreshold.Size()

	mtr.PpsThresholdExceedEvents = mtr.metrics.GetCounter(offset)
	offset += mtr.PpsThresholdExceedEvents.Size()

	mtr.PpsThresholdExceedEventFirstTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PpsThresholdExceedEventFirstTimestamp.Size()

	mtr.PpsThresholdExceedEventLastTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PpsThresholdExceedEventLastTimestamp.Size()

	mtr.BwThreshold = mtr.metrics.GetCounter(offset)
	offset += mtr.BwThreshold.Size()

	mtr.BwThresholdExceedEvents = mtr.metrics.GetCounter(offset)
	offset += mtr.BwThresholdExceedEvents.Size()

	mtr.BwThresholdExceedEventFirstTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.BwThresholdExceedEventFirstTimestamp.Size()

	mtr.BwThresholdExceedEventLastTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.BwThresholdExceedEventLastTimestamp.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *L2FlowBehavioralMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "PpsThreshold" {
		return offset
	}
	offset += mtr.PpsThreshold.Size()

	if fldName == "PpsThresholdExceedEvents" {
		return offset
	}
	offset += mtr.PpsThresholdExceedEvents.Size()

	if fldName == "PpsThresholdExceedEventFirstTimestamp" {
		return offset
	}
	offset += mtr.PpsThresholdExceedEventFirstTimestamp.Size()

	if fldName == "PpsThresholdExceedEventLastTimestamp" {
		return offset
	}
	offset += mtr.PpsThresholdExceedEventLastTimestamp.Size()

	if fldName == "BwThreshold" {
		return offset
	}
	offset += mtr.BwThreshold.Size()

	if fldName == "BwThresholdExceedEvents" {
		return offset
	}
	offset += mtr.BwThresholdExceedEvents.Size()

	if fldName == "BwThresholdExceedEventFirstTimestamp" {
		return offset
	}
	offset += mtr.BwThresholdExceedEventFirstTimestamp.Size()

	if fldName == "BwThresholdExceedEventLastTimestamp" {
		return offset
	}
	offset += mtr.BwThresholdExceedEventLastTimestamp.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *L2FlowBehavioralMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetPpsThreshold sets cunter in shared memory
func (mtr *L2FlowBehavioralMetrics) SetPpsThreshold(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PpsThreshold"))
	return nil
}

// SetPpsThresholdExceedEvents sets cunter in shared memory
func (mtr *L2FlowBehavioralMetrics) SetPpsThresholdExceedEvents(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PpsThresholdExceedEvents"))
	return nil
}

// SetPpsThresholdExceedEventFirstTimestamp sets gauge in shared memory
func (mtr *L2FlowBehavioralMetrics) SetPpsThresholdExceedEventFirstTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PpsThresholdExceedEventFirstTimestamp"))
	return nil
}

// SetPpsThresholdExceedEventLastTimestamp sets gauge in shared memory
func (mtr *L2FlowBehavioralMetrics) SetPpsThresholdExceedEventLastTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PpsThresholdExceedEventLastTimestamp"))
	return nil
}

// SetBwThreshold sets cunter in shared memory
func (mtr *L2FlowBehavioralMetrics) SetBwThreshold(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BwThreshold"))
	return nil
}

// SetBwThresholdExceedEvents sets cunter in shared memory
func (mtr *L2FlowBehavioralMetrics) SetBwThresholdExceedEvents(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BwThresholdExceedEvents"))
	return nil
}

// SetBwThresholdExceedEventFirstTimestamp sets gauge in shared memory
func (mtr *L2FlowBehavioralMetrics) SetBwThresholdExceedEventFirstTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("BwThresholdExceedEventFirstTimestamp"))
	return nil
}

// SetBwThresholdExceedEventLastTimestamp sets gauge in shared memory
func (mtr *L2FlowBehavioralMetrics) SetBwThresholdExceedEventLastTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("BwThresholdExceedEventLastTimestamp"))
	return nil
}

// L2FlowBehavioralMetricsIterator is the iterator object
type L2FlowBehavioralMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *L2FlowBehavioralMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *L2FlowBehavioralMetricsIterator) Next() *L2FlowBehavioralMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &L2FlowBehavioralMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *L2FlowBehavioralMetricsIterator) Find(key L2FlowKey) (*L2FlowBehavioralMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &L2FlowBehavioralMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *L2FlowBehavioralMetricsIterator) Create(key L2FlowKey) (*L2FlowBehavioralMetrics, error) {
	tmtr := &L2FlowBehavioralMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &L2FlowBehavioralMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *L2FlowBehavioralMetricsIterator) Delete(key L2FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *L2FlowBehavioralMetricsIterator) Free() {
	it.iter.Free()
}

// NewL2FlowBehavioralMetricsIterator returns an iterator
func NewL2FlowBehavioralMetricsIterator() (*L2FlowBehavioralMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("L2FlowBehavioralMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &L2FlowBehavioralMetricsIterator{iter: iter}, nil
}

type IPv4FlowBehavioralMetrics struct {
	ObjectMeta api.ObjectMeta

	key IPv4FlowKey

	Instances metrics.Counter

	PpsThreshold metrics.Counter

	PpsThresholdExceedEvents metrics.Counter

	PpsThresholdExceedEventFirstTimestamp metrics.Gauge

	PpsThresholdExceedEventLastTimestamp metrics.Gauge

	BwThreshold metrics.Counter

	BwThresholdExceedEvents metrics.Counter

	BwThresholdExceedEventFirstTimestamp metrics.Gauge

	BwThresholdExceedEventLastTimestamp metrics.Gauge

	// private state
	metrics gometrics.Metrics
}

func (mtr *IPv4FlowBehavioralMetrics) GetKey() IPv4FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IPv4FlowBehavioralMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.PpsThreshold.Size()

	sz += mtr.PpsThresholdExceedEvents.Size()

	sz += mtr.PpsThresholdExceedEventFirstTimestamp.Size()

	sz += mtr.PpsThresholdExceedEventLastTimestamp.Size()

	sz += mtr.BwThreshold.Size()

	sz += mtr.BwThresholdExceedEvents.Size()

	sz += mtr.BwThresholdExceedEventFirstTimestamp.Size()

	sz += mtr.BwThresholdExceedEventLastTimestamp.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *IPv4FlowBehavioralMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.PpsThreshold = mtr.metrics.GetCounter(offset)
	offset += mtr.PpsThreshold.Size()

	mtr.PpsThresholdExceedEvents = mtr.metrics.GetCounter(offset)
	offset += mtr.PpsThresholdExceedEvents.Size()

	mtr.PpsThresholdExceedEventFirstTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PpsThresholdExceedEventFirstTimestamp.Size()

	mtr.PpsThresholdExceedEventLastTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PpsThresholdExceedEventLastTimestamp.Size()

	mtr.BwThreshold = mtr.metrics.GetCounter(offset)
	offset += mtr.BwThreshold.Size()

	mtr.BwThresholdExceedEvents = mtr.metrics.GetCounter(offset)
	offset += mtr.BwThresholdExceedEvents.Size()

	mtr.BwThresholdExceedEventFirstTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.BwThresholdExceedEventFirstTimestamp.Size()

	mtr.BwThresholdExceedEventLastTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.BwThresholdExceedEventLastTimestamp.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *IPv4FlowBehavioralMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "PpsThreshold" {
		return offset
	}
	offset += mtr.PpsThreshold.Size()

	if fldName == "PpsThresholdExceedEvents" {
		return offset
	}
	offset += mtr.PpsThresholdExceedEvents.Size()

	if fldName == "PpsThresholdExceedEventFirstTimestamp" {
		return offset
	}
	offset += mtr.PpsThresholdExceedEventFirstTimestamp.Size()

	if fldName == "PpsThresholdExceedEventLastTimestamp" {
		return offset
	}
	offset += mtr.PpsThresholdExceedEventLastTimestamp.Size()

	if fldName == "BwThreshold" {
		return offset
	}
	offset += mtr.BwThreshold.Size()

	if fldName == "BwThresholdExceedEvents" {
		return offset
	}
	offset += mtr.BwThresholdExceedEvents.Size()

	if fldName == "BwThresholdExceedEventFirstTimestamp" {
		return offset
	}
	offset += mtr.BwThresholdExceedEventFirstTimestamp.Size()

	if fldName == "BwThresholdExceedEventLastTimestamp" {
		return offset
	}
	offset += mtr.BwThresholdExceedEventLastTimestamp.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *IPv4FlowBehavioralMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetPpsThreshold sets cunter in shared memory
func (mtr *IPv4FlowBehavioralMetrics) SetPpsThreshold(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PpsThreshold"))
	return nil
}

// SetPpsThresholdExceedEvents sets cunter in shared memory
func (mtr *IPv4FlowBehavioralMetrics) SetPpsThresholdExceedEvents(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PpsThresholdExceedEvents"))
	return nil
}

// SetPpsThresholdExceedEventFirstTimestamp sets gauge in shared memory
func (mtr *IPv4FlowBehavioralMetrics) SetPpsThresholdExceedEventFirstTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PpsThresholdExceedEventFirstTimestamp"))
	return nil
}

// SetPpsThresholdExceedEventLastTimestamp sets gauge in shared memory
func (mtr *IPv4FlowBehavioralMetrics) SetPpsThresholdExceedEventLastTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PpsThresholdExceedEventLastTimestamp"))
	return nil
}

// SetBwThreshold sets cunter in shared memory
func (mtr *IPv4FlowBehavioralMetrics) SetBwThreshold(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BwThreshold"))
	return nil
}

// SetBwThresholdExceedEvents sets cunter in shared memory
func (mtr *IPv4FlowBehavioralMetrics) SetBwThresholdExceedEvents(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BwThresholdExceedEvents"))
	return nil
}

// SetBwThresholdExceedEventFirstTimestamp sets gauge in shared memory
func (mtr *IPv4FlowBehavioralMetrics) SetBwThresholdExceedEventFirstTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("BwThresholdExceedEventFirstTimestamp"))
	return nil
}

// SetBwThresholdExceedEventLastTimestamp sets gauge in shared memory
func (mtr *IPv4FlowBehavioralMetrics) SetBwThresholdExceedEventLastTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("BwThresholdExceedEventLastTimestamp"))
	return nil
}

// IPv4FlowBehavioralMetricsIterator is the iterator object
type IPv4FlowBehavioralMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IPv4FlowBehavioralMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IPv4FlowBehavioralMetricsIterator) Next() *IPv4FlowBehavioralMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &IPv4FlowBehavioralMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IPv4FlowBehavioralMetricsIterator) Find(key IPv4FlowKey) (*IPv4FlowBehavioralMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &IPv4FlowBehavioralMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IPv4FlowBehavioralMetricsIterator) Create(key IPv4FlowKey) (*IPv4FlowBehavioralMetrics, error) {
	tmtr := &IPv4FlowBehavioralMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &IPv4FlowBehavioralMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IPv4FlowBehavioralMetricsIterator) Delete(key IPv4FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *IPv4FlowBehavioralMetricsIterator) Free() {
	it.iter.Free()
}

// NewIPv4FlowBehavioralMetricsIterator returns an iterator
func NewIPv4FlowBehavioralMetricsIterator() (*IPv4FlowBehavioralMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IPv4FlowBehavioralMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IPv4FlowBehavioralMetricsIterator{iter: iter}, nil
}

type IPv6FlowBehavioralMetrics struct {
	ObjectMeta api.ObjectMeta

	key IPv6FlowKey

	Instances metrics.Counter

	PpsThreshold metrics.Counter

	PpsThresholdExceedEvents metrics.Counter

	PpsThresholdExceedEventFirstTimestamp metrics.Gauge

	PpsThresholdExceedEventLastTimestamp metrics.Gauge

	BwThreshold metrics.Counter

	BwThresholdExceedEvents metrics.Counter

	BwThresholdExceedEventFirstTimestamp metrics.Gauge

	BwThresholdExceedEventLastTimestamp metrics.Gauge

	// private state
	metrics gometrics.Metrics
}

func (mtr *IPv6FlowBehavioralMetrics) GetKey() IPv6FlowKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IPv6FlowBehavioralMetrics) Size() int {
	sz := 0

	sz += mtr.Instances.Size()

	sz += mtr.PpsThreshold.Size()

	sz += mtr.PpsThresholdExceedEvents.Size()

	sz += mtr.PpsThresholdExceedEventFirstTimestamp.Size()

	sz += mtr.PpsThresholdExceedEventLastTimestamp.Size()

	sz += mtr.BwThreshold.Size()

	sz += mtr.BwThresholdExceedEvents.Size()

	sz += mtr.BwThresholdExceedEventFirstTimestamp.Size()

	sz += mtr.BwThresholdExceedEventLastTimestamp.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *IPv6FlowBehavioralMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.Instances = mtr.metrics.GetCounter(offset)
	offset += mtr.Instances.Size()

	mtr.PpsThreshold = mtr.metrics.GetCounter(offset)
	offset += mtr.PpsThreshold.Size()

	mtr.PpsThresholdExceedEvents = mtr.metrics.GetCounter(offset)
	offset += mtr.PpsThresholdExceedEvents.Size()

	mtr.PpsThresholdExceedEventFirstTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PpsThresholdExceedEventFirstTimestamp.Size()

	mtr.PpsThresholdExceedEventLastTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.PpsThresholdExceedEventLastTimestamp.Size()

	mtr.BwThreshold = mtr.metrics.GetCounter(offset)
	offset += mtr.BwThreshold.Size()

	mtr.BwThresholdExceedEvents = mtr.metrics.GetCounter(offset)
	offset += mtr.BwThresholdExceedEvents.Size()

	mtr.BwThresholdExceedEventFirstTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.BwThresholdExceedEventFirstTimestamp.Size()

	mtr.BwThresholdExceedEventLastTimestamp = mtr.metrics.GetGauge(offset)
	offset += mtr.BwThresholdExceedEventLastTimestamp.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *IPv6FlowBehavioralMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Instances" {
		return offset
	}
	offset += mtr.Instances.Size()

	if fldName == "PpsThreshold" {
		return offset
	}
	offset += mtr.PpsThreshold.Size()

	if fldName == "PpsThresholdExceedEvents" {
		return offset
	}
	offset += mtr.PpsThresholdExceedEvents.Size()

	if fldName == "PpsThresholdExceedEventFirstTimestamp" {
		return offset
	}
	offset += mtr.PpsThresholdExceedEventFirstTimestamp.Size()

	if fldName == "PpsThresholdExceedEventLastTimestamp" {
		return offset
	}
	offset += mtr.PpsThresholdExceedEventLastTimestamp.Size()

	if fldName == "BwThreshold" {
		return offset
	}
	offset += mtr.BwThreshold.Size()

	if fldName == "BwThresholdExceedEvents" {
		return offset
	}
	offset += mtr.BwThresholdExceedEvents.Size()

	if fldName == "BwThresholdExceedEventFirstTimestamp" {
		return offset
	}
	offset += mtr.BwThresholdExceedEventFirstTimestamp.Size()

	if fldName == "BwThresholdExceedEventLastTimestamp" {
		return offset
	}
	offset += mtr.BwThresholdExceedEventLastTimestamp.Size()

	return offset
}

// SetInstances sets cunter in shared memory
func (mtr *IPv6FlowBehavioralMetrics) SetInstances(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Instances"))
	return nil
}

// SetPpsThreshold sets cunter in shared memory
func (mtr *IPv6FlowBehavioralMetrics) SetPpsThreshold(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PpsThreshold"))
	return nil
}

// SetPpsThresholdExceedEvents sets cunter in shared memory
func (mtr *IPv6FlowBehavioralMetrics) SetPpsThresholdExceedEvents(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PpsThresholdExceedEvents"))
	return nil
}

// SetPpsThresholdExceedEventFirstTimestamp sets gauge in shared memory
func (mtr *IPv6FlowBehavioralMetrics) SetPpsThresholdExceedEventFirstTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PpsThresholdExceedEventFirstTimestamp"))
	return nil
}

// SetPpsThresholdExceedEventLastTimestamp sets gauge in shared memory
func (mtr *IPv6FlowBehavioralMetrics) SetPpsThresholdExceedEventLastTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("PpsThresholdExceedEventLastTimestamp"))
	return nil
}

// SetBwThreshold sets cunter in shared memory
func (mtr *IPv6FlowBehavioralMetrics) SetBwThreshold(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BwThreshold"))
	return nil
}

// SetBwThresholdExceedEvents sets cunter in shared memory
func (mtr *IPv6FlowBehavioralMetrics) SetBwThresholdExceedEvents(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BwThresholdExceedEvents"))
	return nil
}

// SetBwThresholdExceedEventFirstTimestamp sets gauge in shared memory
func (mtr *IPv6FlowBehavioralMetrics) SetBwThresholdExceedEventFirstTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("BwThresholdExceedEventFirstTimestamp"))
	return nil
}

// SetBwThresholdExceedEventLastTimestamp sets gauge in shared memory
func (mtr *IPv6FlowBehavioralMetrics) SetBwThresholdExceedEventLastTimestamp(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("BwThresholdExceedEventLastTimestamp"))
	return nil
}

// IPv6FlowBehavioralMetricsIterator is the iterator object
type IPv6FlowBehavioralMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IPv6FlowBehavioralMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IPv6FlowBehavioralMetricsIterator) Next() *IPv6FlowBehavioralMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &IPv6FlowBehavioralMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IPv6FlowBehavioralMetricsIterator) Find(key IPv6FlowKey) (*IPv6FlowBehavioralMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &IPv6FlowBehavioralMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IPv6FlowBehavioralMetricsIterator) Create(key IPv6FlowKey) (*IPv6FlowBehavioralMetrics, error) {
	tmtr := &IPv6FlowBehavioralMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &IPv6FlowBehavioralMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IPv6FlowBehavioralMetricsIterator) Delete(key IPv6FlowKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// Free frees the iterator memory
func (it *IPv6FlowBehavioralMetricsIterator) Free() {
	it.iter.Free()
}

// NewIPv6FlowBehavioralMetricsIterator returns an iterator
func NewIPv6FlowBehavioralMetricsIterator() (*IPv6FlowBehavioralMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IPv6FlowBehavioralMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IPv6FlowBehavioralMetricsIterator{iter: iter}, nil
}
