/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IMonitoringAlertDestinationStatus {
    'total-notifications-sent'?: number;
}


export class MonitoringAlertDestinationStatus extends BaseModel implements IMonitoringAlertDestinationStatus {
    'total-notifications-sent': number = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'total-notifications-sent': {
            type: 'number'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringAlertDestinationStatus.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringAlertDestinationStatus.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringAlertDestinationStatus.propInfo[prop] != null &&
                        MonitoringAlertDestinationStatus.propInfo[prop].default != null &&
                        MonitoringAlertDestinationStatus.propInfo[prop].default != '');
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
        if (values && values['total-notifications-sent'] != null) {
            this['total-notifications-sent'] = values['total-notifications-sent'];
        } else if (fillDefaults && MonitoringAlertDestinationStatus.hasDefaultValue('total-notifications-sent')) {
            this['total-notifications-sent'] = MonitoringAlertDestinationStatus.propInfo['total-notifications-sent'].default;
        } else {
            this['total-notifications-sent'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'total-notifications-sent': CustomFormControl(new FormControl(this['total-notifications-sent']), MonitoringAlertDestinationStatus.propInfo['total-notifications-sent'].description),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['total-notifications-sent'].setValue(this['total-notifications-sent']);
        }
    }
}

