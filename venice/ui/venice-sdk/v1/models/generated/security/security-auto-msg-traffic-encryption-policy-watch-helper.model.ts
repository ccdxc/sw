/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent, ISecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent } from './security-auto-msg-traffic-encryption-policy-watch-helper-watch-event.model';

export interface ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper {
    'events'?: Array<ISecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent>;
}


export class SecurityAutoMsgTrafficEncryptionPolicyWatchHelper extends BaseModel implements ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper {
    'events': Array<SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'events': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecurityAutoMsgTrafficEncryptionPolicyWatchHelper.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SecurityAutoMsgTrafficEncryptionPolicyWatchHelper.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecurityAutoMsgTrafficEncryptionPolicyWatchHelper.propInfo[prop] != null &&
                        SecurityAutoMsgTrafficEncryptionPolicyWatchHelper.propInfo[prop].default != null &&
                        SecurityAutoMsgTrafficEncryptionPolicyWatchHelper.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['events'] = new Array<SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent>(this, 'events', values['events'], SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent);
        } else {
            this['events'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'events': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent>('events', this['events'], SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent>(this, 'events', this['events'], SecurityAutoMsgTrafficEncryptionPolicyWatchHelperWatchEvent);
        }
    }
}

