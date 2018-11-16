package asicerrord

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/ntranslate"
)

func init() {
	tstr := ntranslate.MustGetTranslator()

	tstr.Register("DppintcreditMetricsKey", &dppIntCreditFns{})
	tstr.Register("DppintfifoMetricsKey", &dppIntFifoFns{})
	tstr.Register("Dppintreg1MetricsKey", &dppIntReg1Fns{})
	tstr.Register("Dppintreg2MetricsKey", &dppIntReg2Fns{})
	tstr.Register("IntspareMetricsKey", &intSpareFns{})
	tstr.Register("DppintsramseccMetricsKey", &dppIntSramsEccFns{})
	tstr.Register("DprintcreditMetricsKey", &dprIntCreditFns{})
	tstr.Register("DprintfifoMetricsKey", &dprIntFifoFns{})
	tstr.Register("DprintflopfifoMetricsKey", &dprIntFlopFifoFns{})
	tstr.Register("Dprintreg1MetricsKey", &dprIntReg1Fns{})
	tstr.Register("Dprintreg2MetricsKey", &dprIntReg2Fns{})
	tstr.Register("DprintsramseccMetricsKey", &dprIntSramsEccFns{})
	tstr.Register("SsepicsintbadaddrMetricsKey", &ssePicsIntBadaddrFns{})
	tstr.Register("SsepicsintbgMetricsKey", &ssePicsIntBgFns{})
	tstr.Register("SsepicsintpicsMetricsKey", &ssePicsIntPicsFns{})
	tstr.Register("DbwaintdbMetricsKey", &dbWaIntDbFns{})
	tstr.Register("DbwaintlifqstatemapMetricsKey", &dbWaIntLifQstateMapFns{})
	tstr.Register("SgeteinterrMetricsKey", &sgeTeIntErrFns{})
	tstr.Register("SgeteintinfoMetricsKey", &sgeTeIntInfoFns{})
	tstr.Register("SgempuinterrMetricsKey", &sgeMpuIntErrFns{})
	tstr.Register("SgempuintinfoMetricsKey", &sgeMpuIntInfoFns{})
	tstr.Register("MdhensintaxierrMetricsKey", &mdHensIntAxiErrFns{})
	tstr.Register("MdhensinteccMetricsKey", &mdHensIntEccFns{})
	tstr.Register("MdhensintipcoreMetricsKey", &mdHensIntIpcoreFns{})
	tstr.Register("MpmpnsintcryptoMetricsKey", &mpMpnsIntCryptoFns{})
	tstr.Register("PbpbcintcreditunderflowMetricsKey", &pbPbcIntCreditUnderflowFns{})
	tstr.Register("InteccdescMetricsKey", &intEccDescFns{})
	tstr.Register("PbpbcintpbusviolationMetricsKey", &pbPbcIntPbusViolationFns{})
	tstr.Register("PbpbcintrplMetricsKey", &pbPbcIntRplFns{})
	tstr.Register("PbpbcintwriteMetricsKey", &pbPbcIntWriteFns{})
	tstr.Register("PbpbchbmintecchbmrbMetricsKey", &pbPbcHbmIntEccHbmRbFns{})
	tstr.Register("PbpbchbminthbmaxierrrspMetricsKey", &pbPbcHbmIntHbmAxiErrRspFns{})
	tstr.Register("PbpbchbminthbmdropMetricsKey", &pbPbcHbmIntHbmDropFns{})
	tstr.Register("PbpbchbminthbmpbusviolationMetricsKey", &pbPbcHbmIntHbmPbusViolationFns{})
	tstr.Register("PbpbchbminthbmxoffMetricsKey", &pbPbcHbmIntHbmXoffFns{})
	tstr.Register("McmchintmcMetricsKey", &mcMchIntMcFns{})

}

type dppIntCreditFns struct{}

