/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringAlertDestination, IMonitoringAlertDestination } from './monitoring-alert-destination.model';

export interface IMonitoringAutoMsgAlertDestinationWatchHelperWatchEvent {
    'Type'?: string;
    'Object'?: IMonitoringAlertDestination;
}


export class MonitoringAutoMsgAlertDestinationWatchHelperWatchEvent extends BaseModel implements IMonitoringAutoMsgAlertDestinationWatchHelperWatchEvent {
    'Type': string = null;
    'Object': MonitoringAlertDestination = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'Type': {
            type: 'string'
        },
        'Object': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringAutoMsgAlertDestinationWatchHelperWatchEvent.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringAutoMsgAlertDestinationWatchHelperWatchEvent.propInfo[prop] != null &&
                        MonitoringAutoMsgAlertDestinationWatchHelperWatchEvent.propInfo[prop].default != null &&
                        MonitoringAutoMsgAlertDestinationWatchHelperWatchEvent.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Object'] = new MonitoringAlertDestination();
        this.setValues(values);
    }

    /**
     * set the values. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values && values['Type'] != null) {
            this['Type'] = values['Type'];
        } else if (MonitoringAutoMsgAlertDestinationWatchHelperWatchEvent.hasDefaultValue('Type')) {
            this['Type'] = MonitoringAutoMsgAlertDestinationWatchHelperWatchEvent.propInfo['Type'].default;
        }
        if (values) {
            this['Object'].setValues(values['Object']);
        }
    }




    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'Type': new FormControl(this['Type']),
                'Object': this['Object'].$formGroup,
            });
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['Type'].setValue(this['Type']);
            this['Object'].setFormGroupValues();
        }
    }
}

