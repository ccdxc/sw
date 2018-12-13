// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"
	"time"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateApp creates a app
func (na *Nagent) CreateApp(app *netproto.App) error {
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

		log.Infof("Received duplicate app create for {%+v}", app)
		return nil
	}
	// Validate App Timeout is parseable.
	if len(app.Spec.AppTimeout) > 0 {
		err = validateTimeout(app.Spec.AppTimeout)
		if err != nil {
			log.Errorf("invalid AppTimeout duration %s", app.Spec.AppTimeout)
			return fmt.Errorf("invalid AppTimeout duration %s", app.Spec.AppTimeout)
		}
	}
	// Validate only one ALG is specified
	alg := app.Spec.ALG

	if alg != nil {
		if alg.DNS != nil {
			if len(alg.DNS.QueryResponseTimeout) > 0 {
				err = validateTimeout(alg.DNS.QueryResponseTimeout)
				if err != nil {
					log.Errorf("invalid QueryResponseTimeout format in SIP App. %v", alg.DNS.QueryResponseTimeout)
					return err
				}
			}
			algMapper = setBit(algMapper, 0)
		}
		if alg.ICMP != nil {
			algMapper = setBit(algMapper, 1)
		}
		if alg.FTP != nil {
			algMapper = setBit(algMapper, 2)
		}
		if alg.MSRPC != nil {
			err = validateTimeout(alg.MSRPC.MapEntryTimeout)
			if err != nil {
				log.Errorf("invalid map entry timeout format in MSRPC App. %v", alg.MSRPC.MapEntryTimeout)
				return err
			}
			algMapper = setBit(algMapper, 3)
		}
		if alg.RTSP != nil {
			algMapper = setBit(algMapper, 4)
		}
		if alg.SIP != nil {
			if len(alg.SIP.CTimeout) > 0 {
				err = validateTimeout(alg.SIP.CTimeout)
				if err != nil {
					log.Errorf("invalid cTimeout format in SIP App. %v", alg.SIP.CTimeout)
					return err
				}
			}

			if len(alg.SIP.MaxCallDuration) > 0 {
				err = validateTimeout(alg.SIP.MaxCallDuration)
				if err != nil {
					log.Errorf("invalid MaxCallDuration format in SIP App. %v", alg.SIP.MaxCallDuration)
					return err
				}
			}

			if len(alg.SIP.MediaInactivityTimeout) > 0 {
				err = validateTimeout(alg.SIP.MediaInactivityTimeout)
				if err != nil {
					log.Errorf("invalid MediaInactivityTimeout format in SIP App. %v", alg.SIP.MediaInactivityTimeout)
					return err
				}
			}

			if len(alg.SIP.T1Timeout) > 0 {
				err = validateTimeout(alg.SIP.T1Timeout)
				if err != nil {
					log.Errorf("invalid T1Timeout format in SIP App. %v", alg.SIP.T1Timeout)
					return err
				}
			}

			if len(alg.SIP.T4Timeout) > 0 {
				err = validateTimeout(alg.SIP.T4Timeout)
				if err != nil {
					log.Errorf("invalid T4Timeout format in SIP App. %v", alg.SIP.T4Timeout)
					return err
				}
			}
			algMapper = setBit(algMapper, 5)
		}

		if alg.SUNRPC != nil {
			err = validateTimeout(alg.SUNRPC.MapEntryTimeout)
			if err != nil {
				log.Errorf("invalid map entry timeout format in SunRPC App. %v", alg.SUNRPC.MapEntryTimeout)
				return err
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
	ns, err := na.FindNamespace(app.Tenant, app.Namespace)
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
	key := na.Solver.ObjectKey(app.ObjectMeta, app.TypeMeta)
	na.Lock()
	na.AppDB[key] = app
	na.Unlock()
	err = na.Store.Write(app)

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
	// find the corresponding namespace
	_, err := na.FindNamespace(app.Tenant, app.Namespace)
	if err != nil {
		return err
	}

	existingApp, err := na.FindApp(app.ObjectMeta)
	if err != nil {
		log.Errorf("App %v not found", app.ObjectMeta)
		return err
	}

	if proto.Equal(app, existingApp) {
		log.Infof("Nothing to update.")
		return nil
	}

	// Set the app ID from the existing app
	app.Status.AppID = existingApp.Status.AppID

	key := na.Solver.ObjectKey(app.ObjectMeta, app.TypeMeta)
	na.Lock()
	na.AppDB[key] = app
	na.Unlock()
	err = na.Store.Write(app)
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
	ns, err := na.FindNamespace(app.Tenant, app.Namespace)
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
	key := na.Solver.ObjectKey(app.ObjectMeta, app.TypeMeta)
	na.Lock()
	delete(na.AppDB, key)
	na.Unlock()
	err = na.Store.Delete(app)

	return err
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
