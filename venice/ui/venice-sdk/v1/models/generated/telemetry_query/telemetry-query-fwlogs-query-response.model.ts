/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { Telemetry_queryFwlogsQueryResult, ITelemetry_queryFwlogsQueryResult } from './telemetry-query-fwlogs-query-result.model';

export interface ITelemetry_queryFwlogsQueryResponse {
    'tenant'?: string;
    'namespace'?: string;
    'results'?: Array<ITelemetry_queryFwlogsQueryResult>;
    '_ui'?: any;
}


export class Telemetry_queryFwlogsQueryResponse extends BaseModel implements ITelemetry_queryFwlogsQueryResponse {
    /** Field for holding arbitrary ui state */
    '_ui': any = {};
    /** Tenant for the request. */
    'tenant': string = null;
    /** Namespace for the request. */
    'namespace': string = null;
    'results': Array<Telemetry_queryFwlogsQueryResult> = null;
    public static propInfo: { [prop in keyof ITelemetry_queryFwlogsQueryResponse]: PropInfoItem } = {
        'tenant': {
            description:  `Tenant for the request.`,
            required: false,
            type: 'string'
        },
        'namespace': {
            description:  `Namespace for the request.`,
            required: false,
            type: 'string'
        },
        'results': {
            required: false,
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return Telemetry_queryFwlogsQueryResponse.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return Telemetry_queryFwlogsQueryResponse.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (Telemetry_queryFwlogsQueryResponse.propInfo[prop] != null &&
                        Telemetry_queryFwlogsQueryResponse.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['results'] = new Array<Telemetry_queryFwlogsQueryResult>();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['_ui']) {
            this['_ui'] = values['_ui']
        }
        if (values && values['tenant'] != null) {
            this['tenant'] = values['tenant'];
        } else if (fillDefaults && Telemetry_queryFwlogsQueryResponse.hasDefaultValue('tenant')) {
            this['tenant'] = Telemetry_queryFwlogsQueryResponse.propInfo['tenant'].default;
        } else {
            this['tenant'] = null
        }
        if (values && values['namespace'] != null) {
            this['namespace'] = values['namespace'];
        } else if (fillDefaults && Telemetry_queryFwlogsQueryResponse.hasDefaultValue('namespace')) {
            this['namespace'] = Telemetry_queryFwlogsQueryResponse.propInfo['namespace'].default;
        } else {
            this['namespace'] = null
        }
        if (values) {
            this.fillModelArray<Telemetry_queryFwlogsQueryResult>(this, 'results', values['results'], Telemetry_queryFwlogsQueryResult);
        } else {
            this['results'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'tenant': CustomFormControl(new FormControl(this['tenant']), Telemetry_queryFwlogsQueryResponse.propInfo['tenant']),
                'namespace': CustomFormControl(new FormControl(this['namespace']), Telemetry_queryFwlogsQueryResponse.propInfo['namespace']),
                'results': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<Telemetry_queryFwlogsQueryResult>('results', this['results'], Telemetry_queryFwlogsQueryResult);
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
            this.fillModelArray<Telemetry_queryFwlogsQueryResult>(this, 'results', this['results'], Telemetry_queryFwlogsQueryResult);
        }
    }
}

