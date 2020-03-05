package vcprobe

import (
	"errors"
	"fmt"

	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"
)

// AddPenPG returns a new instance of PenPG, or reconfigures the PG
// to match the given spec if it already exists
func (v *VCProbe) AddPenPG(dcName, dvsName string, pgConfigSpec *types.DVPortgroupConfigSpec, retry int) error {
	fn := func() (interface{}, error) {
		pgName := pgConfigSpec.Name

		var task *object.Task
		var err error
		// Check if it exists already
		if pgObj, err := v.GetPenPG(dcName, pgName, 1); err == nil {
			moPG, err := v.GetPGConfig(dcName, pgName, []string{"config"}, 1)
			if err != nil {
				v.Log.Errorf("Failed at getting pg properties, err: %s", err)
				return nil, err
			}
			pgConfigSpec.ConfigVersion = moPG.Config.ConfigVersion
			v.Log.Infof("DC: %s - PG %s already exists, reconfiguring...", dcName, pgName)
			task, err = pgObj.Reconfigure(v.ClientCtx, *pgConfigSpec)
		} else {
			// Creating PG
			objDvs, err := v.GetPenDVS(dcName, dvsName, 1)
			if err != nil {
				v.Log.Errorf("Couldn't find DVS %s for PG creation %s: err", dcName, dvsName, err)
				return nil, err
			}

			task, err = objDvs.AddPortgroup(v.ClientCtx, []types.DVPortgroupConfigSpec{*pgConfigSpec})
		}

		if err != nil {
			v.Log.Errorf("Failed at adding/reconfiguring port group: %s, err: %s", pgName, err)
			return nil, err
		}

		_, err = task.WaitForResult(v.ClientCtx, nil)
		if err != nil {
			v.Log.Errorf("Failed at waiting results of add port group: %s, err: %s", pgName, err)
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
		finder := v.CreateFinder(client)
		defer v.ReleaseClientsRLock()
		return v.getPenPG(dcName, pgName, finder)
	}
	ret, err := v.withRetry(fn, retry)
	if ret == nil {
		return nil, err
	}
	return ret.(*object.DistributedVirtualPortgroup), err
}

func (v *VCProbe) getPenPG(dcName string, pgName string, finder *find.Finder) (*object.DistributedVirtualPortgroup, error) {
	dc, err := finder.Datacenter(v.ClientCtx, dcName)
	if err != nil {
		v.Log.Errorf("Datacenter: %s doesn't exist, err: %s", dcName, err)
		// return nil, err
		return nil, err
	}

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
			return nil, err
		}
		task, err := objPg.Rename(v.ClientCtx, newName)
		if err != nil {
			// Failed to delete PG
			v.Log.Errorf("Failed to rename PG %s to %s, err", oldName, newName, err)
			// TODO: Generate Event and mark object?
			return nil, err
		}

		_, err = task.WaitForResult(v.ClientCtx, nil)
		if err != nil {
			// Failed to delete PG
			v.Log.Errorf("Failed to wait for PG rename %s to %s, err", oldName, newName, err)
			// TODO: Generate Event and mark object?
			return nil, err
		}

		return nil, nil
	}
	_, err := v.withRetry(fn, retry)
	return err
}

// RemovePenPG removes the pg with the given name
func (v *VCProbe) RemovePenPG(dcName, pgName string, retry int) error {
	fn := func() (interface{}, error) {
		client := v.GetClientWithRLock()
		finder := v.CreateFinder(client)
		defer v.ReleaseClientsRLock()

		objPG, err := v.getPenPG(dcName, pgName, finder)
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
