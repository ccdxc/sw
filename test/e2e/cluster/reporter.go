package cluster

import (
	"fmt"
	"os"
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
	fspec := []string{}

	w := new(tabwriter.Writer)
	// Format in tab-separated columns with a tab stop of 8.
	w.Init(os.Stdout, 0, 8, 1, '\t', 0)
	fmt.Fprintf(w, "\nTest summary: \n")

	i := 1
	for k, s := range s.spec {
		fmt.Fprintf(w, "%d\t%v\t%v\n", i, k, s)
		i++

		if s == "FAIL" {
			fspec = append(fspec, k)
		}
	}

	if len(fspec) > 0 {
		fmt.Fprintf(w, "\nFailed tests:\n")
		for i, s := range fspec {
			fmt.Fprintf(w, "%d\t%v\t%v\n", i+1, s, "FAIL")
		}
	}
	w.Flush()
}
