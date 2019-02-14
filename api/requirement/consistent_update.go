package requirement

import (
	"context"
	"fmt"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/graph"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

type constUpdateReq struct {
	store  graph.Interface
	ov     apiintf.CacheInterface
	apiSrv apiserver.Server
	items  []apiintf.ConstUpdateItem
}

// NewConstUpdateReq creates a new consistent update requirement.
func NewConstUpdateReq(reqs []apiintf.ConstUpdateItem, g graph.Interface, kvs apiintf.CacheInterface, apiSrv apiserver.Server) apiintf.Requirement {
	for i := range reqs {
		if reqs[i].Key == "" || reqs[i].Func == nil || reqs[i].Into == nil {

		}
	}
	ret := &constUpdateReq{
		store:  g,
		ov:     kvs,
		apiSrv: apiSrv,
		items:  reqs,
	}
	log.Infof("Created new ConsistentUpdateReq [%s]", ret.String())
	return ret
}

// Check implements the the Requirement interface
func (r *constUpdateReq) Check(ctx context.Context) []error {
	// Nothing to do for check
	return nil
}

// Apply does a consistent update of all the keys in requrement and updates the cache.
func (r *constUpdateReq) Apply(ctx context.Context, txn kvstore.Txn, cache apiintf.CacheInterface) error {
	for _, v := range r.items {
		log.Infof("Apply ConsistentUpdate for [%v]", v.Key)
		newObj, err := runtime.NewEmpty(v.Into)
		if err != nil {
			return errors.New("could not create new empty object")
		}
		err = cache.Get(ctx, v.Key, newObj)
		if err != nil {
			return errors.New("Consistent update needed on object not in backend")
		}

		meta, _ := apiutils.MustGetObjectMetaVersion(newObj)
		o, err := v.Func(newObj)
		if err != nil {
			return errors.Wrap(err, "consistent update failed")
		}
		err = txn.Update(v.Key, o)
		txn.AddComparator(kvstore.Compare(kvstore.WithVersion(v.Key), "=", meta.ResourceVersion))
		log.Infof("updated object is [%+v]", o)
	}
	return nil
}

// Finalize implements the Requirement interface
func (r *constUpdateReq) Finalize(ctx context.Context) error {
	// No action needed on finalize. References for updated objects are handled by the reference requirement.
	return nil
}

// String returns a humand readable requirement
func (r *constUpdateReq) String() string {
	ret := "\n Consistent Update Requirement: \n"
	for _, v := range r.items {
		ret = ret + fmt.Sprintf("     Key : [%v]\n", v.Key)
	}
	return ret
}
