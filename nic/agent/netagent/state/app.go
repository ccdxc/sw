// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"
	"strings"
	"time"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	dnetproto "github.com/pensando/sw/nic/agent/protos/generated/delphi/netproto/delphi"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateApp creates a app
func (na *Nagent) CreateApp(app *netproto.App) error {
	log.Infof("Got App Create: %v", app)
	var algMapper int
	err := na.validateMeta(app.Kind, app.ObjectMeta)
	if err != nil {
		return err
	}
	// check if app already exists
	oldApp, err := na.FindApp(app.ObjectMeta)
	if err == nil {
		// check if app contents are same
		if !proto.Equal(oldApp, app) {
			log.Errorf("App %+v already exists", oldApp)
			return errors.New("app already exists")
		}

		log.Infof("Received duplicate app create for {%+v}", app.ObjectMeta)
		return nil
	}
	// Validate App Idle Timeout is parseable.
	if len(app.Spec.AppIdleTimeout) > 0 {
		err = validateTimeout(app.Spec.AppIdleTimeout)
		if err != nil {
			log.Errorf("invalid AppIdleTimeout duration %s", app.Spec.AppIdleTimeout)
			return fmt.Errorf("invalid AppIdleTimeout duration %s", app.Spec.AppIdleTimeout)
		}
	}
	// Validate only one ALG is specified
	alg := app.Spec.ALG

	if alg != nil {
		if alg.DNS != nil {
			if len(alg.DNS.QueryResponseTimeout) > 0 {
				if err := validateTimeout(alg.DNS.QueryResponseTimeout); err != nil {
					log.Errorf("invalid QueryResponseTimeout format in SIP App. %v", alg.DNS.QueryResponseTimeout)
					return err
				}
			}
			algMapper = setBit(algMapper, 0)
		}
		if alg.ICMP != nil {
			// TODO Validate ICMP Code and Type Ranges here
			for _, protoPort := range app.Spec.ProtoPorts {
				if strings.TrimSpace(protoPort.Protocol) != "icmp" {
					log.Errorf("icmp app must specify only the protocol and it must be icmp. Found: %v", app.Spec.ProtoPorts)
					return fmt.Errorf("icmp app must specify only the protocol and it must be icmp. Found: %v", app.Spec.ProtoPorts)
				}

				if len(protoPort.Port) != 0 {
					log.Errorf("icmp app must not specify port. Found: %v", protoPort.Port)
					return fmt.Errorf("icmp app must not specify port. Found: %v", protoPort.Port)
				}
			}

			algMapper = setBit(algMapper, 1)
		}
		if alg.FTP != nil {
			algMapper = setBit(algMapper, 2)
		}
		if alg.MSRPC != nil {
			for _, r := range alg.MSRPC {
				if err := validateTimeout(r.ProgramIDTimeout); err != nil {
					log.Errorf("invalid program ID timeout %v. Err: %v", r.ProgramIDTimeout, err)
					return err
				}
			}
			algMapper = setBit(algMapper, 3)
		}
		if alg.RTSP != nil {
			algMapper = setBit(algMapper, 4)
		}
		if alg.SIP != nil {
			if len(alg.SIP.CTimeout) > 0 {
				if err := validateTimeout(alg.SIP.CTimeout); err != nil {
					log.Errorf("invalid cTimeout format in SIP App. %v", alg.SIP.CTimeout)
					return err
				}
			}

			if len(alg.SIP.MaxCallDuration) > 0 {
				if err := validateTimeout(alg.SIP.MaxCallDuration); err != nil {
					log.Errorf("invalid MaxCallDuration format in SIP App. %v", alg.SIP.MaxCallDuration)
					return err
				}
			}

			if len(alg.SIP.MediaInactivityTimeout) > 0 {
				if err := validateTimeout(alg.SIP.MediaInactivityTimeout); err != nil {
					log.Errorf("invalid MediaInactivityTimeout format in SIP App. %v", alg.SIP.MediaInactivityTimeout)
					return err
				}
			}

			if len(alg.SIP.T1Timeout) > 0 {
				if err := validateTimeout(alg.SIP.T1Timeout); err != nil {
					log.Errorf("invalid T1Timeout format in SIP App. %v", alg.SIP.T1Timeout)
					return err
				}
			}

			if len(alg.SIP.T4Timeout) > 0 {
				if err := validateTimeout(alg.SIP.T4Timeout); err != nil {
					log.Errorf("invalid T4Timeout format in SIP App. %v", alg.SIP.T4Timeout)
					return err
				}
			}
			algMapper = setBit(algMapper, 5)
		}

		if alg.SUNRPC != nil {
			for _, r := range alg.SUNRPC {
				if err := validateTimeout(r.ProgramIDTimeout); err != nil {
					log.Errorf("invalid program ID timeout %v. Err: %v", r.ProgramIDTimeout, err)
					return err
				}
			}
			algMapper = setBit(algMapper, 6)
		}
		if alg.TFTP != nil {
			algMapper = setBit(algMapper, 7)
		}
	}

	// Only one of the bits in algMapper should be 1 if there is more than one field set, then we reject the config, as
	// don't support multiple ALG configurations in the same object

	if (algMapper & -algMapper) != algMapper {
		log.Errorf("Multiple ALG configurations specified in a single app object. %v", app)
		return fmt.Errorf("multiple ALG configurations specified in a single app object. %v", app)
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(app.ObjectMeta)
	if err != nil {
		return err
	}

	// Add the current app as a dependency to the namespace.
	err = na.Solver.Add(ns, app)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, app)
		return err
	}

	// save it in db
	return na.saveApp(app)
}

