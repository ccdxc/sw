package genfields

// mapping of group to classes to properties
var globalMetricsMap = map[string]map[string][]string{}

// mapping of class to list of fields
var kindToFieldNameMap = map[string][]string{}

// GetFieldNamesFromKind returns a list of all fields belonging to the given class
func GetFieldNamesFromKind(k string) []string {
	return kindToFieldNameMap[k]
}

// GetAllFieldNames returns the union of all fields across different classes
func GetAllFieldNames() []string {
	var properties []string
	for property := range kindToFieldNameMap {
		properties = append(properties, property)
	}
	return properties
}

// IsGroupValid returns true if the given group is available on the global metrics map
func IsGroupValid(group string) bool {
	_, found := globalMetricsMap[group]
	return found
}

// IsKindValid returns true when there is a group->class mapping available on the global metrics map
func IsKindValid(group, class string) bool {
	if classes, found := globalMetricsMap[group]; found {
		_, found := classes[class]
		return found
	}

	return false
}

// IsFieldNameValid returns true when there is group->class->property mapping available on the global metrics map
func IsFieldNameValid(group, class, property string) bool {
	if classes, found := globalMetricsMap[group]; found {
		if fields, found := classes[class]; found {
			for _, field := range fields {
				if field == property {
					return true
				}
			}
		}
	}

	return false
}
