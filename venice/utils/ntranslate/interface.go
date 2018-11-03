package ntranslate

import "github.com/pensando/sw/api"

// TranslatorFns interface representing the list of translator functions
type TranslatorFns interface {
	// converts key to venice meta
	KeyToMeta(key interface{}) *api.ObjectMeta

	// converts venice meta to key
	MetaToKey(meta *api.ObjectMeta) interface{}
}

// NameTranslator interface responsible for converting
// key to meta and meta to key.
type NameTranslator interface {
	// Register registers the translator functions for given kind.
	Register(kind string, tstr TranslatorFns)

	// GetObjectMeta converts the given kind and key to object metadata.
	// Internally, it will make use of the translate func of respective kind.
	GetObjectMeta(kind string, key interface{}) *api.ObjectMeta

	// GetKey converts given kind and meta to key.
	// Internally, it will make use of the translate func of respective kind.
	GetKey(kind string, meta *api.ObjectMeta) interface{}
}
