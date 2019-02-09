/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringAutoMsgTroubleshootingSessionWatchHelperWatchEvent, IMonitoringAutoMsgTroubleshootingSessionWatchHelperWatchEvent } from './monitoring-auto-msg-troubleshooting-session-watch-helper-watch-event.model';

export interface IMonitoringAutoMsgTroubleshootingSessionWatchHelper {
    'events'?: Array<IMonitoringAutoMsgTroubleshootingSessionWatchHelperWatchEvent>;
}


export class MonitoringAutoMsgTroubleshootingSessionWatchHelper extends BaseModel implements IMonitoringAutoMsgTroubleshootingSessionWatchHelper {
    'events': Array<MonitoringAutoMsgTroubleshootingSessionWatchHelperWatchEvent> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'events': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringAutoMsgTroubleshootingSessionWatchHelper.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringAutoMsgTroubleshootingSessionWatchHelper.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringAutoMsgTroubleshootingSessionWatchHelper.propInfo[prop] != null &&
                        MonitoringAutoMsgTroubleshootingSessionWatchHelper.propInfo[prop].default != null &&
                        MonitoringAutoMsgTroubleshootingSessionWatchHelper.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['events'] = new Array<MonitoringAutoMsgTroubleshootingSessionWatchHelperWatchEvent>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<MonitoringAutoMsgTroubleshootingSessionWatchHelperWatchEvent>(this, 'events', values['events'], MonitoringAutoMsgTroubleshootingSessionWatchHelperWatchEvent);
        } else {
            this['events'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'events': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<MonitoringAutoMsgTroubleshootingSessionWatchHelperWatchEvent>('events', this['events'], MonitoringAutoMsgTroubleshootingSessionWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<MonitoringAutoMsgTroubleshootingSessionWatchHelperWatchEvent>(this, 'events', this['events'], MonitoringAutoMsgTroubleshootingSessionWatchHelperWatchEvent);
        }
    }
}

