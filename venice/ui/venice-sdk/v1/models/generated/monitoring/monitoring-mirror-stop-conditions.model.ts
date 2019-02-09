/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IMonitoringMirrorStopConditions {
    'max-packets'?: number;
    'expiry-duration': string;
}


export class MonitoringMirrorStopConditions extends BaseModel implements IMonitoringMirrorStopConditions {
    'max-packets': number = null;
    /** should be a valid time duration of at most 2h0m0s
     */
    'expiry-duration': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'max-packets': {
            type: 'number'
        },
        'expiry-duration': {
            default: '2h',
            description:  'should be a valid time duration of at most 2h0m0s ',
            hint:  '2h',
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringMirrorStopConditions.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringMirrorStopConditions.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringMirrorStopConditions.propInfo[prop] != null &&
                        MonitoringMirrorStopConditions.propInfo[prop].default != null &&
                        MonitoringMirrorStopConditions.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['max-packets'] != null) {
            this['max-packets'] = values['max-packets'];
        } else if (fillDefaults && MonitoringMirrorStopConditions.hasDefaultValue('max-packets')) {
            this['max-packets'] = MonitoringMirrorStopConditions.propInfo['max-packets'].default;
        } else {
            this['max-packets'] = null
        }
        if (values && values['expiry-duration'] != null) {
            this['expiry-duration'] = values['expiry-duration'];
        } else if (fillDefaults && MonitoringMirrorStopConditions.hasDefaultValue('expiry-duration')) {
            this['expiry-duration'] = MonitoringMirrorStopConditions.propInfo['expiry-duration'].default;
        } else {
            this['expiry-duration'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'max-packets': CustomFormControl(new FormControl(this['max-packets']), MonitoringMirrorStopConditions.propInfo['max-packets'].description),
                'expiry-duration': CustomFormControl(new FormControl(this['expiry-duration'], [required, ]), MonitoringMirrorStopConditions.propInfo['expiry-duration'].description),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['max-packets'].setValue(this['max-packets']);
            this._formGroup.controls['expiry-duration'].setValue(this['expiry-duration']);
        }
    }
}

