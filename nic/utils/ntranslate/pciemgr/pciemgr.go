package pciemgr

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/ntranslate"
)

func init() {
	tstr := ntranslate.MustGetTranslator()

	tstr.Register("PcieMgrMetricsKey", &pcieMgrFns{})
	tstr.Register("PciePortMetricsKey", &pciePortFns{})
}

type pcieMgrFns struct{}

// KeyToMeta converts network key to meta
func (n *pcieMgrFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if pciemgrkey, ok := key.(uint32); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", pciemgrkey)}
	}
	return nil
}

// MetaToKey converts meta to network key
func (n *pcieMgrFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint32
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type pciePortFns struct{}

// KeyToMeta converts network key to meta
func (n *pciePortFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if pcieportkey, ok := key.(uint32); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", pcieportkey)}
	}
	return nil
}

// MetaToKey converts meta to network key
func (n *pciePortFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint32
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}
