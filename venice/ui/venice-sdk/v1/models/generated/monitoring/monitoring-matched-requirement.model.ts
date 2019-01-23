/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringMatchedRequirement_operator,  MonitoringMatchedRequirement_operator_uihint  } from './enums';

export interface IMonitoringMatchedRequirement {
    'key'?: string;
    'operator'?: MonitoringMatchedRequirement_operator;
    'values'?: Array<string>;
    'observed-value'?: string;
}


export class MonitoringMatchedRequirement extends BaseModel implements IMonitoringMatchedRequirement {
    'key': string = null;
    'operator': MonitoringMatchedRequirement_operator = null;
    'values': Array<string> = null;
    'observed-value': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'key': {
            type: 'string'
        },
        'operator': {
            enum: MonitoringMatchedRequirement_operator_uihint,
            default: 'equals',
            type: 'string'
        },
        'values': {
            type: 'Array<string>'
        },
        'observed-value': {
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringMatchedRequirement.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringMatchedRequirement.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringMatchedRequirement.propInfo[prop] != null &&
                        MonitoringMatchedRequirement.propInfo[prop].default != null &&
                        MonitoringMatchedRequirement.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['values'] = new Array<string>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['key'] != null) {
            this['key'] = values['key'];
        } else if (fillDefaults && MonitoringMatchedRequirement.hasDefaultValue('key')) {
            this['key'] = MonitoringMatchedRequirement.propInfo['key'].default;
        } else {
            this['key'] = null
        }
        if (values && values['operator'] != null) {
            this['operator'] = values['operator'];
        } else if (fillDefaults && MonitoringMatchedRequirement.hasDefaultValue('operator')) {
            this['operator'] = <MonitoringMatchedRequirement_operator>  MonitoringMatchedRequirement.propInfo['operator'].default;
        } else {
            this['operator'] = null
        }
        if (values && values['values'] != null) {
            this['values'] = values['values'];
        } else if (fillDefaults && MonitoringMatchedRequirement.hasDefaultValue('values')) {
            this['values'] = [ MonitoringMatchedRequirement.propInfo['values'].default];
        } else {
            this['values'] = [];
        }
        if (values && values['observed-value'] != null) {
            this['observed-value'] = values['observed-value'];
        } else if (fillDefaults && MonitoringMatchedRequirement.hasDefaultValue('observed-value')) {
            this['observed-value'] = MonitoringMatchedRequirement.propInfo['observed-value'].default;
        } else {
            this['observed-value'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'key': new FormControl(this['key']),
                'operator': new FormControl(this['operator'], [enumValidator(MonitoringMatchedRequirement_operator), ]),
                'values': new FormControl(this['values']),
                'observed-value': new FormControl(this['observed-value']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['key'].setValue(this['key']);
            this._formGroup.controls['operator'].setValue(this['operator']);
            this._formGroup.controls['values'].setValue(this['values']);
            this._formGroup.controls['observed-value'].setValue(this['observed-value']);
        }
    }
}

