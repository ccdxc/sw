/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IMonitoringAlertPolicyStatus {
    'total-hits'?: number;
    'open-alerts'?: number;
    'acknowledged-alerts'?: number;
}


export class MonitoringAlertPolicyStatus extends BaseModel implements IMonitoringAlertPolicyStatus {
    'total-hits': number = null;
    'open-alerts': number = null;
    'acknowledged-alerts': number = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'total-hits': {
            type: 'number'
        },
        'open-alerts': {
            type: 'number'
        },
        'acknowledged-alerts': {
            type: 'number'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringAlertPolicyStatus.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringAlertPolicyStatus.propInfo[prop] != null &&
                        MonitoringAlertPolicyStatus.propInfo[prop].default != null &&
                        MonitoringAlertPolicyStatus.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this.setValues(values);
    }

    /**
     * set the values. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values && values['total-hits'] != null) {
            this['total-hits'] = values['total-hits'];
        } else if (MonitoringAlertPolicyStatus.hasDefaultValue('total-hits')) {
            this['total-hits'] = MonitoringAlertPolicyStatus.propInfo['total-hits'].default;
        }
        if (values && values['open-alerts'] != null) {
            this['open-alerts'] = values['open-alerts'];
        } else if (MonitoringAlertPolicyStatus.hasDefaultValue('open-alerts')) {
            this['open-alerts'] = MonitoringAlertPolicyStatus.propInfo['open-alerts'].default;
        }
        if (values && values['acknowledged-alerts'] != null) {
            this['acknowledged-alerts'] = values['acknowledged-alerts'];
        } else if (MonitoringAlertPolicyStatus.hasDefaultValue('acknowledged-alerts')) {
            this['acknowledged-alerts'] = MonitoringAlertPolicyStatus.propInfo['acknowledged-alerts'].default;
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

