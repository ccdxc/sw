/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';


export interface IMonitoringTimeWindow {
    'start-time'?: Date;
    'stop-time'?: Date;
}


export class MonitoringTimeWindow extends BaseModel implements IMonitoringTimeWindow {
    /** Start/Stop Time - when start time is not specified, it implies start NOW. */
    'start-time': Date = null;
    /** Stop time - when not specified, default will be used */
    'stop-time': Date = null;
    public static propInfo: { [prop in keyof IMonitoringTimeWindow]: PropInfoItem } = {
        'start-time': {
            description:  'Start/Stop Time - when start time is not specified, it implies start NOW.',
            required: false,
            type: 'Date'
        },
        'stop-time': {
            description:  'Stop time - when not specified, default will be used',
            required: false,
            type: 'Date'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringTimeWindow.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringTimeWindow.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringTimeWindow.propInfo[prop] != null &&
                        MonitoringTimeWindow.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['start-time'] != null) {
            this['start-time'] = values['start-time'];
        } else if (fillDefaults && MonitoringTimeWindow.hasDefaultValue('start-time')) {
            this['start-time'] = MonitoringTimeWindow.propInfo['start-time'].default;
        } else {
            this['start-time'] = null
        }
        if (values && values['stop-time'] != null) {
            this['stop-time'] = values['stop-time'];
        } else if (fillDefaults && MonitoringTimeWindow.hasDefaultValue('stop-time')) {
            this['stop-time'] = MonitoringTimeWindow.propInfo['stop-time'].default;
        } else {
            this['stop-time'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'start-time': CustomFormControl(new FormControl(this['start-time']), MonitoringTimeWindow.propInfo['start-time']),
                'stop-time': CustomFormControl(new FormControl(this['stop-time']), MonitoringTimeWindow.propInfo['stop-time']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['start-time'].setValue(this['start-time']);
            this._formGroup.controls['stop-time'].setValue(this['stop-time']);
        }
    }
}