// KeyToMeta converts key to meta
func (n *dppIntCreditFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dppIntCreditFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dppIntFifoFns struct{}

// KeyToMeta converts key to meta
func (n *dppIntFifoFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dppIntFifoFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dppIntReg1Fns struct{}

// KeyToMeta converts key to meta
func (n *dppIntReg1Fns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dppIntReg1Fns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dppIntReg2Fns struct{}

// KeyToMeta converts key to meta
func (n *dppIntReg2Fns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dppIntReg2Fns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type intSpareFns struct{}

// KeyToMeta converts key to meta
func (n *intSpareFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *intSpareFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dppIntSramsEccFns struct{}

// KeyToMeta converts key to meta
func (n *dppIntSramsEccFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dppIntSramsEccFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dprIntCreditFns struct{}

// KeyToMeta converts key to meta
func (n *dprIntCreditFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dprIntCreditFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dprIntFifoFns struct{}

// KeyToMeta converts key to meta
func (n *dprIntFifoFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dprIntFifoFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dprIntFlopFifoFns struct{}

// KeyToMeta converts key to meta
func (n *dprIntFlopFifoFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dprIntFlopFifoFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dprIntReg1Fns struct{}

// KeyToMeta converts key to meta
func (n *dprIntReg1Fns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dprIntReg1Fns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dprIntReg2Fns struct{}

// KeyToMeta converts key to meta
func (n *dprIntReg2Fns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dprIntReg2Fns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dprIntSramsEccFns struct{}

// KeyToMeta converts key to meta
func (n *dprIntSramsEccFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dprIntSramsEccFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type ssePicsIntBadaddrFns struct{}

// KeyToMeta converts key to meta
func (n *ssePicsIntBadaddrFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *ssePicsIntBadaddrFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type ssePicsIntBgFns struct{}

// KeyToMeta converts key to meta
func (n *ssePicsIntBgFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *ssePicsIntBgFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type ssePicsIntPicsFns struct{}

// KeyToMeta converts key to meta
func (n *ssePicsIntPicsFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *ssePicsIntPicsFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dbWaIntDbFns struct{}

// KeyToMeta converts key to meta
func (n *dbWaIntDbFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dbWaIntDbFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type dbWaIntLifQstateMapFns struct{}

// KeyToMeta converts key to meta
func (n *dbWaIntLifQstateMapFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *dbWaIntLifQstateMapFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type sgeTeIntErrFns struct{}

// KeyToMeta converts key to meta
func (n *sgeTeIntErrFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *sgeTeIntErrFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type sgeTeIntInfoFns struct{}

// KeyToMeta converts key to meta
func (n *sgeTeIntInfoFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *sgeTeIntInfoFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type sgeMpuIntErrFns struct{}

// KeyToMeta converts key to meta
func (n *sgeMpuIntErrFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *sgeMpuIntErrFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type sgeMpuIntInfoFns struct{}

// KeyToMeta converts key to meta
func (n *sgeMpuIntInfoFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *sgeMpuIntInfoFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type mdHensIntAxiErrFns struct{}

// KeyToMeta converts key to meta
func (n *mdHensIntAxiErrFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *mdHensIntAxiErrFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type mdHensIntEccFns struct{}

// KeyToMeta converts key to meta
func (n *mdHensIntEccFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *mdHensIntEccFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type mdHensIntIpcoreFns struct{}

// KeyToMeta converts key to meta
func (n *mdHensIntIpcoreFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *mdHensIntIpcoreFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type mpMpnsIntCryptoFns struct{}

// KeyToMeta converts key to meta
func (n *mpMpnsIntCryptoFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *mpMpnsIntCryptoFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type pbPbcIntCreditUnderflowFns struct{}

// KeyToMeta converts key to meta
func (n *pbPbcIntCreditUnderflowFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *pbPbcIntCreditUnderflowFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type intEccDescFns struct{}

// KeyToMeta converts key to meta
func (n *intEccDescFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *intEccDescFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type pbPbcIntPbusViolationFns struct{}

// KeyToMeta converts key to meta
func (n *pbPbcIntPbusViolationFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *pbPbcIntPbusViolationFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type pbPbcIntRplFns struct{}

// KeyToMeta converts key to meta
func (n *pbPbcIntRplFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *pbPbcIntRplFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type pbPbcIntWriteFns struct{}

// KeyToMeta converts key to meta
func (n *pbPbcIntWriteFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *pbPbcIntWriteFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type pbPbcHbmIntEccHbmRbFns struct{}

// KeyToMeta converts key to meta
func (n *pbPbcHbmIntEccHbmRbFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *pbPbcHbmIntEccHbmRbFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type pbPbcHbmIntHbmAxiErrRspFns struct{}

// KeyToMeta converts key to meta
func (n *pbPbcHbmIntHbmAxiErrRspFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *pbPbcHbmIntHbmAxiErrRspFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type pbPbcHbmIntHbmDropFns struct{}

// KeyToMeta converts key to meta
func (n *pbPbcHbmIntHbmDropFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *pbPbcHbmIntHbmDropFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type pbPbcHbmIntHbmPbusViolationFns struct{}

// KeyToMeta converts key to meta
func (n *pbPbcHbmIntHbmPbusViolationFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *pbPbcHbmIntHbmPbusViolationFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type pbPbcHbmIntHbmXoffFns struct{}

// KeyToMeta converts key to meta
func (n *pbPbcHbmIntHbmXoffFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *pbPbcHbmIntHbmXoffFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type mcMchIntMcFns struct{}

// KeyToMeta converts key to meta
func (n *mcMchIntMcFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *mcMchIntMcFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}
