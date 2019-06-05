import { MetricTransform, TransformDataset, TransformMetricData, TransformQuery } from './types';
import { ITelemetry_queryMetricsQueryResult } from '@sdk/v1/models/telemetry_query';
import { ITelemetry_queryMetricsQuerySpec } from '@sdk/v1/models/generated/telemetry_query';

export interface ValueMap {
  // Map from measurement to function to modify the value.
  [key: string]: (res: ITelemetry_queryMetricsQueryResult) => void;
}

export interface QueryMap {
  // Map from measurement to function to modify the query.
  [key: string]: (res: ITelemetry_queryMetricsQuerySpec) => void;
}

/**
 * Modifies the dataset so that they use the displayName from the
 * field metadata instead of name for the label.
 */
export class FieldValueTransform extends MetricTransform {
  transformName = 'FieldValue';

  constructor(protected queryTransformMap: QueryMap, protected valueTransformMap: ValueMap) {
    super();
  }

  transformQuery(opts: TransformQuery) {
    const measurement = opts.query.kind;
    const funcTransform = this.queryTransformMap[measurement];
    funcTransform(opts.query);
  }

  transformMetricData(opts: TransformMetricData) {
    const measurement = this.measurement;
    const funcTransform = this.valueTransformMap[measurement];
    funcTransform(opts.result);
  }
}

