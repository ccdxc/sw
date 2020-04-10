package fwlog

import (
	"context"
	"fmt"

	_ "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/fwlog"
	. "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	mapper "github.com/pensando/sw/venice/utils/elastic/mapper"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	testUser     = "test"
	testPassword = TestLocalPassword
)

var (
	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("fwlog_integ_test",
		log.GetNewLogger(log.GetDefaultConfig("fwlog_integ_test"))))
)

// createFwLogsElasticIndex helper function to create index template for fw logs.
func createFwLogsElasticIndex(t *TestInfo) error {
	docType := elastic.GetDocType(globals.FwLogs)
	mapping, err := mapper.ElasticMapper(fwlog.FwLog{},
		docType,
		mapper.WithShardCount(3),
		mapper.WithReplicaCount(1),
		mapper.WithMaxInnerResults(globals.SpyglassMaxResults),
		mapper.WithIndexPatterns(fmt.Sprintf("*.%s.*", docType)),
		mapper.WithCharFilter())
	if err != nil {
		return err
	}
	// JSON string mapping
	strMapping, err := mapping.JSONString()
	if err != nil {
		return err
	}
	t.Logger.Infof("fw log elastic mapping: %v", strMapping)
	// create fwlog index
	if err := t.ESClient.CreateIndex(context.Background(), elastic.GetIndex(globals.FwLogs, globals.DefaultTenant), strMapping); err != nil {
		t.Logger.Errorf("failed to create fw logs index template, err: %v", err)
		return err
	}
	return nil
}
