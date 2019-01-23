/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringAutoMsgFwlogPolicyWatchHelperWatchEvent, IMonitoringAutoMsgFwlogPolicyWatchHelperWatchEvent } from './monitoring-auto-msg-fwlog-policy-watch-helper-watch-event.model';

export interface IMonitoringAutoMsgFwlogPolicyWatchHelper {
    'events'?: Array<IMonitoringAutoMsgFwlogPolicyWatchHelperWatchEvent>;
}


export class MonitoringAutoMsgFwlogPolicyWatchHelper extends BaseModel implements IMonitoringAutoMsgFwlogPolicyWatchHelper {
    'events': Array<MonitoringAutoMsgFwlogPolicyWatchHelperWatchEvent> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'events': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringAutoMsgFwlogPolicyWatchHelper.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringAutoMsgFwlogPolicyWatchHelper.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringAutoMsgFwlogPolicyWatchHelper.propInfo[prop] != null &&
                        MonitoringAutoMsgFwlogPolicyWatchHelper.propInfo[prop].default != null &&
                        MonitoringAutoMsgFwlogPolicyWatchHelper.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['events'] = new Array<MonitoringAutoMsgFwlogPolicyWatchHelperWatchEvent>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<MonitoringAutoMsgFwlogPolicyWatchHelperWatchEvent>(this, 'events', values['events'], MonitoringAutoMsgFwlogPolicyWatchHelperWatchEvent);
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
            this.fillFormArray<MonitoringAutoMsgFwlogPolicyWatchHelperWatchEvent>('events', this['events'], MonitoringAutoMsgFwlogPolicyWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<MonitoringAutoMsgFwlogPolicyWatchHelperWatchEvent>(this, 'events', this['events'], MonitoringAutoMsgFwlogPolicyWatchHelperWatchEvent);
        }
    }
}

