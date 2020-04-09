package vcprobe

import (
	"fmt"

	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/mo"
)

// AddPenDC adds a new PenDC to the given vcprobe instance
// This is for testing since we don't really create DC
func (v *VCProbe) AddPenDC(dcName string, retry int) error {
	fn := func() (interface{}, error) {
		client := v.GetClientWithRLock()
		defer v.ReleaseClientsRLock()
		folder := object.NewRootFolder(client.Client)
		_, err := folder.CreateDatacenter(v.ClientCtx, dcName)
		if err != nil {
			v.Log.Errorf("Couldn't create datacenter: %s, err: %s", dcName, err)
		}
		return nil, err
	}
	_, err := v.withRetry(fn, retry)
	return err
}

func (v *VCProbe) getDCObj(dcName string, client *govmomi.Client) (*object.Datacenter, error) {
	// attempt to look up in state
	v.State.DcMapLock.RLock()
	ref, ok := v.State.DcIDMap[dcName]
	v.State.DcMapLock.RUnlock()
	if !ok {
		finder := v.CreateFinder(client)
		dc, err := finder.Datacenter(v.ClientCtx, dcName)
		if err != nil {
			v.Log.Errorf("Datacenter: %s doesn't exist, err: %s", dcName, err)
			return nil, err
		}
		return dc, nil
	}
	dcObj := object.NewDatacenter(client.Client, ref)
	// check ref is still valid
	var dc mo.Datacenter
	err := dcObj.Properties(v.ClientCtx, dcObj.Reference(), []string{"name"}, &dc)
	if err != nil {
		v.Log.Errorf("Failed at getting dc properties, err: %s", err)
		return nil, err
	}

	return dcObj, nil
}

// RenameDC renames the DC
func (v *VCProbe) RenameDC(oldName, newName string, retry int) error {
	fn := func() (interface{}, error) {
		client := v.GetClientWithRLock()
		defer v.ReleaseClientsRLock()
		dcObj, err := v.getDCObj(oldName, client)
		if err != nil {
			v.Log.Errorf("Failed to rename PG %s to %s, failed to find PG: err %s", oldName, newName, err)
			return nil, err
		}
		ctx := v.ClientCtx
		if ctx == nil {
			return nil, fmt.Errorf("Client Context was nil")
		}
		task, err := dcObj.Rename(ctx, newName)
		if err != nil {
			v.Log.Errorf("Failed to rename DC %s to %s, err %s", oldName, newName, err)
			return nil, err
		}

		_, err = task.WaitForResult(ctx, nil)
		if err != nil {
			v.Log.Errorf("Failed to wait for DC rename %s to %s, err %s", oldName, newName, err)
			return nil, err
		}

		return nil, nil
	}
	_, err := v.withRetry(fn, retry)
	return err
}

// RemovePenDC removes the given PenDC
// This is for testing since we don't really delete DC
func (v *VCProbe) RemovePenDC(dcName string, retry int) error {
	fn := func() (interface{}, error) {
		client := v.GetClientWithRLock()
		defer v.ReleaseClientsRLock()

		dc, err := v.getDCObj(dcName, client)
		if err != nil {
			v.Log.Errorf("Failed at finding DC %s for destroying, %s", dcName, err)
			return nil, err
		}

		task, err := dc.Destroy(v.ClientCtx)
		if err != nil {
			v.Log.Errorf("Failed at destroying DC %s, err: %s", dcName, err)
			return nil, err
		}

		err = task.Wait(v.ClientCtx)
		if err != nil {
			v.Log.Errorf("Failed at destroying DC %s, err: %s", dcName, err)
			return nil, err
		}

		return nil, nil
	}

	_, err := v.withRetry(fn, retry)
	return err
}
