package cloud

import (
	"fmt"
	"os"
	"sort"
	"strings"
	"text/tabwriter"

	"github.com/onsi/ginkgo/config"
	"github.com/onsi/ginkgo/reporters"
	"github.com/onsi/ginkgo/types"
)

type summaryReporter struct {
	spec map[string]string
}

// NewReporter create new reporter
func NewReporter() reporters.Reporter {
	return &summaryReporter{spec: map[string]string{}}
}

// SpecSuiteWillBegin callback
func (s *summaryReporter) SpecSuiteWillBegin(config config.GinkgoConfigType, summary *types.SuiteSummary) {
}

// BeforeSuiteDidRun callback
func (s *summaryReporter) BeforeSuiteDidRun(setupSummary *types.SetupSummary) {}

// SpecWillRun callback
func (s *summaryReporter) SpecWillRun(specSummary *types.SpecSummary) {}

// SpecDidComplete callback
func (s *summaryReporter) SpecDidComplete(specSummary *types.SpecSummary) {
	spec := strings.Join(specSummary.ComponentTexts[1:], "/")
	if specSummary.HasFailureState() {
		s.spec[spec] = "FAIL"
	} else if specSummary.Skipped() {
		s.spec[spec] = "SKIP"
	} else {
		s.spec[spec] = "PASS"
	}
}

// AfterSuiteDidRun callback
func (s *summaryReporter) AfterSuiteDidRun(setupSummary *types.SetupSummary) {}

// SpecSuiteDidEnd callback
func (s *summaryReporter) SpecSuiteDidEnd(summary *types.SuiteSummary) {

	keys := []string{}
	for k := range s.spec {
		keys = append(keys, k)
	}
	sort.Strings(keys)

	w := new(tabwriter.Writer)
	// Format in tab-separated columns with a tab stop of 8.
	w.Init(os.Stdout, 0, 8, 1, '\t', 0)
	fmt.Fprintf(w, "\n%v%vTest summary:%v \n", "\x1b[32m", "\x1b[1m", "\x1b[0m")

	cmap := map[string]string{
		"PASS": "\x1b[32m",
		"SKIP": "\x1b[36m",
		"FAIL": "\x1b[91m",
	}

	for i, k := range keys {
		fmt.Fprintf(w, "%v%d\t%v\t%v%v\n", cmap[s.spec[k]], i+1, k, s.spec[k], "\x1b[0m")
	}

	w.Flush()
}
