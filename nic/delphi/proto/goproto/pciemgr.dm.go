// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

type PciePortMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint32

	IntrTotal metrics.Counter

	IntrPolled metrics.Counter

	IntrPerstn metrics.Counter

	IntrLtssmstEarly metrics.Counter

	IntrLtssmst metrics.Counter

	IntrLinkup2Dn metrics.Counter

	IntrLinkdn2Up metrics.Counter

	IntrRstup2Dn metrics.Counter

	IntrRstdn2Up metrics.Counter

	IntrSecbus metrics.Counter

	Linkup metrics.Counter

	Hostup metrics.Counter

	Phypolllast metrics.Counter

	Phypollmax metrics.Counter

	Phypollperstn metrics.Counter

	Phypollfail metrics.Counter

	Gatepolllast metrics.Counter

	Gatepollmax metrics.Counter

	Markerpolllast metrics.Counter

	Markerpollmax metrics.Counter

	Axipendpolllast metrics.Counter

	Axipendpollmax metrics.Counter

	Faults metrics.Counter

	Powerdown metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PciePortMetrics) GetKey() uint32 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PciePortMetrics) Size() int {
	sz := 0

	sz += mtr.IntrTotal.Size()

	sz += mtr.IntrPolled.Size()

	sz += mtr.IntrPerstn.Size()

	sz += mtr.IntrLtssmstEarly.Size()

	sz += mtr.IntrLtssmst.Size()

	sz += mtr.IntrLinkup2Dn.Size()

	sz += mtr.IntrLinkdn2Up.Size()

	sz += mtr.IntrRstup2Dn.Size()

	sz += mtr.IntrRstdn2Up.Size()

	sz += mtr.IntrSecbus.Size()

	sz += mtr.Linkup.Size()

	sz += mtr.Hostup.Size()

	sz += mtr.Phypolllast.Size()

	sz += mtr.Phypollmax.Size()

	sz += mtr.Phypollperstn.Size()

	sz += mtr.Phypollfail.Size()

	sz += mtr.Gatepolllast.Size()

	sz += mtr.Gatepollmax.Size()

	sz += mtr.Markerpolllast.Size()

	sz += mtr.Markerpollmax.Size()

	sz += mtr.Axipendpolllast.Size()

	sz += mtr.Axipendpollmax.Size()

	sz += mtr.Faults.Size()

	sz += mtr.Powerdown.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PciePortMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.IntrTotal = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrTotal.Size()

	mtr.IntrPolled = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrPolled.Size()

	mtr.IntrPerstn = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrPerstn.Size()

	mtr.IntrLtssmstEarly = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrLtssmstEarly.Size()

	mtr.IntrLtssmst = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrLtssmst.Size()

	mtr.IntrLinkup2Dn = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrLinkup2Dn.Size()

	mtr.IntrLinkdn2Up = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrLinkdn2Up.Size()

	mtr.IntrRstup2Dn = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrRstup2Dn.Size()

	mtr.IntrRstdn2Up = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrRstdn2Up.Size()

	mtr.IntrSecbus = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrSecbus.Size()

	mtr.Linkup = mtr.metrics.GetCounter(offset)
	offset += mtr.Linkup.Size()

	mtr.Hostup = mtr.metrics.GetCounter(offset)
	offset += mtr.Hostup.Size()

	mtr.Phypolllast = mtr.metrics.GetCounter(offset)
	offset += mtr.Phypolllast.Size()

	mtr.Phypollmax = mtr.metrics.GetCounter(offset)
	offset += mtr.Phypollmax.Size()

	mtr.Phypollperstn = mtr.metrics.GetCounter(offset)
	offset += mtr.Phypollperstn.Size()

	mtr.Phypollfail = mtr.metrics.GetCounter(offset)
	offset += mtr.Phypollfail.Size()

	mtr.Gatepolllast = mtr.metrics.GetCounter(offset)
	offset += mtr.Gatepolllast.Size()

	mtr.Gatepollmax = mtr.metrics.GetCounter(offset)
	offset += mtr.Gatepollmax.Size()

	mtr.Markerpolllast = mtr.metrics.GetCounter(offset)
	offset += mtr.Markerpolllast.Size()

	mtr.Markerpollmax = mtr.metrics.GetCounter(offset)
	offset += mtr.Markerpollmax.Size()

	mtr.Axipendpolllast = mtr.metrics.GetCounter(offset)
	offset += mtr.Axipendpolllast.Size()

	mtr.Axipendpollmax = mtr.metrics.GetCounter(offset)
	offset += mtr.Axipendpollmax.Size()

	mtr.Faults = mtr.metrics.GetCounter(offset)
	offset += mtr.Faults.Size()

	mtr.Powerdown = mtr.metrics.GetCounter(offset)
	offset += mtr.Powerdown.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PciePortMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "IntrTotal" {
		return offset
	}
	offset += mtr.IntrTotal.Size()

	if fldName == "IntrPolled" {
		return offset
	}
	offset += mtr.IntrPolled.Size()

	if fldName == "IntrPerstn" {
		return offset
	}
	offset += mtr.IntrPerstn.Size()

	if fldName == "IntrLtssmstEarly" {
		return offset
	}
	offset += mtr.IntrLtssmstEarly.Size()

	if fldName == "IntrLtssmst" {
		return offset
	}
	offset += mtr.IntrLtssmst.Size()

	if fldName == "IntrLinkup2Dn" {
		return offset
	}
	offset += mtr.IntrLinkup2Dn.Size()

	if fldName == "IntrLinkdn2Up" {
		return offset
	}
	offset += mtr.IntrLinkdn2Up.Size()

	if fldName == "IntrRstup2Dn" {
		return offset
	}
	offset += mtr.IntrRstup2Dn.Size()

	if fldName == "IntrRstdn2Up" {
		return offset
	}
	offset += mtr.IntrRstdn2Up.Size()

	if fldName == "IntrSecbus" {
		return offset
	}
	offset += mtr.IntrSecbus.Size()

	if fldName == "Linkup" {
		return offset
	}
	offset += mtr.Linkup.Size()

	if fldName == "Hostup" {
		return offset
	}
	offset += mtr.Hostup.Size()

	if fldName == "Phypolllast" {
		return offset
	}
	offset += mtr.Phypolllast.Size()

	if fldName == "Phypollmax" {
		return offset
	}
	offset += mtr.Phypollmax.Size()

	if fldName == "Phypollperstn" {
		return offset
	}
	offset += mtr.Phypollperstn.Size()

	if fldName == "Phypollfail" {
		return offset
	}
	offset += mtr.Phypollfail.Size()

	if fldName == "Gatepolllast" {
		return offset
	}
	offset += mtr.Gatepolllast.Size()

	if fldName == "Gatepollmax" {
		return offset
	}
	offset += mtr.Gatepollmax.Size()

	if fldName == "Markerpolllast" {
		return offset
	}
	offset += mtr.Markerpolllast.Size()

	if fldName == "Markerpollmax" {
		return offset
	}
	offset += mtr.Markerpollmax.Size()

	if fldName == "Axipendpolllast" {
		return offset
	}
	offset += mtr.Axipendpolllast.Size()

	if fldName == "Axipendpollmax" {
		return offset
	}
	offset += mtr.Axipendpollmax.Size()

	if fldName == "Faults" {
		return offset
	}
	offset += mtr.Faults.Size()

	if fldName == "Powerdown" {
		return offset
	}
	offset += mtr.Powerdown.Size()

	return offset
}

