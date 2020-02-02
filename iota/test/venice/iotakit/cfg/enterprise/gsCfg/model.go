package gscfg

import (
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/base"
)

//GsCfg encapsulate all Gs configuration objects
type GsCfg struct {
	base.EntBaseCfg
}

func NewGsCfg() *GsCfg {
	return &GsCfg{}
}
