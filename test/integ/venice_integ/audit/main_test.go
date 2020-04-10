package audit

import (
	_ "github.com/pensando/sw/api/generated/audit"
	. "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	testUser     = "test"
	testPassword = TestLocalPassword
	testTenant   = "testtenant"
)

var (
	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("audit_integ_test",
		log.GetNewLogger(log.GetDefaultConfig("audit_integ_test"))))
)
