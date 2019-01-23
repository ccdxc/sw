/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringMatchedRequirement, IMonitoringMatchedRequirement } from './monitoring-matched-requirement.model';

export interface IMonitoringAlertReason {
    'matched-requirements'?: Array<IMonitoringMatchedRequirement>;
    'alert-policy-id'?: string;
}


export class MonitoringAlertReason extends BaseModel implements IMonitoringAlertReason {
    'matched-requirements': Array<MonitoringMatchedRequirement> = null;
    'alert-policy-id': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'matched-requirements': {
            type: 'object'
        },
        'alert-policy-id': {
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringAlertReason.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringAlertReason.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringAlertReason.propInfo[prop] != null &&
                        MonitoringAlertReason.propInfo[prop].default != null &&
                        MonitoringAlertReason.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['matched-requirements'] = new Array<MonitoringMatchedRequirement>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<MonitoringMatchedRequirement>(this, 'matched-requirements', values['matched-requirements'], MonitoringMatchedRequirement);
        } else {
            this['matched-requirements'] = [];
        }
        if (values && values['alert-policy-id'] != null) {
            this['alert-policy-id'] = values['alert-policy-id'];
        } else if (fillDefaults && MonitoringAlertReason.hasDefaultValue('alert-policy-id')) {
            this['alert-policy-id'] = MonitoringAlertReason.propInfo['alert-policy-id'].default;
        } else {
            this['alert-policy-id'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'matched-requirements': new FormArray([]),
                'alert-policy-id': new FormControl(this['alert-policy-id']),
            });
            // generate FormArray control elements
            this.fillFormArray<MonitoringMatchedRequirement>('matched-requirements', this['matched-requirements'], MonitoringMatchedRequirement);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<MonitoringMatchedRequirement>(this, 'matched-requirements', this['matched-requirements'], MonitoringMatchedRequirement);
            this._formGroup.controls['alert-policy-id'].setValue(this['alert-policy-id']);
        }
    }
}

