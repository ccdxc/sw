/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';

import { MonitoringEvent } from './monitoring-event.model';

export interface IMonitoringEventList {
    'kind'?: string;
    'api-version'?: string;
    'resource-version'?: string;
    'Items'?: Array<MonitoringEvent>;
}


export class MonitoringEventList extends BaseModel implements IMonitoringEventList {
    'kind': string;
    'api-version': string;
    'resource-version': string;
    'Items': Array<MonitoringEvent>;
    public static enumProperties: { [key: string] : EnumDef } = {
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Items'] = new Array<MonitoringEvent>();
        if (values) {
            this.setValues(values);
        }
    }

    /**
     * set the values.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values) {
            this['kind'] = values['kind'];
            this['api-version'] = values['api-version'];
            this['resource-version'] = values['resource-version'];
            this.fillModelArray<MonitoringEvent>(this, 'Items', values['Items'], MonitoringEvent);
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'kind': new FormControl(this['kind']),
                'api-version': new FormControl(this['api-version']),
                'resource-version': new FormControl(this['resource-version']),
                'Items': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<MonitoringEvent>('Items', this['Items'], MonitoringEvent);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['kind'].setValue(this['kind']);
            this._formGroup.controls['api-version'].setValue(this['api-version']);
            this._formGroup.controls['resource-version'].setValue(this['resource-version']);
            this.fillModelArray<MonitoringEvent>(this, 'Items', this['Items'], MonitoringEvent);
        }
    }
}

