import { Component, OnInit, ViewEncapsulation, Input, Output, EventEmitter, ViewChild, AfterViewInit, OnDestroy, OnChanges, SimpleChanges } from '@angular/core';
import { FormControl, ValidatorFn, AbstractControl, ValidationErrors, FormGroup } from '@angular/forms';
import * as moment from 'moment';
import { TimeRange, TimeRangeUtility, KeywordFactory, ParserResp, MomentTimeInstance, KeywordUtility } from './utility';
import { OverlayPanel } from 'primeng/primeng';
import { Subscription } from 'rxjs';


export interface TimeRangeOption {
  text: string;
  startTime: string;
  // If endtime isn't provided, it default to now
  endTime?: string;
}

export const citadelTimeOptions: TimeRangeOption[] = [
  {
    text: 'Past hour',
    startTime: 'end - 1h',
  },
  {
    text: 'Past 6 hour',
    startTime: 'end - 6h',
  },
  {
    text: 'Past 12 hour',
    startTime: 'end - 12h',
  },
  {
    text: 'Past day',
    startTime: 'end - d',
  },
  {
    text: 'Previous day',
    startTime: 'now - 2d',
    endTime: 'now - d',
  },
  {
    text: 'Past week',
    startTime: 'end - w',
  },
];

export const citadelMaxTimePeriod = moment.duration(7, 'days');

@Component({
  selector: 'app-timerange',
  templateUrl: './timerange.component.html',
  styleUrls: ['./timerange.component.scss'],
  encapsulation: ViewEncapsulation.None,
})
export class TimeRangeComponent implements OnInit, AfterViewInit, OnDestroy, OnChanges {

  @ViewChild('timeRangePanel') overlayPanel: OverlayPanel;

  @Input() startTime: string = 'now - 24h';
  @Input() endTime: string = 'now';
  @Input() selectedTimeRange: TimeRange;
  @Input() maxTimePeriod: moment.Duration;

  // min-max start and end date  // VS-1063
  @Input() minStartSelectDateValue: Date = null;
  @Input() maxStartSelectDateValue: Date = null;
  @Input() minEndSelectDateValue: Date = null;
  @Input() maxEndSelectDateValue: Date =  new Date();

  @Input() timeRangeOptions: TimeRangeOption[] = [
    {
      text: 'Past hour',
      startTime: 'end - 1h',
    },
    {
      text: 'Past 6 hour',
      startTime: 'end - 6h',
    },
    {
      text: 'Past 12 hour',
      startTime: 'end - 12h',
    },
    {
      text: 'Past day',
      startTime: 'end - d',
    },
    {
      text: 'Past week',
      startTime: 'end - w',
    },
    {
      text: 'Past month',
      startTime: 'end - M',
    },
    {
      text: 'Previous day',
      startTime: 'now - 2d',
      endTime: 'now - d',
    },
    {
      text: 'Previous week',
      startTime: 'now - 2w',
      endTime: 'now - w',
    },
    {
      text: 'Previous month',
      startTime: 'now - 2M',
      endTime: 'now - M',
    },
  ];
  @Output() timeRange: EventEmitter<TimeRange> = new EventEmitter<TimeRange>();

  timeFormGroup: FormGroup;
  groupErrorMessage: string = '';
  displayString: string = '';
  lastSelectedTimeRange: TimeRange;

  subscriptions: Subscription[] = [];



  startTimeCalendar: Date;
  endTimeCalendar: Date;

  ngOnInit() {
    let startTime = this.startTime;
    let endTime = this.endTime;
    if (this.selectedTimeRange != null) {
      startTime = this.selectedTimeRange.startTime.getString();
      endTime = this.selectedTimeRange.endTime.getString();
    }
    this.timeFormGroup = new FormGroup(
      {
        startTime: new FormControl(startTime, [
          this.timeRangeInputValidator(KeywordUtility.getStartKeywords())]
        ),
        endTime: new FormControl(endTime, [
          this.timeRangeInputValidator(KeywordUtility.getEndKeywords())]
        ),
      },
      {
        validators: [this.timeRangeGroupValidator()],
        updateOn: 'change',
      }
    );
    this.timeFormGroup.updateValueAndValidity();
  }

  calendarStartSelect(value) {
    this.timeFormGroup.get('startTime').reset(value.toISOString());
  }

  calendarEndSelect(value) {
    this.timeFormGroup.get('endTime').reset(value.toISOString());
  }

  ngAfterViewInit() {
    // If a user enters an invalid time range, and then closes the
    // overlay, the next time they open the overlay it should show
    // the values for the current timerange that is in effect.
    const sub = this.overlayPanel.onShow.subscribe(() => {
      if (this.lastSelectedTimeRange == null) {
        return;
      }
      const start = this.lastSelectedTimeRange.startTime.getString();
      const end = this.lastSelectedTimeRange.endTime.getString();
      this.timeFormGroup.reset({
        startTime: start,
        endTime: end
      });
    });
    this.subscriptions.push(sub);
  }

