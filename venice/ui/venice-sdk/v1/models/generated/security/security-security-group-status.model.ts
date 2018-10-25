/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface ISecuritySecurityGroupStatus {
    'workloads'?: Array<string>;
    'Policies'?: Array<string>;
}


export class SecuritySecurityGroupStatus extends BaseModel implements ISecuritySecurityGroupStatus {
    'workloads': Array<string> = null;
    'Policies': Array<string> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'workloads': {
            type: 'Array<string>'
        },
        'Policies': {
            type: 'Array<string>'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecuritySecurityGroupStatus.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecuritySecurityGroupStatus.propInfo[prop] != null &&
                        SecuritySecurityGroupStatus.propInfo[prop].default != null &&
                        SecuritySecurityGroupStatus.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['workloads'] = new Array<string>();
        this['Policies'] = new Array<string>();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['workloads'] != null) {
            this['workloads'] = values['workloads'];
        }
        if (values && values['Policies'] != null) {
            this['Policies'] = values['Policies'];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'workloads': new FormControl(this['workloads']),
                'Policies': new FormControl(this['Policies']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['workloads'].setValue(this['workloads']);
            this._formGroup.controls['Policies'].setValue(this['Policies']);
        }
    }
}

