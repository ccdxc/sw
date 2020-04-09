package vcprobe

import (
	"errors"
	"fmt"

	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"
)

// AddPenPG returns a new instance of PenPG, or reconfigures the PG
// to match the given spec if it already exists
func (v *VCProbe) AddPenPG(dcName string, dvsName string, pgConfigSpec *types.DVPortgroupConfigSpec, equalFn IsPGConfigEqual, retry int) error {
	pgName := pgConfigSpec.Name
	getAndCheck := func() (*mo.DistributedVirtualPortgroup, bool, error) {
		moPG, err := v.GetPGConfig(dcName, pgName, []string{"config"}, 1)
		if err != nil {
			v.Log.Errorf("Failed at getting pg properties, err: %s", err)
			return nil, false, err
		}
		// Check if config is equal
		if equalFn != nil && equalFn(pgConfigSpec, moPG) {
			v.Log.Infof("DC: %s - PG %s  config in vCenter is equal to given spec", dcName, pgName)
			return moPG, true, nil
		}
		return moPG, false, nil
	}

	fn := func() (interface{}, error) {
		ctx := v.ClientCtx
		if ctx == nil {
			return nil, fmt.Errorf("Client context canceled")
		}

		var task *object.Task
		var err error
		// Check if it exists already
		if pgObj, err := v.GetPenPG(dcName, pgName, 1); err == nil {
			moPG, isEqual, err := getAndCheck()
			if err != nil {
				return nil, err
			}
			if isEqual {
				return nil, nil
			}

			pgConfigSpec.ConfigVersion = moPG.Config.ConfigVersion
			v.Log.Infof("DC: %s - PG %s already exists, reconfiguring...", dcName, pgName)
			task, err = pgObj.Reconfigure(ctx, *pgConfigSpec)
		} else {
			// Creating PG
			objDvs, dvsErr := v.GetPenDVS(dcName, dvsName, 1)
			if dvsErr != nil {
				v.Log.Errorf("Couldn't find DVS %s for PG creation %s: err", dcName, dvsName, err)
				return nil, dvsErr
			}

			task, err = objDvs.AddPortgroup(ctx, []types.DVPortgroupConfigSpec{*pgConfigSpec})
		}

		if err != nil {
			v.Log.Errorf("Failed at adding/reconfiguring port group: %s, err: %s", pgName, err)

			_, isEqual, err1 := getAndCheck()
			if err1 == nil && isEqual {
				return nil, nil
			}

			return nil, err
		}

		_, err = task.WaitForResult(ctx, nil)
		if err != nil {
			v.Log.Errorf("Failed at waiting results of add port group: %s, err: %s", pgName, err)

			_, isEqual, err1 := getAndCheck()
			if err1 == nil && isEqual {
				return nil, nil
			}

			return nil, err
		}

		return nil, nil
	}
	_, err := v.withRetry(fn, retry)
	return err
}

// GetPenPG returns the PG if it exists, or an error
func (v *VCProbe) GetPenPG(dcName string, pgName string, retry int) (*object.DistributedVirtualPortgroup, error) {
	fn := func() (interface{}, error) {
		client := v.GetClientWithRLock()
		defer v.ReleaseClientsRLock()
		return v.getPenPG(dcName, pgName, client)
	}
	ret, err := v.withRetry(fn, retry)
	if ret == nil {
		return nil, err
	}
	return ret.(*object.DistributedVirtualPortgroup), err
}

func (v *VCProbe) getPenPG(dcName string, pgName string, client *govmomi.Client) (*object.DistributedVirtualPortgroup, error) {

	dc, err := v.getDCObj(dcName, client)
	if err != nil {
		v.Log.Errorf("Datacenter: %s doesn't exist, err: %s", dcName, err)
		return nil, err
	}

	finder := v.CreateFinder(client)
	finder.SetDatacenter(dc)

	net, err := finder.Network(v.ClientCtx, pgName)
	if err != nil {
		v.Log.Errorf("Port group for %s is not present in vCenter", pgName)
		return nil, fmt.Errorf("Port group for %s is not present in vCenter", pgName)
	}
	objPg, ok := net.(*object.DistributedVirtualPortgroup)
	if !ok {
		v.Log.Errorf("Failed at getting dvs port group(%s) object. Net obj %v", pgName, net)
		return nil, errors.New("Failed at getting dvs port group object")
	}

	return objPg, nil
}

// GetPGConfig returns the mo object for the given PG
func (v *VCProbe) GetPGConfig(dcName string, pgName string, ps []string, retry int) (*mo.DistributedVirtualPortgroup, error) {
	fn := func() (interface{}, error) {
		objPg, err := v.GetPenPG(dcName, pgName, 1)
		if err != nil {
			return nil, err
		}
		var dvsPg mo.DistributedVirtualPortgroup
		err = objPg.Properties(v.ClientCtx, objPg.Reference(), ps, &dvsPg)
		if err != nil {
			v.Log.Errorf("Failed at getting dv port group properties, err: %s", err)
			return nil, err
		}
		return &dvsPg, nil
	}
	ret, err := v.withRetry(fn, retry)
	if ret == nil {
		return nil, err
	}
	return ret.(*mo.DistributedVirtualPortgroup), err
}

// RenamePG renames the given PG
func (v *VCProbe) RenamePG(dcName string, oldName string, newName string, retry int) error {
	fn := func() (interface{}, error) {
		objPg, err := v.GetPenPG(dcName, oldName, 1)
		if err != nil {
			v.Log.Errorf("Failed to rename PG %s to %s, failed to find PG: err %s", oldName, newName, err)
			return nil, err
		}
		ctx := v.ClientCtx
		if ctx == nil {
			return nil, fmt.Errorf("Client Context was nil")
		}
		task, err := objPg.Rename(ctx, newName)
		if err != nil {
			// Failed to rename PG
			v.Log.Errorf("Failed to rename PG %s to %s, err %s", oldName, newName, err)
			return nil, err
		}

		_, err = task.WaitForResult(ctx, nil)
		if err != nil {
			// Failed to rename PG
			v.Log.Errorf("Failed to wait for PG rename %s to %s, err %s", oldName, newName, err)
			return nil, err
		}

		return nil, nil
	}
	_, err := v.withRetry(fn, retry)
	return err
}

// RemovePenPG removes the pg with the given name
func (v *VCProbe) RemovePenPG(dcName string, pgName string, retry int) error {
	fn := func() (interface{}, error) {
		client := v.GetClientWithRLock()
		defer v.ReleaseClientsRLock()

		objPG, err := v.getPenPG(dcName, pgName, client)
		if err != nil {
			return nil, err
		}
		task, err := objPG.Destroy(v.ClientCtx)
		if err != nil {
			// Failed to delete PG
			v.Log.Errorf("Failed to delete PG, err", err)
			// TODO: Generate Event and mark object?
			return nil, err
		}

		_, err = task.WaitForResult(v.ClientCtx, nil)
		if err != nil {
			// Failed to delete PG
			v.Log.Errorf("Failed to wait for delete PG, err %s", err)
			// TODO: Generate Event and mark object?
			return nil, err
		}

		return nil, nil
	}
	_, err := v.withRetry(fn, retry)
	return err
}
