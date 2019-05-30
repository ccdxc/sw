import { PipeTransform, Pipe } from '@angular/core';
import { DatePipe } from '@angular/common';

@Pipe({
  name: 'PrettyDate'
})
export class PrettyDatePipe extends DatePipe implements PipeTransform {
  transform(value: string, args?: any): string {
    if (args === 'ns') {
      if (value == null) {
        return '';
      }
      // Assuming value is in RFC spec
      const ns = value.substring(value.indexOf('.'), value.length - 1);
      if (super.transform(value, 'shortDate') === super.transform(Date.now(), 'shortDate')) {
        // If same day only show time
        return super.transform(value, 'H:mm:ss', 'UTC') + ns;
      } else if (super.transform(value, 'yyyy') === super.transform(Date.now(), 'yyyy')) {
        // If same year only show time, month, day
        return super.transform(value, 'H:mm:ss', 'UTC') + ns + super.transform(value, ' - M/d', 'UTC');
      } else {
        // Else we show time, month, day, year
        return super.transform(value, 'H:mm:ss', 'UTC') + ns + super.transform(value, ' - M/d/yy', 'UTC');
      }
    }
    if (super.transform(value, 'shortDate') === super.transform(Date.now(), 'shortDate')) {
      // If same day only show time
      return super.transform(value, 'H:mm:ss', 'UTC');
    } else if (super.transform(value, 'yyyy') === super.transform(Date.now(), 'yyyy')) {
      // If same year only show time, month, day
      return super.transform(value, 'H:mm:ss -  M/d', 'UTC');
    } else {
      // Else we show time, month, day, year
      return super.transform(value, 'H:mm:ss -  M/d/yy', 'UTC');
    }
  }
}
