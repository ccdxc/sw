import { GroupByTimeTransform } from './groupbytime.transform';
import * as moment from 'moment';
import { TransformQuery } from './types';
import { Telemetry_queryMetricsQuerySpec } from '@sdk/v1/models/generated/telemetry_query';

interface TestCase {
  maxPoints?: number;
  startTime: string;
  endTime: string;
  expGroupByTime: string;
}

describe('Group by time transform', () => {
  let transform: GroupByTimeTransform;

  it('should transform query', () => {

    transform = new GroupByTimeTransform();


    const testCases: TestCase[] = [
      {
        startTime: moment().subtract('1', 'h').toISOString(),
        endTime: moment().toISOString(),
        expGroupByTime: '60s', //  VS-1098, VS-1121
      },
      {
        startTime: moment().subtract('6', 'h').toISOString(),
        endTime: moment().toISOString(),
        expGroupByTime: '1m',
      },
      {
        startTime: moment().subtract('1', 'd').toISOString(),
        endTime: moment().toISOString(),
        expGroupByTime: '5m',
      },
    ];

    testCases.forEach( (tc) => {
      if (tc.maxPoints == null) {
        tc.maxPoints = 500;
      }
      transform.maxPoints = tc.maxPoints;

      const opts: TransformQuery = {
        query: new Telemetry_queryMetricsQuerySpec()
      };
      opts.query['start-time'] = tc.startTime as any;
      opts.query['end-time'] = tc.endTime as any;

      transform.transformQuery(opts);

      expect(opts.query['group-by-time']).toBe(tc.expGroupByTime);
    });
  });

});
