import { TransformGraphOptions, GraphTransform, TransformDataset, TransformNames, TransformMetricDatasets, TransformQueries } from './types';
import { FormGroup, FormControl } from '@angular/forms';
import { patternValidator } from '@sdk/v1/utils/validators';
import { ChartYAxe, ChartOptions } from 'chart.js';
import { getFieldData } from '../utility';
import { Utility } from '@app/common/Utility';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { MetricsMetadata, MetricField } from '@sdk/metrics/generated/metadata';
import { Telemetry_queryMetricsQuerySpec_function } from '@sdk/v1/models/generated/telemetry_query';
import { MetricsUtility } from '@app/common/MetricsUtility';

/**
 * Modifies queries to use derivative for counters
 */
export class DerivativeTransform extends GraphTransform<{}> {
  transformName = TransformNames.Derivative;

  public static doesFieldUseDerivative(field: MetricField) {
    return field.baseType.toLowerCase() === 'counter' && (field.units == null || field.units.toLowerCase() !== 'gauge');
  }


  transformQueries(opts: TransformQueries) {
    // TODO: Optimize query only if user has requested derivative counters
    const newQueries  = [];
    opts.queries.queries.forEach( (query) => {
      const newQuery = Utility.getLodash().cloneDeep(query);
      newQuery.query.function = Telemetry_queryMetricsQuerySpec_function.derivative;
      newQueries.push(newQuery);
    });
    opts.queries.queries.push(...newQueries);
  }

  transformMetricData(opts: TransformMetricDatasets) {
    if (opts.results && opts.results.length === 0) {
      return;
    }
    for (let i = 0; i < opts.results.length / 2; i++) {
      const origQuery = opts.results[i];
      const diffQuery = opts.results[(opts.results.length / 2) + i];
      if (!MetricsUtility.resultHasData(origQuery) || !MetricsUtility.resultHasData(diffQuery)) {
        continue;
      }
      if (origQuery.series.length !== diffQuery.series.length) {
        console.error('Derivative transform: got different series length');
        continue;
      }
      // We merge diffQuery into origQuery for fields that are counters
      const measurement = origQuery.series[0].name;
      const measurementData = MetricsMetadata[measurement];
      if (measurementData == null) {
        continue;
      }
      measurementData.fields.forEach( (field) => {
        if (DerivativeTransform.doesFieldUseDerivative(field)) {
          // Replace column with data from new query
          origQuery.series.forEach( (s, ii) => {
            const diffSeries = diffQuery.series[ii];
            const origIndex = MetricsUtility.findFieldIndex(s.columns, field.name);
            const newIndex = MetricsUtility.findFieldIndex(diffSeries.columns, field.name);
            // Orig Index has one extra point since it isn't a difference function
            // We remove the first point to make timestamp's match
            s.values.splice(0, 1);

            // origQuery may have more points as null values come back for
            // it, but not for rate/derivative queries

            s.columns[origIndex] = diffSeries.columns[newIndex];
            s.values.forEach( (v, iii) => {
              if (diffSeries.values[iii] != null) {
                v[origIndex] = diffSeries.values[iii][newIndex];
              }
              return v;
            });
          });
        }
      });
    }
    // Remove extra queries
    opts.results.splice(opts.results.length / 2, opts.results.length / 2);
  }

  load(config: {}) {
  }

  save(): {} {
    return {};
  }

}

