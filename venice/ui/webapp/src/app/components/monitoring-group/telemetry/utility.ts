import { MetricsMetadata, MetricField } from '@sdk/metrics/generated/metadata';

export const sourceFieldKey = (sourceID: string, measurement: string, field: string) => {
  return measurement + '-' + field;
};

export const getFieldData = (measurement: string, field: string): MetricField => {
  return MetricsMetadata[measurement].fields.find( x => x.name === field);
};

export const getFieldsData = (measurement: string): MetricField[] => {
  return MetricsMetadata[measurement].fields;
};
