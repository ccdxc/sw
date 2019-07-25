import { MetricTransform, TransformNames, TransformMetricData } from './types';
import { MetricsUtility } from '@app/common/MetricsUtility';

/**
 * For fields that are of type counter, we round their values to be whole numbers
 */
export class RoundCountersTransform extends MetricTransform<{}> {
  transformName = TransformNames.RoundCounters;

  transformMetricData(opts: TransformMetricData) {
    opts.result.series.forEach( (s) => {
      // For each series, each field that we have needs to be rounded
      this.fields.forEach( (f) => {
        const fieldData = this.getFieldData(this.measurement, f);
        if (fieldData.baseType !== 'Counter') {
          return;
        }
        const index = MetricsUtility.findFieldIndex(s.columns, f);
        if (index === 0) {
          console.error('roundcounter transform: field ' + f + ' missing in cols ' + s.columns.toString());
        }
        s.values.forEach( val => {
          if (val[index] != null) {
            val[index] = Math.round(val[index]);
          }
        });
      });
    });
  }

  load(config: {}) {
  }

  save(): {} {
    return {};
  }
}

