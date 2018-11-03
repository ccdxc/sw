package example

import "github.com/pensando/sw/api"

type networkTranslatorFns struct{}

// KeyToMeta converts network key to meta
func (n *networkTranslatorFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	return nil
}

// MetaToKey converts meta to network key
func (n *networkTranslatorFns) MetaToKey(*api.ObjectMeta) interface{} {
	return nil
}
