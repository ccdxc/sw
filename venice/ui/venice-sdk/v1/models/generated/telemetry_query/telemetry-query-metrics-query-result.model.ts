/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { Telemetry_queryResultSeries, ITelemetry_queryResultSeries } from './telemetry-query-result-series.model';

export interface ITelemetry_queryMetricsQueryResult {
    'statement_id'?: number;
    'series'?: Array<ITelemetry_queryResultSeries>;
}


export class Telemetry_queryMetricsQueryResult extends BaseModel implements ITelemetry_queryMetricsQueryResult {
    'statement_id': number = null;
    'series': Array<Telemetry_queryResultSeries> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'statement_id': {
            type: 'number'
        },
        'series': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return Telemetry_queryMetricsQueryResult.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return Telemetry_queryMetricsQueryResult.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (Telemetry_queryMetricsQueryResult.propInfo[prop] != null &&
                        Telemetry_queryMetricsQueryResult.propInfo[prop].default != null &&
                        Telemetry_queryMetricsQueryResult.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['series'] = new Array<Telemetry_queryResultSeries>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['statement_id'] != null) {
            this['statement_id'] = values['statement_id'];
        } else if (fillDefaults && Telemetry_queryMetricsQueryResult.hasDefaultValue('statement_id')) {
            this['statement_id'] = Telemetry_queryMetricsQueryResult.propInfo['statement_id'].default;
        } else {
            this['statement_id'] = null
        }
        if (values) {
            this.fillModelArray<Telemetry_queryResultSeries>(this, 'series', values['series'], Telemetry_queryResultSeries);
        } else {
            this['series'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'statement_id': CustomFormControl(new FormControl(this['statement_id']), Telemetry_queryMetricsQueryResult.propInfo['statement_id'].description),
                'series': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<Telemetry_queryResultSeries>('series', this['series'], Telemetry_queryResultSeries);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['statement_id'].setValue(this['statement_id']);
            this.fillModelArray<Telemetry_queryResultSeries>(this, 'series', this['series'], Telemetry_queryResultSeries);
        }
    }
}

