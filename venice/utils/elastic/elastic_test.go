package elastic

import (
	"testing"
	"time"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func SkipTestElasticGetIndexForFwLogs(t *testing.T) {
	temp := time.Date(2020, time.January, 1, 0, 0, 0, 0, time.UTC)
	output := getClockHourTimeForIndex(temp, 6)
	Assert(t, output == "2020-01-01T00:00:00", "output is not correct")

	temp = time.Date(2020, time.January, 1, 7, 0, 0, 0, time.UTC)
	output = getClockHourTimeForIndex(temp, 6)
	Assert(t, output == "2020-01-01T06:00:00", "output is not correct")

	temp = time.Date(2020, time.January, 1, 14, 0, 0, 0, time.UTC)
	output = getClockHourTimeForIndex(temp, 6)
	Assert(t, output == "2020-01-01T12:00:00", "output is not correct")

	temp = time.Date(2020, time.January, 1, 19, 0, 0, 0, time.UTC)
	output = getClockHourTimeForIndex(temp, 6)
	Assert(t, output == "2020-01-01T18:00:00", "output is not correct")

	temp = time.Date(2020, time.January, 1, 23, 0, 0, 0, time.UTC)
	output = getClockHourTimeForIndex(temp, 6)
	Assert(t, output == "2020-01-01T18:00:00", "output is not correct")

	temp = time.Date(2020, time.January, 1, 28, 0, 0, 0, time.UTC)
	output = getClockHourTimeForIndex(temp, 6)
	Assert(t, output == "2020-01-02T00:00:00", "output is not correct")
}
