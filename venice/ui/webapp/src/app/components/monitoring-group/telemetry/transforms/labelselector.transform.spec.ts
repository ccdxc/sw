import { LabelSelectorTransform } from './labelselector.transform';
import * as _ from 'lodash';
import { fakeAsync, tick } from '@angular/core/testing';
import { FormControl } from '@angular/forms';

describe('Label Selector transform', () => {
  let transform: LabelSelectorTransform;
  let reqMetricsSpy: jasmine.Spy;
  const labelKindMap = {
    'Node': {
      'env': {
        'prod': ['node1', 'node2'],
        'test': ['node3']
      }
    },
    'DistributedServiceCard': {
      'env': {
        'prod': ['naples1', 'naples2', 'naples3'],
        'test': ['naples4', 'naples5'],
        'dev': ['naples6']
      },
      'app': {
        'db': ['naples1', 'naples4'],
        'service': ['naples6']
      },
      'region': {
        'na': ['naples1']
      }
    }
  };

  /**
   * 1. Switching measurements should update the repeater options
   * 2. Form array should be replaced
   * 3. Form array callback should merge results.
   */
  it('on measurement change', fakeAsync(() => {

    transform = new LabelSelectorTransform(labelKindMap);
    reqMetricsSpy = spyOn(transform, 'requestMetrics');

    transform.measurement = 'Node';
    transform.onMeasurementChange();
    tick();

    // Verifying repeater data
    verifyLabelData('Node');


    // Verifying currValue and stringValue are updated when the
    // control is changed
    let newValue: any = [{
      keyFormControl: 'env',
      operatorFormControl: 'equals',
      valueFormControl: ['prod'],
    }];

    transform.formArray.push(new FormControl(newValue[0]));
    transform.valueChange(transform.formArray.value);
    tick(1000);

    // Metrics should be requested
    expect(reqMetricsSpy).toHaveBeenCalled();
    expect(_.isEqual(transform.currValue, newValue)).toBeTruthy('Curr value was not upgraded correctly');
    expect(transform.stringForm).toBe('env = prod');

    // Switching measurement
    transform.measurement = 'DistributedServiceCard';
    transform.onMeasurementChange();

    // Verifying repeater data
    verifyLabelData('DistributedServiceCard');

    // Verifying currValue and stringValue are updated when the
    // control is changed
    expect(_.isEqual(transform.currValue, [])).toBeTruthy('Curr value was not reset');

    newValue = [
      {
        keyFormControl: 'env',
        operatorFormControl: 'equals',
        valueFormControl: ['test', 'dev'],
      },
      {
        keyFormControl: 'app',
        operatorFormControl: 'equals',
        valueFormControl: ['db'],
      },
      { // incomplete req should be filtered out
        keyFormControl: 'region',
        operatorFormControl: 'equals',
      },
    ];
    const expValue = [
      {
        keyFormControl: 'env',
        operatorFormControl: 'equals',
        valueFormControl: ['test', 'dev'],
      },
      {
        keyFormControl: 'app',
        operatorFormControl: 'equals',
        valueFormControl: ['db'],
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
    expect(transform.stringForm).toBe('env = test, dev    app = db');

  }));

  it('transform query', () => {
    transform = new LabelSelectorTransform(labelKindMap);
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
        keyFormControl: 'env',
        operatorFormControl: 'equals',
        valueFormControl: ['test', 'dev'],
      },
      {
        keyFormControl: 'app',
        operatorFormControl: 'equals',
        valueFormControl: ['db'],
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
              key: 'reporterID',
              operator: 'equals',
              values: ['naples4', 'naples5', 'naples6'],
            },
            {
              key: 'reporterID',
              operator: 'equals',
              values: ['naples1', 'naples4'],
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

  function verifyLabelData(selectedMeasurement) {
    const labelData = transform.labelData;
    expect(labelData.length).toBe(Object.keys(labelKindMap[selectedMeasurement]).length, 'Repeater data lenght did not match');
    // keys
    Object.keys(labelKindMap[selectedMeasurement]).forEach((key) => {
      const entry = labelData.find(x => x.key.label === key);
      expect(entry).toBeTruthy('Expected entry for key ' + key);
      expect(entry.key.value).toBe(key);
      // Check values
      const expValues = Object.keys(labelKindMap[selectedMeasurement][key]);
      expValues.forEach( (expValue) => {
        const valEntry = entry.values.find(x => x.label === expValue);
        expect(valEntry).toBeTruthy('Expected value entry for key ' + key + ' with value ' + expValue);
        expect(valEntry.value).toBe(expValue);
      });
    });
  }
});
