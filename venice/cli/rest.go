package vcli

import (
	"encoding/json"
	"fmt"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/venice/utils/runtime"
)

func restDelete(ctx *context) error {
	objName := ctx.names[0]
	hostName := ctx.cli.GlobalString("server")
	log.Debugf("put object: %s/%s", hostName, objName)

	obj := getNewObj(ctx)
	objm, err := runtime.GetObjectMeta(obj)
	if err != nil {
		return fmt.Errorf("Unable to fetch object meta from the object: %+v", obj)
	}
	objm.Name = objName
	objm.Tenant = ctx.tenant

	if err := ctx.restDeleteFunc(hostName, ctx.token, obj); err != nil {
		return err
	}

	if out, err := json.Marshal(obj); err != nil {
		fmt.Printf("Unable to marshal object %+v\n", obj)
	} else {
		fmt.Printf("%s", string(out))
	}

	return nil
}