// SetIntrTotal sets cunter in shared memory
func (mtr *PciePortMetrics) SetIntrTotal(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrTotal"))
	return nil
}

// SetIntrPolled sets cunter in shared memory
func (mtr *PciePortMetrics) SetIntrPolled(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrPolled"))
	return nil
}

// SetIntrPerstn sets cunter in shared memory
func (mtr *PciePortMetrics) SetIntrPerstn(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrPerstn"))
	return nil
}

// SetIntrLtssmstEarly sets cunter in shared memory
func (mtr *PciePortMetrics) SetIntrLtssmstEarly(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrLtssmstEarly"))
	return nil
}

// SetIntrLtssmst sets cunter in shared memory
func (mtr *PciePortMetrics) SetIntrLtssmst(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrLtssmst"))
	return nil
}

// SetIntrLinkup2Dn sets cunter in shared memory
func (mtr *PciePortMetrics) SetIntrLinkup2Dn(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrLinkup2Dn"))
	return nil
}

// SetIntrLinkdn2Up sets cunter in shared memory
func (mtr *PciePortMetrics) SetIntrLinkdn2Up(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrLinkdn2Up"))
	return nil
}

// SetIntrRstup2Dn sets cunter in shared memory
func (mtr *PciePortMetrics) SetIntrRstup2Dn(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrRstup2Dn"))
	return nil
}

