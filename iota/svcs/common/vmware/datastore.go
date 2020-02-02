package vmware

import (
	"io"

	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/soap"
)

// Datastore encapsulates all communication with a vmware data store.
type Datastore struct {
	Name string

	Entity     *Entity
	datacenter *object.Datacenter
	datastore  *object.Datastore
}

// Upload uploads the contents of a reader to the path provided.
func (ds *Datastore) Upload(fullPath string, r io.Reader) error {
	return ds.datastore.Upload(ds.Entity.ConnCtx.context.context, r, fullPath, &soap.DefaultUpload)
}

// Remove removes a path from the datastore.
func (ds *Datastore) Remove(p string) error {
	return ds.datastore.NewFileManager(ds.datacenter, true).Delete(ds.Entity.ConnCtx.context.context, p)
}
