package genfields

var msgFieldMaps = map[string][]string{}

// GetFields get field list from msgFieldMaps based on input string
func GetFields(k string) ([]string, bool) {
	v, ok := msgFieldMaps[k]
	return v, ok
}

// GetAllKeys get all messages from msgFieldMaps
func GetAllKeys() []string {
	keys := []string{}
	for k := range msgFieldMaps {
		keys = append(keys, k)
	}
	return keys
}
