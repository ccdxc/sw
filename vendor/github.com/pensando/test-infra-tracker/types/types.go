package types

import (
	"errors"
	"time"
)

const (
	// TrackerPort is the port of test tracker
	TrackerPort = 7890
)

// Report is the actual test report
type Report struct {
	Name        string
	Description string
	Owner       string
	LogURL      string
	Area        string
	Subarea     string
	Feature     string
	SourceDir   string
	GenDir      string
	BuildName   string
	FinishTime  time.Time
	BuildFound  int32
	BuildErrors int32
	BuildWarns  int32
	SimFound    int32
	SimErrors   int32
	SimWarns    int32
	Completed   int32
	CPUTime     int32
	CycPerNs    int32
	SimDsMb     int32
	Duration    int32
	Coverage    int32
	Result      int32
	Seed        int64
	First       string
	Detail      string
}

// Validate validates the structure of report
func (r *Report) Validate() error {
	if r.Name == "" || r.LogURL == "" || r.Area == "" {
		return errors.New("Test case Name/LogURL/Area can't be empty")
	}
	return nil
}

// NewTestcase creates testcase structure based on report data
func (r *Report) NewTestcase() Testcase {
	return Testcase{
		Name:        r.Name,
		Description: r.Description,
		Owner:       r.Owner,
		Area:        r.Area,
		Subarea:     r.Subarea,
	}
}

// NewRecord creates record structure based on report data
func (r *Report) NewRecord(targetID int32, testbed string) Record {
	return Record{
		TargetID:    targetID,
		LogURL:      r.LogURL,
		Testbed:     testbed,
		FinishTime:  r.FinishTime,
		Duration:    r.Duration,
		Coverage:    r.Coverage,
		Result:      r.Result,
		Detail:      r.Detail,
		Feature:     r.Feature,
		SourceDir:   r.SourceDir,
		GenDir:      r.GenDir,
		BuildName:   r.BuildName,
		First:       r.First,
		BuildFound:  r.BuildFound,
		BuildErrors: r.BuildErrors,
		BuildWarns:  r.BuildWarns,
		SimFound:    r.SimFound,
		SimErrors:   r.SimErrors,
		SimWarns:    r.SimWarns,
		Seed:        r.Seed,
		Completed:   r.Completed,
		CPUTime:     r.CPUTime,
		CycPerNs:    r.CycPerNs,
		SimDsMb:     r.SimDsMb,
	}
}

// Reports contains the list of test report
type Reports struct {
	Repository string
	SHA        string
	SHATitle   string
	Testbed    string
	TargetID   int32
	Testcases  []*Report
}

// Validate validates the structure of reports
func (r *Reports) Validate() error {
	if r.Repository == "" || r.SHA == "" || r.SHATitle == "" || r.Testbed == "" {
		return errors.New("Reports Repository/SHA/SHATitle/Testbed can't be empty")
	}
	if len(r.Testcases) == 0 {
		return errors.New("No Testcases in the reports")
	}
	for _, tc := range r.Testcases {
		if err := tc.Validate(); err != nil {
			return err
		}
	}
	return nil
}

// Repository encapulates a repository
type Repository struct {
	ID         int32
	Repository string
}

// Ref is the repository and sha this testcase refers to.
type Ref struct {
	ID int32 `json:"-"` // this should always be looked up via other means externally

	// omitempty here is for cosmetics / parsing only
	RepositoryID int32 `json:"-" sql:",notnull,fk:Repository"` // same here
	SHA          string
	Title        string
}

// Testcase is the static information for one test case
type Testcase struct {
	ID int32

	RepositoryID int32 `json:"-" sql:",notnull,fk:Repository"` // same here

	Name        string
	Description string
	Owner       string
	Area        string
	Subarea     string
}

// Record is the test result for each test case
type Record struct {
	ID           int32
	TestcaseID   int32
	RefID        int32
	RepositoryID int32 // mainly for browse optimization and avoid join multiple tables
	LogURL       string
	Testbed      string
	Feature      string
	SourceDir    string
	GenDir       string
	BuildName    string
	First        string
	Detail       string
	FinishTime   time.Time
	TargetID     int32
	Duration     int32
	Coverage     int32
	Result       int32
	BuildFound   int32
	BuildErrors  int32
	BuildWarns   int32
	SimFound     int32
	SimErrors    int32
	SimWarns     int32
	Completed    int32
	CPUTime      int32
	CycPerNs     int32
	SimDsMb      int32
	Seed         int64
}

// Equals compare 2 test case parameters
func (tc *Testcase) Equals(tc2 *Testcase) bool {
	return tc.RepositoryID == tc2.RepositoryID &&
		tc.Name == tc2.Name &&
		tc.Description == tc2.Description &&
		tc.Owner == tc2.Owner &&
		tc.Area == tc2.Area &&
		tc.Subarea == tc2.Subarea
}
