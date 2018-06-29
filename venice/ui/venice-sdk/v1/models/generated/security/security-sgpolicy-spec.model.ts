/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';

import { SecuritySGRule } from './security-sg-rule.model';

export interface ISecuritySgpolicySpec {
    'attach-groups'?: Array<string>;
    'in-rules'?: Array<SecuritySGRule>;
    'out-rules'?: Array<SecuritySGRule>;
}


export class SecuritySgpolicySpec extends BaseModel implements ISecuritySgpolicySpec {
    'attach-groups': Array<string>;
    'in-rules': Array<SecuritySGRule>;
    'out-rules': Array<SecuritySGRule>;
    public static enumProperties: { [key: string] : EnumDef } = {
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['attach-groups'] = new Array<string>();
        this['in-rules'] = new Array<SecuritySGRule>();
        this['out-rules'] = new Array<SecuritySGRule>();
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
            this.fillModelArray<string>(this, 'attach-groups', values['attach-groups']);
            this.fillModelArray<SecuritySGRule>(this, 'in-rules', values['in-rules'], SecuritySGRule);
            this.fillModelArray<SecuritySGRule>(this, 'out-rules', values['out-rules'], SecuritySGRule);
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'attach-groups': new FormArray([]),
                'in-rules': new FormArray([]),
                'out-rules': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<string>('attach-groups', this['attach-groups']);
            // generate FormArray control elements
            this.fillFormArray<SecuritySGRule>('in-rules', this['in-rules'], SecuritySGRule);
            // generate FormArray control elements
            this.fillFormArray<SecuritySGRule>('out-rules', this['out-rules'], SecuritySGRule);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this.fillModelArray<string>(this, 'attach-groups', this['attach-groups']);
            this.fillModelArray<SecuritySGRule>(this, 'in-rules', this['in-rules'], SecuritySGRule);
            this.fillModelArray<SecuritySGRule>(this, 'out-rules', this['out-rules'], SecuritySGRule);
        }
    }
}

