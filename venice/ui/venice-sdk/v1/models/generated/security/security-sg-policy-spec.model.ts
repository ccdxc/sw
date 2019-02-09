/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { SecuritySGRule, ISecuritySGRule } from './security-sg-rule.model';

export interface ISecuritySGPolicySpec {
    'attach-groups'?: Array<string>;
    'attach-tenant'?: boolean;
    'rules'?: Array<ISecuritySGRule>;
}


export class SecuritySGPolicySpec extends BaseModel implements ISecuritySGPolicySpec {
    'attach-groups': Array<string> = null;
    'attach-tenant': boolean = null;
    'rules': Array<SecuritySGRule> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'attach-groups': {
            type: 'Array<string>'
        },
        'attach-tenant': {
            type: 'boolean'
        },
        'rules': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecuritySGPolicySpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SecuritySGPolicySpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecuritySGPolicySpec.propInfo[prop] != null &&
                        SecuritySGPolicySpec.propInfo[prop].default != null &&
                        SecuritySGPolicySpec.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['attach-groups'] = new Array<string>();
        this['rules'] = new Array<SecuritySGRule>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['attach-groups'] != null) {
            this['attach-groups'] = values['attach-groups'];
        } else if (fillDefaults && SecuritySGPolicySpec.hasDefaultValue('attach-groups')) {
            this['attach-groups'] = [ SecuritySGPolicySpec.propInfo['attach-groups'].default];
        } else {
            this['attach-groups'] = [];
        }
        if (values && values['attach-tenant'] != null) {
            this['attach-tenant'] = values['attach-tenant'];
        } else if (fillDefaults && SecuritySGPolicySpec.hasDefaultValue('attach-tenant')) {
            this['attach-tenant'] = SecuritySGPolicySpec.propInfo['attach-tenant'].default;
        } else {
            this['attach-tenant'] = null
        }
        if (values) {
            this.fillModelArray<SecuritySGRule>(this, 'rules', values['rules'], SecuritySGRule);
        } else {
            this['rules'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'attach-groups': CustomFormControl(new FormControl(this['attach-groups']), SecuritySGPolicySpec.propInfo['attach-groups'].description),
                'attach-tenant': CustomFormControl(new FormControl(this['attach-tenant']), SecuritySGPolicySpec.propInfo['attach-tenant'].description),
                'rules': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<SecuritySGRule>('rules', this['rules'], SecuritySGRule);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['attach-groups'].setValue(this['attach-groups']);
            this._formGroup.controls['attach-tenant'].setValue(this['attach-tenant']);
            this.fillModelArray<SecuritySGRule>(this, 'rules', this['rules'], SecuritySGRule);
        }
    }
}

