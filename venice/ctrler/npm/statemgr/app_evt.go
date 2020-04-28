// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"strconv"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	dNSDefaultPort = "53"
	dNSProto       = "udp"

	fTPDefaultPort = "21"
	fTPProto       = "tcp"

	tFTPDefaultPort = "69"
	tFTPProto       = "udp"

	sUNRPCDefaultPort = "111"
	sUNRPCProto       = "tcp"

	mSRPCDefaultPort = "135"
	mSRPCProto       = "tcp"
)

func getProtoPorts(app *security.App) []*netproto.ProtoPort {

	if app.Spec.ProtoPorts != nil && len(app.Spec.ProtoPorts) != 0 {
		var protoPorts []*netproto.ProtoPort
		for _, protoPort := range app.Spec.ProtoPorts {
			p := &netproto.ProtoPort{
				Protocol: protoPort.Protocol,
				Port:     protoPort.Ports,
			}
			protoPorts = append(protoPorts, p)

		}
		return protoPorts
	}
	if app.Spec.ALG != nil {
		switch app.Spec.ALG.Type {
		case security.ALG_DNS.String():
			return []*netproto.ProtoPort{
				&netproto.ProtoPort{
					Port:     dNSDefaultPort,
					Protocol: dNSProto,
				},
			}
		case security.ALG_MSRPC.String():
			return []*netproto.ProtoPort{
				&netproto.ProtoPort{
					Port:     mSRPCDefaultPort,
					Protocol: mSRPCProto,
				},
			}
		case security.ALG_SunRPC.String():
			return []*netproto.ProtoPort{
				&netproto.ProtoPort{
					Port:     sUNRPCDefaultPort,
					Protocol: sUNRPCProto,
				},
			}

		case security.ALG_FTP.String():
			return []*netproto.ProtoPort{
				&netproto.ProtoPort{
					Port:     fTPDefaultPort,
					Protocol: fTPProto,
				},
			}
		case security.ALG_TFTP.String():
			return []*netproto.ProtoPort{
				&netproto.ProtoPort{
					Port:     tFTPDefaultPort,
					Protocol: tFTPProto,
				},
			}
		}

	}

	return []*netproto.ProtoPort{}
}

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
	stateMgr.mbus.AddObjectWithReferences(app.MakeKey("security"), convertApp(aps), references(app))

	return aps, nil
}

// convertApp converts from npm state to netproto app
func convertApp(aps *AppState) *netproto.App {
	// build sg message
	creationTime, _ := types.TimestampProto(time.Now())
	app := netproto.App{
		TypeMeta:   aps.App.TypeMeta,
		ObjectMeta: agentObjectMeta(aps.App.ObjectMeta),
		Spec: netproto.AppSpec{
			AppIdleTimeout: aps.App.Spec.Timeout,
			ALG:            &netproto.ALG{},
		},
	}
	app.CreationTime = api.Timestamp{Timestamp: *creationTime}

	if aps.App.Spec.ALG != nil {
		switch aps.App.Spec.ALG.Type {
		case security.ALG_ICMP.String():
			if aps.App.Spec.ALG.Icmp != nil {
				app.Spec.ProtoPorts = []*netproto.ProtoPort{
					{
						Protocol: "icmp",
					},
				}
				ictype, _ := strconv.Atoi(aps.App.Spec.ALG.Icmp.Type)
				icode, _ := strconv.Atoi(aps.App.Spec.ALG.Icmp.Code)

				app.Spec.ALG.ICMP = &netproto.ICMP{
					Type: uint32(ictype),
					Code: uint32(icode),
				}
			}
		case security.ALG_DNS.String():
			if aps.App.Spec.ALG.Dns != nil {
				app.Spec.ALG.DNS = &netproto.DNS{
					DropMultiQuestionPackets: aps.App.Spec.ALG.Dns.DropMultiQuestionPackets,
					DropLargeDomainPackets:   aps.App.Spec.ALG.Dns.DropLargeDomainNamePackets,
					DropLongLabelPackets:     aps.App.Spec.ALG.Dns.DropLongLabelPackets,
					MaxMessageLength:         aps.App.Spec.ALG.Dns.MaxMessageLength,
					QueryResponseTimeout:     aps.App.Spec.ALG.Dns.QueryResponseTimeout,
				}
			} else {
				app.Spec.ALG.DNS = &netproto.DNS{}
			}
		case security.ALG_FTP.String():
			if aps.App.Spec.ALG.Ftp != nil {
				app.Spec.ALG.FTP = &netproto.FTP{
					AllowMismatchIPAddresses: aps.App.Spec.ALG.Ftp.AllowMismatchIPAddress,
				}
			} else {
				app.Spec.ALG.FTP = &netproto.FTP{}
			}
		case security.ALG_SunRPC.String():
			for _, sunrpc := range aps.App.Spec.ALG.Sunrpc {
				app.Spec.ALG.SUNRPC = append(app.Spec.ALG.SUNRPC,
					&netproto.RPC{
						ProgramID:        sunrpc.ProgramID,
						ProgramIDTimeout: sunrpc.Timeout,
					})
			}

			if len(app.Spec.ALG.SUNRPC) == 0 {
				app.Spec.ALG.SUNRPC = []*netproto.RPC{}
			}
		case security.ALG_MSRPC.String():
			for _, msrpc := range aps.App.Spec.ALG.Msrpc {
				app.Spec.ALG.MSRPC = append(app.Spec.ALG.MSRPC,
					&netproto.RPC{
						ProgramID:        msrpc.ProgramUUID,
						ProgramIDTimeout: msrpc.Timeout,
					})
			}
			if len(app.Spec.ALG.MSRPC) == 0 {
				app.Spec.ALG.MSRPC = []*netproto.RPC{}
			}
		case security.ALG_TFTP.String():
			app.Spec.ALG.TFTP = &netproto.TFTP{}
		case security.ALG_RTSP.String():
			app.Spec.ALG.RTSP = &netproto.RTSP{}
		}
	}

	app.Spec.ProtoPorts = getProtoPorts(&aps.App.App)
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
	app.App.Write()

	return nil
}

//GetAppWatchOptions gets options
func (sm *Statemgr) GetAppWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec"}
	return &opts
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
func (sm *Statemgr) OnAppUpdate(app *ctkit.App, napp *security.App) error {
	// see if anything changed
	_, ok := ref.ObjDiff(app.Spec, napp.Spec)
	if (napp.GenerationID == app.GenerationID) && !ok {
		app.ObjectMeta = napp.ObjectMeta
		return nil
	}
	app.ObjectMeta = napp.ObjectMeta
	app.Spec = napp.Spec

	aps, err := sm.FindApp(app.Tenant, app.Name)
	if err != nil {
		return err
	}

	// save the updated app
	sm.mbus.UpdateObjectWithReferences(app.MakeKey("security"), convertApp(aps), references(napp))

	return nil
}

// OnAppDelete handles app deletion
func (sm *Statemgr) OnAppDelete(app *ctkit.App) error {
	// see if we have the app
	fapp, err := AppStateFromObj(app)
	if err != nil {
		log.Errorf("Could not find the app %v. Err: %v", app, err)
		return err
	}
	log.Infof("Deleting app: %+v", fapp)

	// delete the object
	return sm.mbus.DeleteObjectWithReferences(app.MakeKey("security"), convertApp(fapp),
		references(app))
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

// OnAppReconnect is called when ctkit reconnects to apiserver
func (sm *Statemgr) OnAppReconnect() {
	return
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
