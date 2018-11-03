package ntranslate

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
)

var tstr *Translator

// Translator holds the mapping of kind and translator functions
type Translator struct {
	kindToTranslatorFn map[string]TranslatorFns
}

// MustGetTranslator returns the translator
func MustGetTranslator() *Translator {
	if tstr != nil {
		return tstr
	}

	tstr = &Translator{
		kindToTranslatorFn: map[string]TranslatorFns{},
	}

	return tstr
}

// Register registers translator functions against the given kind
func (t *Translator) Register(kind string, tstrFns TranslatorFns) {
	if _, ok := t.kindToTranslatorFn[kind]; ok {
		log.Debugf("overwriting translator functions for kind: %s", kind)
	}

	t.kindToTranslatorFn[kind] = tstrFns
}

// GetObjectMeta converts and returns given key to meta
func (t *Translator) GetObjectMeta(kind string, key interface{}) *api.ObjectMeta {
	if fns, ok := t.kindToTranslatorFn[kind]; ok && fns != nil {
		return fns.KeyToMeta(key)
	}

	return nil
}

// GetKey converts and returns given meta to key
func (t *Translator) GetKey(kind string, meta *api.ObjectMeta) interface{} {
	if fns, ok := t.kindToTranslatorFn[kind]; ok && fns != nil {
		return fns.MetaToKey(meta)
	}

	return nil
}
