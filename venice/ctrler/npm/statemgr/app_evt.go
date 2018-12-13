// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"sync"

	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
)

// AppReactor is the event reactor for app events
type AppReactor struct {
	stateMgr *Statemgr // state manager
}

// AppHandler app event handler interface
type AppHandler interface {
	CreateApp(app security.App) error
	DeleteApp(app security.App) error
}

// AppState is a wrapper for app object
type AppState struct {
	sync.Mutex             // lock the app object
	security.App           // app object
	stateMgr     *Statemgr // pointer to state manager
}

// AppStateFromObj conerts from memdb object to app state
func AppStateFromObj(obj memdb.Object) (*AppState, error) {
	switch obj.(type) {
	case *AppState:
		nsobj := obj.(*AppState)
		return nsobj, nil
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewAppState creates new app state object
func NewAppState(app security.App, stateMgr *Statemgr) (*AppState, error) {
	hs := &AppState{
		App:      app,
		stateMgr: stateMgr,
	}

	// store it in local DB
	stateMgr.memDB.AddObject(hs)

	return hs, nil
}

// attachPolicy adds a policy to attached policy list
func (app *AppState) attachPolicy(sgpName string) error {
	// see if the policy is already part of the list
	for _, pn := range app.Status.AttachedPolicies {
		if pn == sgpName {
			return nil
		}
	}

	// add the policy to the list
	app.Status.AttachedPolicies = append(app.Status.AttachedPolicies, sgpName)

	// save the updated app
	app.stateMgr.memDB.UpdateObject(app)
	app.stateMgr.writer.WriteApp(&app.App)

	return nil
}

// detachPolicy removes a policy from
func (app *AppState) detachPolicy(sgpName string) error {
	// see if the policy is already part of the list
	for idx, pn := range app.Status.AttachedPolicies {
		if pn == sgpName {
			app.Status.AttachedPolicies = append(app.Status.AttachedPolicies[:idx], app.Status.AttachedPolicies[idx+1:]...)
		}
	}
	// save the updated app
	app.stateMgr.memDB.UpdateObject(app)
	app.stateMgr.writer.WriteApp(&app.App)

	return nil
}

// CreateApp handles app creation
func (hr *AppReactor) CreateApp(app security.App) error {
	// see if we already have the app
	hs, err := hr.stateMgr.FindApp(app.Tenant, app.Name)
	if err == nil {
		hs.App = app
		return nil
	}

	log.Infof("Creating app: %+v", app)

	// create new app object
	hs, err = NewAppState(app, hr.stateMgr)
	if err != nil {
		log.Errorf("Error creating app %+v. Err: %v", app, err)
		return err
	}

	return nil
}

// DeleteApp handles app deletion
func (hr *AppReactor) DeleteApp(app security.App) error {
	// see if we have the app
	hs, err := hr.stateMgr.FindApp(app.Tenant, app.Name)
	if err != nil {
		log.Errorf("Could not find the app %v. Err: %v", app, err)
		return err
	}

	log.Infof("Deleting app: %+v", app)

	// delete the object
	return hr.stateMgr.memDB.DeleteObject(hs)
}

// NewAppReactor creates new app event reactor
func NewAppReactor(sm *Statemgr) (*AppReactor, error) {
	app := AppReactor{
		stateMgr: sm,
	}

	return &app, nil
}

// FindApp finds a app
func (sm *Statemgr) FindApp(tenant, name string) (*AppState, error) {
	// find the object
	obj, err := sm.FindObject("App", tenant, name)
	if err != nil {
		return nil, err
	}

	return AppStateFromObj(obj)
}

// ListApps lists all apps
func (sm *Statemgr) ListApps() ([]*AppState, error) {
	objs := sm.memDB.ListObjects("App")

	var apps []*AppState
	for _, obj := range objs {
		app, err := AppStateFromObj(obj)
		if err != nil {
			return apps, err
		}

		apps = append(apps, app)
	}

	return apps, nil
}
