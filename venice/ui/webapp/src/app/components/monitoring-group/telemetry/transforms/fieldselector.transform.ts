import { MetricTransform, TransformQuery, TransformNames } from './types';
import { RepeaterData, ValueType } from 'web-app-framework';
import { MetricsMetadata } from '@sdk/metrics/generated/metadata';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { Utility } from '@app/common/Utility';
import { FormArray, FormControl } from '@angular/forms';
import { debounceTime } from 'rxjs/operators';
import { IFieldsRequirement } from '@sdk/v1/models/generated/telemetry_query';
import { Subject } from 'rxjs';

interface FieldSelectorTransformConfig {
  selectedValues: any[];
}

/**
 * Modifies the telemetry query to include selector requirements
 * on the fields included in the current measurement.
 */
export class FieldSelectorTransform extends MetricTransform<FieldSelectorTransformConfig> {
  transformName = TransformNames.FieldSelector;

  // Backend does not currently support = and !=
  numberOperators = [
    { label: '>', value: 'gt' },
    { label: '>=', value: 'gte' },
    { label: '<', value: 'lt' },
    { label: '<=', value: 'lte' }
  ];

  fieldData: RepeaterData[];
  currValue: any[] = [];
  formArray = new FormArray([]);
  stringForm: string = '';
  valueChangeObserver: Subject<any> = new Subject();

  constructor() {
    super();
    this.valueChangeObserver.pipe(debounceTime(500)).subscribe(
      (value) => {
        this.handleValueChange(value);
      }
    );
  }

  onMeasurementChange() {
    // Reset the form array
    this.currValue = [];
    this.formArray = new FormArray([]);
    this.updateRepeaterOptions();
  }

  valueChange(event) {
    this.valueChangeObserver.next(event);
  }

  handleValueChange(event) {
    let values = Utility.formatRepeaterData(event);
    if (!Utility.getLodash().isEqual(values, this.currValue)) {
      this.currValue = values;
      // Replace key values with the displayName
      const fields = MetricsMetadata[this.measurement].fields;
      values = values.map( (req) => {
        const keyField = req.keyFormControl;
        const match = fields.find( (f) => {
          return f.name === keyField;
        });
        return {
          keyFormControl: match.displayName,
          operatorFormControl: req.operatorFormControl,
          valueFormControl: req.valueFormControl
        };
      });
      this.stringForm = Utility.stringifyRepeaterData(values).join('    ');
      this.requestMetrics();
    }
  }


  onDebugModeChange() {
    // Reset the form array
    this.currValue = [];
    this.formArray = new FormArray([]);
    this.updateRepeaterOptions();
  }

  updateRepeaterOptions() {
    const res: RepeaterData[] = [];
    if (this.measurement == null) {
      this.fieldData = res;
      return;
    }
    const fields = MetricsMetadata[this.measurement].fields;
    fields.forEach( (field) => {
      const type = field.jsType;
      let op;
      if (!this.debugMode && field.isTag) {
        op = SearchUtil.stringOperators;
      } else if (!this.debugMode) {
        // Don't add any non tag fields if we are not in debug mode
        return;
      }
      if (type === 'string') {
        op = SearchUtil.stringOperators;
      } else {
        op = this.numberOperators;
      }
      const resEntry: RepeaterData  = {
        key: {label: field.displayName, value: field.name},
        operators: op,
        valueType: ValueType.inputField
      };
      if (field.allowedValues != null) {
        resEntry.valueType = ValueType.multiSelect;
        resEntry.values = Utility.convertEnumToSelectItem (field.allowedValues);
      }
      res.push(resEntry);
    });
    this.fieldData = res;
  }

  buildFieldValuePlaceholder() {
    return 'Value 1, Value 2';
  }

  transformQuery(opts: TransformQuery) {
    if (opts.query.selector == null || opts.query.selector.requirements == null) {
     opts.query.selector =  {
      requirements: [],
     };
    }
    const values = this.currValue.map((item): IFieldsRequirement => {
      return {
        key: item.keyFormControl,
        operator: item.operatorFormControl,
        values: item.valueFormControl
      };
    });
    opts.query.selector.requirements.push(...values);
  }

  load(config: FieldSelectorTransformConfig) {
    if (config == null) {
      return;
    }
    const values = config.selectedValues;
    // Remove all old controls
    while (this.formArray.length !== 0) {
      this.formArray.removeAt(0);
    }
    values.forEach( (val) => {
      this.formArray.push(new FormControl(val));
    });
    this.currValue = values;
    // Form array subscription should run and update the rest of the variables
  }

  save(): FieldSelectorTransformConfig {
    return {
      selectedValues: this.currValue,
    };
  }
}


