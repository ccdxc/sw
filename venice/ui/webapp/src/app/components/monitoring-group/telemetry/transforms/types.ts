import { ITelemetry_queryMetricsQuerySpec } from '@sdk/v1/models/generated/telemetry_query';
import { ITelemetry_queryMetricsResultSeries, ITelemetry_queryMetricsQueryResult } from '@sdk/v1/models/telemetry_query';
import { ChartData as ChartJSData, ChartDataSets as ChartJSDataSets, ChartOptions } from 'chart.js';
import { Observer } from 'rxjs';
import { Utility } from '@app/common/Utility';
import { MetricsMetadata, MetricField } from '@sdk/metrics/generated/metadata';

export interface ChartDataSets extends ChartJSDataSets {
  sourceID: string;
  sourceMeasurement: string;
  sourceField: string;
}

export interface ChartData extends ChartJSData {
  datasets: ChartDataSets[];
}

export interface TransformQuery {
  query: ITelemetry_queryMetricsQuerySpec;
}

export interface TransformMetricData {
  result: ITelemetry_queryMetricsQueryResult;
}

export interface TransformDataset {
  dataset: ChartDataSets;
  series: ITelemetry_queryMetricsResultSeries;
  measurement: string;
  field: string;
  units: string;
  fieldIndex: number;
}

export interface TransformGraphOptions {
  data: TransformDataset[];
  graphOptions: ChartOptions;
  oldGraphOptions: ChartOptions;
}

export interface TransformDatasets extends Array<TransformDataset> {}

export abstract class MetricTransform {
  abstract transformName: string;

  // Fields that will be populated by MetricSource
  getTransform: (transformName: string) => MetricTransform;
  reqMetrics: Observer<any>;
  measurement;
  fields;

  // Hooks to be overridden
  onMeasurementChange() {}
  onFieldChange() {}
  // Hook called before query goes to Venice
  transformQuery(opts: TransformQuery) {}
  // Hook called before data is converted to chart js datasets
  transformMetricData(opts: TransformMetricData) {}
  // Hook called after each dataset is created
  transformDataset(opts: TransformDataset) {}
  // Hook called after all datasets have been created for a data source
  transformDatasets(opts: TransformDatasets) {}

  // Utility functions
  requestMetrics() {
    this.reqMetrics.next(true);
  }

  getFieldData(measurement, field) {
    return MetricsMetadata[measurement].fields.find(x => x.name === field);
  }
}

export abstract class GraphTransform {
  abstract transformName: string;

  // Hooks to be overridden
  // Hook to set graph options after all datasets have been created for all results
  transformGraphOptions(opts: TransformGraphOptions) {}

  // Utility functions
  getFieldData(measurement, field) {
    return MetricsMetadata[measurement].fields.find(x => x.name === field);
  }
}


/**
 * Data source object is used for storing user input
 * of graph options/data, and for data transforms to alter
 * the metric queries and rendering
 *
 * There can be multiple data sources and they function independently.
 * Each data source has one measurement, and any number of fields under
 * that measurement.
 * Transforms can also be registered during creation, and the data transform
 * hooks will be called on each registered transform.
 */
export class DataSource {
  private _measurement: string;
  private _fields: string[] = [];

  // Unique id is used to differentiate datasets that are exactly the same,
  // but come from two different data sources
  id: string = Utility.s4();

  // List of transforms registered
  transforms: MetricTransform[] = [];

  // Map from transform name to the instance
  transformMap: { [transformName: string]: MetricTransform };

  // Observer that will be taken in during construction. Will emit
  // whenever it detects we need to execute a new metric request or change the rendering options.
  reqMetrics: Observer<any>;

  get measurement() {
    return this._measurement;
  }

  set measurement(value) {
    // Update all transforms and null the field values
    this._measurement = value;
    this._fields = [];
    this.transforms.forEach( (t) => {
      t.measurement = this.measurement;
      t.fields = null;
      t.onMeasurementChange();
      t.onFieldChange();
    });
    this.reqMetrics.next(true);
  }

  get fields() {
    return this._fields;
  }

  set fields(value) {
    if (value == null) {
      value = [];
    }
    this._fields = value;
    this.transforms.forEach( (t) => {
      t.fields = this.fields;
      t.onFieldChange();
    });
    this.reqMetrics.next(true);
  }

  getTransform(transformName: string): MetricTransform {
    return this.transformMap[transformName];
  }

  constructor(reqMetrics: Observer<any>, transforms: MetricTransform[] = []) {
    this.transforms = transforms;
    this.reqMetrics = reqMetrics;
    this.transformMap = {};
    // For each transform we populate fields so that transforms can see
    // the state of other transforms, and can request metrics.
    transforms.forEach( (t) => {
      t.reqMetrics = reqMetrics;
      this.transformMap[t.transformName] = t;
      t.getTransform = (name: string) => {
        return this.getTransform(name);
      };
    });
  }

  transformQuery(opts: TransformQuery) {
    this.transforms.forEach( (t) => {
      t.transformQuery(opts);
    });
  }

  transformMetricData(opts: TransformMetricData) {
    this.transforms.forEach( (t) => {
      t.transformMetricData(opts);
    });
  }

  transformDataset(opts: TransformDataset) {
    this.transforms.forEach( (t) => {
      t.transformDataset(opts);
    });
  }

  transformDatasets(opts: TransformDatasets) {
    this.transforms.forEach( (t) => {
      t.transformDatasets(opts);
    });
  }
}