// SetIntrRstdn2Up sets cunter in shared memory
func (mtr *PciePortMetrics) SetIntrRstdn2Up(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrRstdn2Up"))
	return nil
}

// SetIntrSecbus sets cunter in shared memory
func (mtr *PciePortMetrics) SetIntrSecbus(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrSecbus"))
	return nil
}

// SetLinkup sets cunter in shared memory
func (mtr *PciePortMetrics) SetLinkup(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Linkup"))
	return nil
}

// SetHostup sets cunter in shared memory
func (mtr *PciePortMetrics) SetHostup(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Hostup"))
	return nil
}

// SetPhypolllast sets cunter in shared memory
func (mtr *PciePortMetrics) SetPhypolllast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phypolllast"))
	return nil
}

// SetPhypollmax sets cunter in shared memory
func (mtr *PciePortMetrics) SetPhypollmax(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phypollmax"))
	return nil
}

// SetPhypollperstn sets cunter in shared memory
func (mtr *PciePortMetrics) SetPhypollperstn(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phypollperstn"))
	return nil
}

// SetPhypollfail sets cunter in shared memory
func (mtr *PciePortMetrics) SetPhypollfail(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phypollfail"))
	return nil
}

// SetGatepolllast sets cunter in shared memory
func (mtr *PciePortMetrics) SetGatepolllast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Gatepolllast"))
	return nil
}

// SetGatepollmax sets cunter in shared memory
func (mtr *PciePortMetrics) SetGatepollmax(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Gatepollmax"))
	return nil
}

// SetMarkerpolllast sets cunter in shared memory
func (mtr *PciePortMetrics) SetMarkerpolllast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Markerpolllast"))
	return nil
}

// SetMarkerpollmax sets cunter in shared memory
func (mtr *PciePortMetrics) SetMarkerpollmax(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Markerpollmax"))
	return nil
}

// SetAxipendpolllast sets cunter in shared memory
func (mtr *PciePortMetrics) SetAxipendpolllast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Axipendpolllast"))
	return nil
}

// SetAxipendpollmax sets cunter in shared memory
func (mtr *PciePortMetrics) SetAxipendpollmax(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Axipendpollmax"))
	return nil
}

// SetFaults sets cunter in shared memory
func (mtr *PciePortMetrics) SetFaults(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Faults"))
	return nil
}

// SetPowerdown sets cunter in shared memory
func (mtr *PciePortMetrics) SetPowerdown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Powerdown"))
	return nil
}

// PciePortMetricsIterator is the iterator object
type PciePortMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PciePortMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PciePortMetricsIterator) Next() *PciePortMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PciePortMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PciePortMetricsIterator) Find(key uint32) (*PciePortMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PciePortMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PciePortMetricsIterator) Create(key uint32) (*PciePortMetrics, error) {
	tmtr := &PciePortMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PciePortMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PciePortMetricsIterator) Delete(key uint32) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PciePortMetricsIterator) Free() {
	it.iter.Free()
}

// NewPciePortMetricsIterator returns an iterator
func NewPciePortMetricsIterator() (*PciePortMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PciePortMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PciePortMetricsIterator{iter: iter}, nil
}

type PcieMgrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint32

	NotIntr metrics.Counter

	NotSpurious metrics.Counter

	NotCnt metrics.Counter

	NotMax metrics.Counter

	NotCfgrd metrics.Counter

	NotCfgwr metrics.Counter

	NotMemrd metrics.Counter

	NotMemwr metrics.Counter

	NotIord metrics.Counter

	NotIowr metrics.Counter

	NotUnknown metrics.Counter

	NotRsrv0 metrics.Counter

	NotRsrv1 metrics.Counter

	NotMsg metrics.Counter

	NotUnsupported metrics.Counter

	NotPmv metrics.Counter

	NotDbpmv metrics.Counter

	NotAtomic metrics.Counter

	NotPmtmiss metrics.Counter

	NotPmrmiss metrics.Counter

	NotPrtmiss metrics.Counter

	NotBdf2Vfidmiss metrics.Counter

	NotPrtoor metrics.Counter

	NotVfidoor metrics.Counter

	NotBdfoor metrics.Counter

	NotPmrind metrics.Counter

	NotPrtind metrics.Counter

	NotPmrecc metrics.Counter

	NotPrtecc metrics.Counter

	IndIntr metrics.Counter

	IndSpurious metrics.Counter

	IndCfgrd metrics.Counter

	IndCfgwr metrics.Counter

	IndMemrd metrics.Counter

	IndMemwr metrics.Counter

	IndIord metrics.Counter

	IndIowr metrics.Counter

	IndUnknown metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PcieMgrMetrics) GetKey() uint32 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PcieMgrMetrics) Size() int {
	sz := 0

	sz += mtr.NotIntr.Size()

	sz += mtr.NotSpurious.Size()

	sz += mtr.NotCnt.Size()

	sz += mtr.NotMax.Size()

	sz += mtr.NotCfgrd.Size()

	sz += mtr.NotCfgwr.Size()

	sz += mtr.NotMemrd.Size()

	sz += mtr.NotMemwr.Size()

	sz += mtr.NotIord.Size()

	sz += mtr.NotIowr.Size()

	sz += mtr.NotUnknown.Size()

	sz += mtr.NotRsrv0.Size()

	sz += mtr.NotRsrv1.Size()

	sz += mtr.NotMsg.Size()

	sz += mtr.NotUnsupported.Size()

	sz += mtr.NotPmv.Size()

	sz += mtr.NotDbpmv.Size()

	sz += mtr.NotAtomic.Size()

	sz += mtr.NotPmtmiss.Size()

	sz += mtr.NotPmrmiss.Size()

	sz += mtr.NotPrtmiss.Size()

	sz += mtr.NotBdf2Vfidmiss.Size()

	sz += mtr.NotPrtoor.Size()

	sz += mtr.NotVfidoor.Size()

	sz += mtr.NotBdfoor.Size()

	sz += mtr.NotPmrind.Size()

	sz += mtr.NotPrtind.Size()

	sz += mtr.NotPmrecc.Size()

	sz += mtr.NotPrtecc.Size()

	sz += mtr.IndIntr.Size()

	sz += mtr.IndSpurious.Size()

	sz += mtr.IndCfgrd.Size()

	sz += mtr.IndCfgwr.Size()

	sz += mtr.IndMemrd.Size()

	sz += mtr.IndMemwr.Size()

	sz += mtr.IndIord.Size()

	sz += mtr.IndIowr.Size()

	sz += mtr.IndUnknown.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PcieMgrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.NotIntr = mtr.metrics.GetCounter(offset)
	offset += mtr.NotIntr.Size()

	mtr.NotSpurious = mtr.metrics.GetCounter(offset)
	offset += mtr.NotSpurious.Size()

	mtr.NotCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.NotCnt.Size()

	mtr.NotMax = mtr.metrics.GetCounter(offset)
	offset += mtr.NotMax.Size()

	mtr.NotCfgrd = mtr.metrics.GetCounter(offset)
	offset += mtr.NotCfgrd.Size()

	mtr.NotCfgwr = mtr.metrics.GetCounter(offset)
	offset += mtr.NotCfgwr.Size()

	mtr.NotMemrd = mtr.metrics.GetCounter(offset)
	offset += mtr.NotMemrd.Size()

	mtr.NotMemwr = mtr.metrics.GetCounter(offset)
	offset += mtr.NotMemwr.Size()

	mtr.NotIord = mtr.metrics.GetCounter(offset)
	offset += mtr.NotIord.Size()

	mtr.NotIowr = mtr.metrics.GetCounter(offset)
	offset += mtr.NotIowr.Size()

	mtr.NotUnknown = mtr.metrics.GetCounter(offset)
	offset += mtr.NotUnknown.Size()

	mtr.NotRsrv0 = mtr.metrics.GetCounter(offset)
	offset += mtr.NotRsrv0.Size()

	mtr.NotRsrv1 = mtr.metrics.GetCounter(offset)
	offset += mtr.NotRsrv1.Size()

	mtr.NotMsg = mtr.metrics.GetCounter(offset)
	offset += mtr.NotMsg.Size()

	mtr.NotUnsupported = mtr.metrics.GetCounter(offset)
	offset += mtr.NotUnsupported.Size()

	mtr.NotPmv = mtr.metrics.GetCounter(offset)
	offset += mtr.NotPmv.Size()

	mtr.NotDbpmv = mtr.metrics.GetCounter(offset)
	offset += mtr.NotDbpmv.Size()

	mtr.NotAtomic = mtr.metrics.GetCounter(offset)
	offset += mtr.NotAtomic.Size()

	mtr.NotPmtmiss = mtr.metrics.GetCounter(offset)
	offset += mtr.NotPmtmiss.Size()

	mtr.NotPmrmiss = mtr.metrics.GetCounter(offset)
	offset += mtr.NotPmrmiss.Size()

	mtr.NotPrtmiss = mtr.metrics.GetCounter(offset)
	offset += mtr.NotPrtmiss.Size()

	mtr.NotBdf2Vfidmiss = mtr.metrics.GetCounter(offset)
	offset += mtr.NotBdf2Vfidmiss.Size()

	mtr.NotPrtoor = mtr.metrics.GetCounter(offset)
	offset += mtr.NotPrtoor.Size()

	mtr.NotVfidoor = mtr.metrics.GetCounter(offset)
	offset += mtr.NotVfidoor.Size()

	mtr.NotBdfoor = mtr.metrics.GetCounter(offset)
	offset += mtr.NotBdfoor.Size()

	mtr.NotPmrind = mtr.metrics.GetCounter(offset)
	offset += mtr.NotPmrind.Size()

	mtr.NotPrtind = mtr.metrics.GetCounter(offset)
	offset += mtr.NotPrtind.Size()

	mtr.NotPmrecc = mtr.metrics.GetCounter(offset)
	offset += mtr.NotPmrecc.Size()

	mtr.NotPrtecc = mtr.metrics.GetCounter(offset)
	offset += mtr.NotPrtecc.Size()

	mtr.IndIntr = mtr.metrics.GetCounter(offset)
	offset += mtr.IndIntr.Size()

	mtr.IndSpurious = mtr.metrics.GetCounter(offset)
	offset += mtr.IndSpurious.Size()

	mtr.IndCfgrd = mtr.metrics.GetCounter(offset)
	offset += mtr.IndCfgrd.Size()

	mtr.IndCfgwr = mtr.metrics.GetCounter(offset)
	offset += mtr.IndCfgwr.Size()

	mtr.IndMemrd = mtr.metrics.GetCounter(offset)
	offset += mtr.IndMemrd.Size()

	mtr.IndMemwr = mtr.metrics.GetCounter(offset)
	offset += mtr.IndMemwr.Size()

	mtr.IndIord = mtr.metrics.GetCounter(offset)
	offset += mtr.IndIord.Size()

	mtr.IndIowr = mtr.metrics.GetCounter(offset)
	offset += mtr.IndIowr.Size()

	mtr.IndUnknown = mtr.metrics.GetCounter(offset)
	offset += mtr.IndUnknown.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PcieMgrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "NotIntr" {
		return offset
	}
	offset += mtr.NotIntr.Size()

	if fldName == "NotSpurious" {
		return offset
	}
	offset += mtr.NotSpurious.Size()

	if fldName == "NotCnt" {
		return offset
	}
	offset += mtr.NotCnt.Size()

	if fldName == "NotMax" {
		return offset
	}
	offset += mtr.NotMax.Size()

	if fldName == "NotCfgrd" {
		return offset
	}
	offset += mtr.NotCfgrd.Size()

	if fldName == "NotCfgwr" {
		return offset
	}
	offset += mtr.NotCfgwr.Size()

	if fldName == "NotMemrd" {
		return offset
	}
	offset += mtr.NotMemrd.Size()

	if fldName == "NotMemwr" {
		return offset
	}
	offset += mtr.NotMemwr.Size()

	if fldName == "NotIord" {
		return offset
	}
	offset += mtr.NotIord.Size()

	if fldName == "NotIowr" {
		return offset
	}
	offset += mtr.NotIowr.Size()

	if fldName == "NotUnknown" {
		return offset
	}
	offset += mtr.NotUnknown.Size()

	if fldName == "NotRsrv0" {
		return offset
	}
	offset += mtr.NotRsrv0.Size()

	if fldName == "NotRsrv1" {
		return offset
	}
	offset += mtr.NotRsrv1.Size()

	if fldName == "NotMsg" {
		return offset
	}
	offset += mtr.NotMsg.Size()

	if fldName == "NotUnsupported" {
		return offset
	}
	offset += mtr.NotUnsupported.Size()

	if fldName == "NotPmv" {
		return offset
	}
	offset += mtr.NotPmv.Size()

	if fldName == "NotDbpmv" {
		return offset
	}
	offset += mtr.NotDbpmv.Size()

	if fldName == "NotAtomic" {
		return offset
	}
	offset += mtr.NotAtomic.Size()

	if fldName == "NotPmtmiss" {
		return offset
	}
	offset += mtr.NotPmtmiss.Size()

	if fldName == "NotPmrmiss" {
		return offset
	}
	offset += mtr.NotPmrmiss.Size()

	if fldName == "NotPrtmiss" {
		return offset
	}
	offset += mtr.NotPrtmiss.Size()

	if fldName == "NotBdf2Vfidmiss" {
		return offset
	}
	offset += mtr.NotBdf2Vfidmiss.Size()

	if fldName == "NotPrtoor" {
		return offset
	}
	offset += mtr.NotPrtoor.Size()

	if fldName == "NotVfidoor" {
		return offset
	}
	offset += mtr.NotVfidoor.Size()

	if fldName == "NotBdfoor" {
		return offset
	}
	offset += mtr.NotBdfoor.Size()

	if fldName == "NotPmrind" {
		return offset
	}
	offset += mtr.NotPmrind.Size()

	if fldName == "NotPrtind" {
		return offset
	}
	offset += mtr.NotPrtind.Size()

	if fldName == "NotPmrecc" {
		return offset
	}
	offset += mtr.NotPmrecc.Size()

	if fldName == "NotPrtecc" {
		return offset
	}
	offset += mtr.NotPrtecc.Size()

	if fldName == "IndIntr" {
		return offset
	}
	offset += mtr.IndIntr.Size()

	if fldName == "IndSpurious" {
		return offset
	}
	offset += mtr.IndSpurious.Size()

	if fldName == "IndCfgrd" {
		return offset
	}
	offset += mtr.IndCfgrd.Size()

	if fldName == "IndCfgwr" {
		return offset
	}
	offset += mtr.IndCfgwr.Size()

	if fldName == "IndMemrd" {
		return offset
	}
	offset += mtr.IndMemrd.Size()

	if fldName == "IndMemwr" {
		return offset
	}
	offset += mtr.IndMemwr.Size()

	if fldName == "IndIord" {
		return offset
	}
	offset += mtr.IndIord.Size()

	if fldName == "IndIowr" {
		return offset
	}
	offset += mtr.IndIowr.Size()

	if fldName == "IndUnknown" {
		return offset
	}
	offset += mtr.IndUnknown.Size()

	return offset
}

