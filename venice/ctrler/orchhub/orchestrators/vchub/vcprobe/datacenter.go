package vcprobe

import (
	"github.com/vmware/govmomi/find"
	"github.com/vmware/govmomi/object"
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

func (v *VCProbe) getPenDC(dcName string, finder *find.Finder) (*object.Datacenter, error) {
	dc, err := finder.Datacenter(v.ClientCtx, dcName)
	if err != nil {
		v.Log.Errorf("Datacenter: %s doesn't exist, err: %s", dcName, err)
		return nil, err
	}
	return dc, nil
}

// RemovePenDC removes the given PenDC
// This is for testing since we don't really delete DC
func (v *VCProbe) RemovePenDC(dcName string, retry int) error {
	fn := func() (interface{}, error) {
		client := v.GetClientWithRLock()
		finder := v.CreateFinder(client)
		defer v.ReleaseClientsRLock()

		dc, err := v.getPenDC(dcName, finder)
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
