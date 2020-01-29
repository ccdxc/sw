import { walk } from "./utils";
import * as fs from 'fs';
import * as utils from './utils';
import { customMetrics } from '../custom-metrics'
import * as _ from 'lodash';
import * as rmdir from 'rimraf';

export function generateMetricMetadata(inputBaseFolder, outputFolder) {
  if (fs.existsSync(outputFolder)){
    // Delete all contents
    rmdir.sync(outputFolder);
  }
  fs.mkdirSync(outputFolder, {recursive: true});

  const basetypeToJSType = {
    int8: 'number',
    uint8: 'number',
    int16: 'number',
    uint16: 'number',
    int32: 'number',
    uint32: 'number',
    int64: 'number',
    uint64: 'number',
    int: 'number',
    uint: 'number',
    Counter: 'number',
    float32: 'number',
    float64: 'number',
    complex64: 'number',
    complex128: 'number',
    byte: 'number',
    number: 'number',
    Gauge: 'number',
    PrecisionGauge: 'number',
    Bitmap: 'number',
    Celsius: 'number',
    KB: 'number',
    MHz: 'number',
    MilliWatts: 'number',

    string: 'string',
  }

  const files = walk(inputBaseFolder, x => x.endsWith('.json'));
  const messages = [];
  files.forEach( (f) => {
    const metadata = JSON.parse(fs.readFileSync(f, 'utf8').trim());
    metadata.Messages.forEach((m) => {
      m = _.transform(m, function (result, val, key) {
        result[_.camelCase(key)] = val;
      });
      m.objectKind = "DistributedServiceCard"
      if (m.fields == null) {
        return;
      }
      m.fields.push({
        Name: 'reporterID',
        Description: 'Name of reporting object',
        BaseType: 'string',
        JsType: 'string',
        IsTag: true,
      })
      m.fields = m.fields.map( (field) => {
        if (field.DisplayName == null) {
          field.DisplayName = field.Name;
        }
        if (field.Tags == null) {
          field.Tags = ['Level4']
        }
        if (basetypeToJSType[field.BaseType] == null) {
          throw new Error("base type " + field.BaseType + " not recognized for field " + field.Name);
        }
        field.jsType = basetypeToJSType[field.BaseType];
        if (field.BaseType === 'Bitmap') {
          // Set as debug metric for now
          field.tags = ["Level7"]
        }
        return _.transform(field, function (result, val, key) {
          result[_.camelCase(key)] = val;
        });
      });
      messages.push(m);
    });
  });
  customMetrics.forEach( (metric) => {
    messages.push(metric);
  })
  const template = 'generate-metrics-ts.hbs';
  const outputFile = 'metrics/generated/metadata.ts';
  utils.writeTemplate(template, messages, outputFile);
}
