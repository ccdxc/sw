/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { MonitoringMirrorSessionStatus_schedule_state,  MonitoringMirrorSessionStatus_schedule_state_uihint  } from './enums';
import { MonitoringPropagationStatus, IMonitoringPropagationStatus } from './monitoring-propagation-status.model';

export interface IMonitoringMirrorSessionStatus {
    'schedule-state': MonitoringMirrorSessionStatus_schedule_state;
    'started-at'?: Date;
    'propagation-status'?: IMonitoringPropagationStatus;
    '_ui'?: any;
}


export class MonitoringMirrorSessionStatus extends BaseModel implements IMonitoringMirrorSessionStatus {
    /** Field for holding arbitrary ui state */
    '_ui': any = {};
    'schedule-state': MonitoringMirrorSessionStatus_schedule_state = null;
    'started-at': Date = null;
    'propagation-status': MonitoringPropagationStatus = null;
    public static propInfo: { [prop in keyof IMonitoringMirrorSessionStatus]: PropInfoItem } = {
        'schedule-state': {
            enum: MonitoringMirrorSessionStatus_schedule_state_uihint,
            default: 'none',
            required: true,
            type: 'string'
        },
        'started-at': {
            required: false,
            type: 'Date'
        },
        'propagation-status': {
            required: false,
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringMirrorSessionStatus.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringMirrorSessionStatus.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringMirrorSessionStatus.propInfo[prop] != null &&
                        MonitoringMirrorSessionStatus.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['propagation-status'] = new MonitoringPropagationStatus();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['_ui']) {
            this['_ui'] = values['_ui']
        }
        if (values && values['schedule-state'] != null) {
            this['schedule-state'] = values['schedule-state'];
        } else if (fillDefaults && MonitoringMirrorSessionStatus.hasDefaultValue('schedule-state')) {
            this['schedule-state'] = <MonitoringMirrorSessionStatus_schedule_state>  MonitoringMirrorSessionStatus.propInfo['schedule-state'].default;
        } else {
            this['schedule-state'] = null
        }
        if (values && values['started-at'] != null) {
            this['started-at'] = values['started-at'];
        } else if (fillDefaults && MonitoringMirrorSessionStatus.hasDefaultValue('started-at')) {
            this['started-at'] = MonitoringMirrorSessionStatus.propInfo['started-at'].default;
        } else {
            this['started-at'] = null
        }
        if (values) {
            this['propagation-status'].setValues(values['propagation-status'], fillDefaults);
        } else {
            this['propagation-status'].setValues(null, fillDefaults);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'schedule-state': CustomFormControl(new FormControl(this['schedule-state'], [required, enumValidator(MonitoringMirrorSessionStatus_schedule_state), ]), MonitoringMirrorSessionStatus.propInfo['schedule-state']),
                'started-at': CustomFormControl(new FormControl(this['started-at']), MonitoringMirrorSessionStatus.propInfo['started-at']),
                'propagation-status': CustomFormGroup(this['propagation-status'].$formGroup, MonitoringMirrorSessionStatus.propInfo['propagation-status'].required),
            });
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('propagation-status') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('propagation-status').get(field);
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
            this._formGroup.controls['schedule-state'].setValue(this['schedule-state']);
            this._formGroup.controls['started-at'].setValue(this['started-at']);
            this['propagation-status'].setFormGroupValuesToBeModelValues();
        }
    }
}

