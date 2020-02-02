// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package enterprise

import (
	//"fmt"
	//"strconv"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
)

// NewMirrorSession creates a new mirrorsession
func (sm *SysModel) NewMirrorSession(name string) *objects.MirrorSessionCollection {
	return objects.NewMirrorSession(name, sm.ObjClient(), sm.Tb)
}

// MirrorSessions returns all Mirror Sessions in the model
func (sm *SysModel) MirrorSessions() *objects.MirrorSessionCollection {
	msc := objects.MirrorSessionCollection{}
	for _, sess := range msc.Sessions {
		msc.Sessions = append(msc.Sessions, sess)
	}

	return &msc
}
