package pkg

import (
	"encoding/json"
	"io/ioutil"

	"github.com/pkg/errors"
)

//StationDevice created on host side.
type StationDevice struct {
	LifID   int
	MacAddr string
}

//ReadStationDevices read device json file
func ReadStationDevices(deviceFile string) ([]StationDevice, error) {

	file, e := ioutil.ReadFile(deviceFile)
	if e != nil {
		return nil, errors.Wrap(e, "Error opening device Json")
	}

	var deviceJSON map[string]interface{}
	var err error
	err = json.Unmarshal(file, &deviceJSON)
	if err != nil {
		return nil, errors.Wrap(err, "Error in unmarshalling json file")
	}

	var sdevices []StationDevice
	ethDevices := deviceJSON["eth_dev"].([]interface{})
	for _, data := range ethDevices {
		devData := data.(map[string]interface{})
		sdevice := StationDevice{MacAddr: devData["mac_addr"].(string)}
		sdevices = append(sdevices, sdevice)
	}

	return sdevices, nil
}
