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
 *
 * The form array always stores the user's intention.
 *  - Ex. label key, operator, label value
 *
 * During transform query, we expand these label requirements
 * into name, operator, objName requirements.
 */
export class LabelSelectorTransform extends MetricTransform {
  transformName = 'LabelSelector';

  labelData: RepeaterData[] = [];
  currValue: any[] = [];
  formArray = new FormArray([]);
  stringForm: string = '';
  sub: Subscription;

  constructor(public labelKindMap: { [kind: string]:
    { [labelKey: string]:
      { [labelValue: string]: string[] }
    }
  } ) {
    super();
  }

  onMeasurementChange() {
    // Reset the form array
    this.currValue = [];
    this.formArray = new FormArray([]);
    if (this.sub) {
      this.sub.unsubscribe();
    }
    this.sub = this.formArray.valueChanges.pipe(debounceTime(500)).subscribe(formValues => {
      const reqs = Utility.formatRepeaterData(formValues);
      if (!Utility.getLodash().isEqual(reqs, this.currValue)) {
        this.currValue = reqs;
        this.stringForm = Utility.stringifyRepeaterData(reqs).join('    ');
        this.requestMetrics();
      }
    });
    this.updateRepeaterOptions();
  }

  updateRepeaterOptions() {
    const objKind = MetricsMetadata[this.measurement].objectKind;
    const labels = this.labelKindMap[objKind];
    if (labels == null) {
      this.labelData = [];
      return;
    }

    const res: RepeaterData[] = [];
    Object.keys(labels).forEach( (key) => {
      const values = [];
      const valueMap = labels[key];
      Object.keys(valueMap).forEach( (value) => {
        values.push( {
          label: value,
          value: value,
        });
      });
      const resEntry: RepeaterData  = {
        key: {label: key, value: key},
        operators: SearchUtil.stringOperators,
        valueType: ValueType.multiSelect,
        values: values
      };
      if (resEntry.values.length === 0) {
        return;
      }
      res.push(resEntry);
    });

    this.labelData = res;
  }

  transformQuery(opts: TransformQuery) {
    if (opts.query.selector == null || opts.query.selector.requirements == null) {
     opts.query.selector =  {
      requirements: [],
     };
    }

    const reqs = this.currValue.map((item): IFieldsRequirement => {
      let values = item.valueFormControl;
      // Expand the selected labels into actual object names
      const objKind = MetricsMetadata[this.measurement].objectKind;
      const labelMap = this.labelKindMap[objKind][item.keyFormControl];
      values = values.map( (labelVal) => {
        return labelMap[labelVal];
      });
      if (values != null) {
        // Merging the array of arrays into a single level array and removing duplicates
        values = Utility.getLodash().uniq(values.reduce((a, b) => a.concat(b), []));
      }
      return {
        key: 'reporterID',
        operator: item.operatorFormControl,
        values: values
      };
    });
    opts.query.selector.requirements.push(...reqs);
  }
}


