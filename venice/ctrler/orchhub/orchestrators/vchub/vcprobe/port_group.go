package vcprobe

import (
	"errors"
	"sync"

	"github.com/vmware/govmomi/object"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"
)

// PenPG represents an instance of a port group on a DVS
type PenPG struct {
	vcInst
	ObjPg   *object.DistributedVirtualPortgroup
	PgName  string
	PgMutex sync.Mutex
}

// AddPenPG returns a new instance of PenPG
func (d *PenDVS) AddPenPG(pgConfigSpec *types.DVPortgroupConfigSpec) (*PenPG, error) {
	d.DvsMutex.Lock()
	defer d.DvsMutex.Unlock()

	pgName := pgConfigSpec.Name

	task, err := d.ObjDvs.AddPortgroup(d.ctx, []types.DVPortgroupConfigSpec{*pgConfigSpec})
	if err != nil {
		d.Log.Errorf("Failed at adding port group: %s, err: %s", pgName, err)
		return nil, err
	}

	_, err = task.WaitForResult(d.ctx, nil)
	if err != nil {
		d.Log.Errorf("Failed at waiting results of adding port group: %s, err: %s", pgName, err)
		return nil, err
	}

	net, err := d.finder.Network(d.ctx, pgConfigSpec.Name)
	objPg, ok := net.(*object.DistributedVirtualPortgroup)
	if !ok {
		d.Log.Errorf("Failed at getting dvs port group(%s) object", pgName)
		return nil, errors.New("Failed at getting dvs port group object")
	}

	var dvsPg mo.DistributedVirtualPortgroup
	err = objPg.Properties(d.ctx, objPg.Reference(), nil, &dvsPg)
	if err != nil {
		d.Log.Errorf("Failed at getting dv port group properties, err: %s", err)
		return nil, err
	}

	penPG := &PenPG{
		vcInst: d.vcInst,
		ObjPg:  objPg,
		PgName: pgName,
	}

	d.dvsPgs = append(d.dvsPgs, penPG)

	return penPG, nil
}

/*
// getMoPGRef converts object.DistributedVirtualPortgroup to mo.DistributedVirtualPortgroup
func (p *PenPG) getMoPGRef() (*mo.DistributedVirtualPortgroup, error) {
	p.PgMutex.Lock()
	defer p.PgMutex.Unlock()

	var pg mo.DistributedVirtualPortgroup
	err := p.ObjPg.Properties(p.ctx, p.ObjPg.Reference(), nil, &pg)
	if err != nil {
		p.Log.Errorf("Failed at getting PG properties, err: %s", err)
		return nil, err
	}

	return &pg, nil
}*/
