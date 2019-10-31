import * as moment from 'moment';
import * as _ from 'lodash';
import { PrettyDatePipe } from '../Pipes/PrettyDate.pipe';

enum Durations {
  minute = 'm',
  hour = 'h',
  day = 'd',
  week  = 'w',
  month= 'M',
  year = 'y',
}

export class TimeRangeUtility {

  public static keyOperatorValueParser(value: string, keyword, operators, durations): {errMsg: string, timeSyntax: TimeSyntaxEntry} {
    const ret = {
      errMsg: '',
      timeSyntax: null
    };
    if (value == null || value === '') {
      ret.errMsg = 'Input cannot be empty';
      return ret;
    }
    if (!value.toLowerCase().startsWith(keyword)) {
      ret.errMsg = 'Invalid input. Input should begin with a keyword';
      return ret;
    }
    let parsedString = value.trim();
    parsedString = parsedString.substring(keyword.length).trim();
    // check for operator
    const operator = operators.find((op) => {
      return parsedString.startsWith(op);
    });
    if (operator == null) {
      ret.errMsg = 'Invalid operator for keyword ' + keyword + '. Valid operators are ' + operators.join(', ');
      return ret;
    }
    parsedString = parsedString.substring(operator.length).trim();

    // Last character must be a duration keyword
    const duration = durations.find((d) => {
      return parsedString.endsWith(d);
    });

    if (duration == null) {
      ret.errMsg = 'Invalid duration. Valid durations are ' + durations.join(', ');
      return ret;
    }
    // Prefix of duration must be a valid number
    parsedString = parsedString.substring(0, parsedString.length - duration.length).trim();
    if (parsedString === '') {
      // Set default to be 1 if not given
      parsedString = '1';
    }
    const durationInt = parseInt(parsedString, 10);
    if (isNaN(durationInt) || durationInt === 0) {
      ret.errMsg = 'Duration prefix must be a valid number of at least 0';
      return ret;
    }
    ret.timeSyntax = {
      keyword: keyword,
      operator: operator,
      durationInt: durationInt,
      duration: duration
    };
    return ret;

  }
}

export class TimeRange {

  constructor(public startTime: TimeInstance, public endTime: TimeInstance) {
  }

  helper() {
    const keywordMap: { [key: string]: moment.Moment } = {};
    keywordMap['now'] = moment.utc();
    let startTime = this.startTime.getTime(keywordMap);
    let endTime = this.endTime.getTime(keywordMap);

    if (endTime == null && startTime == null) {
      // circular reference, since we couldn't calculate either of them
      return {
        errMsg: 'Start time and end time cannot both refer to each other'
      };
    }

    if (startTime == null) {
      keywordMap['end'] = endTime;
      startTime = this.startTime.getTime(keywordMap);
    }
    if (endTime == null) {
      keywordMap['start'] = startTime;
      endTime = this.endTime.getTime(keywordMap);
    }

    // Make sure start is less than end time.
    if (!startTime.isBefore(endTime)) {
      return {
        errMsg: 'Start time must be less than end time',
      };
    }

    // check if end time is now
    if (endTime.isSame(moment(), 'second')) {
      // start is now - 1h, text will be Past Hour
      if (startTime.isSame(moment().subtract(1, 'hour'), 'second')) {
        return {
          timeStr: 'Past hour',
          startTime: startTime,
          endTime: endTime
        };
      }
      // start is now - 1d, text will be Past Day
      if (startTime.isSame(moment().subtract(1, 'day'), 'second')) {
        return {
          timeStr: 'Past day',
          startTime: startTime,
          endTime: endTime
        };
      }
      // start is now - 1w, text will be Past Week
      if (startTime.isSame(moment().subtract(1, 'week'), 'second')) {
        return {
          timeStr: 'Past week',
          startTime: startTime,
          endTime: endTime
        };
      }
      // start is now - 1m, text will be Past Month
      if (startTime.isSame(moment().subtract(1, 'month'), 'second')) {
        return {
          timeStr: 'Past month',
          startTime: startTime,
          endTime: endTime
        };
      }
      // Text will be Past XXXX
      return {
        timeStr: 'Past ' + startTime.fromNow(true),
        startTime: startTime,
        endTime: endTime
      };
    }
    // check end time is now -1d, and start is now -2d
    if (endTime.isSame(moment().subtract(1, 'day'), 'second') &&
        startTime.isSame(moment().subtract(2, 'days'), 'second')) {
      // Text will be Previous Day
      return {
        timeStr: 'Previous day',
        startTime: startTime,
        endTime: endTime
      };
    }
    // check end time is now -1w, and start is now -2w
    if (endTime.isSame(moment().subtract(1, 'week'), 'second') &&
        startTime.isSame(moment().subtract(2, 'weeks'), 'second')) {
      // Text will be Previous Week
      return {
        timeStr: 'Previous week',
        startTime: startTime,
        endTime: endTime
      };
    }
    // check end time is now -1m, and start is now -2m
    if (endTime.isSame(moment().subtract(1, 'month'), 'second') &&
        startTime.isSame(moment().subtract(2, 'months'), 'second')) {
      // Text will be Previous Month
      return {
        timeStr: 'Previous month',
        startTime: startTime,
        endTime: endTime
      };
    }

    // Else we don't do any special casing
    return {
      timeStr: this.startTime.getDisplayString() + ' to ' + this.endTime.getDisplayString(),
      startTime: startTime,
      endTime: endTime
    };
  }

