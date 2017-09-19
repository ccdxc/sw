package etcd

import (
	"github.com/coreos/etcd/clientv3"

	"github.com/pensando/sw/venice/utils/kvstore"
)

// translateCmps converts kvstore Cmps to clientv3 Cmps.
func translateCmps(cs ...kvstore.Cmp) []clientv3.Cmp {
	v3Cmps := []clientv3.Cmp{}
	for _, c := range cs {
		switch c.Target {
		case kvstore.Version:
			v3Cmps = append(v3Cmps, clientv3.Compare(clientv3.ModRevision(c.Key), c.Operator, c.Version))
		case kvstore.Value:
			v3Cmps = append(v3Cmps, clientv3.Compare(clientv3.Value(c.Key), c.Operator, c.Value))
		}
	}
	return v3Cmps
}