  ngOnChanges(changes: SimpleChanges): void {
    if (!this.maxEndSelectDateValue) {
      this.maxEndSelectDateValue = new Date();
    }
  }

  setTimeRange(opt: TimeRangeOption) {
    const startTime = opt.startTime;
    let endTime = opt.endTime;
    if (endTime == null) {
      endTime = 'now';
    }

    this.timeFormGroup.reset({
      startTime: startTime,
      endTime: endTime
    });
    this.overlayPanel.hide();
  }

  /**
   * Make sure start is less than end time.
   * Make sure both times don't reference each other
   * Ex.
   * startTime = end - 5m
   * endTiem = start + 10m
   */
  timeRangeGroupValidator(): ValidatorFn {
    return (control: FormGroup): ValidationErrors | null => {
      if (control.get('startTime').errors != null) {
        return null;
      }
      if (control.get('endTime').errors != null) {
        return null;
      }
      const values = control.value;
      const startTimeInput = values.startTime;
      const endTimeInput = values.endTime;
      const startTimeEntry = this.timeRangeParser(KeywordUtility.getStartKeywords(), startTimeInput);
      const endTimeEntry = this.timeRangeParser(KeywordUtility.getEndKeywords(), endTimeInput);

      // Update the calendar


      const timeRange = new TimeRange(startTimeEntry.time, endTimeEntry.time);

      if (startTimeEntry.time.constructor.name === 'MomentTimeInstance') {
        // Moment instance doesn't need a keyword map
        this.startTimeCalendar = (startTimeEntry.time.getTime(null).toDate());
      }
      if (endTimeEntry.time.constructor.name === 'MomentTimeInstance') {
        // Moment instance doesn't need a keyword map
        this.endTimeCalendar = (endTimeEntry.time.getTime(null).toDate());
      }

      const err = timeRange.getErrMsg();
      if (err != null) {
        this.groupErrorMessage = err;
        return {
          timeRangeGroup: {
            required: true,
            message: this.groupErrorMessage
          }
        };
      }

      // Check that timeRange is within the given bounds
      if (this.maxTimePeriod != null && timeRange.getDuration().asMilliseconds() > this.maxTimePeriod.asMilliseconds()) {
        this.groupErrorMessage = 'Time range cannot be greater than ' + this.maxTimePeriod.humanize();
        return {
          timeRangeGroup: {
            required: true,
            message: this.groupErrorMessage
          }
        };
      }

      if (this.maxEndSelectDateValue) {
         if (this.endTimeCalendar > this.maxEndSelectDateValue ) {
          this.groupErrorMessage = 'End time cannot be greater than max value ' + this.maxEndSelectDateValue.toISOString();
          return {
            timeRangeGroup: {
              required: true,
              message: this.groupErrorMessage
            }
          };
         }
      }

      this.groupErrorMessage = '';
      this.timeRange.emit(timeRange);
      this.lastSelectedTimeRange = timeRange;
      this.displayString = timeRange.toString();
      return null;

    };
  }

  /**
   * Time range validator
   * Valid inputs are a date string, or time synyax
   *
   * keywords  = start, end, now
   * operators = +, -
   * durations = m, h, d, w, M
   *
   * Syntax:
   *  <keyword> <operator> <number><duration>
   */
  timeRangeInputValidator(keywords: { [key: string]: KeywordFactory }): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      if (control.value == null || control.value.length === 0) {
        return {
          timeRangeInput: {
            required: true,
            message: 'Value cannot be empty'
          }
        };
      }
      const parsedResp = this.timeRangeParser(keywords, control.value);
      if (parsedResp.errMsg !== '') {
        return {
          timeRangeInput: {
            required: true,
            message: parsedResp.errMsg
          }
        };
      }
      return null;
    };
  }

  timeRangeParser(keywords: { [key: string]: KeywordFactory }, value: string): ParserResp {
    // check keyword, and delegate.
    if (value == null || value === '') {
      return {
        errMsg: 'Input cannot be empty'
      };
    }
    const parsedString = value.trim();
    const keyword = Object.keys(keywords).find((k) => {
      return parsedString.toLowerCase().startsWith(k);
    });
    if (keyword != null) {
      return keywords[keyword].parser(parsedString);
    }
    // Try to parse as a date
    const dateValue = moment.utc(value);
    if (!dateValue.isValid()) {
      return {
        errMsg: 'Must be a valid date.'
      };
    }


    return {
      time: new MomentTimeInstance(value),
      errMsg: '',
    };
  }

  ngOnDestroy() {
    this.subscriptions.forEach(s => {
      s.unsubscribe();
    });
  }
}
