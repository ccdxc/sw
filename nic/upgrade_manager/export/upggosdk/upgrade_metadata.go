package upggosdk

import (
	"encoding/json"
	"io/ioutil"
	"os"

	"github.com/pensando/sw/venice/utils/log"
)

type table struct {
	Version int    `json:"version"`
	Name    string `json:"name"`
}

type upgMeta struct {
	Tables []table `json:"tables"`
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
	log.Infof("Found %d tables in metadata.json file", len(meta.Tables))
	if upgCtx.PreUpgTables == nil {
		log.Info("PreUpgTables is not initialized. doing it now")
		upgCtx.PreUpgTables = make(map[string]TableMeta)
	}
	for i := 0; i < len(meta.Tables); i++ {
		log.Infof("version %d name %s", meta.Tables[i].Version, meta.Tables[i].Name)
		upgCtx.PreUpgTables[meta.Tables[i].Name] = TableMeta{
			meta.Tables[i].Version,
			meta.Tables[i].Name,
		}
	}
	return nil
}
