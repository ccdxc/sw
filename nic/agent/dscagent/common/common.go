package common

import (
	"github.com/gogo/protobuf/proto"
	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils/validator"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleApp handles CRUD Methods for App Object
func HandleApp(i types.InfraAPI, oper types.Operation, app netproto.App) (apps []netproto.App, err error) {
	err = utils.ValidateMeta(oper, app.Kind, app.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}
	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.App
		)
		dat, err = i.Read(app.Kind, app.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err)
		}
		apps = append(apps, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = i.List(app.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var app netproto.App
			err := proto.Unmarshal(o, &app)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err))
				continue
			}
			apps = append(apps, app)
		}

		return
	case types.Create:
		// Alloc ID if ID field is empty. This will be pre-populated in case of config replays
		if app.Status.AppID == 0 {
			app.Status.AppID = i.AllocateID(types.AppID, 0)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingApp netproto.App
		dat, err := i.Read(app.Kind, app.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound)
		}
		err = existingApp.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&app.Spec, &existingApp.Spec) {
			//log.Infof("App: %s | Info: %s ", app.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		// Reuse ID from store
		app.Status.AppID = existingApp.Status.AppID
	case types.Delete:
		var existingApp netproto.App
		dat, err := i.Read(app.Kind, app.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingApp.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err)
		}
		app = existingApp
	}
	// Perform object validations
	err = validator.ValidateApp(app)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	log.Infof("App: %v | Op: %s | %s", app, oper, types.InfoHandleObjBegin)
	defer log.Infof("App: %v | Op: %s | %s", app, oper, types.InfoHandleObjEnd)

	// Take a lock to ensure a single HAL API is active at any given point
	err = handleApp(i, oper, app)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	return
}
