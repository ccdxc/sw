import { PipeTransform, Pipe } from '@angular/core';
import { DatePipe } from '@angular/common';

@Pipe({
  name: 'PrettyDate'
})
export class PrettyDatePipe extends DatePipe implements PipeTransform {
  transform(value: any, args?: any): any {
    if (super.transform(value, 'shortDate') === super.transform(Date.now(), 'shortDate')) {
      // If same day only show time
      return super.transform(value, 'HH:mm a');
    } else if (super.transform(value, 'yyyy') === super.transform(Date.now(), 'yyyy')) {
      // If same year only show time, month, day
      return super.transform(value, 'HH:mm a  -  M/d');
    } else {
      // Else we show time, month, day, year
      return super.transform(value, 'HH:mm a  -  M/d/yy');
    }
  }
}
