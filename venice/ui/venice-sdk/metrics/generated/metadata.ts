export interface MetricField {
  name: string,
  description?: string,
  displayName: string,
  units?: string,
  baseType: string,
  jsType: string,
  allowedValues?: string[],
  scaleMin?: number,
  scaleMax?: number,
  tags?: string[],
  scope?: string,
  isTag?: boolean,
}

export interface MetricMeasurement {
  name: string,
  description: string,
  displayName: string,
  objectKind?: string,
  // Type of interface, if objectKind is networkInterface
  interfaceType?: string,
  scope?: string,
  fields: MetricField[],
  tags?: string[],
}

export let MetricsMetadata: { [key: string]: MetricMeasurement } = {}

export function setMetricsMetadata(val: { [key: string]: MetricMeasurement }) {
  MetricsMetadata = val;
}