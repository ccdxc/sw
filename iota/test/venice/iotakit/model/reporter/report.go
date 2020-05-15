package reporter

import (
	"fmt"
	"os"
	"time"

	"github.com/fatih/color"
	"github.com/olekukonko/tablewriter"
	ginkgo "github.com/onsi/ginkgo"
	"github.com/onsi/ginkgo/config"
	"github.com/onsi/ginkgo/types"
)

type modelReporter struct {
	testResult   map[string]bool            // test result
	taskResult   map[string]error           // sub task result
	caseResult   map[string]*TestCaseResult // test case result counts
	bundleResult map[string]*TestBundleResult
	callbacks    Callbacks
}

// TestCaseResult stores test case results
type TestCaseResult struct {
	failCount int
	passCount int
	skipCount int
	duration  time.Duration
}

// TestGroupResult stores test case results
type TestGroupResult struct {
	duration  time.Duration
	failCount int
	passCount int
	skipCount int
	results   map[string]*TestCaseResult
}

// TestBundleResult stores test case results
type TestBundleResult struct {
	duration  time.Duration
	failCount int
	passCount int
	skipCount int
	results   map[string]*TestGroupResult
}

//Reporter reporter interface
type Reporter interface {
	ginkgo.Reporter
	PrintReport()
}

//NewReporter new instance of reporter
func NewReporter(cb Callbacks) Reporter {
	rep := modelReporter{callbacks: cb}

	rep.testResult = make(map[string]bool)
	rep.taskResult = make(map[string]error)
	rep.caseResult = make(map[string]*TestCaseResult)
	rep.bundleResult = make(map[string]*TestBundleResult)
	return &rep
}

func (sm *modelReporter) SpecSuiteWillBegin(config config.GinkgoConfigType, summary *types.SuiteSummary) {

}
func (sm *modelReporter) BeforeSuiteDidRun(setupSummary *types.SetupSummary) {

}

const defaultStyle = "\x1b[0m"
const boldStyle = "\x1b[1m"
const redColor = "\x1b[91m"
const greenColor = "\x1b[32m"
const yellowColor = "\x1b[33m"
const cyanColor = "\x1b[36m"
const grayColor = "\x1b[90m"
const lightGrayColor = "\x1b[37m"

func (sm *modelReporter) SpecWillRun(specSummary *types.SpecSummary) {
	var testcaseDescr string
	bundleName := specSummary.ComponentTexts[1]
	groupName := specSummary.ComponentTexts[2]
	if len(specSummary.ComponentTexts) > 3 {
		testcaseDescr = specSummary.ComponentTexts[3]
	}
	if !specSummary.Skipped() {
		fmt.Printf("%sRunning Test :%v:%v:%v%s\n", boldStyle, color.GreenString(bundleName), groupName, testcaseDescr, defaultStyle)
	}

	sm.callbacks.BeforeTestCallback()

}

