/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';


export interface IMonitoringAlertPolicyStatus {
    'total-hits'?: number;
    'open-alerts'?: number;
    'acknowledged-alerts'?: number;
}


export class MonitoringAlertPolicyStatus extends BaseModel implements IMonitoringAlertPolicyStatus {
    'total-hits': number;
    'open-alerts': number;
    'acknowledged-alerts': number;
    public static enumProperties: { [key: string] : EnumDef } = {
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
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
            this['total-hits'] = values['total-hits'];
            this['open-alerts'] = values['open-alerts'];
            this['acknowledged-alerts'] = values['acknowledged-alerts'];
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'total-hits': new FormControl(this['total-hits']),
                'open-alerts': new FormControl(this['open-alerts']),
                'acknowledged-alerts': new FormControl(this['acknowledged-alerts']),
            });
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['total-hits'].setValue(this['total-hits']);
            this._formGroup.controls['open-alerts'].setValue(this['open-alerts']);
            this._formGroup.controls['acknowledged-alerts'].setValue(this['acknowledged-alerts']);
        }
    }
}