// SetNotIntr sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotIntr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotIntr"))
	return nil
}

// SetNotSpurious sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotSpurious(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotSpurious"))
	return nil
}

// SetNotCnt sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotCnt"))
	return nil
}

// SetNotMax sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotMax(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotMax"))
	return nil
}

// SetNotCfgrd sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotCfgrd(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotCfgrd"))
	return nil
}

// SetNotCfgwr sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotCfgwr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotCfgwr"))
	return nil
}

// SetNotMemrd sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotMemrd(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotMemrd"))
	return nil
}

// SetNotMemwr sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotMemwr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotMemwr"))
	return nil
}

// SetNotIord sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotIord(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotIord"))
	return nil
}

// SetNotIowr sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotIowr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotIowr"))
	return nil
}

// SetNotUnknown sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotUnknown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotUnknown"))
	return nil
}

// SetNotRsrv0 sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotRsrv0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotRsrv0"))
	return nil
}

// SetNotRsrv1 sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotRsrv1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotRsrv1"))
	return nil
}

// SetNotMsg sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotMsg(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotMsg"))
	return nil
}

// SetNotUnsupported sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotUnsupported(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotUnsupported"))
	return nil
}

// SetNotPmv sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotPmv(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotPmv"))
	return nil
}

// SetNotDbpmv sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotDbpmv(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotDbpmv"))
	return nil
}

