/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringAutoMsgAlertWatchHelperWatchEvent, IMonitoringAutoMsgAlertWatchHelperWatchEvent } from './monitoring-auto-msg-alert-watch-helper-watch-event.model';

export interface IMonitoringAutoMsgAlertWatchHelper {
    'Events'?: Array<IMonitoringAutoMsgAlertWatchHelperWatchEvent>;
}


export class MonitoringAutoMsgAlertWatchHelper extends BaseModel implements IMonitoringAutoMsgAlertWatchHelper {
    'Events': Array<MonitoringAutoMsgAlertWatchHelperWatchEvent> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'Events': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringAutoMsgAlertWatchHelper.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringAutoMsgAlertWatchHelper.propInfo[prop] != null &&
                        MonitoringAutoMsgAlertWatchHelper.propInfo[prop].default != null &&
                        MonitoringAutoMsgAlertWatchHelper.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Events'] = new Array<MonitoringAutoMsgAlertWatchHelperWatchEvent>();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<MonitoringAutoMsgAlertWatchHelperWatchEvent>(this, 'Events', values['Events'], MonitoringAutoMsgAlertWatchHelperWatchEvent);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'Events': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<MonitoringAutoMsgAlertWatchHelperWatchEvent>('Events', this['Events'], MonitoringAutoMsgAlertWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<MonitoringAutoMsgAlertWatchHelperWatchEvent>(this, 'Events', this['Events'], MonitoringAutoMsgAlertWatchHelperWatchEvent);
        }
    }
}

