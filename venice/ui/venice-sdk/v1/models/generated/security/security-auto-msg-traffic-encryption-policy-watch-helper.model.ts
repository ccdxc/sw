/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';

import { SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent } from './security-auto-msg-traffic-encryption-policy-watch-helper-watch-event.model';

export interface ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper {
    'Events'?: Array<SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent>;
}


export class SecurityAutoMsgTrafficEncryptionPolicyWatchHelper extends BaseModel implements ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper {
    'Events': Array<SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent>;
    public static enumProperties: { [key: string] : EnumDef } = {
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Events'] = new Array<SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent>();
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
            this.fillModelArray<SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent>(this, 'Events', values['Events'], SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent);
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'Events': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent>('Events', this['Events'], SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this.fillModelArray<SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent>(this, 'Events', this['Events'], SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent);
        }
    }
}

