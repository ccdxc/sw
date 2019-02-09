/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { LabelsSelector, ILabelsSelector } from './labels-selector.model';

export interface ISecuritySecurityGroupSpec {
    'workload-selector'?: ILabelsSelector;
    'service-labels'?: Array<string>;
    'match-prefixes'?: Array<string>;
}


export class SecuritySecurityGroupSpec extends BaseModel implements ISecuritySecurityGroupSpec {
    'workload-selector': LabelsSelector = null;
    'service-labels': Array<string> = null;
    'match-prefixes': Array<string> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'workload-selector': {
            type: 'object'
        },
        'service-labels': {
            type: 'Array<string>'
        },
        'match-prefixes': {
            type: 'Array<string>'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecuritySecurityGroupSpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SecuritySecurityGroupSpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecuritySecurityGroupSpec.propInfo[prop] != null &&
                        SecuritySecurityGroupSpec.propInfo[prop].default != null &&
                        SecuritySecurityGroupSpec.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['workload-selector'] = new LabelsSelector();
        this['service-labels'] = new Array<string>();
        this['match-prefixes'] = new Array<string>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this['workload-selector'].setValues(values['workload-selector'], fillDefaults);
        } else {
            this['workload-selector'].setValues(null, fillDefaults);
        }
        if (values && values['service-labels'] != null) {
            this['service-labels'] = values['service-labels'];
        } else if (fillDefaults && SecuritySecurityGroupSpec.hasDefaultValue('service-labels')) {
            this['service-labels'] = [ SecuritySecurityGroupSpec.propInfo['service-labels'].default];
        } else {
            this['service-labels'] = [];
        }
        if (values && values['match-prefixes'] != null) {
            this['match-prefixes'] = values['match-prefixes'];
        } else if (fillDefaults && SecuritySecurityGroupSpec.hasDefaultValue('match-prefixes')) {
            this['match-prefixes'] = [ SecuritySecurityGroupSpec.propInfo['match-prefixes'].default];
        } else {
            this['match-prefixes'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'workload-selector': this['workload-selector'].$formGroup,
                'service-labels': CustomFormControl(new FormControl(this['service-labels']), SecuritySecurityGroupSpec.propInfo['service-labels'].description),
                'match-prefixes': CustomFormControl(new FormControl(this['match-prefixes']), SecuritySecurityGroupSpec.propInfo['match-prefixes'].description),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this['workload-selector'].setFormGroupValuesToBeModelValues();
            this._formGroup.controls['service-labels'].setValue(this['service-labels']);
            this._formGroup.controls['match-prefixes'].setValue(this['match-prefixes']);
        }
    }
}

