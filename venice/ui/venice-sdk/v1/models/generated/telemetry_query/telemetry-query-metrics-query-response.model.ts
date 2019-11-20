/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { Telemetry_queryMetricsQueryResult, ITelemetry_queryMetricsQueryResult } from './telemetry-query-metrics-query-result.model';

export interface ITelemetry_queryMetricsQueryResponse {
    'tenant'?: string;
    'namespace'?: string;
    'results'?: Array<ITelemetry_queryMetricsQueryResult>;
}


export class Telemetry_queryMetricsQueryResponse extends BaseModel implements ITelemetry_queryMetricsQueryResponse {
    /** Tenant for the request */
    'tenant': string = null;
    /** Namespace for the request */
    'namespace': string = null;
    'results': Array<Telemetry_queryMetricsQueryResult> = null;
    public static propInfo: { [prop in keyof ITelemetry_queryMetricsQueryResponse]: PropInfoItem } = {
        'tenant': {
            description:  'Tenant for the request',
            required: false,
            type: 'string'
        },
        'namespace': {
            description:  'Namespace for the request',
            required: false,
            type: 'string'
        },
        'results': {
            required: false,
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return Telemetry_queryMetricsQueryResponse.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return Telemetry_queryMetricsQueryResponse.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (Telemetry_queryMetricsQueryResponse.propInfo[prop] != null &&
                        Telemetry_queryMetricsQueryResponse.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['results'] = new Array<Telemetry_queryMetricsQueryResult>();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['tenant'] != null) {
            this['tenant'] = values['tenant'];
        } else if (fillDefaults && Telemetry_queryMetricsQueryResponse.hasDefaultValue('tenant')) {
            this['tenant'] = Telemetry_queryMetricsQueryResponse.propInfo['tenant'].default;
        } else {
            this['tenant'] = null
        }
        if (values && values['namespace'] != null) {
            this['namespace'] = values['namespace'];
        } else if (fillDefaults && Telemetry_queryMetricsQueryResponse.hasDefaultValue('namespace')) {
            this['namespace'] = Telemetry_queryMetricsQueryResponse.propInfo['namespace'].default;
        } else {
            this['namespace'] = null
        }
        if (values) {
            this.fillModelArray<Telemetry_queryMetricsQueryResult>(this, 'results', values['results'], Telemetry_queryMetricsQueryResult);
        } else {
            this['results'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'tenant': CustomFormControl(new FormControl(this['tenant']), Telemetry_queryMetricsQueryResponse.propInfo['tenant']),
                'namespace': CustomFormControl(new FormControl(this['namespace']), Telemetry_queryMetricsQueryResponse.propInfo['namespace']),
                'results': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<Telemetry_queryMetricsQueryResult>('results', this['results'], Telemetry_queryMetricsQueryResult);
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('results') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('results').get(field);
                control.updateValueAndValidity();
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['tenant'].setValue(this['tenant']);
            this._formGroup.controls['namespace'].setValue(this['namespace']);
            this.fillModelArray<Telemetry_queryMetricsQueryResult>(this, 'results', this['results'], Telemetry_queryMetricsQueryResult);
        }
    }
}

