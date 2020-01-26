import { MetricTransform, TransformQuery, TransformDataset, TransformNames } from './types';
import * as moment from 'moment';

/**
 * Populates the group by field tag in metric query and
 * transforms the dataset label to include node name
 */
export class GroupByTimeTransform extends MetricTransform<{}> {

  transformName = TransformNames.GroupByTime;
  maxPoints: number = 20;
  minimumGroupByTime: string = '60s';


  transformQuery(opts: TransformQuery): boolean {
    const start = moment(opts.query['start-time']);
    const end = moment(opts.query['end-time']);
    const duration = moment.duration(end.diff(start));

    // metrics are reported every 30 seconds
    let numPoints = duration.asMinutes() * 2;

    if (numPoints < this.maxPoints) {
      opts.query['group-by-time'] =  this.minimumGroupByTime; // VS-1098. Set to be the multiple of 30s
      return;
    }

    // set group by time in min increments
    // so that we never get more than maxPoints

    let groupByMin = 1;
    numPoints = Math.floor(numPoints / 2);
    while (Math.floor(numPoints / groupByMin) > this.maxPoints) {
      groupByMin += 1;
    }

    opts.query['group-by-time'] = groupByMin + 'm';
    return true;
  }

  load(config: {}) {
  }

  save(): {} {
    return {};
  }

}
