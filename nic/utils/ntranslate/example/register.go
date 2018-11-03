package example

import (
	"github.com/pensando/sw/venice/utils/ntranslate"
)

func init() {
	tstr := ntranslate.MustGetTranslator()

	tstr.Register("NetworkKey", &networkTranslatorFns{})
	tstr.Register("SecurityPolicyKey", nil)
}
