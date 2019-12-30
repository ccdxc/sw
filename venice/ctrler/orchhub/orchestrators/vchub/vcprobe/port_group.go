package vcprobe

import (
	"errors"

	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"
)

// AddPenPG returns a new instance of PenPG
func (v *VCProbe) AddPenPG(dcName, dvsName string, pgConfigSpec *types.DVPortgroupConfigSpec) error {
	pgName := pgConfigSpec.Name
	_, finder, _ := v.GetClientWithRLock()
	defer v.ReleaseClientRLock()

	// Check if it exists already
	if _, err := v.getPenPG(dcName, pgName, finder); err == nil {
		v.Log.Infof("DC: %s - PG %s already exists", dcName, pgName)
		// TODO: check error isn't intermittent
		return nil
	}

	objDvs, err := v.GetPenDVS(dcName, dvsName)
	if err != nil {
		v.Log.Errorf("Couldn't find DVS %s for PG creation %s: err", dcName, dvsName, err)
		return err
	}

	task, err := objDvs.AddPortgroup(v.ClientCtx, []types.DVPortgroupConfigSpec{*pgConfigSpec})
	if err != nil {
		v.Log.Errorf("Failed at adding port group: %s, err: %s", pgName, err)
		return err
	}

	_, err = task.WaitForResult(v.ClientCtx, nil)
	if err != nil {
		v.Log.Errorf("Failed at waiting results of adding port group: %s, err: %s", pgName, err)
		return err
	}

	return nil
}

// GetPenPG returns the PG if it exists, or an error
func (v *VCProbe) GetPenPG(dcName string, pgName string) (*object.DistributedVirtualPortgroup, error) {
	_, finder, _ := v.GetClientWithRLock()
	defer v.ReleaseClientRLock()
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
	objPg, ok := net.(*object.DistributedVirtualPortgroup)
	if !ok {
		v.Log.Errorf("Failed at getting dvs port group(%s) object. Net obj %v", pgName, net)
		return nil, errors.New("Failed at getting dvs port group object")
	}

	var dvsPg mo.DistributedVirtualPortgroup
	err = objPg.Properties(v.ClientCtx, objPg.Reference(), nil, &dvsPg)
	if err != nil {
		v.Log.Errorf("Failed at getting dv port group properties, err: %s", err)
		return nil, err
	}

	return objPg, nil
}

// RemovePenPG removes the pg with the given name
func (v *VCProbe) RemovePenPG(dcName, pgName string) error {
	_, finder, _ := v.GetClientWithRLock()
	defer v.ReleaseClientRLock()

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
