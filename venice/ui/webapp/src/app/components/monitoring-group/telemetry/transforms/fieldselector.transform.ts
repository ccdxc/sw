import { MetricTransform, TransformQuery } from './types';
import { RepeaterData, ValueType } from 'web-app-framework';
import { MetricsMetadata } from '@sdk/metrics/generated/metadata';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { Utility } from '@app/common/Utility';
import { FormArray } from '@angular/forms';
import { debounceTime } from 'rxjs/operators';
import { IFieldsRequirement } from '@sdk/v1/models/generated/telemetry_query';
import { Subscription } from 'rxjs';


/**
 * Modifies the telemetry query to include selector requirements
 * on the fields included in the current measurement.
 */
export class FieldSelectorTransform extends MetricTransform {
  // Backend does not currently support = and !=
  numberOperators = [
    { label: '>', value: 'gt' },
    { label: '>=', value: 'gte' },
    { label: '<', value: 'lt' },
    { label: '<=', value: 'lte' }
  ];

  transformName = 'FieldSelector';
  fieldData: RepeaterData[];
  currValue: any[] = [];
  formArray = new FormArray([]);
  stringForm: string = '';
  sub: Subscription;

  onMeasurementChange() {
    // Reset the form array
    this.currValue = [];
    this.formArray = new FormArray([]);
    if (this.sub) {
      this.sub.unsubscribe();
    }
    this.sub = this.formArray.valueChanges.pipe(debounceTime(500)).subscribe(formValues => {
      let values = Utility.formatRepeaterData(formValues);
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
    });
    this.updateRepeaterOptions();
  }

  updateRepeaterOptions() {
    const fields = MetricsMetadata[this.measurement].fields;
    const res: RepeaterData[] = [];
    fields.forEach( (field) => {
      const type = field.jsType;
      let op;
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
    // Some metric field names may be very long, so we change the default to be blank
    return '';
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
}


