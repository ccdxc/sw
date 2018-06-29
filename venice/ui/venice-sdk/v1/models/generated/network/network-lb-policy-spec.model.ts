/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';

import { NetworkHealthCheckSpec } from './network-health-check-spec.model';

export interface INetworkLbPolicySpec {
    'type'?: string;
    'algorithm'?: string;
    'session-affinity'?: string;
    'health-check'?: NetworkHealthCheckSpec;
}


export class NetworkLbPolicySpec extends BaseModel implements INetworkLbPolicySpec {
    'type': string;
    'algorithm': string;
    'session-affinity': string;
    'health-check': NetworkHealthCheckSpec;
    public static enumProperties: { [key: string] : EnumDef } = {
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['health-check'] = new NetworkHealthCheckSpec();
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
            this['type'] = values['type'];
            this['algorithm'] = values['algorithm'];
            this['session-affinity'] = values['session-affinity'];
            this['health-check'].setValues(values['health-check']);
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'type': new FormControl(this['type']),
                'algorithm': new FormControl(this['algorithm']),
                'session-affinity': new FormControl(this['session-affinity']),
                'health-check': this['health-check'].$formGroup,
            });
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['type'].setValue(this['type']);
            this._formGroup.controls['algorithm'].setValue(this['algorithm']);
            this._formGroup.controls['session-affinity'].setValue(this['session-affinity']);
            this['health-check'].setFormGroupValues();
        }
    }
}

