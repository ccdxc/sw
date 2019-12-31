// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateRoutingConfig creates a Routing Config
func (na *Nagent) CreateRoutingConfig(cfg *netproto.RoutingConfig) error {
	log.Infof("CreateRoutingConfig: %v", cfg)
	err := na.saveRoutingConfig(cfg)

	if err != nil {
		log.Error("CreateRoutingConfig: saveRoutingConfig failed, err: ", err)
		return err
	}
	return nil
}

// ListRoutingConfig returns the list of Routing configs
func (na *Nagent) ListRoutingConfig() []*netproto.RoutingConfig {
	log.Infof("ListRoutingConfig")
	var cfgList []*netproto.RoutingConfig

	// lock the db
	na.Lock()
	defer na.Unlock()

	// walk all Routing configs
	for _, cfg := range na.RoutingConfigDB {
		cfgList = append(cfgList, cfg)
	}

	return cfgList
}

// FindRoutingConfig finds a routing config in local db
func (na *Nagent) FindRoutingConfig(meta api.ObjectMeta) (*netproto.RoutingConfig, error) {
	log.Infof("FindRoutingConfig: %+v", meta)
	typeMeta := api.TypeMeta{
		Kind: "RoutingConfig",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	cfg, ok := na.RoutingConfigDB[key]
	if !ok {
		return nil, fmt.Errorf("RoutingConfig not found %v", meta.Name)
	}

	return cfg, nil
}

// UpdateRoutingConfig updates a RoutingConfig
func (na *Nagent) UpdateRoutingConfig(cfg *netproto.RoutingConfig) error {
	log.Infof("UpdateRoutingConfig: %+v", cfg)

	exisitingCfg, err := na.FindRoutingConfig(cfg.ObjectMeta)
	if err != nil {
		log.Errorf("RoutingConfig %v not found", cfg.ObjectMeta)
		return err
	}

	if proto.Equal(&cfg.Spec, &exisitingCfg.Spec) {
		return nil
	}

	return na.saveRoutingConfig(cfg)
}

// DeleteRoutingConfig deletes a Routing Config
func (na *Nagent) DeleteRoutingConfig(tn, namespace, name string) error {
	log.Infof("DeleteRoutingConfig: %s, %s, %s", tn, namespace, name)
	cfg := &netproto.RoutingConfig{
		TypeMeta: api.TypeMeta{Kind: "RoutingConfig"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(cfg.Kind, cfg.ObjectMeta)
	if err != nil {
		log.Error("validateMeta failed: ", cfg.ObjectMeta)
		return errors.New("Invalid ObjectMeta")
	}

	// check if the routing config exists
	_, err = na.FindRoutingConfig(cfg.ObjectMeta)
	if err != nil {
		log.Errorf("RoutingConfig %+v not found", cfg.ObjectMeta)
		return errors.New("RoutingConfig not found")
	}

	// delete from db
	return na.discardRoutingConfig(cfg)
}

// saveRoutingConfig saves the routing config in state stores
func (na *Nagent) saveRoutingConfig(cfg *netproto.RoutingConfig) error {
	// save it in db
	key := na.Solver.ObjectKey(cfg.ObjectMeta, cfg.TypeMeta)
	na.Lock()
	na.RoutingConfigDB[key] = cfg
	na.Unlock()

	// write to emstore
	dat, _ := cfg.Marshal()
	return na.Store.RawWrite(cfg.GetKind(), cfg.GetKey(), dat)
}

// discardRoutingConfig removes the routing config from state stores
func (na *Nagent) discardRoutingConfig(cfg *netproto.RoutingConfig) error {
	// delete from db
	key := na.Solver.ObjectKey(cfg.ObjectMeta, cfg.TypeMeta)
	na.Lock()
	delete(na.RoutingConfigDB, key)
	na.Unlock()

	// delete from emstore
	return na.Store.RawDelete(cfg.GetKind(), cfg.GetKey())
}