// saveApp saves the app in state stores
func (na *Nagent) saveApp(app *netproto.App) error {
	// save it in db
	key := na.Solver.ObjectKey(app.ObjectMeta, app.TypeMeta)
	na.Lock()
	na.AppDB[key] = app
	na.Unlock()

	// write to delphi
	if na.DelphiClient != nil {
		dapp := dnetproto.App{
			Key: key,
			App: app,
		}

		err := na.DelphiClient.SetObject(&dapp)
		if err != nil {
			log.Errorf("Error writing App %s to delphi. Err: %v", key, err)
			return err
		}
	}

	// write to emstore
	dat, _ := app.Marshal()
	err := na.Store.RawWrite(app.GetKind(), app.GetKey(), dat)
	return err
}

// discardApp removes the app from state stores
func (na *Nagent) discardApp(app *netproto.App) error {
	// delete from db
	key := na.Solver.ObjectKey(app.ObjectMeta, app.TypeMeta)
	na.Lock()
	delete(na.AppDB, key)
	na.Unlock()

	// remove from delphi
	if na.DelphiClient != nil {
		dapp := dnetproto.App{
			Key: key,
			App: app,
		}

		err := na.DelphiClient.DeleteObject(&dapp)
		if err != nil {
			log.Errorf("Error writing App %s to delphi. Err: %v", key, err)
			return err
		}
	}

	err := na.Store.RawDelete(app.GetKind(), app.GetKey())

	return err
}

// ListApp returns the list of apps
func (na *Nagent) ListApp() []*netproto.App {
	var appList []*netproto.App

	// lock the db
	na.Lock()
	defer na.Unlock()

	// walk all apps
	for _, rt := range na.AppDB {
		appList = append(appList, rt)
	}

	return appList
}

