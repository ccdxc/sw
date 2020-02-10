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
func (v *VCProbe) AddPenPG(dcName, dvsName string, pgConfigSpec *types.DVPortgroupConfigSpec) error {
	pgName := pgConfigSpec.Name

	var task *object.Task
	var err error
	// Check if it exists already
	if pgObj, err := v.GetPenPG(dcName, pgName); err == nil {
		moPG, err := v.GetPGConfig(dcName, pgName, []string{"config"})
		if err != nil {
			v.Log.Errorf("Failed at getting pg properties, err: %s", err)
			return err
		}
		pgConfigSpec.ConfigVersion = moPG.Config.ConfigVersion
		v.Log.Infof("DC: %s - PG %s already exists, reconfiguring...", dcName, pgName)
		task, err = pgObj.Reconfigure(v.ClientCtx, *pgConfigSpec)
	} else {
		// Creating PG
		objDvs, err := v.GetPenDVS(dcName, dvsName)
		if err != nil {
			v.Log.Errorf("Couldn't find DVS %s for PG creation %s: err", dcName, dvsName, err)
			return err
		}

		task, err = objDvs.AddPortgroup(v.ClientCtx, []types.DVPortgroupConfigSpec{*pgConfigSpec})
	}

	if err != nil {
		v.Log.Errorf("Failed at adding/reconfiguring port group: %s, err: %s", pgName, err)
		return err
	}

	_, err = task.WaitForResult(v.ClientCtx, nil)
	if err != nil {
		v.Log.Errorf("Failed at waiting results of add port group: %s, err: %s", pgName, err)
		return err
	}

	return nil
}

// GetPenPG returns the PG if it exists, or an error
func (v *VCProbe) GetPenPG(dcName string, pgName string) (*object.DistributedVirtualPortgroup, error) {
	finder := v.GetFinderWithRLock()
	defer v.ReleaseClientsRLock()
	return v.getPenPG(dcName, pgName, finder)
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
func (v *VCProbe) GetPGConfig(dcName string, pgName string, ps []string) (*mo.DistributedVirtualPortgroup, error) {
	objPg, err := v.GetPenPG(dcName, pgName)
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

// RenamePG renames the given PG
func (v *VCProbe) RenamePG(dcName string, oldName string, newName string) error {
	objPg, err := v.GetPenPG(dcName, oldName)
	if err != nil {
		return err
	}
	task, err := objPg.Rename(v.ClientCtx, newName)
	if err != nil {
		// Failed to delete PG
		v.Log.Errorf("Failed to rename PG %s to %s, err", oldName, newName, err)
		// TODO: Generate Event and mark object?
		return err
	}

	_, err = task.WaitForResult(v.ClientCtx, nil)
	if err != nil {
		// Failed to delete PG
		v.Log.Errorf("Failed to wait for PG rename %s to %s, err", oldName, newName, err)
		// TODO: Generate Event and mark object?
		return err
	}

	return nil
}

// RemovePenPG removes the pg with the given name
func (v *VCProbe) RemovePenPG(dcName, pgName string) error {
	finder := v.GetFinderWithRLock()
	defer v.ReleaseClientsRLock()

	objPG, err := v.getPenPG(dcName, pgName, finder)
	if err != nil {
		return err
	}
	task, err := objPG.Destroy(v.ClientCtx)
	if err != nil {
		// Failed to delete PG
		v.Log.Errorf("Failed to delete PG, err", err)
		// TODO: Generate Event and mark object?
		return err
	}

	_, err = task.WaitForResult(v.ClientCtx, nil)
	if err != nil {
		// Failed to delete PG
		v.Log.Errorf("Failed to wait for delete PG, err", err)
		// TODO: Generate Event and mark object?
		return err
	}

	return nil
}
