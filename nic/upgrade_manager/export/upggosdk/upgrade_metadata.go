package upggosdk

import (
	"encoding/json"
	"io/ioutil"
	"os"

	"github.com/pensando/sw/venice/utils/log"
)

type upgMeta struct {
	Ver string `json:"version"`
}

func getUpgCtxFromMeta(upgCtx *UpgCtx) error {
	jsonFile, err := os.Open("/sw/nic/upgrade_manager/meta/upgrade_metadata.json")
	if err != nil {
		log.Infof("Error %s", err)
		return err
	}
	defer jsonFile.Close()
	byteValue, _ := ioutil.ReadAll(jsonFile)

	var meta upgMeta
	err = json.Unmarshal(byteValue, &meta)
	if err != nil {
		log.Infof("Unable to unmarshal the json file %s", err)
		return err
	}
	upgCtx.fromVer = meta.Ver
	return nil
}