func (sm *modelReporter) SpecDidComplete(specSummary *types.SpecSummary) {
	var testcaseDescr string
	bundleName := specSummary.ComponentTexts[1]
	groupName := specSummary.ComponentTexts[2]

	if len(specSummary.ComponentTexts) > 3 {
		testcaseDescr = specSummary.ComponentTexts[3]
	}

	if !specSummary.Skipped() {
		resultColor := greenColor
		resultString := "PASS"
		if specSummary.Failed() {
			resultColor = redColor
			resultString = "FAIL"
			fmt.Printf("%s%s%s\n", redColor, "Failure", defaultStyle)
			fmt.Printf("\t%v\n", specSummary.Failure.ComponentCodeLocation.String())
			fmt.Printf("%s%s%s\n", redColor, specSummary.Failure.Message, defaultStyle)
			fmt.Printf("\t%v\n", specSummary.Failure.Location.String())
			sm.callbacks.FailTest(bundleName, groupName, testcaseDescr)
		} else if specSummary.Panicked() {
			resultString = "FAIL"
			fmt.Printf("%s%s%s\n", redColor, "Panicked", defaultStyle)
			fmt.Printf("%s%s%s\n", redColor, specSummary.Failure.ForwardedPanic, defaultStyle)
			fmt.Printf("\t%v\n", specSummary.Failure.Location.String())
			fmt.Printf("%s%s%s\n", redColor, "Full Stack Trace", defaultStyle)
			fmt.Printf("\t%v\n", specSummary.Failure.Location.FullStackTrace)
		}
		fmt.Printf("%sCompleted Test :%v:%v:%v:Result:%s%v%s\n", boldStyle, color.GreenString(bundleName),
			groupName, testcaseDescr, resultColor, resultString, defaultStyle)
	} else {
		fmt.Printf("%sSkipped Test :%v:%v:%v %s\n", boldStyle, color.GreenString(bundleName), groupName, testcaseDescr, defaultStyle)
	}

	bundleResult, ok := sm.bundleResult[bundleName]
	if !ok {
		sm.bundleResult[bundleName] = &TestBundleResult{}
		bundleResult = sm.bundleResult[bundleName]
		bundleResult.results = make(map[string]*TestGroupResult)
	}

	groupResult, ok := bundleResult.results[groupName]
	if !ok {
		bundleResult.results[groupName] = &TestGroupResult{}
		groupResult = bundleResult.results[groupName]
		groupResult.results = make(map[string]*TestCaseResult)
	}

	testcaseResult, ok := groupResult.results[testcaseDescr]
	if !ok {
		groupResult.results[testcaseDescr] = &TestCaseResult{}
		testcaseResult = groupResult.results[testcaseDescr]
	}

	testcaseResult.duration = specSummary.RunTime
	groupResult.duration += specSummary.RunTime
	bundleResult.duration += specSummary.RunTime
	if !specSummary.Skipped() {
		if specSummary.Failed() {
			testcaseResult.failCount++
			groupResult.failCount++
			bundleResult.failCount++
		} else {
			testcaseResult.passCount++
			groupResult.passCount++
			bundleResult.passCount++
		}
		sm.callbacks.AfterTestCallback()
	} else {
		testcaseResult.skipCount++
		groupResult.skipCount++
		bundleResult.skipCount++
	}

}

//Callbacks callbacks for reporter
type Callbacks interface {
	BeforeTestCallback()
	AfterTestCallback()
	FailTest(bundle, group, tcName string)
}

func (sm *modelReporter) AfterSuiteDidRun(setupSummary *types.SetupSummary) {
}
func (sm *modelReporter) SpecSuiteDidEnd(summary *types.SuiteSummary) {

}

// PrintResult prints test result summary
func (sm *modelReporter) PrintReport() {
	fmt.Printf("==========================================================================================================================================\n")
	fmt.Printf("                Test Results \n")
	fmt.Printf("==========================================================================================================================================\n")

	totalCases := 0
	totalPass := 0
	totalFail := 0
	totalSkipped := 0
	var totalDuration time.Duration

	table := tablewriter.NewWriter(os.Stdout)
	table.SetHeader([]string{"TestCase", "Bundle", "Group", "Result", "Time"})
	//	table.SetAutoMergeCells(true)
	table.SetBorder(false) // Set Border to false
	table.SetRowLine(true) // Enable row line
	// Change table lines
	table.SetCenterSeparator("*")
	table.SetColumnSeparator("╪")
	table.SetRowSeparator("-")

	data := [][]string{}
	for bundleName, bundle := range sm.bundleResult {

		for groupName, group := range bundle.results {

			for tcName, tcData := range group.results {
				if tcData.skipCount != 0 {
					continue
				}
				result := fmt.Sprintf("%sPASS%s", greenColor, defaultStyle)
				if tcData.failCount != 0 {
					result = fmt.Sprintf("%sFAIL%s", redColor, defaultStyle)
				}
				data = append(data, []string{tcName, bundleName, groupName, result, tcData.duration.String()})
			}
		}

		//	fmt.Printf("              Bundle Summary Total Cases : %v %s Pass  : %v  %s Fail : %v %s Skipped : %v %s Duration %v \n", bundle.passCount+bundle.failCount,
		//greenColor, bundle.passCount, redColor, bundle.failCount, yellowColor, bundle.skipCount, defaultStyle, bundle.duration.String())
		totalCases += bundle.passCount + bundle.failCount + bundle.skipCount
		totalPass += bundle.passCount
		totalFail += bundle.failCount
		totalSkipped += bundle.skipCount
		totalDuration += bundle.duration

	}
	table.AppendBulk(data) // Add Bulk Data
	table.Render()
	fmt.Printf("==========================================================================================================================================\n")
	fmt.Printf("              Overall Run Summary Total Cases : %v %s Pass  : %v  %s Fail : %v %s Skipped : %v %s Duration %v \n", totalCases,
		greenColor, totalPass, redColor, totalFail, yellowColor, totalSkipped, defaultStyle, totalDuration.String())
	fmt.Printf("==========================================================================================================================================\n")

}
