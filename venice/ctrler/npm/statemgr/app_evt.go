// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"strconv"

	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// AppState is a wrapper for app object
type AppState struct {
	App      *ctkit.App `json:"-"` // app object
	stateMgr *Statemgr  // pointer to state manager
}

// AppStateFromObj converts from runtime object to app state
func AppStateFromObj(obj runtime.Object) (*AppState, error) {
	switch obj.(type) {
	case *ctkit.App:
		apobj := obj.(*ctkit.App)
		switch apobj.HandlerCtx.(type) {
		case *AppState:
			aps := apobj.HandlerCtx.(*AppState)
			return aps, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewAppState creates new app state object
func NewAppState(app *ctkit.App, stateMgr *Statemgr) (*AppState, error) {
	aps := &AppState{
		App:      app,
		stateMgr: stateMgr,
	}
	app.HandlerCtx = aps

	// store it in local DB
	stateMgr.mbus.AddObject(convertApp(aps))

	return aps, nil
}

// convertApp converts from npm state to netproto app
func convertApp(aps *AppState) *netproto.App {
	protoPort := []string{}

	// convert protocol/port
	for _, pp := range aps.App.Spec.ProtoPorts {
		protoPort = append(protoPort, fmt.Sprintf("%s/%s", pp.Protocol, pp.Ports))
	}

	// build sg message
	app := netproto.App{
		TypeMeta:   aps.App.TypeMeta,
		ObjectMeta: aps.App.ObjectMeta,
		Spec: netproto.AppSpec{
			ProtoPorts:     protoPort,
			ALGType:        aps.App.Spec.ALG.Type,
			AppIdleTimeout: aps.App.Spec.Timeout,
			ALG:            &netproto.ALG{},
		},
	}

	switch aps.App.Spec.ALG.Type {
	case "ICMP":
		ictype, _ := strconv.Atoi(aps.App.Spec.ALG.Icmp.Type)
		icode, _ := strconv.Atoi(aps.App.Spec.ALG.Icmp.Code)

		app.Spec.ALG.ICMP = &netproto.ICMP{
			Type: uint32(ictype),
			Code: uint32(icode),
		}
	case "DNS":
		app.Spec.ALG.DNS = &netproto.DNS{
			DropMultiQuestionPackets: aps.App.Spec.ALG.Dns.DropMultiQuestionPackets,
			DropLargeDomainPackets:   aps.App.Spec.ALG.Dns.DropLargeDomainNamePackets,
			DropLongLabelPackets:     aps.App.Spec.ALG.Dns.DropLongLabelPackets,
			MaxMessageLength:         aps.App.Spec.ALG.Dns.MaxMessageLength,
			QueryResponseTimeout:     aps.App.Spec.ALG.Dns.QueryResponseTimeout,
		}
	case "FTP":
		app.Spec.ALG.FTP = &netproto.FTP{
			AllowMismatchIPAddresses: aps.App.Spec.ALG.Ftp.AllowMismatchIPAddress,
		}
	case "SunRPC":
		for _, sunrpc := range aps.App.Spec.ALG.Sunrpc {
			pgmID, _ := strconv.Atoi(sunrpc.ProgramID)
			app.Spec.ALG.SUNRPC = append(app.Spec.ALG.SUNRPC,
				&netproto.RPC{
					ProgramID:        uint32(pgmID),
					ProgramIDTimeout: sunrpc.Timeout,
				})
		}
	case "MSRPC":
		for _, msrpc := range aps.App.Spec.ALG.Msrpc {
			pgmID, _ := strconv.Atoi(msrpc.ProgramUUID)
			app.Spec.ALG.MSRPC = append(app.Spec.ALG.MSRPC,
				&netproto.RPC{
					ProgramID:        uint32(pgmID),
					ProgramIDTimeout: msrpc.Timeout,
				})
		}
	case "TFTP":
	case "RTSP":
	}

	return &app
}

// attachPolicy adds a policy to attached policy list
func (app *AppState) attachPolicy(sgpName string) error {
	// see if the policy is already part of the list
	for _, pn := range app.App.Status.AttachedPolicies {
		if pn == sgpName {
			return nil
		}
	}

	// add the policy to the list
	app.App.Status.AttachedPolicies = append(app.App.Status.AttachedPolicies, sgpName)

	// save the updated app
	app.stateMgr.mbus.UpdateObject(convertApp(app))
	app.App.Write()

	return nil
}

// detachPolicy removes a policy from
func (app *AppState) detachPolicy(sgpName string) error {
	// see if the policy is already part of the list
	for idx, pn := range app.App.Status.AttachedPolicies {
		if pn == sgpName {
			app.App.Status.AttachedPolicies = append(app.App.Status.AttachedPolicies[:idx], app.App.Status.AttachedPolicies[idx+1:]...)
		}
	}
	// save the updated app
	app.stateMgr.mbus.UpdateObject(convertApp(app))
	app.App.Write()

	return nil
}

// OnAppCreate handles app creation
func (sm *Statemgr) OnAppCreate(app *ctkit.App) error {
	// see if we already have the app
	hs, err := sm.FindApp(app.Tenant, app.Name)
	if err == nil {
		hs.App = app
		return nil
	}

	log.Infof("Creating app: %+v", app)

	// create new app object
	hs, err = NewAppState(app, sm)
	if err != nil {
		log.Errorf("Error creating app %+v. Err: %v", app, err)
		return err
	}

	return nil
}

// OnAppUpdate handles update app event
func (sm *Statemgr) OnAppUpdate(app *ctkit.App) error {
	return nil
}

// OnAppDelete handles app deletion
func (sm *Statemgr) OnAppDelete(app *ctkit.App) error {
	// see if we have the app
	fapp, err := sm.FindApp(app.Tenant, app.Name)
	if err != nil {
		log.Errorf("Could not find the app %v. Err: %v", app, err)
		return err
	}

	log.Infof("Deleting app: %+v", fapp)

	// delete the object
	return sm.mbus.DeleteObject(convertApp(fapp))
}

// FindApp finds a app
func (sm *Statemgr) FindApp(tenant, name string) (*AppState, error) {
	// find the object
	obj, err := sm.FindObject("App", tenant, "default", name)
	if err != nil {
		return nil, err
	}

	return AppStateFromObj(obj)
}

// ListApps lists all apps
func (sm *Statemgr) ListApps() ([]*AppState, error) {
	objs := sm.ListObjects("App")

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
