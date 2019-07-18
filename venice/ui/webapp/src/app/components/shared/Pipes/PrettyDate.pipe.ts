import { PipeTransform, Pipe } from '@angular/core';
import { DatePipe } from '@angular/common';
import * as moment from 'moment';

@Pipe({
  name: 'PrettyDate'
})
export class PrettyDatePipe extends DatePipe implements PipeTransform {
  transform(value: string, args?: any): string {
    if (value == null || (typeof(value) === 'string' && value.trim().length === 0)) {
      return '';
    }

    let precision: number = 9;
    if ( !!args && typeof(args) === 'string' && args.includes(',')) {
      const splittedString = args.split(',');
      precision = parseInt(splittedString[1], 10);
      precision = precision ? precision : 9;
      args = splittedString[0];
    }

    if (args === 'ns') {
      // Assuming value is in RFC spec
      const ns = parseFloat(value.substring(value.indexOf('.'), value.length - 1)).toFixed(precision).toString();
      return super.transform(value, 'yyyy-MM-dd HH:mm:ss', 'UTC') + ns + ' UTC';
      // Old display format
      // if (super.transform(value, 'shortDate', 'UTC') === super.transform(Date.now(), 'shortDate', 'UTC')) {
      //   // If same day only show time
      //   return super.transform(value, 'H:mm:ss', 'UTC') + ns;
      // } else if (super.transform(value, 'yyyy', 'UTC') === super.transform(Date.now(), 'yyyy', 'UTC')) {
      //   // If same year only show time, month, day
      //   return super.transform(value, 'H:mm:ss', 'UTC') + ns + super.transform(value, ' - M/d', 'UTC');
      // } else {
      //   // Else we show time, month, day, year
      //   return super.transform(value, 'H:mm:ss', 'UTC') + ns + super.transform(value, ' - M/d/yy', 'UTC');
      // }
    } else if (args === 'graph') {
      if (super.transform(value, 'shortDate', 'UTC') >= super.transform(moment().subtract(1, 'days').valueOf(), 'shortDate', 'UTC')) {
        // If same day only show time
        return super.transform(value, 'H:mm', 'UTC');
      } else if (super.transform(value, 'yyyy', 'UTC') === super.transform(Date.now(), 'yyyy', 'UTC')) {
        // If same year only show time, month, day
        return super.transform(value, 'H:mm -  M/d', 'UTC');
      } else {
        // Else we show time, month, day, year
        return super.transform(value, 'H:mm -  M/d/yy', 'UTC');
      }
    }

    // DEFAULT BEHAVIOUR
    return super.transform(value, 'yyyy-MM-dd HH:mm:ss', 'UTC') + ' UTC';
      // Old display format
    // if (super.transform(value, 'shortDate', 'UTC') === super.transform(Date.now(), 'shortDate', 'UTC')) {
    //   // If same day only show time
    //   return super.transform(value, 'H:mm:ss', 'UTC');
    // } else if (super.transform(value, 'yyyy', 'UTC') === super.transform(Date.now(), 'yyyy', 'UTC')) {
    //   // If same year only show time, month, day
    //   return super.transform(value, 'H:mm:ss -  M/d', 'UTC');
    // } else {
    //   // Else we show time, month, day, year
    //   return super.transform(value, 'H:mm:ss -  M/d/yy', 'UTC');
    // }
  }
}