// SetNotAtomic sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotAtomic(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotAtomic"))
	return nil
}

// SetNotPmtmiss sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotPmtmiss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotPmtmiss"))
	return nil
}

// SetNotPmrmiss sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotPmrmiss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotPmrmiss"))
	return nil
}

// SetNotPrtmiss sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotPrtmiss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotPrtmiss"))
	return nil
}

// SetNotBdf2Vfidmiss sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotBdf2Vfidmiss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotBdf2Vfidmiss"))
	return nil
}

// SetNotPrtoor sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotPrtoor(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotPrtoor"))
	return nil
}

// SetNotVfidoor sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotVfidoor(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotVfidoor"))
	return nil
}

// SetNotBdfoor sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotBdfoor(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotBdfoor"))
	return nil
}

// SetNotPmrind sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotPmrind(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotPmrind"))
	return nil
}

// SetNotPrtind sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotPrtind(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotPrtind"))
	return nil
}

// SetNotPmrecc sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotPmrecc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotPmrecc"))
	return nil
}

// SetNotPrtecc sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetNotPrtecc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NotPrtecc"))
	return nil
}

// SetIndIntr sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetIndIntr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IndIntr"))
	return nil
}

// SetIndSpurious sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetIndSpurious(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IndSpurious"))
	return nil
}

