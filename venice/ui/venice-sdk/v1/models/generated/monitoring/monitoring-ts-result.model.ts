/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { MonitoringTimeWindow, IMonitoringTimeWindow } from './monitoring-time-window.model';

export interface IMonitoringTsResult {
    'time-window'?: IMonitoringTimeWindow;
    'report-url'?: string;
}


export class MonitoringTsResult extends BaseModel implements IMonitoringTsResult {
    /** time window for which the information was collected */
    'time-window': MonitoringTimeWindow = null;
    'report-url': string = null;
    public static propInfo: { [prop in keyof IMonitoringTsResult]: PropInfoItem } = {
        'time-window': {
            description:  'Time window for which the information was collected',
            required: false,
            type: 'object'
        },
        'report-url': {
            required: false,
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringTsResult.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringTsResult.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringTsResult.propInfo[prop] != null &&
                        MonitoringTsResult.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['time-window'] = new MonitoringTimeWindow();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this['time-window'].setValues(values['time-window'], fillDefaults);
        } else {
            this['time-window'].setValues(null, fillDefaults);
        }
        if (values && values['report-url'] != null) {
            this['report-url'] = values['report-url'];
        } else if (fillDefaults && MonitoringTsResult.hasDefaultValue('report-url')) {
            this['report-url'] = MonitoringTsResult.propInfo['report-url'].default;
        } else {
            this['report-url'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'time-window': CustomFormGroup(this['time-window'].$formGroup, MonitoringTsResult.propInfo['time-window'].required),
                'report-url': CustomFormControl(new FormControl(this['report-url']), MonitoringTsResult.propInfo['report-url']),
            });
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('time-window') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('time-window').get(field);
                control.updateValueAndValidity();
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this['time-window'].setFormGroupValuesToBeModelValues();
            this._formGroup.controls['report-url'].setValue(this['report-url']);
        }
    }
}

