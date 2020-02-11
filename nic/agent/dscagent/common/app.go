package common

import (
	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

func handleApp(infraAPI types.InfraAPI, oper types.Operation, app netproto.App) error {
	switch oper {
	case types.Create:
		return createAppHandler(infraAPI, app)
	case types.Update:
		return updateAppHandler(infraAPI, app)
	case types.Delete:
		return deleteAppHandler(infraAPI, app)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createAppHandler(infraAPI types.InfraAPI, app netproto.App) error {
	dat, _ := app.Marshal()

	if err := infraAPI.Store(app.Kind, app.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "App: %s | App: %v", app.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "App: %s | App: %v", app.GetKey(), err)
	}
	return nil
}

func updateAppHandler(infraAPI types.InfraAPI, app netproto.App) error {
	dat, _ := app.Marshal()

	if err := infraAPI.Store(app.Kind, app.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "App: %s | App: %v", app.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "App: %s | App: %v", app.GetKey(), err)
	}
	return nil
}

func deleteAppHandler(infraAPI types.InfraAPI, app netproto.App) error {
	if err := infraAPI.Delete(app.Kind, app.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "App: %s | Err: %v", app.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "App: %s | Err: %v", app.GetKey(), err)
	}
	return nil
}