// SetIndCfgrd sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetIndCfgrd(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IndCfgrd"))
	return nil
}

// SetIndCfgwr sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetIndCfgwr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IndCfgwr"))
	return nil
}

// SetIndMemrd sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetIndMemrd(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IndMemrd"))
	return nil
}

// SetIndMemwr sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetIndMemwr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IndMemwr"))
	return nil
}

// SetIndIord sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetIndIord(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IndIord"))
	return nil
}

// SetIndIowr sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetIndIowr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IndIowr"))
	return nil
}

// SetIndUnknown sets cunter in shared memory
func (mtr *PcieMgrMetrics) SetIndUnknown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IndUnknown"))
	return nil
}

// PcieMgrMetricsIterator is the iterator object
type PcieMgrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PcieMgrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PcieMgrMetricsIterator) Next() *PcieMgrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PcieMgrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PcieMgrMetricsIterator) Find(key uint32) (*PcieMgrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PcieMgrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PcieMgrMetricsIterator) Create(key uint32) (*PcieMgrMetrics, error) {
	tmtr := &PcieMgrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PcieMgrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PcieMgrMetricsIterator) Delete(key uint32) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PcieMgrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcieMgrMetricsIterator returns an iterator
func NewPcieMgrMetricsIterator() (*PcieMgrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PcieMgrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PcieMgrMetricsIterator{iter: iter}, nil
}