  toString() {
    // still missing keyword mapping if value is null
    return this.helper().timeStr;
  }

  getErrMsg() {
    return this.helper().errMsg;
  }

  getTime() {
    const temp = this.helper();
    return {
      startTime: temp.startTime,
      endTime: temp.endTime,
    };
  }

  isSame(time: TimeRange): boolean {
    if (time == null) {
      return false;
    }
    const newTimeRange = time.helper();
    const currTimeRange = this.helper();

    return newTimeRange.startTime.isSame(currTimeRange.startTime, 'minute') &&  newTimeRange.endTime.isSame(currTimeRange.endTime, 'minute');
  }

  isEndTimeNow() {
    const endTime = this.helper().endTime;
    return endTime.isSame(moment(), 'second');
  }

  getDuration(): moment.Duration {
    const temp = this.helper();
    return moment.duration(temp.endTime.diff(temp.startTime));
  }
}

export interface TimeSyntaxEntry {
  keyword: string;
  operator: string;
  durationInt: number;
  duration: string;
}

// All keywords should be based off this interface.
export interface KeywordFactory {
  // Parses the input and returns an errMsg if it fails to parse, and a
  // time instance if it was able to parse.
  parser: (inputStr) => ParserResp;
  // Whether this keyword is allowed to be in the start time field
  validStart: boolean;
  // Whether this keyword is allowed to be in the end time field
  validEnd: boolean;
}

export interface ParserResp {
  time?: TimeInstance;
  errMsg: string;
}

interface TimeInstance {
  getTime(keywordMap: { [key: string]: moment.Moment }): moment.Moment;
  getString(): string;
  getDisplayString(): string;
}

export class MomentTimeInstance implements TimeInstance {
  constructor(protected date: string) {}

  getTime(keywordMap) {
    return moment.utc(this.date);
  }

  getString() {
    return moment.utc(this.date).toISOString();
  }

  getDisplayString() {
    const prettyDate = new PrettyDatePipe('en-US');
    return prettyDate.transform(this.date);
  }

}

class KeyOperatorValueKeyword implements KeywordFactory {
  // The time instance this keyword will create
  public static instance = class KeywordInstance implements TimeInstance {

    constructor(public keyword, public operator,
      public durationInt,
      public duration) {
    }

    getTime(keywordMap: { [key: string]: moment.Moment }) {
      const keyVal = keywordMap[this.keyword];
      if (keyVal == null) {
        return null;
      }
      // Cloning time so we don't change map
      const time = moment(keyVal);
      if (time == null) {
        return null;
      }
      if (this.duration === '') {
        return time;
      }
      if (this.operator === '-') {
        time.subtract(this.durationInt, this.duration);
      } else {
        time.add(this.durationInt, this.duration);
      }
      return time;
    }

    getString() {
      if (this.durationInt === 0) {
        return this.keyword;
      }
      return this.keyword + ' ' + this.operator + ' ' + this.durationInt + this.duration;
    }

    getDisplayString() {
      return this.getString();
    }
  };

  operators = [
    '-',
    '+'
  ];
  constructor(public keyword,
    public validStart,
    public validEnd) {
  }


  parser(inputStr: string) {
    const res = TimeRangeUtility.keyOperatorValueParser(inputStr, this.keyword,  this.operators,  Object.values(Durations));
    if (res.errMsg !== '') {
      return {
        errMsg: res.errMsg
      };
    }
    return {
      time: new KeyOperatorValueKeyword.instance(this.keyword, res.timeSyntax.operator, res.timeSyntax.durationInt, res.timeSyntax.duration),
      errMsg: '',
    };
  }
}

/**************** keyword classes *******************/

class NowKeyword extends KeyOperatorValueKeyword {
  constructor() {
    super('now', true, true);
  }

  parser(inputStr: string) {
    // Now is valid without an operator or duration
    if (inputStr.toLowerCase().trim() === 'now') {
      return {
        time: new NowKeyword.instance(this.keyword, '', 0, ''),
        errMsg: '',
      };
    }
    return super.parser(inputStr);
  }
}

class StartKeyword extends KeyOperatorValueKeyword {
  operators = [
    '+',
  ];
  constructor() {
    super('start', false, true);
  }
}

class EndKeyword extends KeyOperatorValueKeyword {
  operators = [
    '-',
  ];

  constructor() {
    super('end', true, false);
  }
}

const Keywords = {
  'now': new NowKeyword(),
  'start': new StartKeyword(),
  'end': new EndKeyword(),
};

export class KeywordUtility {
  public static getStartKeywords() {
    return _.pickBy(Keywords, (k) => {
      return k.validStart;
    });
  }

  public static getEndKeywords() {
    return _.pickBy(Keywords, (k) => {
      return k.validEnd;
    });
  }
}
