import { FieldSelectorTransform } from './fieldselector.transform';
import * as _ from 'lodash';
import { fakeAsync, tick } from '@angular/core/testing';
import { FormControl } from '@angular/forms';
import { MetricsMetadata } from '@sdk/metrics/generated/metadata';

describe('Field Selector transform', () => {
  let transform: FieldSelectorTransform;
  let reqMetricsSpy: jasmine.Spy;

  /**
   * 1. Switching measurements should update the repeater options
   * 2. Form array should be replaced
   * 3. String from should use displayName
   */
  it('on measurement change', fakeAsync(() => {

    transform = new FieldSelectorTransform();
    reqMetricsSpy = spyOn(transform, 'requestMetrics');

    transform.measurement = 'Node';
    transform.onMeasurementChange();
    tick();

    // Verifying repeater data
    verifyFieldData('Node');

    // Setting debug mode
    transform.debugMode = true;
    transform.onDebugModeChange();

    verifyFieldData('Node', true);

    // Verifying currValue and stringValue are updated when the
    // control is changed
    let newValue: any = [{
      keyFormControl: 'CPUUsedPercent',
      operatorFormControl: '>=',
      valueFormControl: [2],
    }];

    transform.formArray.push(new FormControl(newValue[0]));
    transform.valueChange(transform.formArray.value);
    tick(1000);

    // Metrics should be requested
    expect(reqMetricsSpy).toHaveBeenCalled();
    expect(_.isEqual(transform.currValue, newValue)).toBeTruthy('Curr value was not upgraded correctly');
    expect(transform.stringForm).toBe('Percent CPU Used >= 2');

    // Switching measurement
    transform.measurement = 'DistributedServiceCard';
    transform.onMeasurementChange();

    // Verifying repeater data
    verifyFieldData('DistributedServiceCard', true);

    // Verifying currValue and stringValue are updated when the
    // control is changed
    expect(_.isEqual(transform.currValue, [])).toBeTruthy('Curr value was not reset');

    newValue = [
      {
        keyFormControl: 'CPUUsedPercent',
        operatorFormControl: '<=',
        valueFormControl: [10],
      },
      {
        keyFormControl: 'reporterID',
        operatorFormControl: 'equals',
        valueFormControl: ['naples1', 'naples2'],
      },
      { // incomplete req should be filtered out
        keyFormControl: 'DiskFree',
        operatorFormControl: '>=',
      },
    ];
    const expValue = [
      {
        keyFormControl: 'CPUUsedPercent',
        operatorFormControl: '<=',
        valueFormControl: [10],
      },
      {
        keyFormControl: 'reporterID',
        operatorFormControl: 'equals',
        valueFormControl: ['naples1', 'naples2'],
      },
    ];
    transform.formArray.removeAt(0);
    transform.formArray.push(new FormControl(newValue[0]));
    transform.formArray.push(new FormControl(newValue[1]));
    transform.formArray.push(new FormControl(newValue[2]));
    transform.valueChange(transform.formArray.value);
    tick(1000);

    // Metrics should be requested
    expect(reqMetricsSpy).toHaveBeenCalledTimes(2);
    expect(_.isEqual(transform.currValue, expValue)).toBeTruthy('Curr value was not upgraded correctly');
    expect(transform.stringForm).toBe('Percent CPU Used <= 10    DSC = naples1, naples2');

  }));

  it('transform query', () => {
    transform = new FieldSelectorTransform();
    reqMetricsSpy = spyOn(transform, 'requestMetrics');
    transform.measurement = 'DistributedServiceCard';

    // Query should remain unmodified
    let opts: any = {
      query: {},
    };
    transform.transformQuery(opts);
    expect(opts.query.selector.requirements.length).toBe(0);
    const defaultReq = {
      key: 'test',
      operator: 'equals',
      values: ['test']
    };
    opts = {
      query: {
        selector: {
          requirements: [
            defaultReq
          ]
        }
      },
    };
    transform.transformQuery(opts);
    expect(opts.query.selector.requirements.length).toBe(1);
    expect(_.isEqual(opts.query.selector.requirements, [defaultReq])).toBeTruthy();

    const newValue = [
      {
        keyFormControl: 'CPUUsedPercent',
        operatorFormControl: '<=',
        valueFormControl: [10],
      },
      {
        keyFormControl: 'reporterID',
        operatorFormControl: 'equals',
        valueFormControl: ['naples1', 'naples2'],
      },
    ];
    transform.currValue = newValue;
    transform.transformQuery(opts);
    const expResults =  {
      query: {
        selector: {
          requirements: [
            defaultReq,
            {
              key: 'CPUUsedPercent',
              operator: '<=',
              values: [10],
            },
            {
              key: 'reporterID',
              operator: 'equals',
              values: ['naples1', 'naples2'],
            }
          ]
        }
      },
    };
    expect(opts.query.selector.requirements.length).toBe(expResults.query.selector.requirements.length);
    const expReqs = expResults.query.selector.requirements;
    expReqs.forEach( expReq => {
      const match = opts.query.selector.requirements.find(x => _.isEqual(x.values, expReq.values));
      expect(match).toBeTruthy();
      expect(_.isEqual(match, expReq)).toBeTruthy('req did not math match: ' + JSON.stringify(match) + ' exp: ' + JSON.stringify(expReq));
    });
    expect(_.isEqual(opts, expResults)).toBeTruthy();
  });

  function verifyFieldData(selectedMeasurement, inDebugMode: boolean = false) {
    const fieldData = transform.fieldData;
    let expFields = MetricsMetadata[selectedMeasurement].fields;

    if (!inDebugMode) {
      expFields = expFields.filter(x => x.isTag);
    }

    expect(fieldData.length).toBe(expFields.length);
    expFields.forEach( (expField) => {
      const entry = fieldData.find( x => x.key.value === expField.name);
      expect(entry).toBeTruthy('Expected entry for ' + expField.name);
      expect(entry.key.label).toBe(expField.displayName);
      if (expField.jsType === 'string') {
        expect(entry.operators.length).toBe(2);
      } else {
        expect(entry.operators.length).toBe(4);
      }
    });
  }
});