// FindApp dins a app in local db
func (na *Nagent) FindApp(meta api.ObjectMeta) (*netproto.App, error) {
	typeMeta := api.TypeMeta{
		Kind: "App",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	rt, ok := na.AppDB[key]
	if !ok {
		return nil, fmt.Errorf("app not found %v", meta.Name)
	}

	return rt, nil
}

// UpdateApp updates a app.
func (na *Nagent) UpdateApp(app *netproto.App) error {
	log.Infof("Got App Update: %v", app)

	var linkedSGPolicies []*netproto.NetworkSecurityPolicy
	var algMapper int

	// find the corresponding namespace
	_, err := na.FindNamespace(app.ObjectMeta)
	if err != nil {
		return err
	}

	existingApp, err := na.FindApp(app.ObjectMeta)
	if err != nil {
		log.Errorf("App %v not found", app.ObjectMeta)
		return err
	}

	if proto.Equal(&app.Spec, &existingApp.Spec) {
		return nil
	}

	// Set the app ID from the existing app
	app.Status.AppID = existingApp.Status.AppID

	// Validate App Idle Timeout is parseable.
	if len(app.Spec.AppIdleTimeout) > 0 {
		err = validateTimeout(app.Spec.AppIdleTimeout)
		if err != nil {
			log.Errorf("invalid AppIdleTimeout duration %s", app.Spec.AppIdleTimeout)
			return fmt.Errorf("invalid AppIdleTimeout duration %s", app.Spec.AppIdleTimeout)
		}
	}
	// Validate only one ALG is specified
	alg := app.Spec.ALG

	if alg != nil {
		if alg.DNS != nil {
			if len(alg.DNS.QueryResponseTimeout) > 0 {
				if err := validateTimeout(alg.DNS.QueryResponseTimeout); err != nil {
					log.Errorf("invalid QueryResponseTimeout format in SIP App. %v", alg.DNS.QueryResponseTimeout)
					return err
				}
			}
			algMapper = setBit(algMapper, 0)
		}
		if alg.ICMP != nil {
			// TODO Validate ICMP Code and Type Ranges here
			for _, protoPort := range app.Spec.ProtoPorts {
				if strings.TrimSpace(protoPort.Protocol) != "icmp" {
					log.Errorf("icmp app must specify only the protocol and it must be icmp. Found: %v", app.Spec.ProtoPorts)
					return fmt.Errorf("icmp app must specify only the protocol and it must be icmp. Found: %v", app.Spec.ProtoPorts)
				}
			}

			algMapper = setBit(algMapper, 1)
		}
		if alg.FTP != nil {
			algMapper = setBit(algMapper, 2)
		}
		if alg.MSRPC != nil {
			for _, r := range alg.MSRPC {
				if err := validateTimeout(r.ProgramIDTimeout); err != nil {
					log.Errorf("invalid program ID timeout %v. Err: %v", r.ProgramIDTimeout, err)
					return err
				}
			}
			algMapper = setBit(algMapper, 3)
		}
		if alg.RTSP != nil {
			algMapper = setBit(algMapper, 4)
		}
		if alg.SIP != nil {
			if len(alg.SIP.CTimeout) > 0 {
				if err := validateTimeout(alg.SIP.CTimeout); err != nil {
					log.Errorf("invalid cTimeout format in SIP App. %v", alg.SIP.CTimeout)
					return err
				}
			}

			if len(alg.SIP.MaxCallDuration) > 0 {
				if err := validateTimeout(alg.SIP.MaxCallDuration); err != nil {
					log.Errorf("invalid MaxCallDuration format in SIP App. %v", alg.SIP.MaxCallDuration)
					return err
				}
			}

			if len(alg.SIP.MediaInactivityTimeout) > 0 {
				if err := validateTimeout(alg.SIP.MediaInactivityTimeout); err != nil {
					log.Errorf("invalid MediaInactivityTimeout format in SIP App. %v", alg.SIP.MediaInactivityTimeout)
					return err
				}
			}

			if len(alg.SIP.T1Timeout) > 0 {
				if err := validateTimeout(alg.SIP.T1Timeout); err != nil {
					log.Errorf("invalid T1Timeout format in SIP App. %v", alg.SIP.T1Timeout)
					return err
				}
			}

			if len(alg.SIP.T4Timeout) > 0 {
				if err := validateTimeout(alg.SIP.T4Timeout); err != nil {
					log.Errorf("invalid T4Timeout format in SIP App. %v", alg.SIP.T4Timeout)
					return err
				}
			}
			algMapper = setBit(algMapper, 5)
		}

		if alg.SUNRPC != nil {
			for _, r := range alg.SUNRPC {
				if err := validateTimeout(r.ProgramIDTimeout); err != nil {
					log.Errorf("invalid program ID timeout %v. Err: %v", r.ProgramIDTimeout, err)
					return err
				}
			}
			algMapper = setBit(algMapper, 6)
		}
		if alg.TFTP != nil {
			algMapper = setBit(algMapper, 7)
		}
	}

	// Only one of the bits in algMapper should be 1 if there is more than one field set, then we reject the config, as
	// don't support multiple ALG configurations in the same object

	if (algMapper & -algMapper) != algMapper {
		log.Errorf("Multiple ALG configurations specified in a single app object. %v", app)
		return fmt.Errorf("multiple ALG configurations specified in a single app object. %v", app)
	}

	err = na.saveApp(app)

	// Find the sg policies that are currently referring to this App and trigger an NetworkSecurityPolicy Update
	for _, sgp := range na.ListNetworkSecurityPolicy() {
		for _, r := range sgp.Spec.Rules {
			if r.AppName == app.Name {
				linkedSGPolicies = append(linkedSGPolicies, sgp)
				continue
			}
		}
	}

	for _, sgp := range linkedSGPolicies {
		if err := na.performNetworkSecurityPolicyUpdate(sgp); err != nil {
			log.Errorf("Failed to update the corresponding SG Policy. Err: %v", err)
			return err
		}
	}

	return err
}

// DeleteApp deletes a app. ToDo implement app deletes in datapath
func (na *Nagent) DeleteApp(tn, namespace, name string) error {
	app := &netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(app.Kind, app.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(app.ObjectMeta)
	if err != nil {
		return err
	}

	// check if app already exists
	existingApp, err := na.FindApp(app.ObjectMeta)
	if err != nil {
		log.Errorf("App %+v not found", app.ObjectMeta)
		return errors.New("app not found")
	}

	// update parent references
	err = na.Solver.Remove(ns, existingApp)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
		return err
	}

	// delete from db
	return na.discardApp(app)
}

// setBit sets a particular bit position to 1. This is used for ALG Mapper
func setBit(n int, pos uint) int {
	n |= (1 << pos)
	return n
}

func validateTimeout(timeout string) error {
	_, err := time.ParseDuration(timeout)
	return err
}
